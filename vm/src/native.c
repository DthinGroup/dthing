#include "vm_common.h"
#include "kni.h"

#include <nativeADCManager.h>
#include <nativeAsyncIO.h>
#include <nativeATCommandConnection.h>
#include <nativeBatteryManager.h>
#include <nativeClass.h>
#include <nativeCommConnectionImpl.h>
#include <nativeDouble.h>
#include <nativeDThread.h>
#include <nativeEarManager.h>
#include <nativeFile.h>
#include <nativeFileInputStream.h>
#include <nativeFileOutputStream.h>
#include <nativeFloat.h>
#include <nativeGpio.h>
#include <nativeI2CManager.h>
#include <nativeLCD.h>
#include <nativeMain.h>
#include <nativeMath.h>
#include <nativeMicophone.h>
#include <nativeMyMessageSender.h>
#include <nativeMySmsConnectionThread.h>
#include <nativeNativeAPIManager.h>
#include <nativeNetNativeBridge.h>
#include <nativeObject.h>
#include <nativeOTADownload.h>
#include <nativePCMChannel.h>
#include <nativePWMManager.h>
#include <nativeRawKeyPdDriver.h>
#include <nativeRuntime.h>
#include <nativeScheduler.h>
#include <nativeSimManager.h>
#include <nativeSpeakerManager.h>
#include <nativeSPIManager.h>
#include <nativeString.h>
#include <nativeSystem.h>
#include <nativeSimpleMqttOps.h>
#include <nativeSystemInputStream.h>
#include <nativeSystemPrintStream.h>
#include <nativeThread.h>
#include <nativeThrowable.h>
#include <nativeTimeZone.h>
#include <nativeUSBConnection.h>
#include <nativeMethods.h>

KniFunc Kni_findFuncPtr(const Method * mthd) {
    ClassObject * clazz = NULL;
    KniFunc funcPtr = NULL;
    int i, j;

    DVM_ASSERT(mthd != NULL);
    clazz = mthd->clazz;
    if (clazz == NULL) {
        return funcPtr;
    }

    for (i = 0; i < NATIVE_CLASSES_COUNT; i++) {
        if (0 != DVM_STRCMP(gNativeMthTab[i].classpath, clazz->descriptor)) {
            continue;
        }
        for (j = 0; j < gNativeMthTab[i].methodCount; j++) {
            if (0 == DVM_STRCMP(gNativeMthTab[i].methods[j].name, mthd->name)) {
                funcPtr = (KniFunc) gNativeMthTab[i].methods[j].fnPtr;
                break;
            }
        }
    }
    return funcPtr;
}
