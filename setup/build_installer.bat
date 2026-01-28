@echo off
echo Building Source67 Installer...

where makensis >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] NSIS (makensis) not found in PATH.
    echo Please install NSIS from https://nsis.sourceforge.io/
    pause
    exit /b 1
)

makensis Source67.nsi
if %errorlevel% equ 0 (
    echo [SUCCESS] Source67_Setup.exe created in setup folder.
) else (
    echo [ERROR] Failed to build installer.
)

pause
