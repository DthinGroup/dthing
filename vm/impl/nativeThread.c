#include <dthing.h>
#include <nativeThread.h>
#include "dthread.h"
#include "schd.h"
#include "kni.h"

#include "AsyncIO.h"

/* test code
ASYNC_Notifier * curNotifier = NULL;
DWORD WINAPI AsyncNotify(PVOID pParam) {
    Sleep(1000);
    AsyncIO_notify(curNotifier);
}
*/

void Java_java_lang_Thread_activeCount(const u4* args, JValue* pResult) {
#if 1
    int count = 0;
    count = Schd_ThreadAccountInTotal();
    DVMTraceInf("Java_java_lang_Thread_activeCount0:%d ..\n", count);
    RETURN_INT(count);
#else
    /*test code for AsyncIO*/
    int count = 0;
    if (AsyncIO_firstCall()) {
        //DWORD threadid;
        AsyncIO_callAgainWhenSignalledOrTimeOut(10 * 1000);
        //curNotifier = Async_getCurNotifier();
        //CreateThread(NULL,0,AsyncNotify,NULL,0,&threadid);
    } else {
        count = Schd_ThreadAccountInTotal();
    }
    RETURN_INT(count);
#endif
}

void Java_java_lang_Thread_currentThread(const u4* args, JValue* pResult) {
    Object * cur = NULL;
    Thread * curthd = NULL;
    curthd = dthread_currentThread();
    DVM_ASSERT(curthd != NULL);
    cur = curthd->threadObj;
    //pResult->l = cur;
    RETURN_PTR(cur);
}

void Java_java_lang_Thread_sleep(const u4* args, JValue* pResult) {
    u8 sleeptime = (u8) args[1];
    Thread * curThd = dthread_currentThread();

    DVMTraceInf("sleep0:%d ms\n", sleeptime);
    if (sleeptime <= 0) {
        return;
    }

    DVM_ASSERT(curThd != NULL);
    curThd->sleepTime = sleeptime;
    dthread_suspend(curThd, THREAD_TIME_SUSPENDED);
    RETURN_VOID();
}

void Java_java_lang_Thread_start(const u4* args, JValue* pResult) {
    //self->threadObj->clazz->vtable[gDvm.voffJavaLangThread_run]
    Method * runMeth = NULL;
    Object * thisObj = (Object*) args[0];

    DVM_ASSERT(thisObj != NULL);
    /* super hard code 21, need to use gDvm.voffJavaLangThread_run to replace. */
    runMeth = thisObj->clazz->vtable[14];  //21

    DVMTraceInf("Java_java_lang_Thread_start..\n");

    dthread_create(runMeth, (Object*) thisObj);

    RETURN_VOID();
}

void Java_java_lang_Thread_isAlive(const u4* args, JValue* pResult) {
    Thread * thd = NULL;
    Object * thisObj = (Object*) args[0];
    jboolean ret = false;

    DVM_ASSERT(thisObj != NULL);
    thd = Schd_FindThreadByJavaObj(thisObj);
    ret = (thd != NULL && (thd->threadState < THREAD_DEAD));
    RETURN_BOOLEAN(ret);
}

void Java_java_lang_Thread_setPriority(const u4* args, JValue* pResult) {
    // TODO : implement
    DVMTraceInf("Java_java_lang_Thread_setPriority0 is not implemented..\n");
}

void Java_java_lang_Thread_interrupt(const u4* args, JValue* pResult) {
    // TODO : implement
    DVMTraceInf("Java_java_lang_Thread_interrupt is not implemented..\n");
}

void Java_java_lang_Thread_interrupted(const u4* args, JValue* pResult) {
    Object * thisObj = (Object*) args[0];
    int param1 = (int) args[1];
    int param2 = (int) args[2];

    int ret = 0;

    printf("Java_java_lang_Thread_print..\n");

    ret = param1 + param2;

    //pResult->i  = ret;
    RETURN_INT(ret);
}

void Java_java_lang_Thread_isInterrupted(const u4* args, JValue* pResult) {
    // TODO : implement
    DVMTraceInf("Java_java_lang_Thread_isInterrupted is not implemented..\n");
}

void Java_java_lang_Thread_yield(const u4* args, JValue* pResult) {
    // TODO : implement
    DVMTraceInf("Java_java_lang_Thread_yield is not implemented..\n");
}

void Java_java_lang_Thread_holdsLock(const u4* args, JValue* pResult) {
    // TODO : implement
    DVMTraceInf("Java_java_lang_Thread_holdsLock is not implemented..\n");
}
