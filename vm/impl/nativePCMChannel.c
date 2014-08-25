#include <vm_common.h>
#include "nativePCMChannel.h"

#define STATIC_iot_oem_pcmchannel_PCMChannel_PCM_CLK_128k 0
#define STATIC_iot_oem_pcmchannel_PCMChannel_PCM_CLK_256k 1
#define STATIC_iot_oem_pcmchannel_PCMChannel_PCM_CLK_512k 2

#define STATIC_iot_oem_pcmchannel_PCMChannel_PCM_DATA_MSB 0
#define STATIC_iot_oem_pcmchannel_PCMChannel_PCM_DATA_LSB 1

/**
 * Class:     iot_oem_pcmchannel_PCMChannel
 * Method:    open0
 * Signature: ()I
 */
void Java_iot_oem_pcmchannel_PCMChannel_open0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_pcmchannel_PCMChannel
 * Method:    getStatus0
 * Signature: ()I
 */
void Java_iot_oem_pcmchannel_PCMChannel_getStatus0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = 1;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_pcmchannel_PCMChannel
 * Method:    setStatus0
 * Signature: (Z)I
 */
void Java_iot_oem_pcmchannel_PCMChannel_setStatus0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jboolean status = (jboolean) args[1];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_pcmchannel_PCMChannel
 * Method:    getFrequency0
 * Signature: ()I
 */
void Java_iot_oem_pcmchannel_PCMChannel_getFrequency0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = STATIC_iot_oem_pcmchannel_PCMChannel_PCM_CLK_128k;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_pcmchannel_PCMChannel
 * Method:    setFrequency0
 * Signature: (I)I
 */
void Java_iot_oem_pcmchannel_PCMChannel_setFrequency0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint freq = (jint) args[1];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_pcmchannel_PCMChannel
 * Method:    getMode0
 * Signature: ()I
 */
void Java_iot_oem_pcmchannel_PCMChannel_getMode0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = STATIC_iot_oem_pcmchannel_PCMChannel_PCM_DATA_MSB;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_pcmchannel_PCMChannel
 * Method:    setMode0
 * Signature: (I)I
 */
void Java_iot_oem_pcmchannel_PCMChannel_setMode0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint mode = (jint) args[1];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_pcmchannel_PCMChannel
 * Method:    close0
 * Signature: ()I
 */
void Java_iot_oem_pcmchannel_PCMChannel_close0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

