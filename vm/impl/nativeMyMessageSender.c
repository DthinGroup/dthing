#include <vm_common.h>
#include "nativeMyMessageSender.h"

/**
 * Class:     jp_co_cmcc_message_sms_MyMessageSender
 * Method:    nSend
 * Signature: (Ljava/lang/String;III[BI)Z
 */
void Java_jp_co_cmcc_message_sms_MyMessageSender_nSend(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    StringObject * addressObj = (StringObject *) args[1];
    const jchar* address = dvmGetStringData(addressObj);
//    const char* address = dvmCreateCstrFromString(addressObj);
    int addressLen = dvmGetStringLength(addressObj);
    jint srcPort = (jint) args[2];
    jint dstPort = (jint) args[3];
    jint type = (jint) args[4];
    ArrayObject * dataBAArr = (ArrayObject *)args[5];
    jbyte * dataBAArrPtr = (byte *)(KNI_GET_ARRAY_BUF(args[5]));
    int dataBAArrLen = KNI_GET_ARRAY_LEN(args[5]);
    jint dataLen = (jint) args[6];
    jboolean ret = FALSE;

    // TODO: implementation
    RETURN_BOOLEAN(ret)
}

