!define APPVERSION "0.1.0"

!ifdef Final
    SetCompressor /SOLID lzma /FINAL
!else
    SetCompressor /SOLID lzma
!endif

!ifdef Platform
    !if "${Platform}" == "win-x64"
        OutFile "./build/ns.${APPVERSION}.Installer.x64.exe"
    !else
        !if "${Platform}" == "win-arm64"
            OutFile "./build/ns.${APPVERSION}.Installer.arm64.exe"
        !else
            OutFile "./build/ns.${APPVERSION}.Installer.exe"
        !endif
    !endif
!else
    OutFile "./build/ns.${APPVERSION}.Installer.exe"
!endif

Name "Notepad# ${APPVERSION}"
InstallDir "$PROGRAMFILES\Notepad#"
RequestExecutionLevel admin

Page directory
Page instfiles

Section "MainSection" SEC01
    SetOutPath "$INSTDIR"
    !ifdef Platform
        File /r "..\publish\${Platform}\*"
    !else
        File /r "..\publish\win-x86\*"
    !endif
SectionEnd
