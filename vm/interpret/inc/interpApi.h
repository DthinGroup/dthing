#ifndef __INTERPAPI_H__
#define __INTERPAPI_H__

#include "vm_common.h"
#include "dthread.h"
#include "interpState.h"
#include "interpStack.h"

#define IS_CLASS_FLAG_SET(clazz, flag) \
    (((clazz)->accessFlags & (flag)) != 0)
/*
 * Portable interpreter.
 */
void dvmInterpretPortable(Thread * self);




/*API FUNCs of Interpret,avoid Errors*/

#ifdef ARCH_X86
	#define ANDROID_MEMBAR_STORE()  do{printf("WARNING xxx:not support mem barrier for NOW! \n");}while(0)
#elif defined ARCH_ARM
	#define ANDROID_MEMBAR_STORE()  do{__asm__ __volatile__ ("" : : : "memory");}while(0)
#endif

void dvmPopJniLocals(Thread* self, StackSaveArea* saveArea);

void dvmReportPostNativeInvoke(const Method* methodToCall, Thread* self, u4* fp);

void dvmReportPreNativeInvoke(const Method* methodToCall, Thread* self, u4* fp);

INLINE vbool dvmIsNativeMethod(const Method* method) ;

void dvmReportInvoke(Thread* self, const Method* methodToCall);

void dvmHandleStackOverflow(Thread* self, const Method* method);

void dvmReleaseTrackedAlloc(Object* obj, Thread* self);

void dvmCleanupStackOverflow(Thread* self, const Object* exception);

int dvmFindCatchBlock(Thread* self, int relPc, Object* exception,
    vbool scanOnly, void** newFrame);
    
void dvmReportExceptionThrow(Thread* self, Object* exception);

int dvmLineNumFromPC(const Method* method, u4 relPc);

void dvmAddTrackedAlloc(Object* obj, Thread* self);

void dvmReportReturn(Thread* self);

Method* dvmFindInterfaceMethodInCache(ClassObject* thisClass,
    u4 methodIdx, const Method* method, DvmDex* methodClassDex);

void dvmThrowNoSuchMethodError(const char* msg);

INLINE vbool dvmIsAbstractMethod(const Method* method) ;

Method* dvmResolveMethod(const ClassObject* referrer, u4 methodIdx,MethodType mType);
    
INLINE Method* dvmDexGetResolvedMethod(const DvmDex* pDvmDex,
    u4 methodIdx);

INLINE void dvmWriteBarrierArray(const ArrayObject *obj,
                                 size_t start, size_t end);

void dvmThrowRuntimeException(const char* msg) ;

void dvmAbort();

INLINE void dvmSetStaticFieldObjectVolatile(StaticField* sfield, Object* val) ;

INLINE Object* dvmGetStaticFieldObjectVolatile(const StaticField* sfield) ;

INLINE void dvmSetFieldObjectVolatile(Object* obj, int offset, Object* val);



INLINE Object* dvmGetException(Thread* self) ;

void dvmSetFinalizable(Object *obj);


INLINE vbool dvmPerformInlineOp4Std(u4 arg0, u4 arg1, u4 arg2, u4 arg3,
    JValue* pResult, int opIndex);

vbool dvmPerformInlineOp4Dbg(u4 arg0, u4 arg1, u4 arg2, u4 arg3,
    JValue* pResult, int opIndex);

void dvmThrowVerificationError(const Method* method, int kind, int ref);

u1 dvmGetOriginalOpcode(const u2* addr);

INLINE void dvmSetStaticFieldLongVolatile(StaticField* sfield, s8 val) ;

INLINE s8 dvmGetStaticFieldLongVolatile(const StaticField* sfield) ;

INLINE void dvmSetFieldLongVolatile(Object* obj, int offset, s8 val);

INLINE s8 dvmGetFieldLongVolatile(const Object* obj, int offset) ;

INLINE Object* dvmGetFieldObjectVolatile(const Object* obj, int offset) ;

INLINE void dvmSetStaticFieldIntVolatile(StaticField* sfield, s4 val) ;

INLINE s4 dvmGetStaticFieldIntVolatile(const StaticField* sfield) ;

