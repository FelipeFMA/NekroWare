#pragma once
#include <thread>
#include <mutex>
#include <memory>
#include <vector>
#include <cstdio>
#include <chrono>
#include <cstring>
#include "../rbx/globals/options.h"
#include "../rbx/globals/globals.h"

namespace AimbotVis
{
	struct WorldPart
	{
		uintptr_t address = 0;
		uintptr_t primitive = 0;
		Vectors::Vector3 position{ 0.f, 0.f, 0.f };
		Vectors::Vector3 size{ 0.f, 0.f, 0.f };
		Matrixes::Matrix3x3 rotation{}; // maps part-local -> world (columns = right/up/look)
	};

	using WorldParts = std::vector<WorldPart>;

	// Snapshot pointer. The render thread grabs it briefly under the mutex
	// (no copy) and iterates after the lock is dropped; the cache thread only
	// ever publishes fully built snapshots, never mutated in place.
	inline std::shared_ptr<WorldParts> World;
	inline std::mutex WorldMutex;

	// Current character-model addresses (for live parent-chain checks). Built
	// during the full walk; read by the aimbot thread to decide whether a
	// blocking part is really attached to an avatar this very moment.
	inline std::shared_ptr<std::vector<uintptr_t>> Characters;
	inline std::mutex CharactersMutex;

	inline void Clear()
	{
		std::lock_guard<std::mutex> lock(WorldMutex);
		World.reset();
		std::lock_guard<std::mutex> lockC(CharactersMutex);
		Characters.reset();
	}

	inline int Count()
	{
		std::lock_guard<std::mutex> lock(WorldMutex);
		return World ? static_cast<int>(World->size()) : 0;
	}

	inline int CharCount()
	{
		std::lock_guard<std::mutex> lock(CharactersMutex);
		return Characters ? static_cast<int>(Characters->size()) : 0;
	}

	// Per-check diagnostics, reset each IsTargetVisible call (menu only).
	inline int DebugTested = 0;
	inline int DebugBlocked = 0;

	// Info about the part that blocked the last visibility decision (menu).
	inline char LastBlockName[64] = "";
	inline char LastBlockClass[32] = "";
	inline Vectors::Vector3 LastBlockPos{ 0.f, 0.f, 0.f };
	inline Vectors::Vector3 LastBlockSize{ 0.f, 0.f, 0.f };
	inline Matrixes::Matrix3x3 LastBlockRot{};
}

inline bool IsBasePartClass(const std::string& className)
{
	static const char* partClasses[] = {
		"Part", "MeshPart", "WedgePart", "CornerWedgePart", "TrussPart",
		"SpawnLocation", "Seat", "VehicleSeat", "SkateboardPlatform",
		"UnionOperation", "NegateOperation", "PartOperation"
	};
	for (auto* c : partClasses)
		if (className == c)
			return true;
	return false;
}

inline bool IsOneOf(const std::vector<uintptr_t>& v, uintptr_t addr)
{
	for (auto& x : v)
		if (x == addr)
			return true;
	return false;
}

// All character models (players, NPCs, local) are NOT occluders - bodies must
// not block line-of-sight. Works for Player objects and NPC Models alike (an
// NPC's object IS its character).
inline void BuildCharacterSet(std::vector<uintptr_t>& chars)
{
	chars.clear();

	// Fresh read of Players each call - NOT the 5s-old CachedPlayers. On a
	// shooter every respawn re-parents the character to a NEW address; with
	// the stale cache the fresh body would be collected as an occluder for
	// up to 5 seconds (your own body after YOU respawn = everything blocked).
	if (Globals::Roblox::Players.address != 0)
	{
		for (auto& p : Globals::Roblox::Players.GetChildren())
		{
			if (!p.address)
				continue;
			auto character = p.Character();
			if (character.address)
				chars.push_back(character.address);
		}
	}

	// NPC models (a Model in CachedPlayers IS the character instance).
	for (auto& p : Globals::Caches::CachedPlayers)
	{
		if (!p.address)
			continue;
		if (p.Class() == "Model")
			chars.push_back(p.address);
	}

	auto local = Globals::Roblox::LocalPlayer.Character();
	if (local.address)
		chars.push_back(local.address);
}

// Walks the instance's parent chain (capped) and returns true if any
// ancestor is a known character model or the camera. Defensive against
// games that parent weapons/viewmodels (guns!) outside the character model
// but weld them on - e.g. first-person arms parented to the Camera.
inline bool BelongsToCharacterOrCamera(const RobloxInstance& part, const std::vector<uintptr_t>& characters)
{
	uintptr_t cur = part.address;
	for (int i = 0; i < 8 && cur != 0; i++)
	{
		if (IsOneOf(characters, cur))
			return true;
		if (cur == Globals::Roblox::Camera.address)
			return true;
		if (cur == Globals::Roblox::Workspace.address)
			return false;
		cur = Memory->read<uintptr_t>(cur + Offsets::Instance::Parent);
	}
	return false;
}

