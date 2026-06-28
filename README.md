# Kill Feed

SKSE plugin that adds a kill feed for Skyrim Special Edition, Anniversary Edition, and VR
[SSE/AE](https://www.nexusmods.com/skyrimspecialedition/mods/179053)
[VR](https://www.nexusmods.com/skyrimspecialedition/mods/183816)

## Requirements
* [CMake](https://cmake.org/)
	* Add this to your `PATH`
* [PowerShell](https://github.com/PowerShell/PowerShell/releases/latest)
* [Vcpkg](https://github.com/microsoft/vcpkg)
	* Add the environment variable `VCPKG_ROOT` with the value as the path to the folder containing vcpkg
* [Visual Studio Community 2022](https://visualstudio.microsoft.com/)
	* Desktop development with C++
* [CommonLibSSE-NG](https://github.com/alandtse/CommonLibVR/tree/ng)
	* Bundled as a submodule at `extern/CommonLibVR`; no separate setup needed

## User Requirements

* [Address Library for SKSE](https://www.nexusmods.com/skyrimspecialedition/mods/32444)
	* Needed for SSE/AE
* [VR Address Library for SKSEVR](https://www.nexusmods.com/skyrimspecialedition/mods/58101)
	* Needed for VR
* [ImGui VR Helper](https://www.nexusmods.com/skyrimspecialedition/mods/183466)
	* Needed for VR (provides the in-world HUD layer)

## Register Visual Studio as a Generator
* Open `x64 Native Tools Command Prompt`
* Run `cmake`
* Close the cmd window

## Building
```
git clone https://github.com/alandtse/KillFeed.git
cd KillFeed
git submodule update --init --recursive
```

One universal build serves Skyrim SE, AE, and VR — it's built against CommonLibSSE-NG
and detects the runtime at load time:
```
cmake --preset vs2022
cmake --build build --config Release
```
The plugin deploys to whichever of the `Skyrim64Path` / `SkyrimAEPath` / `SkyrimVRPath`
environment variables you have set. Use the `vs2026` preset instead for Visual Studio 2026.
## Licensing

[GPL-3.0-or-later](COPYING) WITH a [Modding Exception and a GPL-3.0 Linking
Exception (with Corresponding Source)](EXCEPTIONS.md), where:

- **Modded Code** — Skyrim and its variants
- **Modding Libraries** — [SKSE](https://skse.silverlock.org/), CommonLib and variants

This is a VR fork of [powerof3's Kill Feed](https://github.com/powerof3/KillFeed);
the original work is © powerofthree under the MIT License.
