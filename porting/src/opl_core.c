/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2013/09/22 $
 * Last modified:	$Date: 2013/09/22 $
 * Version:         $ID: opl_core.c#1
 */

/**
 * The implementation of core VM portings.
 */
 
#include <std_global.h>

#ifdef ARCH_ARM_SPD
#include "dal_time.h"
#include "os_api.h"

/* Declare the milliseconds from 1970.1.1 to 1980.1.1*/
#define LOST_TIME_SINCE1970 (315532800000L)
#define NOT_INITIALIZED_TIME 0XFEED
static int64_t timeBase = NOT_INITIALIZED_TIME;
#endif

/* refer to opl_core.h */
int64_t OPL_core_getCurrentTimeMillis()
{
#ifdef ARCH_X86
    //fake implementaion
    static int64_t res = 100;
    return ++res;
#elif defined(ARCH_ARM_SPD)
    int64_t res = 0;
    if (timeBase == NOT_INITIALIZED_TIME) {
        timeBase = ((int64_t)TM_GetTotalSeconds())*1000 - SCI_GetTickCount() + LOST_TIME_SINCE1970;
    }
    res = timeBase + SCI_GetTickCount();
    return res;
#endif
}

#define LOG_BUFFER_SIZE   (128)
#define PRINT_PREFIX  "Java:"
static char buf[LOG_BUFFER_SIZE];
/* refer to opl_core.h */
void OPL_core_logChar(int chr)
{
    static int pos = 0;
    if (pos == 0)
    {
        CRTL_memset(buf, 0x0, LOG_BUFFER_SIZE);
        pos = (int)CRTL_strlen(PRINT_PREFIX);
        CRTL_memcpy(buf, PRINT_PREFIX,pos );
    }

    if (pos < LOG_BUFFER_SIZE)
    {
        buf[pos++] = (char)chr;
    }

    if (chr == '\n' || pos == LOG_BUFFER_SIZE-1)
    {
        buf[pos] = '\0';
        DVMTraceJava("%s", buf);
        pos = 0;
    }
}