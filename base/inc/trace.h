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

#include <vm_common.h>

#ifdef ARCH_X86
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

 /**
 * TRACE_LEV value definitions:
 * 1 - error
 * 2 - warning
 * 3 - info
 * 4 - debug
 */
#ifndef TRACE_LEV 
#define TRACE_LEV 1
#endif
#define LEVEL_0 0
#define LEVEL_1 1
#define LEVEL_2 2
#define LEVEL_3 3
#define LEVEL_4 4

int SetDthingTraceLevel(unsigned int level);
void DVMTraceDbg(const char * fmt,...);
void DVMTraceInf(const char * fmt,...);
void DVMTraceWar(const char * fmt,...);
void DVMTraceErr(const char * fmt,...);

#ifdef __cplusplus
}
#endif

#endif //#ifndef __DVM_TRACE_H__