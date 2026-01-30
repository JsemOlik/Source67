; Source67 Engine Installer Script
; Uses Modern UI 2

!include "MUI2.nsh"

;--------------------------------
; General Attributes

Name "Source67 Engine"
OutFile "Source67_Setup.exe"
InstallDir "$PROGRAMFILES64\Source67"
InstallDirRegKey HKCU "Software\Source67" "InstallDir"
RequestExecutionLevel admin

;--------------------------------
; Interface Settings

!define MUI_ABORTWARNING
!define MUI_ICON "..\assets\engine\app_icon.ico"
!define MUI_UNICON "..\assets\engine\app_icon.ico"
!define MUI_COMPONENTSPAGE_SMALLDESC

;--------------------------------
; Pages

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "License.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

;--------------------------------
; Languages

!insertmacro MUI_LANGUAGE "English"

;--------------------------------
; Installer Sections

Section "Main Engine (Required)" SecMain
  SectionIn RO ; Read-only, user cannot uncheck

  SetOutPath "$INSTDIR"
  
  ; Core Files
  ; Compile-time check for Source67.exe (validation in build_installer.bat ensures it exists)
  !if /FileExists "..\cmake-build-debug\Debug\Source67.exe"
    File "..\cmake-build-debug\Debug\Source67.exe"
  !else if /FileExists "..\cmake-build-debug\Release\Source67.exe"
    File "..\cmake-build-debug\Release\Source67.exe"
  !else if /FileExists "..\cmake-build-debug\Source67.exe"
    File "..\cmake-build-debug\Source67.exe"
  !else
    !error "Source67.exe not found! Run build_installer.bat which validates the build exists."
  !endif
  
  ; Copy any DLL files from Debug/Release folders
  IfFileExists "..\cmake-build-debug\Debug\*.dll" 0 +2
    File /nonfatal "..\cmake-build-debug\Debug\*.dll"
  IfFileExists "..\cmake-build-debug\Release\*.dll" 0 +2
    File /nonfatal "..\cmake-build-debug\Release\*.dll"
  IfFileExists "..\cmake-build-debug\*.dll" 0 +2
    File /nonfatal "..\cmake-build-debug\*.dll"
  
  ; Assets - preserve folder structure
  SetOutPath "$INSTDIR"
  File /r /x .DS_Store "..\assets"
  
  ; Write the installation path into the registry
  WriteRegStr HKCU "Software\Source67" "InstallDir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Source67" "DisplayName" "Source67 Engine"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Source67" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Source67" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Source67" "NoRepair" 1
  WriteUninstaller "uninstall.exe"

  ; Common Shortcuts (Start Menu)
  CreateDirectory "$SMPROGRAMS\Source67"
  CreateShortcut "$SMPROGRAMS\Source67\Source67 Engine.lnk" "$INSTDIR\Source67.exe" "" "$INSTDIR\assets\engine\app_icon.ico" 0
  CreateShortcut "$SMPROGRAMS\Source67\Uninstall Source67.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
SectionEnd

Section "Desktop Shortcut" SecDesktop
  CreateShortcut "$DESKTOP\Source67 Engine.lnk" "$INSTDIR\Source67.exe" "" "$INSTDIR\assets\engine\app_icon.ico" 0
SectionEnd

