@echo off
REM Source67 Engine - Hybrid Build System Build Script (Windows)
REM Builds engine, game DLL, and asset pack
REM 
REM Usage: build.bat [BuildType] [Target]
REM   BuildType: Debug (default) or Release
REM   Target: all (default), game, tools, assets, or engine
REM
REM Examples:
REM   build.bat              - Builds everything in Debug mode
REM   build.bat Debug all    - Builds everything in Debug mode
REM   build.bat Release all  - Builds everything in Release mode
REM   build.bat Debug game   - Builds only Game.dll in Debug mode

setlocal enabledelayedexpansion

REM Set error flag
set BUILD_ERRORS=0

set BUILD_TYPE=%1
set TARGET=%2

if "%BUILD_TYPE%"=="" set BUILD_TYPE=Debug
if "%TARGET%"=="" set TARGET=all

echo.
echo =========================================
echo Source67 Engine - Hybrid Build System
echo =========================================
echo Build Type: %BUILD_TYPE%
echo Target: %TARGET%
echo Working Directory: %CD%
echo.
echo Starting build at %TIME%
echo =========================================
echo.

REM Step 1: Build Game.dll
if "%TARGET%"=="all" goto build_game
if "%TARGET%"=="game" goto build_game
goto skip_game

:build_game
echo [1/4] Building Game.dll...
echo ----------------------------------------
cd game
cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -B build
if errorlevel 1 (
    echo ERROR: CMake configuration failed for Game.dll
    set BUILD_ERRORS=1
    cd ..
    goto error_handler
)
cmake --build build --config %BUILD_TYPE%
if errorlevel 1 (
    echo ERROR: Game.dll compilation failed
    set BUILD_ERRORS=1
    cd ..
    goto error_handler
)
echo SUCCESS: Game.dll compiled
cd ..
echo.

:skip_game

REM Step 2: Build Asset Packer Tool
if "%TARGET%"=="all" goto build_tools
if "%TARGET%"=="tools" goto build_tools
goto skip_tools

:build_tools
echo [2/4] Building asset packer tool...
echo ----------------------------------------
cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DBUILD_ASSET_PACKER=ON -B cmake-build-tools
if errorlevel 1 (
    echo ERROR: CMake configuration failed for asset packer
    set BUILD_ERRORS=1
    goto error_handler
)
cmake --build cmake-build-tools --target asset_packer --config %BUILD_TYPE%
if errorlevel 1 (
    echo ERROR: Asset packer compilation failed
    set BUILD_ERRORS=1
    goto error_handler
)
echo SUCCESS: Asset packer built
echo.

:skip_tools

REM Step 3: Pack Assets
if "%TARGET%"=="all" goto pack_assets
if "%TARGET%"=="assets" goto pack_assets
goto skip_assets

:pack_assets
echo [3/4] Packing assets (GameAssets.apak)...
echo ----------------------------------------
if exist cmake-build-tools\%BUILD_TYPE%\asset_packer.exe (
    echo Using asset packer: cmake-build-tools\%BUILD_TYPE%\asset_packer.exe
    cmake-build-tools\%BUILD_TYPE%\asset_packer.exe -i assets/ -o GameAssets.apak -v --include-lua
    if errorlevel 1 (
        echo ERROR: Asset packing failed
        set BUILD_ERRORS=1
        goto error_handler
    )
) else if exist cmake-build-tools\asset_packer.exe (
    echo Using asset packer: cmake-build-tools\asset_packer.exe
    cmake-build-tools\asset_packer.exe -i assets/ -o GameAssets.apak -v --include-lua
    if errorlevel 1 (
        echo ERROR: Asset packing failed
        set BUILD_ERRORS=1
        goto error_handler
    )
) else (
    echo WARNING: Asset packer not found, skipping asset packing
    echo   Build the asset packer first with: build.bat %BUILD_TYPE% tools
)

if exist GameAssets.apak (
    echo SUCCESS: GameAssets.apak created
    for %%F in (GameAssets.apak) do echo   Size: %%~zF bytes
) else (
    echo WARNING: GameAssets.apak was not created
)
echo.

