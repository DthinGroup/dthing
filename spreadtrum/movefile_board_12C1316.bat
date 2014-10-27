@ECHO OFF

echo ########## Set Dthing Porting Environment Variables ######################################
setlocal

set SRCDIR=D:\nix.long\ReDvmAll\dvm
set DESDIR=D:\WORK\Spreadtrum\MOCOR_12C.W13.16_Source\MS_Code\BASE

copy %SRCDIR%\appdb\*  %DESDIR%\dthing\appdb\
copy %SRCDIR%\base\inc\*  %DESDIR%\dthing\base\inc\
copy %SRCDIR%\base\src\*  %DESDIR%\dthing\base\src\
copy %SRCDIR%\main\*  %DESDIR%\dthing\main\
copy %SRCDIR%\porting\inc\*  %DESDIR%\dthing\porting\inc\
copy %SRCDIR%\porting\src\*  %DESDIR%\dthing\porting\src\
copy %SRCDIR%\rams\inc\*  %DESDIR%\dthing\rams\inc\
copy %SRCDIR%\rams\src\*  %DESDIR%\dthing\rams\src\
copy %SRCDIR%\vm\impl\*  %DESDIR%\dthing\vm\impl\
copy %SRCDIR%\vm\inc\*  %DESDIR%\dthing\vm\inc\
copy %SRCDIR%\vm\src\*  %DESDIR%\dthing\vm\src\
copy %SRCDIR%\vm\common.h  %DESDIR%\dthing\vm\
copy %SRCDIR%\vm\dthing.h  %DESDIR%\dthing\vm\
copy %SRCDIR%\vm\init.c  %DESDIR%\dthing\vm\
copy %SRCDIR%\www\inc\*  %DESDIR%\dthing\www\inc\
copy %SRCDIR%\www\src\*  %DESDIR%\dthing\www\src\
copy %SRCDIR%\zlib\*  %DESDIR%\dthing\zlib\

copy %SRCDIR%\spreadtrum\export\12C.1316\SC6530.modules  %DESDIR%\..\
copy %SRCDIR%\spreadtrum\export\12C.1316\SC6800H.modules  %DESDIR%\..\
copy %SRCDIR%\spreadtrum\export\12C.1316\SC8800G.modules  %DESDIR%\..\
copy %SRCDIR%\spreadtrum\export\12C.1316\Makefile.rules  %DESDIR%\..\

copy %SRCDIR%\spreadtrum\export\12C.1316\dthing.mk  %DESDIR%\make\dthing\
copy %SRCDIR%\spreadtrum\export\12C.1316\priority_app.h  %DESDIR%\..\common\export\inc\
copy %SRCDIR%\spreadtrum\export\12C.1316\sc6500_modem\os_app_cfg.c  %DESDIR%\..\MS_Customize\source\product\config\sc6500_modem\
copy %SRCDIR%\spreadtrum\export\12C.1316\sc6530_modem\os_app_cfg.c  %DESDIR%\..\MS_Customize\source\product\config\sc6530_modem\
endlocal