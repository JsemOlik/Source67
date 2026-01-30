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

REM Check if Source67.exe exists in build directory
set EXE_FOUND=0
if exist "..\cmake-build-debug\Debug\Source67.exe" set EXE_FOUND=1
if exist "..\cmake-build-debug\Release\Source67.exe" set EXE_FOUND=1
if exist "..\cmake-build-debug\Source67.exe" set EXE_FOUND=1

if %EXE_FOUND%==0 (
    echo [ERROR] Source67.exe not found in cmake-build-debug directory.
    echo.
    echo Please build the engine first:
    echo   1. Open cmake-build-debug folder (or create it)
    echo   2. Run: cmake -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
    echo   3. Run: cmake --build cmake-build-debug --config Debug
    echo.
    echo Or use CLion/Visual Studio to build the project.
    echo.
    pause
    exit /b 1
)

echo [OK] Source67.exe found in build directory
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
