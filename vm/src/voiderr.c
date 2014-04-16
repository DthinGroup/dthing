#include "vm_common.h"
#include "interpApi.h"
#include "dthread.h"
#include "Object.h"
#if defined(ARCH_ARM_SPD)
#include "crtl.h"
#endif

#ifdef DVM_LOG
#undef DVM_LOG
#endif

#define DVM_LOG		DVMTraceInf

INLINE void dvmWriteBarrierField(const Object *obj, void *addr)
{
	DVM_LOG(">>>call dvmWriteBarrierField!\n");
    //dvmMarkCard(obj);
}

void dvmPopJniLocals(Thread* self, StackSaveArea* saveArea)
{
	DVM_LOG(">>>call dvmPopJniLocals!\n");
}

void dvmReportPostNativeInvoke(const Method* methodToCall, Thread* self, u4* fp)
{
	DVM_LOG(">>>call dvmReportPostNativeInvoke!\n");
}

void dvmReportPreNativeInvoke(const Method* methodToCall, Thread* self, u4* fp)
{
	DVM_LOG(">>>call dvmReportPreNativeInvoke!\n");
}

#ifndef _TEST_ED_
INLINE vbool dvmIsNativeMethod(const Method* method) 
{
	DVM_LOG(">>>call dvmIsNativeMethod!\n");
    return 0;
}
#endif

void dvmReportInvoke(Thread* self, const Method* methodToCall)
{
	DVM_LOG(">>>call dvmReportInvoke!\n");
}

void dvmHandleStackOverflow(Thread* self, const Method* method)
{
	DVM_LOG(">>>call dvmHandleStackOverflow!\n");
}

void dvmReleaseTrackedAlloc(Object* obj, Thread* self)
{
	DVM_LOG(">>>call dvmReleaseTrackedAlloc!\n");
}

void dvmCleanupStackOverflow(Thread* self, const Object* exception)
{
	DVM_LOG(">>>call dvmCleanupStackOverflow!\n");
}

int dvmFindCatchBlock(Thread* self, int relPc, Object* exception,
    vbool scanOnly, void** newFrame)
{
	DVM_LOG(">>>call dvmFindCatchBlock!\n");
    return 0;
}
    
void dvmReportExceptionThrow(Thread* self, Object* exception)
{
	DVM_LOG(">>>call dvmReportExceptionThrow!\n");
}

int dvmLineNumFromPC(const Method* method, u4 relPc)
{
	DVM_LOG(">>>call dvmLineNumFromPC!\n");
    return 0;
}

void dvmAddTrackedAlloc(Object* obj, Thread* self)
{
	DVM_LOG(">>>call dvmAddTrackedAlloc!\n");
}

void dvmReportReturn(Thread* self)
{
	DVM_LOG(">>>call dvmReportReturn!\n");
}

void dvmThrowIncompatibleClassChangeError(const char* msg) 
{
//    dvmThrowException(gDvm.exIncompatibleClassChangeError, msg);
}

/*
 * Find the concrete method that corresponds to "methodIdx".  The code in
 * "method" is executing invoke-method with "thisClass" as its first argument.
 *
 * Returns NULL with an exception raised on failure.
 */
