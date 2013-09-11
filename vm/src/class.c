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
 * Class loading, including bootstrap class loader, linking, and
 * initialization.
 */

/*
Notes on Linking and Verification

The basic way to retrieve a class is to load it, make sure its superclass
and interfaces are available, prepare its fields, and return it.  This gets
a little more complicated when multiple threads can be trying to retrieve
the class simultaneously, requiring that we use the class object's monitor
to keep things orderly.

The linking (preparing, resolving) of a class can cause us to recursively
load superclasses and interfaces.  Barring circular references (e.g. two
classes that are superclasses of each other), this will complete without
the loader attempting to access the partially-linked class.

With verification, the situation is different.  If we try to verify
every class as we load it, we quickly run into trouble.  Even the lowly
java.lang.Object requires CloneNotSupportedException; follow the list
of referenced classes and you can head down quite a trail.  The trail
eventually leads back to Object, which is officially not fully-formed yet.

The VM spec (specifically, v2 5.4.1) notes that classes pulled in during
verification do not need to be prepared or verified.  This means that we
are allowed to have loaded but unverified classes.  It further notes that
the class must be verified before it is initialized, which allows us to
defer verification for all classes until class init.  You can't execute
code or access fields in an uninitialized class, so this is safe.

It also allows a more peaceful coexistence between verified and
unverifiable code.  If class A refers to B, and B has a method that
refers to a bogus class C, should we allow class A to be verified?
If A only exercises parts of B that don't use class C, then there is
nothing wrong with running code in A.  We can fully verify both A and B,
and allow execution to continue until B causes initialization of C.  The
VerifyError is thrown close to the point of use.

This gets a little weird with java.lang.Class, which is the only class
that can be instantiated before it is initialized.  We have to force
initialization right after the class is created, because by definition we
have instances of it on the heap, and somebody might get a class object and
start making virtual calls on it.  We can end up going recursive during
verification of java.lang.Class, but we avoid that by checking to see if
verification is already in progress before we try to initialize it.
*/

/*
Notes on class loaders and interaction with optimization / verification

In what follows, "pre-verification" and "optimization" are the steps
performed by the dexopt command, which attempts to verify and optimize
classes as part of unpacking jar files and storing the DEX data in the
dalvik-cache directory.  These steps are performed by loading the DEX
files directly, without any assistance from ClassLoader instances.

When we pre-verify and optimize a class in a DEX file, we make some
assumptions about where the class loader will go to look for classes.
If we can't guarantee those assumptions, e.g. because a class ("AppClass")
references something not defined in the bootstrap jars or the AppClass jar,
we can't pre-verify or optimize the class.

The VM doesn't define the behavior of user-defined class loaders.
For example, suppose application class AppClass, loaded by UserLoader,
has a method that creates a java.lang.String.  The first time
AppClass.stringyMethod tries to do something with java.lang.String, it
asks UserLoader to find it.  UserLoader is expected to defer to its parent
loader, but isn't required to.  UserLoader might provide a replacement
for String.

We can run into trouble if we pre-verify AppClass with the assumption that
java.lang.String will come from core.jar, and don't verify this assumption
at runtime.  There are two places that an alternate implementation of
java.lang.String can come from: the AppClass jar, or from some other jar
that UserLoader knows about.  (Someday UserLoader will be able to generate
some bytecode and call DefineClass, but not yet.)

To handle the first situation, the pre-verifier will explicitly check for
conflicts between the class being optimized/verified and the bootstrap
classes.  If an app jar contains a class that has the same package and
class name as a class in a bootstrap jar, the verification resolver refuses
to find either, which will block pre-verification and optimization on
classes that reference ambiguity.  The VM will postpone verification of
the app class until first load.

For the second situation, we need to ensure that all references from a
pre-verified class are satisified by the class' jar or earlier bootstrap
jars.  In concrete terms: when resolving a reference to NewClass,
which was caused by a reference in class AppClass, we check to see if
AppClass was pre-verified.  If so, we require that NewClass comes out
of either the AppClass jar or one of the jars in the bootstrap path.
(We may not control the class loaders, but we do manage the DEX files.
We can verify that it's either (loader==null && dexFile==a_boot_dex)
or (loader==UserLoader && dexFile==AppClass.dexFile).  Classes from
DefineClass can't be pre-verified, so this doesn't apply.)

This should ensure that you can't "fake out" the pre-verifier by creating
a user-defined class loader that replaces system classes.  It should
also ensure that you can write such a loader and have it work in the
expected fashion; all you lose is some performance due to "just-in-time
verification" and the lack of DEX optimizations.

There is a "back door" of sorts in the class resolution check, due to
the fact that the "class ref" entries are shared between the bytecode
and meta-data references (e.g. annotations and exception handler lists).
The class references in annotations have no bearing on class verification,
so when a class does an annotation query that causes a class reference
index to be resolved, we don't want to fail just because the calling
class was pre-verified and the resolved class is in some random DEX file.
The successful resolution adds the class to the "resolved classes" table,
so when optimized bytecode references it we don't repeat the resolve-time
check.  We can avoid this by not updating the "resolved classes" table
when the class reference doesn't come out of something that has been
checked by the verifier, but that has a nonzero performance impact.
Since the ultimate goal of this test is to catch an unusual situation
(user-defined class loaders redefining core classes), the added caution
may not be worth the performance hit.
*/

#include <dthing.h>
#include <heap.h>
#include <opl_file.h>
#include <encoding.h>
#include <rawdexfile.h>
#include <dexclass.h>
#include <dexfile.h>
#include <dexproto.h>
#include <resolve.h>
#include <array.h>
#include <accesscheck.h>
#include <reflect.h>


/* bootstrap classes buffer */
LOCAL const uint8_t bootStrapClasses[] =
#include<dalvik_core.h>
;

#define CLASS_SFIELD_SLOTS 1

/* search for these criteria in the Class hash table */
typedef struct ClassMatchCriteria_s
{
    const char* descriptor;
} ClassMatchCriteria;


static size_t classObjectSize(size_t sfieldCount)
{
    size_t offset = CRTL_offsetof(ClassObject, sfields);
    return offset + sizeof(StaticField) * sfieldCount;
}

/* (documented in header) */
ClassObject* dvmFindPrimitiveClass(char type)
{
    PrimitiveType primitiveType = dexGetPrimitiveTypeFromDescriptorChar(type);

    switch (primitiveType) {
        case PRIM_VOID:    return gDvm.typeVoid;
        case PRIM_BOOLEAN: return gDvm.typeBoolean;
        case PRIM_BYTE:    return gDvm.typeByte;
        case PRIM_SHORT:   return gDvm.typeShort;
        case PRIM_CHAR:    return gDvm.typeChar;
        case PRIM_INT:     return gDvm.typeInt;
        case PRIM_LONG:    return gDvm.typeLong;
        case PRIM_FLOAT:   return gDvm.typeFloat;
        case PRIM_DOUBLE:  return gDvm.typeDouble;
        default: {
            DVMTraceWar("Unknown primitive type '%c'", type);
            return NULL;
        }
    }
}

/*
 * Synthesize a primitive class.
 *
 * Just creates the class and returns it (does not add it to the class list).
 */
static bool_t createPrimitiveType(PrimitiveType primitiveType, ClassObject** pClass)
{
    /*
     * Fill out a few fields in the ClassObject.
     *
     * Note that primitive classes do not sub-class the class Object.
     * This matters for "instanceof" checks. Also, we assume that the
     * primitive class does not override finalize().
     */

    const char* descriptor = dexGetPrimitiveTypeDescriptor(primitiveType);
    //assert(descriptor != NULL);

    ClassObject* newClass = (ClassObject*) heapAllocPersistent(sizeof(*newClass));
    if (newClass == NULL) {
        return FALSE;
    }
    CRTL_memset(newClass, 0x0, sizeof(*newClass));

    DVM_OBJECT_INIT(&newClass->obj, gDvm.classJavaLangClass);
    SET_CLASS_FLAG(newClass, ACC_PUBLIC | ACC_FINAL | ACC_ABSTRACT);
    newClass->primitiveType = primitiveType;
    newClass->descriptor = descriptor;
    newClass->super = NULL;
    newClass->status = CLASS_INITIALIZED;

    /* don't need to set newClass->objectSize */

    DVMTraceInf("Constructed class for primitive type '%s'\n", newClass->descriptor);

    *pClass = newClass;

    return TRUE;
}

static bool_t createPrimitiveTypeArray(PrimitiveType primitiveType, ClassObject** pClass)
{
    const char* descriptor = dexGetPrimitiveTypeArrayDescriptor(primitiveType);
    ClassObject* result = NULL;

    result = dvmFindArrayClass(descriptor);
    if (result == NULL) {
        *pClass = NULL;
        return FALSE;
    }

    *pClass = result;

    return TRUE;
}

/*
 * Create the initial class instances. These consist of the class
 * Class and all of the classes representing primitive types.
 */
static bool_t createPrimitiveInitialClasses()
{
    /*
     * Initialize the class Class. This has to be done specially, particularly
     * because it is an instance of itself.
     */
    bool_t Okay = TRUE;
    ClassObject* clazz = (ClassObject*)heapAllocPersistent((int32_t)classObjectSize(CLASS_SFIELD_SLOTS));
    if (clazz == NULL) {
        return FALSE;
    }
    CRTL_memset(clazz, 0x0, (int32_t)classObjectSize(CLASS_SFIELD_SLOTS));

    DVM_OBJECT_INIT(&clazz->obj, clazz);
    SET_CLASS_FLAG(clazz, ACC_PUBLIC | ACC_FINAL | CLASS_ISCLASS);
    clazz->descriptor = "Ljava/lang/Class;";
    gDvm.classJavaLangClass = clazz;
    DVMTraceInf("Constructed the class Class.");

    /*
     * Initialize the classes representing primitive types. These are
     * instances of the class Class, but other than that they're fairly
     * different from regular classes.
     */
    Okay &= createPrimitiveType(PRIM_VOID,    &gDvm.typeVoid);
    Okay &= createPrimitiveType(PRIM_BOOLEAN, &gDvm.typeBoolean);
    Okay &= createPrimitiveType(PRIM_BYTE,    &gDvm.typeByte);
    Okay &= createPrimitiveType(PRIM_SHORT,   &gDvm.typeShort);
    Okay &= createPrimitiveType(PRIM_CHAR,    &gDvm.typeChar);
    Okay &= createPrimitiveType(PRIM_INT,     &gDvm.typeInt);
    Okay &= createPrimitiveType(PRIM_LONG,    &gDvm.typeLong);
    Okay &= createPrimitiveType(PRIM_FLOAT,   &gDvm.typeFloat);
    Okay &= createPrimitiveType(PRIM_DOUBLE,  &gDvm.typeDouble);

    return Okay;
}


/*
 * Create the initial class instances. These consist of the class
 * Class and all of the classes representing primitive types.
 */
static bool_t createPrimitiveArrayInitialClasses()
{
    bool_t Okay = TRUE;
    /* load Object class as early as we can */
    dvmFindClass("Ljava/lang/Object;");

    Okay &= createPrimitiveTypeArray(PRIM_BOOLEAN, &gDvm.classArrayBoolean);
    Okay &= createPrimitiveTypeArray(PRIM_BYTE,    &gDvm.classArrayByte);
    Okay &= createPrimitiveTypeArray(PRIM_SHORT,   &gDvm.classArrayShort);
    Okay &= createPrimitiveTypeArray(PRIM_CHAR,    &gDvm.classArrayChar);
    Okay &= createPrimitiveTypeArray(PRIM_INT,     &gDvm.classArrayInt);
    Okay &= createPrimitiveTypeArray(PRIM_LONG,    &gDvm.classArrayLong);
    Okay &= createPrimitiveTypeArray(PRIM_FLOAT,   &gDvm.classArrayFloat);
    Okay &= createPrimitiveTypeArray(PRIM_DOUBLE,  &gDvm.classArrayDouble);

    return Okay;
}


/*
 * Get the filename suffix of the given file (everything after the
 * last "." if any, or "<none>" if there's no apparent suffix). The
 * passed-in buffer will always be '\0' terminated.
 */
