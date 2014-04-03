#include "vm_common.h"
#include "kni.h"

#include <nativeAsyncIO.h>
#include <nativeClass.h>
#include <nativeDThread.h>
#include <nativeFloat.h>
#include <nativeMath.h>
#include <nativeNet.h>
#include <nativeObject.h>
#include <nativeRuntime.h>
#include <nativeString.h>
#include <nativeSystem.h>
#include <nativeThread.h>
#include <nativeThrowable.h>
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
