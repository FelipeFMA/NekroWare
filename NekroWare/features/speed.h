#pragma once
#include "../rbx/globals/globals.h"
#include "../rbx/globals/options.h"
#include <thread>
#include <chrono>

void SpeedLoop()
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        // Handle keybind - detect keydown for instant effect
        if (Options::WalkSpeed::WalkSpeedKey != 0)
        {
            static bool wasKeyPressed = false;
            bool isKeyPressed = (GetAsyncKeyState(Options::WalkSpeed::WalkSpeedKey) & 0x8000) != 0;

            // Detect keydown event (transition from not pressed to pressed)
            bool keyDown = isKeyPressed && !wasKeyPressed;

            if (Options::WalkSpeed::ToggleType == 1) // Toggle mode
            {
                if (isKeyPressed && !wasKeyPressed)
                {
                    Options::WalkSpeed::Toggled = !Options::WalkSpeed::Toggled;
                }
                wasKeyPressed = isKeyPressed;
            }
            else // Hold mode - apply instantly on keydown, stop when key released
            {
                // Apply speed instantly on keydown
                if (keyDown)
                {
                    Options::WalkSpeed::Toggled = true;
                }
                // Stop when key released
                else if (!isKeyPressed)
                {
                    Options::WalkSpeed::Toggled = false;
                }
            }
        }

        if (!Options::WalkSpeed::Enabled || !Options::WalkSpeed::Toggled)
            continue;

        try
        {
            auto localPlayer = Globals::Roblox::LocalPlayer;
            if (!localPlayer.address)
                continue;

            auto character = localPlayer.Character();
            if (!character.address)
                continue;

            auto humanoid = character.FindFirstChildWhichIsA("Humanoid");
            if (!humanoid.address)
                continue;

            auto humanoidRootPart = character.FindFirstChild("HumanoidRootPart");
            if (!humanoidRootPart.address)
                continue;

            // Velocity-based speed
            uintptr_t primitive = Memory->read<uintptr_t>(humanoidRootPart.address + Offsets::BasePart::Primitive);
            if (!primitive)
                continue;

            Vectors::Vector3 moveDir = Memory->read<Vectors::Vector3>(humanoid.address + Offsets::Humanoid::MoveDirection);
            Vectors::Vector3 currentVelocity = Memory->read<Vectors::Vector3>(primitive + Offsets::Primitive::AssemblyLinearVelocity);

            Vectors::Vector3 newVelocity(
                moveDir.x * Options::WalkSpeed::Speed,
                currentVelocity.y,
                moveDir.z * Options::WalkSpeed::Speed
            );
            Memory->write<Vectors::Vector3>(primitive + Offsets::Primitive::AssemblyLinearVelocity, newVelocity);
        }
        catch (...)
        {
            // Silently handle errors
        }
    }
}
