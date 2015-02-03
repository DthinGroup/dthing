/**
 * Copyright (C) 2015 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2015/02/03 $
 * Last modified:	$Date: 2015/02/03 $
 * Version:         $ID: opl_trace.h#1
 */

/**
 * The implementation of core VM portings.
 */

 #ifndef __OPL_TRACE_H__
 #define __OPL_TRACE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
* interface call from SDK
*/
void Dthing_log(const char *x_format, ...);

 #ifdef __cplusplus
}
#endif

#endif //__OPL_CORE_H__