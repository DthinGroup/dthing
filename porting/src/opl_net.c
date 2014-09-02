#include "AsyncIO.h"
#include "opl_net.h"
#include "vm_common.h"

#ifdef ARCH_X86
#include <stdio.h>
#include <Winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <os_api.h>
#include <priority_app.h>
#include <socket_api.h>
#include <socket_types.h>
#include <tcpip_types.h>
#include <mn_type.h>
#include <Mn_events.h>
#include <mn_api.h>
#include <mn_error.h>
#endif

#define DVM_PDP_ACTIVE_TIMEOUT_MS         (1 * 1 * 1000)
#define DVM_RECONNECT_TIMEOUT_MS          (1 * 1 * 1000)


#define NetLog	printf


static int s_net_inited = FALSE;
static int commonResult = FALSE;
static ASYNC_Notifier * commonNotifier;


#ifdef ARCH_ARM_SPD

#define MAX_PAP_USER_LEN 64
#define MAX_PAP_PASSWD_LEN 64

static uint32_t       netId =0;
static int            retrytimes;
static BOOLEAN            isActived = FALSE;

static void net_pdpCallbck(uint32 id, uint32 argc, void *argv)
{
    APP_MN_GPRS_EXT_T *signal_ptr = (APP_MN_GPRS_EXT_T *)argv;
    uint32 pdp_id_arr[MN_GPRS_MAX_PDP_CONTEXT_COUNT] = {0,0,0,0,0,0,0,0,0,0,0};
    pdp_id_arr[2]=1;

    SCI_TRACE_LOW("==RMT==net_pdpCallbck - EV_MN_APP_SET_PDP_CONTEXT_CNF_F=%d", EV_MN_APP_SET_PDP_CONTEXT_CNF_F);
    SCI_TRACE_LOW("==RMT==net_pdpCallbck - INFO: id (%d)  argc(%d)", id, argc);
    SCI_TRACE_LOW("==RMT==net_pdpCallbck - INFO: signal_ptr->result (%d)", signal_ptr->result);
    SCI_TRACE_LOW("==RMT==net_pdpCallbck - INFO: signal_ptr->dual_sys (%d)", signal_ptr->dual_sys);
    SCI_TRACE_LOW("==RMT==net_pdpCallbck - INFO: signal_ptr->pdp_id (%d)", signal_ptr->pdp_id);
    SCI_TRACE_LOW("==RMT==net_pdpCallbck - INFO: signal_ptr->nsapi (%d)", signal_ptr->nsapi);

    if(EV_MN_APP_ACTIVATE_PDP_CONTEXT_CNF_F == argc)
    {
        if(MN_GPRS_ERR_SUCCESS == signal_ptr->result || MN_GPRS_ERR_PDP_CONTEXT_ACTIVATED == signal_ptr->result )
        {
            netId = (uint32)signal_ptr->nsapi;
            SCI_TRACE_LOW("==RMT== net_pdpCallbck()-------gprs ok--------isActived=%d netId=%d",isActived,netId);
            isActived = TRUE;
        }
        else if(MN_GPRS_ERR_RETRYING == signal_ptr->result)
        {
            SCI_TRACE_LOW("==RMT== net_pdpCallbck MN_GPRS_ERR_RETRYING == signal_ptr->result retrytimes=%d",retrytimes);
            SCI_Sleep(5000);
            if(retrytimes < 5)
            {
                //if(ERR_MNGPRS_NO_ERR != MNGPRS_SetAndActivePdpContextEx(MN_DUAL_SYS_1,(uint8*)"CMNET",BROWSER_E))
                if(ERR_MNGPRS_NO_ERR != MNGPRS_ActivatePdpContextEx(MN_DUAL_SYS_1, TRUE, pdp_id_arr, BROWSER_E, MN_GSM_PREFER))
                {
                    SCI_TRACE_LOW("==RMT== net_pdpCallbck 2  MNGPRS_SetAndActivePdpContextEx error");
                }
                else
                {
                    SCI_TRACE_LOW("==RMT== net_pdpCallbck 2 MNGPRS_SetAndActivePdpContextEx waiting for gprs rsp");
                }
            }
            retrytimes++;
        }
        else
        {
            SCI_Sleep(5000);
            if(retrytimes < 5)
            {
                //if(ERR_MNGPRS_NO_ERR != MNGPRS_SetAndActivePdpContextEx(MN_DUAL_SYS_1,(uint8*)"CMNET",BROWSER_E))
                if(ERR_MNGPRS_NO_ERR != MNGPRS_ActivatePdpContextEx(MN_DUAL_SYS_1, TRUE, pdp_id_arr, BROWSER_E, MN_GSM_PREFER))
                {
                    SCI_TRACE_LOW("==RMT== net_pdpCallbck 3  MNGPRS_SetAndActivePdpContextEx error");
                }
                else
                {
                    SCI_TRACE_LOW("==RMT== net_pdpCallbck 3 MNGPRS_SetAndActivePdpContextEx waiting for gprs rsp");
                }
            }
            retrytimes++;
            SCI_TRACE_LOW("==RMT== net_pdpCallbck()-------gprs err--------");
        }
    }
    else if(EV_MN_APP_DEACTIVATE_PDP_CONTEXT_CNF_F == argc)
    {
        SCI_TRACE_LOW("==RMT== net_pdpCallbck()-------EV_MN_APP_DEACTIVATE_PDP_CONTEXT_CNF_F in--------signal_ptr->result=&d",signal_ptr->result);
        if(MN_GPRS_ERR_SUCCESS == signal_ptr->result)
        {
            SCI_UnregisterMsg(MN_APP_GPRS_SERVICE,
                        (uint8)EV_MN_APP_ACTIVATE_PDP_CONTEXT_CNF_F,
                        (uint8)(MAX_MN_APP_GPRS_EVENTS_NUM-1),
                        net_pdpCallbck);
        }
    }
    else
    {
        SCI_TRACE_LOW("==RMT== net_pdpCallbck()-------not EV_MN_APP_ACTIVATE_PDP_CONTEXT_CNF_F event--------");
    }
}

