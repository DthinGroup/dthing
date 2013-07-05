/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2013/06/25 $
 * Last modified:	$Date: 2013/06/25 $
 * Version:         $ID: hash.c#1
 */

#ifndef __HASH_H__
#define __HASH_H__

/*
 * This function will be used to free entries in the table.  This can be
 * NULL if no free is required, free(), or a custom function.
 */
typedef void (*HashFreeFunc)(void* ptr);


/*
 * One entry in the hash table.  "data" values are expected to be (or have
 * the same characteristics as) valid pointers.  In particular, a NULL
 * value for "data" indicates an empty slot, and HASH_TOMBSTONE indicates
 * a no-longer-used slot that must be stepped over during probing.
 *
 * Attempting to add a NULL or tombstone value is an error.
 *
 * When an entry is released, we will call (HashFreeFunc)(entry->data).
 */
typedef struct HashEntry {
    uint32_t hashValue;
    void   * data;
} HashEntry;

#define HASH_TOMBSTONE ((void*) 0xcbcacccd)     // invalid ptr value

/*
 * Expandable hash table.
 *
 * This structure should be considered opaque.
 */
typedef struct HashTable {
    int32_t      tableSize;          /* must be power of 2 */
    int32_t      numEntries;         /* current #of "live" entries */
    int32_t      numDeadEntries;     /* current #of tombstone entries */
    HashEntry*   pEntries;           /* array on heap */
    HashFreeFunc freeFunc;
} HashTable;


/*
 * Create and initialize a HashTable structure, using "initialSize" as
 * a basis for the initial capacity of the table.  (The actual initial
 * table size may be adjusted upward.)  If you know exactly how many
 * elements the table will hold, pass the result from dvmHashSize() in.)
 *
 * Returns "false" if unable to allocate the table.
 */
HashTable* dvmHashTableCreate(uint32_t initialSize, HashFreeFunc freeFunc);

/*
 * Compute the capacity needed for a table to hold "size" elements.  Use
 * this when you know ahead of time how many elements the table will hold.
 * Pass this value into dvmHashTableCreate() to ensure that you can add
 * all elements without needing to reallocate the table.
 */
uint32_t dvmHashSize(uint32_t size);

/*
 * Clear out a hash table, freeing the contents of any used entries.
 */
void dvmHashTableClear(HashTable* pHashTable);

/*
 * Free a hash table.  Performs a "clear" first.
 */
void dvmHashTableFree(HashTable* pHashTable);



#endif //__HASH_H__