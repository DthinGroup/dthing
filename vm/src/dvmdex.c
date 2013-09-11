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
 * VM-specific state associated with a DEX file.
 */
#include <dthing.h>
#include <heap.h>
#include <opl_file.h>

/*
 * Create auxillary data structures.
 *
 * We need a 4-byte pointer for every reference to a class, method, field,
 * or string constant.  Summed up over all loaded DEX files (including the
 * whoppers in the boostrap class path), this adds up to be quite a bit
 * of native memory.
 *
 * For more traditional VMs these values could be stuffed into the loaded
 * class file constant pool area, but we don't have that luxury since our
 * classes are memory-mapped read-only.
 *
 * The DEX optimizer will remove the need for some of these (e.g. we won't
 * use the entry for virtual methods that are only called through
 * invoke-virtual-quick), creating the possibility of some space reduction
 * at dexopt time.
 */
static DvmDex* allocateAuxStructures(DexFile* pDexFile)
{
    DvmDex* pDvmDex;
    const DexHeader* pHeader;
    u4 stringCount, classCount, methodCount, fieldCount;
    bool_t success = TRUE;

    pDvmDex = (DvmDex*) heapAllocPersistent(sizeof(DvmDex));
    if (pDvmDex == NULL)
        return NULL;
    CRTL_memset(pDvmDex, 0x0, sizeof(DvmDex));

    pDvmDex->pDexFile = pDexFile;
    pDvmDex->pHeader = pDexFile->pHeader;

    pHeader = pDvmDex->pHeader;

    stringCount = pHeader->stringIdsSize;
    classCount = pHeader->typeIdsSize;
    methodCount = pHeader->methodIdsSize;
    fieldCount = pHeader->fieldIdsSize;

    if (stringCount > 0)
    {
        pDvmDex->pResStrings = (StringObject**)
            heapAllocPersistent(stringCount * sizeof(StringObject*));

        if (pDvmDex->pResStrings == NULL)
        {
            success = FALSE;
            goto bail;
        }
        CRTL_memset(pDvmDex->pResStrings, 0x0, stringCount * sizeof(StringObject*));
    }

    if (classCount > 0)
    {
        pDvmDex->pResClasses = (ClassObject**)
            heapAllocPersistent(classCount * sizeof(ClassObject*));

        if (pDvmDex->pResClasses == NULL)
        {
            success = FALSE;
            goto bail;
        }
        CRTL_memset(pDvmDex->pResClasses, 0x0, classCount * sizeof(ClassObject*));
    }

    if (methodCount > 0)
    {
        pDvmDex->pResMethods = (Method**)
            heapAllocPersistent(methodCount * sizeof(Method*));

        if (pDvmDex->pResMethods == NULL)
        {
            success = FALSE;
            goto bail;
        }
        CRTL_memset(pDvmDex->pResMethods, 0x0, methodCount * sizeof(Method*));
    }

    if (fieldCount > 0)
    {
        pDvmDex->pResFields = (Field**)
            heapAllocPersistent(fieldCount * sizeof(Field*));

        if (pDvmDex->pResFields == NULL)
        {
            success = FALSE;
            goto bail;
        }
        CRTL_memset(pDvmDex->pResFields, 0x0, fieldCount * sizeof(Field*));
    }

bail:

    if (!success)
    {
        DVMTraceErr("allocateAuxStructures - not enough Memory\n");
        pDvmDex = NULL;
    }
    
    return pDvmDex;

}


int dvmDexFileOpenFromRawData(const u1* rawData, u4 length, DvmDex** ppDvmDex)
{
    DvmDex* pDvmDex;
    DexFile* pDexFile;
    int result = -1;
    DexClassLookup* pLookup;

    pDexFile = dexFileParse(rawData, (size_t)length);

    if (pDexFile == NULL)
    {
        DVMTraceErr("dvmDexFileOpenFromFd - Error: DEX parse failed\n");
        goto bail;
    }

    pDvmDex = allocateAuxStructures(pDexFile);

    if (pDvmDex == NULL)
    {
        DVMTraceErr("dvmDexFileOpenFromFd - Error: allocateAuxStructures failed\n");
        goto bail;
    }

    /* create dex classes lookup table */
    pLookup = dexCreateClassLookup(pDvmDex->pDexFile);
    pDvmDex->pDexFile->pClassLookup = pLookup;

    /* tuck this into the DexFile so it gets released later */
    *ppDvmDex = pDvmDex;
    result = 0;

bail:
    return result;
}

/*
 * Given an open optimized DEX file, map it into read-only shared memory and
 * parse the contents.
 *
 * Returns nonzero on error.
 */
int dvmDexFileOpenFromFd(int fd, DvmDex** ppDvmDex)
{
    DvmDex*  pDvmDex;
    DexFile* pDexFile;
    int32_t  result = -1;
    u1*      fData = NULL;
    int32_t  fileLength = 0;
    int32_t  readSize = 0;

    /* read full file content into persistent area */
    fileLength = file_getLengthByFd(fd);
    fData = heapAllocPersistent(fileLength);
    readSize = file_read(fd, fData, fileLength);
    if (readSize < 0 || readSize != fileLength)
    {
        /**
         * TODO: should read file content in a loop as file_read may returns
         * part of file content?
         */
        goto bail;
    }

    pDexFile = dexFileParse(fData, fileLength);
    if (pDexFile == NULL)
    {
        DVMTraceErr("dvmDexFileOpenFromFd - Error: DEX parse failed\n");
        goto bail;
    }

    pDvmDex = allocateAuxStructures(pDexFile);
    if (pDvmDex == NULL)
    {
        DVMTraceErr("dvmDexFileOpenFromFd - Error: allocateAuxStructures failed\n");
        goto bail;
    }

    /* create dex classes lookup table */
    dexCreateClassLookup(pDvmDex->pDexFile);

    /* tuck this into the DexFile so it gets released later */
    *ppDvmDex = pDvmDex;
    result = 0;

bail:
    return result;
}


ClassObject* dvmDexGetResolvedClass(const DvmDex* pDvmDex, u4 classIdx)
{
    //assert(classIdx < pDvmDex->pHeader->typeIdsSize);
    return pDvmDex->pResClasses[classIdx];
}

void dvmDexSetResolvedClass(DvmDex* pDvmDex, u4 classIdx, ClassObject* clazz)
{
    //assert(classIdx < pDvmDex->pHeader->typeIdsSize);
    pDvmDex->pResClasses[classIdx] = clazz;
}

Method* dvmDexGetResolvedMethod(const DvmDex* pDvmDex, u4 methodIdx)
{
    //assert(methodIdx < pDvmDex->pHeader->methodIdsSize);
    return pDvmDex->pResMethods[methodIdx];
}


