@echo off
rem Generate the Visual Studio solution and open it.
rem Also run this after adding or removing source files.
rem NOTE: keep this file ASCII only (cmd.exe mis-parses multibyte text in batch files).
cd /d "%~dp0"
External\TsukinoEngine\vendor\premake5.exe vs2022

if %errorlevel% neq 0 (
    echo Premake generation failed.
    pause
    exit /b %errorlevel%
)

start "" ".build\FruitMagic.sln"
