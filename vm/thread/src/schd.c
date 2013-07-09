#include "vm_common.h"
#include "schd.h"
#include "dthread.h"

#define SCHD_OpenTIMER	start_Timer
#define SCHD_StopTIMER	stop_Timer

volatile u1 schedulerFlag = 0;

void Schd_InitThreadLists(void)
{
    Thread * guard = NULL;
        
    /*first node of readyList/otherList is guard,not used as normal thread*/
    guard = allocThread(kMinStackSize);
    DVM_ASSERT(guard != NULL);
    guard->threadId = READY_GUARD_ID;    
    readyThreadListHead = guard;
    
    guard = allocThread(kMinStackSize);
    DVM_ASSERT(guard != NULL);
    guard->threadId = OTHER_GUARD_ID;    
    otherThreadListHead = guard;
    
    guard = NULL;
    currentThread   = NULL;
	schedulerFlag = 0;
}

void Schd_FinalThreadLists(void)
{
	Thread * guard = readyThreadListHead->next;

	/*final ready list*/
	while(guard != NULL)
	{
		guard->pre->next = guard->next;
		if(guard->next != NULL)
		{
			guard->next->pre = guard->pre;
		}

		DVM_FREE((void*)(guard->interpStackStart - guard->interpStackSize));
        DVM_FREE(guard);
		guard = readyThreadListHead->next;
	}
	DVM_FREE((void*)(readyThreadListHead->interpStackStart - readyThreadListHead->interpStackSize));
    DVM_FREE(readyThreadListHead);
	readyThreadListHead = NULL;

	/*final other list*/
	guard = otherThreadListHead->next;
	while(guard != NULL)
	{
		guard->pre->next = guard->next;
		if(guard->next != NULL)
		{
			guard->next->pre = guard->pre;
		}

		DVM_FREE((void*)(guard->interpStackStart - guard->interpStackSize));
        DVM_FREE(guard);
		guard = otherThreadListHead->next;
	}
	DVM_FREE((void*)(otherThreadListHead->interpStackStart - otherThreadListHead->interpStackSize));
    DVM_FREE(otherThreadListHead);
	otherThreadListHead = NULL;

	/**/
	if(currentThread != NULL)
	{
		DVM_FREE((void*)(currentThread->interpStackStart - currentThread->interpStackSize));
		DVM_FREE(currentThread);
		currentThread = NULL;
	}
}

void Schd_ChangeThreadState(Thread * thread,THREAD_STATE_E newState)
{
    DVM_ASSERT(thread != NULL);
    DVM_ASSERT(thread != readyThreadListHead);
    DVM_ASSERT(thread != otherThreadListHead);
    
    if(newState >THREAD_INITED && newState <=THREAD_DEAD)
    {
        thread->threadState = newState;
    }
    return;
}

/*must put READY STATE thread to the list head*/
void Schd_PushToReadyListHead(Thread * thread)
{
    Thread * tmp = readyThreadListHead;
    
    DVM_ASSERT(thread != readyThreadListHead);
    
    if(thread == NULL)
    {
        return;
    }
    
    /*step 1:delete thread node from list*/
    do
    {
        if(tmp == thread)
        {
            if(tmp->pre != NULL)
                tmp->pre->next = tmp->next;
            if(tmp->next!= NULL)
                tmp->next->pre = tmp->pre;
            break;
        }
        tmp = tmp->next;
    }while(tmp!=NULL);
    
    /*step 2:add thread node at list head*/
    thread->next = readyThreadListHead->next;
    if(readyThreadListHead->next != NULL)
    {
        readyThreadListHead->next->pre = thread;
    }
    readyThreadListHead->next = thread;
    thread->pre = readyThreadListHead;    
}


