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
 * Access the contents of a .dex file.
 */

#include <dthing.h>
#include <dexfile.h>
#include <leb128.h>
#include <opl_file.h>
#include <heap.h>



/* (documented in header) */
char dexGetPrimitiveTypeDescriptorChar(PrimitiveType type)
{
    const char* string = dexGetPrimitiveTypeDescriptor(type);

    return (string == NULL) ? '\0' : string[0];
}

/* (documented in header) */
const char* dexGetPrimitiveTypeDescriptor(PrimitiveType type)
{
    switch (type) {
        case PRIM_VOID:    return "V";
        case PRIM_BOOLEAN: return "Z";
        case PRIM_BYTE:    return "B";
        case PRIM_SHORT:   return "S";
        case PRIM_CHAR:    return "C";
        case PRIM_INT:     return "I";
        case PRIM_LONG:    return "J";
        case PRIM_FLOAT:   return "F";
        case PRIM_DOUBLE:  return "D";
        default:           return NULL;
    }

    return NULL;
}


/* (documented in header) */
const char* dexGetPrimitiveTypeArrayDescriptor(PrimitiveType type)
{
    switch (type) {
        case PRIM_VOID:    return "[V";
        case PRIM_BOOLEAN: return "[Z";
        case PRIM_BYTE:    return "[B";
        case PRIM_SHORT:   return "[S";
        case PRIM_CHAR:    return "[C";
        case PRIM_INT:     return "[I";
        case PRIM_LONG:    return "[J";
        case PRIM_FLOAT:   return "[F";
        case PRIM_DOUBLE:  return "[D";
        default:           return NULL;
    }

    return NULL;
}

/* (documented in header) */
const char* dexGetBoxedTypeDescriptor(PrimitiveType type)
{
    switch (type) {
        case PRIM_VOID:    return NULL;
        case PRIM_BOOLEAN: return "Ljava/lang/Boolean;";
        case PRIM_BYTE:    return "Ljava/lang/Byte;";
        case PRIM_SHORT:   return "Ljava/lang/Short;";
        case PRIM_CHAR:    return "Ljava/lang/Character;";
        case PRIM_INT:     return "Ljava/lang/Integer;";
        case PRIM_LONG:    return "Ljava/lang/Long;";
        case PRIM_FLOAT:   return "Ljava/lang/Float;";
        case PRIM_DOUBLE:  return "Ljava/lang/Double;";
        default:           return NULL;
    }
}

/* (documented in header) */
PrimitiveType dexGetPrimitiveTypeFromDescriptorChar(char descriptorChar)
{
    switch (descriptorChar) {
        case 'V': return PRIM_VOID;
        case 'Z': return PRIM_BOOLEAN;
        case 'B': return PRIM_BYTE;
        case 'S': return PRIM_SHORT;
        case 'C': return PRIM_CHAR;
        case 'I': return PRIM_INT;
        case 'J': return PRIM_LONG;
        case 'F': return PRIM_FLOAT;
        case 'D': return PRIM_DOUBLE;
        default:  return PRIM_NOT;
    }
}


const DexTypeList* dexGetInterfacesList(const DexFile* pDexFile, const DexClassDef* pClassDef)
{
    if (pClassDef->interfacesOff == 0)
        return NULL;
    return (const DexTypeList*)
        (pDexFile->baseAddr + pClassDef->interfacesOff);
}

/* return the Nth entry in a DexTypeList. */
const DexTypeItem* dexGetTypeItem(const DexTypeList* pList, u4 idx)
{
    //assert(idx < pList->size);
    return &pList->list[idx];
}


/* DexClassDef convenience - get class_data_item pointer */
const u1* dexGetClassData(const DexFile* pDexFile, const DexClassDef* pClassDef)
{
    if (pClassDef->classDataOff == 0) {
        return NULL;
    }
    return (const u1*) (pDexFile->baseAddr + pClassDef->classDataOff);
}

/* return the const char* string data referred to by the given string_id */
const char* dexGetStringData(const DexFile* pDexFile,
        const DexStringId* pStringId)
{
    const u1* ptr = pDexFile->baseAddr + pStringId->stringDataOff;

    // Skip the uleb128 length.
    while (*(ptr++) > 0x7f) /* empty */ ;

    return (const char*) ptr;
}

/* return the StringId with the specified index */
const DexStringId* dexGetStringId(const DexFile* pDexFile, u4 idx) {
    //assert(idx < pDexFile->pHeader->stringIdsSize);
    return &pDexFile->pStringIds[idx];
}

