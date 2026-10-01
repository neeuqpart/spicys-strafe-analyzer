# Strafe Analyzer for Counter-Strike: Source

A continuation of [Spicy's Strafe Analyzer](https://github.com/spicy/strafe-analyzer), updated for the 64-bit version of Counter-Strike: Source. This version supports **CS:S only**.

The analyzer displays movement practice tools including strafe trainers, key-switch timing, velocity history, and route recording. The original project and bundled dependency attributions are preserved. You can support Spicy's work on [Patreon](https://www.patreon.com/spicycurrey).

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