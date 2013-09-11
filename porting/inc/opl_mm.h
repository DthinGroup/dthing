/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2013/06/20 $
 * Last modified:	$Date: 2013/06/25 $
 * Version:         $ID: opl_mm.h#1
 */

/**
 * The file provided the memory managment porting layer. 
 */

#ifndef __OPL_MM_H__
#define __OPL_MM_H__

#include <std_global.h>

#if defined(__cplusplus)
extern "C" {
#endif


/**
 * Inported functions which will be called by VM internal.
 */
void Sys_mm_getMemoryPool(void ** base, int32_t * size);
void Sys_mm_freeMemoryPool(void * base);



/**
 * VM lifecycle conversation of memory managment.
 * Exported function which will used by vm lifecycle.
 */
bool_t DVM_mm_initialize();
void   DVM_mm_finalize();

#if defined(__cplusplus)
}
#endif


#endif //__OPL_MM_H__
