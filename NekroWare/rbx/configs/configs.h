#pragma once

#include "../configs/json.hpp"
#include "../globals/options.h"
#include "../globals/globals.h"
#include <fstream>  

using json = nlohmann::json;

inline json CreateConfig(std::string configName)
{
    json j;

    j["ESP"] =
    {
        { "Team Check", Options::ESP::TeamCheck },
        { "Box Type", Options::ESP::BoxType },
        { "Tracers", Options::ESP::Tracers },
        { "TracersStart", Options::ESP::TracersStart },
        { "Skeleton", Options::ESP::Skeleton },
        { "Name", Options::ESP::Name },
        { "Distance", Options::ESP::Distance },
        { "Health", Options::ESP::Health },
        { "Tracer Thickness", Options::ESP::TracerThickness },
        { "Head Circles", Options::ESP::HeadCircle },
        { "Remove Borders", Options::ESP::RemoveBorders },
        { "Headless", Options::ESP::Headless },
        { "Box Thickness", Options::ESP::BoxThickness },
        { "Skeleton Thickness", Options::ESP::SkeletonThickness },
        { "3D ESP Thickness", Options::ESP::ESP3DThickness },
        { "Head Circle Thickness", Options::ESP::HeadCircleThickness },
        { "Head Circle Max Scale", Options::ESP::HeadCircleMaxScale },
        { "Corner ESP", Options::ESP::CornerESP },
        { "Outline Enabled", Options::ESP::OutlineEnabled },
        { "Box Outline", Options::ESP::BoxOutline },
        { "Corner Outline", Options::ESP::CornerOutline },
        { "Tracer Outline", Options::ESP::TracerOutline },
        { "Skeleton Outline", Options::ESP::SkeletonOutline },
        { "Head Circle Outline", Options::ESP::HeadCircleOutline },
        { "Name Outline", Options::ESP::NameOutline },
        { "Distance Outline", Options::ESP::DistanceOutline },
        { "Weapon", Options::ESP::Weapon },
        { "Facing Arrow", Options::ESP::FacingArrow },
        { "Offscreen Arrows", Options::ESP::OffscreenArrows },

        { "Name Color", Options::ESP::Color },
        { "Box Color", Options::ESP::BoxColor },
        { "Corner Color", Options::ESP::CornerColor },
        { "Skeleton Color", Options::ESP::SkeletonColor },
        { "Distance Color", Options::ESP::DistanceColor },
        { "Tracers Color", Options::ESP::TracerColor },
        { "3D ESP Color", Options::ESP::ESP3DColor },
        { "Head Circles Color", Options::ESP::HeadCircleColor },
        { "Chams Color", Options::ESP::ChamsColor },
        { "Weapon Color", Options::ESP::WeaponColor },
        { "Facing Arrow Color", Options::ESP::FacingArrowColor },
        { "Offscreen Arrow Color", Options::ESP::OffscreenArrowColor }
    };

    j["Aimbot"] =
    {
        {"Aimbot Key", Options::Aimbot::AimbotKey},
        {"Aiming Type", Options::Aimbot::AimingType},
        {"Toggle Type", Options::Aimbot::ToggleType},
        {"Aimbot", Options::Aimbot::Aimbot},
        {"Team Check", Options::Aimbot::TeamCheck},
        {"Downed Check", Options::Aimbot::DownedCheck},
        {"Sticky Aim", Options::Aimbot::StickyAim},
        {"Target Bone", Options::Aimbot::TargetBone},
        {"Air Target Bone", Options::Aimbot::AirTargetBone},
        {"FOV", Options::Aimbot::FOV},
        {"Show FOV", Options::Aimbot::ShowFOV},
        {"Show FOV Fill", Options::Aimbot::ShowFOVFill},
        {"FOV Color", Options::Aimbot::FOVColor},
        {"FOV Fill Color", Options::Aimbot::FOVFillColor},
        {"FOV Thickness", Options::Aimbot::FOVThickness},
        {"Smoothness", Options::Aimbot::Smoothness},
        {"Smoothness Curve", Options::Aimbot::SmoothnessCurve},
        {"Range", Options::Aimbot::Range},
        {"Prediction", Options::Aimbot::Prediction},
        {"Prediction X", Options::Aimbot::PredictionX},
        {"Prediction Y", Options::Aimbot::PredictionY},
        {"Shake", Options::Aimbot::Shake},
        {"Shake Intensity", Options::Aimbot::ShakeIntensity},
        {"Stutter", Options::Aimbot::Stutter},
        {"Stutter Ticks", Options::Aimbot::StutterTicks}
    };

    j["Triggerbot"] =
    {
        {"Triggerbot Key", Options::Triggerbot::TriggerbotKey},
        {"Toggle Type", Options::Triggerbot::ToggleType},
        {"Enabled", Options::Triggerbot::Enabled},
        {"Team Check", Options::Triggerbot::TeamCheck},
        {"Downed Check", Options::Triggerbot::DownedCheck},
        {"Radius", Options::Triggerbot::Radius},
        {"Range", Options::Triggerbot::Range},
        {"Delay", Options::Triggerbot::Delay}
    };

    j["Macro"] =
    {
        {"Macro Key", Options::Macro::MacroKey},
        {"Toggle Type", Options::Macro::ToggleType},
        {"Enabled", Options::Macro::Enabled},
        {"Delay", Options::Macro::Delay}
    };

    j["Crosshair"] =
    {
        {"Enabled", Options::Crosshair::Enabled},
        {"Style", Options::Crosshair::Style},
        {"Size", Options::Crosshair::Size},
        {"Gap", Options::Crosshair::Gap},
        {"Thickness", Options::Crosshair::Thickness},
        {"Spin Speed", Options::Crosshair::SpinSpeed},
        {"Gap Speed", Options::Crosshair::GapSpeed},
        {"Gap Tween", Options::Crosshair::GapTween},
        {"Show Text", Options::Crosshair::ShowText},
        {"Color", Options::Crosshair::Color}
    };

    j["Misc"] =
    {
        {"Bypass", Options::Misc::Bypass},
        {"FOV", Options::Misc::FOV},
        {"Cache NPCs", Options::Misc::CacheNPCs},
        {"Keybind List", Options::Misc::KeybindList},
        {"Keybind List X", Options::Misc::KeybindListX},
        {"Keybind List Y", Options::Misc::KeybindListY},
        {"Stream Proof", Options::Misc::StreamProof},
        {"Menu Accent Color", Options::Misc::MenuAccentColor}
    };

    j["HitboxExpander"] =
    {
        {"Enabled", Options::HitboxExpander::Enabled},
        {"Horizontal Size", Options::HitboxExpander::HorizontalSize},
        {"Vertical Size", Options::HitboxExpander::VerticalSize},
        {"Show Hitbox", Options::HitboxExpander::ShowHitbox},
        {"Transparency", Options::HitboxExpander::HitboxTransparency},
        {"Walk Through", Options::HitboxExpander::WalkThrough}
    };

    j["Fly"] =
    {
        {"Fly Key", Options::Fly::FlyKey},
        {"Toggle Type", Options::Fly::ToggleType},
        {"Enabled", Options::Fly::Enabled},
        {"Speed", Options::Fly::Speed}
    };

    j["WalkSpeed"] =
    {
        {"WalkSpeed Key", Options::WalkSpeed::WalkSpeedKey},
        {"Toggle Type", Options::WalkSpeed::ToggleType},
        {"Enabled", Options::WalkSpeed::Enabled},
        {"Speed", Options::WalkSpeed::Speed}
    };

    j["Radar"] =
    {
        {"Enabled", Options::Radar::Enabled},
        {"Size", Options::Radar::Size},
        {"Range", Options::Radar::Range},
        {"Position X", Options::Radar::PositionX},
        {"Position Y", Options::Radar::PositionY},
        {"Opacity", Options::Radar::Opacity},
        {"Dot Size", Options::Radar::DotSize},
        {"Rotate With Camera", Options::Radar::RotateWithCamera}
    };

    j["SilentAim"] =
    {
        {"Key", Options::SilentAim::Key},
        {"Toggle Type", Options::SilentAim::ToggleType},
        {"Enabled", Options::SilentAim::Enabled},
        {"Team Check", Options::SilentAim::TeamCheck},
        {"Downed Check", Options::SilentAim::DownedCheck},
        {"FOV", Options::SilentAim::FOV},
        {"Range", Options::SilentAim::Range},
        {"Target Bone", Options::SilentAim::TargetBone},
        {"Prediction", Options::SilentAim::Prediction},
        {"Prediction X", Options::SilentAim::PredictionX},
        {"Prediction Y", Options::SilentAim::PredictionY},
        {"Method", Options::SilentAim::Method},
        {"Hitbox On Fire", Options::SilentAim::HitboxOnFire},
        {"Hitbox Mult", Options::SilentAim::HitboxMult},
        {"Hitbox Frames", Options::SilentAim::HitboxFrames}
    };

    std::ofstream out(Globals::configsPath + "\\" + configName);
    out << j.dump(4);
    out.close();

    return j;
}

