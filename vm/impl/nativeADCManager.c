#include <vm_common.h>
#include "nativeADCManager.h"

#if defined(ARCH_ARM_SPD)
#include "sci_types.h"
#include "os_api.h"
#include "adc_drvapi.h"
#endif

/**
 * Class:     iot_oem_adc_ADCManager
 * Method:    init0
 * Signature: ()I
 */
void Java_iot_oem_adc_ADCManager_init0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    ret = ADC_Init();
#endif
    DthingTraceD("[INFO][ADCManager] do init with result %d\n", ret);
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
    jint ret = -100;

#if defined(ARCH_ARM_SPD)
    if ((cid >= 0) && (cid < ADC_MAX))
    {
        ret = ADC_GetResultDirectly(cid, ADC_SCALE_3V);  	
    }
#endif
    DthingTraceD("[INFO][ADCManager] do read %d with result %d\n", cid, ret);
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

    // TODO: Nothing need to do 
    DthingTraceD("[INFO][ADCManager] do destroy");
    RETURN_INT(ret);
}