:skip_assets

REM Step 4: Build Engine
if "%TARGET%"=="all" goto build_engine
if "%TARGET%"=="engine" goto build_engine
goto skip_engine

:build_engine
echo [4/4] Building Source67 engine...
echo ----------------------------------------

if "%BUILD_TYPE%"=="Release" (
    cmake -DCMAKE_BUILD_TYPE=Release -DSTANDALONE_MODE=ON -DEDITOR_MODE=OFF -B cmake-build-release
    if errorlevel 1 (
        echo ERROR: CMake configuration failed for Source67 engine
        set BUILD_ERRORS=1
        goto error_handler
    )
    cmake --build cmake-build-release --config Release
    if errorlevel 1 (
        echo ERROR: Source67 engine compilation failed
        set BUILD_ERRORS=1
        goto error_handler
    )
    echo SUCCESS: Source67.exe built (Standalone mode)
) else (
    cmake -DCMAKE_BUILD_TYPE=Debug -DSTANDALONE_MODE=OFF -DEDITOR_MODE=ON -B cmake-build-debug
    if errorlevel 1 (
        echo ERROR: CMake configuration failed for Source67 engine
        set BUILD_ERRORS=1
        goto error_handler
    )
    cmake --build cmake-build-debug --config Debug
    if errorlevel 1 (
        echo ERROR: Source67 engine compilation failed
        set BUILD_ERRORS=1
        goto error_handler
    )
    echo SUCCESS: Source67.exe built (Editor mode)
)
echo.

:skip_engine


echo =========================================
echo Build Complete!
echo =========================================
echo Build finished at %TIME%
echo.
echo Distribution package contents:
if "%BUILD_TYPE%"=="Release" (
    echo   - cmake-build-release\Release\Source67.exe
    if exist cmake-build-release\Release\Source67.exe (
        for %%F in (cmake-build-release\Release\Source67.exe) do echo     Size: %%~zF bytes
    ) else (
        echo     WARNING: Source67.exe not found!
    )
    echo   - GameAssets.apak
    if exist GameAssets.apak (
        for %%F in (GameAssets.apak) do echo     Size: %%~zF bytes
    ) else (
        echo     WARNING: GameAssets.apak not found!
    )
    echo   - game\build\Release\Game.dll
    if exist game\build\Release\Game.dll (
        for %%F in (game\build\Release\Game.dll) do echo     Size: %%~zF bytes
    ) else (
        echo     WARNING: Game.dll not found!
    )
    echo.
    echo To run: cmake-build-release\Release\Source67.exe
) else (
    echo   - cmake-build-debug\Debug\Source67.exe
    if exist cmake-build-debug\Debug\Source67.exe (
        for %%F in (cmake-build-debug\Debug\Source67.exe) do echo     Size: %%~zF bytes
    ) else (
        echo     WARNING: Source67.exe not found!
    )
    echo   - GameAssets.apak
    if exist GameAssets.apak (
        for %%F in (GameAssets.apak) do echo     Size: %%~zF bytes
    ) else (
        echo     WARNING: GameAssets.apak not found!
    )
    echo   - game\build\Debug\Game.dll
    if exist game\build\Debug\Game.dll (
        for %%F in (game\build\Debug\Game.dll) do echo     Size: %%~zF bytes
    ) else (
        echo     WARNING: Game.dll not found!
    )
    echo.
    echo To run: cmake-build-debug\Debug\Source67.exe
)
echo.
echo =========================================
echo Press any key to exit...
echo =========================================
pause > nul
goto :eof

:error_handler
echo.
echo =========================================
echo BUILD FAILED!
echo =========================================
echo Build failed at %TIME%
echo.
echo One or more build steps encountered errors.
echo Please review the error messages above.
echo.
echo Common issues:
echo   - CMake not installed or not in PATH
echo   - C++ compiler not found (install Visual Studio)
echo   - Missing dependencies
echo   - Running from wrong directory (should be Source67 root)
echo.
echo For help, see QUICK_START_GUIDE.md
echo.
echo =========================================
echo Press any key to exit...
echo =========================================
pause > nul
exit /b 1

endlocal
