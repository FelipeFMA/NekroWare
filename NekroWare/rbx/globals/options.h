#pragma once
#include "globals.h"
#include <cstring>

namespace Options
{
	namespace Misc
	{
		inline bool Bypass = false;
		inline bool FOVEnabled = false;
		inline float FOV = 70.f;
		inline bool CacheNPCs = true;
		inline bool KeybindList = false;
		inline float KeybindListX = 20.0f;
		inline float KeybindListY = 80.0f;
		inline bool StreamProof = false;
		inline float MenuAccentColor[3] = {1.0f, 1.0f, 1.0f}; // Monochrome OLED white accent (255, 255, 255)

		// UI Features
		inline bool NotificationsEnabled = true;
		inline int NotificationMaxQueue = 10;
		inline float NotificationAnimationSpeed = 1.0f;
		inline int NotificationDuration = 3000; // milliseconds
		inline int CurrentTheme = 0; // 0 = Dark (OLED), 1 = Light, 2-5 = grayscale variants (unused - ApplyTheme is a stub)
		inline bool AnimationsEnabled = true;
		inline float AnimationSpeed = 1.0f;
		inline bool FPSCounterEnabled = true;
		inline float FPSCounterX = 10.0f;
		inline float FPSCounterY = 10.0f;
		inline bool ShowPerformanceMetrics = false;
	}
	namespace Teams
	{
		// 0 = Team (Teams service pointer), 1 = Clothing (shirt/pants template match)
		inline int Mode = 0;
	}
	namespace HitboxExpander
	{
		inline bool Enabled = false;
		inline float HorizontalSize = 10.0f;
		inline float VerticalSize = 10.0f;
		inline bool ShowHitbox = false;
		inline float HitboxTransparency = 0.5f;
		inline bool WalkThrough = false;
	}
	namespace ESP
	{
		inline bool TeamCheck = false;
		inline int BoxType = 0; // 0 = None, 1 = Normal Box, 2 = 3D Box
		inline bool Tracers = false;
		inline int TracersStart = 0;
		inline bool Skeleton = false;
		inline bool Name = false;
		inline bool Distance = false;
		inline bool Health = false;
		inline bool HeadCircle = false;
		inline bool CornerESP = false;
		inline bool Headless = false;

		inline float Color[3] = {1.0f, 1.0f, 1.0f};
		inline float BoxColor[3] = {1.0f, 1.0f, 1.0f};
		inline float CornerColor[3] = {1.0f, 1.0f, 1.0f};
		inline float SkeletonColor[3] = {1.0f, 1.0f, 1.0f};
		inline float DistanceColor[3] = {1.0f, 1.0f, 1.0f};
		inline float TracerColor[3] = {1.0f, 1.0f, 1.0f};
		inline float TracerThickness = 1.0f;
		inline float BoxThickness = 1.0f;
		inline float SkeletonThickness = 1.0f;
		inline float ESP3DThickness = 1.0f;
		inline bool RemoveBorders = false;
		inline bool OutlineEnabled = true;
		inline float ESP3DColor[3] = {1.0f, 1.0f, 1.0f};
		inline float HeadCircleColor[3] = {1.0f, 1.0f, 1.0f};
		inline float HeadCircleThickness = 1.0f;
		inline float HeadCircleMaxScale = 2.5f;
		inline float ChamsColor[3] = {1.0f, 1.0f, 1.0f}; // White color

		// Per-visual outline toggles (AND'd with OutlineEnabled)
		inline bool BoxOutline = true;
		inline bool CornerOutline = true;
		inline bool TracerOutline = true;
		inline bool SkeletonOutline = true;
		inline bool HeadCircleOutline = true;
		inline bool NameOutline = true;
		inline bool DistanceOutline = true;

		// New ESP features
		inline bool Weapon = false;
		inline float WeaponColor[3] = {1.0f, 1.0f, 1.0f};
		inline bool FacingArrow = false;
		inline float FacingArrowColor[3] = {1.0f, 1.0f, 1.0f};
		inline bool OffscreenArrows = false;
		inline float OffscreenArrowColor[3] = {1.0f, 1.0f, 1.0f};
	}
	namespace Aimbot
	{
		inline int AimbotKey = 0;
		inline int AimingType = 0;

