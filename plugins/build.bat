@echo off
REM ****************************************************************************
REM T-150 UAS Plugins - Windows Build Script
REM
REM This script builds the T-150 custom plugins against the installed AFSIM.
REM
REM Usage: build.bat [Release|Debug] [VS2019|VS2022]
REM
REM Prerequisites:
REM - AFSIM 2.2505 installed (default: C:\Program Files\AFSIM\afsim2.2505)
REM - CMake 3.16 or later
REM - Visual Studio 2019 or 2022
REM
REM Copyright 2025 - Project 005 T-150 UAS Simulation
REM ****************************************************************************

setlocal enabledelayedexpansion

REM Configuration
set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Release

set VS_VERSION=%2
if "%VS_VERSION%"=="" set VS_VERSION=VS2022

set SCRIPT_DIR=%~dp0
set BUILD_DIR=%SCRIPT_DIR%build

REM Determine Visual Studio generator
if "%VS_VERSION%"=="VS2019" (
    set CMAKE_GENERATOR=Visual Studio 16 2019
) else if "%VS_VERSION%"=="VS2022" (
    set CMAKE_GENERATOR=Visual Studio 17 2022
) else (
    echo ERROR: Unknown Visual Studio version: %VS_VERSION%
    echo Usage: build.bat [Release^|Debug] [VS2019^|VS2022]
    exit /b 1
)

REM Find AFSIM installation
set AFSIM_DIR=
if exist "C:\Program Files\AFSIM\afsim2.2505" (
    set AFSIM_DIR=C:\Program Files\AFSIM\afsim2.2505
) else if exist "C:\AFSIM\afsim2.2505" (
    set AFSIM_DIR=C:\AFSIM\afsim2.2505
) else if defined AFSIM_ROOT (
    set AFSIM_DIR=%AFSIM_ROOT%
)

echo ==============================================
echo T-150 UAS Plugins Windows Build
echo ==============================================
echo AFSIM Directory: %AFSIM_DIR%
echo Build Type: %BUILD_TYPE%
echo VS Version: %VS_VERSION%
echo Generator: %CMAKE_GENERATOR%
echo Build Directory: %BUILD_DIR%
echo.

REM Check AFSIM installation
if "%AFSIM_DIR%"=="" (
    echo ERROR: AFSIM not found. Please set AFSIM_ROOT environment variable
    echo or install AFSIM to C:\Program Files\AFSIM\afsim2.2505
    exit /b 1
)

if not exist "%AFSIM_DIR%\include" (
    echo ERROR: AFSIM include directory not found at %AFSIM_DIR%\include
    exit /b 1
)

REM Create build directory
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd /d "%BUILD_DIR%"

REM Configure CMake
echo Configuring CMake...
cmake -G "%CMAKE_GENERATOR%" -A x64 ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DAFSIM_ROOT="%AFSIM_DIR%" ^
    "%SCRIPT_DIR%"

if errorlevel 1 (
    echo ERROR: CMake configuration failed
    exit /b 1
)

REM Build
echo.
echo Building...
cmake --build . --config %BUILD_TYPE% --parallel

if errorlevel 1 (
    echo ERROR: Build failed
    exit /b 1
)

REM Copy built plugins to top-level lib\ directory
echo.
echo Copying plugins to lib\ directory...
set PROJECT_LIB=%SCRIPT_DIR%..\lib
if not exist "%PROJECT_LIB%" mkdir "%PROJECT_LIB%"

for %%p in (t150_battery t150_vrs t150_propulsion t150_slung_load) do (
    if exist "%BUILD_DIR%\lib\%%p.dll" (
        copy /Y "%BUILD_DIR%\lib\%%p.dll" "%PROJECT_LIB%\%%p.dll" >nul
        echo   Copied %%p.dll
    )
    if exist "%BUILD_DIR%\lib\%BUILD_TYPE%\%%p.dll" (
        copy /Y "%BUILD_DIR%\lib\%BUILD_TYPE%\%%p.dll" "%PROJECT_LIB%\%%p.dll" >nul
        echo   Copied %%p.dll ^(from %BUILD_TYPE%^)
    )
)

echo.
echo ==============================================
echo Build complete!
echo ==============================================
echo.
echo Plugin DLLs built:
dir /b "%BUILD_DIR%\lib\*.dll" 2>nul
dir /b "%BUILD_DIR%\lib\%BUILD_TYPE%\*.dll" 2>nul
echo.
echo To install the plugins to AFSIM, run:
echo   install_plugins.bat
echo.

endlocal