INLINE void dvmSetFieldIntVolatile(Object* obj, int offset, s4 val);

INLINE s4 dvmGetFieldIntVolatile(const Object* obj, int offset) ;


float	fmodf(float, float);

void dvmThrowArithmeticException(const char* msg) ;

INLINE void dvmSetStaticFieldObject(StaticField* sfield, Object* val) ;

INLINE void dvmSetStaticFieldLong(StaticField* sfield, s8 val) ;

INLINE void dvmSetStaticFieldInt(StaticField* sfield, s4 val) ;

INLINE Object* dvmGetStaticFieldObject(const StaticField* sfield) ;

INLINE s8 dvmGetStaticFieldLong(const StaticField* sfield) ;

INLINE s4 dvmGetStaticFieldInt(const StaticField* sfield) ;

StaticField* dvmResolveStaticField(const ClassObject* referrer, u4 sfieldIdx);

INLINE void dvmSetFieldObject(Object* obj, int offset, Object* val) ;

INLINE void dvmSetFieldLong(Object* obj, int offset, s8 val) ;

INLINE void dvmSetFieldInt(Object* obj, int offset, s4 val) ;

INLINE Object* dvmGetFieldObject(const Object* obj, int offset) ;

INLINE s8 dvmGetFieldLong(const Object* obj, int offset) ;

INLINE s4 dvmGetFieldInt(const Object* obj, int offset) ;

InstField* dvmResolveInstField(const ClassObject* referrer, u4 ifieldIdx);

INLINE Field* dvmDexGetResolvedField(const DvmDex* pDvmDex,
    u4 fieldIdx);
    
INLINE void dvmSetObjectArrayElement(const ArrayObject* obj, int index,
                                     Object* val) ;
                                     
void dvmThrowArrayStoreExceptionIncompatibleElement(ClassObject* objectType,
        ClassObject* arrayType);

vbool dvmCanPutArrayElement(const ClassObject* objectClass,
    const ClassObject* arrayClass);

void dvmThrowArrayIndexOutOfBoundsException(int length, int index);

s4 dvmInterpHandleSparseSwitch(const u2* switchData, s4 testVal);

s4 dvmInterpHandlePackedSwitch(const u2* switchData, s4 testVal);

vbool dvmCheckSuspendPending(Thread* self);

INLINE vbool dvmCheckSuspendQuick(Thread* self) ;

INLINE void dvmSetException(Thread* self, Object* exception);

vbool dvmInterpHandleFillArrayData(ArrayObject* arrayObj, const u2* arrayData);

void dvmThrowInternalError(const char* msg) ;

INLINE u4 dvmGetMethodInsnsSize(const Method* meth) ;

ArrayObject* dvmAllocArrayByClass(ClassObject* arrayClass,
    size_t length, int allocFlags);
    
INLINE vbool dvmIsArrayClass(const ClassObject* clazz);

void dvmThrowNegativeArraySizeException(s4 size) ;

Object* dvmAllocObject(ClassObject* clazz, int flags);

vbool dvmInitClass(ClassObject* clazz);

INLINE vbool dvmIsClassInitialized(const ClassObject* clazz) ;

void dvmThrowClassCastException(ClassObject* actual, ClassObject* desired);

INLINE int dvmInstanceof(const ClassObject* instance, const ClassObject* clazz);

INLINE vbool dvmCheckException(Thread* self) ;

vbool dvmUnlockObject(Thread* self, Object *obj);

void dvmLockObject(Thread* self, Object *obj);

ClassObject* dvmResolveClass(const ClassObject* referrer, u4 classIdx,
    vbool fromUnverifiedConstant);
    
INLINE  ClassObject* dvmDexGetResolvedClass(const DvmDex* pDvmDex,
    u4 classIdx);
    
StringObject* dvmResolveString(const ClassObject* referrer, u4 stringIdx);

INLINE StringObject* dvmDexGetResolvedString(const DvmDex* pDvmDex,
    u4 stringIdx);
    
INLINE void dvmClearException(Thread* self) ;

void dvmCheckBefore(const u2 *pc, u4 *fp, Thread* self);

void dvmThrowNullPointerException(const char* msg) ;


#endif