		inline int ToggleType = 0;

		inline bool Aimbot = false;
		inline bool TeamCheck = false;
		inline bool VisibilityCheck = false;
		inline bool DebugRays = false;
		inline float Sensitivity = 1.f; // manual fallback when MouseService::SensitivityPointer is missing
		inline bool DownedCheck = false;
		inline bool StickyAim = false;
		inline float FOV = 100.f;
		inline float Smoothness = 0.f;
		inline int SmoothnessCurve = 0; // 0=Linear, 1=Ease In, 2=Ease Out, 3=Ease In-Out, 4=Custom
		inline bool CustomCurveEnabled = false;
		inline float CustomCurveP1[2] = {0.25f, 0.25f}; // First control point
		inline float CustomCurveP2[2] = {0.75f, 0.75f}; // Second control point
		inline bool ShowFOV = false;
		inline bool ShowFOVFill = false;
		inline float Range = 100.f;

		inline float FOVColor[3] = {1.0f, 1.0f, 1.0f}; // White color
		inline float FOVFillColor[4] = {1.0f, 1.0f, 1.0f, 0.1f}; // White with transparency
		inline float FOVThickness = 1.0f;
		
		inline int TargetBone = 0;
		inline int AirTargetBone = 0; // Air part selection
		
		inline bool Prediction = false;
		inline float PredictionX = 1.0f;
		inline float PredictionY = 1.0f;
		
		inline bool Shake = false;
		inline float ShakeIntensity = 1.0f;
		
		inline bool Stutter = false;
		inline int StutterTicks = 5;
		
		inline bool NearestAim = false;
		inline bool NearestHead = true;
		inline bool NearestChest = true;
		inline bool NearestLegs = true;

		inline RobloxPlayer CurrentTarget;
		inline bool Toggled = false;
	}
	namespace Triggerbot
	{
		inline int TriggerbotKey = 0;
		inline int ToggleType = 0;

		inline bool Enabled = false;
		inline bool TeamCheck = false;
		inline bool DownedCheck = false;
		inline float Radius = 15.f;
		inline float Range = 100.f;
		inline int Delay = 50;
		
		inline bool AdvancedFOV = false;
		inline bool ShowAdvancedFOV = false;
		
		// Advanced FOV per body part (X = horizontal, Y = vertical)
		inline float HeadFOV_X = 0.0f;
		inline float HeadFOV_Y = 0.0f;
		inline float TorsoFOV_X = 0.0f;
		inline float TorsoFOV_Y = 0.0f;
		inline float UpperTorsoFOV_X = 0.0f;
		inline float UpperTorsoFOV_Y = 0.0f;
		inline float LowerTorsoFOV_X = 0.0f;
		inline float LowerTorsoFOV_Y = 0.0f;
		inline float LeftUpperArmFOV_X = 0.0f;
		inline float LeftUpperArmFOV_Y = 0.0f;
		inline float LeftLowerArmFOV_X = 0.0f;
		inline float LeftLowerArmFOV_Y = 0.0f;
		inline float LeftHandFOV_X = 0.0f;
		inline float LeftHandFOV_Y = 0.0f;
		inline float RightUpperArmFOV_X = 0.0f;
		inline float RightUpperArmFOV_Y = 0.0f;
		inline float RightLowerArmFOV_X = 0.0f;
		inline float RightLowerArmFOV_Y = 0.0f;
		inline float RightHandFOV_X = 0.0f;
		inline float RightHandFOV_Y = 0.0f;
		inline float LeftUpperLegFOV_X = 0.0f;
		inline float LeftUpperLegFOV_Y = 0.0f;
		inline float LeftLowerLegFOV_X = 0.0f;
		inline float LeftLowerLegFOV_Y = 0.0f;
		inline float LeftFootFOV_X = 0.0f;
		inline float LeftFootFOV_Y = 0.0f;
		inline float RightUpperLegFOV_X = 0.0f;
		inline float RightUpperLegFOV_Y = 0.0f;
		inline float RightLowerLegFOV_X = 0.0f;
		inline float RightLowerLegFOV_Y = 0.0f;
		inline float RightFootFOV_X = 0.0f;
		inline float RightFootFOV_Y = 0.0f;
		
