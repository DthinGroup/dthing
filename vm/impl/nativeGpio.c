#include <vm_common.h>
#include "nativeGpio.h"

/**
 * Class:     iot_oem_gpio_Gpio
 * Method:    open0
 * Signature: (I)I
 */
void Java_iot_oem_gpio_Gpio_open0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint number = (jint) args[1];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_gpio_Gpio
 * Method:    setCurrentMode0
 * Signature: (II)I
 */
void Java_iot_oem_gpio_Gpio_setCurrentMode0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint number = (jint) args[1];
    jint mode = (jint) args[2];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_gpio_Gpio
 * Method:    getCurrentMode0
 * Signature: (I)I
 */
void Java_iot_oem_gpio_Gpio_getCurrentMode0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint number = (jint) args[1];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_gpio_Gpio
 * Method:    read0
 * Signature: (I)I
 */
void Java_iot_oem_gpio_Gpio_read0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint number = (jint) args[1];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_gpio_Gpio
 * Method:    write0
 * Signature: (IZ)I
 */
void Java_iot_oem_gpio_Gpio_write0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint number = (jint) args[1];
    jboolean value = (jboolean) args[2];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_gpio_Gpio
 * Method:    registerInt0
 * Signature: (II)I
 */
void Java_iot_oem_gpio_Gpio_registerInt0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint number = (jint) args[1];
    jint type = (jint) args[2];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_gpio_Gpio
 * Method:    unregisterInt0
 * Signature: (I)I
 */
void Java_iot_oem_gpio_Gpio_unregisterInt0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint number = (jint) args[1];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_gpio_Gpio
 * Method:    close0
 * Signature: (I)I
 */
void Java_iot_oem_gpio_Gpio_close0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint number = (jint) args[1];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

