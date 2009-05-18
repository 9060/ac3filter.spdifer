rem @echo off
echo ------------------------
echo Register %1
set filter=%1\spdifer.ax
if not exist "%filter%" set filter=%1\spdifer64.ax

regsvr32 /s /u "%filter%"
regsvr32 /s "%filter%"