int net_pdpactive()
{
    int times = 30;
    ERR_MNGPRS_CODE_E err = ERR_MNGPRS_NO_ERR;
    ERR_MNPHONE_CODE_E err2 = ERR_MNPHONE_NO_ERR;
    uint32 pdp_id_arr[MN_GPRS_MAX_PDP_CONTEXT_COUNT] = {0,0,0,0,0,0,0,0,0,0,0};
    BOOLEAN param_used[7]={TRUE,TRUE,TRUE,FALSE,FALSE,FALSE,FALSE};
    uint8  user[MAX_PAP_USER_LEN + 1]={0};
    uint8  passwd[MAX_PAP_PASSWD_LEN + 1] = {0};
    MN_GPRS_PDP_ADDR_T addr={0};
    pdp_id_arr[2] = 1;

    err = MNGPRS_SetPdpContextEx(MN_DUAL_SYS_1,param_used,3,"IP","CMNET",addr,0,0,0);
    SCI_TRACE_LOW("==RMT==net_pdpactive - MNGPRS_SetPdpContextEx  ERR=%d",err);

    err = MNGPRS_SetPdpContextPcoEx(MN_DUAL_SYS_1,3,user,passwd);
    SCI_TRACE_LOW("==RMT==net_pdpactive - MNGPRS_SetPdpContextPcoEx  ERR=%d",err);

    //SCI_TRACE_LOW("==RMT==net_pdpactive - ERR_MNGPRS_CALL_FAILURE=%d ",ERR_MNGPRS_CALL_FAILURE);
    err = MNGPRS_ActivatePdpContextEx(MN_DUAL_SYS_1, TRUE, pdp_id_arr, BROWSER_E, MN_GSM_PREFER);
    SCI_TRACE_LOW("==RMT==net_pdpactive  MNGPRS_ActivatePdpContextEx ERR=%d",err);

    while(times-->0)
    {
        if(isActived == TRUE)
        {
            SCI_TRACE_LOW("==RMT== while true isActived == 1");
            break;
        }
        SCI_Sleep(1000);
    }

    return err;
}

