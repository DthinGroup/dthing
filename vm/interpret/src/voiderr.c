#include "vm_common.h"
#include "interpApi.h"


void dvmPopJniLocals(Thread* self, StackSaveArea* saveArea)
{

}

void dvmReportPostNativeInvoke(const Method* methodToCall, Thread* self, u4* fp)
{
}

void dvmReportPreNativeInvoke(const Method* methodToCall, Thread* self, u4* fp)
{}

INLINE vbool dvmIsNativeMethod(const Method* method) 
{
    return 0;
}
void dvmReportInvoke(Thread* self, const Method* methodToCall)
{
}

void dvmHandleStackOverflow(Thread* self, const Method* method)
{
}

void dvmReleaseTrackedAlloc(Object* obj, Thread* self)
{}

void dvmCleanupStackOverflow(Thread* self, const Object* exception)
{}

int dvmFindCatchBlock(Thread* self, int relPc, Object* exception,
    vbool scanOnly, void** newFrame)
{
    return 0;
}
    
void dvmReportExceptionThrow(Thread* self, Object* exception)
{}

int dvmLineNumFromPC(const Method* method, u4 relPc)
{
    return 0;
}

void dvmAddTrackedAlloc(Object* obj, Thread* self)
{}

void dvmReportReturn(Thread* self)
{}

Method* dvmFindInterfaceMethodInCache(ClassObject* thisClass,
    u4 methodIdx, const Method* method, DvmDex* methodClassDex)
{
    return NULL;
}

void dvmThrowNoSuchMethodError(const char* msg) {
}

INLINE vbool dvmIsAbstractMethod(const Method* method) 
{
    return 0;
}

Method* dvmResolveMethod(const ClassObject* referrer, u4 methodIdx,
    MethodType methodType)
{
    return NULL;
}
    
INLINE Method* dvmDexGetResolvedMethod(const DvmDex* pDvmDex,
    u4 methodIdx)
{
    return NULL;
}

INLINE void dvmWriteBarrierArray(const ArrayObject *obj,
                                 size_t start, size_t end)
{
}

void dvmThrowRuntimeException(const char* msg) {
}

void dvmAbort()
{}

INLINE void dvmSetStaticFieldObjectVolatile(StaticField* sfield, Object* val) 
{
}

INLINE Object* dvmGetStaticFieldObjectVolatile(const StaticField* sfield) 
{
    return NULL;
}

INLINE void dvmSetFieldObjectVolatile(Object* obj, int offset, Object* val) 
{
}



INLINE Object* dvmGetException(Thread* self) 
{
    return self->exception;
}

void dvmSetFinalizable(Object *obj)
{}


INLINE vbool dvmPerformInlineOp4Std(u4 arg0, u4 arg1, u4 arg2, u4 arg3,
    JValue* pResult, int opIndex)
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

INLINE void dvmSetStaticFieldLongVolatile(StaticField* sfield, s8 val) 
{
}

INLINE s8 dvmGetStaticFieldLongVolatile(const StaticField* sfield) 
{
    return 0;
}

INLINE void dvmSetFieldLongVolatile(Object* obj, int offset, s8 val) {
}

INLINE s8 dvmGetFieldLongVolatile(const Object* obj, int offset) 
{
    return 0;
}

INLINE Object* dvmGetFieldObjectVolatile(const Object* obj, int offset) 
{
  return NULL; 
}

INLINE void dvmSetStaticFieldIntVolatile(StaticField* sfield, s4 val) 
{
}

INLINE s4 dvmGetStaticFieldIntVolatile(const StaticField* sfield) 
{
return 0;
}

INLINE void dvmSetFieldIntVolatile(Object* obj, int offset, s4 val) 
{
}

INLINE s4 dvmGetFieldIntVolatile(const Object* obj, int offset) 
{
return 0;
}


float	fmodf(float a, float b)
{
    return 0.0;
}

void dvmThrowArithmeticException(const char* msg) 
{
}

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

