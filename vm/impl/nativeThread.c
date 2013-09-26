#include "vm_common.h"
#include "nativeThread.h"
#include "dthread.h"
#include "schd.h"
#include "kni.h"

void Java_java_lang_Thread_activeCount0(const u4* args, JValue* pResult)
{
	int count = 0;
	count = Schd_ThreadAccountInTotal();
	printf("Java_java_lang_Thread_activeCount0:%d ..\n",count);
	RETURN_INT(count);
}

void Java_java_lang_Thread_currentThread0(const u4* args, JValue* pResult)
{
	Object * cur = NULL;
	Thread * curthd = NULL;
	curthd = dthread_currentThread();
	DVM_ASSERT(cur!=NULL);
	cur = curthd->threadObj;
	//pResult->l = cur;
	RETURN_PTR(cur);
}

void Java_java_lang_Thread_sleep0(const u4* args, JValue* pResult)
{
    u8 sleeptime = (u8) args[1];
	Thread * curThd =  dthread_currentThread();

	printf("sleep0:%d ms\n",sleeptime);
	if(sleeptime <= 0)
		return;

	DVM_ASSERT(curThd != NULL);
	curThd->sleepTime = sleeptime;
	dthread_suspend(curThd,THREAD_TIME_SUSPENDED);
	RETURN_VOID();
}


void Java_java_lang_Thread_start0(const u4* args, JValue* pResult)
{
	//self->threadObj->clazz->vtable[gDvm.voffJavaLangThread_run]
	Method * runMeth = NULL;//
	Object * thisObj = (Object*)args[0];
	
	DVM_ASSERT(thisObj != NULL);
	runMeth = thisObj->clazz->vtable[14];   //gDvm.voffJavaLangThread_run

	printf("Java_java_lang_Thread_start..\n");

	dthread_create(runMeth,(Object* )thisObj);

	RETURN_VOID();
}

void Java_java_lang_Thread_isAlive0(const u4* args, JValue* pResult)
{
	Thread * thd = NULL;
    Object * thisObj = (Object*)args[0];  
	int  ret = 0;
	
	DVM_ASSERT(thisObj != NULL);
	thd = Schd_FindThreadByJavaObj(thisObj);

	if(thd != NULL && (thd->threadState < THREAD_DEAD))
	{
		ret = 1;
	}
	else
	{
		ret = 0;
	}

	//pResult->b = ret;
	RETURN_BOOLEAN(ret);
}

void Java_java_lang_Thread_setPriority0(const u4* args, JValue* pResult)
{

}

void Java_java_lang_Thread_printQ(const u4* args, JValue* pResult)
{
	Object * thisObj = (Object*)args[0];
	int param1 = (int) args[1];
	int param2 = (int) args[2];

	int ret =0;

	printf("Java_java_lang_Thread_print..\n");

	ret = param1 + param2;

	//pResult->i  = ret;
	RETURN_INT(ret);
}

