MINCPATH  = chip_drv/export/inc
MINCPATH += chip_drv/export/inc/outdated 
MINCPATH += CAF/Interface/source/resource 
MINCPATH += CAF/Template/include 
MINCPATH += DAPS/source/wbxml_parser/inc 
MINCPATH += DAPS/source/wsp_header_decoder/inc
MINCPATH += DAPS/source/xml_expat/inc 
MINCPATH += Base/l4/export/inc 
MINCPATH += Base/PS/export/inc 
MINCPATH += PARSER/export/inc 
MINCPATH += Base/sim/export/inc 
MINCPATH += external/juniversalchardet/include
MINCPATH += external/juniversalchardet/c/include
MINCPATH += BASE/myriad/h/svm_tgt
MINCPATH += BASE/myriad/h
MINCPATH += build/$(PROJECT)_builddir/tmp
MINCPATH += ms_customize/export/inc
MINCPATH += Base/layer1/export/inc
MINCPATH += MS_Ref/export/inc
MINCPATH += rtos/export/inc
MINCPATH += common/export/inc/trace_id
MINCPATH += common/export/inc
MINCPATH += ms_customize/source/product/driver/gps/sirf
MINCPATH += Base/atc/export/inc
MINCPATH += Base/atc/modem/source/h

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
           ansicextend.c encoding.c leb128.c properties.c trace.c ams_sms.c \
           Eventsystem.c jarparser.c ams.c ams_remote.c ams_utils.c vmTime.c vm_app.c \
           nativeAsyncIO.c nativeClass.c nativeFile.c nativeFileInputStream.c nativeFileOutputStream.c \
           nativeFloat.c nativeMath.c nativeNetNativeBridge.c nativeObject.c nativeOTADownload.c nativeRuntime.c \
           nativeScheduler.c nativeString.c nativeSystem.c nativeThread.c nativeThrowable.c \
           nativeTimeZone.c init.c accesscheck.c annotation.c array.c AsyncIO.c class.c classmisc.c \
           compact.c dexcatch.c dexclass.c dexfile.c dexproto.c dthread.c dvmdex.c exception.c gc.c hash.c \
           heap.c interpApi.c interpCore.c interpOpcode.c interpStack.c kni.c mm.c native.c object.c \
           rawdexfile.c resolve.c schd.c sync.c typecheck.c upcall.c utfstring.c voiderr.c \
           adler32.c compress.c crc32.c deflate.c infback.c inffast.c inflate.c inftrees.c trees.c uncompr.c zutil.c \
           voiderr.c opl_core.c opl_es.c opl_file.c opl_mm.c opl_net.c opl_rams.c opl_trace.c nativeADCManager.c nativeATCommandConnection.c \
           nativeBatteryManager.c nativeEarManager.c nativeGpio.c nativeI2CManager.c nativeLCD.c nativeMicophone.c \
           nativeMyMessageSender.c nativeMySmsConnectionThread.c nativePCMChannel.c nativePWMManager.c nativeSimManager.c \
           nativeSpeakerManager.c nativeSPIManager.c nativeUSBConnection.c nativeCommConnectionImpl.c nativeRawKeyPdDriver.c opl_atc.c