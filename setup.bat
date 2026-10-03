@echo off
rem ---------------------------------------------------------------------------
rem 初回セットアップ。
rem   1) サブモジュール（TsukinoEngine とその依存ライブラリ）を取得
rem   2) premake で Visual Studio ソリューションを生成
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
