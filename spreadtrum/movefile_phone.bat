@ECHO OFF

echo ########## Set Dthing Porting Environment Variables ######################################
setlocal

REM ########## TODO: check if %1 and %2 is "" #################################################
REM ########## %1: root dir of dthing  ########################################################
REM ########## %2: root dir of spd sdk ########################################################
set SRCDIR=%1
set DESDIR=%2

copy %SRCDIR%\spreadtrum\export\1308\dthing.mk  %DESDIR%\make\dthing\
copy %SRCDIR%\spreadtrum\export\1308\priority_app.h  %DESDIR%\..\common\export\inc\
copy %SRCDIR%\spreadtrum\export\1308\sc7701_sp7701\os_app_cfg.c  %DESDIR%\..\MS_Customize\source\product\config\sc7701_sp7701\
endlocal