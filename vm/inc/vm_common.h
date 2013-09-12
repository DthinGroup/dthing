#ifndef __VM_COMMON_H__
#define __VM_COMMON_H__

//#pragma comment(lib,"../loader/Debug/loader.lib")

#define ARCH_X86
#define INLINE

#define _TEST_ED_    //test edward code of classloader


#ifdef ARCH_X86
	#pragma comment(lib, "Winmm.lib")
	#include <Windows.h>
	#include <Mmsystem.h>
#endif

#include <stdint.h>
#include <crtl.h>
#include <stdio.h>
#include <assert.h>

//#pragma comment(lib, "loader.lib" )
//#include <Object.h>

typedef uint8_t		u1 ;
typedef uint16_t	u2 ;
typedef uint32_t	u4 ;
typedef uint64_t	u8 ;
typedef  int8_t		s1 ;
typedef  int16_t	s2 ;
typedef  int32_t	s4 ;
typedef  int64_t	s8 ;
typedef  int   vbool;



#define DVM_MALLOC		CRTL_malloc
#define DVM_MEMSET		CRTL_memset
#define DVM_FREE		CRTL_free
#define DVM_STRCMP      CRTL_strcmp
#define DVM_ASSERT		assert

#define false (0)
#define true  (1)

/*
struct Object;
struct ClassObject;
struct  DvmDex;
struct Field;
struct InstField;
struct StaticField;
struct Method;
*/
/*----------------void err------------------------*/
/* flags for dvmMalloc */
enum {
    ALLOC_DEFAULT = 0x00,
    ALLOC_DONT_TRACK = 0x01,  /* don't add to internal tracking list */
    ALLOC_NON_MOVING = 0x02,
};


#if 0
enum {
    ALLOC_DEFAULT = 0x00,
    ALLOC_DONT_TRACK = 0x01,  /* don't add to internal tracking list */
    ALLOC_NON_MOVING = 0x02,
};

enum ClassFlags {
    CLASS_ISFINALIZABLE        = (1<<31), // class/ancestor overrides finalize()
    CLASS_ISARRAY              = (1<<30), // class is a "[*"
    CLASS_ISOBJECTARRAY        = (1<<29), // class is a "[L*" or "[[*"
    CLASS_ISCLASS              = (1<<28), // class is *the* class Class

    CLASS_ISREFERENCE          = (1<<27), // class is a soft/weak/phantom ref
                                          // only ISREFERENCE is set --> soft
    CLASS_ISWEAKREFERENCE      = (1<<26), // class is a weak reference
    CLASS_ISFINALIZERREFERENCE = (1<<25), // class is a finalizer reference
    CLASS_ISPHANTOMREFERENCE   = (1<<24), // class is a phantom reference

    CLASS_MULTIPLE_DEFS        = (1<<23), // DEX verifier: defs in multiple DEXs

    /* unlike the others, these can be present in the optimized DEX file */
    CLASS_ISOPTIMIZED          = (1<<17), // class may contain opt instrs
    CLASS_ISPREVERIFIED        = (1<<16), // class has been pre-verified
};

typedef enum  {
    METHOD_UNKNOWN  = 0,
    METHOD_DIRECT,      // <init>, private
    METHOD_STATIC,      // static
    METHOD_VIRTUAL,     // virtual, super
    METHOD_INTERFACE    // interface
}MethodType;
#endif

#define __MAY_ERROR__ 1

#if 0

struct object;
struct classObject;
struct  dvmDex;
struct field;
struct instField;
struct staticField;
struct method;

typedef union  jValue{
    u1      z;
    s1      b;
    u2      c;
    s2      s;
    s4      i;
    s8      j;
    float   f;
    double  d;
    struct object * l;
}JValue;

typedef struct field {
    struct classObject*    clazz;          /* class in which the field is declared */
    const char*     name;
    const char*     signature;      /* e.g. "I", "[C", "Landroid/os/Debug;" */
    u4              accessFlags;
}Field;

typedef struct instField {
	struct classObject*    clazz;          /* class in which the field is declared */
    const char*     name;
    const char*     signature;      /* e.g. "I", "[C", "Landroid/os/Debug;" */
    u4              accessFlags;

    /*
     * This field indicates the byte offset from the beginning of the
     * (Object *) to the actual instance data; e.g., byteOffset==0 is
     * the same as the object pointer (bug!), and byteOffset==4 is 4
     * bytes farther.
     */
    int             byteOffset;
}InstField;

