@echo off
rem ---------------------------------------------------------------------------
rem FruitMagic（エンジン込み）のビルド。
rem   build.bat            … Debug をビルド
rem   build.bat Release    … Release をビルド
rem
rem 成功時の出力は 0 行、失敗時はエラー行だけになる。
rem MSBuild を直接叩かないこと（静音フラグを忘れると数千行出る）。
rem External\TsukinoEngine\build.bat をベースにしている（MSBuild 探索の理由はそちらを参照）。
rem ---------------------------------------------------------------------------
setlocal

set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Debug"

set "ROOT=%~dp0"
set "SLN=%ROOT%.build\FruitMagic.sln"
set "PREMAKE=%ROOT%External\TsukinoEngine\vendor\premake5.exe"

rem ---------------------------------------------------------------------------
rem MSBuild の場所を解決する（VS2022 → 最新 VS → 決め打ちパスの順）
rem ---------------------------------------------------------------------------
set "MSBUILD="
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" call :find_vs2022
if not defined MSBUILD if exist "%VSWHERE%" call :find_latest
if not defined MSBUILD set "MSBUILD=C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"

if not exist "%MSBUILD%" (
    echo build.bat: MSBuild not found. Install Visual Studio 2022 with the "Desktop development with C++" workload.
    exit /b 1
)

rem ---------------------------------------------------------------------------
rem プロジェクトファイルの生成。
rem ソースファイルを追加・削除したときにも反映されるよう毎回 premake を通す（数百ms）
rem ---------------------------------------------------------------------------
pushd "%ROOT%"
rem エンジン側 premake5.lua の deprecated 警告が stderr に出るため、成功時は両方捨てる。
rem 失敗したときだけもう一度実行して、原因を表示する
"%PREMAKE%" vs2022 >nul 2>&1
if errorlevel 1 (
    "%PREMAKE%" vs2022
    popd
    echo build.bat: premake generation failed.
    exit /b 1
)
popd

rem NuGet の復元。premake が吐くのは旧形式の packages.config なので
rem -t:restore だけでは復元されず -p:RestorePackagesConfig=true が要る
if not exist "%ROOT%.build\packages" (
    "%MSBUILD%" "%SLN%" -t:restore -p:RestorePackagesConfig=true -p:Configuration=%CONFIG% -p:Platform=x64 -nologo -v:q -clp:"ErrorsOnly;NoSummary"
    if errorlevel 1 exit /b 1
)

"%MSBUILD%" "%SLN%" /p:Configuration=%CONFIG% /p:Platform=x64 /m /nologo /v:q /clp:"ErrorsOnly;NoSummary"
exit /b %ERRORLEVEL%

rem ---------------------------------------------------------------------------
rem vswhere で MSBuild を探すサブルーチン。
rem for /f 内の引用符の扱いが壊れやすいため、エンジン側の build.bat の形を崩さないこと
rem ---------------------------------------------------------------------------
:find_vs2022
for /f "usebackq tokens=*" %%i in (`^""%VSWHERE%" -version "[17.0,18.0)" -products * -requires Microsoft.Component.MSBuild -property installationPath^"`) do (
    if exist "%%i\MSBuild\Current\Bin\MSBuild.exe" set "MSBUILD=%%i\MSBuild\Current\Bin\MSBuild.exe"
)
goto :eof

:find_latest
for /f "usebackq tokens=*" %%i in (`^""%VSWHERE%" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath^"`) do (
    if exist "%%i\MSBuild\Current\Bin\MSBuild.exe" set "MSBUILD=%%i\MSBuild\Current\Bin\MSBuild.exe"
)
goto :eof
