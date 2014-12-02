@ECHO OFF

echo ########## Set Dthing Porting Environment Variables ######################################
setlocal

REM ########## TODO: check if %1 and %2 is "" #################################################
REM ########## %1: root dir of dthing  ########################################################
REM ########## %2: root dir of spd sdk ########################################################
set SRCDIR=%1
set DESDIR=%2

copy %SRCDIR%\spreadtrum\export\12C.1316\SC6530.modules  %DESDIR%\..\
copy %SRCDIR%\spreadtrum\export\12C.1316\SC6800H.modules  %DESDIR%\..\
copy %SRCDIR%\spreadtrum\export\12C.1316\SC8800G.modules  %DESDIR%\..\
copy %SRCDIR%\spreadtrum\export\12C.1316\Makefile.rules  %DESDIR%\..\

copy %SRCDIR%\spreadtrum\export\12C.1316\dthing.mk  %DESDIR%\make\dthing\
copy %SRCDIR%\spreadtrum\export\12C.1316\priority_app.h  %DESDIR%\..\common\export\inc\
copy %SRCDIR%\spreadtrum\export\12C.1316\sc6500_modem\os_app_cfg.c  %DESDIR%\..\MS_Customize\source\product\config\sc6500_modem\
copy %SRCDIR%\spreadtrum\export\12C.1316\sc6530_modem\os_app_cfg.c  %DESDIR%\..\MS_Customize\source\product\config\sc6530_modem\
endlocal