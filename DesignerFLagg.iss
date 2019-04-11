; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "DesignerFLagg.exe"
#define MyAppVersion "1.0"
#define MyAppPublisher "Andrew Katz"
#define MyAppURL "http://www.akatz712.com/"
#define MyAppExeName "DesignerFLagg.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{D7C65383-3AC7-4C3A-A62E-C92EFB42A304}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\MagicDesigner
DisableProgramGroupPage=yes
LicenseFile=LICENSE
OutputBaseFilename=MagicDesignersetup
Compression=lzma
SolidCompression=yes
PrivilegesRequired=none
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Files]
Source: "DesignerFLagg.exe"; DestDir: "{app}"; Check: Is64BitInstallMode
Source: "mdesigner.png"; DestDir: "{app}"; Flags: ignoreversion
Source: "mdesigner_BZm_icon.ico"; DestDir: "{app}"; Flags: ignoreversion
Source: "USEME.html"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{commonprograms}\Magic Designer"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\mdesigner_BZm_icon.ico"
Name: "{commondesktop}\Magic Designer"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\mdesigner_BZm_icon.ico"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\Magic Designer"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\mdesigner_BZm_icon.ico"; Tasks: quicklaunchicon

[Registry]
Root: HKCR; Subkey: "MagicDesigner"; ValueType: string; ValueData: "Magic Designer Emulator"; Flags: uninsdeletekey
Root: HKCR; Subkey: "MagicDesigner\DefaultIcon"; ValueType: string; ValueData: "{app}\mdesigner_BZm_icon.ico"
Root: HKCR; Subkey: "MagicDesigner\shell"
Root: HKCR; Subkey: "MagicDesigner\shell\open"
Root: HKCR; Subkey: "MagicDesigner\shell\open\command"; ValueType: string; ValueData: "{app}\DesignerFLagg.exe ""%1"""
Root: HKCR; Subkey: ".MGS"; ValueType: string; ValueData: "MagicDesigner"; Flags: uninsdeletekey
Root: HKCR; Subkey: ".MGS"; ValueName: "PerceivedType"; ValueType: string; ValueData: "Text"; Flags: uninsdeletekey
Root: HKCR; Subkey: ".MGS"; ValueName: "Content Type"; ValueType: string; ValueData: "text/plain"; Flags: uninsdeletekey

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "Magic Designer"; Flags: nowait postinstall skipifsilent
