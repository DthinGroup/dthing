#include "vm_common.h"
#include "interpApi.h"

#ifdef DVM_LOG
#undef DVM_LOG
#endif

#define DVM_LOG		printf

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

Method* dvmFindInterfaceMethodInCache(ClassObject* thisClass,
    u4 methodIdx, const Method* method, DvmDex* methodClassDex)
{
	DVM_LOG(">>>call dvmFindInterfaceMethodInCache!\n");
    return NULL;
}

void dvmThrowNoSuchMethodError(const char* msg) 
{
	DVM_LOG(">>>call dvmThrowNoSuchMethodError!\n");
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

INLINE Field* dvmDexGetResolvedField(const DvmDex* pDvmDex,
    u4 fieldIdx)
{
	DVM_LOG(">>>call dvmDexGetResolvedField!\n");
    return NULL;
}
                      
void dvmThrowArrayStoreExceptionIncompatibleElement(ClassObject* objectType,
        ClassObject* arrayType)
{
	DVM_LOG(">>>call dvmThrowArrayStoreExceptionIncompatibleElement!\n");
}

void dvmThrowArrayIndexOutOfBoundsException(int length, int index)
{
	DVM_LOG(">>>call dvmThrowArrayIndexOutOfBoundsException!\n");
}

s4 dvmInterpHandleSparseSwitch(const u2* switchData, s4 testVal)
{
	DVM_LOG(">>>call dvmInterpHandleSparseSwitch!\n");
    return -1;
}

s4 dvmInterpHandlePackedSwitch(const u2* switchData, s4 testVal)
{
	DVM_LOG(">>>call dvmInterpHandlePackedSwitch!\n");
    return -1;
}

vbool dvmCheckSuspendPending(Thread* self)
{
	DVM_LOG(">>>call dvmCheckSuspendPending!\n");
    return FALSE;
}

INLINE vbool dvmCheckSuspendQuick(Thread* self) 
{
	DVM_LOG(">>>call dvmCheckSuspendQuick!\n");
    return 0;
}

INLINE void dvmSetException(Thread* self, Object* exception)
{
	DVM_LOG(">>>call dvmSetException!\n");
}

vbool dvmInterpHandleFillArrayData(ArrayObject* arrayObj, const u2* arrayData)
{
	DVM_LOG(">>>call dvmInterpHandleFillArrayData!\n");
    return 0;
}

void dvmThrowInternalError(const char* msg) 
{
	DVM_LOG(">>>call dvmThrowInternalError!\n");
}

void dvmThrowNegativeArraySizeException(s4 size) 
{
	DVM_LOG(">>>call dvmThrowNegativeArraySizeException!\n");
}

void dvmThrowClassCastException(ClassObject* actual, ClassObject* desired)
{
	DVM_LOG(">>>call dvmThrowClassCastException!\n");
}

INLINE vbool dvmCheckException(Thread* self) 
{
	DVM_LOG(">>>call dvmCheckException!\n");
    return 0;
}

vbool dvmUnlockObject(Thread* self, Object *obj)
{
	DVM_LOG(">>>call dvmUnlockObject!\n");
    return 0;
}

void dvmLockObject(Thread* self, Object *obj)
{
	DVM_LOG(">>>call dvmLockObject!\n");
}

INLINE StringObject* dvmDexGetResolvedString(const DvmDex* pDvmDex,
    u4 stringIdx)
{
	DVM_LOG(">>>call dvmDexGetResolvedString!\n");
    return NULL;
}
    
INLINE void dvmClearException(Thread* self) 
{
	DVM_LOG(">>>call dvmClearException!\n");
}

void dvmCheckBefore(const u2 *pc, u4 *fp, Thread* self)
{
	DVM_LOG(">>>call dvmCheckBefore!\n");
}

void dvmThrowNullPointerException(const char* msg) 
{
	DVM_LOG(">>>call dvmThrowNullPointerException!\n");
}
