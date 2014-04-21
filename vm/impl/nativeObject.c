#include "vm_common.h"
#include "nativeObject.h"
#include "dthread.h"
#include "schd.h"
#include "kni.h"
#include <array.h>
#include <heap.h>
#include <sync.h>

void Java_java_lang_Object_internalClone(const u4* args, JValue* pResult)
{
    Object* thisObj = (Object*) args[0];
    Object* clone = NULL;

    ClassObject* clazz = thisObj->clazz;

    /* Class.java shouldn't let us get here (java.lang.Class is final
     * and does not implement Clonable), but make extra sure.
     * A memcpy() clone will wreak havoc on a ClassObject's "innards".
     */
    //assert(!dvmIsTheClassClass(clazz));
    size_t offset;
    size_t size;
    if (IS_CLASS_FLAG_SET(clazz, CLASS_ISARRAY)) {
        size = dvmArrayObjectSize((ArrayObject *)thisObj);
    } else {
        size = clazz->objectSize;
    }

    clone = (Object*)heapAllocObject(size, ALLOC_DONT_TRACK);
    if (clone != NULL)
    {
        DVM_OBJECT_INIT(clone, clazz);
        offset = sizeof(Object);
        /* Copy instance data.  We assume memcpy copies by words. */
        CRTL_memcpy((char*)clone + offset, (char*)thisObj + offset, size - offset);
    }
    RETURN_PTR(clone);
}

void Java_java_lang_Object_getClass(const u4* args, JValue* pResult)
{
    Object* thisPtr = (Object*) args[0];
    DVMTraceInf("Java_java_lang_Object_getClass \n");
    RETURN_PTR(thisPtr->clazz);
}

void Java_java_lang_Object_hashCode(const u4* args, JValue* pResult) {
    Object* thisPtr = (Object*) args[0];

    DVMTraceInf("Java_java_lang_Object_hashCode\n");
    RETURN_INT((u4)thisPtr);
}

void Java_java_lang_Object_wait(const u4* args, JValue* pResult) {
    Thread * thd = NULL;
    Object * thisObj = (Object*) args[0];
    long msec = (long) args[1];
    int nsec = (int) args[2];

    thd = dthread_currentThread();
    DVM_ASSERT(thd != NULL);

    Sync_dvmObjectWait(thd, thisObj, msec, nsec, false);
    RETURN_VOID();
}

void Java_java_lang_Object_notify(const u4* args, JValue* pResult) {
    Thread * thd = NULL;
    Object * thisObj = (Object*) args[0];
    long msec = (long) args[1];
    int nsec = (int) args[2];

    thd = dthread_currentThread();
    DVM_ASSERT(thd != NULL);

    Sync_dvmObjectNotify(thd, thisObj);
    RETURN_VOID();
}

void Java_java_lang_Object_notifyAll(const u4* args, JValue* pResult) {
    Thread * thd = NULL;
    Object * thisObj = (Object*) args[0];
    long msec = (long) args[1];
    int nsec = (int) args[2];

    thd = dthread_currentThread();
    DVM_ASSERT(thd != NULL);

    Sync_dvmObjectNotifyAll(thd, thisObj);
    RETURN_VOID();
}
