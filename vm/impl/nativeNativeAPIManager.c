#include <vm_common.h>
#include "nativeNativeAPIManager.h"


#define API_ID_MIN 0
#define API_ID_1 1
#define API_ID_2 2
#define API_ID_3 3
#define API_ID_4 4
#define API_ID_5 5 
#define API_ID_6 6
#define API_ID_7 7
#define API_ID_8 8
#define API_ID_9 9
#define API_ID_MAX 10

/**
 * Class:     iot_oem_NativeAPI_NativeAPIManager
 * Method:    init0
 * Signature: ()I
 */
void Java_iot_oem_NativeAPI_NativeAPIManager_init0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = 0;
    DthingTraceD("[INFO][NAPI] init0");
    // TODO: implementation
    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_NativeAPI_NativeAPIManager
 * Method:    call0
 * Signature: (IZ)I
 */
void Java_iot_oem_NativeAPI_NativeAPIManager_call0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint cid = (jint) args[1];
    jboolean swt = (jboolean) args[2];
    jint ret = 0;
    DthingTraceD("[INFO][NAPI] call0");
    // TODO: implementation
  switch(cid)
  {
    case API_ID_1:// key pad led
    #if defined(ARCH_ARM_SPD)
        GPIO_SetKeyPadBackLight(swt);
    #endif
    break;
    case API_ID_2:// vibrate
    #if defined(ARCH_ARM_SPD)
        GPIO_SetVibrator(swt);
    #endif
    break;
    default:
        ret = -1;
    }
    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_NativeAPI_NativeAPIManager
 * Method:    destroy0
 * Signature: (I)I
 */
void Java_iot_oem_NativeAPI_NativeAPIManager_destroy0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint channelID = (jint) args[1];
    jint ret = 0;
    DthingTraceD("[INFO][NAPI] destroy0");
    // TODO: implementation
  switch(channelID)
  {
    case API_ID_1:// key pad led
    #if defined(ARCH_ARM_SPD)
        GPIO_SetKeyPadBackLight(0);
    #endif
    break;
    case API_ID_2:// vibrate
    #if defined(ARCH_ARM_SPD)
        GPIO_SetVibrator(0);
    #endif
    break;
    default:
        ret = -1;
    }
    RETURN_INT(ret);
}

