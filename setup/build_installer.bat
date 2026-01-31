@echo off
echo ========================================
echo Building Source67 Installer
echo ========================================
echo.

REM Check if NSIS is installed
where makensis >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] NSIS (makensis) not found in PATH.
    echo Please install NSIS from https://nsis.sourceforge.io/
    pause
    exit /b 1
)

REM Check if Source67.exe exists in Release build directory
if not exist "..\cmake-build-release\Source67.exe" (
    echo [ERROR] Source67.exe not found in cmake-build-release directory.
    echo.
    echo Please build the RELEASE version of the engine first:
    echo   1. Run: cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
    echo   2. Run: cmake --build cmake-build-release --config Release
    echo.
    echo Or use the build script:
    echo   build.bat Release
    echo.
    echo NOTE: Always use Release builds for distribution!
    echo Debug builds are larger, slower, and may not run on other systems.
    echo.
    pause
    exit /b 1
)

echo [OK] Source67.exe found in Release build directory
echo.

REM Build the installer
echo Building installer with NSIS...
makensis Source67.nsi
if %errorlevel% equ 0 (
    echo.
    echo ========================================
    echo [SUCCESS] Source67_Setup.exe created!
    echo ========================================
    echo.
    echo Installer location: setup\Source67_Setup.exe
    echo.
    echo You can now distribute this installer to users.
    echo.
) else (
    echo.
    echo [ERROR] Failed to build installer.
    echo Check the output above for errors.
    echo.
)

pause
