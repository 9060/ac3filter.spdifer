@echo off
call cmd\build_vc.cmd %*
cd spdifer_config
call build.cmd %*
cd ..
