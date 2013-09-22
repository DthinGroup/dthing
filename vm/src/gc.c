/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2013/06/25 $
 * Last modified:	$Date: 2013/07/04 $
 * Version:         $ID: gc.c#1
 */

/**
 * garbage collect.
 */

#include <dthing.h>
#include <gc.h>
#include <heap.h>


static Object** tempRoots = NULL;
static Object** globalRoots = NULL;
static int32_t  gcStatus = GC_NONE;

bool_t dvmGCStartup()
{
    tempRoots = heapAllocPersistent(sizeof(Object*) * MAX_TEMP_ROOTS_NUM);
    globalRoots = heapAllocPersistent(sizeof(Object*) * MAX_GLOBAL_ROOTS_NUM);

    if (tempRoots == NULL || globalRoots == NULL)
    {
        DVMTraceErr("dvmGCStartup - Error, gc starup failure with reason no enough memory\n");
        return FALSE;
    }

    CRTL_memset(tempRoots, 0x0, sizeof(Object*) * MAX_TEMP_ROOTS_NUM);
    CRTL_memset(globalRoots, 0x0, sizeof(Object*) * MAX_GLOBAL_ROOTS_NUM);

    gcStatus = GC_NONE;
    return TRUE;
}


void dvmGCShutdown()
{
    tempRoots = NULL;
    globalRoots = NULL;
    gcStatus = GC_NONE;
}

/* Used to mark objects when recursing.  Recursion is done by moving
 * the finger across the bitmaps in address order and marking child
 * objects.  Any newly-marked objects whose addresses are lower than
 * the finger won't be visited by the bitmap scan, so those objects
 * need to be added to the mark stack.
 */
static void markObject(const Object *obj)
{
    if (obj == NULL)
    {
        DVMTraceWar("markObject - Warning: obj is NULL \n");
        return;
    }

    heapMarkObject((void*)obj);
}

/*
 * Scans instance fields.
 */
static void scanFields(const Object *obj)
{
#if 0
    //assert(obj != NULL);
    //assert(obj->clazz != NULL);
    //assert(ctx != NULL);
    if (obj->clazz->refOffsets != CLASS_WALK_SUPER) {
        unsigned int refOffsets = obj->clazz->refOffsets;
        while (refOffsets != 0) {
            size_t rshift = CLZ(refOffsets);
            size_t offset = CLASS_OFFSET_FROM_CLZ(rshift);
            Object *ref = dvmGetFieldObject(obj, offset);
            markObject(ref);
            refOffsets &= ~(CLASS_HIGH_BIT >> rshift);
        }
    }
    else 
    {
        for (ClassObject *clazz = obj->clazz;
             clazz != NULL;
             clazz = clazz->super) {
            InstField *field = clazz->ifields;
            for (int i = 0; i < clazz->ifieldRefCount; ++i, ++field) {
                void *addr = BYTE_OFFSET(obj, field->byteOffset);
                Object *ref = ((JValue *)addr)->l;
                markObject(ref);
            }
        }
    }
#else
    ClassObject* clazz;
    int          i;

    for (clazz = obj->clazz; clazz != NULL; clazz = clazz->super)
    {
        InstField *field = clazz->ifields;
        for (i = 0; i < clazz->ifieldRefCount; ++i, ++field)
        {
            void *addr = BYTE_OFFSET(obj, field->byteOffset);
            Object *ref = ((JValue *)addr)->l;
            markObject(ref);
        }
    }
#endif
}



/*
 * Scans the static fields of a class object.
 */
static void scanStaticFields(const ClassObject *clazz)
{
    int i;
    //assert(clazz != NULL);
    //assert(ctx != NULL);
    for (i = 0; i < clazz->sfieldCount; ++i) {
        char ch = clazz->sfields[i].field.signature[0];
        if (ch == '[' || ch == 'L') {
            Object *obj = clazz->sfields[i].value.l;
            markObject(obj);
        }
    }
}

/*
 * Visit the interfaces of a class object.
 */
static void scanInterfaces(const ClassObject *clazz)
{
    int i;
    //assert(clazz != NULL);
    //assert(ctx != NULL);
    for (i = 0; i < clazz->interfaceCount; ++i) {
        markObject((const Object *)clazz->interfaces[i]);
    }
}

/*
 * Scans the header, static field references, and interface
 * pointers of a class object.
 */
