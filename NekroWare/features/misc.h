#pragma once

#include "../rbx/globals/options.h"
#include "../rbx/globals/globals.h"

#include <thread>

inline void MiscLoop()
{
	static auto character = Globals::Roblox::LocalPlayer.Character();
	static auto humanoid = character.FindFirstChildWhichIsA("Humanoid");
	while (true)
	{
		character = Globals::Roblox::LocalPlayer.Character();
		humanoid = character.FindFirstChildWhichIsA("Humanoid");

		// Camera FOV
		if (Options::Misc::FOVEnabled)
		{
			Globals::Roblox::Camera.SetFOV(Options::Misc::FOV);
		}

		// Headless feature
		if (Options::ESP::Headless)
		{
			auto head = character.FindFirstChild("Head");
			if (head.address != 0)
			{
				Memory->write<float>(head.address + Offsets::BasePart::Transparency, 1.0f);
			}
		}

		// Infinite Jump
		if (Options::InfiniteJump::Enabled && humanoid.address != 0)
		{
			if (GetAsyncKeyState(VK_SPACE) & 0x8000)
			{
				Memory->write<bool>(humanoid.address + Offsets::Humanoid::Jump, true);
			}
		}

		// AutoJump
		if (Options::AutoJump::Enabled && humanoid.address != 0)
		{
			Memory->write<bool>(humanoid.address + Offsets::Humanoid::AutoJumpEnabled, true);
		}

		// PlatformStand
		if (Options::PlatformStand::Enabled && humanoid.address != 0)
		{
			Memory->write<bool>(humanoid.address + Offsets::Humanoid::PlatformStand, true);
		}

		// Noclip
		if (Options::Noclip::Enabled)
		{
			static bool wasNoclipKeyPressed = false;
			if (Options::Noclip::NoclipKey != 0)
			{
				bool isKeyPressed = (GetAsyncKeyState(Options::Noclip::NoclipKey) & 0x8000) != 0;
				if (Options::Noclip::ToggleType == 1)
				{
					if (isKeyPressed && !wasNoclipKeyPressed)
						Options::Noclip::Toggled = !Options::Noclip::Toggled;
					wasNoclipKeyPressed = isKeyPressed;
				}
				else
				{
					Options::Noclip::Toggled = isKeyPressed;
				}
			}
			else
			{
				Options::Noclip::Toggled = true;
			}

			if (Options::Noclip::Toggled && character.address != 0)
			{
				auto parts = character.GetChildren();
				for (auto& part : parts)
				{
					uintptr_t primitive = Memory->read<uintptr_t>(part.address + Offsets::BasePart::Primitive);
					if (primitive)
					{
						uint8_t collideFlags = Memory->read<uint8_t>(primitive + Offsets::PrimitiveFlags::CanCollide);
						if (collideFlags & 0x8)
							Memory->write<uint8_t>(primitive + Offsets::PrimitiveFlags::CanCollide, collideFlags & ~0x8);

						uint8_t queryFlags = Memory->read<uint8_t>(primitive + Offsets::PrimitiveFlags::CanQuery);
						if (queryFlags & 0x1)
							Memory->write<uint8_t>(primitive + Offsets::PrimitiveFlags::CanQuery, queryFlags & ~0x1);
					}
				}
			}
		}

		// Gravity Modifier (via Workspace -> World pointer chain)
		if (Options::GravityMod::Enabled && Globals::Roblox::Workspace.address != 0)
		{
			uintptr_t world = Memory->read<uintptr_t>(Globals::Roblox::Workspace.address + Offsets::Workspace::World);
			if (world != 0)
				Memory->write<float>(world + Offsets::World::Gravity, Options::GravityMod::Value);
			Memory->write<float>(Globals::Roblox::Workspace.address + Offsets::Workspace::ReadOnlyGravity, Options::GravityMod::Value);
		}

		// Jump Power Modifier
		if (Options::JumpPowerMod::Enabled && humanoid.address != 0)
		{
			Memory->write<float>(humanoid.address + Offsets::Humanoid::JumpPower, Options::JumpPowerMod::Value);
		}

		// NameOcclusion / Nameplate hiding
		if (Options::NameOcclusion::Enabled && humanoid.address != 0)
		{
			if (Options::NameOcclusion::HideNameplates)
			{
				Memory->write<float>(humanoid.address + Offsets::Humanoid::NameDisplayDistance, -1.0f);
				Memory->write<float>(humanoid.address + Offsets::Humanoid::HealthDisplayDistance, -1.0f);
			}
			else if (Options::NameOcclusion::NameDisplayDistance > 0.0f)
			{
				Memory->write<float>(humanoid.address + Offsets::Humanoid::NameDisplayDistance, Options::NameOcclusion::NameDisplayDistance);
				Memory->write<float>(humanoid.address + Offsets::Humanoid::HealthDisplayDistance, Options::NameOcclusion::HealthDisplayDistance);
			}
		}

		// AutoRotate
		if (Options::AutoRotate::Enabled && humanoid.address != 0)
		{
			Memory->write<bool>(humanoid.address + Offsets::Humanoid::AutoRotate, true);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}
