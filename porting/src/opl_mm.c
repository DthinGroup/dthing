/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2013/06/20 $
 * Last modified:	$Date: 2013/06/25 $
 * Version:         $ID: opl_mm.c#1
 */

/**
 * The file provided the memory managment porting layer. 
 */

#include <opl_mm.h>

//VC environment
#include <stdlib.h>

#define MEMORY_SIZE (2 * 512 * 1024)

void Sys_mm_getMemoryPool(void ** base, int32_t * size)
{
	void * ptr = CRTL_malloc(MEMORY_SIZE);

	if (ptr == NULL)
	{
		*base = NULL;
		*size = 0;
	}
	
	*base = ptr;
	*size = MEMORY_SIZE;
}

void Sys_mm_freeMemoryPool(void * base)
{
	CRTL_free(base);
}

