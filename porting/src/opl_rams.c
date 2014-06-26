#include <opl_rams.h>
#include <opl_es.h>
#include <eventbase.h>

#ifdef WIN32
#include <windows.h>
#elif defined(ARCH_ARM_SPD)
#endif

static bool_t  networkStarted = FALSE;

/* X86 definitions. */
#ifdef WIN32
#define SET_NON_BLOCKING(sock) \
    do { \
        unsigned long val = 1; \
        ioctlsocket(sock, FIONBIO, &val); \
    } while(0)
#elif defined(ARCH_ARM_SPD)
#define SET_NON_BLOCKING(sock) \
    sci_sock_setsockopt(sock, SO_NBIO, NULL)
#endif


/* refer to opl_rams.h */
int32_t rams_startupNetwork()
{
    int32_t res = RAMS_RES_FAILURE;
#ifdef WIN32
	WSADATA wsaData;
    if (!networkStarted && (WSAStartup(MAKEWORD(2,2), &wsaData) != NO_ERROR))
    {
        DVMTraceErr("WSAStartup failure, error code(%d)\n", WSAGetLastError());
        return FALSE;
    }
    networkStarted = TRUE;
    res = RAMS_RES_SUCCESS;
#elif defined(ARCH_ARM_SPD)	
	res = rmtc_activeNetwowrk();
#endif
    return res;
}

/* refer to opl_rams.h */
int32_t rams_connectServer(int32_t address, uint16_t port, int32_t *instance)
{
    int32_t fInst = -1 ;
#ifdef WIN32
    struct sockaddr_in sa;

    if (ES_firstScheduled())
    {
        if (!networkStarted)
        {
            DVMTraceErr("rams_connectServer: network is uninitialized!\n");
            return RAMS_RES_FAILURE;
        }

        fInst = (int32_t)socket(PF_INET, SOCK_STREAM, 0);
        if (fInst == INVALID_SOCKET)
        {
            DVMTraceErr("rams_connectServer: create socket failure\n");
            return RAMS_RES_FAILURE;
        }

        SET_NON_BLOCKING(fInst);

        *instance = fInst;
    }

    if (fInst < 0 || ES_firstScheduled())
    {
        sa.sin_family = AF_INET;
        sa.sin_port = htons(port);
        *((uint32_t *)&sa.sin_addr) = htonl(address);
        fInst = *instance;
    }

    if (connect(fInst, (struct sockaddr *)&sa, sizeof(sa)) == SOCKET_ERROR)
    {
        int32_t errCode =  WSAGetLastError();
        if (errCode == WSAEWOULDBLOCK || errCode == WSAEINPROGRESS)
        {
            ES_scheduleAgainWithTimeout(1000);
            return RAMS_RES_WOULDBLOCK;
        }
        else if (errCode == WSAEISCONN)
        {
            //already connected.
        }
        else
        {
            /* connected failure */
            DVMTraceErr("getRemoteServerInstance: socket connect error(%d)\n", WSAGetLastError());
            closesocket(fInst);
            return RAMS_RES_FAILURE;
        }
    }

#elif defined(ARCH_ARM_SPD)
	struct sci_sockaddr ssa;

    if (ES_firstScheduled())
    {
        if (!networkStarted)
        {
            DVMTraceErr("rams_connectServer: network is uninitialized!\n");
            return RAMS_RES_FAILURE;
        }

        fInst = sci_sock_socket(AF_INET, SOCK_STREAM, 0, netId);

        if(fInst <= 0)
        {
    	    DVMTraceErr("socket init error\n");
    	    return RAMS_RES_FAILURE;
        }
        SET_NON_BLOCKING(fInst);

        *instance = fInst;
    }

    if (fInst < 0)
    {
        ssa.family = AF_INET;
        ssa.port = (uint16_t)htons(port);
        ssa.ip_addr = htonl(address);
        fInst = *instance;
    }

	if (sci_sock_connect(fInst, &ssa, sizeof(struct sci_sockaddr)) == TCPIP_SOCKET_ERROR)
	{
		int32_t errCode = sci_sock_errno(fInst);
		DVMTraceErr("sci_sock_connect error,code:%d\n",errCode);

		if(errCode == EWOULDBLOCK || errCode == EINPROGRESS)
		{
            ES_scheduleAgainWithTimeout(1000);
			return RAMS_RES_WOULDBLOCK;
		}
		else if(errCode == EISCONN)
		{
            //already connected.
		}
		else
		{			
			DVMTraceErr("sci_sock_connect fail,bad luck\n");
			sci_sock_socketclose(fInst);
            return RAMS_RES_FAILURE;
		}
	}

	//connect success;
    DVMTraceErr("socket connect success~ sock=%d,netId=%d\n",fInst,netId);
#endif
    return RAMS_RES_SUCCESS;
}

