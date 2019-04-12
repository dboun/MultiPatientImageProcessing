; The name of the installer
Name "MLL_v0.3_installer"

; The file to write
OutFile "MLL_v0.3_installer.exe"

; The default installation directory
InstallDir "C:\MLL\0.3.0"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
; InstallDirRegKey HKLM "Software\NSIS_Example2" "Install_Dir"

; ; Request application privileges for Windows Vista
; RequestExecutionLevel admin
RequestExecutionLevel user

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; For run on finish page

!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_FUNCTION RunMyApp
!define MUI_FINISHPAGE_RUN_TEXT "Run MLL"

Function RunMyApp

  SetOutPath $INSTDIR
  Exec "$INSTDIR\MLL Semi-Automatic Segmentation.exe"

FunctionEnd

; For desktop shortcut

!define MUI_FINISHPAGE_SHOWREADME
!define MUI_FINISHPAGE_SHOWREADME_FUNCTION CreateDeskShortcut
!define MUI_FINISHPAGE_SHOWREADME_TEXT "Create desktop shortcut"

Function CreateDeskShortcut

  CreateShortcut "$DESKTOP\MLL Semi-Automatic Segmentation.lnk" "$INSTDIR\MLL Semi-Automatic Segmentation.exe" "" "$INSTDIR\MLL Semi-Automatic Segmentation.exe" 0

FunctionEnd

; Finish page

!include "MUI2.nsh"

; !define MUI_PAGE_CUSTOMFUNCTION_PRE fin_pre

; !define MUI_PAGE_CUSTOMFUNCTION_SHOW fin_show

; !define MUI_PAGE_CUSTOMFUNCTION_LEAVE fin_leave

!insertmacro MUI_PAGE_FINISH

;--------------------------------

; The stuff to install
Section "MLL (required)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File /r "*"
  Delete $INSTDIR\MLL_NSIS_SCRIPT.nsi

  ; Write the installation path into the registry
;   WriteRegStr HKLM SOFTWARE\NSIS_Example2 "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
;   WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Example2" "DisplayName" "NSIS Example2"
;   WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Example2" "UninstallString" '"$INSTDIR\uninstall.exe"'
;   WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Example2" "NoModify" 1
;   WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Example2" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\MLL Semi-Automatic Segmentation"
  CreateShortcut "$SMPROGRAMS\Example2\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortcut "$SMPROGRAMS\MLL Semi-Automatic Segmentation\MLL Semi-Automatic Segmentation.lnk" "$INSTDIR\MLL Semi-Automatic Segmentation.exe" "" "$INSTDIR\MLL Semi-Automatic Segmentation.exe" 0
  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
;   DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Example2"
;   DeleteRegKey HKLM SOFTWARE\NSIS_Example2

  ; Remove files and uninstaller
;   Delete $INSTDIR\example2.nsi
;   Delete $INSTDIR\uninstall.exe

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\MLL Semi-Automatic Segmentation\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\MLL"
  Delete "$DESKTOP\MLL Semi-Automatic Segmentation.lnk"
  RMDir /r "$INSTDIR"

SectionEnd
