#include <vm_common.h>
#include "nativeMicophone.h"

#define STATIC_iot_oem_micophone_Micophone_CI_MIC 0
#define STATIC_iot_oem_micophone_Micophone_CI_HEADSET 1

#define STATIC_iot_oem_micophone_Micophone_CG_MUTE   0
#define STATIC_iot_oem_micophone_Micophone_CG_CLASS0 1
#define STATIC_iot_oem_micophone_Micophone_CG_CLASS1 2
#define STATIC_iot_oem_micophone_Micophone_CG_CLASS2 3
#define STATIC_iot_oem_micophone_Micophone_CG_CLASS3 4
#define STATIC_iot_oem_micophone_Micophone_CG_CLASS4 5
#define STATIC_iot_oem_micophone_Micophone_CG_CLASS5 6
#define STATIC_iot_oem_micophone_Micophone_CG_CLASS6 7
#define STATIC_iot_oem_micophone_Micophone_CG_MAX    8

/**
 * Class:     iot_oem_micophone_Micophone
 * Method:    open0
 * Signature: ()I
 */
void Java_iot_oem_micophone_Micophone_open0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_micophone_Micophone
 * Method:    getStatus0
 * Signature: ()I
 */
void Java_iot_oem_micophone_Micophone_getStatus0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = 1;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_micophone_Micophone
 * Method:    setStatus0
 * Signature: (Z)I
 */
void Java_iot_oem_micophone_Micophone_setStatus0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jboolean status = (jboolean) args[1];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_micophone_Micophone
 * Method:    getInPath0
 * Signature: ()I
 */
void Java_iot_oem_micophone_Micophone_getInPath0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = STATIC_iot_oem_micophone_Micophone_CI_MIC;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_micophone_Micophone
 * Method:    setInPath0
 * Signature: (I)I
 */
void Java_iot_oem_micophone_Micophone_setInPath0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint inPath = (jint) args[1];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_micophone_Micophone
 * Method:    getGain0
 * Signature: ()I
 */
void Java_iot_oem_micophone_Micophone_getGain0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = STATIC_iot_oem_micophone_Micophone_CG_CLASS0;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_micophone_Micophone
 * Method:    setGain0
 * Signature: (I)I
 */
void Java_iot_oem_micophone_Micophone_setGain0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint gain = (jint) args[1];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_micophone_Micophone
 * Method:    close0
 * Signature: ()I
 */
void Java_iot_oem_micophone_Micophone_close0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

