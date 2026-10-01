# Strafe Analyzer

![Game: CS:Source](https://img.shields.io/badge/game-CS%3ASource-yellow)
![Platform: Windows](https://img.shields.io/badge/platform-Windows-blue)
[![Open issues](https://img.shields.io/github/issues/neeuqpart/spicys-strafe-analyzer)](https://github.com/neeuqpart/spicys-strafe-analyzer/issues)
![Language: C++](https://img.shields.io/badge/language-C%2B%2B-pink)
![Architecture: x64](https://img.shields.io/badge/arch-x64-lightgrey)
[![License: MIT](https://img.shields.io/badge/license-MIT-green)](LICENSE)
[![Support Spicy on Patreon](https://img.shields.io/badge/support_Spicy-Patreon-blue)](https://www.patreon.com/spicycurrey)

Free practice software for **Counter-Strike: Source**. Designed as a dynamic-link library (DLL) that is loaded into the 64-bit game process. This version supports **CS:S only**.

A continuation of [Spicy's Strafe Analyzer](https://github.com/spicy/strafe-analyzer), updated for the 64-bit version of Counter-Strike: Source. This version supports **CS:S only**.

This is a practice tool that helps players improve their movement by analyzing player inputs and displaying statistics, real-time graphs, and recorded routes.

## Features

- **Strafe Trainer** — displays a real-time graph of the player's delta yaw alongside the perfect yaw. Shows how fast to move your mouse, with filled, line, horizontal, and vertical graph styles.
- **Sync Trainer** — displays a history of previous key switches. Shows whether you pressed your movement keys early or late compared to your mouse direction, with optional timing statistics.
- **Velocity Graph** — displays a graph of the player's horizontal velocity over time to help track speed changes.
- **Record** — records in-game movement, including view angles, movement key states, position, and velocity, for route visualization during the current session.
- **Recorded Route** — draws a recorded path as lines or dots, with colors for grounded movement, crouching, and optional speed loss.
- **Scroll Graph** — displays a history of jump input to help visualize your jump timing.

<details>
<summary>Customization and controls</summary>

- Adjustable strafe trainer position, graph size, history length, smoothing, and direction.
- Custom colors for perfect yaw, synced and unsynced movement, and counter-strafing.
- Adjustable sync history and colors, plus recorded-route style, distance, thickness, and colors.
- **Insert** opens or closes the menu; **F11** pauses or resumes history collection.
- With the record-route menu enabled, **F1** starts recording, **F2** stops recording, and **F3** clears the recorded route.

</details>

## Download

[Download the source ZIP](https://github.com/neeuqpart/spicys-strafe-analyzer/archive/refs/heads/main.zip), extract it, and follow the build instructions below to create the DLL and loader.

## Build requirements

- Windows with Visual Studio 2022 or Visual Studio 2022 Build Tools.
- The **Desktop development with C++** workload.
- **MSVC v143 C++ x64/x86 build tools** and a **Windows 10 or Windows 11 SDK**.

The required nlohmann JSON 3.9.1 header and its project integration are included. MinHook is included as source. No separate dependency download is required.

Extract the entire archive before building. Keep the included folder structure intact; all project references are relative to the source folder.

## Build in Visual Studio

1. Open `strafe analyzer.sln`.
2. Select **Release** and **x64**.
3. Choose **Build > Build Solution**.

Both projects are built. The resulting files are:

- `x64/Release/strafe analyzer.dll`
- `x64/Release/strafe-analyzer-loader.exe`

## Build without the Visual Studio IDE

Run `build.cmd` from the extracted folder. It locates Visual Studio 2022 or its Build Tools automatically and builds both projects in Release x64.

Alternatively, open an **x64 Native Tools Command Prompt for VS 2022**, change to the extracted folder, and run:

```bat
msbuild "strafe analyzer.sln" /p:Configuration=Release /p:Platform=x64
```

## Load and use

1. Add `-insecure` to the game's Steam launch options and start Counter-Strike: Source.
2. Run `x64/Release/strafe-analyzer-loader.exe` with the DLL in the same folder.
3. Press **Insert** in the game to open the menu. Press **F11** to pause or resume history collection.

The loader accepts only `cstrike_win64.exe`. To use another DLL location, supply its path as the loader's single argument. Restart the game before loading a newly rebuilt DLL.

The game must be installed separately. Engine signatures and interface layouts are version-dependent; future game updates may require source changes. This package does not include game files.

## Source layout

- `strafe analyzer/` — analyzer, interfaces, menu, and bundled MinHook source.
- `loader/` — x64 DLL loader.
- `packages/` — bundled JSON dependency.
- `tests/` — standalone regression checks and instructions.

## License and credits

The original project is distributed under the MIT license; see `LICENSE`. Third-party copyright and license notices remain in their source files. See `THIRD_PARTY_NOTICES.md` for dependency credits.

Special thanks to **Spicy** for creating the original Strafe Analyzer. You can support his work on [Patreon](https://www.patreon.com/spicycurrey).
