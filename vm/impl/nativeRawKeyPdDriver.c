#include <vm_common.h>
#include "nativeRawKeyPdDriver.h"

#if defined(ARCH_ARM_SPD)
#include "sci_types.h"
#include "os_api.h"
#include "AsyncIO.h"

#include "keypad.h"
#include "dal_keypad.h"
#include "sci_service.h"

static ASYNC_Notifier *g_keyEventNotifier = NULL;
void kpd_callback (uint32 id, uint32 argc, void *argv);
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

    DthingTraceD("keypad: init and register key message callback\n");

#if defined(ARCH_ARM_SPD)
    SCI_RegisterMsg(KEYPAD_SERVICE,
                   (uint8)(KPD_DOWN & 0xff),
                   (uint8)(KPD_UP & 0xff),
                   kpd_callback);
#endif //ARCH_ARM_SPD

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

    if (AsyncIO_firstCall())
    {
        g_keyEventNotifier =  Async_getCurNotifier();
        DthingTraceD("keypad: ask for key and waiting\n");
        AsyncIO_callAgainWhenSignalled();
    }
    else
    {
        ret = g_keyEventNotifier->notified;
        DthingTraceD("keypad: get key code %d\n", ret);
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

    DthingTraceD("keypad: init and unregister key message callback\n");

#if defined(ARCH_ARM_SPD)
    SCI_UnregisterMsg(KEYPAD_SERVICE,
                     (uint8)(KPD_DOWN & 0xff),
                     (uint8)(KPD_UP & 0xff),
                     NULL);
#endif //ARCH_ARM_SPD
    RETURN_INT(ret);
}

#if defined(ARCH_ARM_SPD)
/**
 * @brief callback when keypad is pressed
 *
 * @param id Server ID, EXAMPLE: KEYPAD_SERVICE
 * @param argc ui_event_type, Message ID of the registered event
 * @param argv signal_ptr, Signal Sent to Client
 */
void kpd_callback (uint32 id, uint32 argc, void *argv)
{
  uint32  key_code = 0;
  uint16  event_code = 0;

  DthingTraceD("keypad: kpd_callback\n");

  if (g_keyEventNotifier != NULL)
  {
    if (id == KEYPAD_SERVICE)
    {
      key_code = (uint32)(argv);
      event_code = (uint16)(argc & 0xFFFF);

      switch(event_code)
      {
      case KPD_DOWN:
        g_keyEventNotifier->notified = key_code;
        DthingTraceD("keypad: KPD_DOWN %d\n", key_code);
        break;
      case KPD_UP:
        g_keyEventNotifier->notified = key_code;
        DthingTraceD("keypad: KPD_UP %d\n", key_code);
        break;
      default:
        g_keyEventNotifier->notified = 0;
        DthingTraceD("keypad: Unknown key event %d\n", event_code);
        break;
      }
      AsyncIO_notify(g_keyEventNotifier);
    }
  }
}

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