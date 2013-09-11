/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 * Hash table.  The dominant calls are add and lookup, with removals
 * happening very infrequently.  We use probing, and don't worry much
 * about tombstone removal.
 */
#include <dthing.h>
#include <heap.h>
#include <hash.h>

/*
 * Create and initialize a hash table.
 */
HashTable* dvmHashTableCreate()
{
    HashTable* pHashTable;

    pHashTable = (HashTable*) heapAllocPersistent(sizeof(HashTable) * HASH_TABLE_SIZE);
    if (pHashTable == NULL) 
    {
        DVMTraceErr("dvmHashTableCreate - Error. No enough memory for Hash Table!\n");
        return NULL;
    }

    CRTL_memset(pHashTable, 0x0, HASH_TABLE_SIZE * sizeof(HashTable));
    return pHashTable;
}

/*
 * Look up an entry.
 *
 * We probe on collisions, wrapping around the table.
 */
void* dvmHashTableLookup(HashTable* pHashTable, u4 itemHash, void* item, HashCompareFunc cmpFunc, bool_t doAdd)
{
    HashTable* phTable;
    HashEntry* pEntry;
    bool_t     found = FALSE;
    void*      result = NULL;

    phTable = &pHashTable[itemHash & (HASH_TABLE_SIZE - 1)];
    pEntry = phTable->pEntries;

    for (; phTable->numEntries > 0, pEntry != NULL; pEntry = pEntry->next)
    {
        if (pEntry->hashValue == itemHash && (*cmpFunc)(pEntry->data, item) == 0)
        {
            /* match */
            found = TRUE;
            break;
        }
    }

    if (found)
    {
        result = (void *)pEntry->data;
    }
    else if (doAdd)
    {
        pEntry = (HashEntry *)heapAllocPersistent(sizeof(HashEntry));
        if (pEntry == NULL) 
        {
            DVMTraceErr("dvmHashTableLookup - Error. No enough memory for hash entry!\n");
            //TODO: should throw OOME in this case?
            return NULL;
        }

        pEntry->hashValue = itemHash;
        pEntry->data = item;
        pEntry->next = phTable->pEntries;
        phTable->pEntries = pEntry;
        phTable->numEntries++;
        result = item;
    }
    else
    {
        DVMTraceErr("dvmHashTableLookup - Error. Not find the item!\n");
    }

    return result;
}
