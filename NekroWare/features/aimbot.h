#pragma once
#include <algorithm>
#include <cmath>
#include <mutex>
#include <cstring>
#include "../overlay/utils/W2S.h"
#include "../overlay/imgui/imgui.h"
#include "../rbx/globals/options.h"
#include "../rbx/globals/globals.h"
#include "../overlay/imgui/KeyBind.h"
#include "worldcache.h"

inline bool RayIntersectsAABB(const Vectors::Vector3& origin, const Vectors::Vector3& dir, float maxDist,
                              const Vectors::Vector3& center, const Vectors::Vector3& halfSize)
{
	float tmin = 0.0f;
	float tmax = maxDist;
	float t1max = -FLT_MAX;

	float origs[3] = { origin.x, origin.y, origin.z };
	float dirs[3] = { dir.x, dir.y, dir.z };
	float centers[3] = { center.x, center.y, center.z };
	float halves[3] = { halfSize.x, halfSize.y, halfSize.z };

	for (int i = 0; i < 3; ++i)
	{
		if (std::fabs(dirs[i]) < 1e-6f)
		{
			if (origs[i] < centers[i] - halves[i] || origs[i] > centers[i] + halves[i])
				return false;
		}
		else
		{
			float inv = 1.0f / dirs[i];
			float t1 = (centers[i] - halves[i] - origs[i]) * inv;
			float t2 = (centers[i] + halves[i] - origs[i]) * inv;
			if (t1 > t2)
				std::swap(t1, t2);
			t1max = (std::max)(t1max, t1);
			tmin = (std::max)(tmin, t1);
			tmax = (std::min)(tmax, t2);
			if (tmin > tmax)
				return false;
		}
	}
	// The box's entry point is behind the ray origin - the camera is clipping
	// into this part, so it must not block the ray.
	if (t1max < 0.0f)
		return false;
	return true;
}

// Transforms a world-space vector into a part's local space using the
// transpose of its rotation (orthonormal, so inverse == transpose).
inline Vectors::Vector3 InverseRotate(const Matrixes::Matrix3x3& m, const Vectors::Vector3& v)
{
	return Vectors::Vector3{
		m.r00 * v.x + m.r10 * v.y + m.r20 * v.z,
		m.r01 * v.x + m.r11 * v.y + m.r21 * v.z,
		m.r02 * v.x + m.r12 * v.y + m.r22 * v.z
	};
}

// Draws a rotated OBB (part local -> world) on screen as a 3D wireframe box.
inline void DrawOBB(ImDrawList* drawList, const Vectors::Vector3& center, const Vectors::Vector3& size, const Matrixes::Matrix3x3& rot, ImU32 color, float thickness = 1.5f)
{
	Vectors::Vector3 half = size * 0.5f;
	// 8 corners in local space.
	Vectors::Vector3 localCorners[8] = {
		{ -half.x, -half.y, -half.z }, { half.x, -half.y, -half.z },
		{ half.x,  half.y, -half.z }, { -half.x,  half.y, -half.z },
		{ -half.x, -half.y,  half.z }, { half.x, -half.y,  half.z },
		{ half.x,  half.y,  half.z }, { -half.x,  half.y,  half.z }
	};

	// Transform into world space via the rotation matrix (columns = axes).
	Vectors::Vector3 worldCorners[8];
	for (int i = 0; i < 8; i++)
	{
		auto& l = localCorners[i];
		worldCorners[i] = center + Vectors::Vector3{
			rot.r00 * l.x + rot.r01 * l.y + rot.r02 * l.z,
			rot.r10 * l.x + rot.r11 * l.y + rot.r12 * l.z,
			rot.r20 * l.x + rot.r21 * l.y + rot.r22 * l.z
		};
	}

	ImVec2 pts[8];
	for (int i = 0; i < 8; i++)
	{
		auto s = WorldToScreen(worldCorners[i]);
		pts[i] = ImVec2(s.x, s.y);
	}

	// 12 edges of the box.
	static const int edges[12][2] = {
		{0,1},{1,2},{2,3},{3,0},
		{4,5},{5,6},{6,7},{7,4},
		{0,4},{1,5},{2,6},{3,7}
	};
	for (auto& e : edges)
		drawList->AddLine(pts[e[0]], pts[e[1]], color, thickness);
}

