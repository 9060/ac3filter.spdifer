#ifndef appver
#define appver "test"
#endif

[Setup]
AppID=Spdifer
AppVersion={#appver}
AppName=Spdifer
AppVerName=Spdifer {#appver}
AppPublisher=Alexander Vigovsky
AppPublisherURL=http://ac3filter.net
AppCopyright=Copyright (c) 2007-2009 by Alexander Vigovsky
DefaultDirName={pf}\Spdifer
DefaultGroupName=Spdifer
SolidCompression=yes
LanguageDetectionMethod=locale

[Languages]
Name: en; MessagesFile: "compiler:Default.isl"
Name: eu; MessagesFile: "compiler:Languages\Basque.isl"
Name: pt_br; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"
Name: ca; MessagesFile: "compiler:Languages\Catalan.isl"
Name: cs; MessagesFile: "compiler:Languages\Czech.isl"
Name: da; MessagesFile: "compiler:Languages\Danish.isl"
Name: nl; MessagesFile: "compiler:Languages\Dutch.isl"
Name: fi; MessagesFile: "compiler:Languages\Finnish.isl"
Name: fr; MessagesFile: "compiler:Languages\French.isl"
Name: de; MessagesFile: "compiler:Languages\German.isl"
Name: he; MessagesFile: "compiler:Languages\Hebrew.isl"
Name: hu; MessagesFile: "compiler:Languages\Hungarian.isl"
Name: it; MessagesFile: "compiler:Languages\Italian.isl"
Name: nn; MessagesFile: "compiler:Languages\Norwegian.isl"
Name: pl; MessagesFile: "compiler:Languages\Polish.isl"
Name: pt; MessagesFile: "compiler:Languages\Portuguese.isl"
Name: ru; MessagesFile: "compiler:Languages\Russian.isl"
Name: sk; MessagesFile: "compiler:Languages\Slovak.isl"
Name: sl; MessagesFile: "compiler:Languages\Slovenian.isl"
Name: es; MessagesFile: "compiler:Languages\Spanish.isl"

[Components]
Name: "prog";          Description: "Program files:"; Types: full compact
Name: "prog\filter32"; Description: "Spdifer (32bit)"; Types: full compact

[Files]
Source: "Release\spdifer.ax";                            DestDir: "{app}"; Components: prog\filter32; Flags: 32bit Regserver RestartReplace UninsRestartDelete IgnoreVersion
Source: "spdifer_config\Release\spdifer_config.exe";     DestDir: "{app}"; Components: prog\filter32; Flags: 32bit RestartReplace UninsRestartDelete IgnoreVersion

Source: "Readme.txt";  DestDir: "{app}"
Source: "Changes.txt"; DestDir: "{app}"
Source: "GPL.txt";     DestDir: "{app}"

[Icons]
Name: "{group}\AC3Filter home"; Filename: "http://ac3filter.net"
Name: "{group}\Spdifer config"; Filename: "{app}\spdifer_config.exe"; Components: prog\filter32
Name: "{group}\Readme"; Filename: "{app}\Readme.txt"
Name: "{group}\Uninstall Spdifer"; Filename: "{uninstallexe}"
