#include <vm_common.h>
#include "nativePWMManager.h"

/**
 * Class:     iot_oem_pwm_PWMManager
 * Method:    config0
 * Signature: (III)I
 */
void Java_iot_oem_pwm_PWMManager_config0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint id = (jint) args[1];
    jint freq = (jint) args[2];
    jint dutyCycle = (jint) args[3];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret)
}

/**
 * Class:     iot_oem_pwm_PWMManager
 * Method:    command0
 * Signature: (III)I
 */
void Java_iot_oem_pwm_PWMManager_command0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint id = (jint) args[1];
    jint cmd = (jint) args[2];
    jint arg = (jint) args[3];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret)
}

