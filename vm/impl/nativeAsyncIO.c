#include <dthing.h>
#include <vm_common.h>
#include <AsyncIO.h>
#include <kni.h>
#include <nativeAsyncIO.h>

/**
 * Class:     java_lang_AsyncIO
 * Method:    getCurNotifierState
 * Signature: ()I
 */
void Java_java_lang_AsyncIO_getCurNotifierState(const u4* args, JValue* pResult) {
    ASYNC_Notifier * notifier;
    Thread * curthd = dthread_currentThread();

    DVM_ASSERT(curthd != NULL);
    notifier = curthd->asynNotifier;

    RETURN_INT(notifier->asynIoState);
}

/**
 * Class:     java_lang_AsyncIO
 * Method:    setCurNotifierState
 * Signature: (I)I
 */
void Java_java_lang_AsyncIO_setCurNotifierState(const u4* args, JValue* pResult) {
    int state = (int) args[1];
    Thread * curthd = dthread_currentThread();

    DVM_ASSERT(curthd != NULL);

    switch (state) {
    case ASYNC_IO_IDLE: {
            ASYNC_SetAIOState(curthd, ASYNC_IO_IDLE);
            curthd->asynNotifier->timeout = 0;
            curthd->asynNotifier->notified = FALSE;
        }
        break;

    case ASYNC_IO_ASLEEP:
    case ASYNC_IO_FIRST:
    case ASYNC_IO_DONE:
    case ASYNC_IO_TIMEOUT: {
            ASYNC_SetAIOState(curthd, state);
            curthd->asynNotifier->timeout = 0;
            curthd->asynNotifier->notified = FALSE;
        }
        break;

    case ASYNC_IO_BUSY:
        // not support, fall through
    default:
        break;
    }
    RETURN_VOID();
}

/**
 * Class:     java_lang_AsyncIO
 * Method:    waitSignalOrTimeOut
 * Signature: ()V
 */
void Java_java_lang_AsyncIO_waitSignalOrTimeOut(const u4* args, JValue* pResult) {
    Thread * curthd = dthread_currentThread();

    DVM_ASSERT(curthd != NULL);
    DVM_ASSERT(curthd->asynNotifier->asynIoState == ASYNC_IO_ASLEEP);
    curthd->sleepTime = curthd->asynNotifier->timeout;
    dthread_suspend(curthd, THREAD_ASYNCIO_SUSPENDED);
    RETURN_VOID();
}