Method* dvmInterpFindInterfaceMethod(ClassObject* thisClass, u4 methodIdx,
    const Method* method, DvmDex* methodClassDex)
{
    Method* absMethod;
    Method* methodToCall;
    int i, vtableIndex;

    /*
     * Resolve the method.  This gives us the abstract method from the
     * interface class declaration.
     */
    absMethod = dvmDexGetResolvedMethod(methodClassDex, methodIdx);
    if (absMethod == NULL) 
	{
        absMethod = dvmResolveInterfaceMethod(method->clazz, methodIdx);
        if (absMethod == NULL) 
		{
            DVM_LOG("+ unknown method");
            return NULL;
        }
    }

    /* make sure absMethod->methodIndex means what we think it means */
    DVM_ASSERT(dvmIsAbstractMethod(absMethod));

    /*
     * Run through the "this" object's iftable.  Find the entry for
     * absMethod's class, then use absMethod->methodIndex to find
     * the method's entry.  The value there is the offset into our
     * vtable of the actual method to execute.
     *
     * The verifier does not guarantee that objects stored into
     * interface references actually implement the interface, so this
     * check cannot be eliminated.
     */
    for (i = 0; i < thisClass->iftableCount; i++) {
        if (thisClass->iftable[i].clazz == absMethod->clazz)
            break;
    }
    if (i == thisClass->iftableCount) 
	{
        /* impossible in verified DEX, need to check for it in unverified */
        dvmThrowIncompatibleClassChangeError("interface not implemented");
        return NULL;
    }

    DVM_ASSERT(absMethod->methodIndex < thisClass->iftable[i].clazz->virtualMethodCount);

    vtableIndex = thisClass->iftable[i].methodIndexArray[absMethod->methodIndex];
    DVM_ASSERT(vtableIndex >= 0 && vtableIndex < thisClass->vtableCount);
    methodToCall = thisClass->vtable[vtableIndex];

#if 0
    /* this can happen when there's a stale class file */
    if (dvmIsAbstractMethod(methodToCall)) {
        dvmThrowAbstractMethodError("interface method not implemented");
        return NULL;
    }
#else
	#if __NIX__
    DVM_ASSERT(!dvmIsAbstractMethod(methodToCall) || methodToCall->nativeFunc != NULL);
	#endif
#endif

    DVM_LOG("+++ interface=%s.%s concrete=%s.%s", absMethod->clazz->descriptor, absMethod->name,
        methodToCall->clazz->descriptor, methodToCall->name);
    DVM_ASSERT(methodToCall != NULL);

    return methodToCall;
}



Method* dvmFindInterfaceMethodInCache(ClassObject* thisClass,
    u4 methodIdx, const Method* method, DvmDex* methodClassDex)
{
	Method* mthd = NULL;
	DVM_LOG(">>>call dvmFindInterfaceMethodInCache!\n");
	
	mthd = dvmInterpFindInterfaceMethod(thisClass,methodIdx,method,methodClassDex);
    return mthd;
}

void dvmThrowNoSuchMethodError(const char* msg) 
{
	DVM_LOG(">>>call dvmThrowNoSuchMethodError!\n");
}

#ifndef _TEST_ED_
Method* dvmResolveMethod(const ClassObject* referrer, u4 methodIdx,
    MethodType methodType)
{
    return NULL;
}


INLINE vbool dvmIsAbstractMethod(const Method* method) 
{
    return 0;
}


    
INLINE Method* dvmDexGetResolvedMethod(const DvmDex* pDvmDex,
    u4 methodIdx)
{
    return NULL;
}
#endif
INLINE void dvmWriteBarrierArray(const ArrayObject *obj,
                                 size_t start, size_t end)
{
	DVM_LOG(">>>call dvmWriteBarrierArray!\n");
}


void dvmThrowRuntimeException(const char* msg) 
{
}
void dvmAbort()
{
	DVM_LOG(">>>call dvmAbort!\n");
	DVM_ASSERT(0);
}

void dvmSetFinalizable(Object *obj)
{
	DVM_LOG(">>>call dvmSetFinalizable!\n");
	DVM_ASSERT(0);
}



struct Object* dvmGetException(struct dthread* self) 
{
    return self->exception;
}





vbool dvmPerformInlineOp4Std(u4 arg0, u4 arg1, u4 arg2, u4 arg3,JValue* pResult, int opIndex)
{
    return 0;
}
vbool dvmPerformInlineOp4Dbg(u4 arg0, u4 arg1, u4 arg2, u4 arg3,
    JValue* pResult, int opIndex)
{
    return 0;
}

void dvmThrowVerificationError(const Method* method, int kind, int ref)
{}

u1 dvmGetOriginalOpcode(const u2* addr)
{
    return 0;
}


