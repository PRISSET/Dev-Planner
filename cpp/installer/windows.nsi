!include "MUI2.nsh"

Name "Dev Planner"
OutFile "DevPlannerSetup.exe"
InstallDir "$PROGRAMFILES64\Dev Planner"
RequestExecutionLevel admin

!define MUI_ICON "..\..\icon.iconset\icon_256x256.png"
!define MUI_ABORTWARNING
!define MUI_FINISHPAGE_RUN "$INSTDIR\DevPlanner.exe"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\..\LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Russian"

Section "Install"
    SetOutPath "$INSTDIR"
    
    File "DevPlanner.exe"
    File "*.dll"
    File /r "platforms"
    File /r "styles"
    File /r "imageformats"
    File /nonfatal /r "tls"
    File /nonfatal /r "networkinformation"
    
    WriteUninstaller "$INSTDIR\Uninstall.exe"
    
    CreateDirectory "$SMPROGRAMS\Dev Planner"
    CreateShortcut "$SMPROGRAMS\Dev Planner\Dev Planner.lnk" "$INSTDIR\DevPlanner.exe"
    CreateShortcut "$SMPROGRAMS\Dev Planner\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
    CreateShortcut "$DESKTOP\Dev Planner.lnk" "$INSTDIR\DevPlanner.exe"
    
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DevPlanner" "DisplayName" "Dev Planner"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DevPlanner" "UninstallString" "$\"$INSTDIR\Uninstall.exe$\""
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DevPlanner" "DisplayIcon" "$INSTDIR\DevPlanner.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DevPlanner" "Publisher" "Dev Planner"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DevPlanner" "DisplayVersion" "${VERSION}"
SectionEnd

Section "Uninstall"
    Delete "$INSTDIR\DevPlanner.exe"
    Delete "$INSTDIR\*.dll"
    RMDir /r "$INSTDIR\platforms"
    RMDir /r "$INSTDIR\styles"
    RMDir /r "$INSTDIR\imageformats"
    RMDir /r "$INSTDIR\tls"
    RMDir /r "$INSTDIR\networkinformation"
    Delete "$INSTDIR\Uninstall.exe"
    RMDir "$INSTDIR"
    
    Delete "$SMPROGRAMS\Dev Planner\Dev Planner.lnk"
    Delete "$SMPROGRAMS\Dev Planner\Uninstall.lnk"
    RMDir "$SMPROGRAMS\Dev Planner"
    Delete "$DESKTOP\Dev Planner.lnk"
    
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\DevPlanner"
SectionEnd