static void getFileNameSuffix(const char* fileName, char* suffixBuf, size_t suffixBufLen)
{
    const char* lastDot = CRTL_strrchr(fileName, '.');
    int32_t     suffixLen;

    if (lastDot == NULL)
    {
        CRTL_memcpy(suffixBuf, "<none>", 6);
        return;
    }

    suffixLen = (int32_t)CRTL_strlen(fileName) - (int32_t)(lastDot - fileName);

    //not enough space to store data.
    //assert(suffixLen > suffixBufLen);

    CRTL_strncpy(suffixBuf, (lastDot + 1), suffixLen);
}

static int32_t findFirstUnusedClassEntry(ClassesEntry** ppClsEntry)
{
    int32_t i;
    ClassesEntry* pFirstClsEntry = NULL;

    if (gDvm.pClsEntry == NULL)
    {
        DVMTraceErr("findFirstUnusedClassEntry Error!\n");
        return -1;
    }

    for (i = 0; i < MAX_NUM_CLASSES_ENTRY; i++)
    {
        if (gDvm.pClsEntry[i].kind == kCpeUnused)
        {
            pFirstClsEntry = &gDvm.pClsEntry[i];
            break;
        }

    }

    *ppClsEntry = pFirstClsEntry;

    return i;
}

/*
 * Clone a Method, making new copies of anything that will be freed up
 * by freeMethodInnards().  This is used for "miranda" methods.
 */
static void cloneMethod(Method* dst, const Method* src)
{
    CRTL_memcpy(dst, src, sizeof(Method));
}

/*
 * Load information about a static field.
 *
 * This also "prepares" static fields by initializing them
 * to their "standard default values".
 */
static void loadSFieldFromDex(ClassObject* clazz, const DexField* pDexSField, StaticField* sfield)
{
    DexFile* pDexFile = clazz->pDvmDex->pDexFile;
    const DexFieldId* pFieldId;

    pFieldId = dexGetFieldId(pDexFile, pDexSField->fieldIdx);

    sfield->field.clazz = clazz;
    sfield->field.name = dexStringById(pDexFile, pFieldId->nameIdx);
    sfield->field.signature = dexStringByTypeIdx(pDexFile, pFieldId->typeIdx);
    sfield->field.accessFlags = pDexSField->accessFlags;

    /* Static object field values are set to "standard default values"
     * (null or 0) until the class is initialized.  We delay loading
     * constant values from the class until that time.
     */
    //sfield->value.j = 0;
    //assert(sfield->value.j == 0LL);     // cleared earlier with calloc
}


/*
 * Load information about an instance field.
 */
static void loadIFieldFromDex(ClassObject* clazz, const DexField* pDexIField, InstField* ifield)
{
    DexFile* pDexFile = clazz->pDvmDex->pDexFile;
    const DexFieldId* pFieldId;

    pFieldId = dexGetFieldId(pDexFile, pDexIField->fieldIdx);

    ifield->field.clazz = clazz;
    ifield->field.name = dexStringById(pDexFile, pFieldId->nameIdx);
    ifield->field.signature = dexStringByTypeIdx(pDexFile, pFieldId->typeIdx);
    ifield->field.accessFlags = pDexIField->accessFlags;
}

/*
 * Pull the interesting pieces out of a DexMethod.
 *
 * The DEX file isn't going anywhere, so we don't need to make copies of
 * the code area.
 */
static void loadMethodFromDex(ClassObject* clazz, const DexMethod* pDexMethod,
    Method* meth)
{
    DexFile* pDexFile = clazz->pDvmDex->pDexFile;
    const DexMethodId* pMethodId;
    const DexCode* pDexCode;

    pMethodId = dexGetMethodId(pDexFile, pDexMethod->methodIdx);

    meth->name = dexStringById(pDexFile, pMethodId->nameIdx);
    dexProtoSetFromMethodId(&meth->prototype, pDexFile, pMethodId);
    meth->shorty = dexProtoGetShorty(&meth->prototype);
    meth->accessFlags = pDexMethod->accessFlags;
    meth->clazz = clazz;
    //meth->jniArgInfo = 0;

    pDexCode = dexGetCode(pDexFile, pDexMethod);
    if (pDexCode != NULL)
    {
        /* integer constants, copy over for faster access */
        meth->registersSize = pDexCode->registersSize;
        meth->insSize = pDexCode->insSize;
        meth->outsSize = pDexCode->outsSize;

        /* pointer to code area */
        meth->insns = pDexCode->insns;
    }
    else
    {
        /*
         * We don't have a DexCode block, but we still want to know how
         * much space is needed for the arguments (so we don't have to
         * compute it later).  We also take this opportunity to compute
         * JNI argument info.
         *
         * We do this for abstract methods as well, because we want to
         * be able to substitute our exception-throwing "stub" in.
         */
        int argsSize = dvmComputeMethodArgsSize(meth);
        if (!dvmIsStaticMethod(meth))
            argsSize++;
        meth->registersSize = meth->insSize = argsSize;
        //assert(meth->outsSize == 0);
        //assert(meth->insns == NULL);

        if (dvmIsNativeMethod(meth))
        {
            //TODO: How to handle native method?
        }
    }
}

static bool_t processBootstrapClasses(const uint8_t* rawData, int32_t length)
{
    int32_t index;
    ClassesEntry* pClsEntry = NULL;

    index = findFirstUnusedClassEntry(&pClsEntry);

    if (index != 0 || pClsEntry == NULL)
    {
        DVMTraceErr("processBootstrapClasses - "
            "Error: first ClassesEntry should reserve for bootstrap classes\n");
        return FALSE;
    }

    dvmRawDexFileOpenArray((u1*)rawData, length, &pClsEntry->pDvmDex);
    pClsEntry->fileName = NULL; //Read from ROM, no file name;
    pClsEntry->fnameLen = 0;
    pClsEntry->kind     = KCpeRom;
    
    return TRUE;
}

/*
 * Convert a colon-separated list of directories, Zip files, and DEX files
 * into an array of ClassPathEntry structs.
 *
 * During normal startup we fail if there are no entries, because we won't
 * get very far without the basic language support classes, but if we're
 * optimizing a DEX file we allow it.
 *
 * If entries are added or removed from the bootstrap class path, the
 * dependencies in the DEX files will break, and everything except the
 * very first entry will need to be regenerated.
 */
static bool_t processClassPath(const char* pathStr)
{

    int32_t index;
    ClassesEntry* pClsEntry = NULL;
    char* mangle;
    char* cp;
    const char* end;
    int idx, count;

    mangle = CRTL_strdup(pathStr);
    /*
     * Run through and essentially strtok() the string.  Get a count of
     * the #of elements while we're at it.
     *
     * If the path was constructed strangely (e.g. ":foo::bar:") this will
     * over-allocate, which isn't ideal but is mostly harmless.
     */
    count = 1;
    for (cp = mangle; *cp != '\0'; cp++)
    {
        if (*cp == ';' || *cp == ':')
        {   /* separates two entries */
            count++;
            *cp = '\0';
        }
    }
    end = cp;

    /*
     * Go through a second time, pulling stuff out.
     */
    cp = mangle;
    idx = 0;

    while(cp < end)
    {
        if (*cp == '\0') 
        {
            /* leading, trailing, or doubled ':'; ignore it */
        } 
        else 
        {

            uint16_t clsPackName[MAX_FILE_NAME_LEN] = {0x0,};
            int32_t  srcBytes = 0;
            int32_t  dstChars = 0;
            char     suffix[10];

            index = findFirstUnusedClassEntry(&pClsEntry);

            if (pClsEntry == NULL || index >= MAX_NUM_CLASSES_ENTRY)
            {
                DVMTraceErr("processClassPath - Too many third-party java libs\n");
                goto bail;
            }

            pClsEntry->fileName = heapDupStr(cp); //directly reference of gDvm.classPathStr
            pClsEntry->fnameLen = (int32_t)CRTL_strlen(cp);
            srcBytes = pClsEntry->fnameLen;

            dstChars = convertAsciiToUcs2(pClsEntry->fileName, srcBytes, clsPackName, MAX_FILE_NAME_LEN);

            if (dstChars <= 0 || file_exists(clsPackName, dstChars) != FILE_RES_ISREG)
            {
                DVMTraceDbg("prepareCpe - Error: unknow file package name.");
                return FALSE;
            }

            getFileNameSuffix(pClsEntry->fileName, suffix, sizeof(suffix));

            if (CRTL_strcmp(suffix, "dex") == 0)
            {
                DvmDex* pDvmDex;

                if (dvmRawDexFileOpen(pClsEntry->fileName, &pDvmDex) == 0)
                {
                    pClsEntry->kind = kCpeDex;
                    pClsEntry->pDvmDex = pDvmDex;
                }
            }
            else if ((CRTL_strcmp(suffix, "jar") == 0) || (CRTL_strcmp(suffix, "zip") == 0)
                    ||(CRTL_strcmp(suffix, "apk") == 0))
            {
                //TODO: not support yet.
                DVMTraceDbg("prepareCpe - suffix(%s) not supported yet.\n", suffix);
                pClsEntry->kind = kCpeDex;
                pClsEntry->pDvmDex = NULL;
            }
            else
            {
                DVMTraceErr("prepareCpe - Error: unknow type suffix(%s).\n", suffix);
            }

        }
    }
    
bail:
    CRTL_free(mangle);
    return TRUE;
}

/*
 * Initialize the bootstrap class loader.
 *
 * Call this after the bootclasspath string has been finalized.
 */
bool_t dvmClassStartup()
{
    /**
     * Create hastable for class loading.
     */
    gDvm.loadedClasses = dvmHashTableCreate();

    gDvm.pClsEntry = heapAllocPersistent(sizeof(ClassesEntry) * MAX_NUM_CLASSES_ENTRY);
    if (gDvm.pClsEntry == NULL)
    {
        DVMTraceErr("dvmClassStartup - no enough memory from platform");
        return FALSE;
    }

    CRTL_memset(gDvm.pClsEntry, 0x0, sizeof(ClassesEntry) * MAX_NUM_CLASSES_ENTRY);

    if (!createPrimitiveInitialClasses())
    {   
        DVMTraceErr("Error of creating primitive classes!\n");
        return FALSE;
    }

    /**
     * Parse bootstrap classes
     */
    processBootstrapClasses(bootStrapClasses, sizeof(bootStrapClasses));

    if (!createPrimitiveArrayInitialClasses())
    {
        DVMTraceErr("Error of creating primitive array classes!\n");
        return FALSE;
    }

    /**
     * Parse third-party classes or java drivers
     */
    if (gDvm.classPathStr != NULL)
    {
        processClassPath(gDvm.classPathStr);
    }

    /**
     * parse application classes.
     */
    if (gDvm.appPathStr == NULL)
    {
        DVMTraceDbg("Dvm_class_startup - launching AMS");
        //TODO: should hint to launch AMS here?
    }
    else
    {
        processClassPath(gDvm.appPathStr);
    }

    return TRUE;
}

void dvmClassShutdown()
{
    //CRTL_freeif(gDvm.pClsEntry);
}


/*
 * Helper for loadClassFromDex, which takes a DexClassDataHeader and
 * encoded data pointer in addition to the other arguments.
 */