/* return the UTF-8 encoded string with the specified string_id index */
const char* dexStringById(const DexFile* pDexFile, u4 idx)
{
    const DexStringId* pStringId = dexGetStringId(pDexFile, idx);
    return dexGetStringData(pDexFile, pStringId);
}

/* return the TypeId with the specified index */
const DexTypeId* dexGetTypeId(const DexFile* pDexFile, u4 idx)
{
    //assert(idx < pDexFile->pHeader->typeIdsSize);
    return &pDexFile->pTypeIds[idx];
}

/* return the FieldId with the specified index */
const DexFieldId* dexGetFieldId(const DexFile* pDexFile, u4 idx)
{
    //assert(idx < pDexFile->pHeader->fieldIdsSize);
    return &pDexFile->pFieldIds[idx];
}

/* get the source file string */
const char* dexGetSourceFile(const DexFile* pDexFile, const DexClassDef* pClassDef)
{
    if (pClassDef->sourceFileIdx == 0xffffffff)
        return NULL;
    return dexStringById(pDexFile, pClassDef->sourceFileIdx);
}


/*
 * Get the descriptor string associated with a given type index.
 * The caller should not free() the returned string.
 */
const char* dexStringByTypeIdx(const DexFile* pDexFile, u4 idx)
{
    const DexTypeId* typeId = dexGetTypeId(pDexFile, idx);
    return dexStringById(pDexFile, typeId->descriptorIdx);
}


/* return the ClassDef with the specified index */
const DexClassDef* dexGetClassDef(const DexFile* pDexFile, u4 idx)
{
    //assert(idx < pDexFile->pHeader->classDefsSize);
    return &pDexFile->pClassDefs[idx];
}


/* DexClassDef convenience - get class descriptor */
const char* dexGetClassDescriptor(const DexFile* pDexFile, const DexClassDef* pClassDef)
{
    return dexStringByTypeIdx(pDexFile, pClassDef->classIdx);
}

/* given a ClassDef pointer, recover its index */
u4 dexGetIndexForClassDef(const DexFile* pDexFile, const DexClassDef* pClassDef)
{
    //assert(pClassDef >= pDexFile->pClassDefs &&
    //       pClassDef < pDexFile->pClassDefs + pDexFile->pHeader->classDefsSize);
    return pClassDef - pDexFile->pClassDefs;
}


/* return the MethodId with the specified index */
const DexMethodId* dexGetMethodId(const DexFile* pDexFile, u4 idx)
{
    //assert(idx < pDexFile->pHeader->methodIdsSize);
    return &pDexFile->pMethodIds[idx];
}

/* return the type_idx for the Nth entry in a TypeList */
u4 dexTypeListGetIdx(const DexTypeList* pList, u4 idx)
{
    const DexTypeItem* pItem = dexGetTypeItem(pList, idx);
    return pItem->typeIdx;
}

/* return the ProtoId with the specified index */
const DexProtoId* dexGetProtoId(const DexFile* pDexFile, u4 idx)
{
    //assert(idx < pDexFile->pHeader->protoIdsSize);
    return &pDexFile->pProtoIds[idx];
}

/*
 * Get the parameter list from a ProtoId. The returns NULL if the ProtoId
 * does not have a parameter list.
 */
const DexTypeList* dexGetProtoParameters(const DexFile *pDexFile, const DexProtoId* pProtoId)
{
    if (pProtoId->parametersOff == 0) {
        return NULL;
    }
    return (const DexTypeList*)
        (pDexFile->baseAddr + pProtoId->parametersOff);
}

/* get the static values list for a DexClass */
const DexEncodedArray* dexGetStaticValuesList(const DexFile* pDexFile, const DexClassDef* pClassDef)
{
    if (pClassDef->staticValuesOff == 0)
        return NULL;
    return (const DexEncodedArray*)
        (pDexFile->baseAddr + pClassDef->staticValuesOff);
}

/* get the annotations directory item for a DexClass */
const DexAnnotationsDirectoryItem* dexGetAnnotationsDirectoryItem(
    const DexFile* pDexFile, const DexClassDef* pClassDef)
{
    if (pClassDef->annotationsOff == 0)
        return NULL;
    return (const DexAnnotationsDirectoryItem*)
        (pDexFile->baseAddr + pClassDef->annotationsOff);
}


/* Return the UTF-8 encoded string with the specified string_id index,
 * also filling in the UTF-16 size (number of 16-bit code points).*/
const char* dexStringAndSizeById(const DexFile* pDexFile, u4 idx, u4* utf16Size)
{
    const DexStringId* pStringId = dexGetStringId(pDexFile, idx);
    const u1* ptr = pDexFile->baseAddr + pStringId->stringDataOff;

    *utf16Size = readUnsignedLeb128(&ptr);
    return (const char*) ptr;
}


