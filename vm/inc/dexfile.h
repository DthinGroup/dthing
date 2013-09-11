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
 * Access .dex (Dalvik Executable Format) files.  The code here assumes that
 * the DEX file has been rewritten (byte-swapped, word-aligned) and that
 * the contents can be directly accessed as a collection of C arrays.  Please
 * see docs/dalvik/dex-format.html for a detailed description.
 *
 * The structure and field names were chosen to match those in the DEX spec.
 *
 * It's generally assumed that the DEX file will be stored in shared memory,
 * obviating the need to copy code and constant pool entries into newly
 * allocated storage.  Maintaining local pointers to items in the shared area
 * is valid and encouraged.
 *
 * All memory-mapped structures are 32-bit aligned unless otherwise noted.
 */

#ifndef LIBDEX_DEXFILE_H_
#define LIBDEX_DEXFILE_H_

#include <common.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum PrimitiveType PrimitiveType;
typedef struct DexFile_s DexFile;
typedef struct DexHeader_s DexHeader;

/* DEX file magic number */
#define DEX_MAGIC       "dex\n035\0"

/*
 * 160-bit SHA-1 digest.
 */
enum { kSHA1DigestLen = 20,
       kSHA1DigestOutputLen = kSHA1DigestLen*2 +1 };

/* general constants */
enum {
    kDexEndianConstant = 0x12345678,    /* the endianness indicator */
    kDexNoIndex = 0xffffffff,           /* not a valid index value */
};

/*
 * Enumeration of all the primitive types.
 */
enum PrimitiveType_s {
    PRIM_NOT        = 0,       /* value is a reference type, not a primitive type */
    PRIM_VOID       = 1,
    PRIM_BOOLEAN    = 2,
    PRIM_BYTE       = 3,
    PRIM_SHORT      = 4,
    PRIM_CHAR       = 5,
    PRIM_INT        = 6,
    PRIM_LONG       = 7,
    PRIM_FLOAT      = 8,
    PRIM_DOUBLE     = 9,
};

/*
 * access flags and masks; the "standard" ones are all <= 0x4000
 *
 * Note: There are related declarations in vm/oo/Object.h in the ClassFlags
 * enum.
 */
enum {
    ACC_PUBLIC       = 0x00000001,       // class, field, method, ic
    ACC_PRIVATE      = 0x00000002,       // field, method, ic
    ACC_PROTECTED    = 0x00000004,       // field, method, ic
    ACC_STATIC       = 0x00000008,       // field, method, ic
    ACC_FINAL        = 0x00000010,       // class, field, method, ic
    ACC_SYNCHRONIZED = 0x00000020,       // method (only allowed on natives)
    ACC_SUPER        = 0x00000020,       // class (not used in Dalvik)
    ACC_VOLATILE     = 0x00000040,       // field
    ACC_BRIDGE       = 0x00000040,       // method (1.5)
    ACC_TRANSIENT    = 0x00000080,       // field
    ACC_VARARGS      = 0x00000080,       // method (1.5)
    ACC_NATIVE       = 0x00000100,       // method
    ACC_INTERFACE    = 0x00000200,       // class, ic
    ACC_ABSTRACT     = 0x00000400,       // class, method, ic
    ACC_STRICT       = 0x00000800,       // method
    ACC_SYNTHETIC    = 0x00001000,       // field, method, ic
    ACC_ANNOTATION   = 0x00002000,       // class, ic (1.5)
    ACC_ENUM         = 0x00004000,       // class, field, ic (1.5)
    ACC_CONSTRUCTOR  = 0x00010000,       // method (Dalvik only)
    ACC_DECLARED_SYNCHRONIZED =
                       0x00020000,       // method (Dalvik only)
    ACC_CLASS_MASK =
        (ACC_PUBLIC | ACC_FINAL | ACC_INTERFACE | ACC_ABSTRACT
                | ACC_SYNTHETIC | ACC_ANNOTATION | ACC_ENUM),
    ACC_INNER_CLASS_MASK =
        (ACC_CLASS_MASK | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC),
    ACC_FIELD_MASK =
        (ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC | ACC_FINAL
                | ACC_VOLATILE | ACC_TRANSIENT | ACC_SYNTHETIC | ACC_ENUM),
    ACC_METHOD_MASK =
        (ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC | ACC_FINAL
                | ACC_SYNCHRONIZED | ACC_BRIDGE | ACC_VARARGS | ACC_NATIVE
                | ACC_ABSTRACT | ACC_STRICT | ACC_SYNTHETIC | ACC_CONSTRUCTOR
                | ACC_DECLARED_SYNCHRONIZED),
};

