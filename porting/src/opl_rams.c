#include "vm_common.h"
#include <std_global.h>
#include <opl_rams.h>
#include <vmTime.h>
#ifdef ARCH_X86
#include <windows.h>
#elif defined(ARCH_ARM_SPD)
#include <mn_api.h>
#include <mn_error.h>
#include <mn_type.h>
#include <Mn_events.h>
#include <socket_api.h>
#include <priority_app.h>
#endif

#ifdef ARCH_X86
static bool_t  wsaStarted = FALSE;
static int32_t lockObjInst;
#define SET_NON_BLOCKING(sock) \
    do { \
        unsigned long val = 1; \
        ioctlsocket(sock, FIONBIO, &val); \
    } while(0)
#endif

#if defined(ARCH_ARM_SPD)	
/**/
static MN_DUAL_SYS_E s_mn_dual_card = MN_DUAL_SYS_1;
static uint32      netId = 0;
static uint32 retrytimes = 0;
static bool_t isActived = FALSE ;
static bool_t isSuccess = FALSE ;

void rmtc_pdpactive();

static void rmtc_pdpCallbck(uint32 id, uint32 argc, void *argv)
{
    APP_MN_GPRS_EXT_T *signal_ptr = (APP_MN_GPRS_EXT_T *)argv;
    uint32 pdp_id_arr[MN_GPRS_MAX_PDP_CONTEXT_COUNT] = {0,0,0,0,0,0,0,0,0,0,0};
    pdp_id_arr[2]=1;

    DVMTraceErr("===RMT==rmtc_pdpCallbck - EV_MN_APP_SET_PDP_CONTEXT_CNF_F=%d\n", EV_MN_APP_SET_PDP_CONTEXT_CNF_F);
    DVMTraceErr("===RMT==rmtc_pdpCallbck - INFO: id (%d)  argc(%d)\n", id, argc);
    DVMTraceErr("===RMT==rmtc_pdpCallbck - INFO: signal_ptr->result (%d)\n", signal_ptr->result);
    DVMTraceErr("===RMT==rmtc_pdpCallbck - INFO: signal_ptr->dual_sys (%d)\n", signal_ptr->dual_sys);
    DVMTraceErr("===RMT==rmtc_pdpCallbck - INFO: signal_ptr->pdp_id (%d)\n", signal_ptr->pdp_id);
    DVMTraceErr("===RMT==rmtc_pdpCallbck - INFO: signal_ptr->nsapi (%d)\n", signal_ptr->nsapi);

	switch(argc)
	{
		case EV_MN_APP_ACTIVATE_PDP_CONTEXT_CNF_F:			 
			if(MN_GPRS_ERR_SUCCESS == signal_ptr->result || 
			   MN_GPRS_ERR_SAME_PDP_CONTEXT ==signal_ptr->result ||
			   MN_GPRS_ERR_PDP_CONTEXT_ACTIVATED == signal_ptr->result)
	        {
	            netId = (uint32)signal_ptr->nsapi;	            
	            isSuccess = TRUE;
	            isActived = TRUE;	   
	            DVMTraceErr("===RMT== rmtc_pdpCallbck()-------gprs ok--------isActived=%d netId=%d\n",isActived,netId);         
	        }
	        else if(MN_GPRS_ERR_RETRYING == signal_ptr->result)
	        {
	            DVMTraceErr("===RMT== rmtc_pdpCallbck MN_GPRS_ERR_RETRYING == signal_ptr->result retrytimes=%d\n",retrytimes);
	            vmtime_sleep(5000);
	            if(retrytimes < 5)
	            {
	                //if(ERR_MNGPRS_NO_ERR != MNGPRS_SetAndActivePdpContextEx(s_mn_dual_card,(uint8*)"CMNET",BROWSER_E))
	                if(ERR_MNGPRS_NO_ERR != MNGPRS_ActivatePdpContextEx(s_mn_dual_card, TRUE, pdp_id_arr, BROWSER_E, MN_GSM_PREFER))
	                {
	                    DVMTraceErr("===RMT== rmtc_pdpCallbck 2  MNGPRS_SetAndActivePdpContextEx error\n");
	                }
	                else
	                {
	                    DVMTraceErr("===RMT== rmtc_pdpCallbck 2 MNGPRS_SetAndActivePdpContextEx waiting for gprs rsp\n");
	                    isActived = TRUE ;
	                    isSuccess = FALSE;
	                    return;
	                }
	            }
	            else
	            {
	            	DVMTraceErr("===RMT:> 5 times,fail!\n");
	            	isActived = TRUE ;
	               	isSuccess = FALSE;
	             	return;
	            }
	            retrytimes++;
	        }
	        else
	        {
	        	DVMTraceErr("===RMT== rmtc_pdpCallbck()-------gprs err--------\n");
	        	DVMTraceErr("===RMT==rmtc_pdpCallbck - INFO: signal_ptr->result (%d)\n", signal_ptr->result);	
	        	rmtc_pdpactive();            
	        }
			break;
			
		case EV_MN_APP_DEACTIVATE_PDP_CONTEXT_CNF_F:
			DVMTraceErr("===RMT== rmtc_pdpCallbck()-------EV_MN_APP_DEACTIVATE_PDP_CONTEXT_CNF_F in--------signal_ptr->result=&d\n",signal_ptr->result);
	        if(MN_GPRS_ERR_SUCCESS == signal_ptr->result)
	        {
	            SCI_UnregisterMsg(MN_APP_GPRS_SERVICE,
	                        (uint8)EV_MN_APP_ACTIVATE_PDP_CONTEXT_CNF_F,
	                        (uint8)(MAX_MN_APP_GPRS_EVENTS_NUM-1),
	                        rmtc_pdpCallbck);
	        }
			break;
			
		default:
			DVMTraceErr("===RMT==rmtc_pdpCallbckun-handle(%d)\n", argc);
			break;
	}
}

