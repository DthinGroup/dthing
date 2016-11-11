#include <utfstring.h>
#include <vm_common.h>
#include "nativeDevice.h"

#ifdef ARCH_ARM_SPD	
#include <mn_api.h>
#include <atc_common.h>
#endif

/**
 * Class:     com_yarlungsoft_util_Device
 * Method:    getImei0
 * Signature: ()Ljava/lang/String;
 */
void Java_com_yarlungsoft_util_Device_getImei0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    StringObject * retObj = NULL;
	char * fake = "unsupport";
#ifdef ARCH_ARM_SPD	

	#define MN_MAX_IMEI_LENGTH 8

	MN_IMEI_T           imei_arr; // the imei array
    unsigned char       temp_str[20] = {0};
    // get the manufactory name
    if (!MNNV_GetIMEIEx(MN_DUAL_SYS_1, imei_arr))
    {
        srand(SCI_GetTickCount());
        sprintf(&temp_str[0], "%d", rand());
		retObj = dvmCreateStringFromCstr(&temp_str[0]);
    } else {
		ConvertBcdToDigitalStr(MN_MAX_IMEI_LENGTH, imei_arr, &temp_str[0]);
		retObj = dvmCreateStringFromCstr(&temp_str[0]);
	}	
#else
	retObj = dvmCreateStringFromCstr(fake);	
#endif
	
    RETURN_PTR(retObj);
}

/**
 * Class:     com_yarlungsoft_util_Device
 * Method:    restartDevice0
 * Signature: ()V
 */
void Java_com_yarlungsoft_util_Device_restartDevice0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];

    //WORKAROUND keywords: [dthing-workaround-nix-1] in workaround-readme.md
	DVMTraceJava("System Restart!!!!\n");
	(*(void (*)( ))0)();
}

