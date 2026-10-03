@echo off
rem ---------------------------------------------------------------------------
rem First-time setup.
rem   1) Fetch submodules (TsukinoEngine and its dependencies)
rem   2) Generate the Visual Studio solution with premake
rem NOTE: keep this file ASCII only (cmd.exe mis-parses multibyte text in batch files).
rem ---------------------------------------------------------------------------
setlocal
cd /d "%~dp0"

git config core.longpaths true
git submodule update --init --recursive
if errorlevel 1 (
    echo setup.bat: git submodule update failed.
    exit /b 1
)

External\TsukinoEngine\vendor\premake5.exe vs2022
if errorlevel 1 (
    echo setup.bat: premake generation failed.
    exit /b 1
)

echo setup.bat: done. Run build.bat to build, or open.bat to open Visual Studio.