static ClassObject* loadClassFromDex0(DvmDex* pDvmDex,
    const DexClassDef* pClassDef, const DexClassDataHeader* pHeader,
    const u1* pEncodedData)
{
#if 1
    ClassObject* newClass = NULL;
    const DexFile* pDexFile;
    const char* descriptor;
    const DexTypeList* pInterfacesList;
    int32_t size;
    int i;

    pDexFile = pDvmDex->pDexFile;
    descriptor = dexGetClassDescriptor(pDexFile, pClassDef);

    /*
     * Allocate storage for the class object on the GC heap, so that other
     * objects can have references to it.  We bypass the usual mechanism
     * (allocObject), because we don't have all the bits and pieces yet.
     *
     * Note that we assume that java.lang.Class does not override
     * finalize().
     */
    /* TODO: Can there be fewer special checks in the usual path? */
    size = (int32_t)classObjectSize(pHeader->staticFieldsSize);
    newClass = (ClassObject*)heapAllocPersistent(size);
    if (newClass == NULL)
        return NULL;
    CRTL_memset(newClass, 0x0, size);

    newClass->descriptor = descriptor;
    SET_CLASS_FLAG(newClass, pClassDef->accessFlags);

    newClass->pDvmDex = pDvmDex;
    newClass->primitiveType = PRIM_NOT;
    newClass->status = CLASS_IDX;

    /*
     * Stuff the superclass index into the object pointer field.  The linker
     * pulls it out and replaces it with a resolved ClassObject pointer.
     * I'm doing it this way (rather than having a dedicated superclassIdx
     * field) to save a few bytes of overhead per class.
     *
     * newClass->super is not traversed or freed by dvmFreeClassInnards, so
     * this is safe.
     */
    //assert(sizeof(u4) == sizeof(ClassObject*)); /* 32-bit check */
    newClass->super = (ClassObject*) pClassDef->superclassIdx;

    /*
     * Stuff class reference indices into the pointer fields.
     *
     * The elements of newClass->interfaces are not traversed or freed by
     * dvmFreeClassInnards, so this is GC-safe.
     */
    pInterfacesList = dexGetInterfacesList(pDexFile, pClassDef);
    if (pInterfacesList != NULL)
    {
        newClass->interfaceCount = pInterfacesList->size;
        newClass->interfaces = (ClassObject**) heapAllocPersistent(newClass->interfaceCount * sizeof(ClassObject*));

        for (i = 0; i < newClass->interfaceCount; i++)
        {
            const DexTypeItem* pType = dexGetTypeItem(pInterfacesList, i);
            newClass->interfaces[i] = (ClassObject*)((u4)pType->typeIdx);
        }
    }

    /* load field definitions */

    /*
     * Over-allocate the class object and append static field info
     * onto the end.  It's fixed-size and known at alloc time.  This
     * seems to increase zygote sharing.  Heap compaction will have to
     * be careful if it ever tries to move ClassObject instances,
     * because we pass Field pointers around internally. But at least
     * now these Field pointers are in the object heap.
     */

    if (pHeader->staticFieldsSize != 0)
    {
        /* static fields stay on system heap; field data isn't "write once" */
        int count = (int) pHeader->staticFieldsSize;
        u4 lastIndex = 0;
        DexField field;

        newClass->sfieldCount = count;
        for (i = 0; i < count; i++)
        {
            dexReadClassDataField(&pEncodedData, &field, &lastIndex);
            loadSFieldFromDex(newClass, &field, &newClass->sfields[i]);
        }
    }

    if (pHeader->instanceFieldsSize != 0)
    {
        int count = (int) pHeader->instanceFieldsSize;
        u4 lastIndex = 0;
        DexField field;

        newClass->ifieldCount = count;
        newClass->ifields = (InstField*)heapAllocPersistent(count * sizeof(InstField));
        for (i = 0; i < count; i++)
        {
            dexReadClassDataField(&pEncodedData, &field, &lastIndex);
            loadIFieldFromDex(newClass, &field, &newClass->ifields[i]);
        }
    }

    if (pHeader->directMethodsSize != 0)
    {
        int count = (int) pHeader->directMethodsSize;
        u4 lastIndex = 0;
        DexMethod method;

        newClass->directMethodCount = count;
        newClass->directMethods = (Method*)heapAllocPersistent(count * sizeof(Method));
        for (i = 0; i < count; i++)
        {
            dexReadClassDataMethod(&pEncodedData, &method, &lastIndex);
            loadMethodFromDex(newClass, &method, &newClass->directMethods[i]);
        }
    }

    if (pHeader->virtualMethodsSize != 0)
    {
        int count = (int) pHeader->virtualMethodsSize;
        u4 lastIndex = 0;
        DexMethod method;

        newClass->virtualMethodCount = count;
        newClass->virtualMethods = (Method*) heapAllocPersistent(count * sizeof(Method));
        for (i = 0; i < count; i++)
        {
            dexReadClassDataMethod(&pEncodedData, &method, &lastIndex);
            loadMethodFromDex(newClass, &method, &newClass->virtualMethods[i]);
        }
    }

    newClass->sourceFile = dexGetSourceFile(pDexFile, pClassDef);

    /* caller must call dvmReleaseTrackedAlloc */
    return newClass;
#else
    return NULL;
#endif
}

/*
 * Try to load the indicated class from the specified DEX file.
 *
 * This is effectively loadClass()+defineClass() for a DexClassDef.  The
 * loading was largely done when we crunched through the DEX.
 *
 * Returns NULL on failure.  If we locate the class but encounter an error
 * while processing it, an appropriate exception is thrown.
 */
static ClassObject* loadClassFromDex(DvmDex* pDvmDex, const DexClassDef* pClassDef)
{
#if 1
    ClassObject* result;
    DexClassDataHeader header;
    const u1* pEncodedData;
    const DexFile* pDexFile;

    if ((pDvmDex == NULL) || (pClassDef == NULL))
    {
        DVMTraceErr("loadClassFromDex: pDvmDex or pClassDef is null\n");
        return NULL;
    }
    pDexFile = pDvmDex->pDexFile;



    pEncodedData = dexGetClassData(pDexFile, pClassDef);

    if (pEncodedData != NULL) {
        dexReadClassDataHeader(&pEncodedData, &header);
    } else {
        // Provide an all-zeroes header for the rest of the loading.
        CRTL_memset(&header, 0, sizeof(header));
    }

    result = loadClassFromDex0(pDvmDex, pClassDef, &header, pEncodedData);

    if (result != NULL) {
        DVMTraceInf("[Loaded %s from DEX %p]", result->descriptor, pDvmDex);
    }

    return result;
#else
    return 0;
#endif
}

/*
 * Like hashcmpClassByCrit, but passing in a fully-formed ClassObject
 * instead of a ClassMatchCriteria.
 */
static int hashcmpClassByClass(const void* vclazz, const void* vaddclazz)
{
    const ClassObject* clazz = (const ClassObject*) vclazz;
    const ClassObject* addClazz = (const ClassObject*) vaddclazz;
    bool_t match;

    match = (CRTL_strcmp(clazz->descriptor, addClazz->descriptor) == 0);

    return !match;
}



/*
 * Add a new class to the hash table.
 *
 * The class is considered "new" if it doesn't match on both the class
 * descriptor and the defining class loader.
 *
 * TODO: we should probably have separate hash tables for each
 * ClassLoader. This could speed up dvmLookupClass and
 * other common operations. It does imply a VM-visible data structure
 * for each ClassLoader object with loaded classes, which we don't
 * have yet.
 */
bool_t dvmAddClassToHash(ClassObject* clazz)
{
    void* found;
    uint32_t hash;

    hash = dvmComputeUtf8Hash(clazz->descriptor);

    found = dvmHashTableLookup(gDvm.loadedClasses, hash, clazz,
                hashcmpClassByClass, TRUE);

    DVMTraceInf("+++ dvmAddClassToHash '%s' (isnew=%d) --> %p",
        clazz->descriptor, (found == (void*) clazz), clazz);

    /* can happen if two threads load the same class simultaneously */
    return (found == (void*) clazz);
}


/*
 * Create the virtual method table.
 *
 * The top part of the table is a copy of the table from our superclass,
 * with our local methods overriding theirs.  The bottom part of the table
 * has any new methods we defined.
 */
static bool_t createVtable(ClassObject* clazz)
{
#if 1
    bool_t result = FALSE;
    int maxCount;
    int i;

    if (clazz->super != NULL) {
        //LOGI("SUPER METHODS %d %s->%s", clazz->super->vtableCount,
        //    clazz->descriptor, clazz->super->descriptor);
    }

    /* the virtual methods we define, plus the superclass vtable size */
    maxCount = clazz->virtualMethodCount;
    if (clazz->super != NULL) {
        maxCount += clazz->super->vtableCount;
    } else {
        /* TODO: is this invariant true for all java/lang/Objects,
         * regardless of the class loader?  For now, assume it is.
         */
        //assert(strcmp(clazz->descriptor, "Ljava/lang/Object;") == 0);
    }
    //LOGD("+++ max vmethods for '%s' is %d", clazz->descriptor, maxCount);

    /*
     * Over-allocate the table, then realloc it down if necessary.  So
     * long as we don't allocate anything in between we won't cause
     * fragmentation, and reducing the size should be unlikely to cause
     * a buffer copy.
     */
    //dvmLinearReadWrite(clazz->classLoader, clazz->virtualMethods);
    clazz->vtable = (Method**)heapAllocPersistent(sizeof(Method*) * maxCount);
    if (clazz->vtable == NULL)
        goto bail;

    if (clazz->super != NULL)
    {
        int actualCount;

        CRTL_memcpy(clazz->vtable, clazz->super->vtable,
            sizeof(*(clazz->vtable)) * clazz->super->vtableCount);
        actualCount = clazz->super->vtableCount;

        /*
         * See if any of our virtual methods override the superclass.
         */
        for (i = 0; i < clazz->virtualMethodCount; i++)
        {
            Method* localMeth = &clazz->virtualMethods[i];
            int si;

            for (si = 0; si < clazz->super->vtableCount; si++)
            {
                Method* superMeth = clazz->vtable[si];

                if (dvmCompareMethodNamesAndProtos(localMeth, superMeth) == 0)
                {
                    /* verify */
                    if (dvmIsFinalMethod(superMeth))
                    {
                        DVMTraceErr("Method %s.%s overrides final %s.%s",
                            localMeth->clazz->descriptor, localMeth->name,
                            superMeth->clazz->descriptor, superMeth->name);
                        goto bail;
                    }
                    clazz->vtable[si] = localMeth;
                    localMeth->methodIndex = (u2) si;
                    //LOGV("+++   override %s.%s (slot %d)",
                    //    clazz->descriptor, localMeth->name, si);
                    break;
                }
            }

            if (si == clazz->super->vtableCount)
            {
                /* not an override, add to end */
                clazz->vtable[actualCount] = localMeth;
                localMeth->methodIndex = (u2) actualCount;
                actualCount++;

                //LOGV("+++   add method %s.%s",
                //    clazz->descriptor, localMeth->name);
            }
        }

        if (actualCount != (u2) actualCount)
        {
            DVMTraceErr("Too many methods (%d) in class '%s'", actualCount,
                 clazz->descriptor);
            goto bail;
        }

        //assert(actualCount <= maxCount);

        #if 0
        //I don't think we need realloc here since actualCount is LE maxCount.
        //The only thing needs to consider is more memory is used than actual use.
        if (actualCount < maxCount)
        {
            //assert(clazz->vtable != NULL);
            //dvmLinearReadOnly(clazz->classLoader, clazz->vtable);
            clazz->vtable = (Method **)dvmLinearRealloc(clazz->classLoader,
                clazz->vtable, sizeof(*(clazz->vtable)) * actualCount);
            if (clazz->vtable == NULL) {
                DVMTraceErr("vtable realloc failed");
                goto bail;
            } else {
                DVMTraceInf("+++  reduced vtable from %d to %d", maxCount, actualCount);
            }
        }
        #endif
        
        clazz->vtableCount = actualCount;
    } 
    else
    {
        /* java/lang/Object case */
        int count = clazz->virtualMethodCount;
        if (count != (u2) count) {
            DVMTraceErr("Too many methods (%d) in base class '%s'", count,
                 clazz->descriptor);
            goto bail;
        }

        for (i = 0; i < count; i++)
        {
            clazz->vtable[i] = &clazz->virtualMethods[i];
            clazz->virtualMethods[i].methodIndex = (u2) i;
        }
        clazz->vtableCount = clazz->virtualMethodCount;
    }

    result = TRUE;

bail:
    //dvmLinearReadOnly(clazz->classLoader, clazz->vtable);
    //dvmLinearReadOnly(clazz->classLoader, clazz->virtualMethods);
    return result;
#else
    return FALSE;
#endif
}

/*
 * Create and populate "iftable".
 *
 * The set of interfaces we support is the combination of the interfaces
 * we implement directly and those implemented by our superclass.  Each
 * interface can have one or more "superinterfaces", which we must also
 * support.  For speed we flatten the tree out.
 *
 * We might be able to speed this up when there are lots of interfaces
 * by merge-sorting the class pointers and binary-searching when removing
 * duplicates.  We could also drop the duplicate removal -- it's only
 * there to reduce the memory footprint.
 *
 * Because of "Miranda methods", this may reallocate clazz->virtualMethods.
 *
 * Returns "true" on success.
 */
