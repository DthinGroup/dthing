#include <vm_common.h>
#include "nativeMySmsConnectionThread.h"

/**
 * Class:     jp_co_cmcc_message_sms_MySmsConnectionThread
 * Method:    nReadMessage
 * Signature: ()Z
 */
void Java_jp_co_cmcc_message_sms_MySmsConnectionThread_nReadMessage(const u4* args, JValue* pResult) {
    jboolean ret = FALSE;

    // TODO: implementation
    RETURN_BOOLEAN(ret);
}

/**
 * Class:     jp_co_cmcc_message_sms_MySmsConnectionThread
 * Method:    nDeleteMessage
 * Signature: ()V
 */
void Java_jp_co_cmcc_message_sms_MySmsConnectionThread_nDeleteMessage(const u4* args, JValue* pResult) {

    // TODO: implementation

    // return type : void
}

/**
 * Class:     jp_co_cmcc_message_sms_MySmsConnectionThread
 * Method:    nRegister
 * Signature: ()Z
 */
void Java_jp_co_cmcc_message_sms_MySmsConnectionThread_nRegister(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jboolean ret = FALSE;

    // TODO: implementation
    RETURN_BOOLEAN(ret);
}

/**
 * Class:     jp_co_cmcc_message_sms_MySmsConnectionThread
 * Method:    nUnregister
 * Signature: ()V
 */
void Java_jp_co_cmcc_message_sms_MySmsConnectionThread_nUnregister(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];

    // TODO: implementation

    // return type : void
}