		inline bool Toggled = false;
	}
	namespace Macro
	{
		inline int MacroKey = 0;
		inline int ToggleType = 0;

		inline bool Enabled = false;
		inline int Delay = 100;
		
		inline bool Toggled = false;
	}
	namespace Crosshair
	{
		inline bool Enabled = false;
		inline int Style = 0; // 0 = Static, 1 = Pulse
		inline float Size = 10.0f;
		inline float Gap = 5.0f;
		inline float Thickness = 2.0f;
		inline float SpinSpeed = 50.0f;
		inline float GapSpeed = 1.0f;
		inline bool GapTween = false;
		inline bool ShowText = true;
		inline float Color[4] = {1.0f, 1.0f, 1.0f, 1.0f}; // White with full alpha
	}
	namespace Fly
	{
		inline int FlyKey = 0;
		inline int ToggleType = 0;
		
		inline bool Enabled = false;
		inline float Speed = 50.0f;
		
		inline bool Toggled = false;
	}
	namespace WalkSpeed
	{
		inline int WalkSpeedKey = 0;
		inline int ToggleType = 0;

		inline bool Enabled = false;
		inline float Speed = 16.0f;

		inline bool Toggled = false;
	}
	namespace InfiniteJump
	{
		inline bool Enabled = false;
	}
	namespace Noclip
	{
		inline int NoclipKey = 0;
		inline int ToggleType = 0;

		inline bool Enabled = false;

		inline bool Toggled = false;
	}
	namespace GravityMod
	{
		inline bool Enabled = false;
		inline float Value = 196.0f;
	}
	namespace JumpPowerMod
	{
		inline bool Enabled = false;
		inline float Value = 50.0f;
	}
	namespace AutoJump
	{
		inline bool Enabled = false;
	}
	namespace PlatformStand
	{
		inline bool Enabled = false;
	}
	namespace NameOcclusion
	{
		inline bool Enabled = false;
		inline bool HideNameplates = false;
		inline float NameDisplayDistance = 0.0f;
		inline float HealthDisplayDistance = 0.0f;
	}
	namespace AutoRotate
	{
		inline bool Enabled = false;
	}
	namespace Radar
	{
		inline bool Enabled = false;
		inline float Size = 120.0f;
		inline float Range = 200.0f;
		inline float PositionX = 100.0f;
		inline float PositionY = 100.0f;
		inline float Opacity = 0.8f;
		inline float DotSize = 3.0f;
		inline bool RotateWithCamera = true;
		inline bool TeamCheck = true;
		inline bool ShowLocalPlayer = true;
		inline bool ShowBorder = true;
		inline float Zoom = 1.0f;
		inline float EnemyColor[3] = {1.0f, 0.0f, 0.0f};
		inline float TeamColor[3] = {0.0f, 1.0f, 0.0f};
		inline float BackgroundColor[4] = {0.0f, 0.0f, 0.0f, 0.7f};
		inline float CrosshairColor[3] = {1.0f, 1.0f, 1.0f};
	}
	namespace SilentAim
	{
		inline int Key = 0;
		inline int ToggleType = 0;   // 0 Hold, 1 Toggle

		inline bool Enabled = false;
		inline bool TeamCheck = true;
		inline bool DownedCheck = true;
		inline float FOV = 100.0f;
		inline float Range = 1000.0f;
		inline int TargetBone = 0;   // reuses aimbot's part enum
		inline bool Prediction = false;
		inline float PredictionX = 1.0f;
		inline float PredictionY = 1.0f;

