#include <vm_common.h>
#include "nativeSpeakerManager.h"

#if defined(ARCH_ARM_SPD)
#include "audio_api.h"
#endif

#define JBNI_TRUE (1 == 1)
#define JBNI_FALSE (1 != 1)

/**
 * Class:     iot_oem_spk_SpeakerManager
 * Method:    init0
 * Signature: ()I
 */
void Java_iot_oem_spk_SpeakerManager_init0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = 0;

    //TODO:

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_spk_SpeakerManager
 * Method:    getStatus0
 * Signature: ()I
 */
void Java_iot_oem_spk_SpeakerManager_getStatus0(const u4* args, JValue* pResult) {
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
        if (mode == AUDIO_DEVICE_MODE_HANDFREE)
        {
            ret = 1;
        }
	}
#endif

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_spk_SpeakerManager
 * Method:    setStatus0
 * Signature: (Z)I
 */
void Java_iot_oem_spk_SpeakerManager_setStatus0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jboolean status = (jboolean) args[1];
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    if (JBNI_TRUE == status)
    {
        AUDIO_SetDevMode(AUDIO_DEVICE_MODE_HANDFREE);
        AUDIO_SetVolume(7);//FIXME: set to default volume
    }
    else
    {
        AUDIO_SetDevMode(AUDIO_DEVICE_MODE_HANDHOLD);
    }
#endif

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_spk_SpeakerManager
 * Method:    close0
 * Signature: ()I
 */
void Java_iot_oem_spk_SpeakerManager_close0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    AUDIO_SetDevMode(AUDIO_DEVICE_MODE_HANDHOLD);
#endif

    RETURN_INT(ret);
}