INLINE void dvmSetStaticFieldObjectVolatile(StaticField* sfield, Object* val) 
{
	Object** ptr = &(sfield->value.l);
    //ANDROID_MEMBAR_STORE();
    *ptr = val;
    //ANDROID_MEMBAR_FULL();
    if (val != NULL) 
	{
        dvmWriteBarrierField(sfield->field.clazz, &sfield->value.l);
    }
}

INLINE Object* dvmGetStaticFieldObjectVolatile(const StaticField* sfield) 
{
	Object* const* ptr = &(sfield->value.l);
    return (Object*)(*(int32_t*)ptr);   // android_atomic_acquire_load((int32_t*)ptr);
}

INLINE void dvmSetFieldObjectVolatile(Object* obj, int offset, Object* val) 
{
	Object** ptr = &((JValue*)BYTE_OFFSET(obj, offset))->l;
    //ANDROID_MEMBAR_STORE();
    *ptr = val;
    //ANDROID_MEMBAR_FULL();
    if (val != NULL) 
	{
        dvmWriteBarrierField(obj, ptr);
    }
}

INLINE Object* dvmGetFieldObjectVolatile(const Object* obj, int offset) 
{
	Object** ptr = &((JValue*)BYTE_OFFSET(obj, offset))->l;
    return (Object*)(*(int32_t*)ptr);   ///android_atomic_acquire_load((int32_t*)ptr);
}

INLINE void dvmSetStaticFieldIntVolatile(StaticField* sfield, s4 val) 
{
	s4* ptr = &sfield->value.i;
    //ANDROID_MEMBAR_STORE();
    *ptr = val;
    //ANDROID_MEMBAR_FULL();
}

INLINE s4 dvmGetStaticFieldIntVolatile(const StaticField* sfield) 
{
	const s4* ptr = &(sfield->value.i);
    return (s4)(*((s4*)ptr));   //android_atomic_acquire_load((s4*)ptr);
}

INLINE void dvmSetFieldIntVolatile(Object* obj, int offset, s4 val) 
{
	s4* ptr = &((JValue*)BYTE_OFFSET(obj, offset))->i;
    /*
     * TODO: add an android_atomic_synchronization_store() function and
     * use it in the 32-bit volatile set handlers.  On some platforms we
     * can use a fast atomic instruction and avoid the barriers.
     */
    //ANDROID_MEMBAR_STORE();
    *ptr = val;
    //ANDROID_MEMBAR_FULL();
}

INLINE s4 dvmGetFieldIntVolatile(const Object* obj, int offset) 
{
	s4* ptr = &((JValue*)BYTE_OFFSET(obj, offset))->i;
    return (s4)(*ptr);  //android_atomic_acquire_load(ptr);
}

float	fmodf(float a, float b)
{
    return 0.0;
}

void dvmThrowArithmeticException(const char* msg) 
{
}

#ifndef _TEST_ED_
INLINE void dvmSetStaticFieldObject(StaticField* sfield, Object* val) 
{
}
INLINE void dvmSetStaticFieldLong(StaticField* sfield, s8 val) 
{
}

INLINE void dvmSetStaticFieldInt(StaticField* sfield, s4 val) 
{
    sfield->value.i = val;
}

INLINE Object* dvmGetStaticFieldObject(const StaticField* sfield) 
{
    return sfield->value.l;
}

INLINE s8 dvmGetStaticFieldLong(const StaticField* sfield) 
{

    return sfield->value.j;
}

INLINE s4 dvmGetStaticFieldInt(const StaticField* sfield) 
{
    return sfield->value.i;
}



INLINE void dvmSetFieldObject(Object* obj, int offset, Object* val) 
{
	DVM_LOG(">>>call dvmSetFieldObject!\n");
}

INLINE void dvmSetFieldLong(Object* obj, int offset, s8 val) 
{
	DVM_LOG(">>>call dvmSetFieldLong!\n");
}

