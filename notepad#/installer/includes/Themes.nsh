SectionGroup "Themes" ThemesComponent
    SetOverwrite on

    Section "-Dark Mode Default" DarkModeDefault
        SetOutPath "$INSTDIR\themes"
        File ".\components\themes\DarkModeDefault.xml"
    SectionEnd

    Section "Monokai" Monokai
        SetOutPath "$INSTDIR\themes"
        File ".\components\themes\Monokai.xml"
    SectionEnd
SectionGroupEnd

SectionGroup un.Themes
    Section un.DarkModeDefault
        Delete "$INSTDIR\themes\DarkModeDefault.xml"
    SectionEnd

    Section un.Monokai
        Delete "$INSTDIR\themes\Monokai.xml"
    SectionEnd
SectionGroupEnd