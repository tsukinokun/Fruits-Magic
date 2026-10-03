@echo off
rem ---------------------------------------------------------------------------
rem Build FruitMagic (including the engine).
rem   build.bat            ... Debug build
rem   build.bat Release    ... Release build
rem
rem Prints nothing on success and only error lines on failure.
rem Do not call MSBuild directly (it prints thousands of lines without the quiet flags).
rem Based on External\TsukinoEngine\build.bat (see it for why MSBuild is located this way).
rem
rem NOTE: keep this file ASCII only. cmd.exe mis-parses multibyte (Japanese) text
rem       in batch files depending on the console code page.
rem ---------------------------------------------------------------------------
setlocal

set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Debug"

set "ROOT=%~dp0"
set "SLN=%ROOT%.build\FruitMagic.sln"
set "PREMAKE=%ROOT%External\TsukinoEngine\vendor\premake5.exe"

rem ---------------------------------------------------------------------------
rem Locate MSBuild (VS2022 -> latest VS -> hard-coded path)
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
rem Generate project files.
rem Runs premake every time so that added/removed source files are picked up.
rem The engine's premake5.lua prints deprecation warnings to stderr, so both
rem streams are discarded on success; on failure premake is run again to show why.
rem ---------------------------------------------------------------------------
pushd "%ROOT%"
"%PREMAKE%" vs2022 >nul 2>&1
if errorlevel 1 (
    "%PREMAKE%" vs2022
    popd
    echo build.bat: premake generation failed.
    exit /b 1
)
popd

rem NuGet restore. premake emits the old packages.config format, so
rem -t:restore alone is not enough and -p:RestorePackagesConfig=true is required.
if not exist "%ROOT%.build\packages" (
    "%MSBUILD%" "%SLN%" -t:restore -p:RestorePackagesConfig=true -p:Configuration=%CONFIG% -p:Platform=x64 -nologo -v:q -clp:"ErrorsOnly;NoSummary"
    if errorlevel 1 exit /b 1
)

"%MSBUILD%" "%SLN%" /p:Configuration=%CONFIG% /p:Platform=x64 /m /nologo /v:q /clp:"ErrorsOnly;NoSummary"
exit /b %ERRORLEVEL%

rem ---------------------------------------------------------------------------
rem Find MSBuild with vswhere.
rem Quoting inside for /f is fragile; keep the same form as the engine's build.bat.
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
