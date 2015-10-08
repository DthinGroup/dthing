#include <utfstring.h>
#include <vm_common.h>
#include "nativeLCD.h"

/**
 * Class:     iot_oem_lcd_LCD
 * Method:    setBackgroundStatus0
 * Signature: (Z)I
 */
void Java_iot_oem_lcd_LCD_setBackgroundStatus0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jboolean status = (jboolean) args[1];
    jint ret = 0;
#if defined(ARCH_ARM_SPD)
    if (status) {
        SPILCD_Open();
    } else {
        SPILCD_Close();
    }
#endif
    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_lcd_LCD
 * Method:    getBackgroundStatus0
 * Signature: ()I
 */
void Java_iot_oem_lcd_LCD_getBackgroundStatus0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_lcd_LCD
 * Method:    getHeight0
 * Signature: ()I
 */
void Java_iot_oem_lcd_LCD_getHeight0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_lcd_LCD
 * Method:    getWidth0
 * Signature: ()I
 */
void Java_iot_oem_lcd_LCD_getWidth0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_lcd_LCD
 * Method:    drawString0
 * Signature: (IILjava/lang/String;)I
 */
void Java_iot_oem_lcd_LCD_drawString0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint x = (jint) args[1];
    jint y = (jint) args[2];
    StringObject * valueObj = (StringObject *) args[3];
    const jchar* value = dvmGetStringData(valueObj);
//    const char* value = dvmCreateCstrFromString(valueObj);
    int valueLen = dvmGetStringLength(valueObj);
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_lcd_LCD
 * Method:    clean0
 * Signature: ()I
 */
void Java_iot_oem_lcd_LCD_clean0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

