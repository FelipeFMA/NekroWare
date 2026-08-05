#pragma once

#include "../overlay/utils/W2S.h"
#include "../overlay/imgui/imgui.h"
#include "../rbx/globals/options.h"
#include "../rbx/globals/globals.h"
#include <algorithm>
#include <vector>
#include <cmath>

inline bool ESP_OutlineOn(bool perVisual)
{
    return Options::ESP::OutlineEnabled && perVisual;
}

inline void RenderESP(ImDrawList* drawList, bool menuOpen = false)
{
    auto localTeam = Globals::Roblox::LocalPlayer.Team();
    auto localHead = Globals::Roblox::LocalPlayer.Character().FindFirstChild("Head");
    auto localTorso = Globals::Roblox::LocalPlayer.Character().FindFirstChild("Torso");
    if (localTorso.address == 0)
        localTorso = Globals::Roblox::LocalPlayer.Character().FindFirstChild("UpperTorso");
    auto localHeadPos = localHead.Position();
    auto localTorsoPos = localTorso.Position();
    auto localTorsoPos2D = WorldToScreen(localTorsoPos);
    auto Dimensions = Memory->read<Vectors::Vector2>(Globals::Roblox::VisualEngine + Offsets::VisualEngine::Dimensions);

    auto cameraPos = Memory->read<Vectors::Vector3>(Globals::Roblox::Camera.address + Offsets::Camera::Position);

    // Camera CFrame - used for both view-space transform (offscreen arrows) and normal rendering
    sCFrame camCFrame = Globals::Roblox::Camera.CFrame();
    Vectors::Vector3 camRight = camCFrame.GetRightVector();
    Vectors::Vector3 camUp = camCFrame.GetUpVector();
    Vectors::Vector3 camLook = camCFrame.GetLookVector();

    ImFont* font = ImGui::GetFont();

    if (Globals::Caches::CachedPlayerObjects.empty())
        return;

    for (auto& player : Globals::Caches::CachedPlayerObjects)
    {
        if (player.address == Globals::Roblox::LocalPlayer.address)
            continue;

        if (player.Health <= 0)
            continue;

        // Team check: skip teammates if enabled
        if (Options::ESP::TeamCheck && player.Team.address != 0 && localTeam.address != 0 &&
            player.Team.address == localTeam.address)
            continue;

        auto rigType = player.RigType;

        auto head = player.Head;

        RobloxInstance torso(0), rightLeg(0), leftLeg(0), rightArm(0), leftArm(0);
        if (rigType == 0) {
            torso = player.Torso;
            rightLeg = player.Right_Leg;
            leftLeg = player.Left_Leg;
            rightArm = player.Right_Arm;
            leftArm = player.Left_Arm;
        }
        else if (rigType == 1) {
            torso = player.Upper_Torso;
            rightLeg = player.Right_Foot;
            leftLeg = player.Left_Foot;
            rightArm = player.Right_Hand;
            leftArm = player.Left_Hand;
        }

        auto head3D = head.Position();
        auto torso3D = torso.Position();
        auto rLeg3D = rightLeg.Position();
        auto lLeg3D = leftLeg.Position();
        auto rArm3D = rightArm.Position();
        auto lArm3D = leftArm.Position();

        auto head2D = WorldToScreen(head3D);
        auto torso2D = WorldToScreen(torso3D);
        auto rLeg2D = WorldToScreen(rLeg3D);
        auto lLeg2D = WorldToScreen(lLeg3D);
        auto rArm2D = WorldToScreen(rArm3D);
        auto lArm2D = WorldToScreen(lArm3D);

        // Offscreen arrow: when player is out of frustum (WorldToScreen returned -1,-1),
        // draw an arrow on the screen-edge circle pointing toward them.
        if (torso2D.x == -1 || torso2D.y == -1)
        {
            if (Options::ESP::OffscreenArrows)
            {
                Vectors::Vector3 relative = torso3D - cameraPos;
                // Project onto camera basis: forward (look), right, up
                float dotLook = relative.x * camLook.x + relative.y * camLook.y + relative.z * camLook.z;
                float dotRight = relative.x * camRight.x + relative.y * camRight.y + relative.z * camRight.z;
                float dotUp = relative.x * camUp.x + relative.y * camUp.y + relative.z * camUp.z;

                float cx = Dimensions.x * 0.5f;
                float cy = Dimensions.y * 0.5f;
                float radius = std::min<float>(Dimensions.x, Dimensions.y) * 0.4f;

                float angle;
                if (dotLook > 0.0f)
                    angle = atan2f(dotRight, std::max<float>(dotLook, 0.01f));
                else
                    // Behind: use 2D angle in the right/up plane, flipped so it points backward
                    angle = atan2f(dotRight, -std::abs(dotLook)) + 3.14159265f;

                // Place arrow on the circle border (FOV ring position)
                float ax = cx + cosf(angle) * radius;
                float ay = cy + sinf(angle) * radius;

                ImU32 arrowCol = IM_COL32(
                    (int)(Options::ESP::OffscreenArrowColor[0] * 255.f),
                    (int)(Options::ESP::OffscreenArrowColor[1] * 255.f),
                    (int)(Options::ESP::OffscreenArrowColor[2] * 255.f),
                    220);

                // Small triangle pointing outward from center
                float tipLen = 12.0f;
                float baseLen = 7.0f;
                float dirX = cosf(angle);
                float dirY = sinf(angle);
                ImVec2 tip(ax + dirX * tipLen, ay + dirY * tipLen);
                ImVec2 baseL(ax - dirY * baseLen, ay + dirX * baseLen);
                ImVec2 baseR(ax + dirY * baseLen, ay - dirX * baseLen);
                drawList->AddTriangleFilled(tip, baseL, baseR, arrowCol);
                drawList->AddTriangle(tip, baseL, baseR, IM_COL32(0, 0, 0, 200), 1.0f);
            }
            continue;
        }

        float distance = (localHeadPos - head3D).Magnitude();
        float camDistance = (cameraPos - head3D).Magnitude();
        float scale = std::clamp(1.0f / camDistance * 20.0f, 0.8f, 2.0f);

        auto newHeadPos = WorldToScreen(head3D + Vectors::Vector3{ 0.f, 0.8f, 0.f });
        auto newHeadName = WorldToScreen(head3D + Vectors::Vector3{ 0.f, 2.2f, 0.f });
        auto newLeftLeg = WorldToScreen(lLeg3D - Vectors::Vector3{ 0.f, 1.f, 0.f });
        auto newRightLeg = WorldToScreen(rLeg3D - Vectors::Vector3{ 2.5f, 0.0f, 0.f });
        auto newHeadPosHealth = WorldToScreen(head3D - Vectors::Vector3{ 2.f, 0.f, 0.f });

        auto hrpCFrame = player.HumanoidRootPart.CFrame();
        Vectors::Vector3 rightVector = hrpCFrame.GetRightVector();

        float halfWidth = 2.f;

        Vectors::Vector3 leftWorld = torso3D - (rightVector * halfWidth);
        Vectors::Vector3 rightWorld = torso3D + (rightVector * halfWidth);

        auto left2D = WorldToScreen(leftWorld);
        auto right2D = WorldToScreen(rightWorld);

        ImVec2 topLeft(std::min<float>(left2D.x, right2D.x), newHeadPos.y);
        ImVec2 bottomRight(std::max<float>(left2D.x, right2D.x), newLeftLeg.y);
        int roundedDistance = static_cast<int>(std::round(distance));

        std::vector<ImVec2> points = {
            ImVec2(newHeadPos.x, newHeadPos.y), ImVec2(torso2D.x, torso2D.y), ImVec2(newLeftLeg.x, newLeftLeg.y),
            ImVec2(rLeg2D.x, rLeg2D.y), ImVec2(lArm2D.x, lArm2D.y), ImVec2(rArm2D.x, rArm2D.y),
            ImVec2(left2D.x, left2D.y), ImVec2(right2D.x, right2D.y)
        };

        float left = FLT_MAX, top = FLT_MAX;
        float right = -FLT_MAX, bottom = -FLT_MAX;

        for (const auto& pt : points)
        {
            if (pt.x == -1 || pt.y == -1)
                continue;

            left = std::min<float>(left, pt.x);
            right = std::max<float>(right, pt.x);
            top = std::min<float>(top, pt.y);
            bottom = std::max<float>(bottom, pt.y);
        }

        ImColor	color = IM_COL32(
            static_cast<int>(Options::ESP::Color[0] * 255.f),
            static_cast<int>(Options::ESP::Color[1] * 255.f),
            static_cast<int>(Options::ESP::Color[2] * 255.f),
            255);

        ImColor	boxColor = IM_COL32(
            static_cast<int>(Options::ESP::BoxColor[0] * 255.f),
            static_cast<int>(Options::ESP::BoxColor[1] * 255.f),
            static_cast<int>(Options::ESP::BoxColor[2] * 255.f),
            255);

        ImColor	skeletonColor = IM_COL32(
            static_cast<int>(Options::ESP::SkeletonColor[0] * 255.f),
            static_cast<int>(Options::ESP::SkeletonColor[1] * 255.f),
            static_cast<int>(Options::ESP::SkeletonColor[2] * 255.f),
            255);

        ImColor	distanceColor = IM_COL32(
            static_cast<int>(Options::ESP::DistanceColor[0] * 255.f),
            static_cast<int>(Options::ESP::DistanceColor[1] * 255.f),
            static_cast<int>(Options::ESP::DistanceColor[2] * 255.f),
            255);

        ImColor	tracerColor = IM_COL32(
            static_cast<int>(Options::ESP::TracerColor[0] * 255.f),
            static_cast<int>(Options::ESP::TracerColor[1] * 255.f),
            static_cast<int>(Options::ESP::TracerColor[2] * 255.f),
            255);

        ImColor	cornerColor = IM_COL32(
            static_cast<int>(Options::ESP::CornerColor[0] * 255.f),
            static_cast<int>(Options::ESP::CornerColor[1] * 255.f),
            static_cast<int>(Options::ESP::CornerColor[2] * 255.f),
            255);

        // Chams: filled fill inside the box (drawn before box outline so box sits on top)
        if (!menuOpen && left < right && top < bottom &&
            (Options::ESP::BoxType == 1 || Options::ESP::BoxType == 2))
        {
            int chamsAlpha = 61; // Default alpha (~0.24 * 255, same as old ChamsAlpha default)
            ImU32 chamsFill = IM_COL32(
                (int)(Options::ESP::ChamsColor[0] * 255.f),
                (int)(Options::ESP::ChamsColor[1] * 255.f),
                (int)(Options::ESP::ChamsColor[2] * 255.f),
                chamsAlpha);
            drawList->AddRectFilled(ImVec2(left, top), ImVec2(right, bottom), chamsFill);
        }

        if (Options::ESP::BoxType == 1) // Normal Box
        {

            if (left < right && top < bottom)
            {
                if (ESP_OutlineOn(Options::ESP::BoxOutline))
                    drawList->AddRect(ImVec2(left, top), ImVec2(right, bottom), IM_COL32(0, 0, 0, 255), 0, 0, Options::ESP::BoxThickness + 1.5f);
                drawList->AddRect(ImVec2(left, top), ImVec2(right, bottom), boxColor, 0, 0, Options::ESP::BoxThickness);
            }
        }

        if (Options::ESP::CornerESP && left < right && top < bottom)
        {
            float w = right - left;
            float h = bottom - top;
            float cornerLen = (w < h ? w : h) * 0.25f;
            float thick = Options::ESP::BoxThickness;
            float outThick = thick + 1.5f;

            ImVec2 tl(left, top), tr(right, top), bl(left, bottom), br(right, bottom);

            if (ESP_OutlineOn(Options::ESP::CornerOutline))
            {
                drawList->AddLine(tl, ImVec2(tl.x + cornerLen, tl.y), IM_COL32(0, 0, 0, 255), outThick);
                drawList->AddLine(tl, ImVec2(tl.x, tl.y + cornerLen), IM_COL32(0, 0, 0, 255), outThick);
                drawList->AddLine(tr, ImVec2(tr.x - cornerLen, tr.y), IM_COL32(0, 0, 0, 255), outThick);
                drawList->AddLine(tr, ImVec2(tr.x, tr.y + cornerLen), IM_COL32(0, 0, 0, 255), outThick);
                drawList->AddLine(bl, ImVec2(bl.x + cornerLen, bl.y), IM_COL32(0, 0, 0, 255), outThick);
                drawList->AddLine(bl, ImVec2(bl.x, bl.y - cornerLen), IM_COL32(0, 0, 0, 255), outThick);
                drawList->AddLine(br, ImVec2(br.x - cornerLen, br.y), IM_COL32(0, 0, 0, 255), outThick);
                drawList->AddLine(br, ImVec2(br.x, br.y - cornerLen), IM_COL32(0, 0, 0, 255), outThick);
            }

            drawList->AddLine(tl, ImVec2(tl.x + cornerLen, tl.y), cornerColor, thick);
            drawList->AddLine(tl, ImVec2(tl.x, tl.y + cornerLen), cornerColor, thick);
            drawList->AddLine(tr, ImVec2(tr.x - cornerLen, tr.y), cornerColor, thick);
            drawList->AddLine(tr, ImVec2(tr.x, tr.y + cornerLen), cornerColor, thick);
            drawList->AddLine(bl, ImVec2(bl.x + cornerLen, bl.y), cornerColor, thick);
            drawList->AddLine(bl, ImVec2(bl.x, bl.y - cornerLen), cornerColor, thick);
            drawList->AddLine(br, ImVec2(br.x - cornerLen, br.y), cornerColor, thick);
            drawList->AddLine(br, ImVec2(br.x, br.y - cornerLen), cornerColor, thick);
        }

        if (Options::ESP::Tracers)
        {
            float tracerThickness = Options::ESP::TracerThickness;
            float outlineThickness = tracerThickness + 1.5f;
            bool tracerOutline = ESP_OutlineOn(Options::ESP::TracerOutline);

            switch (Options::ESP::TracersStart)
            {
            case 0: // Bottom
                if (tracerOutline)
                    drawList->AddLine(ImVec2(Dimensions.x / 2, Dimensions.y), ImVec2(head2D.x, head2D.y), IM_COL32(0, 0, 0, 255), outlineThickness);
                drawList->AddLine(ImVec2(Dimensions.x / 2, Dimensions.y), ImVec2(head2D.x, head2D.y), tracerColor, tracerThickness);
                break;
            case 1: // Top
                if (tracerOutline)
                    drawList->AddLine(ImVec2(Dimensions.x / 2, 0), ImVec2(head2D.x, head2D.y), IM_COL32(0, 0, 0, 255), outlineThickness);
                drawList->AddLine(ImVec2(Dimensions.x / 2, 0), ImVec2(head2D.x, head2D.y), tracerColor, tracerThickness);
                break;
            case 2: // Mouse
                POINT point;
                GetCursorPos(&point);

                if (tracerOutline)
                    drawList->AddLine(ImVec2(point.x, point.y), ImVec2(head2D.x, head2D.y), IM_COL32(0, 0, 0, 255), outlineThickness);
                drawList->AddLine(ImVec2(point.x, point.y), ImVec2(head2D.x, head2D.y), tracerColor, tracerThickness);
                break;
            case 3: // Torso
                if (tracerOutline)
                    drawList->AddLine(ImVec2(localTorsoPos2D.x, localTorsoPos2D.y), ImVec2(head2D.x, head2D.y), IM_COL32(0, 0, 0, 255), outlineThickness);
                drawList->AddLine(ImVec2(localTorsoPos2D.x, localTorsoPos2D.y), ImVec2(head2D.x, head2D.y), tracerColor, tracerThickness);
                break;
            }
        }

        if (Options::ESP::Skeleton)
        {
            auto leftShoulder = WorldToScreen(torso3D + Vectors::Vector3{ -1.0f, 0.5f, 0.f });
            auto rightShoulder = WorldToScreen(torso3D + Vectors::Vector3{ 1.0f, 0.5f, 0.f });
            auto hipCenter = WorldToScreen(torso3D + Vectors::Vector3{ 0.f, -0.5f, 0.f });

            auto leftHand = WorldToScreen(lArm3D + Vectors::Vector3{ -0.5f, 0.f, 0.f });
            auto rightHand = WorldToScreen(rArm3D + Vectors::Vector3{ 0.5f, 0.f, 0.f });
            auto leftFoot = WorldToScreen(lLeg3D + Vectors::Vector3{ -0.3f, -0.5f, 0.f });
            auto rightFoot = WorldToScreen(rLeg3D + Vectors::Vector3{ 0.3f, -0.5f, 0.f });

            float outlineThickness = Options::ESP::SkeletonThickness + 1.5f;
            bool skOutline = ESP_OutlineOn(Options::ESP::SkeletonOutline);

            if (skOutline)
                drawList->AddLine(ImVec2(head2D.x, head2D.y), ImVec2(torso2D.x, torso2D.y), IM_COL32(0, 0, 0, 255), outlineThickness);
            drawList->AddLine(ImVec2(head2D.x, head2D.y), ImVec2(torso2D.x, torso2D.y), skeletonColor, Options::ESP::SkeletonThickness);

            if (skOutline)
                drawList->AddLine(ImVec2(leftShoulder.x, leftShoulder.y), ImVec2(lArm2D.x, lArm2D.y), IM_COL32(0, 0, 0, 255), outlineThickness);
            drawList->AddLine(ImVec2(leftShoulder.x, leftShoulder.y), ImVec2(lArm2D.x, lArm2D.y), skeletonColor, Options::ESP::SkeletonThickness);
            if (skOutline)
                drawList->AddLine(ImVec2(lArm2D.x, lArm2D.y), ImVec2(leftHand.x, leftHand.y), IM_COL32(0, 0, 0, 255), outlineThickness);
            drawList->AddLine(ImVec2(lArm2D.x, lArm2D.y), ImVec2(leftHand.x, leftHand.y), skeletonColor, Options::ESP::SkeletonThickness);

            if (skOutline)
                drawList->AddLine(ImVec2(rightShoulder.x, rightShoulder.y), ImVec2(rArm2D.x, rArm2D.y), IM_COL32(0, 0, 0, 255), outlineThickness);
            drawList->AddLine(ImVec2(rightShoulder.x, rightShoulder.y), ImVec2(rArm2D.x, rArm2D.y), skeletonColor, Options::ESP::SkeletonThickness);
            if (skOutline)
                drawList->AddLine(ImVec2(rArm2D.x, rArm2D.y), ImVec2(rightHand.x, rightHand.y), IM_COL32(0, 0, 0, 255), outlineThickness);
            drawList->AddLine(ImVec2(rArm2D.x, rArm2D.y), ImVec2(rightHand.x, rightHand.y), skeletonColor, Options::ESP::SkeletonThickness);

            if (skOutline)
                drawList->AddLine(ImVec2(hipCenter.x, hipCenter.y), ImVec2(lLeg2D.x, lLeg2D.y), IM_COL32(0, 0, 0, 255), outlineThickness);
            drawList->AddLine(ImVec2(hipCenter.x, hipCenter.y), ImVec2(lLeg2D.x, lLeg2D.y), skeletonColor, Options::ESP::SkeletonThickness);
            if (skOutline)
                drawList->AddLine(ImVec2(lLeg2D.x, lLeg2D.y), ImVec2(leftFoot.x, leftFoot.y), IM_COL32(0, 0, 0, 255), outlineThickness);
            drawList->AddLine(ImVec2(lLeg2D.x, lLeg2D.y), ImVec2(leftFoot.x, leftFoot.y), skeletonColor, Options::ESP::SkeletonThickness);

            if (skOutline)
                drawList->AddLine(ImVec2(hipCenter.x, hipCenter.y), ImVec2(rLeg2D.x, rLeg2D.y), IM_COL32(0, 0, 0, 255), outlineThickness);
            drawList->AddLine(ImVec2(hipCenter.x, hipCenter.y), ImVec2(rLeg2D.x, rLeg2D.y), skeletonColor, Options::ESP::SkeletonThickness);
            if (skOutline)
                drawList->AddLine(ImVec2(rLeg2D.x, rLeg2D.y), ImVec2(rightFoot.x, rightFoot.y), IM_COL32(0, 0, 0, 255), outlineThickness);
            drawList->AddLine(ImVec2(rLeg2D.x, rLeg2D.y), ImVec2(rightFoot.x, rightFoot.y), skeletonColor, Options::ESP::SkeletonThickness);

            if (skOutline)
                drawList->AddLine(ImVec2(torso2D.x, torso2D.y), ImVec2(hipCenter.x, hipCenter.y), IM_COL32(0, 0, 0, 255), outlineThickness);
            drawList->AddLine(ImVec2(torso2D.x, torso2D.y), ImVec2(hipCenter.x, hipCenter.y), skeletonColor, Options::ESP::SkeletonThickness);

            if (skOutline)
                drawList->AddLine(ImVec2(leftShoulder.x, leftShoulder.y), ImVec2(rightShoulder.x, rightShoulder.y), IM_COL32(0, 0, 0, 255), outlineThickness);
            drawList->AddLine(ImVec2(leftShoulder.x, leftShoulder.y), ImVec2(rightShoulder.x, rightShoulder.y), skeletonColor, Options::ESP::SkeletonThickness);
        }

        if (Options::ESP::HeadCircle && distance <= 150.0f)
        {
            ImColor headCircleColor = IM_COL32(
                static_cast<int>(Options::ESP::HeadCircleColor[0] * 255.f),
                static_cast<int>(Options::ESP::HeadCircleColor[1] * 255.f),
                static_cast<int>(Options::ESP::HeadCircleColor[2] * 255.f),
                180);

            float boxWidth = right - left;
            float maxRadius = (boxWidth * 0.4f < Options::ESP::HeadCircleMaxScale) ? boxWidth * 0.4f : Options::ESP::HeadCircleMaxScale;
            float baseRadius = 6.0f * scale;

            float distanceScale = std::clamp(1.5f - (distance / 150.0f) * 1.2f, 0.3f, 1.5f);
            float scaledRadius = baseRadius * distanceScale;

            float boxHeight = bottom - top;
            float maxCircleSize = boxHeight * 0.15f;
            float circleRadius = (scaledRadius < 3.0f) ? 3.0f : ((scaledRadius > maxCircleSize) ? maxCircleSize : scaledRadius);

            drawList->AddCircleFilled(ImVec2(head2D.x, head2D.y), circleRadius, headCircleColor);

            if (ESP_OutlineOn(Options::ESP::HeadCircleOutline))
                drawList->AddCircle(ImVec2(head2D.x, head2D.y), circleRadius, IM_COL32(0, 0, 0, 255), 0, Options::ESP::HeadCircleThickness + 1.5f);
            drawList->AddCircle(ImVec2(head2D.x, head2D.y), circleRadius, headCircleColor, 0, Options::ESP::HeadCircleThickness);
        }

        if (Options::ESP::Name)
        {
            const std::string& nameStr = player.Name;
            float fontSize = (12.f * scale > 10.0f) ? 12.f * scale : 10.0f;
            ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.f, nameStr.c_str());

            ImVec2 namePos(newHeadName.x - textSize.x / 2.0f, newHeadName.y);

            // Draw black outline
            if (ESP_OutlineOn(Options::ESP::NameOutline))
            {
                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        if (dx != 0 || dy != 0) {
                            drawList->AddText(font, fontSize, ImVec2(namePos.x + dx, namePos.y + dy), IM_COL32(0, 0, 0, 255), nameStr.c_str());
                        }
                    }
                }
            }

            // Draw main text
            drawList->AddText(font, fontSize, namePos, color, nameStr.c_str());
        }

        if (Options::ESP::Distance)
        {
            std::string distStr = std::to_string(roundedDistance) + " studs";
            float fontSize = (12.f * scale > 10.0f) ? 12.f * scale : 10.0f; // Same font size as username
            ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.f, distStr.c_str());

            ImVec2 distPos(head2D.x - textSize.x / 2.0f, newLeftLeg.y);

            // Draw black outline
            if (ESP_OutlineOn(Options::ESP::DistanceOutline))
            {
                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        if (dx != 0 || dy != 0) {
                            drawList->AddText(font, fontSize, ImVec2(distPos.x + dx, distPos.y + dy), IM_COL32(0, 0, 0, 255), distStr.c_str());
                        }
                    }
                }
            }

            // Draw main text
            drawList->AddText(font, fontSize, distPos, distanceColor, distStr.c_str());
        }

        // Weapon / held tool
        if (Options::ESP::Weapon && !player.ToolName.empty())
        {
            const std::string& toolStr = player.ToolName;
            float fontSize = (12.f * scale > 10.0f) ? 12.f * scale : 10.0f;
            ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.f, toolStr.c_str());

            // Below the distance line (offset a full text line down)
            float yPos = newLeftLeg.y + fontSize + 2.0f;
            ImVec2 toolPos(head2D.x - textSize.x / 2.0f, yPos);

            ImU32 weaponCol = IM_COL32(
                (int)(Options::ESP::WeaponColor[0] * 255.f),
                (int)(Options::ESP::WeaponColor[1] * 255.f),
                (int)(Options::ESP::WeaponColor[2] * 255.f),
                255);

            // Simple outline for weapon text (reuses NameOutline toggle)
            if (ESP_OutlineOn(Options::ESP::NameOutline))
            {
                for (int dx = -1; dx <= 1; dx++)
                    for (int dy = -1; dy <= 1; dy++)
                        if (dx != 0 || dy != 0)
                            drawList->AddText(font, fontSize, ImVec2(toolPos.x + dx, toolPos.y + dy), IM_COL32(0, 0, 0, 255), toolStr.c_str());
            }
            drawList->AddText(font, fontSize, toolPos, weaponCol, toolStr.c_str());
        }

        // Facing direction arrow: short line from head-2D toward projected look direction
        if (Options::ESP::FacingArrow && head2D.x != -1 && head2D.y != -1)
        {
            Vectors::Vector3 hrpLook = hrpCFrame.GetLookVector();
            // Project head3D + lookVec to screen to get 2D direction
            Vectors::Vector3 aheadWorld = head3D + hrpLook * 3.0f;
            auto ahead2D = WorldToScreen(aheadWorld);
            if (ahead2D.x != -1 && ahead2D.y != -1)
            {
                float dx = ahead2D.x - head2D.x;
                float dy = ahead2D.y - head2D.y;
                float mag = std::sqrt(dx * dx + dy * dy);
                if (mag > 0.001f)
                {
                    float len = std::clamp<float>(mag, 12.0f, 40.0f);
                    dx = dx / mag * len;
                    dy = dy / mag * len;
                    ImVec2 startP(head2D.x, head2D.y);
                    ImVec2 endP(head2D.x + dx, head2D.y + dy);

                    ImU32 faceCol = IM_COL32(
                        (int)(Options::ESP::FacingArrowColor[0] * 255.f),
                        (int)(Options::ESP::FacingArrowColor[1] * 255.f),
                        (int)(Options::ESP::FacingArrowColor[2] * 255.f),
                        255);

                    drawList->AddLine(startP, endP, IM_COL32(0, 0, 0, 220), 3.0f);
                    drawList->AddLine(startP, endP, faceCol, 1.6f);

                    // Small triangular tip
                    float nx = -dy / len;
                    float ny = dx / len;
                    ImVec2 tipBaseL(endP.x - dx * 0.25f + nx * 4.0f, endP.y - dy * 0.25f + ny * 4.0f);
                    ImVec2 tipBaseR(endP.x - dx * 0.25f - nx * 4.0f, endP.y - dy * 0.25f - ny * 4.0f);
                    drawList->AddTriangleFilled(endP, tipBaseL, tipBaseR, faceCol);
                }
            }
        }

        if (Options::ESP::Health)
        {
            float healthPercent = std::clamp(player.Health / player.MaxHealth, 0.f, 1.f);

            float barWidth = std::clamp(scale * 4.f, 2.f, 6.f);
            float boxHeight = bottom - top;

            ImVec2 barTopLeft(left - barWidth - 2.f, top);
            ImVec2 barBottomRight(left - 2.f, top + boxHeight);

            drawList->AddRectFilled(barTopLeft, barBottomRight, IM_COL32(50, 50, 50, 180));

            float filledHeight = boxHeight * healthPercent;
            ImVec2 filledTopLeft(barTopLeft.x, barBottomRight.y - filledHeight);
            ImVec2 filledBottomRight(barBottomRight.x, barBottomRight.y);

            drawList->AddRectFilled(filledTopLeft, filledBottomRight, IM_COL32(0, 255, 0, 220));

            drawList->AddRect(barTopLeft, barBottomRight, IM_COL32(0, 0, 0, 255));
        }

        if (Options::ESP::BoxType == 2) // 3D Box
        {
            ImColor esp3dColor = IM_COL32(
                static_cast<int>(Options::ESP::ESP3DColor[0] * 255.f),
                static_cast<int>(Options::ESP::ESP3DColor[1] * 255.f),
                static_cast<int>(Options::ESP::ESP3DColor[2] * 255.f),
                255);

            Vectors::Vector3 hrpPos = player.HumanoidRootPart.Position();
            sCFrame hrpCFrame = player.HumanoidRootPart.CFrame();

            Vectors::Vector3 rightVec = hrpCFrame.GetRightVector();
            Vectors::Vector3 upVec = hrpCFrame.GetUpVector();
            Vectors::Vector3 lookVec = hrpCFrame.GetLookVector();

            auto head = player.Head;
            auto leftLeg = (player.RigType == 0) ? player.Left_Leg : player.Left_Foot;

            Vectors::Vector3 headPos = head.Position();
            Vectors::Vector3 legPos = leftLeg.Position();

            Vectors::Vector3 constrainedHeadPos = headPos + Vectors::Vector3{ 0.f, 0.8f, 0.f };
            Vectors::Vector3 constrainedLegPos = legPos - Vectors::Vector3{ 0.f, 1.f, 0.f };

            Vectors::Vector3 headOffset = constrainedHeadPos - hrpPos;
            Vectors::Vector3 legOffset = constrainedLegPos - hrpPos;

            float width = 2.0f;
            float height = (headOffset.y - legOffset.y) - 2.0f;
            float depth = 2.0f;

            std::vector<Vectors::Vector3> corners3D = {
                hrpPos + rightVec * width + upVec * height + lookVec * depth,
                hrpPos - rightVec * width + upVec * height + lookVec * depth,
                hrpPos + rightVec * width - upVec * height + lookVec * depth,
                hrpPos - rightVec * width - upVec * height + lookVec * depth,
                hrpPos + rightVec * width + upVec * height - lookVec * depth,
                hrpPos - rightVec * width + upVec * height - lookVec * depth,
                hrpPos + rightVec * width - upVec * height - lookVec * depth,
                hrpPos - rightVec * width - upVec * height - lookVec * depth
            };

            std::vector<ImVec2> corners2D;
            for (const auto& corner : corners3D)
            {
                auto screenPos = WorldToScreen(corner);
                if (screenPos.x != -1 && screenPos.y != -1)
                {
                    corners2D.push_back(ImVec2(screenPos.x, screenPos.y));
                }
            }

            if (corners2D.size() >= 8)
            {
                drawList->AddLine(corners2D[0], corners2D[1], esp3dColor, Options::ESP::ESP3DThickness);
                drawList->AddLine(corners2D[1], corners2D[3], esp3dColor, Options::ESP::ESP3DThickness);
                drawList->AddLine(corners2D[3], corners2D[2], esp3dColor, Options::ESP::ESP3DThickness);
                drawList->AddLine(corners2D[2], corners2D[0], esp3dColor, Options::ESP::ESP3DThickness);

                drawList->AddLine(corners2D[4], corners2D[5], esp3dColor, Options::ESP::ESP3DThickness);
                drawList->AddLine(corners2D[5], corners2D[7], esp3dColor, Options::ESP::ESP3DThickness);
                drawList->AddLine(corners2D[7], corners2D[6], esp3dColor, Options::ESP::ESP3DThickness);
                drawList->AddLine(corners2D[6], corners2D[4], esp3dColor, Options::ESP::ESP3DThickness);

                drawList->AddLine(corners2D[0], corners2D[4], esp3dColor, Options::ESP::ESP3DThickness);
                drawList->AddLine(corners2D[1], corners2D[5], esp3dColor, Options::ESP::ESP3DThickness);
                drawList->AddLine(corners2D[2], corners2D[6], esp3dColor, Options::ESP::ESP3DThickness);
                drawList->AddLine(corners2D[3], corners2D[7], esp3dColor, Options::ESP::ESP3DThickness);
            }
        }
    }
}

