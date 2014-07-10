/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 * 
 * Last modified:	$Date: 2013/06/24 $
 * Version:         $ID: mm.h#1
 */

/**
 * The file provided the basic memory management functions.
 */

#ifndef __MM_H__
#define __MM_H__

#include <std_global.h>

#ifdef __cplusplus
extern "C" {
#endif


struct malloc_chunk
{
    uint32_t              prev_foot;
	uint32_t              head;
	struct malloc_chunk*  fd;
	struct malloc_chunk*  bk;
};
typedef struct malloc_chunk   mchunk;
typedef struct malloc_chunk*  mchunkptr;
typedef struct malloc_chunk*  sbinptr;  /* The type of bins of chunks */
typedef uint32_t              binmap_t;

struct malloc_tree_chunk {
    /* The first four fields must be compatible with malloc_chunk */
    uint32_t                   prev_foot;
    uint32_t                   head;
    struct malloc_tree_chunk*  fd;
    struct malloc_tree_chunk*  bk;

    struct malloc_tree_chunk*  child[2];
    struct malloc_tree_chunk*  parent;
    uint32_t                   index;
};

typedef struct malloc_tree_chunk   tchunk;
typedef struct malloc_tree_chunk*  tchunkptr;
typedef struct malloc_tree_chunk*  tbinptr; /* The type of bins of trees */


/* Bin types, widths and sizes */
#define NSMALLBINS        (32U)
#define NTREEBINS         (32U)

struct memory_pool
{
	//for object area. such as new object;
	void*     mem_start;
	void*     mem_end; 
	
	//for permanent data area, such class loading.
	void*     perm_start; 
	void*     perm_end;

	uint32_t  free_size;

	//bins map to express which box has free memory.
	binmap_t  sbinsmap;
	binmap_t  tbinsmap;

	sbinptr   sbins[(NSMALLBINS+1)*2];
	tbinptr   tbins[NTREEBINS];

	//next memory pool
	struct    memory_pool* next;
};

typedef struct memory_pool mpglobal;


/**
 * Global memory pool structure.
 */
extern PUBLIC mpglobal* mg;

/**
 * Basic functions prototypes
 */
void * dmalloc(uint32_t size);
void * dcalloc(uint32_t num, uint32_t size);
void * drealloc(void* ptr, uint32_t size);
void   dfree(void* ptr);


/**
 * memory allocation from end of memory pool.
 * just privately api. used for permanent data.
 */
void * endmalloc(uint32_t size);


/**
 * GC relevant;
 */
/**
 * Mark the GC bit for current mem block.
 */
void dmark(void* mem);

/**
 * Check current memory block in mark bit. 
 * @return TRUE means marked, otherwise return FALSE;
 */
bool_t dismarked(void* mem);


/**
 * Scan all memory chunk and clear all marked memories.
 */
void dsweep();

/**
 * Check the address whether is valid object address.
 */
bool_t dvalidMem(void* mem);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif //__MM_H__


