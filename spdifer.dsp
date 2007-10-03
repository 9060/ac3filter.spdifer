# Microsoft Developer Studio Project File - Name="spdifer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=spdifer - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "spdifer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "spdifer.mak" CFG="spdifer - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "spdifer - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "spdifer - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "spdifer - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SPDIFER_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /O2 /Ob2 /I "valib" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SPDIFER_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 strmbase.lib msvcrt.lib oldnames.lib quartz.lib winmm.lib comctl32.lib kernel32.lib shell32.lib gdi32.lib user32.lib advapi32.lib uuid.lib dsound.lib ole32.lib oleaut32.lib /nologo /dll /pdb:none /machine:I386 /nodefaultlib /out:"Release/spdifer.ax"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=_reg release
# End Special Build Tool

!ELSEIF  "$(CFG)" == "spdifer - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SPDIFER_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "valib" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SPDIFER_EXPORTS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 strmbasd.lib msvcrtd.lib msvcprtd.lib oldnames.lib quartz.lib winmm.lib comctl32.lib kernel32.lib gdi32.lib shell32.lib user32.lib advapi32.lib uuid.lib dsound.lib ole32.lib oleaut32.lib /nologo /dll /debug /machine:I386 /nodefaultlib /out:"Debug/spdifer.ax" /pdbtype:sept
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=_reg debug
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "spdifer - Win32 Release"
# Name "spdifer - Win32 Debug"
# Begin Group "valib"

# PROP Default_Filter ""
# Begin Group "filters"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\valib\filters\parser_filter.cpp
# End Source File
# Begin Source File

SOURCE=.\valib\filters\parser_filter.h
# End Source File
# Begin Source File

SOURCE=.\valib\filters\spdifer.cpp
# End Source File
# Begin Source File

SOURCE=.\valib\filters\spdifer.h
# End Source File
# End Group
# Begin Group "sink"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\valib\sink\sink_dshow.cpp
# End Source File
# Begin Source File

SOURCE=.\valib\sink\sink_dshow.h
# End Source File
# End Group
# Begin Group "win32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\valib\win32\winspk.cpp
# End Source File
# Begin Source File

SOURCE=.\valib\win32\winspk.h
# End Source File
# End Group
# Begin Group "parsers"

# PROP Default_Filter ""
# Begin Group "spdif"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\valib\parsers\spdif\spdif_header.cpp
# End Source File
# Begin Source File

SOURCE=.\valib\parsers\spdif\spdif_header.h
# End Source File
# Begin Source File

SOURCE=.\valib\parsers\spdif\spdif_parser.cpp
# End Source File
# Begin Source File

SOURCE=.\valib\parsers\spdif\spdif_parser.h
# End Source File
# Begin Source File

SOURCE=.\valib\parsers\spdif\spdif_wrapper.cpp
# End Source File
# Begin Source File

SOURCE=.\valib\parsers\spdif\spdif_wrapper.h
# End Source File
# End Group
# Begin Group "ac3"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\valib\parsers\ac3\ac3_header.cpp
# End Source File
# Begin Source File

SOURCE=.\valib\parsers\ac3\ac3_header.h
# End Source File
# End Group
# Begin Group "dts"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\valib\parsers\dts\dts_header.cpp
# End Source File
# Begin Source File

SOURCE=.\valib\parsers\dts\dts_header.h
# End Source File
# End Group
# Begin Group "mpa"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\valib\parsers\mpa\mpa_header.cpp
# End Source File
# Begin Source File

SOURCE=.\valib\parsers\mpa\mpa_header.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\valib\parsers\multi_header.cpp
# End Source File
# Begin Source File

SOURCE=.\valib\parsers\multi_header.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\valib\bitstream.cpp
# End Source File
# Begin Source File

SOURCE=.\valib\bitstream.h
# End Source File
# Begin Source File

SOURCE=.\controls.cpp
# End Source File
# Begin Source File

SOURCE=.\controls.h
# End Source File
# Begin Source File

SOURCE=.\valib\data.h
# End Source File
# Begin Source File

SOURCE=.\valib\defs.cpp
# End Source File
# Begin Source File

SOURCE=.\valib\defs.h
# End Source File
# Begin Source File

SOURCE=.\valib\filter.h
# End Source File
# Begin Source File

SOURCE=.\valib\parser.cpp
# End Source File
# Begin Source File

SOURCE=.\valib\parser.h
# End Source File
# Begin Source File

SOURCE=.\valib\spk.cpp
# End Source File
# Begin Source File

SOURCE=.\valib\spk.h
# End Source File
# Begin Source File

SOURCE=.\valib\vtime.cpp
# End Source File
# Begin Source File

SOURCE=.\valib\vtime.h
# End Source File
# End Group
# Begin Group "decss"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\decss\CSSauth.cpp
# End Source File
# Begin Source File

SOURCE=.\decss\CSSauth.h
# End Source File
# Begin Source File

SOURCE=.\decss\CSSscramble.cpp
# End Source File
# Begin Source File

SOURCE=.\decss\CSSscramble.h
# End Source File
# Begin Source File

SOURCE=.\decss\DeCSSInputPin.cpp
# End Source File
# Begin Source File

SOURCE=.\decss\DeCSSInputPin.h
# End Source File
# End Group
# Begin Group "spdifer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\guids.cpp
# End Source File
# Begin Source File

SOURCE=.\guids.h
# End Source File
# Begin Source File

SOURCE=.\register.cpp
# End Source File
# Begin Source File

SOURCE=.\registry.cpp
# End Source File
# Begin Source File

SOURCE=.\registry.h
# End Source File
# Begin Source File

SOURCE=.\spdifer.def
# End Source File
# Begin Source File

SOURCE=.\spdifer.rc
# End Source File
# Begin Source File

SOURCE=.\spdifer_dlg.cpp
# End Source File
# Begin Source File

SOURCE=.\spdifer_dlg.h
# End Source File
# Begin Source File

SOURCE=.\spdifer_ds.cpp
# End Source File
# Begin Source File

SOURCE=.\spdifer_ds.h
# End Source File
# Begin Source File

SOURCE=.\spdifer_ver.h
# End Source File
# End Group
# End Target
# End Project
