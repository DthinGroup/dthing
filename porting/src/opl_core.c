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

/* refer to opl_core.h */
int64_t OPL_core_getCurrentTimeMillis()
{
    //fake implementaion
    static int64_t res = 100;
       
    return ++res;
}