/*must put READY STATE thread to the list tail*/
void Schd_PushToReadyListTail(Thread * thread)
{
    Thread * tmp = NULL;
    Thread * last = NULL;
    
    DVM_ASSERT(thread != readyThreadListHead);
    
    if(thread == NULL)
    {
        return;
    }
    
    tmp = readyThreadListHead;
    
    do
    {
        if(tmp == thread)
        {
            return;   /*find thread in the list,return,not need to move it to tail*/
        }        
        last = tmp;
        tmp=tmp->next;        
    }while(tmp!=NULL);
    
    last->next = thread;
    thread->next = NULL;
    thread->pre = last;
}

/*get next READY thread from ready list head*/
Thread * Schd_PopFromReadyList(void)
{
    Thread * tmp = readyThreadListHead->next;
    
    if(readyThreadListHead->next == NULL)
    {
        return NULL;
    }
        
    if(readyThreadListHead->next->next != NULL)
        readyThreadListHead->next->next->pre = readyThreadListHead;
    readyThreadListHead->next = readyThreadListHead->next->next;

    return tmp;
}

/*push non-READY thread to other list*/
void Schd_PushToOtherList(Thread * thread)
{
    Thread * tmp = otherThreadListHead;
    
    DVM_ASSERT(thread != otherThreadListHead);
    
    if(thread == NULL)
    {
        return;
    }
    
    /*step 1:delete thread node from list*/
    do
    {
        if(tmp == thread)
        {
            if(tmp->pre != NULL)
                tmp->pre->next = tmp->next;
            if(tmp->next!= NULL)
                tmp->next->pre = tmp->pre;
            break;
        }
        tmp = tmp->next;
    }while(tmp!=NULL);
    
    /*step 2:add thread node at list head*/
    thread->next = otherThreadListHead->next;
    if(otherThreadListHead->next != NULL)
    {
        otherThreadListHead->next->pre = thread;
    }
    otherThreadListHead->next = thread;
    thread->pre = otherThreadListHead; 
}

/*get READY thread,one per pop,get untill return null*/
Thread * Schd_PopReadyFromOtherList(void)
{
    Thread * tmp = otherThreadListHead->next;
    Thread * ret = NULL;
    
    if(tmp == NULL)
    {
        return NULL;
    }
    
    do
    {
        if(tmp->threadState == THREAD_READY)
        {
            ret = tmp;
            tmp->pre->next = tmp->next;
            if(tmp->next !=NULL)
                tmp->next->pre = tmp->pre;
            
            return ret;
        }
        tmp = tmp->next;
    }while(tmp != NULL);
    
    return NULL;
}

void Schd_DecSleepTime(u4 deltaTime)
{
    Thread * tmp = otherThreadListHead->next;
    while(tmp != NULL)
    {
        if(tmp->threadState == THREAD_TIME_SUSPENDED)
        {
			printf("Thread %d is in sleep\n",tmp->threadId);
            tmp->sleepTime = (tmp->sleepTime > deltaTime) ? (tmp->sleepTime - deltaTime) : (tmp->threadState = THREAD_READY,0) ;
       }
       tmp = tmp->next;
    }
    
    tmp = NULL;
    do
    {
        tmp = Schd_PopReadyFromOtherList();        
        if(tmp != NULL)
        {
			printf("Thread %d sleep over!\n",tmp->threadId);
            Schd_PushToReadyListHead(tmp);
        }
    }while(tmp!=NULL);
    
}

/*delete and free DEAD state thread*/
void Schd_DelDeadThread(void)
{
    Thread * tmp  = otherThreadListHead->next;
    Thread * find = NULL;
    
    while(tmp != NULL)
    {
        if(tmp->threadState == THREAD_DEAD)
        {
			printf("Destroy thread:%d \n",tmp->threadId);
            find = tmp->pre;
            tmp->pre->next = tmp->next;
            if(tmp->next != NULL)
            {
                tmp->next->pre = tmp->pre;
            }
            
            DVM_FREE((void*)(tmp->interpStackStart - tmp->interpStackSize));
            DVM_FREE(tmp);
            tmp = find;
        }
        tmp = tmp->next;
    }
}

/*calc ready thread account*/
int Schd_ReadyThreadAccount(void)
{
    int accu = 0;
    Thread * tmp  = readyThreadListHead->next;
    
    while(tmp!=NULL)
    {
        accu++;
        tmp = tmp->next;
    }
    
    return accu;
}

