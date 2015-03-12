; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Unicode Db IME"
#define MyAppVersion "0.99.4"
#define MyAppPublisher "squidssquids"
#define MyAppURL "http://sourceforge.net/p/itf-unicode-db/wiki/Home/"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{4DD51F85-E837-4799-941F-32CC08F93E80}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
LicenseFile=TwoLicences.txt
OutputDir=..\InnoSetup
OutputBaseFilename=UnicodeDbIme-{#MyAppVersion}
Compression=lzma
SolidCompression=yes
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "..\Debug\UnicodeDbIME.dll"; DestDir: "{app}\x86"; Flags: ignoreversion regserver 32bit
Source: "..\x64\Debug\UnicodeDbIME.dll"; DestDir: "{app}\x64"; Flags: ignoreversion regserver 64bit
Source: "..\data\UnicodeDataIndex.txt"; DestDir: "{app}\x64"; Flags: ignoreversion
Source: "..\data\UnicodeDataIndex.txt"; DestDir: "{app}\x86"; Flags: ignoreversion
Source: "..\data\UnicodeData.txt"; DestDir: "{app}\x64"; Flags: ignoreversion
Source: "..\data\UnicodeData.txt"; DestDir: "{app}\x86"; Flags: ignoreversion
Source: "..\UnicodeDbIMESettings\Debug\UnicodeDbIMESettings2.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\README.md"; DestDir: "{app}"; DestName: "README.txt"; Flags: isreadme
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\Unicode DB IME Settings"; Filename: "{app}\UnicodeDbIMESettings2.exe"