/* annotation constants */
enum {
    kDexVisibilityBuild         = 0x00,     /* annotation visibility */
    kDexVisibilityRuntime       = 0x01,
    kDexVisibilitySystem        = 0x02,

    kDexAnnotationByte          = 0x00,
    kDexAnnotationShort         = 0x02,
    kDexAnnotationChar          = 0x03,
    kDexAnnotationInt           = 0x04,
    kDexAnnotationLong          = 0x06,
    kDexAnnotationFloat         = 0x10,
    kDexAnnotationDouble        = 0x11,
    kDexAnnotationString        = 0x17,
    kDexAnnotationType          = 0x18,
    kDexAnnotationField         = 0x19,
    kDexAnnotationMethod        = 0x1a,
    kDexAnnotationEnum          = 0x1b,
    kDexAnnotationArray         = 0x1c,
    kDexAnnotationAnnotation    = 0x1d,
    kDexAnnotationNull          = 0x1e,
    kDexAnnotationBoolean       = 0x1f,

    kDexAnnotationValueTypeMask = 0x1f,     /* low 5 bits */
    kDexAnnotationValueArgShift = 5,
};

/* map item type codes */
enum {
    kDexTypeHeaderItem               = 0x0000,
    kDexTypeStringIdItem             = 0x0001,
    kDexTypeTypeIdItem               = 0x0002,
    kDexTypeProtoIdItem              = 0x0003,
    kDexTypeFieldIdItem              = 0x0004,
    kDexTypeMethodIdItem             = 0x0005,
    kDexTypeClassDefItem             = 0x0006,
    kDexTypeMapList                  = 0x1000,
    kDexTypeTypeList                 = 0x1001,
    kDexTypeAnnotationSetRefList     = 0x1002,
    kDexTypeAnnotationSetItem        = 0x1003,
    kDexTypeClassDataItem            = 0x2000,
    kDexTypeCodeItem                 = 0x2001,
    kDexTypeStringDataItem           = 0x2002,
    kDexTypeDebugInfoItem            = 0x2003,
    kDexTypeAnnotationItem           = 0x2004,
    kDexTypeEncodedArrayItem         = 0x2005,
    kDexTypeAnnotationsDirectoryItem = 0x2006,
};

/* auxillary data section chunk codes */
enum {
    kDexChunkClassLookup            = 0x434c4b50,   /* CLKP */
    kDexChunkRegisterMaps           = 0x524d4150,   /* RMAP */

    kDexChunkEnd                    = 0x41454e44,   /* AEND */
};

/* debug info opcodes and constants */
enum {
    DBG_END_SEQUENCE         = 0x00,
    DBG_ADVANCE_PC           = 0x01,
    DBG_ADVANCE_LINE         = 0x02,
    DBG_START_LOCAL          = 0x03,
    DBG_START_LOCAL_EXTENDED = 0x04,
    DBG_END_LOCAL            = 0x05,
    DBG_RESTART_LOCAL        = 0x06,
    DBG_SET_PROLOGUE_END     = 0x07,
    DBG_SET_EPILOGUE_BEGIN   = 0x08,
    DBG_SET_FILE             = 0x09,
    DBG_FIRST_SPECIAL        = 0x0a,
    DBG_LINE_BASE            = -4,
    DBG_LINE_RANGE           = 15,
};

/*
 * Lookup table for classes.  It provides a mapping from class name to
 * class definition.  Used by dexFindClass().
 *
 * We calculate this at DEX optimization time and embed it in the file so we
 * don't need the same hash table in every VM.  This is slightly slower than
 * a hash table with direct pointers to the items, but because it's shared
 * there's less of a penalty for using a fairly sparse table.
 */
typedef struct DexClassLookup_s {
    int     size;                       // total size, including "size"
    int     numEntries;                 // size of table[]; always power of 2
    struct {
        u4      classDescriptorHash;    // class descriptor hash code
        int     classDescriptorOffset;  // in bytes, from start of DEX
        int     classDefOffset;         // in bytes, from start of DEX
    } table[1];
} DexClassLookup;

/*
 * Direct-mapped "header_item" struct.
 */
