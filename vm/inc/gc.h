/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2013/06/25 $
 * Last modified:	$Date: 2013/06/25 $
 * Version:         $ID: hash.c#1
 */

#ifndef __GC_H__
#define __GC_H__

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_TEMP_ROOTS_NUM (32)

#define MAX_GLOBAL_ROOTS_NUM (32)


typedef enum GC_STATUS_e {
    GC_NONE,
    GC_INPROGRESS,
    GC_DONE,
    GC_UNUSED
} GC_STATUS;


/*
 * Create an instance of the specified class.
 *
 * Returns NULL and throws an exception on failure.
 */
Object* dvmAllocObject(ClassObject* clazz, int flags);


#ifdef __cplusplus
}
#endif

#endif //__GC_H__