//temp defined here to avoid build error. please remove this after network
//profile is refactor.
int  net_activeNetwowrk()
{
    ERR_MNGPRS_CODE_E err = ERR_MNGPRS_NO_ERR;
    ERR_MNPHONE_CODE_E err2 = ERR_MNPHONE_NO_ERR;
    BOOLEAN param_used[7]={TRUE,TRUE,TRUE,FALSE,FALSE,FALSE,FALSE};
    uint8  user[MAX_PAP_USER_LEN + 1]={0};
    uint8  passwd[MAX_PAP_PASSWD_LEN + 1] = {0};
    MN_GPRS_PDP_ADDR_T addr={0};

    SCI_TRACE_LOW("==RMT==  MNPHONE_StartupPsEx -  start");
    err = MNPHONE_StartupPsEx(MN_DUAL_SYS_1,MN_GMMREG_RAT_GPRS);
    SCI_TRACE_LOW("==RMT==  MNPHONE_StartupPsEx -  end  ERR=%d",err);
    SCI_Sleep(40*1000);

    SCI_TRACE_LOW("==RMT==  MNPHONE_GprsAttachEx -  start");
    err = MNPHONE_GprsAttachEx(MN_DUAL_SYS_1,MN_PHONE_GPRS_ATTACH);
    SCI_TRACE_LOW("==RMT== - MNPHONE_GprsAttachEx   end  ERR=%d",err);
    SCI_Sleep(40*1000);

    SCI_TRACE_LOW("==RMT==net_activeNetwowrk - SCI_RegisterMsg  net_pdpCallbck");
    SCI_RegisterMsg(MN_APP_GPRS_SERVICE,
                (uint8)EV_MN_APP_ACTIVATE_PDP_CONTEXT_CNF_F,
                (uint8)(MAX_MN_APP_GPRS_EVENTS_NUM-1),
                net_pdpCallbck);

    return net_pdpactive();
}

void net_deactiveNetwowrk()
{
    ERR_MNGPRS_CODE_E err = ERR_MNGPRS_NO_ERR;
    SCI_TRACE_LOW("==RMT== net_deactiveNetwowrk MNGPRS_ResetAndDeactivePdpContextEx--------");
    err = MNGPRS_ResetAndDeactivePdpContextEx(MN_DUAL_SYS_1);
    SCI_TRACE_LOW("==RMT== net_deactiveNetwowrk MNGPRS_ResetAndDeactivePdpContextEx--------err=%d",err);
}
#endif

int Opl_net_getNetId()
{
#ifdef ARCH_ARM_SPD
    SCI_TRACE_LOW("==RMT==net id:%d",netId);
	return netId;
#endif	
	return 0;
}

int Opl_net_activate(void)
{
    NetLog("call Opl_net_activate");
#ifdef ARCH_X86

#elif defined(ARCH_ARM_SPD)
	net_activeNetwowrk();
#endif
    NetLog("call Opl_net_activate over");
}

int Opl_net_deactivate(void)
{
#ifdef ARCH_X86

#elif defined(ARCH_ARM_SPD)
    net_deactiveNetwowrk();
#endif
}

int Opl_net_isActivated(void)
{
#ifdef ARCH_X86
	return FALSE;
#elif defined(ARCH_ARM_SPD)
    SCI_TRACE_LOW("Opl_net_isActivated:%d",isActived);
    return isActived;
#endif	
}


