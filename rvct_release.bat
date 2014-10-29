@ECHO OFF

echo ########## gen release files ######################################
setlocal

echo ##### delete old files ########################

set CURDIR=%cd%
rd %CURDIR%\gen\release\MS_Code\ /S /Q
md %CURDIR%\gen\release\MS_Code\
set DESDIR=%CURDIR%\gen\release\MS_Code\
set IDHDIR=D:\WORK\Spreadtrum\KX8800B_CODE_UPDATE_1112\MS_Code\

REM ##Copy IDH files###

copy %CURDIR%\spreadtrum\export\KX8800\SC6530.modules  %DESDIR%\
copy %CURDIR%\spreadtrum\export\KX8800\SC6800H.modules  %DESDIR%\
copy %CURDIR%\spreadtrum\export\KX8800\SC8800G.modules  %DESDIR%\
copy %CURDIR%\spreadtrum\export\KX8800\Makefile.rules  %DESDIR%\
copy %CURDIR%\spreadtrum\export\KX8800\Makefile.third-party  %DESDIR%\
md %DESDIR%\BASE\make\dthing\
copy %CURDIR%\spreadtrum\export\KX8800\dthing.mk  %DESDIR%\BASE\make\dthing\
md %DESDIR%\common\export\inc\
copy %CURDIR%\spreadtrum\export\KX8800\priority_app.h  %DESDIR%\common\export\inc\
md %DESDIR%\MS_Customize\source\product\config\sc6500_modem\
copy %CURDIR%\spreadtrum\export\KX8800\sc6500_modem\os_app_cfg.c  %DESDIR%\MS_Customize\source\product\config\sc6500_modem\
md %DESDIR%\MS_Customize\source\product\config\sc6530_modem\
copy %CURDIR%\spreadtrum\export\KX8800\sc6530_modem\os_app_cfg.c  %DESDIR%\MS_Customize\source\product\config\sc6530_modem\
md %DESDIR%\MS_Ref\source\base\src\
copy %CURDIR%\spreadtrum\export\KX8800\MS_Ref\source\base\src\init.c %DESDIR%\MS_Ref\source\base\src\

REM ##Copy dthing files###
md %DESDIR%\BASE\dthing\base\inc\
md %DESDIR%\BASE\dthing\main\
md %DESDIR%\BASE\dthing\porting\inc
md %DESDIR%\BASE\dthing\rams\inc
md %DESDIR%\BASE\dthing\vm\impl
md %DESDIR%\BASE\dthing\vm\inc
md %DESDIR%\BASE\dthing\vm
md %DESDIR%\BASE\dthing\zlib

md %DESDIR%\BASE\dthing\vm\src
md %DESDIR%\BASE\dthing\vm\impl
md %DESDIR%\BASE\dthing\porting\src


copy %CURDIR%\base\inc\*.h  %DESDIR%\BASE\dthing\base\inc\
copy %CURDIR%\porting\inc\*.h  %DESDIR%\BASE\dthing\porting\inc\
copy %CURDIR%\rams\inc\*.h  %DESDIR%\BASE\dthing\rams\inc\
copy %CURDIR%\vm\impl\*.h  %DESDIR%\BASE\dthing\vm\impl\
copy %CURDIR%\vm\inc\*.h  %DESDIR%\BASE\dthing\vm\inc\
copy %CURDIR%\vm\common.h  %DESDIR%\BASE\dthing\vm\
copy %CURDIR%\vm\dthing.h  %DESDIR%\BASE\dthing\vm\
copy %CURDIR%\zlib\*.h  %DESDIR%\BASE\dthing\zlib\

copy %CURDIR%\main\dthing_main.c  %DESDIR%\BASE\dthing\main\
copy %CURDIR%\vm\src\vmTime.c  %DESDIR%\BASE\dthing\vm\src\
copy %CURDIR%\porting\src\*.c  %DESDIR%\BASE\dthing\porting\src\

copy %CURDIR%\vm\impl\nativeADCManager.c  %DESDIR%\BASE\dthing\vm\impl\
copy %CURDIR%\vm\impl\nativeATCommandConnection.c  %DESDIR%\BASE\dthing\vm\impl\
copy %CURDIR%\vm\impl\nativeBatteryManager.c  %DESDIR%\BASE\dthing\vm\impl\
copy %CURDIR%\vm\impl\nativeEarManager.c  %DESDIR%\BASE\dthing\vm\impl\
copy %CURDIR%\vm\impl\nativeGpio.c  %DESDIR%\BASE\dthing\vm\impl\
copy %CURDIR%\vm\impl\nativeI2CManager.c  %DESDIR%\BASE\dthing\vm\impl\
copy %CURDIR%\vm\impl\nativeLCD.c  %DESDIR%\BASE\dthing\vm\impl\
copy %CURDIR%\vm\impl\nativeMicophone.c  %DESDIR%\BASE\dthing\vm\impl\
copy %CURDIR%\vm\impl\nativeMyMessageSender.c  %DESDIR%\BASE\dthing\vm\impl\
copy %CURDIR%\vm\impl\nativeMySmsConnectionThread.c  %DESDIR%\BASE\dthing\vm\impl\
copy %CURDIR%\vm\impl\nativePCMChannel.c  %DESDIR%\BASE\dthing\vm\impl\
copy %CURDIR%\vm\impl\nativePWMManager.c  %DESDIR%\BASE\dthing\vm\impl\
copy %CURDIR%\vm\impl\nativeSimManager.c  %DESDIR%\BASE\dthing\vm\impl\
copy %CURDIR%\vm\impl\nativeSpeakerManager.c  %DESDIR%\BASE\dthing\vm\impl\
copy %CURDIR%\vm\impl\nativeSPIManager.c  %DESDIR%\BASE\dthing\vm\impl\
copy %CURDIR%\vm\impl\nativeUSBConnection.c  %DESDIR%\BASE\dthing\vm\impl\

xcopy %CURDIR%\gen\rvct\libs\DthingVM.a %IDHDIR%\Third-party\dthing\  /s /h /y /i

echo ==============================================================
echo ==================== copy files to IDH =======================
xcopy %CURDIR%\gen\release\MS_Code\* %IDHDIR%  /s /h /d /y /i
echo =======================copy over =============================

echo ==============================================================
echo ==================== gen release over ========================
echo Release files in %CURDIR%\gen\release\MS_Code\
echo ==============================================================

endlocal