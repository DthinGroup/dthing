#include <utfstring.h>
#include <vm_common.h>
#include <opl_file.h>
#include <ams.h>
#include "nativeOTADownload.h"

/**
 * Class:     java_net_ota_OTADownload
 * Method:    getAppInstalledDir
 * Signature: ()Ljava/lang/String;
 */
void Java_java_net_ota_OTADownload_getAppInstalledDir(const u4* args, JValue* pResult) {
    StringObject * retObj = NULL;

    // TODO: initialize retObj via dvmCreateStringFrom* methods:
    retObj = dvmCreateStringFromCstr(file_getDthingDir());
    RETURN_PTR(retObj);
}

/**
 * Class:     java_net_ota_OTADownload
 * Method:    notifyOTAResult0
 * Signature: (I)V
 */
void Java_java_net_ota_OTADownload_notifyOTAResult0(const u4* args, JValue* pResult) {
    jint result = (jint) args[1];

	uint8_t	retBuf[4] = {0x0,};
    uint8_t	*pByte;
    SafeBuffer	*safeBuf;

	pByte = (uint8_t*)retBuf;
	writebeIU32(&pByte[0], result);
	safeBuf = CreateSafeBufferByBin(retBuf, sizeof(retBuf));

    // TODO: implementation
	DthingTraceD("OTA result: %d\n",result);
//	ams_remote_sendOTAExeResult(result);
	Ams_handleAck(Ams_getCurCrtlModule(),AMS_FASM_STATE_ACK_OTA,(void*)safeBuf);

    RETURN_VOID();
}

