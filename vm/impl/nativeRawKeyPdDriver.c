#include <vm_common.h>
#include "nativeRawKeyPdDriver.h"

#if defined(ARCH_ARM_SPD)
#include "sci_types.h"
#include "os_api.h"
#include "Keypad.h"
#include "AsyncIO.h"

static ASYNC_Notifier *g_keyEventNotifier = NULL;
static jlong convertNativeKeyToJava(KPDSVR_SIG_T *key_sig_ptr);
#endif //ARCH_ARM_SPD

/**
 * Class:     iot_oem_kpd_RawKeyPdDriver
 * Method:    init0
 * Signature: ()I
 */
void Java_iot_oem_kpd_RawKeyPdDriver_init0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_kpd_RawKeyPdDriver
 * Method:    getKey0
 * Signature: ()J
 */
void Java_iot_oem_kpd_RawKeyPdDriver_getKey0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jlong ret = 0;

#if defined(ARCH_ARM_SPD)
    KPDSVR_SIG_T *key_sig_ptr = 0x0;

    if (AsyncIO_firstCall())
    {
        g_keyEventNotifier =  Async_getCurNotifier();
        key_sig_ptr = (KPDSVR_SIG_T *)SCI_GetSignal(KPDSVR);

        if (key_sig_ptr == 0x0)
        {
            goto end;
        }

        if(key_sig_ptr->key < KPD_VK_NUM)
        {
            //Convert Key
            ret = convertNativeKeyToJava(key_sig_ptr);
        }

        SCI_FREE(key_sig_ptr);
        key_sig_ptr = 0x0;
    }

end:
    g_keyEventNotifier = NULL;

#endif //ARCH_ARM_SPD
    RETURN_LONG(ret);
}


/**
 * Class:     iot_oem_kpd_RawKeyPdDriver
 * Method:    close0
 * Signature: ()I
 */
void Java_iot_oem_kpd_RawKeyPdDriver_close0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

#if defined(ARCH_ARM_SPD)
static jlong convertNativeKeyToJava(KPDSVR_SIG_T *key_sig_ptr)
{
  jlong keyCode = 0;

  if (key_sig_ptr == 0x0)
  {
    return keyCode;
  }

  switch(key_sig_ptr->SignalCode)
  {
  case KPD_DOWN:
    keyCode = (jlong)key_sig_ptr->key;
    DthingTraceD("[KeyPd] key %d down\n", keyCode);
    break;
  case KPD_UP:
    keyCode = (jlong)key_sig_ptr->key;
    DthingTraceD("[KeyPd] key %d down\n", keyCode);
    break;
  default:
    keyCode = (jlong)key_sig_ptr->key;
    DthingTraceD("[KeyPd] get key %d with unknown status %d\n", key_sig_ptr->key, key_sig_ptr->SignalCode);
    break;
  }
  return keyCode;
}
#endif //ARCH_ARM_SPD