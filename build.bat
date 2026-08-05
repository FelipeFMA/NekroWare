@echo off
setlocal enabledelayedexpansion

:: Run the build from this file's directory (project root)
cd /d "%~dp0"

set "SLN=NekroWare.sln"
set "CFG=/p:Configuration=Release /p:Platform=x64"

:: 1) Known Build Tools install path
set "MSBUILD=%ProgramFiles(x86)%\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
if not exist "%MSBUILD%" set "MSBUILD=%ProgramFiles%\Microsoft Visual Studio\18\BuildTools\MSBuild\Current\Bin\MSBuild.exe"

:: 2) Known full Visual Studio install path
if not exist "%MSBUILD%" set "MSBUILD=%ProgramFiles(x86)%\Microsoft Visual Studio\18\MSBuild\Current\Bin\MSBuild.exe"
if not exist "%MSBUILD%" set "MSBUILD=%ProgramFiles%\Microsoft Visual Studio\18\MSBuild\Current\Bin\MSBuild.exe"

:: 3) vswhere lookup (any edition / year)
if not exist "%MSBUILD%" (
    set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
    if exist "!VSWHERE!" (
        for /f "usebackq tokens=*" %%i in (`"!VSWHERE!" -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
            set "MSBUILD=%%i"
        )
    )
)

:: 4) Fall back to PATH (e.g. launched from a Developer Command Prompt)
if not exist "%MSBUILD%" (
    where msbuild >nul 2>nul
    if !errorlevel!==0 set "MSBUILD=msbuild"
)

if not exist "%MSBUILD%" (
    echo [ERROR] MSBuild.exe not found. Install Visual Studio 2022^+ with the v145 toolset,
    echo         or run this from a Developer Command Prompt for VS.
    pause
    exit /b 1
)

echo Using MSBuild: %MSBUILD%
echo.
"%MSBUILD%" "%SLN%" %CFG% /m /v:m /nologo
if errorlevel 1 (
    echo.
    echo [BUILD FAILED]
    pause
    exit /b 1
)

echo.
echo [BUILD OK] Output: %~dp0build\NekroWare.exe
pause
exit /b 0