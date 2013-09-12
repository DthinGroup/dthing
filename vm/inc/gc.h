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