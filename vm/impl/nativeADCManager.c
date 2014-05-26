#include <vm_common.h>
#include "nativeADCManager.h"

/**
 * Class:     iot_oem_adc_ADCManager
 * Method:    init0
 * Signature: ()I
 */
void Java_iot_oem_adc_ADCManager_init0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_adc_ADCManager
 * Method:    read0
 * Signature: (I)I
 */
void Java_iot_oem_adc_ADCManager_read0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint cid = (jint) args[1];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_adc_ADCManager
 * Method:    destroy0
 * Signature: ()I
 */
void Java_iot_oem_adc_ADCManager_destroy0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

