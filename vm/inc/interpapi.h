#ifndef __INTERPAPI_H__
#define __INTERPAPI_H__

#include <dthing.h>
#include "dvmdex.h"
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

void dvmInterpretEntry(Thread * self,JValue *pResult);

void dvmInterpretMakeNativeCall(const u4* args, JValue* pResult, const Method* method, Thread* self);

void dvmCallClinitMethod(const Method* method, Object* obj);

void dvmCallInitMethod(const Method* method, Object* obj, ...);

s4 dvmInterpHandlePackedSwitch(const u2* switchData, s4 testVal);

/*API FUNCs of Interpret,avoid Errors*/

#ifdef ARCH_X86
	#define ANDROID_MEMBAR_STORE()  do{printf("WARNING xxx:not support mem barrier for NOW! \n");}while(0)
#elif defined ARCH_ARM_SPD
	#define ANDROID_MEMBAR_STORE()  do{__asm__ __volatile__ ("" : : : "memory");}while(0)
#else
	#error "have a think!"
#endif

void dvmPopJniLocals(Thread* self, StackSaveArea* saveArea);

void dvmReportPostNativeInvoke(const Method* methodToCall, Thread* self, u4* fp);

void dvmReportPreNativeInvoke(const Method* methodToCall, Thread* self, u4* fp);

#ifndef _TEST_ED_
INLINE vbool dvmIsNativeMethod(const Method* method) ;
#endif

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

#ifndef _TEST_ED_
INLINE vbool dvmIsAbstractMethod(const Method* method) ;

Method* dvmResolveMethod(const ClassObject* referrer, u4 methodIdx,MethodType mType);

void dvmAbort();

void dvmSetFinalizable(Object *obj);

#endif
    
INLINE Method* dvmDexGetResolvedMethod(const DvmDex* pDvmDex,
    u4 methodIdx);

INLINE void dvmWriteBarrierArray(const ArrayObject *obj,
                                 size_t start, size_t end);

void dvmThrowRuntimeException(const char* msg) ;



INLINE void dvmSetStaticFieldObjectVolatile(StaticField* sfield, Object* val) ;

INLINE Object* dvmGetStaticFieldObjectVolatile(const StaticField* sfield) ;

INLINE void dvmSetFieldObjectVolatile(Object* obj, int offset, Object* val);



struct Object* dvmGetException(struct dthread* self) ;




vbool dvmPerformInlineOp4Std(u4 arg0, u4 arg1, u4 arg2, u4 arg3, JValue* pResult, int opIndex);

vbool dvmPerformInlineOp4Dbg(u4 arg0, u4 arg1, u4 arg2, u4 arg3,
    JValue* pResult, int opIndex);

void dvmThrowVerificationError(const Method* method, int kind, int ref);

u1 dvmGetOriginalOpcode(const u2* addr);


INLINE Object* dvmGetFieldObjectVolatile(const Object* obj, int offset) ;

INLINE void dvmSetStaticFieldIntVolatile(StaticField* sfield, s4 val) ;

INLINE s4 dvmGetStaticFieldIntVolatile(const StaticField* sfield) ;

INLINE void dvmSetFieldIntVolatile(Object* obj, int offset, s4 val);

INLINE s4 dvmGetFieldIntVolatile(const Object* obj, int offset) ;


//float	fmodf(float a , float b);

INLINE void dvmSetStaticFieldObject(StaticField* sfield, Object* val) ;

INLINE void dvmSetStaticFieldLong(StaticField* sfield, s8 val) ;

INLINE void dvmSetStaticFieldInt(StaticField* sfield, s4 val) ;

INLINE Object* dvmGetStaticFieldObject(const StaticField* sfield) ;

INLINE s8 dvmGetStaticFieldLong(const StaticField* sfield) ;

INLINE s4 dvmGetStaticFieldInt(const StaticField* sfield) ;

#ifndef _TEST_ED_
StaticField* dvmResolveStaticField(const ClassObject* referrer, u4 sfieldIdx);
#endif

INLINE void dvmSetFieldObject(Object* obj, int offset, Object* val) ;

INLINE void dvmSetFieldLong(Object* obj, int offset, s8 val) ;

INLINE void dvmSetFieldInt(Object* obj, int offset, s4 val) ;

INLINE Object* dvmGetFieldObject(const Object* obj, int offset) ;

INLINE s8 dvmGetFieldLong(const Object* obj, int offset) ;

INLINE s4 dvmGetFieldInt(const Object* obj, int offset) ;

#ifndef _TEST_ED_
InstField* dvmResolveInstField(const ClassObject* referrer, u4 ifieldIdx);
#endif

INLINE Field* dvmDexGetResolvedField(const DvmDex* pDvmDex,
    u4 fieldIdx);
    
INLINE void dvmSetObjectArrayElement(const ArrayObject* obj, int index,
                                     Object* val) ;                                     

#ifndef _TEST_ED_
extern "C" bool dvmCanPutArrayElement(const ClassObject* objectClass, const ClassObject* arrayClass);
#endif


s4 dvmInterpHandleSparseSwitch(const u2* switchData, s4 testVal);



vbool dvmCheckSuspendPending(Thread* self);

vbool dvmCheckSuspendQuick(struct dthread* self) ;

void dvmSetException(struct dthread* self, struct Object* exception);

bool_t dvmInterpHandleFillArrayData(ArrayObject* arrayObj, const u2* arrayData);

void dvmThrowInternalError(const char* msg) ;

INLINE u4 dvmGetMethodInsnsSize(const Method* meth) ;

#ifndef _TEST_ED_
ArrayObject* dvmAllocArrayByClass(ClassObject* arrayClass,
    size_t length, int allocFlags);


bool dvmIsArrayClass( const struct ClassObject * clazz);

Object* dvmAllocObject(ClassObject* clazz, int flags);

#endif

void dvmThrowNegativeArraySizeException(s4 size) ;



#ifndef _TEST_ED_
extern "C" bool dvmInitClass(ClassObject* clazz);

INLINE vbool dvmIsClassInitialized(const ClassObject* clazz) ;

StringObject* dvmResolveString(const ClassObject* referrer, u4 stringIdx);
#endif

INLINE int dvmInstanceof(const ClassObject* instance, const ClassObject* clazz);

/*
 * Returns "true" if an exception is pending.  Use this if you have a
 * "self" pointer.
 */
bool_t dvmCheckException(Thread* self);

vbool dvmUnlockObject(Thread* self, Object *obj);

void dvmLockObject(Thread* self, Object *obj);

ClassObject* dvmResolveClass(const ClassObject* referrer, u4 classIdx,
    bool_t fromUnverifiedConstant);

INLINE  ClassObject* dvmDexGetResolvedClass(const DvmDex* pDvmDex,
    u4 classIdx);
    

INLINE StringObject* dvmDexGetResolvedString(const DvmDex* pDvmDex,
    u4 stringIdx);
    
void dvmClearException(struct dthread* self) ;

void dvmCheckBefore(const u2 *pc, u4 *fp, Thread* self);

void dvmThrowNullPointerException(const char* msg) ;

int dvmInstanceofNonTrivial(const ClassObject* instance,
    const ClassObject* clazz);


#endif