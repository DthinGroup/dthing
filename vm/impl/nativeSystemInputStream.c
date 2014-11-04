#include <vm_common.h>
#include "nativeSystemInputStream.h"

/**
 * Class:     com_yarlungsoft_util_SystemInputStream
 * Method:    readN
 * Signature: ([BII)I
 */
void Java_com_yarlungsoft_util_SystemInputStream_readN(const u4* args, JValue* pResult) {
    ArrayObject * bArr = (ArrayObject *)args[0];
    jbyte * bArrPtr = (jbyte *)(KNI_GET_ARRAY_BUF(args[0]));
    int bArrLen = KNI_GET_ARRAY_LEN(args[0]);
    jint off = (jint) args[1];
    jint len = (jint) args[2];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

