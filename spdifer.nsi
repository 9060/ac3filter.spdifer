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
  CreateDirectory "$SMPROGRAMS\AC3Filter project"
  CreateDirectory "$SMPROGRAMS\AC3Filter project\Spdifer"
  CreateShortCut  "$SMPROGRAMS\AC3Filter project\Spdifer\Spdifer Config.lnk" "$INSTDIR\spdifer_config.exe"
  CreateShortCut  "$SMPROGRAMS\AC3Filter project\Spdifer\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut  "$SMPROGRAMS\AC3Filter project\AC3Filter home.lnk" "http://ac3filter.net"

  ;; Register filter
  UnRegDLL "$INSTDIR\spdifer.ax"
  RegDll   "$INSTDIR\spdifer.ax"

  ;; Default audio renderer
  ReadEnvStr $1 "OS"
  StrCmp $1 "Windows_NT" RegNT Reg9x

RegNT:
  CreateShortCut  "$SMPROGRAMS\AC3Filter project\Spdifer\Set DirectSound as default renderer.lnk" "$INSTDIR\DirectSound.win2k.reg"
  CreateShortCut  "$SMPROGRAMS\AC3Filter project\Spdifer\Set WaveOut as default renderer.lnk" "$INSTDIR\WaveOut.win2k.reg"
  goto Finish

Reg9x:
  CreateShortCut  "$SMPROGRAMS\AC3Filter project\Spdifer\Set DirectSound as default renderer (Win9x).lnk" "$INSTDIR\DirectSound.win9x.reg"
  CreateShortCut  "$SMPROGRAMS\AC3Filter project\Spdifer\Set WaveOut as default renderer (Win9x).lnk" "$INSTDIR\WaveOut.win9x.reg"
  goto Finish

Finish:

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