/* refer to opl_rams.h */
int32_t rams_recvData(int32_t instance, uint8_t* buf, int32_t bufSize)
{
    int32_t ret;

#ifdef WIN32
    ret = recv(instance, buf, bufSize, 0);

    if (ret == SOCKET_ERROR)
    {
        int32_t errCode =  WSAGetLastError();
        if (errCode == WSAEWOULDBLOCK)
        {
            ES_scheduleAgainWithTimeout(2000);
			ret = RAMS_RES_WOULDBLOCK;
        }
        else
        {
            ret = RAMS_RES_FAILURE;
        }
    }
#elif defined(ARCH_ARM_SPD)
    ret = sci_sock_recv(instance, (char*)buf, bufSize, 0);

    if (ret == TCPIP_SOCKET_ERROR)
    {
        int32_t errCode =  sci_sock_errno(instance);
        if (errCode == EWOULDBLOCK || errCode == EINPROGRESS)
        {
            ES_scheduleAgainWithTimeout(1000);
			ret = RAMS_RES_WOULDBLOCK;
        }
        else
        {
            ret = RAMS_RES_FAILURE;
        }
    }
#endif

    return ret;
}

/* refer to opl_rams.h */
int32_t rams_sendData(int32_t instance, uint8_t* buf, int32_t bufSize)
{
    int32_t ret;

#ifdef WIN32
    ret = send(instance, buf, bufSize, 0);

    if (ret == SOCKET_ERROR)
    {
        int32_t errCode =  WSAGetLastError();
        if (errCode == WSAEWOULDBLOCK)
        {
            ES_scheduleAgainWithTimeout(500);
			ret = RAMS_RES_WOULDBLOCK;
        }
        else
        {
            ret = RAMS_RES_FAILURE;
        }
    }
#elif defined(ARCH_ARM_SPD)
    ret = sci_sock_send(instance, (char*)buf, bufSize, 0);

    if (ret == TCPIP_SOCKET_ERROR)
    {
        int32_t errCode =  sci_sock_errno(instance);
        if (errCode == EWOULDBLOCK || errCode == EINPROGRESS)
        {
            ES_scheduleAgainWithTimeout(1000);
			ret = RAMS_RES_WOULDBLOCK;
        }
        else
        {
            ret = RAMS_RES_FAILURE;
        }
    }
#endif
    return ret;
}

/* refer to opl_rams.h */
int32_t rams_closeConnection(int32_t instance)
{
#ifdef WIN32
    closesocket(instance);
#elif defined(ARCH_ARM_SPD)
    sci_sock_socketclose(instance);
#endif
    return RAMS_RES_SUCCESS;
}

/* refer to opl_rams.h */
int32_t rams_shutdownNetwork()
{
#ifdef WIN32
    WSACleanup();
#elif defined(ARCH_ARM_SPD)
    rmtc_deactiveNetwowrk();
#endif
    networkStarted = FALSE;

    return RAMS_RES_SUCCESS;
}

#ifdef WIN32
static DWORD WINAPI VMThreadFunc(PVOID pvParam)
{
    uint32_t* params = (uint32_t*)pvParam;
    DVMThreadFunc vmProc = (DVMThreadFunc)params[0];
    int32_t argc = (int32_t)params[1];
    char** argv = (char**)params[2];

    vmProc(argc, argv);
    return 0;
}
#elif defined(ARCH_ARM_SPD)
uint32_t g_dthing_threadid = SCI_INVALID_BLOCK_ID;
uint8*   g_dthing_mem_space_ptr = NULL;
#endif

int32_t rams_createVMThread(DVMThreadFunc pDvmThreadProc, int argc, void* argv[])
{
#ifdef WIN32	
    HANDLE handle;
    static uint32_t params[3] = {0x0,};//static variable is better, any side effect?
    params[0] = (uint32_t)pDvmThreadProc;
    params[1] = argc;
    params[2] = (uint32_t)argv;

    handle = CreateThread(NULL, 0, VMThreadFunc, (LPVOID)params, 0, 0);
    if (handle > 0)
        return (int32_t)handle;

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
                    PRI_JBED_TASK,
                    SCI_PREEMPT,
                    "DthingVmQueue",
                    (g_dthing_mem_space_ptr+DTHING_VM_THREAD_STACK_SIZE),
                    DTHING_VM_THREAD_QUEUE_SIZE,
                    SCI_AUTO_START
                );
                
    DVMTraceInf("Create dthing vm thread success! thread id = %d \n",g_dthing_threadid);                
	return g_dthing_threadid;
#endif
	return RAMS_RES_FAILURE;
}
