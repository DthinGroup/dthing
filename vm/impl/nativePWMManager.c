#include <vm_common.h>
#include "nativePWMManager.h"

#if defined(ARCH_ARM_SPD)
#include "sci_types.h"
#include "os_api.h"
#include "pwm_drvapi.h"
#endif

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

#if defined(ARCH_ARM_SPD)
    PWM_Config (id,  (uint)freq,  (uint16)dutyCycle);
    SCI_Sleep(1000);
    DVMTraceDbg("[INFO][PWMManager] config with id[%d] freq[%d] dutyCycle[%d]\n", id, freq, dutyCycle);
#endif

    RETURN_INT(ret);
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

#if defined(ARCH_ARM_SPD)
    uint32 myArg = (uint32)arg;

    PWM_Ioctl ((uint32)id, (uint32)cmd, &myArg);
    DVMTraceDbg("[INFO][PWMManager] ioctl with id[%d] cmd[%d] arg[%d]\n", id, cmd, arg);
	ret = myArg;
#endif

    RETURN_INT(ret);
}