inline void LoadConfig(std::string configName)
{
    std::ifstream f(Globals::configsPath + "\\" + configName);
    json data = json::parse(f);

    //ESP Loading

    Options::ESP::TeamCheck = data["ESP"]["Team Check"];
    
    if (data["ESP"].contains("Box Type"))
        Options::ESP::BoxType = data["ESP"]["Box Type"];
    else if (data["ESP"].contains("Box"))
    {
        // Backward compatibility: convert old Box boolean to BoxType
        bool oldBox = data["ESP"]["Box"];
        Options::ESP::BoxType = oldBox ? 1 : 0;
    }
    
    Options::ESP::Tracers = data["ESP"]["Tracers"];
    Options::ESP::TracersStart = data["ESP"]["TracersStart"];
    Options::ESP::Skeleton = data["ESP"]["Skeleton"];
    Options::ESP::Name = data["ESP"]["Name"];
    Options::ESP::Distance = data["ESP"]["Distance"];
    Options::ESP::Health = data["ESP"]["Health"];
    Options::ESP::TracerThickness = data["ESP"]["Tracer Thickness"];
    Options::ESP::HeadCircle = data["ESP"]["Head Circles"];
    Options::ESP::RemoveBorders = data["ESP"]["Remove Borders"];
    
    if (data["ESP"].contains("Headless"))
        Options::ESP::Headless = data["ESP"]["Headless"];

    Options::ESP::Color[0] = data["ESP"]["Name Color"][0];
    Options::ESP::Color[1] = data["ESP"]["Name Color"][1];
    Options::ESP::Color[2] = data["ESP"]["Name Color"][2];

    Options::ESP::BoxColor[0] = data["ESP"]["Box Color"][0];
    Options::ESP::BoxColor[1] = data["ESP"]["Box Color"][1];
    Options::ESP::BoxColor[2] = data["ESP"]["Box Color"][2];

    Options::ESP::DistanceColor[0] = data["ESP"]["Distance Color"][0];
    Options::ESP::DistanceColor[1] = data["ESP"]["Distance Color"][1];
    Options::ESP::DistanceColor[2] = data["ESP"]["Distance Color"][2];

    Options::ESP::TracerColor[0] = data["ESP"]["Tracers Color"][0];
    Options::ESP::TracerColor[1] = data["ESP"]["Tracers Color"][1];
    Options::ESP::TracerColor[2] = data["ESP"]["Tracers Color"][2];

    Options::ESP::ESP3DColor[0] = data["ESP"]["3D ESP Color"][0];
    Options::ESP::ESP3DColor[1] = data["ESP"]["3D ESP Color"][1];
    Options::ESP::ESP3DColor[2] = data["ESP"]["3D ESP Color"][2];

    Options::ESP::HeadCircleColor[0] = data["ESP"]["Head Circles Color"][0];
    Options::ESP::HeadCircleColor[1] = data["ESP"]["Head Circles Color"][1];
    Options::ESP::HeadCircleColor[2] = data["ESP"]["Head Circles Color"][2];

    if (data["ESP"].contains("Chams Color"))
    {
        Options::ESP::ChamsColor[0] = data["ESP"]["Chams Color"][0];
        Options::ESP::ChamsColor[1] = data["ESP"]["Chams Color"][1];
        Options::ESP::ChamsColor[2] = data["ESP"]["Chams Color"][2];
    }

    // New ESP thickness + outline fields (all guarded for backward compat)
    if (data["ESP"].contains("Box Thickness")) Options::ESP::BoxThickness = data["ESP"]["Box Thickness"];
    if (data["ESP"].contains("Skeleton Thickness")) Options::ESP::SkeletonThickness = data["ESP"]["Skeleton Thickness"];
    if (data["ESP"].contains("3D ESP Thickness")) Options::ESP::ESP3DThickness = data["ESP"]["3D ESP Thickness"];
    if (data["ESP"].contains("Head Circle Thickness")) Options::ESP::HeadCircleThickness = data["ESP"]["Head Circle Thickness"];
    if (data["ESP"].contains("Head Circle Max Scale")) Options::ESP::HeadCircleMaxScale = data["ESP"]["Head Circle Max Scale"];
    if (data["ESP"].contains("Corner ESP")) Options::ESP::CornerESP = data["ESP"]["Corner ESP"];
    if (data["ESP"].contains("Outline Enabled")) Options::ESP::OutlineEnabled = data["ESP"]["Outline Enabled"];
    if (data["ESP"].contains("Box Outline")) Options::ESP::BoxOutline = data["ESP"]["Box Outline"];
    if (data["ESP"].contains("Corner Outline")) Options::ESP::CornerOutline = data["ESP"]["Corner Outline"];
    if (data["ESP"].contains("Tracer Outline")) Options::ESP::TracerOutline = data["ESP"]["Tracer Outline"];
    if (data["ESP"].contains("Skeleton Outline")) Options::ESP::SkeletonOutline = data["ESP"]["Skeleton Outline"];
    if (data["ESP"].contains("Head Circle Outline")) Options::ESP::HeadCircleOutline = data["ESP"]["Head Circle Outline"];
    if (data["ESP"].contains("Name Outline")) Options::ESP::NameOutline = data["ESP"]["Name Outline"];
    if (data["ESP"].contains("Distance Outline")) Options::ESP::DistanceOutline = data["ESP"]["Distance Outline"];
    if (data["ESP"].contains("Weapon")) Options::ESP::Weapon = data["ESP"]["Weapon"];
    if (data["ESP"].contains("Facing Arrow")) Options::ESP::FacingArrow = data["ESP"]["Facing Arrow"];
    if (data["ESP"].contains("Offscreen Arrows")) Options::ESP::OffscreenArrows = data["ESP"]["Offscreen Arrows"];
    if (data["ESP"].contains("Corner Color"))
    {
        Options::ESP::CornerColor[0] = data["ESP"]["Corner Color"][0];
        Options::ESP::CornerColor[1] = data["ESP"]["Corner Color"][1];
        Options::ESP::CornerColor[2] = data["ESP"]["Corner Color"][2];
    }
    if (data["ESP"].contains("Weapon Color"))
    {
        Options::ESP::WeaponColor[0] = data["ESP"]["Weapon Color"][0];
        Options::ESP::WeaponColor[1] = data["ESP"]["Weapon Color"][1];
        Options::ESP::WeaponColor[2] = data["ESP"]["Weapon Color"][2];
    }
    if (data["ESP"].contains("Facing Arrow Color"))
    {
        Options::ESP::FacingArrowColor[0] = data["ESP"]["Facing Arrow Color"][0];
        Options::ESP::FacingArrowColor[1] = data["ESP"]["Facing Arrow Color"][1];
        Options::ESP::FacingArrowColor[2] = data["ESP"]["Facing Arrow Color"][2];
    }
    if (data["ESP"].contains("Offscreen Arrow Color"))
    {
        Options::ESP::OffscreenArrowColor[0] = data["ESP"]["Offscreen Arrow Color"][0];
        Options::ESP::OffscreenArrowColor[1] = data["ESP"]["Offscreen Arrow Color"][1];
        Options::ESP::OffscreenArrowColor[2] = data["ESP"]["Offscreen Arrow Color"][2];
    }

    // Aimbot Loading

    Options::Aimbot::AimbotKey = data["Aimbot"]["Aimbot Key"];
    Options::Aimbot::AimingType = data["Aimbot"]["Aiming Type"];
    
    if (data["Aimbot"].contains("Toggle Type"))
        Options::Aimbot::ToggleType = data["Aimbot"]["Toggle Type"];
    
    Options::Aimbot::Aimbot = data["Aimbot"]["Aimbot"];
    Options::Aimbot::TeamCheck = data["Aimbot"]["Team Check"];
    Options::Aimbot::DownedCheck = data["Aimbot"]["Downed Check"];
    Options::Aimbot::StickyAim = data["Aimbot"]["Sticky Aim"];
    Options::Aimbot::TargetBone = data["Aimbot"]["Target Bone"];
    
    if (data["Aimbot"].contains("Air Target Bone"))
        Options::Aimbot::AirTargetBone = data["Aimbot"]["Air Target Bone"];
    
    Options::Aimbot::FOV = data["Aimbot"]["FOV"];
    Options::Aimbot::ShowFOV = data["Aimbot"]["Show FOV"];
    
    if (data["Aimbot"].contains("Show FOV Fill"))
        Options::Aimbot::ShowFOVFill = data["Aimbot"]["Show FOV Fill"];

    Options::Aimbot::FOVColor[0] = data["Aimbot"]["FOV Color"][0];
    Options::Aimbot::FOVColor[1] = data["Aimbot"]["FOV Color"][1];
    Options::Aimbot::FOVColor[2] = data["Aimbot"]["FOV Color"][2];

    Options::Aimbot::FOVFillColor[0] = data["Aimbot"]["FOV Fill Color"][0];
    Options::Aimbot::FOVFillColor[1] = data["Aimbot"]["FOV Fill Color"][1];
    Options::Aimbot::FOVFillColor[2] = data["Aimbot"]["FOV Fill Color"][2];

    if (data["Aimbot"].contains("FOV Thickness"))
        Options::Aimbot::FOVThickness = data["Aimbot"]["FOV Thickness"];

    Options::Aimbot::Smoothness = data["Aimbot"]["Smoothness"];
    
    if (data["Aimbot"].contains("Smoothness Curve"))
        Options::Aimbot::SmoothnessCurve = data["Aimbot"]["Smoothness Curve"];
    
    Options::Aimbot::Range = data["Aimbot"]["Range"];
    
    if (data["Aimbot"].contains("Prediction"))
        Options::Aimbot::Prediction = data["Aimbot"]["Prediction"];
    
    if (data["Aimbot"].contains("Prediction X"))
        Options::Aimbot::PredictionX = data["Aimbot"]["Prediction X"];
    
    if (data["Aimbot"].contains("Prediction Y"))
        Options::Aimbot::PredictionY = data["Aimbot"]["Prediction Y"];
    
    if (data["Aimbot"].contains("Shake"))
        Options::Aimbot::Shake = data["Aimbot"]["Shake"];
    
    if (data["Aimbot"].contains("Shake Intensity"))
        Options::Aimbot::ShakeIntensity = data["Aimbot"]["Shake Intensity"];
    
    if (data["Aimbot"].contains("Stutter"))
        Options::Aimbot::Stutter = data["Aimbot"]["Stutter"];
    
    if (data["Aimbot"].contains("Stutter Ticks"))
        Options::Aimbot::StutterTicks = data["Aimbot"]["Stutter Ticks"];

    // Triggerbot Loading
    if (data.contains("Triggerbot"))
    {
        if (data["Triggerbot"].contains("Triggerbot Key"))
            Options::Triggerbot::TriggerbotKey = data["Triggerbot"]["Triggerbot Key"];
        
        if (data["Triggerbot"].contains("Toggle Type"))
            Options::Triggerbot::ToggleType = data["Triggerbot"]["Toggle Type"];
        
        if (data["Triggerbot"].contains("Enabled"))
            Options::Triggerbot::Enabled = data["Triggerbot"]["Enabled"];
        
        if (data["Triggerbot"].contains("Team Check"))
            Options::Triggerbot::TeamCheck = data["Triggerbot"]["Team Check"];
        
        if (data["Triggerbot"].contains("Downed Check"))
            Options::Triggerbot::DownedCheck = data["Triggerbot"]["Downed Check"];
        
        if (data["Triggerbot"].contains("Radius"))
            Options::Triggerbot::Radius = data["Triggerbot"]["Radius"];
        
        if (data["Triggerbot"].contains("Range"))
            Options::Triggerbot::Range = data["Triggerbot"]["Range"];
        
        if (data["Triggerbot"].contains("Delay"))
            Options::Triggerbot::Delay = data["Triggerbot"]["Delay"];
    }

    // Macro Loading
    if (data.contains("Macro"))
    {
        if (data["Macro"].contains("Macro Key"))
            Options::Macro::MacroKey = data["Macro"]["Macro Key"];
        
        if (data["Macro"].contains("Toggle Type"))
            Options::Macro::ToggleType = data["Macro"]["Toggle Type"];
        
        if (data["Macro"].contains("Enabled"))
            Options::Macro::Enabled = data["Macro"]["Enabled"];
        
        if (data["Macro"].contains("Delay"))
            Options::Macro::Delay = data["Macro"]["Delay"];
    }

    // Crosshair Loading
    if (data.contains("Crosshair"))
    {
        if (data["Crosshair"].contains("Enabled"))
            Options::Crosshair::Enabled = data["Crosshair"]["Enabled"];
        
        if (data["Crosshair"].contains("Style"))
            Options::Crosshair::Style = data["Crosshair"]["Style"];
        
        if (data["Crosshair"].contains("Size"))
            Options::Crosshair::Size = data["Crosshair"]["Size"];
        
        if (data["Crosshair"].contains("Gap"))
            Options::Crosshair::Gap = data["Crosshair"]["Gap"];
        
        if (data["Crosshair"].contains("Thickness"))
            Options::Crosshair::Thickness = data["Crosshair"]["Thickness"];
        
        if (data["Crosshair"].contains("Spin Speed"))
            Options::Crosshair::SpinSpeed = data["Crosshair"]["Spin Speed"];
        
        if (data["Crosshair"].contains("Gap Speed"))
            Options::Crosshair::GapSpeed = data["Crosshair"]["Gap Speed"];
        
        if (data["Crosshair"].contains("Gap Tween"))
            Options::Crosshair::GapTween = data["Crosshair"]["Gap Tween"];
        
        if (data["Crosshair"].contains("Show Text"))
            Options::Crosshair::ShowText = data["Crosshair"]["Show Text"];
        
        if (data["Crosshair"].contains("Color"))
        {
            Options::Crosshair::Color[0] = data["Crosshair"]["Color"][0];
            Options::Crosshair::Color[1] = data["Crosshair"]["Color"][1];
            Options::Crosshair::Color[2] = data["Crosshair"]["Color"][2];
            Options::Crosshair::Color[3] = data["Crosshair"]["Color"][3];
        }
    }

    // Misc Loading

    if (data["Misc"].contains("Bypass"))
        Options::Misc::Bypass = data["Misc"]["Bypass"];
    
    Options::Misc::FOV = data["Misc"]["FOV"];
    
    if (data["Misc"].contains("Cache NPCs"))
        Options::Misc::CacheNPCs = data["Misc"]["Cache NPCs"];
    
    if (data["Misc"].contains("Keybind List"))
        Options::Misc::KeybindList = data["Misc"]["Keybind List"];
    
    if (data["Misc"].contains("Keybind List X"))
        Options::Misc::KeybindListX = data["Misc"]["Keybind List X"];
    
    if (data["Misc"].contains("Keybind List Y"))
        Options::Misc::KeybindListY = data["Misc"]["Keybind List Y"];
    
    if (data["Misc"].contains("Stream Proof"))
        Options::Misc::StreamProof = data["Misc"]["Stream Proof"];
    
    if (data["Misc"].contains("Menu Accent Color"))
    {
        Options::Misc::MenuAccentColor[0] = data["Misc"]["Menu Accent Color"][0];
        Options::Misc::MenuAccentColor[1] = data["Misc"]["Menu Accent Color"][1];
        Options::Misc::MenuAccentColor[2] = data["Misc"]["Menu Accent Color"][2];
    }

    // Hitbox Expander Loading
    if (data.contains("HitboxExpander"))
    {
        if (data["HitboxExpander"].contains("Enabled"))
            Options::HitboxExpander::Enabled = data["HitboxExpander"]["Enabled"];
        
        if (data["HitboxExpander"].contains("Horizontal Size"))
            Options::HitboxExpander::HorizontalSize = data["HitboxExpander"]["Horizontal Size"];
        
        if (data["HitboxExpander"].contains("Vertical Size"))
            Options::HitboxExpander::VerticalSize = data["HitboxExpander"]["Vertical Size"];
        
        if (data["HitboxExpander"].contains("Show Hitbox"))
            Options::HitboxExpander::ShowHitbox = data["HitboxExpander"]["Show Hitbox"];
        
        if (data["HitboxExpander"].contains("Transparency"))
            Options::HitboxExpander::HitboxTransparency = data["HitboxExpander"]["Transparency"];
        
        if (data["HitboxExpander"].contains("Walk Through"))
            Options::HitboxExpander::WalkThrough = data["HitboxExpander"]["Walk Through"];
    }

    // Fly Loading
    if (data.contains("Fly"))
    {
        if (data["Fly"].contains("Fly Key"))
            Options::Fly::FlyKey = data["Fly"]["Fly Key"];
        
        if (data["Fly"].contains("Toggle Type"))
            Options::Fly::ToggleType = data["Fly"]["Toggle Type"];
        
        if (data["Fly"].contains("Enabled"))
            Options::Fly::Enabled = data["Fly"]["Enabled"];
        
        if (data["Fly"].contains("Speed"))
            Options::Fly::Speed = data["Fly"]["Speed"];
    }

    // WalkSpeed Loading
    if (data.contains("WalkSpeed"))
    {
        if (data["WalkSpeed"].contains("WalkSpeed Key"))
            Options::WalkSpeed::WalkSpeedKey = data["WalkSpeed"]["WalkSpeed Key"];

        if (data["WalkSpeed"].contains("Toggle Type"))
            Options::WalkSpeed::ToggleType = data["WalkSpeed"]["Toggle Type"];

        if (data["WalkSpeed"].contains("Enabled"))
            Options::WalkSpeed::Enabled = data["WalkSpeed"]["Enabled"];

        if (data["WalkSpeed"].contains("Speed"))
            Options::WalkSpeed::Speed = data["WalkSpeed"]["Speed"];
    }

    // Radar Loading
    if (data.contains("Radar"))
    {
        if (data["Radar"].contains("Enabled")) Options::Radar::Enabled = data["Radar"]["Enabled"];
        if (data["Radar"].contains("Size")) Options::Radar::Size = data["Radar"]["Size"];
        if (data["Radar"].contains("Range")) Options::Radar::Range = data["Radar"]["Range"];
        if (data["Radar"].contains("Position X")) Options::Radar::PositionX = data["Radar"]["Position X"];
        if (data["Radar"].contains("Position Y")) Options::Radar::PositionY = data["Radar"]["Position Y"];
        if (data["Radar"].contains("Opacity")) Options::Radar::Opacity = data["Radar"]["Opacity"];
        if (data["Radar"].contains("Dot Size")) Options::Radar::DotSize = data["Radar"]["Dot Size"];
        if (data["Radar"].contains("Rotate With Camera")) Options::Radar::RotateWithCamera = data["Radar"]["Rotate With Camera"];
    }

    // SilentAim Loading
    if (data.contains("SilentAim"))
    {
        if (data["SilentAim"].contains("Key")) Options::SilentAim::Key = data["SilentAim"]["Key"];
        if (data["SilentAim"].contains("Toggle Type")) Options::SilentAim::ToggleType = data["SilentAim"]["Toggle Type"];
        if (data["SilentAim"].contains("Enabled")) Options::SilentAim::Enabled = data["SilentAim"]["Enabled"];
        if (data["SilentAim"].contains("Team Check")) Options::SilentAim::TeamCheck = data["SilentAim"]["Team Check"];
        if (data["SilentAim"].contains("Downed Check")) Options::SilentAim::DownedCheck = data["SilentAim"]["Downed Check"];
        if (data["SilentAim"].contains("FOV")) Options::SilentAim::FOV = data["SilentAim"]["FOV"];
        if (data["SilentAim"].contains("Range")) Options::SilentAim::Range = data["SilentAim"]["Range"];
        if (data["SilentAim"].contains("Target Bone")) Options::SilentAim::TargetBone = data["SilentAim"]["Target Bone"];
        if (data["SilentAim"].contains("Prediction")) Options::SilentAim::Prediction = data["SilentAim"]["Prediction"];
        if (data["SilentAim"].contains("Prediction X")) Options::SilentAim::PredictionX = data["SilentAim"]["Prediction X"];
        if (data["SilentAim"].contains("Prediction Y")) Options::SilentAim::PredictionY = data["SilentAim"]["Prediction Y"];
        if (data["SilentAim"].contains("Method")) Options::SilentAim::Method = data["SilentAim"]["Method"];
        if (data["SilentAim"].contains("Hitbox On Fire")) Options::SilentAim::HitboxOnFire = data["SilentAim"]["Hitbox On Fire"];
        if (data["SilentAim"].contains("Hitbox Mult")) Options::SilentAim::HitboxMult = data["SilentAim"]["Hitbox Mult"];
        if (data["SilentAim"].contains("Hitbox Frames")) Options::SilentAim::HitboxFrames = data["SilentAim"]["Hitbox Frames"];
    }
}