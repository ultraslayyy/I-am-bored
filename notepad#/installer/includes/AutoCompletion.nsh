SectionGroup "Auto-completion Files" AutoCompletionComponent
    SetOverwrite off

    Section "C#" C#
        SetOutPath "$INSTDIR\autoCompletion"
        File ".\components\apis\cs.xml"
    SectionEnd
SectionGroupEnd