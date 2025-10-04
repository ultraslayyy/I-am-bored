[Setup]
AppName=Notepad#
AppVersion=1.0
DefaultDirName={pf}\Notepad#
DefaultGroupName=Notepad#
OutputBaseFilename=n#.1.0.0.Installer.x64
Compression=lzma
SolidCompression=yes
WizardStyle=modern
UninstallDisplayIcon={app}\Notepad#.exe

[Files]
Source: "..\publish\win-x64\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{userdesktop}\Notepad#"; Filename: "{app}\Notepad#.exe"
Name: "{group}\Notepad#"; Filename: "{app}\Notepad#.exe"

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"

[LicenseFile]
LicenseFile=D:\Repos\Harry\ui\test\LICENSE

[Code]
var
  PageComponents: TOutputProgressWizardPage;
  CheckDesktopShortcut: TCheckBox;
  CheckUseAppData: TCheckBox;
  CheckShowDetails: TCheckBox;

procedure InitializeWizard();
var
  LanguagePage: TWizardPage;
begin
  LanguagePage := CreateCustomPage(wpWelcome, 'Please select a language', '');
  with TComboBox.Create(WizardForm) do
  begin
    Parent := LanguagePage.Surface;
    Items.Add('English');
    Items.Add('French');
    ItemIndex := 0;
    Left := 10;
    Top := 10;
    Width := 200;
  end;

  PageComponents := CreateOutputProgressPage('Options', 'Select components to install:');

  CheckDesktopShortcut := TCheckBox.Create(WizardForm);
  CheckDesktopShortcut.Parent := PageComponents.Surface;
  CheckDesktopShortcut.Caption := 'Create Desktop shortcut';
  CheckDesktopShortcut.Checked := True;
  CheckDesktopShortcut.Top := 0;
  CheckDesktopShortcut.Left := 0;

  CheckUseAppData := TCheckBox.Create(WizardForm);
  CheckUseAppData.Parent := PageComponents.Surface;
  CheckUseAppData.Caption := 'Do not use %APPDATA%';
  CheckUseAppData.Checked := False;
  CheckUseAppData.Top := 25;
  CheckUseAppData.Left := 0;

  CheckShowDetails := TCheckBox.Create(WizardForm);
  CheckShowDetails.Parent := PageComponents.Surface;
  CheckShowDetails.Caption := 'Show installation details';
  CheckShowDetails.Checked := False;
  CheckShowDetails.Top := 50;
  CheckShowDetails.Left := 0;
end;

function NextButtonClick(CurPageID: Integer): Boolean;
begin
  Result := True;
end;

procedure CurStepChanged(CurStep: TSetupStep);
var
  ExitCode: Integer;
begin
  if CurStep = ssPostInstall then
  begin
    if MsgBox('Launch Notepad# now?', mbConfirmation, MB_YESNO) = IDYES then
      Exec(ExpandConstant('{app}\Notepad#.exe'), '', '', SW_SHOWNORMAL, ewNoWait, ExitCode);
  end;
end;