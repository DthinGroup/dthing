#include <opl_rams.h>
#include <opl_es.h>
#include <opl_net.h>
#include <eventbase.h>
#include <vm_common.h>

#ifdef ARCH_X86
#include <windows.h>
#elif defined(ARCH_ARM_SPD)
#include <os_api.h>
#include <priority_app.h>
#include <socket_api.h>
#endif

static bool_t  networkStarted = FALSE;

/* X86 definitions. */
#ifdef ARCH_X86
#define SET_NON_BLOCKING(sock) \
    do { \
        unsigned long val = 1; \
        ioctlsocket(sock, FIONBIO, &val); \
    } while(0)
#elif defined(ARCH_ARM_SPD)
#define SET_NON_BLOCKING(sock) \
	do { \
		int ret =0; \
		ret = sci_sock_setsockopt(sock, SO_NBIO, NULL); \
		SCI_TRACE_LOW("===RMT==set opt ret:%d",ret); \
	} while(0)

#endif


/* refer to opl_rams.h */
int32_t rams_startupNetwork()
{
    int32_t res = RAMS_RES_FAILURE;
#ifdef ARCH_X86
	WSADATA wsaData;
    if (!networkStarted && (WSAStartup(MAKEWORD(2,2), &wsaData) != NO_ERROR))
    {
        DVMTraceErr("WSAStartup failure, error code(%d)\n", WSAGetLastError());
        return FALSE;
    }
    networkStarted = TRUE;
    res = RAMS_RES_SUCCESS;
#elif defined(ARCH_ARM_SPD)	
	if(Opl_net_isActivated() ==TRUE)
	{
		networkStarted = TRUE;
		res = RAMS_RES_SUCCESS;
	}
#endif
    return res;
}

/* refer to opl_rams.h */
int32_t rams_connectServer(int32_t address, uint16_t port, int32_t *instance)
{
    int32_t fInst = -1 ;
#ifdef ARCH_X86
    struct sockaddr_in sa;

	DVMTraceDbg("rams_connectServer to 0x%x:%d\n",address,port);
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

    DVMTraceDbg("rams_connectServer to 0x%x:%d\n",address,port);

    if (ES_firstScheduled())
    {
        if (!networkStarted)
        {
            DVMTraceErr("rams_connectServer: network is uninitialized!\n");
            return RAMS_RES_FAILURE;
        }

        fInst = sci_sock_socket(AF_INET, SOCK_STREAM, 0,  Opl_net_getNetId());

        if(fInst <= 0)
        {
    	    DVMTraceErr("socket init error\n");
    	    return RAMS_RES_FAILURE;
        }
        SET_NON_BLOCKING(fInst);

        *instance = fInst;
    }

    if (fInst < 0 || ES_firstScheduled())
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
    DVMTraceErr("socket connect success~ sock=%d,netId=%d\n",fInst, Opl_net_getNetId());
#endif
    return RAMS_RES_SUCCESS;
}

/* refer to opl_rams.h */
int32_t rams_recvData(int32_t instance, uint8_t* buf, int32_t bufSize)
{
    int32_t ret;

#ifdef ARCH_X86
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
#if 1
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
#endif
	DVMTraceDbg("===rams_recvData,handle:0x%x,ret:%d\n",instance,ret);

    return ret;
}

/* refer to opl_rams.h */
int32_t rams_sendData(int32_t instance, uint8_t* buf, int32_t bufSize)
{
    int32_t ret;

#ifdef ARCH_X86
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
#if 1
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
#endif
	DVMTraceDbg("===rams_sendData,handle:0x%x,ret:%d\n",instance,ret);
    return ret;
}

/* refer to opl_rams.h */
int32_t rams_closeConnection(int32_t instance)
{
	DVMTraceDbg("===rams_closeConnection,handle:0x%x\n",instance);
#ifdef ARCH_X86
    closesocket(instance);
#elif defined(ARCH_ARM_SPD)
    sci_sock_socketclose(instance);
#endif
    return RAMS_RES_SUCCESS;
}

/* refer to opl_rams.h */
int32_t rams_shutdownNetwork()
{
#ifdef ARCH_X86
    WSACleanup();
#elif defined(ARCH_ARM_SPD)
    Opl_net_deactivate();
#endif
    networkStarted = FALSE;

    return RAMS_RES_SUCCESS;
}