void rmtc_pdpactive()
{
#define MAX_PAP_USER_LEN 64
#define MAX_PAP_PASSWD_LEN 64	
    ERR_MNGPRS_CODE_E err = ERR_MNGPRS_NO_ERR;
    uint32 pdp_id_arr[MN_GPRS_MAX_PDP_CONTEXT_COUNT] = {0,0,0,0,0,0,0,0,0,0,0};
    bool_t param_used[7]={TRUE,TRUE,TRUE,FALSE,FALSE,FALSE,FALSE};
    uint8  user[MAX_PAP_USER_LEN + 1]={0};
    uint8  passwd[MAX_PAP_PASSWD_LEN + 1] = {0};
    MN_GPRS_PDP_ADDR_T addr={0};
    pdp_id_arr[2] = 1;

    DVMTraceErr("===RMT==  MNPHONE_StartupPsEx -  start\n");
    err = MNPHONE_StartupPsEx(s_mn_dual_card,MN_GMMREG_RAT_GPRS);
    DVMTraceErr("===RMT==  MNPHONE_StartupPsEx -  end  ERR=%d\n",err);
    vmtime_sleep(40*1000);

    DVMTraceErr("===RMT==  MNPHONE_GprsAttachEx -  start\n");
    err = MNPHONE_GprsAttachEx(s_mn_dual_card,MN_PHONE_GPRS_ATTACH);
    DVMTraceErr("===RMT== - MNPHONE_GprsAttachEx   end  ERR=%d\n",err);
    vmtime_sleep(40*1000);

    err = MNGPRS_SetPdpContextEx(s_mn_dual_card,param_used,3,"IP","CMNET",addr,0,0,0);
    DVMTraceErr("===RMT==rmtc_pdpactive - MNGPRS_SetPdpContextEx  ERR=%d\n",err);

    err = MNGPRS_SetPdpContextPcoEx(s_mn_dual_card,3,user,passwd);
    DVMTraceErr("===RMT==rmtc_pdpactive - MNGPRS_SetPdpContextPcoEx  ERR=%d\n",err);

    err = MNGPRS_ActivatePdpContextEx(s_mn_dual_card, TRUE, pdp_id_arr, BROWSER_E, MN_GSM_PREFER);
    DVMTraceErr("===RMT==rmtc_pdpactive  MNGPRS_ActivatePdpContextEx ERR=%d\n",err);
}

