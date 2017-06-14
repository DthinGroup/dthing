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

#if !defined(__MQTT_LINUX_)
#define __MQTT_LINUX_

#if defined(WIN32_DLL) || defined(WIN64_DLL)
  #define DLLImport __declspec(dllimport)
  #define DLLExport __declspec(dllexport)
#elif defined(LINUX_SO)
  #define DLLImport extern
  #define DLLExport  __attribute__ ((visibility ("default")))
#else
  #define DLLImport
  #define DLLExport
#endif

//1 s = 1000ms
typedef struct _timeval {
    long	tv_sec;     /* seconds */
	long 	tv_msec;    /* milliseconds */
} s_timeval;

typedef struct Timer
{
	s_timeval end_time;
} Timer;


#ifndef timeradd
	#define timeradd(a, b, r) 							\
		do {											\
		(r)->tv_sec	= (a)->tv_sec + (b)->tv_sec;		\
		(r)->tv_msec	= (a)->tv_msec + (b)->tv_msec;	\
		if ((r)->tv_msec >= 1000) {					\
			++(r)->tv_sec;								\
			(r)->tv_msec -= 1000;						\
		}												\
	} while (0)
#endif

#ifndef timersub   
	#define timersub(a, b, res)                           	\
		do {                                              	\
			(res)->tv_sec = (a)->tv_sec - (b)->tv_sec;    	\
			(res)->tv_msec = (a)->tv_msec - (b)->tv_msec; 	\
			if ((res)->tv_msec < 0) {                     	\
				(res)->tv_msec += 1000;						\
				(res)->tv_sec -= 1;                       	\
			}                                             	\
		} while(0)
#endif

void TimerInit(Timer*);
char TimerIsExpired(Timer*);
void TimerCountdownMS(Timer*, unsigned int);
void TimerCountdown(Timer*, unsigned int);
int TimerLeftMS(Timer*);

typedef struct Network
{
	int my_socket;
	int (*mqttread) (struct Network*, unsigned char*, int, int);
	int (*mqttwrite) (struct Network*, unsigned char*, int, int);
} Network;

int network_read(Network*, unsigned char*, int, int);
int network_write(Network*, unsigned char*, int, int);

void workaround_alive_task_check(void);


DLLExport void NetworkInit(Network*);
DLLExport int NetworkConnect(Network*, char*, int);
DLLExport void NetworkDisconnect(Network*);
DLLExport void workaround_alive_task_check_create(void);
DLLExport void workaround_alive_task_check_destroy(void);


typedef enum{
	LINK_TO_MCU = 0x00,
	LINK_TO_WIFI = 0x01,
	LINK_TO_ALL = 0x02,
	UNLINK_TO_ALL = 0x03,	
}COMM_LINK_MODE_E;

///GPIO 
void GpioCrtlInit(void);

void GpioSwitchMode(COMM_LINK_MODE_E mode);


#endif
