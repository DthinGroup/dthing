#include <vm_common.h>
#include "nativeUSBConnection.h"

/**
 * Class:     iot_oem_usb_USBConnection
 * Method:    open0
 * Signature: (Ljava/lang/String;)I
 */
void Java_iot_oem_usb_USBConnection_open0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    StringObject * usbIDObj = (StringObject *) args[1];
    const jchar* usbID = dvmGetStringData(usbIDObj);
//    const char* usbID = dvmCreateCstrFromString(usbIDObj);
    int usbIDLen = dvmGetStringLength(usbIDObj);
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret)
}

/**
 * Class:     iot_oem_usb_USBConnection
 * Method:    close0
 * Signature: (I)I
 */
void Java_iot_oem_usb_USBConnection_close0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint handle = (jint) args[1];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret)
}

