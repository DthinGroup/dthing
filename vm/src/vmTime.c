#include <vm_common.h>
#include <schd.h>
#include <vmTime.h>

#ifdef ARCH_ARM_SPD
#include <os_api.h>
#endif

//just fot test in this file!!
//#define ARCH_X86

#ifdef ARCH_X86
	#pragma comment(lib, "Winmm.lib")
	#include <Windows.h>
	#include <Mmsystem.h>
#endif

#ifdef DVM_LOG
#undef DVM_LOG
#endif

#define DVM_LOG DVMTraceInf

#ifdef ARCH_X86
static HANDLE g_hMutex;
#elif defined(ARCH_ARM_SPD)
static SCI_MUTEX_PTR s_vm_timer_mutex = NULL;
#endif

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
    	DVM_LOG("uTimerID:%d \n",uTimerID);
    	//DVM_ASSERT(timer_id == uTimerID);
    	vmtime_stopTimer();
    	SET_SCHEDULE();
    	
    	ReleaseMutex( g_hMutex );
    }
#elif defined(ARCH_ARM_SPD)
    SCI_TIMER_PTR s_vm_schduler_timer = NULL;
    void lpTimerCb(uint32_t param)
    {
        //SCI_GetMutex(s_vm_timer_mutex,SCI_WAIT_FOREVER);
        vmtime_stopTimer();
    	SET_SCHEDULE();
    	//SCI_PutMutex(s_vm_timer_mutex);
    }
#endif

void vmtime_init(void)
{
#ifdef ARCH_X86
	g_hMutex = CreateMutex( NULL , FALSE , NULL );
#elif defined(ARCH_ARM_SPD)
    s_vm_timer_mutex = SCI_CreateMutex("VM_TIMER_MUTEX", SCI_NO_INHERIT);
    DVM_ASSERT(s_vm_timer_mutex !=NULL);
    s_vm_schduler_timer = SCI_CreateTimer("dthing_schduler_timer",lpTimerCb,0,SCHEDULER_TIMER,SCI_NO_ACTIVATE);
    if(s_vm_schduler_timer ==NULL)
    {
        DVM_LOG("vm schduler timer create fail!");
        DVM_ASSERT(0);
    }    
#endif
}


void vmtime_term(void)
{
#ifdef ARCH_X86
	ReleaseMutex(g_hMutex);
#elif defined(ARCH_ARM_SPD)
    if(s_vm_schduler_timer !=NULL)
    {
        if(SCI_IsTimerActive(s_vm_schduler_timer))
		SCI_DeactiveTimer(s_vm_schduler_timer);
        SCI_DeleteTimer(s_vm_schduler_timer);
    }
    SCI_DeleteMutex(s_vm_timer_mutex);
#endif
}

u8 vmtime_getTickCount(void)
{
#ifdef ARCH_X86
	return timeGetTime();
#elif defined(ARCH_ARM_SPD)
	return SCI_GetTickCount();
#endif
}

void vmtime_startTimer(void)
{
    static int timers = 0;
#ifdef ARCH_X86	
	WaitForSingleObject( g_hMutex , INFINITE );

	if(timer_id != 0)
	{
		DVM_LOG("------------------exception to stop_Timer \n");
		vmtime_stopTimer();
	}

	timer_id = timeSetEvent(
					SCHEDULER_TIMER,     
					1,
					(LPTIMECALLBACK )lpTimerCb,  
					0,
					TIME_ONESHOT
					);
	DVM_LOG("timer_id:%d \n",timer_id);
	DVM_ASSERT(timer_id != 0);
	CLR_SCHEDULE();
	timers++;
	DVM_LOG("------------------start_Timer times:%d \n",timers);
	ReleaseMutex( g_hMutex );
#elif defined(ARCH_ARM_SPD)
    int ret;
    //SCI_GetMutex(s_vm_timer_mutex,SCI_WAIT_FOREVER);
    //if(SCI_IsTimerActive(s_vm_schduler_timer))
	//	SCI_DeactiveTimer(s_vm_schduler_timer);
	CLR_SCHEDULE();
    SCI_ChangeTimer(s_vm_schduler_timer, lpTimerCb, SCHEDULER_TIMER);
    ret = SCI_ActiveTimer(s_vm_schduler_timer);
    DVM_ASSERT(ret ==SCI_SUCCESS);
    timers++;
    DVM_LOG("------------------start_Timer times:%d \n",timers);
    //SCI_PutMutex(s_vm_timer_mutex);
#endif	
}

void vmtime_stopTimer(void)
{
    static int timers = 0;
#ifdef ARCH_X86
	MMRESULT ret =0;	
	timers++;
	DVM_LOG("------------------stop_Timer times:%d \n",timers);
	if(timer_id != 0)
	{
		DVM_LOG("stop_Timer times:%d \n",timers);
		DVM_LOG("stop timer:%d \n",timer_id);
		ret = timeKillEvent(timer_id);
		DVM_LOG("stop ret:%d \n",ret);
		DVM_ASSERT(ret ==0);
		timer_id = 0;
	}
#elif defined(ARCH_ARM_SPD)
    int ret;
    //SCI_GetMutex(s_vm_timer_mutex,SCI_WAIT_FOREVER);
   // ret = SCI_DeactiveTimer(s_vm_schduler_timer);
  //  DVM_ASSERT(ret ==SCI_SUCCESS);
    timers++;
	DVM_LOG("------------------stop_Timer times:%d \n",timers);
	//SCI_PutMutex(s_vm_timer_mutex);
#endif	
}

void vmtime_sleep(int  mSecond)
{
	if(mSecond <=0)
		return ;
#ifdef ARCH_X86
    Sleep(mSecond);
#elif defined(ARCH_ARM_SPD)
    SCI_Sleep(mSecond);
#else
    #error "don't forget me!"
#endif
}
