#include <utfstring.h>
#include <vm_common.h>
#include "nativeSimManager.h"

#if defined(ARCH_ARM_SPD)
#include "sci_types.h"
#include "os_api.h"
#include "mn_api.h"
#endif

/**
 * Class:     iot_oem_sim_SimManager
 * Method:    close0
 * Signature: ()I
 */
void Java_iot_oem_sim_SimManager_close0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_sim_SimManager
 * Method:    enablePinLock0
 * Signature: (Ljava/lang/String;)I
 */
void Java_iot_oem_sim_SimManager_enablePinLock0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    StringObject * pinCodeObj = (StringObject *) args[1];
    const jchar* pinCode = dvmGetStringData(pinCodeObj);
//    const char* pinCode = dvmCreateCstrFromString(pinCodeObj);
    int pinCodeLen = dvmGetStringLength(pinCodeObj);
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    MN_BLOCK_T password = {0};
	char * utf8Str = dvmCreateCstrFromString(pinCodeObj);
	password.blocklen = (pinCodeLen >= MN_MAX_BLOCK_LEN)? MN_MAX_BLOCK_LEN : pinCodeLen;
    memset(password.blockbuf, 0x0, MN_MAX_BLOCK_LEN);
	memcpy(password.blockbuf, utf8Str, password.blocklen);

	if (utf8Str != NULL)
	{
	    free(utf8Str);
	}

    if (ERR_MNPHONE_NO_ERR != MNPHONE_OperatePinEx(MN_DUAL_SYS_1, MNSIM_PIN_ENABLE, MNSIM_PIN1, &password, NULL))
    {
        ret = -1;
    }
    DthingTraceD("[INFO][SIM] do enable pin lock with result %u\n", ret);
#endif

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_sim_SimManager
 * Method:    disablePinLock0
 * Signature: (Ljava/lang/String;)I
 */
void Java_iot_oem_sim_SimManager_disablePinLock0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    StringObject * pinCodeObj = (StringObject *) args[1];
    const jchar* pinCode = dvmGetStringData(pinCodeObj);
//    const char* pinCode = dvmCreateCstrFromString(pinCodeObj);
    int pinCodeLen = dvmGetStringLength(pinCodeObj);
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    MN_BLOCK_T password = {0};
	char * utf8Str = dvmCreateCstrFromString(pinCodeObj);
    password.blocklen = (pinCodeLen >= MN_MAX_BLOCK_LEN)? MN_MAX_BLOCK_LEN : pinCodeLen;
    memset(password.blockbuf, 0x0, MN_MAX_BLOCK_LEN);
	memcpy(password.blockbuf, utf8Str, password.blocklen);

	if (utf8Str != NULL)
	{
	    free(utf8Str);
	}

    if (ERR_MNPHONE_NO_ERR != MNPHONE_OperatePinEx(MN_DUAL_SYS_1, MNSIM_PIN_DISABLE, MNSIM_PIN1, &password, NULL))
    {
        ret = -1;
    }

    DthingTraceD("[INFO][SIM] do disable pin lock with result %u\n", ret);
#endif

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_sim_SimManager
 * Method:    unlockPin0
 * Signature: (Ljava/lang/String;)I
 */
void Java_iot_oem_sim_SimManager_unlockPin0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    StringObject * pinCodeObj = (StringObject *) args[1];
    const jchar* pinCode = dvmGetStringData(pinCodeObj);
//    const char* pinCode = dvmCreateCstrFromString(pinCodeObj);
    int pinCodeLen = dvmGetStringLength(pinCodeObj);
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    MN_BLOCK_T password = {0};

    if(MNSIM_IsPin1EnableEx(MN_DUAL_SYS_1))//PIN1 activated
    {
	    char * utf8Str = dvmCreateCstrFromString(pinCodeObj);
        password.blocklen = (pinCodeLen >= MN_MAX_BLOCK_LEN)? MN_MAX_BLOCK_LEN : pinCodeLen;
        memset(password.blockbuf, 0x0, MN_MAX_BLOCK_LEN);
	    memcpy(password.blockbuf, utf8Str, password.blocklen);

	    if (utf8Str != NULL)
	    {
	        free(utf8Str);
	    }

        if (ERR_MNPHONE_NO_ERR != MNPHONE_ResponsePinEx(MN_DUAL_SYS_1, MNSIM_PIN1, FALSE, &password, NULL))
        {
            ret = -1;
        }
    }
    else
    {
        ret = -1;
    }

    DthingTraceD("[INFO][SIM] do unlock pin with result %u\n", ret);
#endif

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_sim_SimManager
 * Method:    changePin0
 * Signature: (Ljava/lang/String;Ljava/lang/String;)I
 */
void Java_iot_oem_sim_SimManager_changePin0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    StringObject * oldPinObj = (StringObject *) args[1];
    const jchar* oldPin = dvmGetStringData(oldPinObj);
//    const char* oldPin = dvmCreateCstrFromString(oldPinObj);
    int oldPinLen = dvmGetStringLength(oldPinObj);
    StringObject * newPinObj = (StringObject *) args[2];
    const jchar* newPin = dvmGetStringData(newPinObj);
//    const char* newPin = dvmCreateCstrFromString(newPinObj);
    int newPinLen = dvmGetStringLength(newPinObj);
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    MN_BLOCK_T password = {0};
    MN_BLOCK_T new_password = {0};

    if(MNSIM_IsPin1EnableEx(MN_DUAL_SYS_1))//PIN1 activated
    {
		char * utf8Str = dvmCreateCstrFromString(oldPinObj);
        password.blocklen = (oldPinLen >= MN_MAX_BLOCK_LEN)? MN_MAX_BLOCK_LEN : oldPinLen;
        memset(password.blockbuf, 0x0, MN_MAX_BLOCK_LEN);
	    memcpy(password.blockbuf, utf8Str, password.blocklen);

	    if (utf8Str != NULL)
	    {
	        free(utf8Str);
	    }

		utf8Str = dvmCreateCstrFromString(newPinObj);
        new_password.blocklen = (newPinLen >= MN_MAX_BLOCK_LEN)? MN_MAX_BLOCK_LEN : newPinLen;
        memset(new_password.blockbuf, 0x0, MN_MAX_BLOCK_LEN);
	    memcpy(new_password.blockbuf, utf8Str, new_password.blocklen);

	    if (utf8Str != NULL)
	    {
	        free(utf8Str);
	    }

        if (ERR_MNPHONE_NO_ERR != MNPHONE_OperatePinEx(MN_DUAL_SYS_1, MNSIM_PIN_CHANGE, MNSIM_PIN1, &password, &new_password))
        {
            ret = -1;
        }
    }
    else
    {
        ret = -1;
    }

    DthingTraceD("[INFO][SIM] do change pin with result %u\n", ret);
#endif

    RETURN_INT(ret);
}