//temp defined here to avoid build error. please remove this after network
//profile is refactor.
bool_t rmtc_activeNetwowrk()
{
    DVMTraceErr("===RMT==rmtc_activeNetwowrk - SCI_RegisterMsg  rmtc_pdpCallbck\n");
    SCI_RegisterMsg(MN_APP_GPRS_SERVICE,
                (uint8)EV_MN_APP_ACTIVATE_PDP_CONTEXT_CNF_F,
                (uint8)(MAX_MN_APP_GPRS_EVENTS_NUM-1),
                rmtc_pdpCallbck);

    rmtc_pdpactive();
    
    while(TRUE)
    {
    	if(isActived && isSuccess){
    		DVMTraceErr("===RMT:active pdp sucess!\n");
    		return TRUE;
    	}
    	else if(isActived && !isSuccess){
    		DVMTraceErr("===RMT:active pdp fail!\n");
    		return FALSE;
    	}
    	else
    	{
    		DVMTraceErr("===RMT:sleep 1s to wait!\n");
    		vmtime_sleep(1000);
    	}    			
    }
}

void rmtc_deactiveNetwowrk()
{
    ERR_MNGPRS_CODE_E err = ERR_MNGPRS_NO_ERR;
    DVMTraceErr("===RMT== rmtc_deactiveNetwowrk MNGPRS_ResetAndDeactivePdpContextEx--------\n");
    err = MNGPRS_ResetAndDeactivePdpContextEx(s_mn_dual_card);
    DVMTraceErr("===RMT== rmtc_deactiveNetwowrk MNGPRS_ResetAndDeactivePdpContextEx--------err=%d\n",err);
}
#endif

bool_t StartupNetwork()
{
#ifdef ARCH_X86
	WSADATA wsaData;
    struct sockaddr_in sa;
    if (!wsaStarted && (WSAStartup(MAKEWORD(2,2), &wsaData) != NO_ERROR))
    {
        DVMTraceErr("WSAStartup failure, error code(%d)\n", WSAGetLastError());
        return FALSE;
    }
    wsaStarted = TRUE;
    return TRUE;
#elif defined(ARCH_ARM_SPD)	
	return rmtc_activeNetwowrk();
#endif
}

