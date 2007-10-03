OutFile "${SETUP_FILE}"
Name "Spdifer"
CRCCheck on
SetCompressor lzma


Icon "ac3filter.ico"
UninstallIcon "ac3filter.ico"

InstallDir "$PROGRAMFILES\AC3Filter\Spdifer"
InstallDirRegKey HKLM "Software\AC3Filter\Spdifer" "Install_Dir"

InstallColors {000000 C0C0C0}
InstProgressFlags "smooth"
ShowInstDetails "show"

UninstallText 'This will uninstall Spdifer ver ${VERSION}. Hit "Uninstall" to continue.'
ShowUnInstDetails "show"

Page directory
Page instfiles
UninstPage uninstConfirm
UninstPage instfiles

Section "Install"
  SetOutPath $INSTDIR

  ;; Copy Files
  File /r "${SOURCE_DIR}\*.*"

  ;; Make an uninstaller
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Spdifer" "DisplayName" "Spdifer (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Spdifer" "UninstallString" "$INSTDIR\uninstall.exe"
  WriteUninstaller "uninstall.exe"

  ;; Create Start Menu shortcuts
  CreateDirectory "$SMPROGRAMS\AC3Filter"
  CreateDirectory "$SMPROGRAMS\AC3Filter\Spdifer"
  CreateShortCut  "$SMPROGRAMS\AC3Filter\Spdifer\Spdifer Config.lnk" "$INSTDIR\spdifer_config.exe"
  CreateShortCut  "$SMPROGRAMS\AC3Filter\Spdifer\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut  "$SMPROGRAMS\AC3Filter\AC3Filter home.lnk" "http://ac3filter.net"

  ;; Register filter
  UnRegDLL "$INSTDIR\spdifer.ax"
  RegDll   "$INSTDIR\spdifer.ax"
SectionEnd

Section "Uninstall"

  ;; Unregister filter
  UnRegDLL "$INSTDIR\spdifer.ax"

  ;; Remove registry settings
  DeleteRegKey HKCU "Software\AC3Filter\Spdifer"

  ;; Remove Start Menu shortcuts
  Delete "$SMPROGRAMS\AC3Filter\Spdifer\*.*"
  RMDir "$SMPROGRAMS\AC3Filter\Spdifer"

  ;; Delete uninstaller
  DeleteRegKey   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Spdifer"

  ;; Delete files
  Delete "$INSTDIR\*.*"
  RMDir  "$INSTDIR"

SectionEnd
