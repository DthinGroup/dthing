/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2013/06/25 $
 * Last modified:	$Date: 2013/06/28 $
 * Version:         $ID: heap.h#1
 */

/**
 * The file provided a layer of memory managment. It mainly 
 * provides APIs for interpret or other VM module to use.
 */

#ifndef __DTHING_HEAP_H__
#define __DTHING_HEAP_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Allocate a object in the memory pool. 
 *
 * @size the object size in bytes to allocated.
 * @flag the memory property will have.
 */
void* heapAllocObject(int32_t size, int32_t flag);


/**
 * Allocate the persistent memory data, for class or relevant data
 * structures, such as class loading, hash table, etc.
 *
 * @size the memory size in bytes to allocated.
 */
void* heapAllocPersistent(int32_t size);


/**
 * Duplicate a String in persistent memory area. 
 * And returns the String head address.
 * Note:
 *     This return memory space will not be freed until end of VM
 *     lifecycle, in this case, only persistent data(string) is
 *     recommended to duplicate by this API.
 */
void* heapDupStr(const int8_t* str);


/**
 * Mark the specified object as GC object.
 * @ptr the object pointer.
 */
void heapMarkObject(void* ptr);

/**
 * Check the specified object whether is marked.
 * @return, If marked return TRUE, otherwise return FALSE;
 */
bool_t heapIsMarked(void* ptr);

bool_t heapIsValid(void* ptr);

/**
 * Sweep the marked object and return the memory to MM.
 */
void heapSweep();

void heapStatus(int32_t *freeSize, int32_t usedSize);

#ifdef __cplusplus
}
#endif

#endif //__DTHING_HEAP_H__

