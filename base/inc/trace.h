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


/**
 * TRACE_LEV value definitions:
 * 1 - error
 * 2 - warning
 * 2 - debug
 * 4 - info
 */
#ifndef TRACE_LEV 
#define TRACE_LEV 1
#endif


#define DVMTraceErr (TRACE_LEV < 1) ? (void) 0 : (void)printf
#define DVMTraceWar (TRACE_LEV < 2) ? (void) 0 : (void)printf
#define DVMTraceDbg (TRACE_LEV < 3) ? (void) 0 : (void)printf
#define DVMTraceInf (TRACE_LEV < 4) ? (void) 0 : (void)printf

#endif //#ifndef __DVM_TRACE_H__