/* see opl_rams.h */
int32_t getRemoteServerInstance(uint32_t address, uint16_t port)
{
#ifdef ARCH_X86
    int32_t fd;
    struct sockaddr_in sa;
    /*
    WSADATA wsaData;    
    if (!wsaStarted && (WSAStartup(MAKEWORD(2,2), &wsaData) != NO_ERROR))
    {
        DVMTraceErr("WSAStartup failure, error code(%d)\n", WSAGetLastError());
        return -1;
    }
    wsaStarted = TRUE;
    */
    if(!StartupNetwork())
    	return -1;
    	
    fd = (int32_t)socket(PF_INET, SOCK_STREAM, 0);
    if (fd == INVALID_SOCKET)
    {
        DVMTraceErr("getRemoteServerInstance: create socket failure\n");
        return -1;
    }

    lockObjInst = createLockObject();
    SET_NON_BLOCKING(fd);

    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    *((uint32_t *)&sa.sin_addr) = htonl(address);

    while (connect(fd, (struct sockaddr *)&sa, sizeof(sa)) == SOCKET_ERROR)
    {
        int32_t errCode =  WSAGetLastError();
        if (errCode == WSAEWOULDBLOCK)
        {
            //non-blocking mode, wait 1000ms, then try again.
            waitObjectSignalOrTimeout(lockObjInst, 1000);
            continue;
        }
        else if (errCode == WSAEISCONN)
        {
            //already connected.
            break;
        }

        /* connected failure */
        DVMTraceErr("getRemoteServerInstance: socket connect error(%d)\n", WSAGetLastError());
        destroyLockObject(lockObjInst);
        closesocket(fd);
        return -1;
    }
    return fd;

#elif defined(ARCH_ARM_SPD)
	int32_t fd;
	struct sci_sockaddr ssa;
    if(!StartupNetwork())
    	return -1;
    	
    DVMTraceErr("pdp activate success...\n");
    
    fd = sci_sock_socket(AF_INET, SOCK_STREAM, 0, netId);
    if(fd<=0)
    {
    	DVMTraceErr("socket init error\n");
    	return -1;
    }
    
    sci_sock_setsockopt(fd, SO_NBIO, NULL);
    DVMTraceErr("ip:0x%x,port:%d\n",address,port);
    ssa.family = AF_INET;
    ssa.port = (uint16_t)htons(port);
    ssa.ip_addr = htonl(address);
    
	while(sci_sock_connect(fd, &ssa, sizeof(struct sci_sockaddr)) == TCPIP_SOCKET_ERROR)
	{
		int32_t errCode = sci_sock_errno(fd);
		DVMTraceErr("sci_sock_connect error,code:%d\n",errCode);
		
		if(errCode == EWOULDBLOCK ||
		   errCode == EINPROGRESS)
		{
			DVMTraceErr("sci_sock_connect blocking,wait 1s\n");
			vmtime_sleep(1000);
			DVMTraceErr("sci_sock_connect blocking,wake up\n");
			continue;	
		}
		else if(errCode == EISCONN)
		{
			DVMTraceErr("sci_sock_connect connected,exit\n");
			break;	
		}
		else
		{			
			DVMTraceErr("sci_sock_connect fail,bad luck\n");
			sci_sock_socketclose(fd);
    		return -1;
		}
	}
	
	//connect success;
    DVMTraceErr("socket connect success~ sock=%d,netId=%d\n",fd,netId);
    return fd;
#endif
    return 0;
}

/* see opl_rams.h */
int32_t sendData(int32_t instance, uint8_t* buf, int32_t bufSize)
{
    int32_t ret;
#ifdef ARCH_X86
    ret = send(instance, buf, bufSize, 0);
    while (ret == SOCKET_ERROR)
    {
        int32_t errCode =  WSAGetLastError();
        if (errCode == WSAEWOULDBLOCK)
        {
            //non-blocking mode, wait 1000ms, then try again.
            waitObjectSignalOrTimeout(lockObjInst, 1000);
            continue;
        }
        DVMTraceErr("getRemoteServerInstance: send error(%d)\n", errCode);
        break;
    }
#elif defined(ARCH_ARM_SPD)
    //TODO:
    ret = sci_sock_send(instance, (char*)buf, bufSize, 0);
    while (ret == TCPIP_SOCKET_ERROR)
    {
        int32_t errCode =  sci_sock_errno(instance);
        if(errCode == EWOULDBLOCK ||
		   errCode == EINPROGRESS)
		{
            //non-blocking mode, wait 1000ms, then try again.
            vmtime_sleep(1000);
            continue;
        }
        DVMTraceErr("getRemoteServerInstance: send error(%d)\n", errCode);
        break;
    }
#endif

    return ret;
}

/* see opl_rams.h */
int32_t recvDataWithTimeout(int32_t instance, uint8_t* buf, int32_t bufSize, int32_t timeout)
{
    bool_t loop = TRUE;
    int32_t ret;
    do 
    {
#ifdef ARCH_X86
    ret = recv(instance, buf, bufSize, 0);
    DVMTraceErr("sci_sock_recv: ret(%d)\n", ret);
    if (ret == SOCKET_ERROR && loop && timeout > 0)
    {
        int32_t errCode =  WSAGetLastError();
        DVMTraceErr("getRemoteServerInstance: recv error(%d)\n", errCode);
        if (errCode == WSAEWOULDBLOCK)
        {
            waitObjectSignalOrTimeout(lockObjInst, timeout);
            loop = !loop;
        }
    }
    else
    {
        break;
    }

#elif defined(ARCH_ARM_SPD)
    //TODO:
    ret = sci_sock_recv(instance, buf, bufSize, 0);
    DVMTraceErr("sci_sock_recv: ret(%d)\n", ret);
    if (ret == TCPIP_SOCKET_ERROR && loop && timeout > 0)
    {
        int32_t errCode =  sci_sock_errno(instance);
        DVMTraceErr("getRemoteServerInstance: recv error(%d)\n", errCode);
        if(errCode == EWOULDBLOCK ||
		   errCode == EINPROGRESS)
		{
			vmtime_sleep(timeout);
            loop = !loop;
        }
    }
    else
    {
        break;
    }
#endif
    } while(TRUE);

    return ret;
}