		// Method selection: 0 = PlayerMouse.Hit + Target write (stable), 1 = UnitRay + Hit + Target (unverified - may crash)
		inline int Method = 0;
		inline bool HitboxOnFire = false;
		inline float HitboxMult = 5.0f;
		inline int HitboxFrames = 2;

		inline uintptr_t CurrentTarget = 0;
		inline bool Toggled = false;
	}
	namespace UIThemes
	{
		// Theme Color Presets (monochrome OLED)
		inline float ThemeDarkBg[3] = {0.05f, 0.05f, 0.05f};
		inline float ThemeDarkText[3] = {1.0f, 1.0f, 1.0f};
		inline float ThemeDarkAccent[3] = {1.0f, 1.0f, 1.0f}; // White

		inline float ThemeLightBg[3] = {0.90f, 0.90f, 0.90f};
		inline float ThemeLightText[3] = {0.05f, 0.05f, 0.05f};
		inline float ThemeLightAccent[3] = {0.35f, 0.35f, 0.35f}; // Gray

		inline float ThemePurpleBg[3] = {0.05f, 0.05f, 0.05f};
		inline float ThemePurpleAccent[3] = {0.80f, 0.80f, 0.80f};

		inline float ThemeBlueAccent[3] = {0.85f, 0.85f, 0.85f};
		inline float ThemeGreenAccent[3] = {0.90f, 0.90f, 0.90f};
	}
	namespace UIAnimations
	{
		// Pulse effect settings
		inline bool PulseEffectEnabled = true;
		inline float PulseSpeed = 2.0f;
		inline float PulseIntensity = 0.3f;

		// Hover effects
		inline bool HoverEffectsEnabled = true;
		inline float HoverBrightnessIncrease = 0.2f;

		// Transition animation
		inline float TabTransitionSpeed = 0.5f;
		inline bool SmoothTabTransitions = true;

		// Gradient effects
		inline bool GradientEffectsEnabled = true;
	}
}

inline bool IsLocalPlayer(const RobloxPlayer& player)
{
	if (player.address != 0 && player.address == Globals::Roblox::LocalPlayer.address)
		return true;
	if (player.Character.address != 0)
	{
		RobloxInstance localCharacter = Globals::Roblox::LocalPlayer.Character();
		if (localCharacter.address != 0 && player.Character.address == localCharacter.address)
			return true;
	}
	return false;
}

inline bool IsLocalPlayer(const RobloxInstance& player)
{
	return player.address != 0 && player.address == Globals::Roblox::LocalPlayer.address;
}

inline std::string GetCharacterClothingTemplate(const RobloxInstance& character)
{
	if (character.address == 0) return "";
	auto shirt = character.FindFirstChildWhichIsA("Shirt");
	if (shirt.address != 0)
		return Memory->readString(shirt.address + Offsets::Clothing::Template);
	auto pants = character.FindFirstChildWhichIsA("Pants");
	if (pants.address != 0)
		return Memory->readString(pants.address + Offsets::Clothing::Template);
	return "";
}

inline bool ClothingIsTeammate(const RobloxPlayer& player, const std::string& localTemplate)
{
	if (localTemplate.empty()) return false;
	std::string playerTemplate = GetCharacterClothingTemplate(player.Character);
	if (playerTemplate.empty()) return false;
	return localTemplate == playerTemplate;
}

struct TeamState
{
	RobloxInstance team = RobloxInstance(0);
	std::string shirtTemplate;
};

inline TeamState LocalTeamState()
{
	TeamState s;
	s.team = Globals::Roblox::LocalPlayer.Team();
	s.shirtTemplate = GetCharacterClothingTemplate(Globals::Roblox::LocalPlayer.Character());
	return s;
}

inline bool IsTeammate(const RobloxPlayer& player, const TeamState& local)
{
	if (Options::Teams::Mode == 1)
		return ClothingIsTeammate(player, local.shirtTemplate);

	return player.Team.address != 0 && local.team.address != 0 &&
		player.Team.address == local.team.address;
}