// Walks one player's character subtree and collects every descendant part
// (bones, accessories, held tools...) so none of its own geometry ever counts
// as a blocker for its own ray.
inline void CollectSubtreeParts(const RobloxInstance& character, std::vector<uintptr_t>& out, int count)
{
	if (!character.address || count > 512)
		return;
	for (auto& child : character.GetChildren())
	{
		if (count > 512)
			return;
		if (!child.address)
			continue;
		uintptr_t primitive = Memory->read<uintptr_t>(child.address + Offsets::BasePart::Primitive);
		if (primitive != 0)
		{
			out.push_back(child.address);
			count++;
		}
		else
		{
			CollectSubtreeParts(child, out, count);
		}
	}
}

// Re-reads one part's geometry from the process right now (single syscall).
// Returns false for terrain-scale/stale entries.
inline bool ReadLivePart(AimbotVis::WorldPart& wp)
{
	if (!wp.primitive)
		return false;

	const uintptr_t SNAP_BASE = Offsets::Primitive::Rotation;
	const uintptr_t SNAP_POS = Offsets::Primitive::Position - SNAP_BASE;
	const uintptr_t SNAP_SIZE = Offsets::Primitive::Size - SNAP_BASE;
	const uintptr_t SNAP_LEN = Offsets::Primitive::Size - SNAP_BASE + sizeof(Vectors::Vector3);

	static thread_local std::vector<uint8_t> buffer;
	buffer.resize(SNAP_LEN);
	Memory->readRaw(wp.primitive + SNAP_BASE, buffer.data(), SNAP_LEN);

	memcpy(&wp.rotation, buffer.data() + 0, sizeof(wp.rotation));
	memcpy(&wp.position, buffer.data() + SNAP_POS, sizeof(wp.position));
	memcpy(&wp.size, buffer.data() + SNAP_SIZE, sizeof(wp.size));

	if (wp.size.x > 100000.0f || wp.size.y > 100000.0f || wp.size.z > 100000.0f)
	{
		wp.size = { 0.f, 0.f, 0.f };
		return false;
	}
	return true;
}

inline bool IsTargetVisible(const RobloxPlayer& target, const Vectors::Vector3& targetPos, AimbotVis::WorldPart* outBlock = nullptr)
{
	AimbotVis::DebugTested = 0;
	AimbotVis::DebugBlocked = 0;

	if (Globals::Roblox::Camera.address == 0)
		return true;

	Vectors::Vector3 camPos = Memory->read<Vectors::Vector3>(Globals::Roblox::Camera.address + Offsets::Camera::Position);

	Vectors::Vector3 delta = targetPos - camPos;
	float maxDist = delta.Magnitude();
	if (maxDist < 0.001f)
		return true;
	Vectors::Vector3 dir = delta * (1.0f / maxDist);

	std::shared_ptr<AimbotVis::WorldParts> parts;
	std::shared_ptr<std::vector<uintptr_t>> chars;
	{
		std::lock_guard<std::mutex> lock(AimbotVis::WorldMutex);
		parts = AimbotVis::World;
	}
	{
		std::lock_guard<std::mutex> lock(AimbotVis::CharactersMutex);
		chars = AimbotVis::Characters;
	}
	if (!parts)
		return true;

	for (auto& wp : *parts)
	{
		if (wp.size.x < 0.25f && wp.size.y < 0.25f && wp.size.z < 0.25f)
			continue; // tiny debris
		if (wp.size.x == 0.0f && wp.size.y == 0.0f && wp.size.z == 0.0f)
			continue; // stale/terrain entry

		AimbotVis::DebugTested++;

		// Cheap pass against the cached snapshot (may be up to ~150ms old).
		// The box is inflated (+50% extent) so a part that moved between
		// refreshes can never dodge the live re-verify below by sitting just
		// outside where the snapshot thinks it was.
		Vectors::Vector3 localOrigin = InverseRotate(wp.rotation, camPos - wp.position);
		Vectors::Vector3 localDir = InverseRotate(wp.rotation, dir);
		Vectors::Vector3 halfSize = wp.size * 0.75f;

		if (!RayIntersectsAABB(localOrigin, localDir, maxDist, { 0.f, 0.f, 0.f }, halfSize))
			continue;

		// Cached geometry says this part blocks. Verify against THIS frame's
		// live geometry before refusing to aim, so stale data can't cause a
		// wrong verdict for 150ms.
		AimbotVis::DebugBlocked++;
		if (!ReadLivePart(wp))
			continue;
		if (wp.size.x < 0.25f && wp.size.y < 0.25f && wp.size.z < 0.25f)
			continue;

		localOrigin = InverseRotate(wp.rotation, camPos - wp.position);
		localDir = InverseRotate(wp.rotation, dir);
		halfSize = wp.size * 0.5f;

		if (RayIntersectsAABB(localOrigin, localDir, maxDist, { 0.f, 0.f, 0.f }, halfSize))
		{
			// Final safety: walk the part's LIVE parent chain right now. If it
			// is currently attached to a known character model or the camera
			// (e.g. a body that just respawned and is being added to an avatar,
			// custom-avatar accessories, held weapons outside the model), it is
			// not a real cover - ignore it. Zero impact on the common case.
			uintptr_t cur = wp.address;
			bool isAvatarNow = false;
			for (int i = 0; i < 8 && cur != 0; i++)
			{
				if (cur == Globals::Roblox::Camera.address)
				{
					isAvatarNow = true;
					break;
				}
				if (cur == Globals::Roblox::Workspace.address)
					break;
				if (chars && IsOneOf(*chars, cur))
				{
					isAvatarNow = true;
					break;
				}
				cur = Memory->read<uintptr_t>(cur + Offsets::Instance::Parent);
			}
			if (isAvatarNow)
				continue;
			// Record what really blocked this ray for the menu debug.
			RobloxInstance blockInst(wp.address);
			std::string cls = blockInst.Class();
			std::string nm = blockInst.Name();
			{
				std::lock_guard<std::mutex> lock(AimbotVis::WorldMutex);
				strncpy_s(AimbotVis::LastBlockClass, cls.c_str(), sizeof(AimbotVis::LastBlockClass) - 1);
				strncpy_s(AimbotVis::LastBlockName, nm.c_str(), sizeof(AimbotVis::LastBlockName) - 1);
				AimbotVis::LastBlockPos = wp.position;
				AimbotVis::LastBlockSize = wp.size;
				AimbotVis::LastBlockRot = wp.rotation;
			}
			if (outBlock)
				*outBlock = wp;
			return false;
		}
	}
	return true;
}