static bool_t createIftable(ClassObject* clazz)
{
#if 1
    bool_t result = FALSE;
    bool_t zapIftable = FALSE;
    bool_t zapVtable = FALSE;
    bool_t zapIfvipool = FALSE;
    int poolOffset = 0, poolSize = 0;
    Method** mirandaList = NULL;
    int mirandaCount = 0, mirandaAlloc = 0;
    int ifCount;
    int i, idx;

    int superIfCount;
    if (clazz->super != NULL)
        superIfCount = clazz->super->iftableCount;
    else
        superIfCount = 0;

    ifCount = superIfCount;
    ifCount += clazz->interfaceCount;
    for (i = 0; i < clazz->interfaceCount; i++)
        ifCount += clazz->interfaces[i]->iftableCount;

    DVMTraceInf("INTF: class '%s' direct w/supra=%d super=%d total=%d",
        clazz->descriptor, ifCount - superIfCount, superIfCount, ifCount);

    if (ifCount == 0) {
        //assert(clazz->iftableCount == 0);
        //assert(clazz->iftable == NULL);
        return TRUE;
    }

    /*
     * Create a table with enough space for all interfaces, and copy the
     * superclass' table in.
     */
    clazz->iftable = (InterfaceEntry*)heapAllocPersistent(sizeof(InterfaceEntry) * ifCount);
    zapIftable = TRUE;
    CRTL_memset(clazz->iftable, 0x00, sizeof(InterfaceEntry) * ifCount);
    if (superIfCount != 0)
    {
        CRTL_memcpy(clazz->iftable, clazz->super->iftable,
            sizeof(InterfaceEntry) * superIfCount);
    }

    /*
     * Create a flattened interface hierarchy of our immediate interfaces.
     */
    idx = superIfCount;

    for (i = 0; i < clazz->interfaceCount; i++)
    {
        int j;
        ClassObject* interf = clazz->interfaces[i];
        //assert(interf != NULL);

        /* make sure this is still an interface class */
        if (!dvmIsInterfaceClass(interf)) {
            DVMTraceWar("Class '%s' implements non-interface '%s'",
                clazz->descriptor, interf->descriptor);
            //TODO: Should throw exception here?
            //dvmThrowIncompatibleClassChangeErrorWithClassMessage(
            //    clazz->descriptor);
            goto bail;
        }

        /* add entry for this interface */
        clazz->iftable[idx++].clazz = interf;

        /* add entries for the interface's superinterfaces */
        for (j = 0; j < interf->iftableCount; j++)
        {
            clazz->iftable[idx++].clazz = interf->iftable[j].clazz;
        }
    }

    //assert(idx == ifCount);

    clazz->iftableCount = ifCount;

    /*
     * If we're an interface, we don't need the vtable pointers, so
     * we're done.  If this class doesn't implement an interface that our
     * superclass doesn't have, then we again have nothing to do.
     */
    if (dvmIsInterfaceClass(clazz) || superIfCount == ifCount) {
        //dvmDumpClass(clazz, kDumpClassFullDetail);
        result = TRUE;
        goto bail;
    }

    /*
     * When we're handling invokeinterface, we probably have an object
     * whose type is an interface class rather than a concrete class.  We
     * need to convert the method reference into a vtable index.  So, for
     * every entry in "iftable", we create a list of vtable indices.
     *
     * Because our vtable encompasses the superclass vtable, we can use
     * the vtable indices from our superclass for all of the interfaces
     * that weren't directly implemented by us.
     *
     * Each entry in "iftable" has a pointer to the start of its set of
     * vtable offsets.  The iftable entries in the superclass point to
     * storage allocated in the superclass, and the iftable entries added
     * for this class point to storage allocated in this class.  "iftable"
     * is flat for fast access in a class and all of its subclasses, but
     * "ifviPool" is only created for the topmost implementor.
     */
    for (i = superIfCount; i < ifCount; i++)
    {
        /*
         * Note it's valid for an interface to have no methods (e.g.
         * java/io/Serializable).
         */
        DVMTraceInf("INTF: pool: %d from %s",
            clazz->iftable[i].clazz->virtualMethodCount,
            clazz->iftable[i].clazz->descriptor);
        poolSize += clazz->iftable[i].clazz->virtualMethodCount;
    }

    if (poolSize == 0)
    {
        DVMTraceInf("INTF: didn't find any new interfaces with methods");
        result = TRUE;
        goto bail;
    }

    clazz->ifviPoolCount = poolSize;
    clazz->ifviPool = (int*) heapAllocPersistent(poolSize * sizeof(int*));
    zapIfvipool = TRUE;

    /*
     * Fill in the vtable offsets for the interfaces that weren't part of
     * our superclass.
     */
    for (i = superIfCount; i < ifCount; i++)
    {
        ClassObject* interface;
        int methIdx;

        clazz->iftable[i].methodIndexArray = clazz->ifviPool + poolOffset;
        interface = clazz->iftable[i].clazz;
        poolOffset += interface->virtualMethodCount;    // end here

        /*
         * For each method listed in the interface's method list, find the
         * matching method in our class's method list.  We want to favor the
         * subclass over the superclass, which just requires walking
         * back from the end of the vtable.  (This only matters if the
         * superclass defines a private method and this class redefines
         * it -- otherwise it would use the same vtable slot.  In Dalvik
         * those don't end up in the virtual method table, so it shouldn't
         * matter which direction we go.  We walk it backward anyway.)
         *
         *
         * Suppose we have the following arrangement:
         *   public interface MyInterface
         *     public boolean inInterface();
         *   public abstract class MirandaAbstract implements MirandaInterface
         *     //public abstract boolean inInterface(); // not declared!
         *     public boolean inAbstract() { stuff }    // in vtable
         *   public class MirandClass extends MirandaAbstract
         *     public boolean inInterface() { stuff }
         *     public boolean inAbstract() { stuff }    // in vtable
         *
         * The javac compiler happily compiles MirandaAbstract even though
         * it doesn't declare all methods from its interface.  When we try
         * to set up a vtable for MirandaAbstract, we find that we don't
         * have an slot for inInterface.  To prevent this, we synthesize
         * abstract method declarations in MirandaAbstract.
         *
         * We have to expand vtable and update some things that point at it,
         * so we accumulate the method list and do it all at once below.
         */
        for (methIdx = 0; methIdx < interface->virtualMethodCount; methIdx++)
        {
            Method* imeth = &interface->virtualMethods[methIdx];
            int j;

            for (j = clazz->vtableCount-1; j >= 0; j--) {
                if (dvmCompareMethodNamesAndProtos(imeth, clazz->vtable[j])
                    == 0)
                {
                    DVMTraceInf("INTF:   matched at %d", j);
                    if (!dvmIsPublicMethod(clazz->vtable[j])) {
                        DVMTraceErr("Implementation of %s.%s is not public",
                            clazz->descriptor, clazz->vtable[j]->name);
                        //TODO: how to throw exception here?
                        //dvmThrowIllegalAccessError(
                        //    "interface implementation not public");
                        goto bail;
                    }
                    clazz->iftable[i].methodIndexArray[methIdx] = j;
                    break;
                }
            }
            if (j < 0) {
                //dvmThrowRuntimeException("Miranda!");
                //return false;
                int mir;

                if (mirandaCount == mirandaAlloc)
                {
                    mirandaAlloc += 8;
                    mirandaList = (Method**)heapAllocPersistent(mirandaAlloc * sizeof(Method*));
                }
                
                /*
                 * These may be redundant (e.g. method with same name and
                 * signature declared in two interfaces implemented by the
                 * same abstract class).  We can squeeze the duplicates
                 * out here.
                 */
                for (mir = 0; mir < mirandaCount; mir++)
                {
                    if (dvmCompareMethodNamesAndProtos(mirandaList[mir], imeth) == 0)
                    {

                        break;
                    }
                }

                /* point the iftable at a phantom slot index */
                clazz->iftable[i].methodIndexArray[methIdx] = clazz->vtableCount + mir;
                DVMTraceInf("MIRANDA: %s points at slot %d",
                    imeth->name, clazz->vtableCount + mir);

                /* if non-duplicate among Mirandas, add to Miranda list */
                if (mir == mirandaCount) {
                    //LOGV("MIRANDA: holding '%s' in slot %d",
                    //    imeth->name, mir);
                    mirandaList[mirandaCount++] = imeth;
                }
            }
        }
    }

    if (mirandaCount != 0) {
        static const int kManyMirandas = 150;   /* arbitrary */
        Method* newVirtualMethods;
        Method* meth;
        int oldMethodCount, oldVtableCount;

        for (i = 0; i < mirandaCount; i++) {
            DVMTraceInf("MIRANDA %d: %s.%s", i,
                mirandaList[i]->clazz->descriptor, mirandaList[i]->name);
        }

        if (mirandaCount > kManyMirandas) {
            /*
             * Some obfuscators like to create an interface with a huge
             * pile of methods, declare classes as implementing it, and then
             * only define a couple of methods.  This leads to a rather
             * massive collection of Miranda methods and a lot of wasted
             * space, sometimes enough to blow out the LinearAlloc cap.
             */
            DVMTraceDbg("Note: class %s has %d unimplemented (abstract) methods",
                clazz->descriptor, mirandaCount);
        }

        /*
         * We found methods in one or more interfaces for which we do not
         * have vtable entries.  We have to expand our virtualMethods
         * table (which might be empty) to hold some new entries.
         */
        if (clazz->virtualMethods == NULL)
        {
            newVirtualMethods = (Method*)heapAllocPersistent(sizeof(Method) * (clazz->virtualMethodCount + mirandaCount));
        } else {
            //dvmLinearReadOnly(clazz->classLoader, clazz->virtualMethods);
            //TODO: how to use realloc API or to avoid memory leak
            newVirtualMethods = (Method*)heapAllocPersistent(sizeof(Method) * (clazz->virtualMethodCount + mirandaCount));
        }
        if (newVirtualMethods != clazz->virtualMethods) {
            /*
             * Table was moved in memory.  We have to run through the
             * vtable and fix the pointers.  The vtable entries might be
             * pointing at superclasses, so we flip it around: run through
             * all locally-defined virtual methods, and fix their entries
             * in the vtable.  (This would get really messy if sub-classes
             * had already been loaded.)
             *
             * Reminder: clazz->virtualMethods and clazz->virtualMethodCount
             * hold the virtual methods declared by this class.  The
             * method's methodIndex is the vtable index, and is the same
             * for all sub-classes (and all super classes in which it is
             * defined).  We're messing with these because the Miranda
             * stuff makes it look like the class actually has an abstract
             * method declaration in it.
             */
            Method* meth = newVirtualMethods;
            for (i = 0; i < clazz->virtualMethodCount; i++, meth++)
                clazz->vtable[meth->methodIndex] = meth;
        }

        oldMethodCount = clazz->virtualMethodCount;
        clazz->virtualMethods = newVirtualMethods;
        clazz->virtualMethodCount += mirandaCount;

        /*
         * We also have to expand the vtable.
         */
        //assert(clazz->vtable != NULL);
        //TODO: realloc here?
        //clazz->vtable = (Method**) dvmLinearRealloc(clazz->classLoader,
        //                clazz->vtable,
        //                sizeof(Method*) * (clazz->vtableCount + mirandaCount));
        clazz->vtable = (Method**) heapAllocPersistent(sizeof(Method*) * (clazz->vtableCount + mirandaCount));
        if (clazz->vtable == NULL)
        {
            //assert(false);
            goto bail;
        }
        zapVtable = TRUE;

        oldVtableCount = clazz->vtableCount;
        clazz->vtableCount += mirandaCount;

        /*
         * Now we need to create the fake methods.  We clone the abstract
         * method definition from the interface and then replace a few
         * things.
         *
         * The Method will be an "abstract native", with nativeFunc set to
         * dvmAbstractMethodStub().
         */
        meth = clazz->virtualMethods + oldMethodCount;
        for (i = 0; i < mirandaCount; i++, meth++)
        {
            //dvmLinearReadWrite(clazz->classLoader, clazz->virtualMethods);
            cloneMethod(meth, mirandaList[i]);
            meth->clazz = clazz;
            meth->accessFlags |= ACC_MIRANDA;
            meth->methodIndex = (u2) (oldVtableCount + i);
            //dvmLinearReadOnly(clazz->classLoader, clazz->virtualMethods);

            /* point the new vtable entry at the new method */
            clazz->vtable[oldVtableCount + i] = meth;
        }

        //dvmLinearReadOnly(clazz->classLoader, mirandaList);
        //dvmLinearFree(clazz->classLoader, mirandaList);

    }

    /*
     * TODO?
     * Sort the interfaces by number of declared methods.  All we really
     * want is to get the interfaces with zero methods at the end of the
     * list, so that when we walk through the list during invoke-interface
     * we don't examine interfaces that can't possibly be useful.
     *
     * The set will usually be small, so a simple insertion sort works.
     *
     * We have to be careful not to change the order of two interfaces
     * that define the same method.  (Not a problem if we only move the
     * zero-method interfaces to the end.)
     *
     * PROBLEM:
     * If we do this, we will no longer be able to identify super vs.
     * current class interfaces by comparing clazz->super->iftableCount.  This
     * breaks anything that only wants to find interfaces declared directly
     * by the class (dvmFindStaticFieldHier, ReferenceType.Interfaces,
     * dvmDbgOutputAllInterfaces, etc).  Need to provide a workaround.
     *
     * We can sort just the interfaces implemented directly by this class,
     * but that doesn't seem like it would provide much of an advantage.  I'm
     * not sure this is worthwhile.
     *
     * (This has been made largely obsolete by the interface cache mechanism.)
     */

    //dvmDumpClass(clazz);

    result = TRUE;

bail:
    return result;
#else
    return FALSE;
#endif
}


