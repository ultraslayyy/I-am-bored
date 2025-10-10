!include "MUI2.nsh"
!include "LogicLib.nsh"
!include "x64.nsh"
!include "nsDialogs.nsh"
!include "FileFunc.nsh"

!define APPVERSION "0.1.0"
!define APPNAME "Notepad# v${APPVERSION}"

SetCompressor /SOLID lzma

!ifdef WIN64
    OutFile "./build/ns.${APPVERSION}.Installer.x64.exe"
!else ifdef WinArm64
    OutFile "./build/ns.${APPVERSION}.Installer.arm64.exe"
!else
    OutFile "./build/ns.${APPVERSION}.Installer.exe"
!endif

Name "Notepad# ${APPVERSION}"
!ifdef WIN64
    InstallDir "$PROGRAMFILES64\Notepad#"
!else
    InstallDir "$PROGRAMFILES\Notepad#"
!endif
RequestExecutionLevel admin
ShowInstDetails show

BrandingText "I use VSCode, not ts"

InstType "Minimalistic"

; --- Signing directives ---
!finalize        'sign-install.bat "%1"' = 0
!uninstfinalize  'sign-install.bat "%1"' = 0

; MUI Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "LICENSE.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_COMPONENTS
Page custom OptionsPageCreate OptionsPageLeave
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; Languages
!include "includes\Languages.nsh"

; Components Page
!include "includes\AutoCompletion.nsh"
!include "includes\FunctionList.nsh"
!include "includes\Themes.nsh"

Section "Context Menu Entry" ExplorerContextMenuEntry
    ${If} ${RunningX64}
        SetRegView 64
    ${Else}
        SetRegView 32
    ${EndIf}

    ClearErrors
    WriteRegStr HKLM "Software\Classes\*\shell\Edit with Notepad#" "" "Edit with Notepad#"
    IfErrors 0 +3
        WriteRegStr HKCU "Software\Classes\*\shell\Edit with Notepad#" "" "Edit with Notepad#"
        Goto doneShell

    doneShell:

    WriteRegStr HKLM "Software\Classes\*\shell\Edit with Notepad#" "Icon" "$INSTDIR\notepad#.exe,0"
    WriteRegStr HKLM "Software\Classes\*\shell\Edit with Notepad#\command" "" '"$INSTDIR\notepad#.exe" "%1"'

    WriteRegStr HKCU "Software\Classes\*\shell\Edit with Notepad#" "Icon" "$INSTDIR\notepad#.exe,0"
    WriteRegStr HKCU "Software\Classes\*\shell\Edit with Notepad#\command" "" '"$INSTDIR\notepad#.exe" "%1"'
SectionEnd

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${AutoCompletionComponent} 'Install the API files you need for the auto-completion feature (Ctrl+Space).'
    !insertmacro MUI_DESCRIPTION_TEXT ${FunctionListComponent} 'Install the function list files you need for the function list feature (Ctrl+Space).'
    !insertmacro MUI_DESCRIPTION_TEXT ${ThemesComponent} 'The eye-candy to change visual effects. Use Theme selector to switch among them.'
    !insertmacro MUI_DESCRIPTION_TEXT ${ExplorerContextMenuEntry} 'Explorer context menu entry for Notepad# : Open whatever you want in Notepad# from Windows Explorer.'
!insertmacro MUI_FUNCTION_DESCRIPTION_END

; Variables for options
Var Dialog
Var ShowDetails
Var CheckboxShortcut
Var CheckboxNoAppData
Var NoAppDataChecked
Var CheckboxShowDetails

; Options Page (Custom)
Function OnChange_CheckboxNoAppData
	${NSD_GetState} $CheckboxNoAppData $NoAppDataChecked
FunctionEnd

Function OptionsPageCreate
    nsDialogs::Create /NOUNLOAD 1018
    Pop $Dialog

    ${If} $Dialog == error
        Abort
    ${EndIf}

    ; Desktop shortcut
    ${NSD_CreateCheckbox} 0 0 100% 30u "Create shortcut on Desktop"
    Pop $CheckboxShortcut
    ${NSD_SetState} $CheckboxShortcut ${BST_CHECKED}

    ; Don't use %APPDATA%
    ${NSD_CreateCheckbox} 0 80 100% 30u "Don't use %APPDATA%\nEnable this option to make Notepad# load/write the configuration files from/to its install directory. Check it if you use Notepad# on a USB device."
    Pop $CheckboxNoAppData
    IfFileExists "$INSTDIR\doLocalConf.xml" doLocalConfIfExists doLocalConfDoesNotExist
    doLocalConfIfExists:
        ${NSD_SetState} $CheckboxNoAppData ${BST_CHECKED}
        StrCpy $NoAppDataChecked ${BST_CHECKED}
    doLocalConfDoesNotExist:
    ${NSD_SetState} $CheckboxNoAppData OnChange_CheckboxNoAppData

    ; Show installation details
    ${NSD_CreateCheckbox} 0 160 100% 30u "Show installation details"
    Pop $CheckboxShowDetails
    ${NSD_SetState} $CheckboxShowDetails ${BST_CHECKED}

    ; Yoinked fron npp
    StrLen $0 $PROGRAMFILES
    StrCpy $1 $InstDir $0

    StrLen $0 $PROGRAMFILES64
	StrCpy $2 $InstDir $0
    ${If} $1 == "$PROGRAMFILES"
    ${ORIF} $2 == "$PROGRAMFILES64"
        ${NSD_Uncheck} $NoAppDataChecked
        EnableWindow $NoAppDataChecked 0
    ${Else}
        EnableWindow $NoAppDataChecked 1
    ${EndIf}

    nsDialogs::Show
