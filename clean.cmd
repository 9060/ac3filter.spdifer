@echo off
call cmd\clean_vc.cmd %*
cd spdifer_config
call clean.cmd
cd ..