INLINE void dvmSetFieldInt(Object* obj, int offset, s4 val) 
{
	DVM_LOG(">>>call dvmSetFieldInt!\n");
}
INLINE Object* dvmGetFieldObject(const Object* obj, int offset) 
{
	DVM_LOG(">>>call dvmGetFieldObject!\n");
    return NULL;
}
INLINE s8 dvmGetFieldLong(const Object* obj, int offset) 
{
	DVM_LOG(">>>call dvmGetFieldLong!\n");
    return 0;
}
INLINE s4 dvmGetFieldInt(const Object* obj, int offset) 
{
	DVM_LOG(">>>call dvmGetFieldInt!\n");
    return 0;
}


InstField* dvmResolveInstField(const ClassObject* referrer, u4 ifieldIdx)
{
	DVM_LOG(">>>call dvmResolveInstField!\n");
    return NULL;
}


StaticField* dvmResolveStaticField(const ClassObject* referrer, u4 sfieldIdx)
{
	DVM_LOG(">>>call dvmResolveStaticField!\n");
    return NULL;
}
#endif
                           
void dvmThrowArrayStoreExceptionIncompatibleElement(ClassObject* objectType,
        ClassObject* arrayType)
{
	DVM_LOG(">>>call dvmThrowArrayStoreExceptionIncompatibleElement!\n");
}

void dvmThrowArrayIndexOutOfBoundsException(int length, int index)
{
	DVM_LOG(">>>call dvmThrowArrayIndexOutOfBoundsException!\n");
}

#if 0
s4 dvmInterpHandleSparseSwitch(const u2* switchData, s4 testVal)
{
	DVM_LOG(">>>call dvmInterpHandleSparseSwitch!\n");
	return 0;
}
#endif

vbool dvmCheckSuspendPending(Thread* self)
{
	DVM_LOG(">>>call dvmCheckSuspendPending!\n");
	return 0;
}

vbool dvmCheckSuspendQuick(struct dthread * self) 
{
	DVM_LOG(">>>call dvmCheckSuspendQuick!\n");
    return 0;
}

void dvmSetException(struct dthread* self, struct Object* exception)
{
	DVM_LOG(">>>call dvmSetException!\n");
}

void dvmThrowInternalError(const char* msg) 
{
	DVM_LOG(">>>call dvmThrowInternalError!\n");
}
#ifndef _TEST_ED_
INLINE u4 dvmGetMethodInsnsSize(const Method* meth) 
{
	DVM_LOG(">>>call dvmGetMethodInsnsSize!\n");
    return 0;
}

ArrayObject* dvmAllocArrayByClass(ClassObject* arrayClass,
    size_t length, int allocFlags)
{
	DVM_LOG(">>>call dvmAllocArrayByClass!\n");
    return NULL;
}
 
#endif 

void dvmThrowNegativeArraySizeException(s4 size) 
{
	DVM_LOG(">>>call dvmThrowNegativeArraySizeException!\n");
}

void dvmThrowClassCastException(ClassObject* actual, ClassObject* desired)
{
	DVM_LOG(">>>call dvmThrowClassCastException!\n");
}

vbool dvmUnlockObject(Thread* self, Object *obj)
{
	DVM_LOG(">>>call dvmUnlockObject!\n");
    return 1;
}

void dvmLockObject(Thread* self, Object *obj)
{
	DVM_LOG(">>>call dvmLockObject!\n");
}

void dvmClearException(struct dthread* self) 
{
	DVM_LOG(">>>call dvmClearException!\n");
	self->exception =NULL;//simplely handle
}

void dvmCheckBefore(const u2 *pc, u4 *fp, Thread* self)
{
	DVM_LOG(">>>call dvmCheckBefore!\n");
}

void dvmThrowIllegalArgumentException(char * str)
{

}

void dvmThrowIllegalMonitorStateException(char * str)
{

}

char * strdup (const char *s)  
{  
  size_t len = CRTL_strlen (s) + 1;  
  void *buff = CRTL_malloc (len);  
  if (buff == NULL)  
    return NULL;  
  return (char *) CRTL_memcpy(buff, s, len);  
}  