# NekroWare

External memory cheat for Roblox (formerly "Celex"), built in C++20 for x64 Windows.
It reads/writes the Roblox client process directly and renders an in-game style overlay menu with ImGui + DirectX 11.

> **Disclaimer:** This project is for educational and private use only. Using it violates the Roblox ToS and your account may be banned. Use at your own risk.

---

## Table of Contents

1. [What it is / What it does](#what-it-is--what-it-does)
2. [Project layout](#project-layout)
3. [How it works](#how-it-works)
   - [Memory layer](#memory-layer)
   - [Roblox SDK layer](#roblox-sdk-layer)
   - [Globals & caching](#globals--caching)
   - [Feature loops](#feature-loops)
   - [Overlay & menu](#overlay--menu)
   - [Offsets file](#offsets-file)
4. [Feature list](#feature-list)
5. [Building](#building)
6. [Running](#running)
7. [How to add a new feature](#how-to-add-a-new-feature)
8. [How to remove a feature](#how-to-remove-a-feature)
9. [How to update offsets](#how-to-update-offsets)
10. [Maintenance](#maintenance)
11. [Troubleshooting](#troubleshooting)
12. [Known dead code & leftovers](#known-dead-code--leftovers)

---

## What it is / What it does

NekroWare is an **external** cheat: a separate .exe that attaches to the Roblox client
(`RobloxPlayerBeta.exe`), resolves the Roblox instance tree from memory, and exposes:

- **Combat:** Aimbot (mouse-based, sticky aim), Triggerbot (fires when crosshair is over an enemy), Hitbox expander, Walk-through.
- **Visuals:** Full ESP (boxes, 2D/3D, tracers, skeleton, names, distance, health, head circle, corner ESP), Radar, custom Crosshair, advanced FOV boxes.
- **Movement:** Fly, WalkSpeed (velocity based), Infinite jump, Auto jump, Platform stand, Noclip.
- **Misc:** Camera FOV changer, Headless, Gravity modifier, Jump power modifier, Name occlusion / nameplate hiding, Macro (I/O key spam), keybind list, FPS counter, monochrome OLED UI theme.

Everything is configurable from the in-game menu. No DLL injection is used.

---

## Project layout

```
NekroWare.sln                     Solution (Release|x64 is the only working config)
NekroWare/
  main.cpp                        Entry point: attach, resolve instance tree, start threads
  NekroWare.vcxproj               MSBuild project (toolset v145, MASM, C++20)
  Memory/
    MemoryManager.h/.cpp          Process handle, PID, module base, read/write helpers
    Luck.asm                      Direct syscall stubs (NtReadVirtualMemory / NtWriteVirtualMemory)
  rbx/
    offsets.h                      Offsets fallback header - replaced verbatim from the dump (see below)
    offsets_registry.h             AUTO-GENERATED name->variable map (tools/gen_offsets_registry.ps1)
    OffsetsLoader.h                Resolves the running client version, downloads matching
                                   offsets from offsets.imtheo.lol at startup, applies them at runtime
    SDK/SDK.h                      RobloxInstance class (child tree walking, property reads)
    math/math.h                   Vectors/Matrix math types
    globals/globals.h             Global state (DataModel, Workspace, Players, caches, ...)
    globals/options.h             Every feature toggle & setting lives here (Options::)
    Caches/playercache.h          CachePlayers thread (Players + optional NPCs, every 5 s)
    Caches/playerobjectscache.h   CachePlayerObjects thread (fills RobloxPlayer structs)
    Caches/TPHandler.h            Teleport/leave-game handler (re-resolves the tree)
    configs/                      DEAD CODE - JSON config save/load, not wired up (see below)
  features/
    aimbot.h                      RunAimbot + GetClosestPlayer (mouse aim)
    triggerbot.h                  RunTriggerbot + RenderAdvancedFOV
    esp.h                         RenderESP (all visual ESP types)
    crosshair.h                   RenderCrosshair (custom crosshair styles)
    misc.h                        MiscLoop (FOV, headless, jumps, noclip, gravity, jump power, name occlusion)
    fly.h                         FlyLoop (velocity-based flight)
    speed.h                       SpeedLoop (walk speed)
    hitboxexpander.h              RunHitboxExpander (size + collision bits)
    macro.h                       RunMacro (I/O key spam)
  overlay/
    renderer.h / renderer.cpp     DX11 overlay window + ImGui menu (ShowImgui)
    utils/W2S.h                   WorldToScreen (view matrix projection)
    utils/utils.h                 misc helpers + default accent color
    utils/Header.h                embedded icon/logo font data
    imgui/                        ImGui 1.9x-ish + DX11/Win32 backends + KeyBind widget
build/                            Build output (NekroWare.exe) + intermediates (build/shit)
build.bat                         One-click Release|x64 build script (see Building)
tools/gen_offsets_registry.ps1    Regenerates offsets_registry.h after replacing offsets.h
README.md                         This file
```

---

## How it works

### Memory layer

- `MemoryManager` finds the process by name (`RobloxPlayerBeta.exe`) with a toolhelp snapshot,
  opens it with `OpenProcess(PROCESS_ALL_ACCESS, ...)` and resolves the module base address.
- All reads/writes go through `Memory->read<T>(addr)` / `Memory->write<T>(addr, value)`.
  These call `Luck_ReadVirtualMemory` / `Luck_WriteVirtualMemory` declared in `MemoryManager.h`
  and implemented in `Memory/Luck.asm` as **direct syscalls** (NtReadVirtualMemory = 0x3F,
  NtWriteVirtualMemory = 0x3A, x64). The exe therefore has no `ReadProcessMemory`/`WriteProcessMemory`
  import, which makes it quieter for AV/anti-cheat scanners.
- `readString` understands the Roblox `std::string` layout: length at `+0x18`; if the string is
  >= 16 chars the data lives on the heap (pointer at `+0`), otherwise it is inline.

### Roblox SDK layer

`RobloxInstance` (in `rbx/SDK/SDK.h`) is a thin wrapper around a Roblox instance address.
It walks the instance tree using the generic fields from `Offsets::Instance`
(`ChildrenStart`/`ChildrenEnd` list, `Name`, `ClassDescriptor`/`ClassName`) and reads
typed properties through the per-class offsets:

- `GetChildren()`, `FindFirstChild(name)`, `FindFirstChildWhichIsA(className)`
  (these walk the child list in memory - keep them out of hot loops; the caches exist for that)
- `Position()` / `Size()` / `CFrame()` - read through the `Primitive` pointer
- `Character()`, `Health()` / `MaxHealth()`, `Team()`, `RigType()`
- `SetWalkspeed()` / `SetJumpPower()` / `GetFOV()` / `SetFOV()`

### Globals & caching

Everything important lives in `Globals` (rbx/globals/globals.h):
`Roblox::DataModel / VisualEngine / Workspace / Players / Camera / LocalPlayer`,
plus `Caches::CachedPlayers` (instance list) and `Caches::CachedPlayerObjects`
(rich `RobloxPlayer` structs: name, display name, user id, team instance, health,
character + all body parts by rig type).

Two cache threads keep these fresh:

- `CachePlayers` - every 5 seconds; real players from the `Players` service + optional NPCs
  (Models with a Humanoid + HumanoidRootPart in Workspace/Folders, `Options::Misc::CacheNPCs`).
- `CachePlayerObjects` - a fast loop that rebuilds the rich `RobloxPlayer` structs.
- `TPHandler` - watches every 100 ms for a teleport or leaving the game
  (`dataModel.Name() != "Ugc"` or `PlaceId` changed), then re-resolves the entire tree
  (`FakeDataModel::Pointer -> RealDataModel`, `VisualEngine::Pointer`, Workspace/Players/Camera,
  LocalPlayer) and clears the caches. This is why the cheat survives teleports.

`main.cpp` does the initial attach and resolves the tree the same way the TP handler does.

### Feature loops

`main()` starts one detached thread per background feature: `MiscLoop`, `RunHitboxExpander`,
`FlyLoop`, `SpeedLoop` (+ the cache threads). Every loop:

1. checks its `Options::*::Enabled` flag (set from the menu),
2. grabs the needed instances (usually `CachedPlayerObjects` / local character / humanoid),
3. reads or writes the required fields via `Offsets::*`,
4. sleeps briefly.

Rendering features (`RenderESP`, `RenderAdvancedFOV`, `RenderCrosshair`, `RenderRadar`,
keybind list, FPS text) run every frame from inside the overlay render loop instead.

### Overlay & menu

`ShowImgui()` (overlay/renderer.cpp:172) creates a borderless, topmost, transparent DX11
overlay window, positions it over the Roblox window, and runs the ImGui frame loop:
ImGui renders the menu (tabs + widgets bound to `Options::*`), then the background draw list
is used for ESP/radar/crosshair etc. so they are drawn while the menu is closed too.

The UI is a **monochrome OLED theme** (black/gray/white only): the palette is applied in
`ShowImgui()` right after context creation (near-black window/child/popup backgrounds, gray
frames/buttons, white text), and every accent-colored element (checkmarks, tabs, sliders,
FOV circles, the "NekroWare" logo) is driven by `main_color`
(`overlay/utils/utils.h`) which defaults to white and follows
`Options::Misc::MenuAccentColor` (default `{1,1,1}` in `rbx/globals/options.h`). The "Menu
Accent" color picker (Visuals > Colours) can still recolor the accent at runtime; the old
`ApplyTheme`/`SetXTheme` theme functions are empty stubs and the preset arrays in
`Options::UIThemes` are unused.

`overlay/imgui/KeyBind.h` provides `KeybindSelector` and `KeyBind::IsPressed` for
keybindable features, plus `ToggleType` conventions (0 = hold, 1 = toggle) used by
several features.

### Offsets file

Offsets are **resolved at runtime** by `rbx/OffsetsLoader.h`, right after attaching
(`OffsetsDynamic::LoadOffsets()` in `main.cpp`):

1. **Detect the version** - the running `RobloxPlayerBeta.exe` path is read from the
   toolhelp module snapshot; the `version-xxxxxxxxxxxxxxxx` folder name is extracted
   from it (works for Fishstrap and the official launcher, both use
   `...\Versions\version-xxx\RobloxPlayerBeta.exe`).
2. **Download** `https://offsets.imtheo.lol/<version>/offsets.hpp` over HTTPS (WinHTTP).
3. **Parse & apply** - the header is parsed into `Namespace::Member` keys and each
   value is written into the matching runtime variable in `rbx/offsets.h`, via the
   name->address map in `rbx/offsets_registry.h`.

That means the cheat auto-adapts to the client build it finds - no manual dump swap
needed after every Roblox update. Static pointers (FakeDataModel, VisualEngine, ...)
and any moved offsets all come from the matching build.

**Fallback:** if detection, download or parsing fails, the cheat keeps the built-in
values compiled from `rbx/offsets.h` (a verbatim dump generated for a specific build)
and logs it. `rbx/offsets.h` uses runtime variables (`inline uintptr_t`) whose initial
values are the dump's; the loader only overrides what the downloaded header contains,
so members missing from a newer dump keep their fallback silently (e.g. the aimbot's
`MouseService::SensitivityPointer == 0` guard).

The dump header comment carries the client version it was made for - the current file
is for `version-d584fb6c717a43d9` (Fishstrap). **Never edit `rbx/offsets.h` values by
hand** - replace it verbatim from the dump and run the registry script (see
[How to update offsets](#how-to-update-offsets)). All feature code references offsets
as `Offsets::Namespace::Member`.

Important layout facts this dump enforces:

| What the code needs | Where it lives in this dump |
|---|---|
| `BasePart::Position/Size/Rotation/AssemblyLinearVelocity` | `Offsets::Primitive::*` (read via `BasePart::Primitive` pointer, 0x128) |
| `BasePart::CanCollide` / `CanQuery` | `Offsets::PrimitiveFlags::*` (bit flags at Primitive+0x8 / +0x20) |
| `Workspace::Gravity` | `Offsets::Workspace::World` (0x3f0) -> `Offsets::World::Gravity` (0x210) |
| Team membership | compare `Team` **instance pointers** (no `Team::BrickColorName` string in this dump) |
| Silent aim | not possible (no `PlayerMouse::Hit/Target/UnitRay` in this dump) - feature removed |
| `Player::GroupId` | not in this dump - removed |

---

## Feature list

| Category | Feature | Where |
|---|---|---|
| Combat | Aimbot (FOV, sticky aim, team check, downed check, target bone, stutter) | `features/aimbot.h`, `Options::Aimbot` |
| Combat | Triggerbot (fires on enemy under crosshair) + advanced FOV boxes | `features/triggerbot.h`, `Options::Triggerbot` |
| Combat | Hitbox expander (X/Y size, show hitbox, walk-through) | `features/hitboxexpander.h`, `Options::HitboxExpander` |
| Visual | ESP: boxes (2D/3D), tracers, skeleton, name, distance, health, head circle, corner ESP, outlines, colors | `features/esp.h`, `Options::ESP` |
| Visual | Radar (zoom, range, rotate with camera, team color, team check) | `overlay/renderer.cpp` `RenderRadar`, `Options::Radar` |
| Visual | Custom crosshair (static / pulse / spin, colors, spin speed) | `features/crosshair.h`, `Options::Crosshair` |
| Visual | Keybind list overlay | `renderer.cpp`, `Options::Misc::KeybindList` |
| Visual | FPS counter (top right) + watermark ("NekroWare") | `renderer.cpp` |
| Visual | Monochrome OLED theme (black/gray/white) + menu accent color picker | `renderer.cpp`, `Options::Misc` |
| Movement | Fly (WASD + space/ctrl, speed) | `features/fly.h`, `Options::Fly` |
| Movement | Walk speed (velocity based) | `features/speed.h`, `Options::WalkSpeed` |
| Movement | Infinite jump, Auto jump, Platform stand | `features/misc.h`, `Options::InfiniteJump/AutoJump/PlatformStand` |
| Movement | Noclip / walk-through | `features/misc.h`, `Options::Noclip` |
| Misc | Camera FOV changer | `features/misc.h`, `Options::Misc::FOVEnabled` |
| Misc | Headless (hides your head) | `features/misc.h`, `Options::ESP::Headless` |
| Misc | Gravity modifier | `features/misc.h`, `Options::GravityMod` |
| Misc | Jump power modifier | `features/misc.h`, `Options::JumpPowerMod` |
| Misc | Name occlusion / nameplate hiding | `features/misc.h`, `Options::NameOcclusion` |
| Misc | Macro (alternates I/O key presses, keybind + delay) | `features/macro.h`, `Options::Macro` |
| Misc | Notifications (UI plumbing - hooks exist, some stubs empty) | `renderer.cpp`, `Options::Misc` |

---

## Building

Requirements:

- Windows 10/11 x64.
- Visual Studio 2022+ **or** Build Tools for Visual Studio with:
  - the **v145** C++ toolset (MSVC),
  - Windows SDK,
  - **MASM (ml64.exe)** - needed for `Memory/Luck.asm` (it comes with the "Desktop development with C++" workload).
- No third-party libraries: ImGui and json are vendored in-tree.

From a **Developer Command Prompt for VS** (or using `MSBuild.exe` directly):

```
msbuild NekroWare.sln /p:Configuration=Release /p:Platform=x64
```

Output: `build\NekroWare.exe` (and `build\NekroWare.pdb`).

Notes:

- Only `Release|x64` is actually used/tested. `Debug|Win32` / `Release|Win32` configs exist
  but are not maintained; the cheat is x64 only.
- The project sets `<TargetName>NekroWare</TargetName>` so the exe is named `NekroWare.exe`.
- Build logs: many C4244/C4305 conversion warnings are normal and pre-existing; only
  **errors** (C2xxx/C4xxx with "error") matter.
- If Windows Defender deletes the freshly built exe, add an exclusion for the `build\`
  folder (or the repo), or rebuild with an output dir outside the repo. See Troubleshooting.

### One-click build (build.bat)

Double-click `build.bat` in the repo root (or run it from any terminal). It:

1. Locates `MSBuild.exe` - checks the known VS 2022+/Build Tools install paths
   (`%ProgramFiles(x86)%\Microsoft Visual Studio\18\...`), then falls back to `vswhere`,
   then to `msbuild` on `PATH` (e.g. inside a Developer Command Prompt).
2. Builds `Release|x64` (`NekroWare.sln /p:Configuration=Release /p:Platform=x64 /m`).
3. Prints `[BUILD OK] Output: build\NekroWare.exe` on success, or `[BUILD FAILED]`
   and pauses so the window stays open either way.

### Building from this repo's exact toolchain (PowerShell)

```
& "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\MSBuild.exe" `
  "NekroWare.sln" /p:Configuration=Release /p:Platform=x64 /m /v:m /nologo
```

---

## Running

1. Start Roblox (any launcher is fine - Fishstrap, official, etc.). The cheat looks for a
   window titled **"Roblox"** and the process **`RobloxPlayerBeta.exe`**.
2. Run `build\NekroWare.exe` **as Administrator** (required for `OpenProcess(PROCESS_ALL_ACCESS)`
   and the overlay).
3. Watch the console:
   - it waits until Roblox exists, attaches, prints the PID + base address,
   - waits for the game to load (`DataModel` name becomes `Ugc`),
   - logs the resolved tree and your username,
   - starts the feature threads.
4. The overlay menu appears over the game. Toggle features, bind keys, close the menu
   (`End`/`Insert`-style toggle - see the menu keybind) and the visuals keep running.
5. Teleporting or leaving the game is handled automatically by `TPHandler`.

> Tip: the console must stay open while running (`std::cin.get()` in `main()`).

---

## How to add a new feature

Features follow a simple pattern. Example: add a **"God mode"** visual toggle that sets your
humanoid's MaxHealth to a large value.

### 1. Add the options (rbx/globals/options.h)

Add a namespace (or extend an existing one). Every menu-visible state lives here:

```cpp
namespace GodMode
{
    inline bool Enabled = false;
    inline float Health = 10000.0f;
}
```

### 2. Create the feature header (features/godmode.h)

Loop-style feature - copy the pattern from `misc.h`:

```cpp
#pragma once
#include "../rbx/globals/options.h"
#include "../rbx/globals/globals.h"
#include <thread>

inline void GodModeLoop()
{
    while (true)
    {
        if (Options::GodMode::Enabled)
        {
            auto character = Globals::Roblox::LocalPlayer.Character();
            auto humanoid = character.FindFirstChildWhichIsA("Humanoid");
            if (humanoid.address != 0)
                Memory->write<float>(humanoid.address + Offsets::Humanoid::MaxHealth,
                                     Options::GodMode::Health);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
```

### 3. Start the thread (main.cpp)

```cpp
#include "features/godmode.h"
...
std::thread(GodModeLoop).detach();   // next to the other std::thread(...).detach() calls
```

### 4. Add the menu UI (overlay/renderer.cpp)

Inside the menu drawing code (find where other features are toggled, e.g. `Options::HitboxExpander::Enabled`):

```cpp
ImGui::Checkbox("God Mode", &Options::GodMode::Enabled);
ImGui::SliderFloat("Health", &Options::GodMode::Health, 100.f, 100000.f);
```

### 5. Render-style features (ESP-like)

If the feature *draws* (ESP, radar, crosshair...):

- Add a `void RenderMyFeature(ImDrawList* drawList)` in a new/existing feature header,
- call it from the frame loop in `ShowImgui()` where `RenderESP(...)` / `RenderRadar(...)` are called.

### 6. Keybinds

To make a feature keybindable, follow the existing convention:

- add `inline int Key = 0; inline int ToggleType = 0;` (and optionally `inline bool Toggled = false;`) to the options namespace,
- use `KeyBind::IsPressed(Options::MyFeature::Key)` + the toggle/hold pattern from `macro.h` or the aimbot key handling,
- expose `KeybindSelector("Key", &Options::MyFeature::Key)` in the menu.

### 7. Project file

Headers don't strictly need to be listed in `NekroWare.vcxproj` to compile (they are pulled
in via `#include`), but adding them under `<ClInclude>` keeps Solution Explorer tidy.

---

## How to remove a feature

Example: remove **WalkSpeed** completely.

1. **Remove the menu UI** in `overlay/renderer.cpp` - delete every ImGui control that references
   `Options::WalkSpeed::*`.
2. **Remove the loop/start call** in `main.cpp` - delete `#include "features/speed.h"` and
   `std::thread(SpeedLoop).detach();`.
3. **Delete the feature file** `features/speed.h`.
4. **Delete the options namespace** `Options::WalkSpeed` from `rbx/globals/options.h`
   (and remove any `SpeedLoop` declarations if present).
5. **Remove the file entry** from `NekroWare.vcxproj` (`<ClInclude Include="features\speed.h" />`)
   and `NekroWare.filters`.
6. Rebuild - if anything else still references the removed symbols, the compiler will point
   you at it (usually ESP/other features reading `Options::WalkSpeed::Speed`).

Removing a feature is also the **correct fix** when a new offsets dump no longer contains
the fields the feature needs (see below - that is exactly what happened to Silent Aim).

---

## How to update offsets

Offsets are fetched **automatically at startup** for the client build the cheat finds
(see the [Offsets file](#offsets-file) section): version is detected from the running
process path, the matching header is downloaded from `offsets.imtheo.lol`, and applied
at runtime. `rbx/offsets.h` is only the offline fallback, so in practice you rarely
touch it after a Roblox update.

You still need to refresh `rbx/offsets.h` when:

- **The URL is unreachable / you're offline** - then the fallback build determines
  whether the cheat works.
- The fallback is the *only* source the loader has, so keep it recent.

To update the fallback (`rbx/offsets.h`):

### 1. Get the dump for your client build

Download from `https://offsets.imtheo.lol/Offsets.hpp` (or your preferred source) and
**replace `rbx/offsets.h` with it, verbatim, without any edits**. The file is a
self-contained header; nothing else needs to know about the version.

### 2. Regenerate the registry

The loader needs a map of every offset variable so it can override them at runtime.
After replacing `offsets.h`:

```
powershell -ExecutionPolicy Bypass -File tools\gen_offsets_registry.ps1
```

This script:
- converts the dump's `inline constexpr uintptr_t` members to `inline uintptr_t`
  (values kept as the fallback, allowed to be overridden at startup),
- regenerates `rbx/offsets_registry.h` with `OFFSET_REG(Namespace, Member)` entries.

The registry is a compile-time list, so if a dump *removes* a member the entry simply
never gets overridden (fallback stays) - do not hand-edit the registry.

### 3. Compile and let the compiler find what broke

```
msbuild NekroWare.sln /p:Configuration=Release /p:Platform=x64
```

The compiler will error on every `Offsets::*` member that no longer exists.
Fix the *call sites* (never the dump). Past dumps have forced these changes:

| Old reference (from older dumps) | New reference (current dump) |
|---|---|
| `Offsets::BasePart::Position/Size/Rotation` | `Offsets::Primitive::*` - deref `BasePart::Primitive` (0x128) first (SDK.h already does this) |
| `Offsets::BasePart::AssemblyLinearVelocity` | `Offsets::Primitive::AssemblyLinearVelocity` |
| `Offsets::BasePart::CanCollide` / `CanQuery` | `Offsets::PrimitiveFlags::CanCollide` / `CanQuery` |
| `Offsets::Workspace::Gravity` | `Offsets::Workspace::World` (ptr) -> `Offsets::World::Gravity` |
| `Offsets::Team::BrickColorName` (string compare) | not present -> compare `Team` **instance pointers** |
| `Offsets::Player::GroupId` | not present -> remove the read |
| `Offsets::PlayerMouse::Hit/Target/UnitRay` (silent aim) | not present -> feature must be removed |

### 4. Grep for leftovers that compile but are wrong

Names that still exist in the dump can still mean the wrong thing (e.g. `BasePart::Primitive`
vs an old direct field). After fixing compile errors, search the codebase:

```
grep -rn "Offsets::BasePart::" NekroWare --include=*.h --include=*.cpp
grep -rn "Offsets::Team::"        NekroWare --include=*.h --include=*.cpp
grep -rn "Offsets::Workspace::"   NekroWare --include=*.h --include=*.cpp
grep -rn "Offsets::Player"        NekroWare --include=*.h --include=*.cpp
```

Every hit must be one of the members that exists in the new dump.

### 5. Features that no longer have offsets

If a feature's required fields are gone from the dump and there is no substitute,
**remove the feature** (see [How to remove a feature](#how-to-remove-a-feature)) rather than
leaving broken/wrong reads. Rule of the repo: only offsets from the installed dump are used;
no old values may be kept.

### 6. Verify and rebuild

- Rebuild to zero errors (warnings are fine).
- Sanity checks: ESP shows, aimbot locks, team check separates teams, gravity change works.
- `git diff` (if you use VCS) should show only `rbx/offsets.h` replaced + call-site fixes.

---

## Maintenance

- **Keep offsets in sync with the client build.** Roblox/Fishstrap updates are handled
  automatically: the loader fetches matching offsets at startup from
  `offsets.imtheo.lol`, so stale offsets only bite when offline / download fails
  (then the built-in fallback matters). Keep `rbx/offsets.h` fresh anyway (see
  [How to update offsets](#how-to-update-offsets)).
- **Never hand-edit `rbx/offsets.h`.** Replace it from the dump source and run
  `tools\gen_offsets_registry.ps1`, then fix call sites.
- **Keep `Options` in sync with the menu.** A toggle with no UI (or UI writing to a removed
  option) is dead weight - delete both sides together.
- **Mind the instance-tree cost.** `FindFirstChild` / `GetChildren()` walk memory lists and
  are slow: cache instances (like `CachedPlayerObjects`) instead of calling them per frame.
- **Reads are unchecked.** `Memory->read<T>` on a freed/invalid instance returns garbage, not
  an exception. Always guard with `.address != 0` checks, as the existing code does.
- **Use `Primitive`/`World` pointers correctly.** Anything physical is behind
  `BasePart::Primitive`; gravity is behind `Workspace::World`; the SDK helpers already do
  the chasing - reuse them instead of inlining new offsets.
- **Team checks are pointer comparisons** (`player.Team.address == localTeam.address`),
  not string comparisons, in this dump's layout. Keep that pattern.
- **Build logs** contain many C4244 warnings - they are noise. Only errors block the build.
- **Backups:** `rbx/globals/globals.h.bak` and `options.h.backup` are stale leftover backups;
  they are not compiled. Delete them when you are confident.
- **Credits:** the in-menu credit line ("Made by Zaka | s/o Claude") and the embedded
  icon/logo in `overlay/utils/Header.h` are original assets - leave them unless you rebrand.

---

## Troubleshooting

| Symptom | Cause / fix |
|---|---|
| "Roblox not found!" then hangs | Roblox window isn't titled "Roblox" or isn't open. Open a game first, then start the exe. |
| "Failed to attach" | Not running as Administrator, or Roblox player isn't the `RobloxPlayerBeta.exe` process (some launchers use different process names). |
| Exe disappears after build | Windows Defender quarantines it (direct syscalls + `PROCESS_ALL_ACCESS` look suspicious). Add an exclusion for the `build\` folder or the whole repo, or build with a custom `OutDir` outside the repo. |
| Compile errors like `Offsets::X is not a member` | Offsets dump out of date / wrong build. See [How to update offsets](#how-to-update-offsets). |
| ESP shows nothing | Stale offsets, or you are not in a game (DataModel never became "Ugc"). Check the console log. |
| Aimbot targets teammates | Team check off, or the team instances are 0 (no teams in that game - then pointer compare never matches and no one is filtered, which is correct). |
| Teleport/leave game hangs | `TPHandler` polls; it re-resolves everything. If a new game takes longer than a few seconds, the new client version may have moved `FakeDataModel::Pointer`. |
| Menu keys don't respond | Overlay window doesn't have focus; click the overlay first. |
| Crash on attach | Roblox closed while the cheat was running; restart both. |
| `warning MSB8028` (shared intermediates) | Old `celex.vcxproj` build leftovers in `build\shit\`; delete the `build` folder and rebuild clean. |

---

## Known dead code & leftovers

These exist in the repo but do **nothing** - do not waste time "fixing" them unless you
intend to finish the feature:

- `rbx/configs/configs.h` (+ `json.hpp`, `json_fwd.hpp`) - JSON config save/load was never
  wired in (nothing includes it; `Globals::configsPath` is undefined). Deleting it is safe.
- `Options::SilentAim` namespace in `rbx/globals/options.h` - leftover from the removed
  silent aim feature; nothing reads it. Safe to delete.
- `features/silentaim.h` was **removed** (its offsets, `PlayerMouse::Hit/Target/UnitRay`,
  are not in the current dump). If a future dump contains them, the feature can be
  re-implemented following the aimbot patterns.
- `Offsetsnewest.txt` - an old offsets dump for an **older** client build
  (`version-5cf2272675e145f5`), kept as reference only. Do not confuse it with `rbx/offsets.h`.
- `NekroWare/NekroWare.filters` - the `.filters` file still contains stale include paths from
  an even older layout; it only affects Solution Explorer grouping, not the build.