/*
 * Provide "stub" implementations for methods without them.
 *
 * Currently we provide an implementation for all abstract methods that
 * throws an AbstractMethodError exception.  This allows us to avoid an
 * explicit check for abstract methods in every virtual call.
 *
 * NOTE: for Miranda methods, the method declaration is a clone of what
 * was found in the interface class.  That copy may already have had the
 * function pointer filled in, so don't be surprised if it's not NULL.
 *
 * NOTE: this sets the "native" flag, giving us an "abstract native" method,
 * which is nonsensical.  Need to make sure that this doesn't escape the
 * VM.  We can either mask it out in reflection calls, or copy "native"
 * into the high 16 bits of accessFlags and check that internally.
 */
static bool_t insertMethodStubs(ClassObject* clazz)
{
#if 0
    //dvmLinearReadWrite(clazz->classLoader, clazz->virtualMethods);

    Method* meth;
    int i;

    meth = clazz->virtualMethods;
    for (i = 0; i < clazz->virtualMethodCount; i++, meth++) {
        if (dvmIsAbstractMethod(meth)) {
            assert(meth->insns == NULL);
            assert(meth->nativeFunc == NULL ||
                meth->nativeFunc == (DalvikBridgeFunc)dvmAbstractMethodStub);

            meth->accessFlags |= ACC_NATIVE;
            meth->nativeFunc = (DalvikBridgeFunc) dvmAbstractMethodStub;
        }
    }

    //dvmLinearReadOnly(clazz->classLoader, clazz->virtualMethods);
    return true;
#else
    return TRUE;
#endif
}

/*
 * Swap two instance fields.
 */
static void swapField(InstField* pOne, InstField* pTwo)
{
    InstField swap;

    DVMTraceInf("  --- swap '%s' and '%s'\n", pOne->field.name, pTwo->field.name);
    swap = *pOne;
    *pOne = *pTwo;
    *pTwo = swap;
}


/*
 * Assign instance fields to u4 slots.
 *
 * The top portion of the instance field area is occupied by the superclass
 * fields, the bottom by the fields for this class.
 *
 * "long" and "double" fields occupy two adjacent slots.  On some
 * architectures, 64-bit quantities must be 64-bit aligned, so we need to
 * arrange fields (or introduce padding) to ensure this.  We assume the
 * fields of the topmost superclass (i.e. Object) are 64-bit aligned, so
 * we can just ensure that the offset is "even".  To avoid wasting space,
 * we want to move non-reference 32-bit fields into gaps rather than
 * creating pad words.
 *
 * In the worst case we will waste 4 bytes, but because objects are
 * allocated on >= 64-bit boundaries, those bytes may well be wasted anyway
 * (assuming this is the most-derived class).
 *
 * Pad words are not represented in the field table, so the field table
 * itself does not change size.
 *
 * The number of field slots determines the size of the object, so we
 * set that here too.
 *
 * This function feels a little more complicated than I'd like, but it
 * has the property of moving the smallest possible set of fields, which
 * should reduce the time required to load a class.
 *
 * NOTE: reference fields *must* come first, or precacheReferenceOffsets()
 * will break.
 */
static bool_t computeFieldOffsets(ClassObject* clazz)
{
#if 1
    int fieldOffset;
    int i, j;

    //dvmLinearReadWrite(clazz->classLoader, clazz->ifields);

    if (clazz->super != NULL)
        fieldOffset = (int)clazz->super->objectSize;
    else
        fieldOffset = CRTL_offsetof(DataObject, instanceData);

    DVMTraceInf("--- computeFieldOffsets '%s'", clazz->descriptor);

    //LOGI("OFFSETS fieldCount=%d", clazz->ifieldCount);
    //LOGI("dataobj, instance: %d", offsetof(DataObject, instanceData));
    //LOGI("classobj, access: %d", offsetof(ClassObject, accessFlags));
    //LOGI("super=%p, fieldOffset=%d", clazz->super, fieldOffset);

    /*
     * Start by moving all reference fields to the front.
     */
    clazz->ifieldRefCount = 0;
    j = clazz->ifieldCount - 1;
    for (i = 0; i < clazz->ifieldCount; i++)
    {
        InstField* pField = &clazz->ifields[i];
        char c = pField->field.signature[0];

        if (c != '[' && c != 'L') {
            /* This isn't a reference field; see if any reference fields
             * follow this one.  If so, we'll move it to this position.
             * (quicksort-style partitioning)
             */
            while (j > i)
            {
                InstField* refField = &clazz->ifields[j--];
                char rc = refField->field.signature[0];

                if (rc == '[' || rc == 'L') {
                    /* Here's a reference field that follows at least one
                     * non-reference field.  Swap it with the current field.
                     * (When this returns, "pField" points to the reference
                     * field, and "refField" points to the non-ref field.)
                     */
                    swapField(pField, refField);

                    /* Fix the signature.
                     */
                    c = rc;

                    clazz->ifieldRefCount++;
                    break;
                }
            }
            /* We may or may not have swapped a field.
             */
        } else {
            /* This is a reference field.
             */
            clazz->ifieldRefCount++;
        }

        /*
         * If we've hit the end of the reference fields, break.
         */
        if (c != '[' && c != 'L')
            break;

        pField->byteOffset = fieldOffset;
        fieldOffset += sizeof(u4);
        DVMTraceInf("  --- offset1 '%s'=%d", pField->field.name,pField->byteOffset);
    }

    /*
     * Now we want to pack all of the double-wide fields together.  If we're
     * not aligned, though, we want to shuffle one 32-bit field into place.
     * If we can't find one, we'll have to pad it.
     */
    if (i != clazz->ifieldCount && (fieldOffset & 0x04) != 0)
    {
        InstField* pField = &clazz->ifields[i];
        char c = pField->field.signature[0];

        DVMTraceInf("  +++ not aligned");

        if (c != 'J' && c != 'D') {
            /*
             * The field that comes next is 32-bit, so just advance past it.
             */
            //assert(c != '[' && c != 'L');
            pField->byteOffset = fieldOffset;
            fieldOffset += sizeof(u4);
            i++;
            DVMTraceInf("  --- offset2 '%s'=%d",
                pField->field.name, pField->byteOffset);
        } else {
            /*
             * Next field is 64-bit, so search for a 32-bit field we can
             * swap into it.
             */
            bool_t found = FALSE;
            j = clazz->ifieldCount - 1;
            while (j > i) {
                InstField* singleField = &clazz->ifields[j--];
                char rc = singleField->field.signature[0];

                if (rc != 'J' && rc != 'D') {
                    swapField(pField, singleField);
                    //c = rc;
                    DVMTraceInf("  +++ swapped '%s' for alignment",  pField->field.name);
                    pField->byteOffset = fieldOffset;
                    fieldOffset += sizeof(u4);
                    DVMTraceInf("  --- offset3 '%s'=%d", pField->field.name, pField->byteOffset);
                    found = TRUE;
                    i++;
                    break;
                }
            }
            if (!found) {
                DVMTraceInf("  +++ inserting pad field in '%s'", clazz->descriptor);
                fieldOffset += sizeof(u4);
            }
        }
    }

    /*
     * Alignment is good, shuffle any double-wide fields forward, and
     * finish assigning field offsets to all fields.
     */
    //assert(i == clazz->ifieldCount || (fieldOffset & 0x04) == 0);
    j = clazz->ifieldCount - 1;
    for ( ; i < clazz->ifieldCount; i++) {
        InstField* pField = &clazz->ifields[i];
        char c = pField->field.signature[0];

        if (c != 'D' && c != 'J') {
            /* This isn't a double-wide field; see if any double fields
             * follow this one.  If so, we'll move it to this position.
             * (quicksort-style partitioning)
             */
            while (j > i) {
                InstField* doubleField = &clazz->ifields[j--];
                char rc = doubleField->field.signature[0];

                if (rc == 'D' || rc == 'J') {
                    /* Here's a double-wide field that follows at least one
                     * non-double field.  Swap it with the current field.
                     * (When this returns, "pField" points to the reference
                     * field, and "doubleField" points to the non-double field.)
                     */
                    swapField(pField, doubleField);
                    c = rc;

                    break;
                }
            }
            /* We may or may not have swapped a field.
             */
        } else {
            /* This is a double-wide field, leave it be.
             */
        }

        pField->byteOffset = fieldOffset;
        DVMTraceInf("  --- offset4 '%s'=%d", pField->field.name,pField->byteOffset);
        fieldOffset += sizeof(u4);
        if (c == 'J' || c == 'D')
            fieldOffset += sizeof(u4);
    }

    /*
     * We map a C struct directly on top of java/lang/Class objects.  Make
     * sure we left enough room for the instance fields.
     */
    //assert(!dvmIsTheClassClass(clazz) || (size_t)fieldOffset <
    //    OFFSETOF_MEMBER(ClassObject, instanceData) + sizeof(clazz->instanceData));

    clazz->objectSize = fieldOffset;

    //dvmLinearReadOnly(clazz->classLoader, clazz->ifields);
    return TRUE;
#else
    return FALSE;
#endif
}


/*
 * Link (prepare and resolve).  Verification is deferred until later.
 *
 * This converts symbolic references into pointers.  It's independent of
 * the source file format.
 *
 * If clazz->status is CLASS_IDX, then clazz->super and interfaces[] are
 * holding class reference indices rather than pointers.  The class
 * references will be resolved during link.  (This is done when
 * loading from DEX to avoid having to create additional storage to
 * pass the indices around.)
 *
 * Returns "false" with an exception pending on failure.
 */
