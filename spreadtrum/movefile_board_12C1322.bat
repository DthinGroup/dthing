@ECHO OFF

echo ########## Set Dthing Porting Environment Variables ######################################
setlocal

REM ########## TODO: check if %1 and %2 is "" #################################################
REM ########## %1: root dir of dthing  ########################################################
REM ########## %2: root dir of spd sdk ########################################################
set SRCDIR=%1
set DESDIR=%2

REM ########### Auto Generated Environment Variables ##########################################
set DTHINGDIR=%DESDIR%\BASE\dthing
set EXPORT_SOURCE_ROOT=%SRCDIR%\spreadtrum\export\12C.13.22

RMDIR /S /Q %DTHINGDIR%\
MKDIR %DTHINGDIR%\

XCOPY /R /Y %EXPORT_SOURCE_ROOT%\SC6530.modules  %DESDIR%\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\SC6800H.modules  %DESDIR%\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\SC8800G.modules  %DESDIR%\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\Makefile.rules  %DESDIR%\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\Makefile.third-party  %DESDIR%\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\project_sc6500_32X32_modem.mk  %DESDIR%\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\project_sc6500_32X32_modem_debug.mk  %DESDIR%\

XCOPY /R /Y %EXPORT_SOURCE_ROOT%\dthing.mk  %DESDIR%\BASE\make\dthing\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\priority_app.h  %DESDIR%\common\export\inc\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\sc6500_modem\os_app_cfg.c  %DESDIR%\MS_Customize\source\product\config\sc6500_modem\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\sc6530_modem\os_app_cfg.c  %DESDIR%\MS_Customize\source\product\config\sc6530_modem\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\make\custom_drv\custom_drv.mk %DESDIR%\make\custom_drv\

REM ########## Spreadtrum SDK files changed by Yarlung Software ################################
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\BASE\atc\modem\tools\template\atc.atd %DESDIR%\BASE\atc\modem\tools\template\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\BASE\atc\modem\source\c\atc.c %DESDIR%\BASE\atc\modem\source\c\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\BASE\atc\modem\source\c\atc_basic_cmd.c %DESDIR%\BASE\atc\modem\source\c\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\BASE\atc\modem\source\c\atc_info.c %DESDIR%\BASE\atc\modem\source\c\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\BASE\atc\modem\source\c\atc_lex.c %DESDIR%\BASE\atc\modem\source\c\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\BASE\atc\modem\source\c\atc_yacc.c %DESDIR%\BASE\atc\modem\source\c\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\BASE\atc\modem\source\h\atc_basic_cmd.h %DESDIR%\BASE\atc\modem\source\h\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\BASE\atc\modem\source\h\atc_def.h %DESDIR%\BASE\atc\modem\source\h\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\BASE\atc\modem\source\c\modem*.c %DESDIR%\BASE\atc\modem\source\c\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\BASE\atc\modem\source\h\modem*.h %DESDIR%\BASE\atc\modem\source\h\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\BASE\make\atc\atc.mk %DESDIR%\BASE\make\atc\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\inc\cpl_atcontrol.h %DESDIR%\MS_Ref\export\inc\

XCOPY /R /Y %EXPORT_SOURCE_ROOT%\MS_Ref\source\base\src\init.c %DESDIR%\MS_Ref\source\base\src\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\MS_Ref\source\base\src\keypad.c %DESDIR%\MS_Ref\source\base\src\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\MS_Ref\source\mux\src\mux_driver.c %DESDIR%\MS_Ref\source\mux\src\

XCOPY /R /Y %EXPORT_SOURCE_ROOT%\MS_Customize\export\inc\gps_com.h %DESDIR%\MS_Customize\export\inc\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\MS_Customize\export\inc\gps_drv.h %DESDIR%\MS_Customize\export\inc\

XCOPY /R /Y %EXPORT_SOURCE_ROOT%\MS_Customize\source\product\driver\gps\*.c %DESDIR%\MS_Customize\source\product\driver\gps\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\MS_Customize\source\product\driver\gps\sirf\*.* %DESDIR%\MS_Customize\source\product\driver\gps\sirf\

XCOPY /R /Y %EXPORT_SOURCE_ROOT%\MS_Customize\source\product\config\sc6500_modem\mem_cfg.c %DESDIR%\MS_Customize\source\product\config\sc6500_modem\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\MS_Customize\source\product\config\sc6500_modem\pinmap_cfg.c %DESDIR%\MS_Customize\source\product\config\sc6500_modem\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\MS_Customize\source\product\config\sc6500_modem\spiflash_cfg.c %DESDIR%\MS_Customize\source\product\config\sc6500_modem\

XCOPY /R /Y %EXPORT_SOURCE_ROOT%\chip_drv\chip_plf\sc6530\sc6530_spi_cfg.c %DESDIR%\chip_drv\chip_plf\sc6530\

XCOPY /R /Y %EXPORT_SOURCE_ROOT%\chip_drv\chip_module\uart\uart_hal.c %DESDIR%\chip_drv\chip_module\uart\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\chip_drv\chip_module\uart\uart_phy.h %DESDIR%\chip_drv\chip_module\uart\

XCOPY /R /Y %EXPORT_SOURCE_ROOT%\chip_drv\chip_module\uart\V5\uart_phy_v5.c %DESDIR%\chip_drv\chip_module\uart\v5\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\chip_drv\chip_module\gpio\gpio_hal.c %DESDIR%\chip_drv\chip_module\gpio\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\chip_drv\chip_module\gpio\V5\gpio_phy_v5.c %DESDIR%\chip_drv\chip_module\gpio\v5\

XCOPY /R /Y %EXPORT_SOURCE_ROOT%\chip_drv\chip_module\i2c\i2c_hal.c %DESDIR%\chip_drv\chip_module\i2c\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\chip_drv\export\inc\i2c_drvapi.h %DESDIR%\chip_drv\export\inc\
XCOPY /R /Y %EXPORT_SOURCE_ROOT%\chip_drv\export\inc\com_drvapi.h %DESDIR%\chip_drv\export\inc\

endlocal