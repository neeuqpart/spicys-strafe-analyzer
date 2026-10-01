# Perfect-bar regression checks

From an **x64 Native Tools Command Prompt for VS 2022**, in the project root:

```bat
cl /nologo /EHsc /std:c++17 tests\perfect_bars.cpp "strafe analyzer\Math\vector3d.cpp" /Fe:tests\perfect_bars.exe /Fo:tests\
tests\perfect_bars.exe
```

The test uses the production trainer drawing, tick-history calculation, and
netvar traversal with synthetic interfaces. It checks all four graph styles,
matching and overshooting yaw, small positive and out-of-range targets, 720p
placement, ground/prespeed and airborne targets, replicated ConVar parent
values, and isolation of player offsets from unrelated receive tables.

These checks do not inject into a game or verify the running game's ABI.
After rebuilding, restart CSS and use the loader to load the new DLL, then
check each graph style while prespeeding and strafing in the air.
