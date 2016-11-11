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
MINCPATH += BASE/dthing/mqtt/inc

MSRCPATH  = BASE/dthing/main
MSRCPATH += BASE/dthing/base/src
MSRCPATH += BASE/dthing/porting/src
MSRCPATH += BASE/dthing/rams/src
MSRCPATH += BASE/dthing/vm/impl
MSRCPATH += BASE/dthing/vm/src
MSRCPATH += BASE/dthing/vm
#MSRCPATH += BASE/dthing/www/src
MSRCPATH += BASE/dthing/zlib
MSRCPATH += BASE/dthing/mqtt/src/spreadtrum

# basic porting files,please add by types
#============================================
#core
SOURCES	 = dthing_main.c \
           vmTime.c

#ams
SOURCES	+= ams.c ams_remote.c ams_utils.c ams_sms.c

#opl layer
SOURCES	+= opl_core.c opl_es.c opl_file.c opl_mm.c opl_net.c opl_rams.c opl_atc.c   opl_trace.c  opl_mqtt_transport.c
#mqtt
SOURCES	+= test_mqtt.c MQTTLocal.c MQTTTest.c
#native
SOURCES	+= nativeADCManager.c nativeATCommandConnection.c \
           nativeBatteryManager.c nativeEarManager.c nativeGpio.c nativeI2CManager.c nativeLCD.c nativeMicophone.c \
           nativeMyMessageSender.c nativeMySmsConnectionThread.c nativePCMChannel.c nativePWMManager.c nativeSimManager.c \
           nativeSpeakerManager.c nativeSPIManager.c nativeUSBConnection.c nativeCommConnectionImpl.c nativeRawKeyPdDriver.c \
           nativeNativeAPIManager.c nativeDevice.c
#============================================