bool_t dvmLinkClass(ClassObject* clazz)
{
#if 1
    u4 superclassIdx = 0;
    u4 *interfaceIdxArray = NULL;
    bool_t okay = FALSE;
    int i;

    if (!(clazz->status == CLASS_IDX || clazz->status == CLASS_LOADED))
    {
        DVMTraceErr("dvmLinkClass - Error: bad class status: %d" + clazz->status);
        return FALSE;
    }

    /* "Resolve" the class.
     *
     * At this point, clazz's reference fields may contain Dex file
     * indices instead of direct object references.  Proxy objects are
     * an exception, and may be the only exception.  We need to
     * translate those indices into real references, and let the GC
     * look inside this ClassObject.
     */
    if (clazz->status == CLASS_IDX)
    {
        if (clazz->interfaceCount > 0)
        {
            /* Copy u4 DEX idx values out of the ClassObject* array
             * where we stashed them.
             */
            size_t len = clazz->interfaceCount * sizeof(*interfaceIdxArray);
            interfaceIdxArray = (u4*)CRTL_malloc(len);
            if (interfaceIdxArray == NULL)
            {
                DVMTraceErr("Unable to allocate memory to link %s", clazz->descriptor);
                goto bail;
            }
            CRTL_memcpy(interfaceIdxArray, clazz->interfaces, len);

            CRTL_memset(clazz->interfaces, 0x0, len);
        }

        superclassIdx = (u4) clazz->super;
        clazz->super = NULL;
        /* After this line, clazz will be fair game for the GC. The
         * superclass and interfaces are all NULL.
         */
        clazz->status = CLASS_LOADED;

        if (superclassIdx != kDexNoIndex)
        {
            ClassObject* super = dvmResolveClass(clazz, superclassIdx, FALSE);
            if (super == NULL)
            {
                //TODO: should throw exception
                //assert(dvmCheckException(dvmThreadSelf()));

                DVMTraceErr("Unable to resolve superclass of %s (%d)", clazz->descriptor, superclassIdx);
                goto bail;
            }
            dvmSetFieldObject((Object *)clazz,
                              CRTL_offsetof(ClassObject, super),
                              (Object *)super);
        }

        if (clazz->interfaceCount > 0)
        {
            /* Resolve the interfaces implemented directly by this class. */
            //assert(interfaceIdxArray != NULL);
            //dvmLinearReadWrite(clazz->classLoader, clazz->interfaces);
            for (i = 0; i < clazz->interfaceCount; i++)
            {
                //assert(interfaceIdxArray[i] != kDexNoIndex);
                clazz->interfaces[i] =
                    dvmResolveClass(clazz, interfaceIdxArray[i], FALSE);
                if (clazz->interfaces[i] == NULL)
                {
                    const DexFile* pDexFile = clazz->pDvmDex->pDexFile;

                    //assert(dvmCheckException(dvmThreadSelf()));
                    //dvmLinearReadOnly(clazz->classLoader, clazz->interfaces);

                    const char* classDescriptor;
                    classDescriptor =
                        dexStringByTypeIdx(pDexFile, interfaceIdxArray[i]);

                    DVMTraceErr("Failed resolving %s interface %d '%s'",
                             clazz->descriptor, interfaceIdxArray[i],
                             classDescriptor);
                    goto bail;
                }

                /* are we allowed to implement this interface? */
                if (!dvmCheckClassAccess(clazz, clazz->interfaces[i])) {
                    //dvmLinearReadOnly(clazz->classLoader, clazz->interfaces);
                    DVMTraceWar("Interface '%s' is not accessible to '%s'",
                         clazz->interfaces[i]->descriptor, clazz->descriptor);
                    //dvmThrowIllegalAccessError("interface not accessible");
                    goto bail;
                }
                DVMTraceInf("+++  found interface '%s'", clazz->interfaces[i]->descriptor);
            }
            //dvmLinearReadOnly(clazz->classLoader, clazz->interfaces);
        }
    }
    /*
     * There are now Class references visible to the GC in super and
     * interfaces.
     */

    /*
     * All classes have a direct superclass, except for
     * java/lang/Object and primitive classes. Primitive classes are
     * are created CLASS_INITIALIZED, so won't get here.
     */
    //assert(clazz->primitiveType == PRIM_NOT);
    if (CRTL_strcmp(clazz->descriptor, "Ljava/lang/Object;") == 0)
    {
        if (clazz->super != NULL) {
            /* TODO: is this invariant true for all java/lang/Objects,
             * regardless of the class loader?  For now, assume it is.
             */
            //dvmThrowClassFormatError("java.lang.Object has a superclass");
            goto bail;
        }

        /* Don't finalize objects whose classes use the
         * default (empty) Object.finalize().
         */
        CLEAR_CLASS_FLAG(clazz, CLASS_ISFINALIZABLE);
    } 
    else
    {
        if (clazz->super == NULL)
        {
            //dvmThrowLinkageError("no superclass defined");
            goto bail;
        }
        /* verify */
        if (dvmIsFinalClass(clazz->super)) {
            DVMTraceWar("Superclass of '%s' is final '%s'",
                clazz->descriptor, clazz->super->descriptor);
            //dvmThrowIncompatibleClassChangeError("superclass is final");
            goto bail;
        } else if (dvmIsInterfaceClass(clazz->super)) {
            DVMTraceWar("Superclass of '%s' is interface '%s'",
                clazz->descriptor, clazz->super->descriptor);
            //dvmThrowIncompatibleClassChangeError("superclass is an interface");
            goto bail;
        } else if (!dvmCheckClassAccess(clazz, clazz->super)) {
            DVMTraceWar("Superclass of '%s' (%s) is not accessible",
                clazz->descriptor, clazz->super->descriptor);
            //dvmThrowIllegalAccessError("superclass not accessible");
            goto bail;
        }

        /* Inherit finalizability from the superclass.  If this
         * class also overrides finalize(), its CLASS_ISFINALIZABLE
         * bit will already be set.
         */
        if (IS_CLASS_FLAG_SET(clazz->super, CLASS_ISFINALIZABLE)) {
            SET_CLASS_FLAG(clazz, CLASS_ISFINALIZABLE);
        }

        /* See if this class descends from java.lang.Reference
         * and set the class flags appropriately.
         */
        if (IS_CLASS_FLAG_SET(clazz->super, CLASS_ISREFERENCE)) {
            u4 superRefFlags;

            /* We've already determined the reference type of this
             * inheritance chain.  Inherit reference-ness from the superclass.
             */
            superRefFlags = GET_CLASS_FLAG_GROUP(clazz->super,
                    CLASS_ISREFERENCE |
                    CLASS_ISWEAKREFERENCE |
                    CLASS_ISFINALIZERREFERENCE |
                    CLASS_ISPHANTOMREFERENCE);
            SET_CLASS_FLAG(clazz, superRefFlags);
        } 
    }

    /*
     * Populate vtable.
     */
    if (dvmIsInterfaceClass(clazz)) {
        /* no vtable; just set the method indices */
        int count = clazz->virtualMethodCount;

        if (count != (u2) count) {
            DVMTraceErr("Too many methods (%d) in interface '%s'", count,
                 clazz->descriptor);
            goto bail;
        }

        //dvmLinearReadWrite(clazz->classLoader, clazz->virtualMethods);

        for (i = 0; i < count; i++)
            clazz->virtualMethods[i].methodIndex = (u2) i;

        //dvmLinearReadOnly(clazz->classLoader, clazz->virtualMethods);
    }
    else
    {
        if (!createVtable(clazz))
        {
            DVMTraceWar("failed creating vtable");
            goto bail;
        }
    }

    /*
     * Populate interface method tables.  Can alter the vtable.
     */
    if (!createIftable(clazz))
        goto bail;

    /*
     * Insert special-purpose "stub" method implementations.
     */
    if (!insertMethodStubs(clazz))
        goto bail;

    /*
     * Compute instance field offsets and, hence, the size of the object.
     */
    if (!computeFieldOffsets(clazz))
        goto bail;

#if 0
    /*
     * Cache field and method info for the class Reference (as loaded
     * by the boot classloader). This has to happen after the call to
     * computeFieldOffsets().
     */
    if ((clazz->classLoader == NULL) && (strcmp(clazz->descriptor, "Ljava/lang/ref/Reference;") == 0))
    {
        if (!precacheReferenceOffsets(clazz))
        {
            DVMTraceErr("failed pre-caching Reference offsets");
            //dvmThrowInternalError(NULL);
            goto bail;
        }
    }


    /*
     * Compact the offsets the GC has to examine into a bitmap, if
     * possible.  (This has to happen after Reference.referent is
     * massaged in precacheReferenceOffsets.)
     */
    computeRefOffsets(clazz);
#endif
    /*
     * Done!
     */
    if (IS_CLASS_FLAG_SET(clazz, CLASS_ISPREVERIFIED))
        clazz->status = CLASS_VERIFIED;
    else
        clazz->status = CLASS_RESOLVED;
    okay = TRUE;

bail:
    if (!okay)
    {
        clazz->status = CLASS_ERROR;
        #if 0
        if (!dvmCheckException(dvmThreadSelf())) {
            dvmThrowVirtualMachineError(NULL);
        }
        //TODO: how to throw exception?
        #endif
    }
    if (interfaceIdxArray != NULL) {
        CRTL_free(interfaceIdxArray);
    }

    return okay;
#else
    return FALSE;
#endif
}


/*
 * Find the named class (by descriptor). If it's not already loaded,
 * we load it and link it, but don't execute <clinit>. (The VM has
 * specific limitations on which events can cause initialization.)
 *
 * If "pDexFile" is NULL, we will search the bootclasspath for an entry.
 *
 * On failure, this returns NULL with an exception raised.
 *
 * TODO: we need to return an indication of whether we loaded the class or
 * used an existing definition.  If somebody deliberately tries to load a
 * class twice in the same class loader, they should get a LinkageError,
 * but inadvertent simultaneous class references should "just work".
 */
static ClassObject* findClassNoInit(const char* descriptor, DvmDex* pDvmDex)
{
    ClassObject* clazz;
    DvmDex* lpDvmDex;

    clazz = dvmLookupClass(descriptor);
    if (clazz == NULL)
    {
        const DexClassDef* pClassDef = NULL;

        if (pDvmDex == NULL)
        {
            /*
             * Search for bootstrap classes which are always stored
             * in the first element of pClsEntry array.
             */
            lpDvmDex = gDvm.pClsEntry[0].pDvmDex;
        }
        else
        {
            lpDvmDex = pDvmDex;
        }

        pClassDef = dexFindClass(lpDvmDex->pDexFile, descriptor);

        if (pClassDef == NULL)
        {
            //TODO: should throw exception here?
            DVMTraceErr("findClassNoInit - doesn't find dvm class\n");
            goto bail;
        }

        /* found a match, try to load it */
        clazz = loadClassFromDex(lpDvmDex, pClassDef);
        
        if (clazz == NULL || !dvmAddClassToHash(clazz))
        {
            DVMTraceErr("Add Class to Hash failure? \n");
            goto bail;
        }

        /*
         * Prepare and resolve.
         */
        if (!dvmLinkClass(clazz)) {
            clazz->status = CLASS_ERROR;

            /* Let any waiters know.
             */
            clazz->initThreadId = 0;
            clazz = NULL;
            goto bail;
        }

        /*
         * Add class stats to global counters.
         *
         * TODO: these should probably be atomic ops.
         */
        gDvm.numLoadedClasses++;
        gDvm.numDeclaredMethods +=
            clazz->virtualMethodCount + clazz->directMethodCount;
        gDvm.numDeclaredInstFields += clazz->ifieldCount;
        gDvm.numDeclaredStaticFields += clazz->sfieldCount;

        /*
         * Cache pointers to basic classes.  We want to use these in
         * various places, and it's easiest to initialize them on first
         * use rather than trying to force them to initialize (startup
         * ordering makes it weird).
         */
        if (gDvm.classJavaLangObject == NULL &&
            CRTL_strcmp(descriptor, "Ljava/lang/Object;") == 0)
        {
            /* It should be impossible to get here with anything
             * but the bootclasspath loader.
             */
            //assert(loader == NULL);
            gDvm.classJavaLangObject = clazz;
        }

    }
    else
    {
        if (clazz->status == CLASS_ERROR)
        {
            /*
             * Somebody else tried to load this and failed.  We need to raise
             * an exception and report failure.
             */
            //throwEarlierClassFailure(clazz);
            //TODO: throw exception.
            clazz = NULL;
            goto bail;
        }
    }

bail:
    return clazz;
}


/*
 * (This is a dvmHashTableLookup callback.)
 *
 * Entries in the class hash table are stored as { descriptor, d-loader }
 * tuples.  If the hashed class descriptor matches the requested descriptor,
 * and the hashed defining class loader matches the requested class
 * loader, we're good.  If only the descriptor matches, we check to see if the
 * loader is in the hashed class' initiating loader list.  If so, we
 * can return "true" immediately and skip some of the loadClass melodrama.
 *
 * The caller must lock the hash table before calling here.
 *
 * Returns 0 if a matching entry is found, nonzero otherwise.
 */
static int hashcmpClassByCrit(const void* vclazz, const void* vcrit)
{
#if 1
    const ClassObject* clazz = (const ClassObject*) vclazz;
    const ClassMatchCriteria* pCrit = (const ClassMatchCriteria*) vcrit;
    bool_t match;

    match = (CRTL_strcmp(clazz->descriptor, pCrit->descriptor) == 0);

    return !match;
#else
    return 0;
#endif
}


