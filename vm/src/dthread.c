#include "vm_common.h"
#include "dthread.h"
#include "schd.h"
#include "interpStack.h"
#include "vmTime.h"
#include "Object.h"

#ifdef DVM_LOG
#undef DVM_LOG
#endif

#define DVM_LOG		printf

Thread *currentThread;
Thread *readyThreadListHead;
Thread *otherThreadListHead;


/*Local Functions*/
static int genThreadId(void);
//void mainThreadInit(void);
#ifdef ARCH_X86
void printTrace(Thread * thd);
#endif

/*gen simply now!*/
static int genThreadId(void)  //just using tmply
{
    static int start_main_id = 3225;
	
	DVM_LOG("Gen new thread %d \n",start_main_id);
    
    return start_main_id++;
}

#ifdef ARCH_X86
void printTrace(Thread * thd)
{
	DVM_LOG("This is Thread:%d \n",thd->threadId);

	return;

	while(!CAN_SCHEDULE())
	{
		//Sleep(2);
	}

	switch(vmtime_getTickCount()%10)
	{
	case 1:
		thd->threadState = THREAD_DEAD;
		break;
	case 2:
		//mainThreadInit();
		break;
	case 3:
		thd->threadState = THREAD_TIME_SUSPENDED;
		thd->sleepTime = 500;
		break;
	default:break;
	}
}
#endif

/*Public Functions*/

Thread * dthread_alloc(int stackSize)
{
    u1* stackBottom;
    Thread * tmp = (Thread *) DVM_MALLOC(sizeof(Thread));
    
    if(tmp == NULL)
    {
        return NULL;
    }
    DVM_MEMSET(tmp,0x00,sizeof(Thread));
    
    /*verify stack size*/
    DVM_ASSERT(stackSize >= kMinStackSize && stackSize <=kMaxStackSize);
    
    tmp->threadState = THREAD_INITED;
    
    stackBottom = (u1*)DVM_MALLOC(stackSize);
    if(stackBottom == NULL)
    {
        DVM_FREE(tmp);
        return NULL;
    }
    
    //why set 0xc5? see in dalvik dthread_alloc() func
    DVM_MEMSET(stackBottom, 0xc5, stackSize);     // stop valgrind complaints
    DVM_ASSERT(((u4)stackBottom & 0x03) == 0);    // 4-bytes aligned ?
    tmp->interpStackSize  = stackSize;
    tmp->interpStackStart = stackBottom + stackSize;
    tmp->interpStackEnd   = stackBottom + STACK_OVERFLOW_RESERVE;	
    
    return tmp;
}

void dthread_create(const Method * mth,Object* obj)
{
	const char* desc = &(mth->shorty[1]); // [0] is the return type.
	ClassObject* clazz = NULL;
    Thread * jthread   = NULL;
    int verifyCount    = 0;
    u4* ins            = NULL;
	
	DVM_LOG("call dthread_create \n" );

	if(obj != NULL)
		clazz = obj->clazz;
	else
		clazz = mth->clazz;

	if(clazz ==NULL)
	{
		DVM_ASSERT(0);
	}

	jthread = dthread_alloc(kDefaultStackSize);
    if(jthread == NULL)
    {
        DVM_ASSERT(0);
    }
    
    /*modify all registers,method,stack,etc...*/
    //
#ifdef ARCH_X86
	jthread->cb = printTrace;
#endif
    
    jthread->threadId = genThreadId();  /*magic number*/
    jthread->threadState = THREAD_READY;

	jthread->bInterpFirst = TRUE;
	jthread->threadObj = (Object *)obj;
	jthread->entryMthd = (Method *)mth;
	jthread->creatTime = vmtime_getTickCount();
	
	dvmPushInterpFrame(jthread,mth);

	/* "ins" for new frame start at frame pointer plus locals */
    ins = ((u4*)jthread->interpSave.curFrame) +
           (mth->registersSize - mth->insSize);

	/* put "this" pointer into in0 if appropriate */
	if (!dvmIsStaticMethod(mth)) 
	{
#ifdef WITH_EXTRA_OBJECT_VALIDATION
        assert(obj != NULL && dvmIsHeapAddress(obj));
#endif
        *ins++ = (u4) obj;
        verifyCount++;
    }
    
	/*
	*/

    Schd_PushToReadyListHead(jthread);
}

void dthread_delete(Thread * thread)
{
    DVM_ASSERT(thread != NULL);
	DVM_FREE((void*)(thread->interpStackStart - thread->interpStackSize));
    /*other to free ?*/
	DVM_FREE(thread);
}

void dthread_term(void)
{
    
}

void dthread_start(Thread * thread)
{
    
}

void dthread_stop(Thread * thread)
{

}

void dthread_suspend(Thread * thread,THREAD_STATE_E  newState)
{
	THREAD_STATE_E oldState = thread->threadState;

	DVM_LOG("call dthread_suspend: new State(%d)\n",newState);
	if(thread == NULL)
	{
		return;
	}

	if(oldState == THREAD_INITED || oldState == THREAD_DEAD)
	{
		DVM_LOG("dthread_suspend:old State is invalid !\n");
		return;
	}

	if(oldState == newState)
	{
		DVM_LOG("dthread_suspend:State not changed!\n");
		return;
	}

	if(newState == THREAD_TIME_SUSPENDED)
	{
		SET_SCHEDULE();   //it cause scheduler occuring ! 
	}

	thread->threadState = THREAD_TIME_SUSPENDED;
	return;
}

void dthread_resume(Thread * thread)
{
	
}



Thread * dthread_currentThread(void)
{
	if(currentThread != NULL)
		return currentThread;
	else
		return NULL;
}
