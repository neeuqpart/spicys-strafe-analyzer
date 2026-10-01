@echo off
setlocal
cd /d "%~dp0"
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo Visual Studio Installer was not found. Install Visual Studio 2022 or Build Tools with the Desktop development with C++ workload.
    exit /b 1
)
set "MSBUILD="
for /f "usebackq delims=" %%I in (`call "%VSWHERE%" -latest -products * -version "[17.0,18.0)" -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find MSBuild\**\Bin\MSBuild.exe`) do set "MSBUILD=%%I"
if not defined MSBUILD (
    echo Visual Studio 2022 C++ build tools were not found. Install MSVC v143 and a Windows SDK.
    exit /b 1
)
"%MSBUILD%" "strafe analyzer.sln" /p:Configuration=Release /p:Platform=x64 /m /nologo
set "BUILD_RESULT=%ERRORLEVEL%"
if not "%BUILD_RESULT%"=="0" exit /b %BUILD_RESULT%
echo.
echo Build complete. The DLL and loader are in x64\Release.
exit /b 0