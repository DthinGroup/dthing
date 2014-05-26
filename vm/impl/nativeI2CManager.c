#include <vm_common.h>
#include "nativeI2CManager.h"

/**
 * Class:     iot_oem_i2c_I2CManager
 * Method:    open0
 * Signature: (II)I
 */
void Java_iot_oem_i2c_I2CManager_open0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint busId = (jint) args[1];
    jint freq = (jint) args[2];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret)
}

/**
 * Class:     iot_oem_i2c_I2CManager
 * Method:    getRate0
 * Signature: (I)I
 */
void Java_iot_oem_i2c_I2CManager_getRate0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint handle = (jint) args[1];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret)
}

/**
 * Class:     iot_oem_i2c_I2CManager
 * Method:    setRate0
 * Signature: (II)I
 */
void Java_iot_oem_i2c_I2CManager_setRate0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint handle = (jint) args[1];
    jint rate = (jint) args[2];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret)
}

/**
 * Class:     iot_oem_i2c_I2CManager
 * Method:    close0
 * Signature: (I)I
 */
void Java_iot_oem_i2c_I2CManager_close0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint handle = (jint) args[1];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret)
}

/**
 * Class:     iot_oem_i2c_I2CManager
 * Method:    read0
 * Signature: (III[BI)I
 */
void Java_iot_oem_i2c_I2CManager_read0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint handle = (jint) args[1];
    jint addr = (jint) args[2];
    jint subAddr = (jint) args[3];
    ArrayObject * bufferArr = (ArrayObject *)args[4];
    jbyte * bufferArrPtr = (byte *)(KNI_GET_ARRAY_BUF(args[4]));
    int bufferArrLen = KNI_GET_ARRAY_LEN(args[4]);
    jint len = (jint) args[5];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret)
}

/**
 * Class:     iot_oem_i2c_I2CManager
 * Method:    write0
 * Signature: (III[BI)I
 */
void Java_iot_oem_i2c_I2CManager_write0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint handle = (jint) args[1];
    jint addr = (jint) args[2];
    jint subAddr = (jint) args[3];
    ArrayObject * dataArr = (ArrayObject *)args[4];
    jbyte * dataArrPtr = (byte *)(KNI_GET_ARRAY_BUF(args[4]));
    int dataArrLen = KNI_GET_ARRAY_LEN(args[4]);
    jint len = (jint) args[5];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret)
}

