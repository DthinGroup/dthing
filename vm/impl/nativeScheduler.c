/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 *
 * Created:         $Date: 2014/04/16 $
 * Last modified:	$Date: 2014/04/16 $
 * Version:         $ID: nativeScheduler.c#1
 */

/**
 * The implementation of com.yarlungsoft.ams.Scheduler.
 */

#include <dthing.h>
#include <nativeScheduler.h>
#include <ams_remote.h>
#include <upcall.h>
#include <vm_common.h>
#include <ams.h>
#include <vm_app.h>

/* refer to com/yarlungsoft/ams/Scheduler.java */
#define APPLET_STATE_ERROR          -1
#define APPLET_STATE_UNINITIALIZED   0
#define APPLET_STATE_INITIALIZED     1
#define APPLET_STATE_STARTING        2
#define APPLET_STATE_STARTED         3
#define APPLET_STATE_PAUSE_PENDING   4
#define APPLET_STATE_PAUSING         5
#define APPLET_STATE_PAUSED          6
#define APPLET_STATE_RESUME_PENDING  7
#define APPLET_STATE_RESUMING        8
#define APPLET_STATE_DESTROY_PENDING 9
#define APPLET_STATE_DESTROYING      10
#define APPLET_STATE_INVALID         11

/* see nativeScheduler.h */
void Java_com_yarlungsoft_ams_Scheduler_reportState(const u4* args, JValue* pResult)
{
	int istrue  =1;
    int state = (int) args[1];
#ifndef NOT_LAUNCH_NET_TASK
    uint8_t	idBuf[4] = {0x0,};
    uint8_t	*pByte;
    SafeBuffer	*safeBuf;
    DVMTraceInf("Scheduler_reportState state(%d)\n", state);
    if (state == APPLET_STATE_STARTED)
    {
    		pByte = (uint8_t*)idBuf;
		writebeIU32(&pByte[0], istrue);
		safeBuf = CreateSafeBufferByBin(idBuf, sizeof(idBuf));
		Ams_handleAck(Ams_getCurCrtlModule(),AMS_FASM_STATE_ACK_RUN,(void*)safeBuf);
        //ramsClient_sendBackExecResult(EVT_CMD_RUN, TRUE);
    }
    else if (state == APPLET_STATE_UNINITIALIZED)
    {
        /* UNINITIALIZED is not reported automatically except from destroying state*/
        // ramsClient_sendBackExecResult(EVT_CMD_DESTROY, TRUE);
              pByte = (uint8_t*)idBuf;
		writebeIU32(&pByte[0], istrue);
		safeBuf = CreateSafeBufferByBin(idBuf, sizeof(idBuf));
		Ams_handleAck(Ams_getCurCrtlModule(),AMS_FASM_STATE_ACK_DESTROY,(void*)safeBuf);
    }
#endif
}

/* see nativeScheduler.h */
void Java_com_yarlungsoft_ams_Scheduler_reportRunningObject(const u4* args, JValue* pResult)
{
    Object* obj = (Object*)args[1];

    DVMTraceInf("Scheduler_reportRunningObject\n");

    saveCurrentActiveApplet(obj);
}