inline void CollectWorldPartAddresses(const RobloxInstance& inst, const std::vector<uintptr_t>& characters, std::vector<AimbotVis::WorldPart>& out, int depth, int& count)
{
	if (depth > 10 || count > 20000)
		return;

	for (auto& child : inst.GetChildren())
	{
		if (count > 20000)
			return;
		if (!child.address)
			continue;

		// Skip whole characters/subtrees - they are never occluders.
		if (IsOneOf(characters, child.address))
			continue;

		// Never descend into the camera - first-person weapon/viewmodel
		// parts live under it in some games and would block every ray.
		if (child.address == Globals::Roblox::Camera.address)
			continue;

		uintptr_t primitive = Memory->read<uintptr_t>(child.address + Offsets::BasePart::Primitive);
		if (primitive != 0 && IsBasePartClass(child.Class()))
		{
			// Defensive: a part welded to a character but parented elsewhere
			// (guns, accessories) must never occlude.
			if (BelongsToCharacterOrCamera(child, characters))
				continue;

			AimbotVis::WorldPart wp;
			wp.address = child.address;
			wp.primitive = primitive;
			out.push_back(wp);
			count++;
		}
		else
		{
			CollectWorldPartAddresses(child, characters, out, depth + 1, count);
		}
	}
}

// Reads rotation+position+size in one syscall (they are contiguous in the
// primitive: Rotation 0xc8, Position 0xec, Size 0x1b8). Only parts near the
// camera's aim range get refreshed each cycle - far geometry is irrelevant
// for occlusion and refreshing every part every tick is what ate a whole core.
inline void RefreshWorldPartGeometry(AimbotVis::WorldParts& parts)
{
	const uintptr_t SNAP_BASE = Offsets::Primitive::Rotation;
	const uintptr_t SNAP_POS = Offsets::Primitive::Position - SNAP_BASE;
	const uintptr_t SNAP_SIZE = Offsets::Primitive::Size - SNAP_BASE;
	const uintptr_t SNAP_LEN = Offsets::Primitive::Size - SNAP_BASE + sizeof(Vectors::Vector3);

	float radius = (std::max)(300.0f, Options::Aimbot::Range + 200.0f);
	float radiusSq = radius * radius;

	Vectors::Vector3 camPos{ 0.f, 0.f, 0.f };
	bool haveCam = Globals::Roblox::Camera.address != 0;
	if (haveCam)
		camPos = Memory->read<Vectors::Vector3>(Globals::Roblox::Camera.address + Offsets::Camera::Position);

	static thread_local std::vector<uint8_t> buffer;
	buffer.resize(SNAP_LEN);

	for (auto& wp : parts)
	{
		if (!wp.primitive)
			continue;

		// Fresh parts (size 0) always get their first read; already-sized parts
		// are refreshed only when they are inside the aim radius around the camera.
		bool neverSized = wp.size.x == 0.0f && wp.size.y == 0.0f && wp.size.z == 0.0f;
		if (!neverSized && haveCam)
		{
			float dx = wp.position.x - camPos.x;
			float dy = wp.position.y - camPos.y;
			float dz = wp.position.z - camPos.z;
			if (dx * dx + dy * dy + dz * dz > radiusSq)
				continue;
		}

		Memory->readRaw(wp.primitive + SNAP_BASE, buffer.data(), SNAP_LEN);

		memcpy(&wp.rotation, buffer.data() + 0, sizeof(wp.rotation));
		memcpy(&wp.position, buffer.data() + SNAP_POS, sizeof(wp.position));
		memcpy(&wp.size, buffer.data() + SNAP_SIZE, sizeof(wp.size));

		// Terrain AABBs cover the whole map - skip, they would block every ray.
		if (wp.size.x > 100000.0f || wp.size.y > 100000.0f || wp.size.z > 100000.0f)
			wp.size = { 0.f, 0.f, 0.f };
	}
}

inline void CacheWorldParts()
{
	auto lastAddressRefresh = std::chrono::steady_clock::now() - std::chrono::seconds(10);
	int lastLoggedCount = -1;

	while (true)
	{
		if (Globals::Roblox::Workspace.address == 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(2000));
			continue;
		}

		auto now = std::chrono::steady_clock::now();

		// Full tree walk once per second (addresses change rarely); geometry is
		// refreshed every cycle so moving parts stay accurate.
		if (std::chrono::duration_cast<std::chrono::seconds>(now - lastAddressRefresh).count() >= 1)
		{
			std::vector<uintptr_t> characters;
			BuildCharacterSet(characters);

			{
				auto charSnap = std::make_shared<std::vector<uintptr_t>>(characters);
				std::lock_guard<std::mutex> lock(AimbotVis::CharactersMutex);
				AimbotVis::Characters = charSnap;
			}

			std::vector<AimbotVis::WorldPart> fresh;
			int count = 0;
			CollectWorldPartAddresses(Globals::Roblox::Workspace, characters, fresh, 0, count);

			RefreshWorldPartGeometry(fresh);

			auto snapshot = std::make_shared<AimbotVis::WorldParts>(std::move(fresh));
			{
				std::lock_guard<std::mutex> lock(AimbotVis::WorldMutex);
				AimbotVis::World = snapshot;
			}

			if (count != lastLoggedCount)
			{
				lastLoggedCount = count;
				printf("[+] Visibility cache: %d world parts\n", count);
			}

			lastAddressRefresh = now;
			std::this_thread::sleep_for(std::chrono::milliseconds(150));
			continue;
		}

		// Incremental geometry refresh: copy the snapshot, update it, republish.
		{
			std::lock_guard<std::mutex> lock(AimbotVis::WorldMutex);
			if (AimbotVis::World)
			{
				auto refreshed = std::make_shared<AimbotVis::WorldParts>(*AimbotVis::World);
				RefreshWorldPartGeometry(*refreshed);
				AimbotVis::World = refreshed;
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(150));
	}
}