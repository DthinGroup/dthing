#include <vm_common.h>
#include "nativeBatteryManager.h"

/**
 * Class:     iot_oem_battery_BatteryManager
 * Method:    getStatus0
 * Signature: ()I
 */
void Java_iot_oem_battery_BatteryManager_getStatus0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_battery_BatteryManager
 * Method:    getTemperature0
 * Signature: ()I
 */
void Java_iot_oem_battery_BatteryManager_getTemperature0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

