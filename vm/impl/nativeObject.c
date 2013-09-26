#include "vm_common.h"
#include "nativeObject.h"
#include "dthread.h"
#include "schd.h"
#include "kni.h"

void Java_java_lang_Object_wait(const u4* args, JValue* pResult)
{
	Thread * thd = NULL;
	Object * thisObj = (Object*)args[0];
	long msec = (long) args[1];
	int  nsec = (int)  args[2];

	thd =dthread_currentThread();
	DVM_ASSERT(thd != NULL);

	Sync_dvmObjectWait(thd, thisObj,msec,nsec,false);
	RETURN_VOID();
}

void Java_java_lang_Object_notify(const u4* args, JValue* pResult)
{
	Thread * thd = NULL;
	Object * thisObj = (Object*)args[0];
	long msec = (long) args[1];
	int  nsec = (int)  args[2];

	thd =dthread_currentThread();
	DVM_ASSERT(thd != NULL);

	Sync_dvmObjectNotify(thd, thisObj);
	RETURN_VOID();
}

void Java_java_lang_Object_notifyAll(const u4* args, JValue* pResult)
{
	Thread * thd = NULL;
	Object * thisObj = (Object*)args[0];
	long msec = (long) args[1];
	int  nsec = (int)  args[2];

	thd =dthread_currentThread();
	DVM_ASSERT(thd != NULL);

	Sync_dvmObjectNotifyAll(thd, thisObj);
	RETURN_VOID();
}
