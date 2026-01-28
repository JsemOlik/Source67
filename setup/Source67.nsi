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
  File "..\cmake-build-debug\Source67.exe"
  
  ; Assets
  SetOutPath "$INSTDIR\assets"
  File /r "..\assets\*.*"
  
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
  Delete "$INSTDIR\uninstall.exe"
  RMDir /r "$INSTDIR\assets"
  
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
