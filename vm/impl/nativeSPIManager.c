#include <vm_common.h>
#include "nativeSPIManager.h"

/**
 * Class:     iot_oem_spi_SPIManager
 * Method:    open0
 * Signature: (II)I
 */
void Java_iot_oem_spi_SPIManager_open0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint busId = (jint) args[1];
    jint rate = (jint) args[2];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret)
}

/**
 * Class:     iot_oem_spi_SPIManager
 * Method:    getRate0
 * Signature: (I)I
 */
void Java_iot_oem_spi_SPIManager_getRate0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint busId = (jint) args[1];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret)
}

/**
 * Class:     iot_oem_spi_SPIManager
 * Method:    setRate0
 * Signature: (II)I
 */
void Java_iot_oem_spi_SPIManager_setRate0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint busId = (jint) args[1];
    jint rate = (jint) args[2];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret)
}

/**
 * Class:     iot_oem_spi_SPIManager
 * Method:    close0
 * Signature: (I)I
 */
void Java_iot_oem_spi_SPIManager_close0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint busId = (jint) args[1];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret)
}

/**
 * Class:     iot_oem_spi_SPIManager
 * Method:    read0
 * Signature: (II[BI)I
 */
void Java_iot_oem_spi_SPIManager_read0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint busId = (jint) args[1];
    jint addr = (jint) args[2];
    ArrayObject * bufferArr = (ArrayObject *)args[3];
    jbyte * bufferArrPtr = (byte *)(KNI_GET_ARRAY_BUF(args[3]));
    int bufferArrLen = KNI_GET_ARRAY_LEN(args[3]);
    jint len = (jint) args[4];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret)
}

/**
 * Class:     iot_oem_spi_SPIManager
 * Method:    write0
 * Signature: (II[BI)I
 */
void Java_iot_oem_spi_SPIManager_write0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint busId = (jint) args[1];
    jint addr = (jint) args[2];
    ArrayObject * dataArr = (ArrayObject *)args[3];
    jbyte * dataArrPtr = (byte *)(KNI_GET_ARRAY_BUF(args[3]));
    int dataArrLen = KNI_GET_ARRAY_LEN(args[3]);
    jint len = (jint) args[4];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret)
}

