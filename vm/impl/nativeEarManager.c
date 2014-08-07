#include <vm_common.h>
#include "nativeEarManager.h"

#if defined(ARCH_ARM_SPD)
#include "audio_api.h"
#endif

/**
 * Class:     iot_oem_ear_EarManager
 * Method:    init0
 * Signature: ()I
 */
void Java_iot_oem_ear_EarManager_init0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_ear_EarManager
 * Method:    getStatus0
 * Signature: ()I
 */
void Java_iot_oem_ear_EarManager_getStatus0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = 0;
#if defined(ARCH_ARM_SPD)
    AUDIO_DEVICE_MODE_TYPE_E mode = 0;

    if (AUDIO_NO_ERROR != AUDIO_GetDevMode(&mode))
    {
        ret = -1;
    }
    else
    {
        if (mode == AUDIO_DEVICE_MODE_EARPHONE)
        {
            ret = 1;
        }
    }
#endif
    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_ear_EarManager
 * Method:    setStatus0
 * Signature: (Z)I
 */
void Java_iot_oem_ear_EarManager_setStatus0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jboolean status = (jboolean) args[1];
    jint ret = 1;
#if defined(ARCH_ARM_SPD)
    if (status)
    {
        AUDIO_SetDevMode(AUDIO_DEVICE_MODE_EARPHONE);
        AUDIO_SetVolume(7); //FIXME:
    }
    else
    {
        AUDIO_SetDevMode(AUDIO_DEVICE_MODE_HANDHOLD);
    }
#endif
    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_ear_EarManager
 * Method:    close0
 * Signature: ()I
 */
void Java_iot_oem_ear_EarManager_close0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
	AUDIO_SetDevMode(AUDIO_DEVICE_MODE_HANDHOLD);
#endif
    RETURN_INT(ret);
}

