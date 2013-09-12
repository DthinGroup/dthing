#include <vm_common.h>
#include <schd.h>
#include <vmTime.h>

#ifdef DVM_LOG
#undef DVM_LOG
#endif

#define DVM_LOG printf

HANDLE g_hMutex;

void vmtime_init(void)
{
#ifdef ARCH_X86
	g_hMutex = CreateMutex( NULL , FALSE , NULL );
#endif
}


void vmtime_term(void)
{
#ifdef ARCH_X86
	ReleaseMutex(g_hMutex);
#endif
}

u8 vmtime_getTickCount(void)
{
#ifdef ARCH_X86
	return GetTickCount();
#else
	return 0;
#endif
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
	DVM_LOG("uTimerID:%d \n",uTimerID);
	DVM_ASSERT(timer_id == uTimerID);
	vmtime_stopTimer();
	SET_SCHEDULE();
	
	ReleaseMutex( g_hMutex );
}


void vmtime_startTimer(void)
{
	static int timers = 0;
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
}

void vmtime_stopTimer(void)
{
	MMRESULT ret =0;
	static int timers = 0;
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
}
#endif