inline Vectors::Vector3 GetVelocity(const RobloxInstance& part)
{
	if (!part.address)
		return Vectors::Vector3{ 0.f, 0.f, 0.f };
	
	uintptr_t primitiveAddr = Memory->read<uintptr_t>(part.address + Offsets::BasePart::Primitive);
	if (!primitiveAddr)
		return Vectors::Vector3{ 0.f, 0.f, 0.f };
	
	return Memory->read<Vectors::Vector3>(primitiveAddr + Offsets::Primitive::AssemblyLinearVelocity);
}

inline Vectors::Vector3 GetNearestBonePosition(const RobloxPlayer& player, bool applyPrediction = true)
{
    POINT p;
    GetCursorPos(&p);
    float cursorX = static_cast<float>(p.x);
    float cursorY = static_cast<float>(p.y);

    float bestDist = FLT_MAX;
    Vectors::Vector3 bestPos = player.Head.Position();
    RobloxInstance bestPart(0);

    auto tryBone = [&](const RobloxInstance& part) {
        if (!part.address) return;
        Vectors::Vector3 pos3D = part.Position();
        Vectors::Vector2 pos2D = WorldToScreen(pos3D);
        if (pos2D.x == -1 && pos2D.y == -1) return;
        float dx = pos2D.x - cursorX;
        float dy = pos2D.y - cursorY;
        float dist = dx * dx + dy * dy;
        if (dist < bestDist) {
            bestDist = dist;
            bestPos = pos3D;
            bestPart = part;
        }
    };

    if (Options::Aimbot::NearestHead)
        tryBone(player.Head);

    if (Options::Aimbot::NearestChest) {
        tryBone(player.HumanoidRootPart);
        if (player.RigType == 1) {
            tryBone(player.Upper_Torso);
            tryBone(player.Lower_Torso);
        }
    }

    if (Options::Aimbot::NearestLegs) {
        if (player.RigType == 0) {
            tryBone(player.Left_Leg);
            tryBone(player.Right_Leg);
        } else {
            tryBone(player.Left_Foot);
            tryBone(player.Right_Foot);
            tryBone(player.Left_Lower_Leg);
            tryBone(player.Right_Lower_Leg);
        }
    }

    if (applyPrediction && Options::Aimbot::Prediction && bestPart.address != 0) {
        Vectors::Vector3 velocity = GetVelocity(bestPart);
        return Vectors::Vector3{
            bestPos.x + velocity.x / Options::Aimbot::PredictionX,
            bestPos.y + velocity.y / Options::Aimbot::PredictionY,
            bestPos.z + velocity.z / Options::Aimbot::PredictionX
        };
    }

    return bestPos;
}

