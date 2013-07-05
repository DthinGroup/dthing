/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2013/07/01 $
 * Last modified:	$Date: 2013/07/04 $
 * Version:         $ID: class.h#1
 */

/**
 * The file provides class loading APIs.
 * Include class loading, verifying, linking. 
 */


#ifndef __CLASS_H__
#define __CLASS_H__

#include <std_global.h>

/* current state of the class, increasing as we progress */
typedef enum ClassStatus {
    CLASS_ERROR         = -1,

    CLASS_NOTREADY      = 0,
    CLASS_LOADED        = 1,
    CLASS_PREPARED      = 2,    /* part of linking */
    CLASS_RESOLVED      = 3,    /* part of linking */
    CLASS_VERIFYING     = 4,    /* in the process of being verified */
    CLASS_VERIFIED      = 5,    /* logically part of linking; done pre-init */
    CLASS_INITIALIZING  = 6,    /* class init in progress */
    CLASS_INITIALIZED   = 7,    /* ready to go */
} ClassStatus;


/* class structure */
typedef struct ClassObject_s {

	int32_t a;


} ClassObject;




#endif //__CLASS_H__