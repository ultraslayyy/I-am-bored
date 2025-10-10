SectionGroup "Function List Files" FunctionListComponent
    SetOverwrite off

    Section "C#" C#_FL
        SetOutPath "$INSTDIR\functionList"
        File ".\components\functionList\cs.xml"
    SectionEnd
SectionGroupEnd

SectionGroup un.FunctionListComponent
    Section un.C#_FL
        delete "$INSTDIR\functionList\cs.xml"
    SectionEnd
SectionGroupEnd