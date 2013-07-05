/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2013/06/10 $
 * Last modified:	$Date: 2013/06/25 $
 * Version:         $ID: std_global.h#1
 */

#ifndef __DVM_GLOBAL_H__
#define __DVM_GLOBAL_H__

#include <std_types.h>     /* Provide basic C99 types */
#include <crtl.h>
#include <trace.h>

#ifdef  FALSE
#undef  FALSE
#endif
#define FALSE (0)


#ifdef  TRUE
#undef  TRUE
#endif
#define TRUE (!FALSE)

#ifndef PUBLIC
#define PUBLIC
#endif

#ifndef LOCAL
#define LOCAL static
#endif

#endif  /* __DVM_GLOBAL_H__ */