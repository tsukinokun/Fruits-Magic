@echo off
rem Visual Studio のソリューションを生成して開く。
rem ソースファイルを追加・削除したときもこれ（または build.bat 前の premake）で再生成する。
cd /d "%~dp0"
External\TsukinoEngine\vendor\premake5.exe vs2022

if %errorlevel% neq 0 (
    echo Premake generation failed.
    pause
    exit /b %errorlevel%
)

start "" ".build\FruitMagic.sln"
