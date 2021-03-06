#include <std_global.h>
#include <eventsystem.h>
#include <opl_es.h>
#include "vm_common.h"
#include <opl_net.h>
#include <ams_sms.h>
#include <ams.h>
#include <MQTTLocal.h>

#if defined(ARCH_ARM_SPD)
#include <priority_app.h>
#endif


static void launchESSchdule()
{
    ES_Semaphore *esSem;
    ES_initial();
    esSem = semaphore_create(0);

    do
    {
        int32_t res;
        
        res = ES_Schedule(200);
        
        if (res > 0)
        {
            semaphore_waitOrTimeout(esSem, res);
        }

    } while(TRUE);

    semaphore_destroy(esSem);
    esSem = NULL;
    ES_final();
}

#if defined(ARCH_X86)

extern int MQTTTest_main(int argc, char** argv);

char gArgc = 4;
char * gArgv[] = {"stdoutsub", "fuck", "--host", "182.61.25.208"};

#ifdef    NOT_LAUNCH_NET_TASK
extern bool_t vm_runApp_fake();
#endif

int main(int argc, char *argv[])
{
	//MQTTTest_main(gArgc, gArgv);
	//while(1);
	//test_mqtt_main();
	file_startup();
	Opl_net_activate();
	Ams_init();
    DVMTraceErr("===AT module OK===");
	//DVM_main(argc, argv);
#ifdef    NOT_LAUNCH_NET_TASK	
    vm_runApp(0);
	do{
		vmtime_sleep(1000);
	}
    while(true);
#else	
    //launchRemoteAMSClient(FALSE, argc, argv);
   launchESSchdule();
#endif
    return 1;
}

#elif defined(ARCH_ARM_SPD)

#define DvmLog SCI_TRACE_LOW

#define DTHING_THREAD_STACK_SIZE         (8*1024)
#define DTHING_THREAD_QUEUE_SIZE         (20*sizeof(uint32)*SCI_QUEUE_ITEM_SIZE)

uint32_t dvm_threadId = SCI_INVALID_BLOCK_ID;
uint8*   g_dvm_mem_space_ptr = NULL;

static void Dthing_IThreadProc(int argc, void * argv)
{
    int status =0;
    int    arg = 2;
    char * arga[2] = {"-cp","D:\\helloword.dex"};

    file_startup();
	GpioCrtlInit();
	//GpioSwitchMode(LINK_TO_ALL);	
	smsc_registerEvent();
	Ams_regModuleCallBackHandler(ATYPE_SAMS,smsc_callBack);
    Opl_net_activate();
    //Ams_init();
    DVMTraceErr("====AT module OK====");
    //DVM_main(arg,arga);
//    launchRemoteAMSClient(FALSE, arg, arga);
    launchESSchdule();
    
    if(SCI_INVALID_BLOCK_ID != dvm_threadId)
    {
        status = SCI_TerminateThread(dvm_threadId);
        status = SCI_DeleteThread(dvm_threadId);
        dvm_threadId = SCI_INVALID_BLOCK_ID;
    }

    if(NULL != g_dvm_mem_space_ptr)
    {
        SCI_FREE(g_dvm_mem_space_ptr);
        g_dvm_mem_space_ptr = PNULL;
    }
}

bool_t Dthing_startVM(void)
{
	SCI_TRACE_LOW("===========================================");
	SCI_TRACE_LOW("===========================================");
	SCI_TRACE_LOW("===========================================");
	SCI_TRACE_LOW("===========================================");
	SCI_TRACE_LOW("==================Dthing start======================");
    DvmLog("[Dthing log]:g_dvm_mem_space_ptr try to alloc");

	SCI_Sleep(1000);
    //Opl_net_activate();
	SCI_TRACE_LOW("=========================================2==");
	SCI_TRACE_LOW("=========================================2==");
	SCI_Sleep(1000);
#ifdef    NOT_LAUNCH_NET_TASK
    return ramsClient_runApplet(0);
#endif    
    DvmLog("[Dthing log]:g_dvm_mem_space_ptr try to alloc");
    //try to alloc java heap
    g_dvm_mem_space_ptr = (uint8*)SCI_ALLOCA(/*1024*2048*/ DTHING_THREAD_STACK_SIZE +DTHING_THREAD_QUEUE_SIZE);

    DvmLog("[Dthing log]:g_dvm_mem_space_ptr=0x%x",g_dvm_mem_space_ptr);
    
    if(PNULL == g_dvm_mem_space_ptr)
    {
        DvmLog("[Dthing log]:g_dvm_mem_space_ptr MMI_BL_MALLOC failed");
        return FALSE;
    }

    dvm_threadId = SCI_CreateAppThreadEx
                (
                    "DthingTask",
                    Dthing_IThreadProc,
                    0,
                    NULL,
                    g_dvm_mem_space_ptr,
                    DTHING_THREAD_STACK_SIZE,
                    PRI_JBED_TASK,
                    SCI_PREEMPT,
                    "DthingQueue",
                    (g_dvm_mem_space_ptr+DTHING_THREAD_STACK_SIZE),
                    DTHING_THREAD_QUEUE_SIZE,
                    SCI_AUTO_START
                );
                
    DvmLog("[Dthing log]:dvm_threadId = %d ",dvm_threadId);                

    return TRUE;
}
#endif
