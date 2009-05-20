@echo off

set ver_text=%1
set ver_file=%2
if "%ver_text%"=="" set ver_text=internal
if "%ver_file%"=="" set ver_file=internal

rem -------------------------------------------------------
rem Clean all and make the source distribution
:build_source

cd ../valib
call clean.cmd
cd ../spdifer
call clean.cmd

del ..\spdifer_%ver_file%_win9x.exe

rem -------------------------------------------------------
rem Build projects
:build_projects

call build.cmd vc6
if errorlevel 1 goto fail

rem -------------------------------------------------------
rem Build the installer
:build_installer

call vars.cmd
if "%inno_setup%" == "" set inno_setup=iscc.exe
if not exist "%inno_setup%" goto err_iss

"%inno_setup%" /Dappver="%ver_text%" /o".." /f"spdifer_%ver_file%_win9x" spdifer_win9x.iss
if errorlevel 1 goto fail

rem -------------------------------------------------------
rem All OK

echo All OK!
goto end

rem -------------------------------------------------------
rem Error messages

:err_iss
echo Inno setup compiler (iscc.exe) not found. Set %%inno_setup%% variable at vars.cmd.

:fail
echo Build failed!
error 2>nul
:end
