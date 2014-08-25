#include <vm_common.h>
#include "nativeBatteryManager.h"

#if defined(ARCH_ARM_SPD)
#include "sci_types.h"
#include "os_api.h"
#include "chg_drvapi.h"
#endif

#define BATTERY_VOLTAGE_FULL 3900
#define BATTERY_VOLTAGE_WARNING 3500

#define STATIC_iot_oem_battery_BatteryManager_BATTERY_STATUS_CHARGING 0
#define STATIC_iot_oem_battery_BatteryManager_BATTERY_STATUS_FULL 1
#define STATIC_iot_oem_battery_BatteryManager_BATTERY_STATUS_NORMAL 2
#define STATIC_iot_oem_battery_BatteryManager_BATTERY_STATUS_LOW 3
#define STATIC_iot_oem_battery_BatteryManager_BATTERY_STATUS_UNKNOW 4

/**
 * Class:     iot_oem_battery_BatteryManager
 * Method:    getStatus0
 * Signature: ()I
 */
void Java_iot_oem_battery_BatteryManager_getStatus0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = STATIC_iot_oem_battery_BatteryManager_BATTERY_STATUS_UNKNOW;
    uint32 vol = 0;

#if defined(ARCH_ARM_SPD)
    //TODO: check if need to call CHGMNG_Init()
    if (CHGMNG_IsChargeConnect())
    {
        ret = STATIC_iot_oem_battery_BatteryManager_BATTERY_STATUS_CHARGING;
        goto end;
    }

    vol = CHR_GetCurVoltage();

    if (vol > BATTERY_VOLTAGE_FULL)
    {
        ret = STATIC_iot_oem_battery_BatteryManager_BATTERY_STATUS_FULL;
    }
    else if (vol < BATTERY_VOLTAGE_WARNING)
    {
        ret = STATIC_iot_oem_battery_BatteryManager_BATTERY_STATUS_LOW;
    }
    else
    {
        ret = STATIC_iot_oem_battery_BatteryManager_BATTERY_STATUS_NORMAL;
    }
#endif
end:
    DthingTraceD("[INFO][Battery] do get status %d", ret);
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
    DthingTraceD("[INFO][Battery] do get temperature %d", ret);
    RETURN_INT(ret);
}