inline Vectors::Vector3 GetTargetPosition(const RobloxPlayer& player, bool applyPrediction = true)
{
    if (Options::Aimbot::NearestAim)
        return GetNearestBonePosition(player, applyPrediction);

    Vectors::Vector3 basePos;
    RobloxInstance targetPart(0);
    
    // Check if player is in air (Y velocity > 1 or < -1)
    Vectors::Vector3 velocity = GetVelocity(player.HumanoidRootPart);
    bool isInAir = (velocity.y > 1.0f || velocity.y < -1.0f);
    
    // Use air target bone if player is in air, otherwise use normal target bone
    int boneToUse = isInAir ? Options::Aimbot::AirTargetBone : Options::Aimbot::TargetBone;
    
    switch (boneToUse)
    {
        case 0: // Head
            targetPart = player.Head;
            basePos = player.Head.Position();
            break;
        case 1: // Torso/HumanoidRootPart
            targetPart = player.HumanoidRootPart;
            basePos = player.HumanoidRootPart.Position();
            break;
        case 2: // Left Arm
            if (player.RigType == 0)
            {
                targetPart = player.Left_Arm;
                basePos = player.Left_Arm.Position();
            }
            else
            {
                targetPart = player.Left_Hand;
                basePos = player.Left_Hand.Position();
            }
            break;
        case 3: // Right Arm
            if (player.RigType == 0)
            {
                targetPart = player.Right_Arm;
                basePos = player.Right_Arm.Position();
            }
            else
            {
                targetPart = player.Right_Hand;
                basePos = player.Right_Hand.Position();
            }
            break;
        case 4: // Left Leg
            if (player.RigType == 0)
            {
                targetPart = player.Left_Leg;
                basePos = player.Left_Leg.Position();
            }
            else
            {
                targetPart = player.Left_Foot;
                basePos = player.Left_Foot.Position();
            }
            break;
        case 5: // Right Leg
            if (player.RigType == 0)
            {
                targetPart = player.Right_Leg;
                basePos = player.Right_Leg.Position();
            }
            else
            {
                targetPart = player.Right_Foot;
                basePos = player.Right_Foot.Position();
            }
            break;
        case 6: // Lower Torso
            if (player.RigType == 1) // R15 only
            {
                targetPart = player.Lower_Torso;
                basePos = player.Lower_Torso.Position();
            }
            else
            {
                targetPart = player.HumanoidRootPart;
                basePos = player.HumanoidRootPart.Position();
            }
            break;
        case 7: // Upper Torso
            if (player.RigType == 1) // R15 only
            {
                targetPart = player.Upper_Torso;
                basePos = player.Upper_Torso.Position();
            }
            else
            {
                targetPart = player.HumanoidRootPart;
                basePos = player.HumanoidRootPart.Position();
            }
            break;
        default:
            targetPart = player.Head;
            basePos = player.Head.Position();
            break;
    }
    
    // Apply prediction if enabled
    if (applyPrediction && Options::Aimbot::Prediction && targetPart.address != 0)
    {
        Vectors::Vector3 velocity = GetVelocity(targetPart);
        
        // Divide velocity by prediction factors (higher value = less prediction)
        Vectors::Vector3 predictionOffset = {
            velocity.x / Options::Aimbot::PredictionX,
            velocity.y / Options::Aimbot::PredictionY,
            velocity.z / Options::Aimbot::PredictionX
        };
        
        // Add prediction offset to base position
        return Vectors::Vector3{
            basePos.x + predictionOffset.x,
            basePos.y + predictionOffset.y,
            basePos.z + predictionOffset.z
        };
    }
    
    return basePos;
}