/* see opl_rams.h */
int32_t recvData(int32_t instance, uint8_t* buf, int32_t bufSize)
{
    return recvDataWithTimeout(instance, buf, bufSize, 0);
}

/* see opl_rams.h */
int32_t destroyRemoteServerInstance(int32_t instance)
{
#ifdef ARCH_X86
    destroyLockObject(lockObjInst);
    closesocket(instance);
    WSACleanup();
    wsaStarted = FALSE;
#elif defined(ARCH_ARM_SPD)
	sci_sock_socketclose(instance);
	rmtc_deactiveNetwowrk();
#endif
    return 0;
}



#ifdef ARCH_X86
static DWORD WINAPI ramsClientThreadFunc(PVOID pvParam)
{
    RAMSThreadFunc ramsProc = (RAMSThreadFunc)pvParam;
    ramsProc(0, NULL);
    return 0;
}
#elif defined(ARCH_ARM_SPD)
//TODO:
#endif

/* see opl_rams.h */
int32_t ramsCreateClientThread(RAMSThreadFunc pRamsThreadProc, int argc, void* argv[])
{
    UNUSED(argc);
    UNUSED(argv);
#ifdef ARCH_X86
    {
        HANDLE   handle;

/** Ignored parameters. TBD
        uint32_t params[3] = {
            (uint32_t)pRamsThreadProc,
            argc, //argc is >= 0
            (uint32_t)argv
        };
*/
        handle = CreateThread(NULL, 0, ramsClientThreadFunc, (LPVOID)pRamsThreadProc, 0, 0);
        if (handle > 0)
            return (int32_t)handle;
    }
#elif defined(ARCH_ARM_SPD)
    //TODO:
#endif
    return 0;
}

#ifdef ARCH_X86
static DWORD WINAPI VMThreadFunc(PVOID pvParam)
{
    uint32_t* params = (uint32_t*)pvParam;
    RAMSThreadFunc vmProc = (RAMSThreadFunc)params[0];
    int32_t argc = (int32_t)params[1];
    void** argv = (void**)params[2];

    vmProc(argc, argv);
    return 0;
}
#elif defined(ARCH_ARM_SPD)
uint32_t g_dthing_threadid = SCI_INVALID_BLOCK_ID;
uint8*   g_dthing_mem_space_ptr = NULL;
#endif

/*Should be handled by another thread!*/
void cleanVmThread()
{
	
#ifdef ARCH_X86

#elif defined(ARCH_ARM_SPD)
	DVMTraceInf("clean up dthing vm thread\n");
	if(SCI_INVALID_BLOCK_ID != g_dthing_threadid)
    {
        SCI_TerminateThread(g_dthing_threadid);
        SCI_DeleteThread(g_dthing_threadid);
        g_dthing_threadid = SCI_INVALID_BLOCK_ID;
    }

    if(NULL != g_dthing_mem_space_ptr)
    {
        SCI_FREE(g_dthing_mem_space_ptr);
        g_dthing_mem_space_ptr = PNULL;
    }

#endif	
}

