/**
 * Copyright (C) 2015 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2015/02/03 $
 * Last modified:    $Date: 2015/02/03 $
 * Version:         $ID: opl_trace.c#1
 */


/**
 * This file implement the File porting on WIN32 platform.
 * API abstract is likely standard C file APIs.
 */
 #include <vm_common.h>
#include <std_global.h>
#include <opl_trace.h>

#if defined(ARCH_X86)
/* platform header files */
#include <windows.h>
#elif defined(ARCH_ARM_SPD)
#include <sfs.h>
#include <os_api.h>
#endif

void Dthing_log(const char *x_format, ...)
{
    va_list  args;
    va_start(args, x_format);
#if defined(MSM_WIS_DEBUG)
    wis_debug(x_format, args);
#else
    SCI_TRACE_LOW(x_format, args);
#endif
    va_end(args);
}