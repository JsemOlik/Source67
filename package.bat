@echo off
REM ========================================
REM Source67 - Distribution Package Script
REM ========================================
REM
REM This script creates a distribution-ready package of your game
REM Usage: package.bat [GameName] [Version]
REM Example: package.bat MyGame 1.0.0
REM

setlocal enabledelayedexpansion

REM Get parameters or use defaults
set GAME_NAME=%1
set VERSION=%2

if "%GAME_NAME%"=="" set GAME_NAME=MyGame
if "%VERSION%"=="" set VERSION=1.0.0

set OUTPUT_DIR=%GAME_NAME%_v%VERSION%

echo.
echo ========================================
echo Creating Distribution Package
echo ========================================
echo Game: %GAME_NAME%
echo Version: %VERSION%
echo Output: %OUTPUT_DIR%
echo.

REM Step 1: Build Release version
echo [1/6] Building Release version...
echo ----------------------------------------
call build.bat Release all
if errorlevel 1 (
    echo ERROR: Build failed!
    echo Please fix build errors and try again.
    pause
    exit /b 1
)
echo SUCCESS: Release build complete
echo.

REM Step 2: Create distribution folder
echo [2/6] Creating distribution folder...
echo ----------------------------------------
if exist "%OUTPUT_DIR%" (
    echo Removing old package...
    rmdir /s /q "%OUTPUT_DIR%"
)
mkdir "%OUTPUT_DIR%"
echo SUCCESS: Folder created
echo.

REM Step 3: Copy game files
echo [3/6] Copying game files...
echo ----------------------------------------

REM Copy and rename engine executable
if exist cmake-build-release\Release\Source67.exe (
    copy cmake-build-release\Release\Source67.exe "%OUTPUT_DIR%\%GAME_NAME%.exe"
    echo Copied: %GAME_NAME%.exe
) else if exist cmake-build-release\Source67.exe (
    copy cmake-build-release\Source67.exe "%OUTPUT_DIR%\%GAME_NAME%.exe"
    echo Copied: %GAME_NAME%.exe
) else (
    echo ERROR: Source67.exe not found!
    echo Build the engine first with: build.bat Release all
    pause
    exit /b 1
)

REM Copy Game.dll
if exist game\build\Release\Game.dll (
    copy game\build\Release\Game.dll "%OUTPUT_DIR%\Game.dll"
    echo Copied: Game.dll
) else if exist game\build\Game.dll (
    copy game\build\Game.dll "%OUTPUT_DIR%\Game.dll"
    echo Copied: Game.dll
) else (
    echo ERROR: Game.dll not found!
    echo Build the game first with: build.bat Release game
    pause
    exit /b 1
)

REM Copy GameAssets.apak
if exist GameAssets.apak (
    copy GameAssets.apak "%OUTPUT_DIR%\GameAssets.apak"
    echo Copied: GameAssets.apak
) else (
    echo ERROR: GameAssets.apak not found!
    echo Build assets first with: build.bat Release assets
    pause
    exit /b 1
)

echo SUCCESS: All game files copied
echo.

REM Step 4: Create README
echo [4/6] Creating README.txt...
echo ----------------------------------------
(
echo ========================================
echo     %GAME_NAME% v%VERSION%
echo ========================================
echo.
echo HOW TO PLAY:
echo 1. Double-click %GAME_NAME%.exe to start
echo 2. Use WASD to move, Mouse to look
echo 3. Press ESC to pause/quit
echo.
echo CONTROLS:
echo - W/A/S/D: Move
echo - Mouse: Look around
echo - Space: Jump
echo - E: Interact
echo - ~: Open developer console
echo.
echo SYSTEM REQUIREMENTS:
echo - Windows 10/11 (64-bit)
echo - OpenGL 4.5 compatible GPU
echo - 4GB RAM minimum
echo - 500MB free disk space
echo.
echo VERSION: %VERSION%
echo CREATED WITH: Source67 Game Engine
echo.
echo For support or feedback, visit:
echo [Your website or contact info here]
echo.
echo Thank you for playing!
echo ========================================
) > "%OUTPUT_DIR%\README.txt"
echo SUCCESS: README.txt created
echo.

REM Step 5: Show package info
echo [5/6] Package contents:
echo ----------------------------------------
dir "%OUTPUT_DIR%" /b
echo.
for %%F in ("%OUTPUT_DIR%\*.*") do echo   %%~nxF - %%~zF bytes
echo.

REM Step 6: Create ZIP archive
echo [6/6] Creating ZIP archive...
echo ----------------------------------------
if exist "%OUTPUT_DIR%.zip" del "%OUTPUT_DIR%.zip"

powershell -Command "Compress-Archive -Path '%OUTPUT_DIR%' -DestinationPath '%OUTPUT_DIR%.zip' -CompressionLevel Optimal" 2>nul
if errorlevel 1 (
    echo WARNING: ZIP creation failed
    echo Package folder created but not zipped
    echo You can manually zip the folder: %OUTPUT_DIR%
) else (
    echo SUCCESS: %OUTPUT_DIR%.zip created
    for %%F in ("%OUTPUT_DIR%.zip") do echo   Size: %%~zF bytes
)
echo.

echo ========================================
echo Distribution Package Complete!
echo ========================================
echo.
echo Package folder: %OUTPUT_DIR%\
echo.
if exist "%OUTPUT_DIR%.zip" (
    echo ZIP file: %OUTPUT_DIR%.zip
    echo.
    echo Your game is ready to distribute!
    echo.
    echo Next steps:
    echo 1. Test the package on a clean system
    echo 2. Upload to Steam, itch.io, or your distribution platform
    echo 3. Share with players!
) else (
    echo Folder created (ZIP failed)
    echo Please manually create a ZIP file from: %OUTPUT_DIR%\
)
echo.
echo For more information, see DISTRIBUTION.md
echo ========================================
echo.

pause