inline RobloxPlayer GetClosestPlayer()
{
    RobloxPlayer target;
    auto maxDistance = FLT_MAX;
    auto localTeamState = LocalTeamState();
    auto localCharacter = Globals::Roblox::LocalPlayer.Character();
    auto localHRP = localCharacter.FindFirstChild("HumanoidRootPart");


    POINT p;
    GetCursorPos(&p);

    for (auto& player : Globals::Caches::CachedPlayerObjects)
    {
        auto HRP = player.HumanoidRootPart;
        if (!HRP.address)
            continue;

        if (IsLocalPlayer(player))
            continue;

        if (Options::Aimbot::TeamCheck && IsTeammate(player, localTeamState))
            continue;

        if (player.Health == 0)
            continue;

        // Skip knocked/downed players if check is enabled (health at or below 5)
        if (player.Health > 0 && player.Health <= 5.0f && Options::Aimbot::DownedCheck)
            continue;

        auto targetPos = GetTargetPosition(player);
        auto targetPos2D = WorldToScreen(targetPos);

        if (targetPos2D.x == -1 && targetPos2D.y == -1)
            continue;

        Vectors::Vector3 diff = localHRP.Position() - targetPos;
        float distance3D = diff.Magnitude();
        
        if (distance3D > Options::Aimbot::Range)
            continue;

        auto distance = targetPos2D.Distance({ static_cast<float>(p.x), static_cast<float>(p.y) });

        if (distance < maxDistance && distance <= Options::Aimbot::FOV)
        {
            if (Options::Aimbot::VisibilityCheck && !IsTargetVisible(player, GetTargetPosition(player, false)))
                continue;

            maxDistance = distance;
            target = player;
        }
    }
    return target;
}

inline float ApplySmoothnessCurve(float smoothness, int curveType)
{
    // Apply curve transformation based on selected type
    // Use exponential scaling for more balanced control across the range
    float t;
    switch (curveType)
    {
        case 0: // Linear - exponential scaling for better balance
        {
            // Map 0.0-1.0 smoothness to exponential speed curve
            // Lower values = faster, higher values = much slower
            float exponent = 1.0f + (smoothness * 4.0f); // 1.0 to 5.0
            t = pow(1.0f - smoothness, exponent);
            break;
        }
        case 1: // Ease In (starts slow, ends fast)
        {
            float exponent = 1.5f + (smoothness * 3.0f);
            t = pow(1.0f - smoothness, exponent);
            break;
        }
        case 2: // Ease Out (starts fast, ends slow)
        {
            float exponent = 2.0f + (smoothness * 2.5f);
            t = pow(1.0f - smoothness, exponent);
            break;
        }
        case 3: // Ease In-Out (smooth on both ends)
        {
            float exponent = 1.8f + (smoothness * 3.5f);
            t = pow(1.0f - smoothness, exponent);
            break;
        }
        case 4: // Custom Bezier Curve
        {
            if (Options::Aimbot::CustomCurveEnabled)
            {
                // Cubic Bezier curve with control points
                float p0 = 0.0f;
                float p1 = Options::Aimbot::CustomCurveP1[1];
                float p2 = Options::Aimbot::CustomCurveP2[1];
                float p3 = 1.0f;
                
                float u = 1.0f - smoothness;
                float tt = smoothness * smoothness;
                float ttt = tt * smoothness;
                float uu = u * u;
                float uuu = uu * u;
                
                // Bezier formula
                float curveValue = uuu * p0 + 3 * uu * smoothness * p1 + 3 * u * tt * p2 + ttt * p3;
                t = 1.0f - curveValue;
            }
            else
            {
                // Fallback to linear if custom not enabled
                float exponent = 1.0f + (smoothness * 4.0f);
                t = pow(1.0f - smoothness, exponent);
            }
            break;
        }
        default:
        {
            float exponent = 1.0f + (smoothness * 4.0f);
            t = pow(1.0f - smoothness, exponent);
            break;
        }
    }
    return std::clamp<float>(t, 0.001f, 1.0f);
}