StaticField* dvmResolveStaticField(const ClassObject* referrer, u4 sfieldIdx)
{
    return NULL;
}

INLINE void dvmSetFieldObject(Object* obj, int offset, Object* val) 
{
}
INLINE void dvmSetFieldLong(Object* obj, int offset, s8 val) 
{
}
INLINE void dvmSetFieldInt(Object* obj, int offset, s4 val) 
{
}
INLINE Object* dvmGetFieldObject(const Object* obj, int offset) 
{
    return NULL;
}
INLINE s8 dvmGetFieldLong(const Object* obj, int offset) 
{
    return 0;
}
INLINE s4 dvmGetFieldInt(const Object* obj, int offset) 
{
    return 0;
}

InstField* dvmResolveInstField(const ClassObject* referrer, u4 ifieldIdx)
{
    return NULL;
}

INLINE Field* dvmDexGetResolvedField(const DvmDex* pDvmDex,
    u4 fieldIdx)
{
    return NULL;
}
    
INLINE void dvmSetObjectArrayElement(const ArrayObject* obj, int index,
                                     Object* val) 
{
}
                                     
void dvmThrowArrayStoreExceptionIncompatibleElement(ClassObject* objectType,
        ClassObject* arrayType)
{
}

vbool dvmCanPutArrayElement(const ClassObject* objectClass,
    const ClassObject* arrayClass)
{
}

void dvmThrowArrayIndexOutOfBoundsException(int length, int index)
{}

s4 dvmInterpHandleSparseSwitch(const u2* switchData, s4 testVal)
{}

s4 dvmInterpHandlePackedSwitch(const u2* switchData, s4 testVal)
{}

vbool dvmCheckSuspendPending(Thread* self)
{}

INLINE vbool dvmCheckSuspendQuick(Thread* self) 
{
    return 0;
}

INLINE void dvmSetException(Thread* self, Object* exception)
{
}

vbool dvmInterpHandleFillArrayData(ArrayObject* arrayObj, const u2* arrayData)
{
    return 0;
}

void dvmThrowInternalError(const char* msg) {
}

INLINE u4 dvmGetMethodInsnsSize(const Method* meth) 
{
    return 0;
}

ArrayObject* dvmAllocArrayByClass(ClassObject* arrayClass,
    size_t length, int allocFlags)
{
    return NULL;
}
    
INLINE vbool dvmIsArrayClass(const ClassObject* clazz)
{
    return 0;
}

void dvmThrowNegativeArraySizeException(s4 size) {
}

Object* dvmAllocObject(ClassObject* clazz, int flags)
{
    return NULL;
}

vbool dvmInitClass(ClassObject* clazz)
{
    return 0;
}

INLINE vbool dvmIsClassInitialized(const ClassObject* clazz) 
{
    return 0;
}

void dvmThrowClassCastException(ClassObject* actual, ClassObject* desired)
{}

INLINE int dvmInstanceof(const ClassObject* instance, const ClassObject* clazz)
{
    return 0;
}

INLINE vbool dvmCheckException(Thread* self) 
{
    return 0;
}

vbool dvmUnlockObject(Thread* self, Object *obj)
{
    return 0;
}

void dvmLockObject(Thread* self, Object *obj)
{}

ClassObject* dvmResolveClass(const ClassObject* referrer, u4 classIdx,
    vbool fromUnverifiedConstant)
{
    return NULL;
}
    
INLINE  ClassObject* dvmDexGetResolvedClass(const DvmDex* pDvmDex,
    u4 classIdx)
{
    return NULL;
}
    
StringObject* dvmResolveString(const ClassObject* referrer, u4 stringIdx)
{
    return NULL;
}

INLINE StringObject* dvmDexGetResolvedString(const DvmDex* pDvmDex,
    u4 stringIdx)
{
    return NULL;
}
    
INLINE void dvmClearException(Thread* self) {
}

void dvmCheckBefore(const u2 *pc, u4 *fp, Thread* self)
{}

void dvmThrowNullPointerException(const char* msg) {
}
