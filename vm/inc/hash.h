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
 * General purpose hash table, used for finding classes, methods, etc.
 *
 * When the number of elements reaches a certain percentage of the table's
 * capacity, the table will be resized.
 */
#ifndef DALVIK_HASH_H_
#define DALVIK_HASH_H_

#include <common.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HASH_TABLE_SIZE  (64)

typedef struct HashEntry_s HashEntry;
typedef struct HashTable_s HashTable;

/*
 * Compare a hash entry with a "loose" item after their hash values match.
 * Returns { <0, 0, >0 } depending on ordering of items (same semantics
 * as strcmp()).
 */
typedef int (*HashCompareFunc)(const void* tableItem, const void* looseItem);

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
struct HashEntry_s {
    u4         hashValue;     // hash value.
    void*      data;          // hash data.
    HashEntry* next;          // linked list hash entry.
};

/*
 * Expandable hash table.
 *
 * This structure should be considered opaque.
 */
struct HashTable_s {
    HashEntry* pEntries;
    u4         numEntries;     // current #of "live" entries
};

/*
 * Create and initialize a HashTable structure, using "initialSize" as
 * a basis for the initial capacity of the table.  (The actual initial
 * table size may be adjusted upward.)  If you know exactly how many
 * elements the table will hold, pass the result from dvmHashSize() in.)
 *
 * Returns "false" if unable to allocate the table.
 */
HashTable* dvmHashTableCreate();

/*
 * Clear out a hash table, freeing the contents of any used entries.
 */
void dvmHashTableClear(HashTable* pHashTable);

/*
 * Free a hash table.  Performs a "clear" first.
 */
void dvmHashTableFree(HashTable* pHashTable);

/*
 * Look up an entry in the table, possibly adding it if it's not there.
 *
 * If "item" is not found, and "doAdd" is false, NULL is returned.
 * Otherwise, a pointer to the found or added item is returned.  (You can
 * tell the difference by seeing if return value == item.)
 *
 * An "add" operation may cause the entire table to be reallocated.  Don't
 * forget to lock the table before calling this.
 */
void* dvmHashTableLookup(HashTable* pHashTable, u4 itemHash, void* item, HashCompareFunc cmpFunc, bool_t doAdd);

#ifdef __cplusplus
}
#endif

#endif  // DALVIK_HASH_H_