static void scanClassObject(const Object *obj)
{
    const ClassObject *asClass;
    //assert(obj != NULL);
    //assert(dvmIsClassObject(obj));
    //assert(ctx != NULL);
    markObject((const Object *)obj->clazz);
    asClass = (const ClassObject *)obj;
    if (IS_CLASS_FLAG_SET(asClass, CLASS_ISARRAY)) {
        markObject((const Object *)asClass->elementClass);
    }
    /* Do super and the interfaces contain Objects and not dex idx values? */
    if (asClass->status > CLASS_IDX) {
        markObject((const Object *)asClass->super);
    }
    //markObject((const Object *)asClass->classLoader);
    scanFields(obj);
    scanStaticFields(asClass);
    if (asClass->status > CLASS_IDX) {
        scanInterfaces(asClass);
    }
}

/*
 * Scans the header of all array objects.  If the array object is
 * specialized to a reference type, scans the array data as well.
 */
static void scanArrayObject(const Object *obj)
{
    size_t i;
    //assert(obj != NULL);
    //assert(obj->clazz != NULL);
    //assert(ctx != NULL);
    markObject((const Object *)obj->clazz);
    if (IS_CLASS_FLAG_SET(obj->clazz, CLASS_ISOBJECTARRAY)) {
        const ArrayObject *array = (const ArrayObject *)obj;
        const Object **contents = (const Object **)(void *)array->contents;
        for (i = 0; i < array->length; ++i) {
            markObject(contents[i]);
        }
    }
}


/*
 * Scans the header and field references of a data object.
 */
static void scanDataObject(const Object *obj)
{
#if 1
    //assert(obj != NULL);
    //assert(obj->clazz != NULL);
    //assert(ctx != NULL);
    markObject((const Object *)obj->clazz);
    scanFields(obj);
    //if (IS_CLASS_FLAG_SET(obj->clazz, CLASS_ISREFERENCE)) {
        //delayReferenceReferent((Object *)obj);
    //}
#else
#endif
}

/*
 * Scans an object reference.  Determines the type of the reference
 * and dispatches to a specialized scanning routine.
 */
static void scanObject(const Object *obj)
{
    //assert(obj != NULL);
    //assert(obj->clazz != NULL);
    if (obj->clazz == gDvm.classJavaLangClass) {
        scanClassObject(obj);
    } else if (IS_CLASS_FLAG_SET(obj->clazz, CLASS_ISARRAY)) {
        scanArrayObject(obj);
    } else {
        scanDataObject(obj);
    }
}


/**
 * Collects and compacts the Java heap and tries to return unused memory
 * blocks, depending on the mode.
 *
 * This API is none thread independent, so it only can be called in DVM
 * thread task. Other task call this API may causes some unexepect result.
 */
void dvmRunGC()
{
    int i;
    if (gcStatus == GC_INPROGRESS)
    {
        DVMTraceInf("dvmRunGC - GC is inprogress\n");
        return;
    }
    //enter GC process
    gcStatus = GC_INPROGRESS;

    //mark
    //scan temporary roots
    for (i = 0; i < MAX_TEMP_ROOTS_NUM; i++)
    {
        if (tempRoots[i] != NULL)
        {
            scanObject(tempRoots[i]);
        }
    }
    //scan global roots
    for (i = 0; i < MAX_GLOBAL_ROOTS_NUM; i++)
    {
        if (globalRoots[i] != NULL)
        {
            scanObject(globalRoots[i]);
        }
    }

    //TODO: scan thread stacks
    //...
    

    //sweep
    heapSweep();

    /* GC completed, reset status. */
    gcStatus = GC_DONE;
}


/*
 * Create an instance of the specified class.
 *
 * Returns NULL and throws an exception on failure.
 */
Object* dvmAllocObject(ClassObject* clazz, int flags)
{
    Object* newObj = NULL;

    //assert(clazz != NULL);
    //assert(dvmIsClassInitialized(clazz) || dvmIsClassInitializing(clazz));

    /* allocate on GC heap; memory is zeroed out */
    newObj = (Object*)heapAllocObject((int32_t)clazz->objectSize, flags);

    if (newObj == NULL) {
        //run GC to get more memory space.
        dvmRunGC();
    }

    /* allocate on GC heap again after GC; */
    newObj = (Object*)heapAllocObject((int32_t)clazz->objectSize, flags);

    if (newObj != NULL) {
        DVM_OBJECT_INIT(newObj, clazz);
        //dvmTrackAllocation(clazz, clazz->objectSize);   /* notify DDMS */
    }

    return newObj;
}