int Opl_net_startup(void)
{

	if(s_net_inited)
	{
		return OPL_NET_SUCCESS;
	}

	if (AsyncIO_firstCall())
    {
		/*
        commonResult = FALSE;
        netId = 0;

        NetLog(("===Opl_net_startup - ASYNC_firstCall in"));
        Jbed_conn_activeNetwowrk(runningId);
        NetLog(("===net:CPL_net_startup - Jbed_conn_activeNetwowrk called"));
        //netUtil_pdpCallbck() should have been called before this state.
        if(netId != 0 && commonResult)
        {
            NetLog(("===net:CPL_net_startup - commonResult==TRUE retrun"));
            commonResult = FALSE;
            return CPL_NET_SUCCESS;
        }

        
        NetLog(("===net:CPL_net_startup - ASYNC_callAgainWhenSignalledOrTimeout called"));
        commonNotifier = Async_getCurNotifier();
        NetLog(("===net:CPL_net_startup - ASYNC_getNotifier called"));
        ASYNC_setInvalidateFunc(netUtil_checkNotifierValidity);
        NetLog(("===net:CPL_net_startup - ASYNC_setInvalidateFunc called"));
        NetLog(("===net:CPL_net_startup - INFO: activeNetwork would block"));
		*/
        AsyncIO_callAgainWhenSignalledOrTimeOut(DVM_PDP_ACTIVE_TIMEOUT_MS);
		return OPL_NET_WOULDBLOCK;
    }
    else
    {
        NetLog(("===CPL_net_startup - not ASYNC_firstCall in"));
        //if (commonResult)
        {
#ifdef ARCH_X86
			WORD wVersionRequested;
			WSADATA wsaData;//WSAata用来存储系统传回的关于WinSocket的资料。
			int err;

			wVersionRequested = MAKEWORD( 1, 1 );

			err = WSAStartup( wVersionRequested, &wsaData );
			if ( err != 0 ) {
				return OPL_NET_ERROR;
			}

			if ( LOBYTE( wsaData.wVersion ) != 1 ||HIBYTE( wsaData.wVersion ) != 1 ) 
			{
				WSACleanup( );
				return OPL_NET_ERROR;
			}
#elif defined(ARCH_ARM_SPD)
			if(!Opl_net_isActivated())
			{
				return OPL_NET_ERROR;
			}
#endif
			s_net_inited = TRUE;
            commonResult = FALSE;
            NetLog(("===net:CPL_net_startup - INFO: activeNetwork success!"));
            return OPL_NET_SUCCESS;
        }
    }
}

int Opl_net_streamSocket()
{
	int sock = 0;
	/*
	sock = sci_sock_socket(AF_INET, SOCK_STREAM, 0, netId);
    if (sock <= 0)
    {
        NetLog(("===net:CPL_net_clientSocket - ERROR: create socket fail (%d)(%d)", *sock, netId));
        //create socket failure.
        return CPL_NET_ERROR;
    }
	*/
#ifdef ARCH_X86
	SOCKET sockClient=socket(AF_INET,SOCK_STREAM,0);// AF_INET ..tcp连接
	sock = (int)sockClient;
#elif defined(ARCH_ARM_SPD)
	sock = sci_sock_socket(AF_INET, SOCK_STREAM, 0,  Opl_net_getNetId());
	sci_sock_setsockopt(sock, SO_NBIO, NULL);
#endif

	return sock;
}

int Opl_net_datagramSocket()
{
	int sock = 0;

#ifdef ARCH_X86
	SOCKET sockClient=socket(AF_INET,SOCK_DGRAM,0);// AF_INET ..tcp连接
	sock = (int)sockClient;
#elif defined(ARCH_ARM_SPD)
	sock = sci_sock_socket(AF_INET, SOCK_DGRAM, 0,  Opl_net_getNetId());
#endif

	return sock;
}

