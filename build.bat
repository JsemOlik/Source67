@echo off
REM Source67 Engine - Hybrid Build System Build Script (Windows)
REM Builds engine, game DLL, and asset pack

setlocal enabledelayedexpansion

set BUILD_TYPE=%1
set TARGET=%2

if "%BUILD_TYPE%"=="" set BUILD_TYPE=Release
if "%TARGET%"=="" set TARGET=all

echo =========================================
echo Source67 Engine - Hybrid Build System
echo =========================================
echo Build Type: %BUILD_TYPE%
echo Target: %TARGET%
echo.

REM Step 1: Build Game.dll
if "%TARGET%"=="all" goto build_game
if "%TARGET%"=="game" goto build_game
goto skip_game

:build_game
echo [1/3] Building Game.dll...
cd game
cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -B build
cmake --build build --config %BUILD_TYPE%
echo * Game.dll compiled
cd ..
echo.

:skip_game

REM Step 2: Build Asset Packer Tool
if "%TARGET%"=="all" goto build_tools
if "%TARGET%"=="tools" goto build_tools
goto skip_tools

:build_tools
echo [2/3] Building asset packer tool...
cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DBUILD_ASSET_PACKER=ON -B cmake-build-tools
cmake --build cmake-build-tools --target asset_packer --config %BUILD_TYPE%
echo * Asset packer built
echo.

:skip_tools

REM Step 3: Pack Assets
if "%TARGET%"=="all" goto pack_assets
if "%TARGET%"=="assets" goto pack_assets
goto skip_assets

:pack_assets
echo [3/3] Packing assets (GameAssets.apak)...
if exist cmake-build-tools\%BUILD_TYPE%\asset_packer.exe (
    cmake-build-tools\%BUILD_TYPE%\asset_packer.exe -i assets/ -o GameAssets.apak -v --include-lua
) else if exist cmake-build-tools\asset_packer.exe (
    cmake-build-tools\asset_packer.exe -i assets/ -o GameAssets.apak -v --include-lua
) else (
    echo Warning: Asset packer not found, skipping asset packing
)
echo * GameAssets.apak created
echo.

:skip_assets

REM Step 4: Build Engine
if "%TARGET%"=="all" goto build_engine
if "%TARGET%"=="engine" goto build_engine
goto skip_engine

:build_engine
echo [4/4] Building Source67 engine...

if "%BUILD_TYPE%"=="Release" (
    cmake -DCMAKE_BUILD_TYPE=Release -DSTANDALONE_MODE=ON -DEDITOR_MODE=OFF -B cmake-build-release
    cmake --build cmake-build-release --config Release
    echo * Source67.exe built (Standalone mode)
) else (
    cmake -DCMAKE_BUILD_TYPE=Debug -DSTANDALONE_MODE=OFF -DEDITOR_MODE=ON -B cmake-build-debug
    cmake --build cmake-build-debug --config Debug
    echo * Source67.exe built (Editor mode)
)
echo.

:skip_engine

echo =========================================
echo Build Complete!
echo =========================================
echo.
echo Distribution package contents:
if "%BUILD_TYPE%"=="Release" (
    echo   - cmake-build-release\Source67.exe
    echo   - GameAssets.apak
    echo   - game\build\Release\Game.dll
    echo.
    echo To run: cmake-build-release\Source67.exe
) else (
    echo   - cmake-build-debug\Source67.exe
    echo   - GameAssets.apak
    echo   - game\build\Debug\Game.dll
    echo.
    echo To run: cmake-build-debug\Source67.exe
)

endlocal
