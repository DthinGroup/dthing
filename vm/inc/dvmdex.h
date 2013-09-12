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
 * The VM wraps some additional data structures around the DexFile.  These
 * are defined here.
 */
#ifndef DALVIK_DVMDEX_H_
#define DALVIK_DVMDEX_H_

#include <common.h>
#include <dexfile.h>

#ifdef __cplusplus
extern "C" {
#endif

/* extern */
typedef struct ClassObject_s    ClassObject;
typedef struct HashTable_s      HashTable;
typedef struct InstField_s      InstField;
typedef struct Method_s         Method;
typedef struct StringObject_s   StringObject;
typedef struct DvmDex_s         DvmDex;
typedef struct InterfaceEntry_s InterfaceEntry;
typedef struct Object_s         Object;
typedef struct DataObject_s     DataObject;
typedef struct ArrayObject_s    ArrayObject;
typedef struct Field_s          Field;
typedef struct StaticField_s    StaticField;

/*
 * Some additional VM data structures that are associated with the DEX file.
 */
struct DvmDex_s {
    /* pointer to the DexFile we're associated with */
    DexFile*              pDexFile;

    /* clone of pDexFile->pHeader (it's used frequently enough) */
    const DexHeader*      pHeader;

    /* interned strings; parallel to "stringIds" */
    StringObject** pResStrings;

    /* resolved classes; parallel to "typeIds" */
    ClassObject**  pResClasses;

    /* resolved methods; parallel to "methodIds" */
    Method**       pResMethods;

    /* resolved instance fields; parallel to "fieldIds" */
    /* (this holds both InstField and StaticField) */
    Field**        pResFields;

};


/*
 * Given a file descriptor for an open "optimized" DEX file, map it into
 * memory and parse the contents.
 *
 * On success, returns 0 and sets "*ppDvmDex" to a newly-allocated DvmDex.
 * On failure, returns a meaningful error code [currently just -1].
 */
int dvmDexFileOpenFromFd(int fd, DvmDex** ppDvmDex);

/*
 * Open a partial DEX file.  Only useful as part of the optimization process.
 */
int dvmDexFileOpenFromRawData(const u1* rawData, u4 length, DvmDex** ppDvmDex);

/*
 * Free a DvmDex structure, along with any associated structures.
 */
void dvmDexFileFree(DvmDex* pDvmDex);



/*
 * Return the requested item if it has been resolved, or NULL if it hasn't.
 */
StringObject* dvmDexGetResolvedString(const DvmDex* pDvmDex, u4 stringIdx);

ClassObject* dvmDexGetResolvedClass(const DvmDex* pDvmDex, u4 classIdx);

Method* dvmDexGetResolvedMethod(const DvmDex* pDvmDex, u4 methodIdx);

Field* dvmDexGetResolvedField(const DvmDex* pDvmDex, u4 fieldIdx);

/*
 * Update the resolved item table.  Resolution always produces the same
 * result, so we're not worried about atomicity here.
 */
void dvmDexSetResolvedString(DvmDex* pDvmDex, u4 stringIdx, StringObject* str);


void dvmDexSetResolvedClass(DvmDex* pDvmDex, u4 classIdx, ClassObject* clazz);

void dvmDexSetResolvedMethod(DvmDex* pDvmDex, u4 methodIdx, Method* method);


void dvmDexSetResolvedField(DvmDex* pDvmDex, u4 fieldIdx, Field* field);


#ifdef __cplusplus
}
#endif

#endif  // DALVIK_DVMDEX_H_
