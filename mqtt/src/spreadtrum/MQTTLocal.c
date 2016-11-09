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

#include <vm_common.h>
#include <os_api.h>
#include <Socket_types.h>

//The defines are from socket_types.h in spd
#define ECONNRESET      8
#define ENOTCONN        9

#undef MQTT_TRACE
#define MQTT_TRACE
#if defined(MQTT_TRACE)
	#define MQTT_Trace DVMTraceJava
#else
	#define MQTT_Trace
#endif

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
	unsigned long tick = SCI_GetTickCount();  //ms
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
	sci_sock_setsockopt(n->my_socket, SO_RCVTIMEO, (void*)&interval);	
	//setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&interval, sizeof(struct _timeval));

	while (bytes < len)
	{
		int rc = sci_sock_recv(n->my_socket, &buffer[bytes], (int)(len - bytes), 0);
		if (rc == -1)
		{
			int errno = sci_sock_errno(n->my_socket);
			MQTT_Trace("=== network_read: error code of network_read is %d \n", errno);
			if (errno != ENOTCONN && errno != ECONNRESET)
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
	int rc = -1, err=0;
	tv.tv_sec = 0;  /* 30 Secs Timeout */
	tv.tv_msec = timeout_ms * 1000;  // Not init'ing this can cause strange errors
	
	sci_sock_setsockopt(n->my_socket, SO_RCVTIMEO, (void*)&tv);	
	rc = sci_sock_send(n->my_socket, buffer, len, 0);
	if(rc < 0){  //Trace info		
		err = sci_sock_errno(n->my_socket);
		MQTT_Trace("=== network_write: error code of network_write is %d \n", err);
	}
	return rc;
}


void NetworkInit(Network* n)
{
	MQTT_Trace("=== NetworkInit ===\n");
	n->my_socket = 0;
	n->mqttread = network_read;
	n->mqttwrite = network_write;
}


int NetworkConnect(Network* n, char* addr, int port)
{	
	int rc = -1;	
	struct sci_sockaddr dst_addr = {0};
	int temp_count = 0;
	
	assert(addr != NULL);

	MQTT_Trace("=== NetworkConnect ,rc=%d===\n", rc);
	do
	{
		rc = sci_parse_host(addr, &dst_addr.ip_addr, 0);
		temp_count ++;
		SCI_Sleep(100);
		MQTT_Trace("=== NetworkConnect sci_parse_host %d ===\n", rc);
	}while((rc == 1) && (temp_count < 50));
	
	if(0 != rc)
	{	
		MQTT_Trace("=== NetworkConnect sci_parse_host res = %d ===\n", rc);
		return -1;
	}

	dst_addr.family = AF_INET;
	dst_addr.port = htons(port);

	if (rc == 0)
	{
		n->my_socket =	sci_sock_socket(AF_INET, SOCK_STREAM, 0, 0);	
		MQTT_Trace("=== NetworkConnect socket = %d ===\n", n->my_socket);		
		if (n->my_socket != -1)
		{
			rc = sci_sock_connect(n->my_socket, (struct sci_sockaddr*)&dst_addr, sizeof(dst_addr));			
			MQTT_Trace("=== NetworkConnect sci_sock_connect = %d ===\n", rc);
		}
	}	

	MQTT_Trace("=== NetworkConnect rc =  %d ===\n", rc);
	return rc;
}


void NetworkDisconnect(Network* n)
{
	sci_sock_socketclose(n->my_socket);
}

static SCI_TIMER_PTR g_heartbeat_timer = SCI_NULL;


//WORKAROUND keywords: [dthing-workaround-nix-1] in workaround-readme.md
extern uint32_t mqtt_publish_timestamp_op(int op);
extern uint32_t comm_read_timestamp_op(int op);
static volatile int check_flag = 0;
void workaround_alive_task_check(unsigned int unused){
	//int ret = mqtt_keepalive();
	
	uint32_t timestamp1 = mqtt_publish_timestamp_op(1);
	uint32_t timestamp2 = comm_read_timestamp_op(1);

	uint32_t cur = SCI_GetTickCount();
	MQTT_Trace("=== workaround_alive_task_check: mqtt-timestamp - %d, comm-timestamp - %d, current-timestamp - %d \n",timestamp1,timestamp2,cur);	

	if(!check_flag)
		return;

	//3 minutes restart
	if((cur > timestamp1 + 180*1000) || (cur > timestamp2 + 180*1000)){
		MQTT_Trace("=== ready to re-start device!!!");
		SCI_Sleep(1000);
		(*(void (*)( ))0)();
	}
/*
	if(g_heartbeat_timer != NULL && SCI_TRUE != SCI_IsTimerActive(g_heartbeat_timer))
			SCI_ActiveTimer(g_heartbeat_timer);
*/
}

void NetworkHeartBeatCreate(void){
	mqtt_publish_timestamp_op(0);
	comm_read_timestamp_op(0);
	check_flag = 1;
#if 0	
	if(g_heartbeat_timer == SCI_NULL){
		g_heartbeat_timer = SCI_CreatePeriodTimer("DTHING_MQTT_KEEPALIVE_TIMER",    // Name string of the timer
								workaround_alive_task_check,	//callback
								0,		//params of callback
								5000,	//period time 
								SCI_AUTO_ACTIVATE);	//auto start
	} else {
		if(SCI_TRUE != SCI_IsTimerActive(g_heartbeat_timer))
			SCI_ActiveTimer(g_heartbeat_timer);
	}
#endif	
}

void NetworkHeartBeatDestroy(void){
		check_flag = 0;
#if 0
	if(g_heartbeat_timer != SCI_NULL){
		SCI_DeleteTimer(g_heartbeat_timer);
	}
	g_heartbeat_timer = SCI_NULL;
#endif
}