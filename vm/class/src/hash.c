/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2013/06/25 $
 * Last modified:	$Date: 2013/06/26 $
 * Version:         $ID: hash.c#1
 */

/**
 * The file provides class loading APIs.
 * Include class loading, verifying, linking. 
 */

#include <std_global.h>
#include <hash.h>
#include <class.h>

/*
 * Round up to the next highest power of 2.
 *
 * Found on http://graphics.stanford.edu/~seander/bithacks.html.
 */
LOCAL uint32_t dexRoundUpPower2(uint32_t val)
{
    val--;
    val |= val >> 1;
    val |= val >> 2;
    val |= val >> 4;
    val |= val >> 8;
    val |= val >> 16;
    val++;

    return val;
}


/*
 * Create and initialize a hash table.
 */
HashTable* dvmHashTableCreate(uint32_t initialSize, HashFreeFunc freeFunc)
{
    HashTable* pHashTable;


    pHashTable = (HashTable*) CRTL_malloc(sizeof(*pHashTable));
    if (pHashTable == NULL)
	{
        return NULL;
	}


    pHashTable->tableSize = dexRoundUpPower2(initialSize);
    pHashTable->numEntries = pHashTable->numDeadEntries = 0;
    pHashTable->freeFunc = freeFunc;
    pHashTable->pEntries = (HashEntry*) CRLT_malloc(pHashTable->tableSize * sizeof(HashEntry));
    if (pHashTable->pEntries == NULL) 
	{
        CRTL_free(pHashTable);
        return NULL;
    }

    CRTL_memset(pHashTable->pEntries, 0, pHashTable->tableSize * sizeof(HashEntry));
    return pHashTable;
}

/*
 * Clear out all entries.
 */
void dvmHashTableClear(HashTable* pHashTable)
{
    HashEntry* pEnt;
    int i;

    pEnt = pHashTable->pEntries;
    for (i = 0; i < pHashTable->tableSize; i++, pEnt++)
	{
        if (pEnt->data == HASH_TOMBSTONE)
		{
            // nuke entry
            pEnt->data = NULL;
        } 
		else if (pEnt->data != NULL)
		{
            // call free func then nuke entry
            if (pHashTable->freeFunc != NULL)
                (*pHashTable->freeFunc)(pEnt->data);
            pEnt->data = NULL;
        }
    }

    pHashTable->numEntries = 0;
    pHashTable->numDeadEntries = 0;
}

/*
 * Free the table.
 */
void dvmHashTableFree(HashTable* pHashTable)
{
    if (pHashTable == NULL)
	{
        return;
	}
    dvmHashTableClear(pHashTable);
    CRTL_free(pHashTable->pEntries);
    CRTL_free(pHashTable);
}