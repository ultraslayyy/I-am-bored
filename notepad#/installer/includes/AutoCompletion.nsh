SectionGroup "Auto-completion Files" AutoCompletionComponent
    SetOverwrite off

    Section "C#" C#
        SectionIn 1
        SetOutPath "$INSTDIR\autoCompletion"
        File ".\components\apis\cs.xml"
    SectionEnd
SectionGroupEnd

SectionGroup un.AutoCompletionComponent
    Section un.C#
        Delete "$INSTDIR\autoCompletion/cs.xml"
    SectionEnd
SectionGroupEnd