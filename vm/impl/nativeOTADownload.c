#include <utfstring.h>
#include <vm_common.h>
#include <opl_file.h>
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
	

    // TODO: implementation
	DthingTraceD("OTA result: %d\n",result);
	sendOTAExeResult(result);
	refreshInstalledApp();

    RETURN_VOID();
}

