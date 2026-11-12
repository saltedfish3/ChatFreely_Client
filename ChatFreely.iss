#define MyAppName "³©ÁÄ ChatFreely"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "ChatFreely"
#define MyAppExeName "ChatClient.exe"
#define DistDir "dist\ChatFreely-Client-1.0.0"

[Setup]
AppId={{8E4B7A2C-5F1D-4C3A-9B6E-2D7F3A1C9E05}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\ChatFreely
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
OutputDir=dist\installer
OutputBaseFilename=ChatFreely-Client-{#MyAppVersion}-win64-setup
SetupIconFile=src\app.ico
UninstallDisplayIcon={app}\{#MyAppExeName}
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
MinVersion=10.0
VersionInfoVersion=1.0.0.0
VersionInfoCompany=ChatFreely
VersionInfoDescription=³©ÁÄ ChatFreely °²×°³ÌÐò
VersionInfoProductName=ChatFreely
VersionInfoProductVersion=1.0.0.0

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#DistDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\Ð¶ÔØ {#MyAppName}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#MyAppName}}"; Flags: nowait postinstall skipifsilent

[Languages]
Name: "chinesesimplified"; MessagesFile: "compiler:Languages\ChineseSimplified.isl"