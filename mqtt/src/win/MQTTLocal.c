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
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *******************************************************************************/

#include "MQTTLocal.h"

//Win VC
#pragma comment( lib,"winmm.lib" )
#pragma comment (lib,"Ws2_32.lib")

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#define MAXHOSTNAMELEN 256
#define EAGAIN WSAEWOULDBLOCK
#define EINTR WSAEINTR
#define EINVAL WSAEINVAL
#define EINPROGRESS WSAEINPROGRESS
#define EWOULDBLOCK WSAEWOULDBLOCK
#define ENOTCONN WSAENOTCONN
#define ECONNRESET WSAECONNRESET	

void TimerInit(Timer* timer)
{
	timer->end_time.tv_sec = 0;
	timer->end_time.tv_msec = 0;
}
/*
 * Attention : because cannot get escaplsed time from 1971 in spd system, the value is the time from system powers on
 * Maybe bugs
 */
static void gettimeofday(s_timeval * time, void * unuse){	
	unsigned long tick = timeGetTime();  //ms
	unuse = unuse;
	time->tv_sec  = tick/1000;
	time->tv_msec = tick%1000;	
}

char TimerIsExpired(Timer* timer)
{
	s_timeval now, res;
	gettimeofday(&now, NULL);
	timersub(&timer->end_time, &now, &res);		
	return res.tv_sec < 0 || (res.tv_sec == 0 && res.tv_msec <= 0);
}


void TimerCountdownMS(Timer* timer, unsigned int timeout)
{
	s_timeval now, interval;
	gettimeofday(&now, NULL);
	interval.tv_sec =  timeout / 1000 ;
	interval.tv_msec = timeout % 1000 ;
	timeradd(&now, &interval, &timer->end_time);
}


void TimerCountdown(Timer* timer, unsigned int timeout)
{
	s_timeval now, interval;
	gettimeofday(&now, NULL);
	interval.tv_sec = timeout;
	interval.tv_msec = 0;
	timeradd(&now, &interval, &timer->end_time);
}

// return : millisecond
int TimerLeftMS(Timer* timer)
{
	int leftms = 0;
	s_timeval now, res;
	gettimeofday(&now, NULL);
	timersub(&timer->end_time, &now, &res);
	//printf("left %d ms\n", (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_msec / 1000);
	return (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_msec;
}


int network_read(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
	unsigned int timeout = 0;
	int bytes = 0;
	s_timeval interval;
	interval.tv_sec  = timeout_ms / 1000;
	interval.tv_msec = timeout_ms % 1000;
	if (interval.tv_sec < 0 || (interval.tv_sec == 0 && interval.tv_msec <= 0))
	{
		interval.tv_sec = 0;
		interval.tv_msec = 100;
	}
	
	timeout = interval.tv_sec * 1000 + interval.tv_msec;
	setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(int));

	while (bytes < len)
	{
		int rc = recv(n->my_socket, &buffer[bytes], (int)(len - bytes), 0);
		if (rc == -1)
		{
			int err = WSAGetLastError();
			if (err != ENOTCONN && err != ECONNRESET)
			{
				bytes = -1;
				break;
			}
		}
		else
			bytes += rc;
	}
	return bytes;
}

int network_write(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
	s_timeval tv;
	unsigned int timeout;
	int rc = -1;
	tv.tv_sec = 0;  /* 30 Secs Timeout */
	tv.tv_msec = timeout_ms * 1000;  // Not init'ing this can cause strange errors
	
	timeout = tv.tv_sec * 1000 + tv.tv_msec;
	setsockopt(n->my_socket,SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(int));	
	rc = send(n->my_socket, buffer, len, 0);
	return rc;
}


void NetworkInit(Network* n)
{
	n->my_socket = 0;
	n->mqttread = network_read;
	n->mqttwrite = network_write;
}


int NetworkConnect(Network* n, char* addr, int port)
{	
	int type = SOCK_STREAM;
	struct sockaddr_in address;
	int rc = -1;
	short family;
	struct addrinfo *result = NULL;
	struct addrinfo hints = {0, AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP, 0, NULL, NULL, NULL};	

	WORD wVersionRequested;
	WSADATA wsaData;//WSAata用来存储系统传回的关于WinSocket的资料�?    int err;
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
		n->my_socket =	(int)socket(family, type, 0);		
		if (n->my_socket != -1)
		{
			if (family == AF_INET)
				rc = connect(n->my_socket, (struct sockaddr*)&address, sizeof(address));
		}
	}
	
	return rc;	
}


void NetworkDisconnect(Network* n)
{
	closesocket(n->my_socket);
}
