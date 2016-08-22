/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *    Sergio R. Caprile - "commonalization" from prior samples and/or documentation extension
 *******************************************************************************/

#include <sys/types.h>
#include "global_def.h"

#if !defined(SOCKET_ERROR)
	/** error in socket operation */
	#define SOCKET_ERROR -1
#endif

#if defined(ARCH_X86)
	/* default on Windows is 64 - increase to make Linux and Windows the same */
	#pragma comment (lib,"Ws2_32.lib")

	#define FD_SETSIZE 1024
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#define MAXHOSTNAMELEN 256
	#define EAGAIN WSAEWOULDBLOCK
	#define EINTR WSAEINTR
	#define EINVAL WSAEINVAL
	#define EINPROGRESS WSAEINPROGRESS
	#define EWOULDBLOCK WSAEWOULDBLOCK
	#define ENOTCONN WSAENOTCONN
	#define ECONNRESET WSAECONNRESET
	#define ioctl ioctlsocket
	#define socklen_t int

#elif defined(ARCH_ARM_SPD)

	#include <os_api.h>
	#include <priority_app.h>
	#include <socket_api.h>
	#include <socket_types.h>
	#include <tcpip_types.h>
	#include <mn_type.h>
	#include <Mn_events.h>
	#include <mn_api.h>
	#include <mn_error.h>
	#include <sci_api.h>
#else

	#define INVALID_SOCKET SOCKET_ERROR
	#include <sys/socket.h>
	#include <sys/param.h>
	#include <sys/time.h>
	#include <netinet/in.h>
	#include <netinet/tcp.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <stdio.h>
	#include <unistd.h>
	#include <errno.h>
	#include <fcntl.h>
	#include <string.h>
	#include <stdlib.h>

#endif

#if defined(ARCH_X86)
	#include <Iphlpapi.h>
#elif defined(ARCH_ARM_SPD)

#else

	#include <sys/ioctl.h>
	#include <net/if.h>

#endif

/**
This simple low-level implementation assumes a single connection for a single thread. Thus, a static
variable is used for that connection.
On other scenarios, the user must solve this by taking into account that the current implementation of
MQTTPacket_read() has a function pointer for a function call to get the data to a buffer, but no provisions
to know the caller or other indicator (the socket id): int (*getfn)(unsigned char*, int)
*/
static int mysock = INVALID_SOCKET;


int transport_sendPacketBuffer(int sock, unsigned char* buf, int buflen)
{
	int rc = 0;
#ifdef(ARCH_ARM_SPD)
	rc = sci_sock_send(sock, buf, buflen, 0);
#else 
	rc = send(sock, buf, buflen, 0);//write(sock, buf, buflen);
#endif
	return rc;
}


int transport_getdata(unsigned char* buf, int count)
{
	int rc = 0;
#ifdef (ARCH_ARM_SPD)	
	rc = sci_sock_recv(mysock, buf, count, 0);
#else
	rc = recv(mysock, buf, count, 0);
	//printf("received %d bytes count %d\n", rc, (int)count);

#endif

	return rc;
}

int transport_getdatanb(void *sck, unsigned char* buf, int count)
{
	int sock = *((int *)sck); 	/* sck: pointer to whatever the system may use to identify the transport */
	/* this call will return after the timeout set on initialization if no bytes;
	   in your system you will use whatever you use to get whichever outstanding
	   bytes your socket equivalent has ready to be extracted right now, if any,
	   or return immediately */
	int rc = 0;
#if defined(ARCH_ARM_SPD)

	rc = sci_sock_recv(sock, buf, count, 0);

#else

	rc = recv(sock, buf, count, 0);	

#endif

	if (rc == -1) {
		/* check error conditions from your system here, and return -1 */
		return 0;
	}
	return rc;
}

/**
return >=0 for a socket descriptor, <0 for an error code
@todo Basically moved from the sample without changes, should accomodate same usage for 'sock' for clarity,
removing indirections
*/
#if defined(ARCH_ARM_SPD)

