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
static uint32 g_lastKeyCode = 0;
void kpd_callback (uint32 id, uint32 argc, void *argv);
#endif //ARCH_ARM_SPD

/**
 * Class:     iot_oem_kpd_RawKeyPdDriver
 * Method:    init0
 * Signature: ()I
 */
void Java_iot_oem_kpd_RawKeyPdDriver_init0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = 0;

    DVMTraceDbg("keypad: init and register key message callback\n");

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
        DVMTraceDbg("keypad: ask for key and waiting\n");
        AsyncIO_callAgainWhenSignalled();
    }
    else
    {
        ret = (jlong)g_lastKeyCode;
        g_keyEventNotifier = NULL;
        DVMTraceDbg("keypad: get key code %d\n", g_lastKeyCode);
    }

end:

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

    DVMTraceDbg("keypad: init and unregister key message callback\n");

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

  DVMTraceDbg("keypad: kpd_callback\n");

  if (g_keyEventNotifier != NULL)
  {
    //FIXME: Just ignore the incorrect service id
    //if (id == KEYPAD_SERVICE)
    {
      key_code = (uint32)(argv);
      event_code = (uint16)(argc & 0xFFFF);

      switch(event_code)
      {
      case KPD_DOWN:
        g_lastKeyCode = key_code;
        DVMTraceDbg("keypad: KPD_DOWN %d\n", key_code);
        break;
      case KPD_UP:
        g_lastKeyCode = key_code;
        DVMTraceDbg("keypad: KPD_UP %d\n", key_code);
        break;
      default:
        g_lastKeyCode = 0;
        DVMTraceDbg("keypad: Unknown key event %d\n", event_code);
        break;
      }
      AsyncIO_notify(g_keyEventNotifier);
    }
  }
}
#endif //ARCH_ARM_SPD