FunctionEnd

Function OptionsPageLeave
    ${NSD_GetState} $CheckboxShortcut $0
    StrCpy $CheckboxShortcut $0

    ${NSD_GetState} $CheckboxNoAppData $1
    StrCpy $CheckboxNoAppData $1

    ${NSD_GetState} $CheckboxShowDetails $2
    StrCpy $ShowDetails $2
FunctionEnd

; Functions
Function .onInit
    !insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd

; Main Installation
Section "-Main Program" SEC01
    SetOutPath "$INSTDIR"

    ${If} $ShowDetails == ${BST_CHECKED}
        SetDetailsPrint textonly
    ${Else}
        SetDetailsPrint none
    ${EndIf}

    !ifdef Win64
        File /r "..\publish\win-x64\*"
    !else ifdef WinArm64
        File /r "..\publish\win-arm64"
    !else
        File /r "..\publish\win-x86\*"
    !endif

    ${If} ${RunningX64}
        SetRegView 64
    ${Else}
        SetRegView 32
    ${EndIf}

    WriteRegStr HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\notepad#.exe" "" "$INSTDIR\notepad#.exe"

    WriteUninstaller "$INSTDIR\uninstall.exe"
    !ifdef WIN64
        !ifdef CONTAINED
            WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Notepad#" "DisplayName" "Notepad# (64-bit x64) (contained)"
        !else
            WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Notepad#" "DisplayName" "Notepad# (64-bit x64)"
        !endif
    !else ifdef WinArm64
        !ifdef CONTAINED
            WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Notepad#" "DisplayName" "Notepad# (ARM 64-bit) (contained)"
        !else
            WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Notepad#" "DisplayName" "Notepad# (ARM 64-bit)"
        !endif
    !else
        !ifdef CONTAINED
            WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Notepad#" "DisplayName" "Notepad# (32-bit x86) (contained)"
        !else
            WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Notepad#" "DisplayName" "Notepad# (32-bit x86)"
        !endif
    !endif
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Notepad#" "DisplayVersion" "${APPVERSION}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Notepad#" "Publisher" "ultraslayyy"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Notepad#" "InstallLocation" "$INSTDIR"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Notepad#" "DisplayIcon" "$INSTDIR\notepad#.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Notepad#" "UninstallString" "$INSTDIR\uninstall.exe"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Notepad#" "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Notepad#" "NoRepair" 1

    ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
    IfErrors +3 0
    IntFmt $0 "0x%08X" $0
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Notepad#" "EstimatedSize" "$0"

    ${If} $CheckboxShortcut == ${BST_CHECKED}
        CreateShortcut "$DESKTOP\notepad#.lnk" "$INSTDIR\notepad#.exe"
    ${EndIf}
    ${If} $CheckboxNoAppData == ${BST_CHECKED}
        File '.\components\doLocalConfig.xml'
    ${EndIf}
SectionEnd

; Uninstaller
Section "Uninstall"
    Delete "$DESKTOP/notepad#.lnk"
    
    ${If} ${RunningX64}
        SetRegView 64
    ${Else}
        SetRegView 32
    ${EndIf}

    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Notepad#"
    DeleteRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\notepad#.exe"
    DeleteRegKey HKLM "Software\Classes\*\shell\Edit with Notepad#"
    DeleteRegKey HKCU "Software\Classes\*\shell\Edit with Notepad#"

    IfFileExists "$INSTDIR\notepad#.exe" 0 +2
    StrCpy $0 1

    StrCmp $0 1 0 done_cleanup
        Delete "$INSTDIR\uninstall.exe"
        IfErrors 0 +3
        Delete /REBOOTOK "$INSTDIR\uninstall.exe"
        RMDir /r "$INSTDIR"

    done_cleanup:
SectionEnd