MINCPATH =
MINCPATH += rtos/export/inc
MINCPATH += common/export/inc
MINCPATH += CAF/Template/include
#MINCPATH += wre/wrekernel/include/sdk
MINCPATH += BASE/PS/export/inc
MINCPATH += BASE/l4/export/inc
MINCPATH += BASE/sim/export/inc
#MINCPATH += MSL/gas/inc
MINCPATH += daps/export/inc

MINCPATH += BASE/dthing/base/inc
MINCPATH += BASE/dthing/porting/inc
MINCPATH += BASE/dthing/rams/inc
MINCPATH += BASE/dthing/vm/impl
MINCPATH += BASE/dthing/vm/inc
MINCPATH += BASE/dthing/vm
#MINCPATH += BASE/dthing/www/inc
MINCPATH += BASE/dthing/zlib

MSRCPATH  = BASE/dthing/main
MSRCPATH += BASE/dthing/base/src
MSRCPATH += BASE/dthing/porting/src
MSRCPATH += BASE/dthing/rams/src
MSRCPATH += BASE/dthing/vm/impl
MSRCPATH += BASE/dthing/vm/src
MSRCPATH += BASE/dthing/vm
#MSRCPATH += BASE/dthing/www/src
MSRCPATH += BASE/dthing/zlib

SOURCES	 = dthing_main.c \
           vmTime.c \
           opl_core.c opl_es.c opl_file.c opl_mm.c opl_net.c opl_rams.c \
           nativeADCManager.c nativeATCommandConnection.c nativeBatteryManager.c nativeEarManager.c nativeGpio.c nativeI2CManager.c nativeLCD.c nativeMicophone.c nativeMyMessageSender.c nativeMySmsConnectionThread.c \
           nativePCMChannel.c nativePWMManager.c nativeSimManager.c nativeSpeakerManager.c nativeSPIManager.c nativeUSBConnection.c

#nativeADCManager.c nativeAsyncIO.c nativeATCommandConnection.c nativeBatteryManager.c nativeClass.c nativeEarManager.c nativeFile.c nativeFileInputStream.c nativeFileOutputStream.c nativeFloat.c nativeGpio.c nativeI2CManager.c nativeLCD.c nativeMath.c nativeMicophone.c nativeMyMessageSender.c nativeMySmsConnectionThread.c nativeNet.c nativeObject.c nativeOTADownload.c nativePCMChannel.c nativePWMManager.c nativeRuntime.c nativeScheduler.c nativeSimManager.c nativeSpeakerManager.c nativeSPIManager.c nativeString.c nativeSystem.c nativeThread.c nativeThrowable.c nativeTimeZone.c nativeUSBConnection.c \