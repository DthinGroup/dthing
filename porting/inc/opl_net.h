#ifndef __OPL_NET_H__
#define __OPL_NET_H__

/** Successful operation. */
#define OPL_NET_SUCCESS            0
/** General failure. */
#define OPL_NET_ERROR             (-1)
/** Connection refused. */
#define OPL_NET_CONNREFUSED       (-2)
/** Operation will be handled asynchronously */
#define OPL_NET_WOULDBLOCK        (-100)

#define INADDR4SZ  (4)
#define INADDR16SZ (16)

int Opl_net_activate(void);

int Opl_net_deactivate(void);

int Opl_net_isActivated(void);

int Opl_net_startup(void);

int Opl_net_streamSocket();

int Opl_net_datagramSocket();

int Opl_net_connect(int sock,int ip,int port,int timeout);

int Opl_net_recv(int sock,char * recvbuf,int count);

int Opl_net_send(int sock,char * sendbuf,int count);

int Opl_net_recvfrom(int sock,char * recvbuf,int count,int ip,int port);

int Opl_net_sendto(int sock,char * sendbuf,int count,int ip,int port);

int Opl_net_shutdown(int sock, int isInput);

int Opl_net_gethostbyname(uint16_t* host, int hostLen, char* addrArrPtr, int addrArrLen);

int Opl_net_closeSocket(int socket);

int Opl_net_getNetId();

#endif