struct DexHeader_s {
    u1  magic[8];           /* includes version number */
    u4  checksum;           /* adler32 checksum */
    u1  signature[kSHA1DigestLen]; /* SHA-1 hash */
    u4  fileSize;           /* length of entire file */
    u4  headerSize;         /* offset to start of next section */
    u4  endianTag;
    u4  linkSize;
    u4  linkOff;
    u4  mapOff;
    u4  stringIdsSize;
    u4  stringIdsOff;
    u4  typeIdsSize;
    u4  typeIdsOff;
    u4  protoIdsSize;
    u4  protoIdsOff;
    u4  fieldIdsSize;
    u4  fieldIdsOff;
    u4  methodIdsSize;
    u4  methodIdsOff;
    u4  classDefsSize;
    u4  classDefsOff;
    u4  dataSize;
    u4  dataOff;
};

/*
 * Direct-mapped "map_item".
 */
typedef struct DexMapItem_s {
    u2 type;              /* type code (see kDexType* above) */
    u2 unused;
    u4 size;              /* count of items of the indicated type */
    u4 offset;            /* file offset to the start of data */
} DexMapItem;

/*
 * Direct-mapped "map_list".
 */
typedef struct DexMapList_s {
    u4  size;               /* #of entries in list */
    DexMapItem list[1];     /* entries */
} DexMapList;

/*
 * Direct-mapped "string_id_item".
 */
typedef struct DexStringId_s {
    u4 stringDataOff;      /* file offset to string_data_item */
} DexStringId;

/*
 * Direct-mapped "type_id_item".
 */
typedef struct DexTypeId_s {
    u4  descriptorIdx;      /* index into stringIds list for type descriptor */
} DexTypeId;

/*
 * Direct-mapped "field_id_item".
 */
typedef struct DexFieldId_s {
    u2  classIdx;           /* index into typeIds list for defining class */
    u2  typeIdx;            /* index into typeIds for field type */
    u4  nameIdx;            /* index into stringIds for field name */
} DexFieldId;

/*
 * Direct-mapped "method_id_item".
 */
typedef struct DexMethodId_s {
    u2  classIdx;           /* index into typeIds list for defining class */
    u2  protoIdx;           /* index into protoIds for method prototype */
    u4  nameIdx;            /* index into stringIds for method name */
} DexMethodId;

/*
 * Direct-mapped "proto_id_item".
 */
typedef struct DexProtoId_s {
    u4  shortyIdx;          /* index into stringIds for shorty descriptor */
    u4  returnTypeIdx;      /* index into typeIds list for return type */
    u4  parametersOff;      /* file offset to type_list for parameter types */
} DexProtoId;

/*
 * Direct-mapped "class_def_item".
 */
typedef struct DexClassDef_s {
    u4  classIdx;           /* index into typeIds for this class */
    u4  accessFlags;
    u4  superclassIdx;      /* index into typeIds for superclass */
    u4  interfacesOff;      /* file offset to DexTypeList */
    u4  sourceFileIdx;      /* index into stringIds for source file name */
    u4  annotationsOff;     /* file offset to annotations_directory_item */
    u4  classDataOff;       /* file offset to class_data_item */
    u4  staticValuesOff;    /* file offset to DexEncodedArray */
} DexClassDef;

/*
 * Direct-mapped "type_item".
 */
typedef struct DexTypeItem_s {
    u2  typeIdx;            /* index into typeIds */
} DexTypeItem;

/*
 * Direct-mapped "type_list".
 */
typedef struct DexTypeList_s {
    u4  size;               /* #of entries in list */
    DexTypeItem list[1];    /* entries */
} DexTypeList;

/*
 * Direct-mapped "code_item".
 *
 * The "catches" table is used when throwing an exception,
 * "debugInfo" is used when displaying an exception stack trace or
 * debugging. An offset of zero indicates that there are no entries.
 */
typedef struct DexCode_s {
    u2  registersSize;
    u2  insSize;
    u2  outsSize;
    u2  triesSize;
    u4  debugInfoOff;       /* file offset to debug info stream */
    u4  insnsSize;          /* size of the insns array, in u2 units */
    u2  insns[1];
    /* followed by optional u2 padding */
    /* followed by try_item[triesSize] */
    /* followed by uleb128 handlersSize */
    /* followed by catch_handler_item[handlersSize] */
} DexCode;

/*
 * Direct-mapped "try_item".
 */
typedef struct DexTry_s {
    u4  startAddr;          /* start address, in 16-bit code units */
    u2  insnCount;          /* instruction count, in 16-bit code units */
    u2  handlerOff;         /* offset in encoded handler data to handlers */
} DexTry;