/*calc other thread account,if don't want to put *
 *dead thread into account,call Schd_DelDeadThread() in advance */
int Schd_OtherThreadAccount(void)
{
    int accu = 0;
    Thread * tmp  = otherThreadListHead->next;
    
    while(tmp!=NULL)
    {
        accu++;
        tmp = tmp->next;
    }
    
    return accu;
}

/* tatal thread account,but dead ones exclude*/
int Schd_ThreadAccountInTotal(void)
{
    int accu = (currentThread != NULL) ? 1 : 0;
    
    Schd_DelDeadThread();
    accu += Schd_ReadyThreadAccount() + Schd_OtherThreadAccount();
    return accu;
}

HANDLE g_hMutex;

void Schd_SCHEDULER(void)
{
	unsigned long lastMs = DVM_GETCURTICK();
#ifdef ARCH_X86
	g_hMutex = CreateMutex( NULL , FALSE , NULL );
#endif
	//start_Timer();
    while(1)
    {
    SCHD_RESTART:
        if(currentThread != NULL)
        {
            if(currentThread->threadState == THREAD_ACTIVE)
            {
                currentThread->threadState = THREAD_READY;
                Schd_PushToReadyListTail(currentThread);
                currentThread = NULL;
            }
            else
            {
                Schd_PushToOtherList(currentThread);
                currentThread = NULL;
            }
        }
        
        currentThread = Schd_PopFromReadyList();

		printf("\nThis loop Time run %d Ms\n",DVM_GETCURTICK() - lastMs);
		lastMs = DVM_GETCURTICK();
        if(currentThread != NULL)
        {
            currentThread->threadState = THREAD_ACTIVE;
						
            SCHD_OpenTIMER();
            //call interp!   
			(*currentThread->cb)(currentThread);
			//CLR_SCHEDULE();
        }
        else    /*no threads to run*/
        {
            Schd_DelDeadThread();
            
            if(Schd_ThreadAccountInTotal() ==0)
            {
                goto SCHD_END;
            }
			SCHD_OpenTIMER();
			while(!CAN_SCHEDULE())
			{
				printf("No ready thread,waiting...\n");
				Sleep(10);
			}
        }
        
        Schd_DecSleepTime(SCHEDULER_TIMER);
        
        goto SCHD_RESTART;
    
    SCHD_END:
		printf("Schd over!\n");
        break;
    }    

#ifdef ARCH_X86
	ReleaseMutex(g_hMutex);
#endif
}

/*API to interpret*/
void Schd_Interpret(Thread * self)
{
	//
}


#ifdef ARCH_X86

volatile unsigned int timer_id = 0;
/*定时器回调函数*/
WINAPI lpTimerCb(
  u4 uTimerID,
  u4 uMsg,
  unsigned long dwUser,
  unsigned long dw1,
  unsigned long dw2
)
{
	WaitForSingleObject( g_hMutex , INFINITE );
	printf("uTimerID:%d \n",uTimerID);
	DVM_ASSERT(timer_id == uTimerID);
	stop_Timer();
	SET_SCHEDULE();
	
	ReleaseMutex( g_hMutex );
}


void start_Timer()
{
	
	WaitForSingleObject( g_hMutex , INFINITE );

	timer_id = timeSetEvent(
					SCHEDULER_TIMER,     
					1,
					(LPTIMECALLBACK )lpTimerCb,  
					0,
					TIME_ONESHOT
					);
	printf("timer_id:%d \n",timer_id);
	DVM_ASSERT(timer_id != 0);
	CLR_SCHEDULE();

	ReleaseMutex( g_hMutex );
}

void stop_Timer()
{
	MMRESULT ret =0;
	if(timer_id != 0)
	{
		printf("stop timer:%d \n",timer_id);
		ret = timeKillEvent(timer_id);
		printf("stop ret:%d \n",ret);
		timer_id = 0;
	}
}

#endif