int Opl_net_connect(int sock,int ip,int port,int timeout)
{
	int ret =0;
	char ipbuf[16] = {0};
	
#ifdef ARCH_X86
	SOCKADDR_IN addrSrv;

	sprintf(ipbuf,"%d.%d.%d.%d",0xff&(ip>>24),0xff&(ip>>16),0xff&(ip>>8),ip&0xff);
    addrSrv.sin_addr.S_un.S_addr=inet_addr(ipbuf/*"127.0.0.1"*/);//本机地址，服务器在本机开启
    addrSrv.sin_family=AF_INET;
    addrSrv.sin_port=htons(port);// 设置端口号
    ret = connect((SOCKET)sock,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR));//连接服务器
#elif defined(ARCH_ARM_SPD)
	struct sci_sockaddr ssa;
	
	ssa.family = AF_INET;
	ssa.port = (uint16_t)htons(port);
    ssa.ip_addr = htonl(ip);
    
	if (AsyncIO_firstCall())
	{
		if (sci_sock_connect(sock, &ssa, sizeof(struct sci_sockaddr)) == TCPIP_SOCKET_ERROR)
		{
			int32_t errCode = sci_sock_errno(sock);
			DVMTraceErr("sci_sock_connect error,code:%d\n",errCode);

			if(errCode == EWOULDBLOCK || errCode == EINPROGRESS)
			{
				DVMTraceDbg("sci_sock_connect EWOULDBLOCK,try later\n");
				AsyncIO_callAgainWhenSignalledOrTimeOut(DVM_RECONNECT_TIMEOUT_MS);
				return OPL_NET_WOULDBLOCK;
			}
			else
			{
				DVMTraceErr("sci_sock_connect fail,return bad\n");
				sci_sock_socketclose(sock);
				return OPL_NET_ERROR;
			}
		}
	}
	else
	{
		if (sci_sock_connect(sock, &ssa, sizeof(struct sci_sockaddr)) == TCPIP_SOCKET_ERROR)
		{			
			DVMTraceErr("sci_sock_connect fail again,return bad\n");
			sci_sock_socketclose(sock);
			return OPL_NET_ERROR;
		}
		else
		{
			return OPL_NET_SUCCESS;
		}
	}
	
#endif
	return ret;
}


int Opl_net_send(int sock,char * sendbuf,int count)
{
	int ret = 0;
#ifdef ARCH_X86	
	ret = send((SOCKET)sock,sendbuf,count,0);
#elif defined(ARCH_ARM_SPD)	
	if (AsyncIO_firstCall())
	{
		ret = sci_sock_send(sock, (char*)sendbuf, count, 0);
		if (ret == TCPIP_SOCKET_ERROR)
		{
			int32_t errCode =  sci_sock_errno(sock);
			if (errCode == EWOULDBLOCK || errCode == EINPROGRESS)
			{
				DVMTraceDbg("first:Opl_net_send wouldblock,retry later!\n");
				AsyncIO_callAgainWhenSignalledOrTimeOut(DVM_RECONNECT_TIMEOUT_MS);
				ret = OPL_NET_WOULDBLOCK;
			}
			else
			{
				DVMTraceDbg("first:Opl_net_send error,return!\n");
				ret = OPL_NET_ERROR;
			}
		}
	}
	else
	{
		ret = sci_sock_send(sock, (char*)sendbuf, count, 0);
		if (ret == TCPIP_SOCKET_ERROR)
		{
			int32_t errCode =  sci_sock_errno(sock);
			if (errCode == EWOULDBLOCK || errCode == EINPROGRESS)
			{
				DVMTraceDbg("second:Opl_net_send wouldblock,retry later!\n");
				AsyncIO_callAgainWhenSignalledOrTimeOut(DVM_RECONNECT_TIMEOUT_MS);
				ret = OPL_NET_WOULDBLOCK;
			}
			else
			{
				DVMTraceDbg("second:Opl_net_send error,return!\n");
				ret = OPL_NET_ERROR;
			}
		}
	}
#endif
	return ret;
}