/*
 * Link table.  Currently undefined.
 */
typedef struct DexLink_s {
    u1  bleargh;
} DexLink;


/*
 * Direct-mapped "annotations_directory_item".
 */
typedef struct DexAnnotationsDirectoryItem_s {
    u4  classAnnotationsOff;  /* offset to DexAnnotationSetItem */
    u4  fieldsSize;           /* count of DexFieldAnnotationsItem */
    u4  methodsSize;          /* count of DexMethodAnnotationsItem */
    u4  parametersSize;       /* count of DexParameterAnnotationsItem */
    /* followed by DexFieldAnnotationsItem[fieldsSize] */
    /* followed by DexMethodAnnotationsItem[methodsSize] */
    /* followed by DexParameterAnnotationsItem[parametersSize] */
} DexAnnotationsDirectoryItem;

/*
 * Direct-mapped "field_annotations_item".
 */
typedef struct DexFieldAnnotationsItem_s {
    u4  fieldIdx;
    u4  annotationsOff;             /* offset to DexAnnotationSetItem */
} DexFieldAnnotationsItem;

/*
 * Direct-mapped "method_annotations_item".
 */
typedef struct DexMethodAnnotationsItem_s {
    u4  methodIdx;
    u4  annotationsOff;             /* offset to DexAnnotationSetItem */
} DexMethodAnnotationsItem;

/*
 * Direct-mapped "parameter_annotations_item".
 */
typedef struct DexParameterAnnotationsItem_s {
    u4  methodIdx;
    u4  annotationsOff;             /* offset to DexAnotationSetRefList */
} DexParameterAnnotationsItem;

/*
 * Direct-mapped "annotation_set_ref_item".
 */
typedef struct DexAnnotationSetRefItem_s {
    u4  annotationsOff;             /* offset to DexAnnotationSetItem */
} DexAnnotationSetRefItem;

/*
 * Direct-mapped "annotation_set_ref_list".
 */
typedef struct DexAnnotationSetRefList_s {
    u4  size;
    DexAnnotationSetRefItem list[1];
} DexAnnotationSetRefList;

/*
 * Direct-mapped "annotation_set_item".
 */
typedef struct DexAnnotationSetItem_s {
    u4  size;
    u4  entries[1];                 /* offset to DexAnnotationItem */
} DexAnnotationSetItem;

/*
 * Direct-mapped "annotation_item".
 *
 * NOTE: this structure is byte-aligned.
 */
typedef struct DexAnnotationItem_s {
    u1  visibility;
    u1  annotation[1];              /* data in encoded_annotation format */
} DexAnnotationItem;

/*
 * Direct-mapped "encoded_array".
 *
 * NOTE: this structure is byte-aligned.
 */
typedef struct DexEncodedArray_s {
    u1  array[1];                   /* data in encoded_array format */
} DexEncodedArray;

/*
 * Structure representing a DEX file.
 *
 * Code should regard DexFile as opaque, using the API calls provided here
 * to access specific structures.
 */
typedef struct DexFile_s {

    /* pointers to directly-mapped structs and arrays in base DEX */
    const DexHeader*      pHeader;
    const DexStringId*    pStringIds;
    const DexTypeId*      pTypeIds;
    const DexFieldId*     pFieldIds;
    const DexMethodId*    pMethodIds;
    const DexProtoId*     pProtoIds;
    const DexClassDef*    pClassDefs;
    const DexLink*        pLinkData;

    /* dexfile classes lookup table */
    const DexClassLookup* pClassLookup;

    /* points to start of DEX file data */
    const u1*             baseAddr;

} DexFile;

/*
 * Utility function -- rounds up to the nearest power of 2.
 */
u4 dexRoundUpPower2(u4 val);

/*
 * Parse an optimized or unoptimized .dex file sitting in memory.
 *
 * On success, return a newly-allocated DexFile.
 */
DexFile* dexFileParse(const u1* data, size_t length);

/*
 * Fix the byte ordering of all fields in the DEX file, and do
 * structural verification. This is only required for code that opens
 * "raw" DEX files, such as the DEX optimizer.
 *
 * Return 0 on success.
 */
int dexSwapAndVerify(u1* addr, int len);

/*
 * Detect the file type of the given memory buffer via magic number.
 * Call dexSwapAndVerify() on an unoptimized DEX file, do nothing
 * but return successfully on an optimized DEX file, and report an
 * error for all other cases.
 *
 * Return 0 on success.
 */