int32_t ramsCreateVMThread(DVMThreadFunc pDvmThreadProc, int argc, void* argv[])
{
#ifdef ARCH_X86	
    HANDLE handle;
    static uint32_t params[3] = {0x0,};//static variable is better, any side effect?
    params[0] = (uint32_t)pDvmThreadProc;
    params[1] = argc;
    params[2] = (uint32_t)argv;

    handle = CreateThread(NULL, 0, VMThreadFunc, (LPVOID)params, 0, 0);
    if (handle > 0)
        return (int32_t)handle;
    return -1;
#elif defined (ARCH_ARM_SPD)    
	
	#define DTHING_VM_THREAD_STACK_SIZE 	(16*1024)
	#define DTHING_VM_THREAD_QUEUE_SIZE 	(20*sizeof(uint32)*SCI_QUEUE_ITEM_SIZE)
	DVMTraceInf("Ready to launch dthing vm thread!\n");    
    //try to alloc java heap
    cleanVmThread();
    g_dthing_mem_space_ptr = (uint8*)SCI_ALLOCA(DTHING_VM_THREAD_STACK_SIZE +DTHING_VM_THREAD_QUEUE_SIZE);

    DVMTraceInf("Alloc g_dthing_mem_space_ptr = 0x%x\n",g_dthing_mem_space_ptr);    
    if(PNULL == g_dthing_mem_space_ptr)
    {
        DVMTraceErr("Error:alloc dthing vm memory fail!\n");
        return -1;
    }

    g_dthing_threadid = SCI_CreateAppThreadEx
                (
                    "DthingVmTask",
                    (void (*)(uint32_t, void*))pDvmThreadProc,
                    argc,
                    (void*)argv,
                    g_dthing_mem_space_ptr,
                    DTHING_VM_THREAD_STACK_SIZE,
                    PRI_DTHING_TASK,
                    SCI_PREEMPT,
                    "DthingVmQueue",
                    (g_dthing_mem_space_ptr+DTHING_VM_THREAD_STACK_SIZE),
                    DTHING_VM_THREAD_QUEUE_SIZE,
                    SCI_AUTO_START
                );
                
    DVMTraceInf("Create dthing vm thread success! thread id = %d \n",g_dthing_threadid);                
	return g_dthing_threadid;
#endif
	return -1;
}

/* see opl_rams.h */
int32_t createLockObject()
{
#ifdef ARCH_X86
    HANDLE loInst = CreateSemaphore(NULL, 0, 0xFF, NULL);
    if (loInst == NULL)
    {
        DVMTraceErr("createLockObject, failure!\n");
        return 0;
    }
    return (int32_t)loInst;
#elif defined(ARCH_ARM_SPD)
    //TODO:
#endif
}
/* see opl_rams.h */
bool_t  destroyLockObject(int32_t instance)
{
#ifdef ARCH_X86
    if (CloseHandle((HANDLE)instance))
    {
        return TRUE;
    }
    return FALSE;
#elif defined(ARCH_ARM_SPD)
    //TODO:
    return FALSE;
#endif
}

/* see opl_rams.h */
bool_t waitObjectSignalOrTimeout(int32_t instance, int32_t timeout)
{
#ifdef ARCH_X86
    int32_t tVal, res;
    if (timeout > 0)
    {
        tVal = timeout;
    }
    else if (timeout == LOCKOBJECT_TIMEOUT_NOWAIT)
    {
        tVal = 0;
    }
    else if (timeout == LOCKOBJECT_TIMEOUT_INFINITE)
    {
        tVal = INFINITE;
    }
    else
    {
        DVMTraceErr("waitObjectSignalOrTimeout, unknown timeout value(%d)\n", timeout);
        return FALSE;
    }
    res = WaitForSingleObject((HANDLE)instance, tVal);
    
    if (res == WAIT_FAILED)
    {
        DVMTraceErr("waitObjectSignalOrTimeout failure (%d)\n", GetLastError());
        return FALSE;
    }
    return TRUE;

#elif defined(ARCH_ARM_SPD)
    //TODO:
#endif
}
/* see opl_rams.h */
bool_t releaseLockObject(int32_t instance)
{
#ifdef ARCH_X86
    if (ReleaseSemaphore((HANDLE)instance, 1, NULL))
    {
        return TRUE;
    }
    return FALSE;
#elif defined(ARCH_ARM_SPD)
    //TODO:
#endif
}
