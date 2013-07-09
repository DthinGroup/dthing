#include "vm_common.h"
#include "dthread.h"
#include "schd.h"


Thread *currentThread;
Thread *readyThreadListHead;
Thread *otherThreadListHead;


/*Local Functions*/
static int genThreadId(void);
void mainThreadInit(void);
#ifdef ARCH_X86
void printTrace(Thread * thd);
#endif

/*gen simply now!*/
static int genThreadId(void)  //just using tmply
{
    static int start_main_id = 3225;
    
    return start_main_id++;
}

void mainThreadInit(void)
{
    Thread * main = allocThread(kDefaultStackSize);
    
    if(main == NULL)
    {
        DVM_ASSERT(1);
    }
    
    /*modify all registers,method,stack,etc...*/
    //
#ifdef ARCH_X86
	main->cb = printTrace;
#endif
    
    main->threadId = genThreadId();  /*magic number*/
    main->threadState = THREAD_READY;

	printf("Gen new thread %d \n",main->threadId);
    
    Schd_PushToReadyListHead(main);
}

#ifdef ARCH_X86
void printTrace(Thread * thd)
{
	printf("This is Thread:%d \n",thd->threadId);

	while(!CAN_SCHEDULE())
	{
		//Sleep(2);
	}

	switch(DVM_GETCURTICK()%10)
	{
	case 1:
		thd->threadState = THREAD_DEAD;
		break;
	case 2:
		mainThreadInit();
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

Thread * allocThread(int stackSize)
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
    
    stackBottom = DVM_MALLOC(stackSize);
    if(stackBottom == NULL)
    {
        DVM_FREE(tmp);
        return NULL;
    }
    
    //why set 0xc5? see in dalvik allocThread() func
    DVM_MEMSET(stackBottom, 0xc5, stackSize);     // stop valgrind complaints
    DVM_ASSERT(((u4)stackBottom & 0x03) == 0);    // 4-bytes aligned ?
    tmp->interpStackSize  = stackSize;
    tmp->interpStackStart = stackBottom + stackSize;
    tmp->interpStackEnd   = stackBottom + STACK_OVERFLOW_RESERVE;
    
    return tmp;
}

/*call Schd_InitThreadLists() before calling this!*/
void dthread_init(void)
{
    /*need to find out main thread of java as the first thread*/
    
    mainThreadInit();
	mainThreadInit();
	mainThreadInit();
	mainThreadInit();
	mainThreadInit();
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

void dthread_suspend(Thread * thread)
{

}

void dthread_resume(Thread * thread)
{

}

void dthread_delete(Thread * thread)
{
    
}