int Opl_net_recv(int sock,char * recvbuf,int count)
{
	int ret = 0;
#ifdef ARCH_X86	
	ret = recv(sock,recvbuf,count,0);
#elif defined(ARCH_ARM_SPD)

	if (AsyncIO_firstCall())
	{
		ret = sci_sock_recv(sock, (char*)recvbuf, count, 0);
		if (ret == TCPIP_SOCKET_ERROR)
		{
			int32_t errCode =  sci_sock_errno(sock);
			if (errCode == EWOULDBLOCK || errCode == EINPROGRESS)
			{
				DVMTraceDbg("first:Opl_net_recv wouldblock,retry later!\n");
				AsyncIO_callAgainWhenSignalledOrTimeOut(DVM_RECONNECT_TIMEOUT_MS);
				ret = OPL_NET_WOULDBLOCK;
			}
			else
			{
				DVMTraceDbg("secon:Opl_net_recv error,return!\n");
				ret = OPL_NET_ERROR;
			}
		}
	}
	else
	{
		ret = sci_sock_recv(sock, (char*)recvbuf, count, 0);
		if (ret == TCPIP_SOCKET_ERROR)
		{
			int32_t errCode =  sci_sock_errno(sock);
			if (errCode == EWOULDBLOCK || errCode == EINPROGRESS)
			{
				DVMTraceDbg("second:Opl_net_recv wouldblock,retry later!\n");
				AsyncIO_callAgainWhenSignalledOrTimeOut(DVM_RECONNECT_TIMEOUT_MS);
				ret = OPL_NET_WOULDBLOCK;
			}
			else
			{
				DVMTraceDbg("second:Opl_net_recv error,return!\n");
				ret = OPL_NET_ERROR;
			}
		}
	}
	
#endif
	return ret;
}

int Opl_net_recvfrom(int sock,char * recvbuf,int count,int ip,int port)
{
	int ret =0;
	char ipbuf[16] = {0};
#ifdef ARCH_X86	
	SOCKADDR_IN addrSrv;
	int len =0;

	sprintf(ipbuf,"%d.%d.%d.%d",0xff&(ip>>24),0xff&(ip>>16),0xff&(ip>>8),ip&0xff);
    addrSrv.sin_addr.S_un.S_addr=inet_addr(ipbuf/*"127.0.0.1"*/);//本机地址，服务器在本机开启
    addrSrv.sin_family=AF_INET;
    addrSrv.sin_port=htons(port);// 设置端口号	
	//memset(&addrSrv,0,sizeof(SOCKADDR));

	ret = recvfrom(sock,recvbuf,count,0,(SOCKADDR*)&addrSrv,&len);
#elif defined(ARCH_ARM_SPD)
	//not support now
#endif

	return ret;	
}

int Opl_net_sendto(int sock,char * sendbuf,int count,int ip,int port)
{
	int ret =0;
	char ipbuf[16] = {0};
#ifdef ARCH_X86	
	SOCKADDR_IN addrSrv;

	sprintf(ipbuf,"%d.%d.%d.%d",0xff&(ip>>24),0xff&(ip>>16),0xff&(ip>>8),ip&0xff);
    addrSrv.sin_addr.S_un.S_addr=inet_addr(ipbuf/*"127.0.0.1"*/);//本机地址，服务器在本机开启
    addrSrv.sin_family=AF_INET;
    addrSrv.sin_port=htons(port);// 设置端口号	

	ret = sendto(sock,sendbuf,count,0,(SOCKADDR*)&addrSrv,sizeof(SOCKADDR));
#elif defined(ARCH_ARM_SPD)
	//not support now
#endif

	return ret;
}

int Opl_net_closeSocket(int socket)
{
	int ret =0;
#ifdef ARCH_X86	

	ret = closesocket((SOCKET)socket);
#elif defined(ARCH_ARM_SPD)
	ret = sci_sock_socketclose(socket);
#endif
	return ret;
}