Section "CMake Build Tools (Recommended)" SecCMake
  ; CMake is required for building Game.dll from C++ source
  ; This bundles a portable version of CMake with the engine
  
  SetOutPath "$INSTDIR\Tools\cmake"
  
  ; If you have CMake binaries in setup\cmake folder, include them here
  ; File /r "..\setup\cmake\*.*"
  
  ; Alternative: Install CMake from official installer
  ; For now, we'll write a helper script that guides users to install CMake
  
  SetOutPath "$INSTDIR\Tools"
  
  ; Create a helper batch file for CMake installation
  FileOpen $0 "$INSTDIR\Tools\install_cmake.bat" w
  FileWrite $0 "@echo off$\r$\n"
  FileWrite $0 "echo ========================================$\r$\n"
  FileWrite $0 "echo CMake Installation Helper$\r$\n"
  FileWrite $0 "echo ========================================$\r$\n"
  FileWrite $0 "echo.$\r$\n"
  FileWrite $0 "echo CMake is required for building Game.dll from C++ source.$\r$\n"
  FileWrite $0 "echo.$\r$\n"
  FileWrite $0 "echo Checking if CMake is installed...$\r$\n"
  FileWrite $0 "where cmake >nul 2>nul$\r$\n"
  FileWrite $0 "if %errorlevel% equ 0 ($\r$\n"
  FileWrite $0 "    echo [SUCCESS] CMake is already installed!$\r$\n"
  FileWrite $0 "    cmake --version$\r$\n"
  FileWrite $0 "    echo.$\r$\n"
  FileWrite $0 "    echo You're ready to build games with Source67!$\r$\n"
  FileWrite $0 "    pause$\r$\n"
  FileWrite $0 "    exit /b 0$\r$\n"
  FileWrite $0 ")$\r$\n"
  FileWrite $0 "echo.$\r$\n"
  FileWrite $0 "echo [INFO] CMake is not installed.$\r$\n"
  FileWrite $0 "echo.$\r$\n"
  FileWrite $0 "echo Opening CMake download page...$\r$\n"
  FileWrite $0 "start https://cmake.org/download/$\r$\n"
  FileWrite $0 "echo.$\r$\n"
  FileWrite $0 "echo Download CMake Windows x64 Installer (MSI)$\r$\n"
  FileWrite $0 "echo Install it and make sure to add CMake to PATH.$\r$\n"
  FileWrite $0 "echo.$\r$\n"
  FileWrite $0 "pause$\r$\n"
  FileClose $0
  
  ; Create a README about CMake
  FileOpen $0 "$INSTDIR\Tools\CMAKE_INFO.txt" w
  FileWrite $0 "========================================$\r$\n"
  FileWrite $0 "CMake Build Tools Information$\r$\n"
  FileWrite $0 "========================================$\r$\n"
  FileWrite $0 "$\r$\n"
  FileWrite $0 "CMake is required for building Game.dll from your C++ game code.$\r$\n"
  FileWrite $0 "$\r$\n"
  FileWrite $0 "INSTALLATION OPTIONS:$\r$\n"
  FileWrite $0 "$\r$\n"
  FileWrite $0 "Option 1: Auto-Install (Recommended)$\r$\n"
  FileWrite $0 "  - Run install_cmake.bat in this folder$\r$\n"
  FileWrite $0 "  - Follow the instructions$\r$\n"
  FileWrite $0 "$\r$\n"
  FileWrite $0 "Option 2: Manual Download$\r$\n"
  FileWrite $0 "  - Visit: https://cmake.org/download/$\r$\n"
  FileWrite $0 "  - Download: cmake-X.XX.X-windows-x86_64.msi$\r$\n"
  FileWrite $0 "  - Install and add to PATH$\r$\n"
  FileWrite $0 "$\r$\n"
  FileWrite $0 "Option 3: Package Manager$\r$\n"
  FileWrite $0 "  - Using Chocolatey: choco install cmake$\r$\n"
  FileWrite $0 "  - Using winget: winget install Kitware.CMake$\r$\n"
  FileWrite $0 "$\r$\n"
  FileWrite $0 "VERIFICATION:$\r$\n"
  FileWrite $0 "  Open a new command prompt and run: cmake --version$\r$\n"
  FileWrite $0 "$\r$\n"
  FileWrite $0 "If CMake is installed, you can build games from the Source67 editor!$\r$\n"
  FileWrite $0 "  Building > Build Game (F7)$\r$\n"
  FileWrite $0 "========================================$\r$\n"
  FileClose $0
  
SectionEnd

Section "File Associations (.source, .s67)" SecAssoc
  ; .source (Projects)
  WriteRegStr HKCR ".source" "" "Source67.Project"
  WriteRegStr HKCR "Source67.Project" "" "Source67 Project"
  WriteRegStr HKCR "Source67.Project\DefaultIcon" "" "$INSTDIR\assets\engine\app_icon.ico"
  WriteRegStr HKCR "Source67.Project\shell\open\command" "" '"$INSTDIR\Source67.exe" "%1"'

  ; .s67 (Levels)
  WriteRegStr HKCR ".s67" "" "Source67.Level"
  WriteRegStr HKCR "Source67.Level" "" "Source67 Level"
  WriteRegStr HKCR "Source67.Level\DefaultIcon" "" "$INSTDIR\assets\engine\app_icon.ico"
  WriteRegStr HKCR "Source67.Level\shell\open\command" "" '"$INSTDIR\Source67.exe" "%1"'
  
  ; Notify shell of changes
  System::Call 'Shell32::SHChangeNotify(i 0x08000000, i 0, i 0, i 0)'
SectionEnd

;--------------------------------
; Descriptions

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecMain} "The core engine files and assets."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} "Creates a shortcut to the engine on your desktop."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecCMake} "CMake build tools helper. Required for building Game.dll from C++ source code. Includes installation guide and helper scripts."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecAssoc} "Associates .source and .s67 files with the Source67 engine."
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
; Uninstaller Section

Section "Uninstall"

  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Source67"
  DeleteRegKey HKCU "Software\Source67"
  
  ; Remove File Associations (if they exist)
  DeleteRegKey HKCR ".source"
  DeleteRegKey HKCR "Source67.Project"
  DeleteRegKey HKCR ".s67"
  DeleteRegKey HKCR "Source67.Level"

  ; Remove files and uninstaller
  Delete "$INSTDIR\Source67.exe"
  Delete "$INSTDIR\*.dll"
  Delete "$INSTDIR\uninstall.exe"
  RMDir /r "$INSTDIR\assets"
  RMDir /r "$INSTDIR\Tools"
  
  ; Remove shortcuts
  Delete "$SMPROGRAMS\Source67\Source67 Engine.lnk"
  Delete "$SMPROGRAMS\Source67\Uninstall Source67.lnk"
  RMDir "$SMPROGRAMS\Source67"
  Delete "$DESKTOP\Source67 Engine.lnk"

  ; Remove directories
  RMDir "$INSTDIR"

  ; Notify shell of changes
  System::Call 'Shell32::SHChangeNotify(i 0x08000000, i 0, i 0, i 0)'

SectionEnd
