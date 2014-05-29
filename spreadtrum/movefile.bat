@ECHO OFF

echo ########## Set Dthing Porting Environment Variables ######################################
setlocal

set SRCDIR=D:\nix.long\ReDvmAll\dvm
set DESDIR=D:\WORK\Spreadtrum\MOCOR_W13.08_Source\MS_Code\BASE

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
copy %SRCDIR%\zlib\*  %DESDIR%\dthing\zlib\

copy %SRCDIR%\spreadtrum\export\1308\dthing.mk  %DESDIR%\make\dthing\
copy %SRCDIR%\spreadtrum\export\1308\priority_app.h  %DESDIR%\..\common\export\inc\
copy %SRCDIR%\spreadtrum\export\1308\sc7701_sp7701\os_app_cfg.c  %DESDIR%\..\MS_Customize\source\product\config\sc7701_sp7701\
endlocal