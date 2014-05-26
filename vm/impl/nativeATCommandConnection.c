#include <vm_common.h>
#include "nativeATCommandConnection.h"

/**
 * Class:     jp_co_cmcc_atcommand_ATCommandConnection
 * Method:    open0
 * Signature: ()I
 */
void Java_jp_co_cmcc_atcommand_ATCommandConnection_open0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret)
}

/**
 * Class:     jp_co_cmcc_atcommand_ATCommandConnection
 * Method:    send0
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
void Java_jp_co_cmcc_atcommand_ATCommandConnection_send0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    StringObject * atcmdObj = (StringObject *) args[1];
    const jchar* atcmd = dvmGetStringData(atcmdObj);
//    const char* atcmd = dvmCreateCstrFromString(atcmdObj);
    int atcmdLen = dvmGetStringLength(atcmdObj);
    StringObject * retObj = NULL;

    // TODO: initialize retObj via dvmCreateStringFrom* methods:
    // retObj = dvmCreateStringFromCstr("");
    RETURN_PTR(retObj);
}

/**
 * Class:     jp_co_cmcc_atcommand_ATCommandConnection
 * Method:    sendAsyn0
 * Signature: (Ljava/lang/String;)I
 */
void Java_jp_co_cmcc_atcommand_ATCommandConnection_sendAsyn0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    StringObject * atcmdObj = (StringObject *) args[1];
    const jchar* atcmd = dvmGetStringData(atcmdObj);
//    const char* atcmd = dvmCreateCstrFromString(atcmdObj);
    int atcmdLen = dvmGetStringLength(atcmdObj);
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret)
}

/**
 * Class:     jp_co_cmcc_atcommand_ATCommandConnection
 * Method:    get0
 * Signature: ()Ljava/lang/String;
 */
void Java_jp_co_cmcc_atcommand_ATCommandConnection_get0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    StringObject * retObj = NULL;

    // TODO: initialize retObj via dvmCreateStringFrom* methods:
    // retObj = dvmCreateStringFromCstr("");
    RETURN_PTR(retObj);
}

/**
 * Class:     jp_co_cmcc_atcommand_ATCommandConnection
 * Method:    close0
 * Signature: ()I
 */
void Java_jp_co_cmcc_atcommand_ATCommandConnection_close0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret)
}

