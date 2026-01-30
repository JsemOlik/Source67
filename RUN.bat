@echo off
REM Source67 Engine - Quick Run Script
REM Automatically finds and runs the built engine executable

echo.
echo =========================================
echo Source67 Engine - Quick Run
echo =========================================
echo.

REM Try Debug build first (more common during development)
if exist cmake-build-debug\Debug\Source67.exe (
    echo Running Debug build...
    echo Location: cmake-build-debug\Debug\Source67.exe
    echo.
    echo =========================================
    echo.
    cmake-build-debug\Debug\Source67.exe
    goto :eof
)

if exist cmake-build-debug\Source67.exe (
    echo Running Debug build...
    echo Location: cmake-build-debug\Source67.exe
    echo.
    echo =========================================
    echo.
    cmake-build-debug\Source67.exe
    goto :eof
)

REM Try Release build
if exist cmake-build-release\Release\Source67.exe (
    echo Running Release build...
    echo Location: cmake-build-release\Release\Source67.exe
    echo.
    echo =========================================
    echo.
    cmake-build-release\Release\Source67.exe
    goto :eof
)

if exist cmake-build-release\Source67.exe (
    echo Running Release build...
    echo Location: cmake-build-release\Source67.exe
    echo.
    echo =========================================
    echo.
    cmake-build-release\Source67.exe
    goto :eof
)

REM Not found
echo ERROR: Source67.exe not found!
echo.
echo The engine has not been built yet.
echo Please run the build script first:
echo.
echo   build.bat Debug all
echo.
echo Then try running this script again.
echo.
echo For detailed instructions, see QUICK_START_GUIDE.md
echo.
echo =========================================
echo Press any key to exit...
echo =========================================
pause > nul
exit /b 1