/*
 * Search through the hash table to find an entry with a matching descriptor
 * and an initiating class loader that matches "loader".
 *
 * The table entries are hashed on descriptor only, because they're unique
 * on *defining* class loader, not *initiating* class loader.  This isn't
 * great, because it guarantees we will have to probe when multiple
 * class loaders are used.
 *
 * Note this does NOT try to load a class; it just finds a class that
 * has already been loaded.
 *
 * If "unprepOkay" is set, this will return classes that have been added
 * to the hash table but are not yet fully loaded and linked.  Otherwise,
 * such classes are ignored.  (The only place that should set "unprepOkay"
 * is findClassNoInit(), which will wait for the prep to finish.)
 *
 * Returns NULL if not found.
 */
ClassObject* dvmLookupClass(const char* descriptor)
{
#if 1
    ClassMatchCriteria crit;
    void*    found;
    uint32_t hash;

    hash = dvmComputeUtf8Hash(descriptor);

    crit.descriptor = descriptor;

    DVMTraceInf(" dvmLookupClass searching for '%s'", descriptor);

    found = dvmHashTableLookup(gDvm.loadedClasses, hash, &crit, hashcmpClassByCrit, FALSE);
    /*
     * The class has been added to the hash table but isn't ready for use.
     * We're going to act like we didn't see it, so that the caller will
     * go through the full "find class" path, which includes locking the
     * object and waiting until it's ready.  We could do that lock/wait
     * here, but this is an extremely rare case, and it's simpler to have
     * the wait-for-class code centralized.
     */
    if (found && !dvmIsClassLinked((ClassObject*)found)) {
        DVMTraceInf("Ignoring not-yet-ready %s, using slow path",
            ((ClassObject*)found)->descriptor);
        found = NULL;
    }

    return (ClassObject*) found;
#else
    return 0;
#endif
}

/*
 * Find the named class (by descriptor), searching for it in the
 * bootclasspath.
 *
 * On failure, this returns NULL with an exception raised.
 */
ClassObject* dvmFindSystemClassNoInit(const char* descriptor)
{
    return findClassNoInit(descriptor, NULL);
}

/*
 * Validate the descriptors in the superclass and interfaces.
 *
 * What we need to do is ensure that the classes named in the method
 * descriptors in our ancestors and ourselves resolve to the same class
 * objects.  We can get conflicts when the classes come from different
 * class loaders, and the resolver comes up with different results for
 * the same class name in different contexts.
 *
 * An easy way to cause the problem is to declare a base class that uses
 * class Foo in a method signature (e.g. as the return type).  Then,
 * define a subclass and a different version of Foo, and load them from a
 * different class loader.  If the subclass overrides the method, it will
 * have a different concept of what Foo is than its parent does, so even
 * though the method signature strings are identical, they actually mean
 * different things.
 *
 * A call to the method through a base-class reference would be treated
 * differently than a call to the method through a subclass reference, which
 * isn't the way polymorphism works, so we have to reject the subclass.
 * If the subclass doesn't override the base method, then there's no
 * problem, because calls through base-class references and subclass
 * references end up in the same place.
 *
 * We don't need to check to see if an interface's methods match with its
 * superinterface's methods, because you can't instantiate an interface
 * and do something inappropriate with it.  If interface I1 extends I2
 * and is implemented by C, and I1 and I2 are in separate class loaders
 * and have conflicting views of other classes, we will catch the conflict
 * when we process C.  Anything that implements I1 is doomed to failure,
 * but we don't need to catch that while processing I1.
 *
 * On failure, throws an exception and returns "false".
 */
static bool_t validateSuperDescriptors(const ClassObject* clazz)
{
#if 0
    int i;

    if (dvmIsInterfaceClass(clazz))
    {
        return TRUE;
    }

    /*
     * Start with the superclass-declared methods.
     */
    if (clazz->super != NULL)
    {
        /*
         * Walk through every overridden method and compare resolved
         * descriptor components.  We pull the Method structs out of
         * the vtable.  It doesn't matter whether we get the struct from
         * the parent or child, since we just need the UTF-8 descriptor,
         * which must match.
         *
         * We need to do this even for the stuff inherited from Object,
         * because it's possible that the new class loader has redefined
         * a basic class like String.
         *
         * We don't need to check stuff defined in a superclass because
         * it was checked when the superclass was loaded.
         */
        const Method* meth;

        //printf("Checking %s %p vs %s %p\n",
        //    clazz->descriptor, clazz->classLoader,
        //    clazz->super->descriptor, clazz->super->classLoader);
        for (i = clazz->super->vtableCount - 1; i >= 0; i--)
        {
            meth = clazz->vtable[i];
            if (meth != clazz->super->vtable[i] &&
                !checkMethodDescriptorClasses(meth, clazz->super, clazz))
            {
                DVMTraceErr("Method mismatch: %s in %s (cl=%p) and super %s (cl=%p)",
                    meth->name, clazz->descriptor, clazz->classLoader,
                    clazz->super->descriptor, clazz->super->classLoader);
                dvmThrowLinkageError(
                    "Classes resolve differently in superclass");
                return false;
            }
        }
    }

    /*
     * Check the methods defined by this class against the interfaces it
     * implements.  If we inherited the implementation from a superclass,
     * we have to check it against the superclass (which might be in a
     * different class loader).  If the superclass also implements the
     * interface, we could skip the check since by definition it was
     * performed when the class was loaded.
     */
    for (i = 0; i < clazz->iftableCount; i++) {
        const InterfaceEntry* iftable = &clazz->iftable[i];

        if (clazz->classLoader != iftable->clazz->classLoader) {
            const ClassObject* iface = iftable->clazz;
            int j;

            for (j = 0; j < iface->virtualMethodCount; j++) {
                const Method* meth;
                int vtableIndex;

                vtableIndex = iftable->methodIndexArray[j];
                meth = clazz->vtable[vtableIndex];

                if (!checkMethodDescriptorClasses(meth, iface, meth->clazz)) {
                    LOGW("Method mismatch: %s in %s (cl=%p) and "
                            "iface %s (cl=%p)",
                        meth->name, clazz->descriptor, clazz->classLoader,
                        iface->descriptor, iface->classLoader);
                    dvmThrowLinkageError(
                        "Classes resolve differently in interface");
                    return false;
                }
            }
        }
    }

    return TRUE;
#else
    return TRUE;
#endif
}


/*
 * Initialize any static fields whose values are stored in
 * the DEX file.  This must be done during class initialization.
 */
static void initSFields(ClassObject* clazz)
{
#if 1
    DexFile* pDexFile;
    const DexClassDef* pClassDef;
    const DexEncodedArray* pValueList;
    EncodedArrayIterator iterator;
    int i;

    if (clazz->sfieldCount == 0) {
        return;
    }
    if (clazz->pDvmDex == NULL) {
        /* generated class; any static fields should already be set up */
        DVMTraceErr("Not initializing static fields in %s", clazz->descriptor);
        return;
    }
    pDexFile = clazz->pDvmDex->pDexFile;

    pClassDef = dexFindClass(pDexFile, clazz->descriptor);
    //assert(pClassDef != NULL);

    pValueList = dexGetStaticValuesList(pDexFile, pClassDef);
    if (pValueList == NULL) {
        return;
    }

    dvmEncodedArrayIteratorInitialize(&iterator, pValueList, clazz);

    /*
     * Iterate over the initial values array, setting the corresponding
     * static field for each array element.
     */

    for (i = 0; dvmEncodedArrayIteratorHasNext(&iterator); i++) {
        AnnotationValue value;
        bool_t parsed = dvmEncodedArrayIteratorGetNext(&iterator, &value);
        StaticField* sfield = &clazz->sfields[i];
        const char* descriptor = sfield->field.signature;
        bool_t isObj = FALSE;

        if (! parsed) {
            /*
             * TODO: Eventually verification should attempt to ensure
             * that this can't happen at least due to a data integrity
             * problem.
             */
            DVMTraceErr("Static initializer parse failed for %s at index %d",
                    clazz->descriptor, i);
            //TODO: Abort VM here or ensure never go this place?
            //dvmAbort();
        }

        /* Verify that the value we got was of a valid type. */

        switch (descriptor[0]) {
            case 'Z': parsed = (value.type == kDexAnnotationBoolean); break;
            case 'B': parsed = (value.type == kDexAnnotationByte);    break;
            case 'C': parsed = (value.type == kDexAnnotationChar);    break;
            case 'S': parsed = (value.type == kDexAnnotationShort);   break;
            case 'I': parsed = (value.type == kDexAnnotationInt);     break;
            case 'J': parsed = (value.type == kDexAnnotationLong);    break;
            case 'F': parsed = (value.type == kDexAnnotationFloat);   break;
            case 'D': parsed = (value.type == kDexAnnotationDouble);  break;
            case '[': parsed = (value.type == kDexAnnotationNull);    break;
            case 'L': {
                switch (value.type) {
                    case kDexAnnotationNull: {
                        /* No need for further tests. */
                        break;
                    }
                    case kDexAnnotationString: {
                        parsed =
                            (CRTL_strcmp(descriptor, "Ljava/lang/String;") == 0);
                        isObj = TRUE;
                        break;
                    }
                    case kDexAnnotationType: {
                        parsed =
                            (CRTL_strcmp(descriptor, "Ljava/lang/Class;") == 0);
                        isObj = TRUE;
                        break;
                    }
                    default: {
                        parsed = FALSE;
                        break;
                    }
                }
                break;
            }
            default: {
                parsed = FALSE;
                break;
            }
        }

        if (parsed) {
            /*
             * All's well, so store the value.
             */
            if (isObj) {
                dvmSetStaticFieldObject(sfield, (Object*)value.value.l);
                //dvmReleaseTrackedAlloc((Object*)value.value.l, self);
            } else {
                /*
                 * Note: This always stores the full width of a
                 * JValue, even though most of the time only the first
                 * word is needed.
                 */
                sfield->value = value.value;
            }
        } else {
            /*
             * Something up above had a problem. TODO: See comment
             * above the switch about verfication.
             */
            DVMTraceErr("Bogus static initialization: value type %d in field type "
                    "%s for %s at index %d",
                value.type, descriptor, clazz->descriptor, i);
            //TODO: Abort VM here or ensure never go this place?
            //dvmAbort();
        }
    }
#else

#endif
}


/*
 * If a class has not been initialized, do so by executing the code in
 * <clinit>.  The sequence is described in the VM spec v2 2.17.5.
 *
 * It is possible for multiple threads to arrive here simultaneously, so
 * we need to lock the class while we check stuff.  We know that no
 * interpreted code has access to the class yet, so we can use the class's
 * monitor lock.
 *
 * We will often be called recursively, e.g. when the <clinit> code resolves
 * one of its fields, the field resolution will try to initialize the class.
 * In that case we will return "true" even though the class isn't actually
 * ready to go.  The ambiguity can be resolved with dvmIsClassInitializing().
 * (TODO: consider having this return an enum to avoid the extra call --
 * return -1 on failure, 0 on success, 1 on still-initializing.  Looks like
 * dvmIsClassInitializing() is always paired with *Initialized())
 *
 * This can get very interesting if a class has a static field initialized
 * to a new instance of itself.  <clinit> will end up calling <init> on
 * the members it is initializing, which is fine unless it uses the contents
 * of static fields to initialize instance fields.  This will leave the
 * static-referenced objects in a partially initialized state.  This is
 * reasonably rare and can sometimes be cured with proper field ordering.
 *
 * On failure, returns "false" with an exception raised.
 *
 * -----
 *
 * It is possible to cause a deadlock by having a situation like this:
 *   class A { static { sleep(10000); new B(); } }
 *   class B { static { sleep(10000); new A(); } }
 *   new Thread() { public void run() { new A(); } }.start();
 *   new Thread() { public void run() { new B(); } }.start();
 * This appears to be expected under the spec.
 *
 * The interesting question is what to do if somebody calls Thread.interrupt()
 * on one of the deadlocked threads.  According to the VM spec, they're both
 * sitting in "wait".  Should the interrupt code quietly raise the
 * "interrupted" flag, or should the "wait" return immediately with an
 * exception raised?
 *
 * This gets a little murky.  The VM spec says we call "wait", and the
 * spec for Thread.interrupt says Object.wait is interruptible.  So it
 * seems that, if we get unlucky and interrupt class initialization, we
 * are expected to throw (which gets converted to ExceptionInInitializerError
 * since InterruptedException is checked).
 *
 * There are a couple of problems here.  First, all threads are expected to
 * present a consistent view of class initialization, so we can't have it
 * fail in one thread and succeed in another.  Second, once a class fails
 * to initialize, it must *always* fail.  This means that a stray interrupt()
 * call could render a class unusable for the lifetime of the VM.
 *
 * In most cases -- the deadlock example above being a counter-example --
 * the interrupting thread can't tell whether the target thread handled
 * the initialization itself or had to wait while another thread did the
 * work.  Refusing to interrupt class initialization is, in most cases,
 * not something that a program can reliably detect.
 *
 * On the assumption that interrupting class initialization is highly
 * undesirable in most circumstances, and that failing to do so does not
 * deviate from the spec in a meaningful way, we don't allow class init
 * to be interrupted by Thread.interrupt().
 */
