/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2013/06/25 $
 * Last modified:	$Date: 2013/06/28 $
 * Version:         $ID: heap.c#1
 */

/**
 * The file provided a layer of memory managment. It mainly 
 * provides APIs for interpret or other VM module to use.
 */

#include <std_global.h>
#include <heap.h>
#include <mm.h>


/**
 * Allocate a object in the memory pool. zeroed out the memory.
 *
 * @size the object size in bytes to allocated.
 * @flag the memory property will have.
 */
void* heapAllocObject(int32_t size, int32_t flag)
{
    void* ptr = NULL;
    //flag is unused at present.
    UNUSED(flag);

    ptr = dmalloc(size);
    if (ptr != NULL) CRTL_memset(ptr, 0x0, size);

    return ptr;
}



/**
 * Allocate the persistent memory data, for class or relevant data
 * structures, such as class loading, hash table, etc.
 *
 * @size the memory size in bytes to allocated.
 */
void* heapAllocPersistent(int32_t size)
{
    void* ptr = NULL;

    ptr = endmalloc(size);
    if (ptr != NULL) CRTL_memset(ptr, 0x0, size);

    return ptr;
}


/**
 * Duplicate a String in persistent memory area. 
 * And returns the String head address.
 * Note:
 *     This return memory space will not be freed until end of VM
 *     lifecycle, in this case, only persistent data(string) is
 *     recommended to duplicate by this API.
 */
void* heapDupStr(const int8_t* str)
{
    int32_t len = (int32_t)CRTL_strlen(str);
    int8_t* dupStr = endmalloc(len+1);

    if (dupStr != NULL)
    {
        CRTL_memset(dupStr, 0x0, len+1);
        CRTL_memcpy(dupStr, str, len);
    }

    return (void *)dupStr;    
}

/**
 * Mark the specified object as GC object.
 * @ptr the object pointer.
 */
void heapMarkObject(void* ptr)
{
    dmark(ptr);
}


/**
 * Check the specified object whether is marked.
 * @return, If marked return TRUE, otherwise return FALSE;
 */
bool_t heapIsMarked(void* ptr)
{
    return dismarked(ptr);
}

bool_t heapIsValid(void* ptr)
{
    return dvalidMem(ptr);
}

/**
 * Sweep the marked object and return the memory to MM.
 */
void heapSweep()
{
    dsweep();
}




