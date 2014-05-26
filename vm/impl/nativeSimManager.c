#include <utfstring.h>
#include <vm_common.h>
#include "nativeSimManager.h"

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

    // TODO: implementation

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

    // TODO: implementation

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

    // TODO: implementation

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

    // TODO: implementation

    RETURN_INT(ret);
}

