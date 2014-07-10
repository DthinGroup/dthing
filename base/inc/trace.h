/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 * 
 * Last modified:	$Date: 2013/06/26 $
 * Version:         $ID: trace.h#1
 */

/**
 * define VM runtime trace tool. Usually it's mapped into 
 * platform trace API.
 */

#ifndef __DVM_TRACE_H__
#define __DVM_TRACE_H__

#ifdef WIN32
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * TRACE_LEV value definitions:
 * 1 - debug
 * 2 - info
 * 3 - warning
 * 4 - error
 */
#ifndef TRACE_LEV 
#define TRACE_LEV 1
#endif

#define DVMTraceErr (TRACE_LEV < 1) ? (void) 0 : (void)DthingTraceE
#define DVMTraceWar (TRACE_LEV < 2) ? (void) 0 : (void)DthingTraceW
#define DVMTraceDbg (TRACE_LEV < 3) ? (void) 0 : (void)DthingTraceD
#define DVMTraceInf (TRACE_LEV < 4) ? (void) 0 : (void)DthingTraceI

void DthingTraceD(const char * fmt,...);
void DthingTraceI(const char * fmt,...);
void DthingTraceW(const char * fmt,...);
void DthingTraceE(const char * fmt,...);

#ifdef __cplusplus
}
#endif

#endif //#ifndef __DVM_TRACE_H__