inline void CameraRotation(const RobloxPlayer& target)
{
    Matrixes::Matrix3x3 currentRotation = Memory->read<Matrixes::Matrix3x3>(Globals::Roblox::Camera.address + Offsets::Camera::Rotation);

    sCFrame cameraCFrame = Globals::Roblox::Camera.CFrame();
    Vectors::Vector3 camPos = Memory->read<Vectors::Vector3>(Globals::Roblox::Camera.address + Offsets::Camera::Position);

    Vectors::Vector3 targetPos = GetTargetPosition(target);
    
    // Add shake if enabled
    if (Options::Aimbot::Shake && Options::Aimbot::ShakeIntensity > 0.0f)
    {
        static float shakeTime = 0.0f;
        shakeTime += 0.1f;
        
        float shakeX = sin(shakeTime * 10.0f) * Options::Aimbot::ShakeIntensity * 0.1f;
        float shakeY = cos(shakeTime * 8.0f) * Options::Aimbot::ShakeIntensity * 0.1f;
        float shakeZ = sin(shakeTime * 12.0f) * Options::Aimbot::ShakeIntensity * 0.1f;
        
        targetPos.x += shakeX;
        targetPos.y += shakeY;
        targetPos.z += shakeZ;
    }

    sCFrame lookAtCFrame = LookAt(camPos, targetPos);

    Vectors::Vector3 rightVec = lookAtCFrame.GetRightVector();
    Vectors::Vector3 upVec = lookAtCFrame.GetUpVector();
    Vectors::Vector3 lookVec = lookAtCFrame.GetLookVector();

    Matrixes::Matrix3x3 rotationMatrix
    {
        rightVec.x, upVec.x, lookVec.x,
        rightVec.y, upVec.y, lookVec.y,
        rightVec.z, upVec.z, lookVec.z
    };

    Vectors::Vector4 currentQuat = Vectors::Vector4::FromMatrix(currentRotation);
    Vectors::Vector4 targetQuat = Vectors::Vector4::FromMatrix(rotationMatrix);

    // Apply smoothness curve
    float t = ApplySmoothnessCurve(Options::Aimbot::Smoothness, Options::Aimbot::SmoothnessCurve);

    Vectors::Vector4 smoothedQuat = Vectors::Vector4::Slerp(currentQuat, targetQuat, t);
    Matrixes::Matrix3x3 smoothedMatrix = smoothedQuat.ToMatrix();

    Memory->write<Matrixes::Matrix3x3>(Globals::Roblox::Camera.address + Offsets::Camera::Rotation, smoothedMatrix);
}

inline void Mouse(const Vectors::Vector2& targetPos, const POINT& p)
{
    static float accumulatedX = 0.0f;
    static float accumulatedY = 0.0f;

    float dx = static_cast<float>(targetPos.x - p.x);
    float dy = static_cast<float>(targetPos.y - p.y);

    // Add shake if enabled
    if (Options::Aimbot::Shake && Options::Aimbot::ShakeIntensity > 0.0f)
    {
        static float shakeTime = 0.0f;
        shakeTime += 0.1f;
        
        float shakeX = sin(shakeTime * 10.0f) * Options::Aimbot::ShakeIntensity * 0.5f;
        float shakeY = cos(shakeTime * 8.0f) * Options::Aimbot::ShakeIntensity * 0.5f;
        
        dx += shakeX;
        dy += shakeY;
    }

    // Apply smoothness curve
    float t = ApplySmoothnessCurve(Options::Aimbot::Smoothness, Options::Aimbot::SmoothnessCurve);
    
    // Scale for mouse movement (higher = faster)
    float speedScale = 50.0f;
    t = t * speedScale;

    float moveX = dx * t;
    float moveY = dy * t;

    accumulatedX += moveX;
    accumulatedY += moveY;

    int intMoveX = static_cast<int>(accumulatedX);
    int intMoveY = static_cast<int>(accumulatedY);

    accumulatedX -= intMoveX;
    accumulatedY -= intMoveY;

    if (intMoveX != 0 || intMoveY != 0)
    {
        SetCursorPos(p.x + intMoveX, p.y + intMoveY);
    }
}

inline void MouseSendInput(const Vectors::Vector2& targetPos, const POINT& currentPos, float sensitivity)
{
    if (currentPos.x == targetPos.x && currentPos.y == targetPos.y)
        return;

    static float accumulatedX = 0.0f;
    static float accumulatedY = 0.0f;

    float dx = static_cast<float>(targetPos.x - currentPos.x);
    float dy = static_cast<float>(targetPos.y - currentPos.y);

    // Add shake if enabled
    if (Options::Aimbot::Shake && Options::Aimbot::ShakeIntensity > 0.0f)
    {
        static float shakeTime = 0.0f;
        shakeTime += 0.1f;
        
        float shakeX = sin(shakeTime * 10.0f) * Options::Aimbot::ShakeIntensity * 0.5f;
        float shakeY = cos(shakeTime * 8.0f) * Options::Aimbot::ShakeIntensity * 0.5f;
        
        dx += shakeX;
        dy += shakeY;
    }

    // Apply smoothness curve
    float t = ApplySmoothnessCurve(Options::Aimbot::Smoothness, Options::Aimbot::SmoothnessCurve);

    float sensitivityScale = 1.0f / (sensitivity + 0.2f);
    float speedScale = 0.5f;

    float moveX = dx * t * sensitivityScale * speedScale;
    float moveY = dy * t * sensitivityScale * speedScale;

    accumulatedX += moveX;
    accumulatedY += moveY;

    int intMoveX = static_cast<int>(accumulatedX);
    int intMoveY = static_cast<int>(accumulatedY);

    if (std::abs(dx) < 1.0f && std::abs(dy) < 1.0f)
    {
        accumulatedX = 0.0f;
        accumulatedY = 0.0f;
        return;
    }

    accumulatedX -= intMoveX;
    accumulatedY -= intMoveY;

    if (intMoveX != 0 || intMoveY != 0)
    {
        INPUT input = {};
        input.type = INPUT_MOUSE;
        input.mi.dx = intMoveX;
        input.mi.dy = intMoveY;
        input.mi.dwFlags = MOUSEEVENTF_MOVE;

        SendInput(1, &input, sizeof(INPUT));
    }
}