bool_t dvmInitClass(ClassObject* clazz)
{
#if 1
    u8 startWhen = 0;

    const Method* method;

    if (!(dvmIsClassLinked(clazz) || clazz->status == CLASS_ERROR))
    {
        DVMTraceErr("dvmInitClass - error: wrong class loading status\n");
        return FALSE;
    }

    /*
     * If the class hasn't been verified yet, do so now.
     */
    if (clazz->status < CLASS_VERIFIED)
    {
        /*
         * If we're in an "erroneous" state, throw an exception and bail.
         */
        if (clazz->status == CLASS_ERROR)
        {
            //throwEarlierClassFailure(clazz);
            //TODO: throw class loader failure error here?
            goto bail_unlock;
        }

        clazz->status = CLASS_VERIFYING;
        /**
         * Not really verify the class. Just keep the code for next use.

        if (!dvmVerifyClass(clazz)) {
            dvmThrowVerifyError(clazz->descriptor);
            dvmSetFieldObject((Object*) clazz,
                OFFSETOF_MEMBER(ClassObject, verifyErrorClass),
                (Object*) dvmGetException(self)->clazz);
            clazz->status = CLASS_ERROR;
            goto bail_unlock;
        }
        */
        clazz->status = CLASS_VERIFIED;
    }

    if (clazz->status == CLASS_INITIALIZED)
        goto bail_unlock;

    /* see if we failed previously */
    if (clazz->status == CLASS_ERROR) {
        // might be wise to unlock before throwing; depends on which class
        // it is that we have locked
        
        //throwEarlierClassFailure(clazz);
        //TODO: throw exception here?
        return FALSE;
    }

    /*
     * We're ready to go, and have exclusive access to the class.
     *
     * Before we start initialization, we need to do one extra bit of
     * validation: make sure that the methods declared here match up
     * with our superclass and interfaces.  We know that the UTF-8
     * descriptors match, but classes from different class loaders can
     * have the same name.
     *
     * We do this now, rather than at load/link time, for the same reason
     * that we defer verification.
     *
     * It's unfortunate that we need to do this at all, but we risk
     * mixing reference types with identical names (see Dalvik test 068).
     */
    if (!validateSuperDescriptors(clazz)) {
        //FIXME: I don't think we need to verfiy supper class descriptor at present.
        //this API returns TRUE directly to skip this step.
        //assert(dvmCheckException(self));
        clazz->status = CLASS_ERROR;
        goto bail_unlock;
    }

    /*
     * Let's initialize this thing.
     *
     * We unlock the object so that other threads can politely sleep on
     * our mutex with Object.wait(), instead of hanging or spinning trying
     * to grab our mutex.
     */
    //assert(clazz->status < CLASS_INITIALIZING);

    /* order matters here, esp. interaction with dvmIsClassInitializing */
    //clazz->initThreadId = self->threadId;

    /* init our superclass */
    if (clazz->super != NULL && clazz->super->status != CLASS_INITIALIZED)
    {
        //assert(!dvmIsInterfaceClass(clazz));
        if (!dvmInitClass(clazz->super))
        {
            //assert(dvmCheckException(self));
            clazz->status = CLASS_ERROR;
            /* wake up anybody who started waiting while we were unlocked */
            //dvmLockObject(self, (Object*) clazz);
            goto bail_notify;
        }
    }

    /* Initialize any static fields whose values are
     * stored in the Dex file.  This should include all of the
     * simple "final static" fields, which are required to
     * be initialized first. (vmspec 2 sec 2.17.5 item 8)
     * More-complicated final static fields should be set
     * at the beginning of <clinit>;  all we can do is trust
     * that the compiler did the right thing.
     */
    initSFields(clazz);

    /* Execute any static initialization code.
     */
    method = dvmFindDirectMethodByDescriptor(clazz, "<clinit>", "()V");
    if (method == NULL) {
        DVMTraceInf("No <clinit> found for %s", clazz->descriptor);
    } else {
        //JValue unused;
        DVMTraceInf("Invoking %s.<clinit>", clazz->descriptor);
        //dvmCallMethod(self, method, NULL, &unused);
        //***************************************************
        //TODO: how to call <clinit> for static code area.
        //***************************************************
    }

    if (FALSE /*dvmCheckException(self)*/ /* TODO: without check exception at present? */) {
        /*
         * We've had an exception thrown during static initialization.  We
         * need to throw an ExceptionInInitializerError, but we want to
         * tuck the original exception into the "cause" field.
         */
        //DVMTraceWar("Exception %s thrown while initializing %s",
        //    (dvmGetException(self)->clazz)->descriptor, clazz->descriptor);
        //dvmThrowExceptionInInitializerError();
        //LOGW("+++ replaced");

        //dvmLockObject(self, (Object*) clazz);
        clazz->status = CLASS_ERROR;
    } else {
        /* success! */
        //dvmLockObject(self, (Object*) clazz);
        clazz->status = CLASS_INITIALIZED;
        DVMTraceInf("Initialized class: %s", clazz->descriptor);

#if 0
        /*
         * Update alloc counters.  TODO: guard with mutex.
         */
        if (gDvm.allocProf.enabled && startWhen != 0) {
            u8 initDuration = dvmGetRelativeTimeNsec() - startWhen;
            gDvm.allocProf.classInitTime += initDuration;
            self->allocProf.classInitTime += initDuration;
            gDvm.allocProf.classInitCount++;
            self->allocProf.classInitCount++;
        }
#endif
    }

bail_notify:
    /*
     * Notify anybody waiting on the object.
     */
    //dvmObjectNotifyAll(self, (Object*) clazz);

bail_unlock:

    //dvmUnlockObject(self, (Object*) clazz);

    return (clazz->status != CLASS_ERROR);
#else
    return FALSE;
#endif
}


/*
 * Find the named class (by descriptor), using the specified
 * initiating ClassLoader.
 *
 * The class will be loaded if it has not already been, as will its
 * superclass.  It will not be initialized.
 *
 * If the class can't be found, returns NULL with an appropriate exception
 * raised.
 */
ClassObject* dvmFindClassNoInit(const char* descriptor)
{
    DVMTraceErr("FindClassNoInit '%s'", descriptor);

    if (*descriptor == '[') {
        /*
         * Array class.  Find in table, generate if not found.
         */
        return dvmFindArrayClass(descriptor);
    } else {
        /*
         * Regular class.  Find in table, load if not found.
         */
        return dvmFindSystemClassNoInit(descriptor);
    }
}


/*
 * Find the named class (by descriptor), using the specified
 * initiating ClassLoader.
 *
 * The class will be loaded and initialized if it has not already been.
 * If necessary, the superclass will be loaded.
 *
 * If the class can't be found, returns NULL with an appropriate exception
 * raised.
 */
ClassObject* dvmFindClass(const char* descriptor)
{
    ClassObject* clazz = NULL;

    clazz = dvmFindClassNoInit(descriptor);
    if (clazz != NULL && clazz->status < CLASS_INITIALIZED)
    {
        /* initialize class */
        if (!dvmInitClass(clazz))
        {
            /* init failed; leave it in the list, marked as bad */
            //assert(dvmCheckException(dvmThreadSelf()));
            //assert(clazz->status == CLASS_ERROR);
            DVMTraceErr("dvmFindClass: class initial error\n");
            return NULL;
        }
    }
    return clazz;
}


/*
 * ===========================================================================
 *      Method Prototypes and Descriptors
 * ===========================================================================
 */

/*
 * Store a copy of the method prototype descriptor string
 * for the given method into the given DexStringCache, returning the
 * stored string for convenience.
 */
char* dvmCopyDescriptorStringFromMethod(const Method* method,
        DexStringCache *pCache)
{
    const char* result =
        dexProtoGetMethodDescriptor(&method->prototype, pCache);
    return dexStringCacheEnsureCopy(pCache, result);
}

/*
 * Compute the number of argument words (u4 units) required by the
 * given method's prototype. For example, if the method descriptor is
 * "(IJ)D", this would return 3 (one for the int, two for the long;
 * return value isn't relevant).
 */
int dvmComputeMethodArgsSize(const Method* method)
{
    return dexProtoComputeArgsSize(&method->prototype);
}

/*
 * Compare the two method prototypes. The two prototypes are compared
 * as if by strcmp() on the result of dexProtoGetMethodDescriptor().
 */
int dvmCompareMethodProtos(const Method* method1, const Method* method2)
{
    return dexProtoCompare(&method1->prototype, &method2->prototype);
}

/*
 * Compare the two method prototypes, considering only the parameters
 * (i.e. ignoring the return types). The two prototypes are compared
 * as if by strcmp() on the result of dexProtoGetMethodDescriptor().
 */
int dvmCompareMethodParameterProtos(const Method* method1, const Method* method2)
{
    return dexProtoCompareParameters(&method1->prototype, &method2->prototype);
}

/*
 * Compare a method descriptor string with the prototype of a method,
 * as if by converting the descriptor to a DexProto and comparing it
 * with dexProtoCompare().
 */
int dvmCompareDescriptorAndMethodProto(const char* descriptor, const Method* method)
{
    // Sense is reversed.
    return -dexProtoCompareToDescriptor(&method->prototype, descriptor);
}


/*
 * Compare the two method names and prototypes, a la strcmp(). The
 * name is considered the "major" order and the prototype the "minor"
 * order. The prototypes are compared as if by dvmCompareMethodProtos().
 */
int dvmCompareMethodNamesAndProtos(const Method* method1, const Method* method2)
{
    int result = CRTL_strcmp(method1->name, method2->name);

    if (result != 0) {
        return result;
    }

    return dvmCompareMethodProtos(method1, method2);
}

/*
 * Compare the two method names and prototypes, a la strcmp(), ignoring
 * the return value. The name is considered the "major" order and the
 * prototype the "minor" order. The prototypes are compared as if by
 * dvmCompareMethodArgProtos().
 */
int dvmCompareMethodNamesAndParameterProtos(const Method* method1, const Method* method2)
{
    int result = CRTL_strcmp(method1->name, method2->name);

    if (result != 0) {
        return result;
    }

    return dvmCompareMethodParameterProtos(method1, method2);
}

/*
 * Compare a (name, prototype) pair with the (name, prototype) of
 * a method, a la strcmp(). The name is considered the "major" order and
 * the prototype the "minor" order. The descriptor and prototype are
 * compared as if by dvmCompareDescriptorAndMethodProto().
 */
int dvmCompareNameProtoAndMethod(const char* name,
    const DexProto* proto, const Method* method)
{
    int result = CRTL_strcmp(name, method->name);

    if (result != 0) {
        return result;
    }

    return dexProtoCompare(proto, &method->prototype);
}

/*
 * Compare a (name, method descriptor) pair with the (name, prototype) of
 * a method, a la strcmp(). The name is considered the "major" order and
 * the prototype the "minor" order. The descriptor and prototype are
 * compared as if by dvmCompareDescriptorAndMethodProto().
 */
int dvmCompareNameDescriptorAndMethod(const char* name,
    const char* descriptor, const Method* method)
{
    int result = CRTL_strcmp(name, method->name);

    if (result != 0) {
        return result;
    }

    return dvmCompareDescriptorAndMethodProto(descriptor, method);
}