/*
 * Format an SHA-1 digest for printing.  tmpBuf must be able to hold at
 * least kSHA1DigestOutputLen bytes.
 */
const char* dvmSHA1DigestToStr(const unsigned char digest[], char* tmpBuf);

/*
 * Compute a SHA-1 digest on a range of bytes.
 */
static void dexComputeSHA1Digest(const unsigned char* data, size_t length, unsigned char digest[])
{

}

/*
 * Format the SHA-1 digest into the buffer, which must be able to hold at
 * least kSHA1DigestOutputLen bytes.  Returns a pointer to the buffer,
 */
static const char* dexSHA1DigestToStr(const unsigned char digest[],char* tmpBuf)
{
    static const char hexDigit[] = "0123456789abcdef";
    char* cp;
    int i;

    cp = tmpBuf;
    for (i = 0; i < kSHA1DigestLen; i++) {
        *cp++ = hexDigit[digest[i] >> 4];
        *cp++ = hexDigit[digest[i] & 0x0f];
    }
    *cp++ = '\0';

    return tmpBuf;
}

/*
 * Compute a hash code on a UTF-8 string, for use with internal hash tables.
 *
 * This may or may not be compatible with UTF-8 hash functions used inside
 * the Dalvik VM.
 *
 * The basic "multiply by 31 and add" approach does better on class names
 * than most other things tried (e.g. adler32).
 */
static u4 classDescriptorHash(const char* str)
{
    u4 hash = 1;

    while (*str != '\0')
        hash = hash * 31 + *str++;

    return hash;
}

/*
 * Add an entry to the class lookup table.  We hash the string and probe
 * until we find an open slot.
 */
static void classLookupAdd(DexFile* pDexFile, DexClassLookup* pLookup,
    int stringOff, int classDefOff, int* pNumProbes)
{
    const char* classDescriptor =
        (const char*) (pDexFile->baseAddr + stringOff);
    const DexClassDef* pClassDef =
        (const DexClassDef*) (pDexFile->baseAddr + classDefOff);
    u4 hash = classDescriptorHash(classDescriptor);
    int mask = pLookup->numEntries-1;
    int idx = hash & mask;

    /*
     * Find the first empty slot.  We oversized the table, so this is
     * guaranteed to finish.
     */
    int probes = 0;
    while (pLookup->table[idx].classDescriptorOffset != 0) {
        idx = (idx + 1) & mask;
        probes++;
    }

    pLookup->table[idx].classDescriptorHash = hash;
    pLookup->table[idx].classDescriptorOffset = stringOff;
    pLookup->table[idx].classDefOffset = classDefOff;
    *pNumProbes = probes;
}

/*
 * Create the class lookup hash table.
 *
 * Returns newly-allocated storage.
 */
DexClassLookup* dexCreateClassLookup(DexFile* pDexFile)
{
    DexClassLookup* pLookup;
    int allocSize;
    int i, numEntries;
    int numProbes, totalProbes, maxProbes;

    numProbes = totalProbes = maxProbes = 0;

    /*
     * Using a factor of 3 results in far less probing than a factor of 2,
     * but almost doubles the flash storage requirements for the bootstrap
     * DEX files.  The overall impact on class loading performance seems
     * to be minor.  We could probably get some performance improvement by
     * using a secondary hash.
     */
    numEntries = dexRoundUpPower2(pDexFile->pHeader->classDefsSize * 2);
    allocSize = CRTL_offsetof(DexClassLookup, table)
                    + numEntries * sizeof(pLookup->table[0]);

    pLookup = (DexClassLookup*) heapAllocPersistent(allocSize);
    if (pLookup == NULL)
        return NULL;
    CRTL_memset(pLookup, 0x0, allocSize);
    pLookup->size = allocSize;
    pLookup->numEntries = numEntries;

    for (i = 0; i < (int)pDexFile->pHeader->classDefsSize; i++) {
        const DexClassDef* pClassDef;
        const char* pString;

        pClassDef = dexGetClassDef(pDexFile, i);
        pString = dexStringByTypeIdx(pDexFile, pClassDef->classIdx);

        classLookupAdd(pDexFile, pLookup,
            (u1*)pString - pDexFile->baseAddr,
            (u1*)pClassDef - pDexFile->baseAddr, &numProbes);

        if (numProbes > maxProbes)
            maxProbes = numProbes;
        totalProbes += numProbes;
    }

    DVMTraceDbg("Class lookup: classes=%d slots=%d (%d%% occ) alloc=%d"
         " total=%d max=%d",
        pDexFile->pHeader->classDefsSize, numEntries,
        (100 * pDexFile->pHeader->classDefsSize) / numEntries,
        allocSize, totalProbes, maxProbes);

    return pLookup;
}