typedef struct staticField  {
	struct classObject*    clazz;          /* class in which the field is declared */
    const char*     name;
    const char*     signature;      /* e.g. "I", "[C", "Landroid/os/Debug;" */
    u4              accessFlags;

    JValue			value;          /* initially set from DEX for primitives */
}StaticField;


typedef struct {
	    /* ptr to class object */
    struct classObject*    clazz;

    /*
     * A word containing either a "thin" lock or a "fat" monitor.  See
     * the comments in Sync.c for a description of its layout.
     */
    u4              lock;

    /* number of elements; immutable after init */
    u4              length;

    /*
     * Array contents; actual size is (length * sizeof(type)).  This is
     * declared as u8 so that the compiler inserts any necessary padding
     * (e.g. for EABI); the actual allocation may be smaller than 8 bytes.
     */
    u8              contents[1];
}ArrayObject;

typedef struct 
{
	int a;
}StringObject;


typedef struct classObject
{
    //u4              instanceData[CLASS_FIELD_SLOTS];

    const char*     descriptor;
    char*           descriptorAlloc;

    u4              accessFlags;

    u4              serialNumber;

    struct dvmDex*         pDvmDex;

    //ClassStatus     status;

    struct classObject*    verifyErrorClass;

    u4              initThreadId;

    size_t          objectSize;

    struct classObject*    elementClass;

    int             arrayDim;

    //PrimitiveType   primitiveType;

    struct classObject*    super;

    //Object*         classLoader;

    //InitiatingLoaderList initiatingLoaderList;

    int             interfaceCount;
    struct classObject**   interfaces;

    int             directMethodCount;
    struct method*         directMethods;

    int             virtualMethodCount;
    struct method*         virtualMethods;

    int             vtableCount;
    struct method**        vtable;

    int             iftableCount;
    //InterfaceEntry* iftable;

    int             ifviPoolCount;
    int*            ifviPool;

    int             ifieldCount;
    int             ifieldRefCount; // number of fields that are object refs
    struct instField*      ifields;

    u4 refOffsets;

    const char*     sourceFile;

    /* static fields */
    int             sfieldCount;
    struct staticField     sfields[]; /* MUST be last item */
}ClassObject;

typedef void (*DalvikBridgeFunc)(const u4* args, JValue* pResult, const struct method* met/*, Thread* self*/);
typedef void (*DalvikNativeFunc)(const u4* args, JValue* pResult);

typedef struct method
{
    struct classObject*    clazz;
    u4              accessFlags;

    u2             methodIndex;
    u2              registersSize;  /* ins + locals */
    u2              outsSize;
    u2              insSize;
    const char*     name;
    //DexProto        prototype;
    /* short-form method descriptor string */
    const char*     shorty;
    const u2*       insns;          /* instructions, in memory-mapped .dex */
    int             jniArgInfo;
    //DalvikBridgeFunc nativeFunc;

    vbool fastJni;
    vbool noRef;
    vbool shouldTrace;
    //const RegisterMap* registerMap;
    vbool            inProfile;

	DalvikBridgeFunc nativeFunc;
}Method;



typedef struct object
{
    struct classObject*    clazz;
    u4              lock;
}Object;



typedef struct  dvmDex{
	    void*            pDexFile;

    /* clone of pDexFile->pHeader (it's used frequently enough) */
    const void*    pHeader;

    /* interned strings; parallel to "stringIds" */
    void** pResStrings;

    /* resolved classes; parallel to "typeIds" */
    struct ClassObject** pResClasses;

    /* resolved methods; parallel to "methodIds" */
    struct Method**     pResMethods;

    /* resolved instance fields; parallel to "fieldIds" */
    /* (this holds both InstField and StaticField) */
    void**      pResFields;

    /* interface method lookup cache */
    void * pInterfaceCache;

    /* shared memory region with file contents */
    vbool                isMappedReadOnly;
    int          memMap;

    /* lock ensuring mutual exclusion during updates */
    int     modLock;
}DvmDex;







#endif




#endif