int transport_open(char* addr, int port)
{
	int* sock = &mysock;	
	int rc = -1;	
	static struct timeval tv;	    
	struct sci_sockaddr dst_addr = {0};
	int temp_count = 0;
	
	assert(addr != NULL);

	*sock = -1;

	do
	{
		rc = sci_parse_host(addr, &dst_addr.ip_addr, 0);
		temp_count ++;
		SCI_SLEEP(500);
	}while((rc == 1) && (temp_count < 50));
	
	if(0 != rc)
	{	
		return -1;
	}

	dst_addr.family = AF_INET;
	dst_addr.port = htons(port);

	if (rc == 0)
	{
		*sock =	sci_sock_socket(AF_INET, SOCK_STREAM, 0, 0);		
		if (*sock != -1)
		{
			rc = sci_sock_connect(*sock, (struct sci_sockaddr*)&dst_addr, sizeof(dst_addr));			
		}
	}
	if (mysock == INVALID_SOCKET)
		return rc;

	//tv.tv_sec = 1;  /* 1 second Timeout */
	//tv.tv_usec = 0;  
	///setsockopt(mysock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
	return mysock;
}

#else

int transport_open(char* addr, int port)
{
	int* sock = &mysock;
	int type = SOCK_STREAM;
	struct sockaddr_in address;
#if defined(AF_INET6)
	struct sockaddr_in6 address6;
#endif
	int rc = -1;
#if defined(ARCH_X86)
	short family;
#else
	sa_family_t family = AF_INET;
#endif
	struct addrinfo *result = NULL;
	struct addrinfo hints = {0, AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP, 0, NULL, NULL, NULL};
	static struct timeval tv;

#if defined(ARCH_X86)
	WORD wVersionRequested;
	WSADATA wsaData;//WSAata用来存储系统传回的关于WinSocket的资料。
    int err;
	
	wVersionRequested = MAKEWORD( 1, 1 );
    
	err = WSAStartup( wVersionRequested, &wsaData );
    if ( err != 0 ) {
        return rc;
    }

    if ( LOBYTE( wsaData.wVersion ) != 1 ||HIBYTE( wsaData.wVersion ) != 1 ) 
    {
        WSACleanup( );
        return rc;
    }
#endif


	*sock = -1;
	if (addr[0] == '[')
	  ++addr;


	if ((rc = getaddrinfo(addr, NULL, &hints, &result)) == 0)
	{
		struct addrinfo* res = result;

		// prefer ip4 addresses 
		while (res)
		{
			if (res->ai_family == AF_INET)
			{
				result = res;
				break;
			}
			res = res->ai_next;
		}

#if defined(AF_INET6)
		if (result->ai_family == AF_INET6)
		{
			address6.sin6_port = htons(port);
			address6.sin6_family = family = AF_INET6;
			address6.sin6_addr = ((struct sockaddr_in6*)(result->ai_addr))->sin6_addr;
		}
		else
#endif
		if (result->ai_family == AF_INET)
		{
			address.sin_port = htons(port);
			address.sin_family = family = AF_INET;
			address.sin_addr = ((struct sockaddr_in*)(result->ai_addr))->sin_addr;
		}
		else
			rc = -1;

		freeaddrinfo(result);
	} else {
		address.sin_port = htons(port);
		address.sin_family = family = AF_INET;
		//address.sin_addr = ((struct sockaddr_in*)(result->ai_addr))->sin_addr;
		address.sin_addr.s_addr=inet_addr(addr);
		rc =0;
	}

	if (rc == 0)
	{
		*sock =	socket(family, type, 0);		
		if (*sock != -1)
		{
#if defined(NOSIGPIPE)
			int opt = 1;

			if (setsockopt(*sock, SOL_SOCKET, SO_NOSIGPIPE, (void*)&opt, sizeof(opt)) != 0)
				Log(TRACE_MIN, -1, "Could not set SO_NOSIGPIPE for socket %d", *sock);
#endif

			if (family == AF_INET)
				rc = connect(*sock, (struct sockaddr*)&address, sizeof(address));
	#if defined(AF_INET6)
			else
				rc = connect(*sock, (struct sockaddr*)&address6, sizeof(address6));
	#endif
		}
	}
	if (mysock == INVALID_SOCKET)
		return rc;

	tv.tv_sec = 1;  /* 1 second Timeout */
	tv.tv_usec = 0;  
	setsockopt(mysock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
	return mysock;
}

#endif

int transport_close(int sock)
{
#define SHUT_WR 1
	int rc;
#ifdef (ARCH_ARM_SPD)
	
	rc = sci_sock_shutdown(sock, SD_SEND);
	rc = sci_sock_recv(sock, NULL, 0, 0);
	rc = sci_sock_socketclose(sock);

#else
	rc = shutdown(sock, SHUT_WR);
	rc = recv(sock, NULL, (size_t)0, 0);
	rc = close(sock);
#endif
	return rc;
}