/*
 * Set up the basic raw data pointers of a DexFile. This function isn't
 * meant for general use.
 */
void dexFileSetupBasicPointers(DexFile* pDexFile, const u1* data)
{
    DexHeader *pHeader = (DexHeader*) data;

    pDexFile->baseAddr = data;
    pDexFile->pHeader = pHeader;
    pDexFile->pStringIds = (const DexStringId*) (data + pHeader->stringIdsOff);
    pDexFile->pTypeIds = (const DexTypeId*) (data + pHeader->typeIdsOff);
    pDexFile->pFieldIds = (const DexFieldId*) (data + pHeader->fieldIdsOff);
    pDexFile->pMethodIds = (const DexMethodId*) (data + pHeader->methodIdsOff);
    pDexFile->pProtoIds = (const DexProtoId*) (data + pHeader->protoIdsOff);
    pDexFile->pClassDefs = (const DexClassDef*) (data + pHeader->classDefsOff);
    pDexFile->pLinkData = (const DexLink*) (data + pHeader->linkOff);
}


/*
 * Parse an optimized or unoptimized .dex file sitting in memory.  This is
 * called after the byte-ordering and structure alignment has been fixed up.
 *
 * On success, return a newly-allocated DexFile.
 */
DexFile* dexFileParse(const u1* data, size_t length)
{
    int32_t    result = -1;
    DexFile*   pDexFile = NULL;
    const DexHeader* pHeader;

    if (length < sizeof(DexHeader))
    {
        DVMTraceErr("dexFileParse - Error: too short to be a valid .dex");
        goto bail;      /* bad file format */
    }

    pDexFile = (DexFile*) heapAllocPersistent(sizeof(DexFile));
    if (pDexFile == NULL)
        goto bail;      /* alloc failure */
    CRTL_memset(pDexFile, 0, sizeof(DexFile));

    if (CRTL_memcmp(data, DEX_MAGIC, 8) != 0)
    {
        DVMTraceErr("dexFileParse - Error: wrong dex magic number\n");
        goto bail; 
    }

    dexFileSetupBasicPointers(pDexFile, data);

    pHeader = pDexFile->pHeader;

    /**
     * TODO: Verify the checksum(s).
     */

    /**
     * TODO: Verify the SHA-1 digest.
     */


    if (pHeader->classDefsSize == 0) {
        DVMTraceErr("ERROR: DEX file has no classes in it, failing\n");
        goto bail;
    }

    /*
     * Success!
     */
    result = 0;

bail:
    if (result != 0 && pDexFile != NULL)
    {
        pDexFile = NULL;
    }
    return pDexFile;
}

/*
 * Look up a class definition entry by descriptor.
 *
 * "descriptor" should look like "Landroid/debug/Stuff;".
 */
const DexClassDef* dexFindClass(const DexFile* pDexFile, const char* descriptor)
{
    const DexClassLookup* pLookup = pDexFile->pClassLookup;
    u4 hash;
    int idx, mask;

    hash = classDescriptorHash(descriptor);
    mask = pLookup->numEntries - 1;
    idx = hash & mask;

    /*
     * Search until we find a matching entry or an empty slot.
     */
    while (TRUE)
    {
        int offset;

        offset = pLookup->table[idx].classDescriptorOffset;
        if (offset == 0)
            return NULL;

        if (pLookup->table[idx].classDescriptorHash == hash)
        {
            const char* str;

            str = (const char*) (pDexFile->baseAddr + offset);
            if (CRTL_strcmp(str, descriptor) == 0) {
                return (const DexClassDef*)
                    (pDexFile->baseAddr + pLookup->table[idx].classDefOffset);
            }
        }

        idx = (idx + 1) & mask;
    }
}


/*
 * Compute the DEX file checksum for a memory-mapped DEX file.
 */
u4 dexComputeChecksum(const DexHeader* pHeader)
{
    //nothing to do.
    return 0;
}

/*
 * Compute the size, in bytes, of a DexCode.
 */
size_t dexGetDexCodeSize(const DexCode* pCode)
{   
    //TODO: to implement
    return 0;
}

/*
 * Round up to the next highest power of 2.
 *
 * Found on http://graphics.stanford.edu/~seander/bithacks.html.
 */
u4 dexRoundUpPower2(u4 val)
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




