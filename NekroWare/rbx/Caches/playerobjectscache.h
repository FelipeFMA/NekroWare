#pragma once
#include <thread>
#include "../globals/options.h"
#include "../../rbx/globals/globals.h"

inline void CachePlayerObjects()
{
	std::vector<RobloxPlayer> tempList;

	while (true)
	{
		tempList.clear();

		if (Globals::Caches::CachedPlayers.empty())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		for (auto& player : Globals::Caches::CachedPlayers)
		{
			RobloxPlayer p;

			if (!player || player.address == 0)
				continue;

			p.address = player.address;

			// Check if this is a Player object or a Model (NPC)
			std::string className = player.Class();
			bool isNPC = (className == "Model");

			if (isNPC)
			{
				// For NPCs (models), the object IS the character
				p.Name = player.Name();
				p.DisplayName = "";
				p.UserId = 0;
				p.Team = RobloxInstance(0); // NPCs don't have teams
				p.Character = player;
				p.Humanoid = p.Character.FindFirstChildWhichIsA("Humanoid");
				p.Health = Memory->read<float>(p.Humanoid.address + Offsets::Humanoid::Health);
				p.MaxHealth = Memory->read<float>(p.Humanoid.address + Offsets::Humanoid::MaxHealth);
			}
			else
			{
				// For real players
				p.Name = player.Name();
				p.DisplayName = Memory->readString(Memory->read<uintptr_t>(player.address + Offsets::Player::DisplayName));
				p.UserId = Memory->read<int64_t>(player.address + Offsets::Player::UserId);
				p.Team = player.Team();
				p.Character = player.Character();
				p.Humanoid = p.Character.FindFirstChildWhichIsA("Humanoid");
				p.Health = player.Health();
				p.MaxHealth = player.MaxHealth();
			}

			p.RigType = p.Humanoid.RigType();

			p.Head = p.Character.FindFirstChild("Head");
			p.HumanoidRootPart = p.Character.FindFirstChild("HumanoidRootPart");

			switch (p.RigType)
			{
			case 0: // R6
				p.Left_Arm = p.Character.FindFirstChild("Left Arm");
				p.Left_Leg = p.Character.FindFirstChild("Left Leg");

				p.Right_Arm = p.Character.FindFirstChild("Right Arm");
				p.Right_Leg = p.Character.FindFirstChild("Right Leg");

				p.Torso = p.Character.FindFirstChild("Torso");

				break;
			case 1: // R15
				p.Upper_Torso = p.Character.FindFirstChild("UpperTorso");
				p.Lower_Torso = p.Character.FindFirstChild("LowerTorso");

				p.Right_Upper_Arm = p.Character.FindFirstChild("RightUpperArm");
				p.Right_Lower_Arm = p.Character.FindFirstChild("RightLowerArm");
				p.Right_Hand = p.Character.FindFirstChild("RightHand");

				p.Left_Upper_Arm = p.Character.FindFirstChild("LeftUpperArm");
				p.Left_Lower_Arm = p.Character.FindFirstChild("LeftLowerArm");
				p.Left_Hand = p.Character.FindFirstChild("LeftHand");

				p.Right_Upper_Leg = p.Character.FindFirstChild("RightUpperLeg");
				p.Right_Lower_Leg = p.Character.FindFirstChild("RightLowerLeg");
				p.Right_Foot = p.Character.FindFirstChild("RightFoot");

				p.Left_Upper_Leg = p.Character.FindFirstChild("LeftUpperLeg");
				p.Left_Lower_Leg = p.Character.FindFirstChild("LeftLowerLeg");
				p.Left_Foot = p.Character.FindFirstChild("LeftFoot");

				break;
			default:
				break;
			}

			// Held tool name (scanned each refresh cycle - lightweight since it's a single child scan)
			if (p.Character.address != 0)
			{
				auto tool = p.Character.FindFirstChildWhichIsA("Tool");
				if (tool.address != 0)
					p.ToolName = tool.Name();
				else
					p.ToolName = "";
			}

			tempList.push_back(p);
		}

		Globals::Caches::CachedPlayerObjects.clear();
		Globals::Caches::CachedPlayerObjects = tempList;

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}