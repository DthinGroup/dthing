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
#include <rams.h>
#include <upcall.h>

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
    int state = (int) args[1];

    DVMTraceInf("Scheduler_reportState state(%d)\n", state);
    if (state == APPLET_STATE_STARTED)
    {
        sendBackExecResult(CMD_RUN, TRUE);
    }
    else if (state == APPLET_STATE_UNINITIALIZED)
    {
        /* UNINITIALIZED is not reported automatically except from destroying state*/
        sendBackExecResult(CMD_DESTROY, TRUE);
    }
}

/* see nativeScheduler.h */
void Java_com_yarlungsoft_ams_Scheduler_reportRunningObject(const u4* args, JValue* pResult)
{
    Object* obj = (Object*)args[1];

    DVMTraceInf("Scheduler_reportRunningObject\n");

    saveCurrentActiveApplet(obj);
}