inline void RunAimbot(ImDrawList* drawList)
{
    // Check if aimbot is enabled first
    if (!Options::Aimbot::Aimbot)
        return;

    auto localTeamState = LocalTeamState();
    auto localCharacter = Globals::Roblox::LocalPlayer.Character();
    auto localHRP = localCharacter.FindFirstChild("HumanoidRootPart");

    auto Dimensions = Memory->read<Vectors::Vector2>(Globals::Roblox::VisualEngine + Offsets::VisualEngine::Dimensions);

    if (Globals::Caches::CachedPlayerObjects.empty())
        return;

    POINT p;
    GetCursorPos(&p);

    HWND robloxWindow = FindWindowA("Roblox", nullptr);
    if (robloxWindow)
    {
        ScreenToClient(robloxWindow, &p);
    }

    int CombatType;
    
    bool yAxisCheck;

    if (Dimensions.x < GetSystemMetrics(SM_CXSCREEN) || Dimensions.y < GetSystemMetrics(SM_CYSCREEN))
    {
        yAxisCheck = (p.y - Dimensions.y / 2) <= 25; // windowed mode
    }
    else
    {
        yAxisCheck = p.y == Dimensions.y / 2;
    }

    if (p.x == Dimensions.x / 2 && yAxisCheck)
    {                                          //likely in first person
        CombatType = 0; // FPS
    }
    else
    {
        CombatType = 1; // TPS
    }

    ImColor FOVColor = IM_COL32(
        static_cast<int>(Options::Aimbot::FOVColor[0] * 255.f),
        static_cast<int>(Options::Aimbot::FOVColor[1] * 255.f),
        static_cast<int>(Options::Aimbot::FOVColor[2] * 255.f),
        255);

    ImColor FOVFillColor = IM_COL32(
        static_cast<int>(Options::Aimbot::FOVFillColor[0] * 255.f),
        static_cast<int>(Options::Aimbot::FOVFillColor[1] * 255.f),
        static_cast<int>(Options::Aimbot::FOVFillColor[2] * 255.f),
        static_cast<int>(Options::Aimbot::FOVFillColor[3] * 255.f));

    if (Options::Aimbot::FOV && Options::Aimbot::ShowFOV)
    {
        drawList->AddCircle(ImVec2(static_cast<float>(p.x), static_cast<float>(p.y)), Options::Aimbot::FOV, FOVColor, 0, Options::Aimbot::FOVThickness);
        if (Options::Aimbot::ShowFOVFill)
        {
            drawList->AddCircleFilled(ImVec2(static_cast<float>(p.x), static_cast<float>(p.y)), Options::Aimbot::FOV, FOVFillColor, 0);
        }
    }

    // Toggle mode: detect key press edge (only trigger once per press)
    static bool wasKeyPressed = false;
    bool isKeyPressed = KeyBind::IsPressed(Options::Aimbot::AimbotKey);
    
    if (Options::Aimbot::ToggleType == 1)
    {
        // Toggle mode: only toggle on key press edge (not while held)
        if (isKeyPressed && !wasKeyPressed)
        {
            Options::Aimbot::Toggled = !Options::Aimbot::Toggled;
        }
        wasKeyPressed = isKeyPressed;
        
        // In toggle mode, check if toggled state is active
        if (!Options::Aimbot::Toggled)
        {
            Options::Aimbot::CurrentTarget = RobloxPlayer(0);
            return;
        }
    }
    else
    {
        // Hold mode: check if key is currently pressed
        if (!isKeyPressed)
        {
            Options::Aimbot::CurrentTarget = RobloxPlayer(0);
            Options::Aimbot::Toggled = false; // Reset toggle state when in hold mode
            return;
        }
    }

    // Stutter logic: skip aiming every X ticks
    static int stutterTickCounter = 0;
    if (Options::Aimbot::Stutter && Options::Aimbot::StutterTicks > 0)
    {
        stutterTickCounter++;
        if (stutterTickCounter >= Options::Aimbot::StutterTicks)
        {
            stutterTickCounter = 0;
            return; // Skip this tick
        }
    }
    else
    {
        stutterTickCounter = 0;
    }

    RobloxPlayer target;
    if (Options::Aimbot::StickyAim)
    {
        if (Options::Aimbot::CurrentTarget.address == 0 ||
            Options::Aimbot::CurrentTarget.Health == 0 ||
            (Options::Aimbot::CurrentTarget.Health <= 1 && Options::Aimbot::DownedCheck) ||
            (Options::Aimbot::TeamCheck && IsTeammate(Options::Aimbot::CurrentTarget, localTeamState)) ||
            (Options::Aimbot::VisibilityCheck && !IsTargetVisible(Options::Aimbot::CurrentTarget, GetTargetPosition(Options::Aimbot::CurrentTarget, false))))
        {
            Options::Aimbot::CurrentTarget = GetClosestPlayer();
        }
        else
        {
            // Check if current target is still within range
            auto targetPos = GetTargetPosition(Options::Aimbot::CurrentTarget);
            Vectors::Vector3 diff = targetPos - localHRP.Position();
            float distance3D = diff.Magnitude();
            
            if (distance3D > Options::Aimbot::Range)
            {
                Options::Aimbot::CurrentTarget = GetClosestPlayer();
            }
        }

        target = Options::Aimbot::CurrentTarget;
    }
    else
    {
        target = GetClosestPlayer();
    }

    float sensitivity;
    if (Offsets::MouseService::SensitivityPointer != 0)
        sensitivity = Memory->read<float>(Memory->getBaseAddress() + Offsets::MouseService::SensitivityPointer);
    else
        sensitivity = Options::Aimbot::Sensitivity;

    // Debug: draw a ray from the camera to every near enemy, green = visible,
    // red = blocked. Lets you SEE what the visibility check thinks.
    if (Options::Aimbot::VisibilityCheck && Options::Aimbot::DebugRays)
    {
        ImVec2 center2D = ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
        for (auto& player : Globals::Caches::CachedPlayerObjects)
        {
            if (IsLocalPlayer(player) || !player.HumanoidRootPart.address || player.Health <= 0)
                continue;
            if (Options::Aimbot::TeamCheck && IsTeammate(player, localTeamState))
                continue;
            auto aimPos = GetTargetPosition(player, false);
            Vectors::Vector3 diff = aimPos - localHRP.Position();
            if (diff.Magnitude() > Options::Aimbot::Range)
                continue;
            Vectors::Vector2 aim2D = WorldToScreen(aimPos);
            if (aim2D.x == -1.0f && aim2D.y == -1.0f)
                continue;
            AimbotVis::WorldPart blk;
            bool visible = IsTargetVisible(player, aimPos, &blk);
            ImColor color = visible ? IM_COL32(0, 255, 0, 200) : IM_COL32(255, 0, 0, 200);
            drawList->AddLine(center2D, ImVec2(aim2D.x, aim2D.y), color, 1.0f);
            if (!visible)
            {
                // Draw a yellow box around whatever blocked this enemy.
                if (blk.size.x > 0.0f && blk.size.y > 0.0f && blk.size.z > 0.0f)
                    DrawOBB(drawList, blk.position, blk.size, blk.rotation, IM_COL32(255, 255, 0, 220), 1.5f);
            }
        }
    }

    if (target.address != 0)
    {
        auto targetPos = WorldToScreen(GetTargetPosition(target));

        if (targetPos.x != -1 && targetPos.y != -1)
        {
            switch (CombatType)
            {
            case 0:
            {
                switch(Options::Aimbot::AimingType)
                {
                    case 0: // Camera
                    {
                        CameraRotation(target);
                        break;
                    }
                    case 1: // Mouse
                    {
                        MouseSendInput(targetPos, p, sensitivity);
                        break;
                    }
                }
                break;
            }

            case 1:
            {
                Mouse(targetPos, p);
                break;
            }

            default:
                break;
            }

        }
    }
}