int dexSwapAndVerifyIfNecessary(u1* addr, int len);

/*
 * Check to see if the file magic and format version in the given
 * header are recognized as valid. Returns true if they are
 * acceptable.
 */
bool_t dexHasValidMagic(const DexHeader* pHeader);

/*
 * Compute DEX checksum.
 */
u4 dexComputeChecksum(const DexHeader* pHeader);

/*
 * Free a DexFile structure, along with any associated structures.
 */
void dexFileFree(DexFile* pDexFile);

/*
 * Create class lookup table.
 */
DexClassLookup* dexCreateClassLookup(DexFile* pDexFile);

/*
 * Find a class definition by descriptor.
 */
const DexClassDef* dexFindClass(const DexFile* pDexFile, const char* descriptor);

/*
 * Set up the basic raw data pointers of a DexFile. This function isn't
 * meant for general use.
 */
void dexFileSetupBasicPointers(DexFile* pDexFile, const u1* data);

/*
 * Get the type descriptor character associated with a given primitive
 * type. This returns '\0' if the type is invalid.
 */
char dexGetPrimitiveTypeDescriptorChar(PrimitiveType type);

/*
 * Get the type descriptor string associated with a given primitive
 * type.
 */
const char* dexGetPrimitiveTypeDescriptor(PrimitiveType type);

/*
 * Get the type descriptor string associated with a given primitive
 * type.
 */
const char* dexGetPrimitiveTypeArrayDescriptor(PrimitiveType type);

/*
 * Get the boxed type descriptor string associated with a given
 * primitive type. This returns NULL for an invalid type, including
 * particularly for type "void". In the latter case, even though there
 * is a class Void, there's no such thing as a boxed instance of it.
 */
const char* dexGetBoxedTypeDescriptor(PrimitiveType type);

/*
 * Get the primitive type constant from the given descriptor character.
 * This returns PRIM_NOT (note: this is a 0) if the character is invalid
 * as a primitive type descriptor.
 */
PrimitiveType dexGetPrimitiveTypeFromDescriptorChar(char descriptorChar);


/* DexClassDef convenience - get class_data_item pointer */
const u1* dexGetClassData(const DexFile* pDexFile, const DexClassDef* pClassDef);

/* get the interface list for a DexClass */
const DexTypeList* dexGetInterfacesList(const DexFile* pDexFile, const DexClassDef* pClassDef);

/* return the Nth entry in a DexTypeList. */
const DexTypeItem* dexGetTypeItem(const DexTypeList* pList, u4 idx);

/* return the FieldId with the specified index */
const DexFieldId* dexGetFieldId(const DexFile* pDexFile, u4 idx);

/* given a ClassDef pointer, recover its index */
u4 dexGetIndexForClassDef(const DexFile* pDexFile, const DexClassDef* pClassDef);

/* return the MethodId with the specified index */
const DexMethodId* dexGetMethodId(const DexFile* pDexFile, u4 idx);

/* return the UTF-8 encoded string with the specified string_id index */
const char* dexStringById(const DexFile* pDexFile, u4 idx);

/* get the source file string */
const char* dexGetSourceFile(const DexFile* pDexFile, const DexClassDef* pClassDef);

/*
 * Get the descriptor string associated with a given type index.
 * The caller should not free() the returned string.
 */
const char* dexStringByTypeIdx(const DexFile* pDexFile, u4 idx);

/* DexClassDef convenience - get class descriptor */
const char* dexGetClassDescriptor(const DexFile* pDexFile, const DexClassDef* pClassDef);

/* return the type_idx for the Nth entry in a TypeList */
u4 dexTypeListGetIdx(const DexTypeList* pList, u4 idx);

/*
 * Get the parameter list from a ProtoId. The returns NULL if the ProtoId
 * does not have a parameter list.
 */
const DexTypeList* dexGetProtoParameters(const DexFile *pDexFile, const DexProtoId* pProtoId);

/* return the ProtoId with the specified index */
const DexProtoId* dexGetProtoId(const DexFile* pDexFile, u4 idx);

/* get the static values list for a DexClass */
const DexEncodedArray* dexGetStaticValuesList(const DexFile* pDexFile, const DexClassDef* pClassDef);

/* get the annotations directory item for a DexClass */
const DexAnnotationsDirectoryItem* dexGetAnnotationsDirectoryItem(
    const DexFile* pDexFile, const DexClassDef* pClassDef);

#ifdef __cplusplus
}
#endif

#endif  // LIBDEX_DEXFILE_H_
