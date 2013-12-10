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
 * Operations on an Object.
 */

#include <dthing.h>
#include <object.h>
#include <resolve.h>
#include <assert.h>


JValue* dvmFieldPtr(const Object* obj, int offset)
{
    return ((JValue*)BYTE_OFFSET(obj, offset));
}

bool_t dvmGetFieldBoolean(const Object* obj, int offset)
{
    return ((JValue*)BYTE_OFFSET(obj, offset))->z;
}

s1 dvmGetFieldByte(const Object* obj, int offset)
{
    return ((JValue*)BYTE_OFFSET(obj, offset))->b;
}

s2 dvmGetFieldShort(const Object* obj, int offset)
{
    return ((JValue*)BYTE_OFFSET(obj, offset))->s;
}

u2 dvmGetFieldChar(const Object* obj, int offset)
{
    return ((JValue*)BYTE_OFFSET(obj, offset))->c;
}

s4 dvmGetFieldInt(const Object* obj, int offset)
{
    return ((JValue*)BYTE_OFFSET(obj, offset))->i;
}

s8 dvmGetFieldLong(const Object* obj, int offset)
{
    return ((JValue*)BYTE_OFFSET(obj, offset))->j;
}

float dvmGetFieldFloat(const Object* obj, int offset)
{
    return ((JValue*)BYTE_OFFSET(obj, offset))->f;
}

double dvmGetFieldDouble(const Object* obj, int offset)
{
    return ((JValue*)BYTE_OFFSET(obj, offset))->d;
}

Object* dvmGetFieldObject(const Object* obj, int offset)
{
    return ((JValue*)BYTE_OFFSET(obj, offset))->l;
}

s8 dvmGetFieldLongVolatile(const Object* obj, int offset)
{
	assert(0);
/*
    const s8* addr = (const s8*)BYTE_OFFSET(obj, offset);
    s8 val = dvmQuasiAtomicRead64(addr);
    ANDROID_MEMBAR_FULL();
    return val;
*/
    return 0; //fake to avoid warning.
}

#if 0
bool_t dvmGetFieldBooleanVolatile(const Object* obj, int offset)
{
    s4* ptr = &((JValue*)BYTE_OFFSET(obj, offset))->i;
    return (bool)android_atomic_acquire_load(ptr);
}

s1 dvmGetFieldByteVolatile(const Object* obj, int offset)
{
    s4* ptr = &((JValue*)BYTE_OFFSET(obj, offset))->i;
    return (s1)android_atomic_acquire_load(ptr);
}

s2 dvmGetFieldShortVolatile(const Object* obj, int offset)
{
    s4* ptr = &((JValue*)BYTE_OFFSET(obj, offset))->i;
    return (s2)android_atomic_acquire_load(ptr);
}

u2 dvmGetFieldCharVolatile(const Object* obj, int offset)
{
    s4* ptr = &((JValue*)BYTE_OFFSET(obj, offset))->i;
    return (u2)android_atomic_acquire_load(ptr);
}

s4 dvmGetFieldIntVolatile(const Object* obj, int offset)
{
    s4* ptr = &((JValue*)BYTE_OFFSET(obj, offset))->i;
    return android_atomic_acquire_load(ptr);
}

float dvmGetFieldFloatVolatile(const Object* obj, int offset)
{
    union { s4 ival; float fval; } alias;
    s4* ptr = &((JValue*)BYTE_OFFSET(obj, offset))->i;
    alias.ival = android_atomic_acquire_load(ptr);
    return alias.fval;
}



double dvmGetFieldDoubleVolatile(const Object* obj, int offset)
{
    union { s8 lval; double dval; } alias;
    const s8* addr = (const s8*)BYTE_OFFSET(obj, offset);
    alias.lval = dvmQuasiAtomicRead64(addr);
    ANDROID_MEMBAR_FULL();
    return alias.dval;
}

Object* dvmGetFieldObjectVolatile(const Object* obj, int offset)
{
    Object** ptr = &((JValue*)BYTE_OFFSET(obj, offset))->l;
    return (Object*)android_atomic_acquire_load((int32_t*)ptr);
}
#endif

void dvmSetFieldBoolean(Object* obj, int offset, bool_t val)
{
    ((JValue*)BYTE_OFFSET(obj, offset))->i = val;
}

void dvmSetFieldByte(Object* obj, int offset, s1 val)
{
    ((JValue*)BYTE_OFFSET(obj, offset))->i = val;
}

void dvmSetFieldShort(Object* obj, int offset, s2 val)
{
    ((JValue*)BYTE_OFFSET(obj, offset))->i = val;
}

void dvmSetFieldChar(Object* obj, int offset, u2 val)
{
    ((JValue*)BYTE_OFFSET(obj, offset))->i = val;
}

void dvmSetFieldInt(Object* obj, int offset, s4 val)
{
    ((JValue*)BYTE_OFFSET(obj, offset))->i = val;
}

void dvmSetFieldFloat(Object* obj, int offset, float val)
{
    ((JValue*)BYTE_OFFSET(obj, offset))->f = val;
}

void dvmSetFieldLong(Object* obj, int offset, s8 val)
{
    ((JValue*)BYTE_OFFSET(obj, offset))->j = val;
}

void dvmSetFieldDouble(Object* obj, int offset, double val)
{
    ((JValue*)BYTE_OFFSET(obj, offset))->d = val;
}

void dvmSetFieldObject(Object* obj, int offset, Object* val)
{
    JValue* lhs = (JValue*)BYTE_OFFSET(obj, offset);
    lhs->l = val;
    //if (val != NULL) {
    //    dvmWriteBarrierField(obj, &lhs->l);
    //}
}

void dvmSetFieldLongVolatile(Object* obj, int offset, s8 val)
{
	assert(0);
//    s8* addr = (s8*)BYTE_OFFSET(obj, offset);
//    dvmQuasiAtomicSwap64Sync(val, addr);
}

#if 0
void dvmSetFieldIntVolatile(Object* obj, int offset, s4 val)
{
    s4* ptr = &((JValue*)BYTE_OFFSET(obj, offset))->i;
    /*
     * TODO: add an android_atomic_synchronization_store() function and
     * use it in the 32-bit volatile set handlers.  On some platforms we
     * can use a fast atomic instruction and avoid the barriers.
     */
    ANDROID_MEMBAR_STORE();
    *ptr = val;
    ANDROID_MEMBAR_FULL();
}

void dvmSetFieldBooleanVolatile(Object* obj, int offset, bool val)
{
    dvmSetFieldIntVolatile(obj, offset, val);
}

void dvmSetFieldByteVolatile(Object* obj, int offset, s1 val)
{
    dvmSetFieldIntVolatile(obj, offset, val);
}

void dvmSetFieldShortVolatile(Object* obj, int offset, s2 val)
{
    dvmSetFieldIntVolatile(obj, offset, val);
}

void dvmSetFieldCharVolatile(Object* obj, int offset, u2 val)
{
    dvmSetFieldIntVolatile(obj, offset, val);
}

void dvmSetFieldFloatVolatile(Object* obj, int offset, float val)
{
    union { s4 ival; float fval; } alias;
    alias.fval = val;
    dvmSetFieldIntVolatile(obj, offset, alias.ival);
}



void dvmSetFieldDoubleVolatile(Object* obj, int offset, double val)
{
    union { s8 lval; double dval; } alias;
    alias.dval = val;
    dvmSetFieldLongVolatile(obj, offset, alias.lval);
}

void dvmSetFieldObjectVolatile(Object* obj, int offset, Object* val)
{
    Object** ptr = &((JValue*)BYTE_OFFSET(obj, offset))->l;
    ANDROID_MEMBAR_STORE();
    *ptr = val;
    ANDROID_MEMBAR_FULL();
    if (val != NULL) {
        dvmWriteBarrierField(obj, ptr);
    }
}
#endif

/*
 * Static field access functions.
 */
JValue* dvmStaticFieldPtr(const StaticField* sfield)
{
    return (JValue*)&sfield->value;
}

bool_t dvmGetStaticFieldBoolean(const StaticField* sfield)
{
    return sfield->value.z;
}
s1 dvmGetStaticFieldByte(const StaticField* sfield)
{
    return sfield->value.b;
}
s2 dvmGetStaticFieldShort(const StaticField* sfield)
{
    return sfield->value.s;
}
u2 dvmGetStaticFieldChar(const StaticField* sfield)
{
    return sfield->value.c;
}
s4 dvmGetStaticFieldInt(const StaticField* sfield)
{
    return sfield->value.i;
}
float dvmGetStaticFieldFloat(const StaticField* sfield)
{
    return sfield->value.f;
}
s8 dvmGetStaticFieldLong(const StaticField* sfield)
{
    return sfield->value.j;
}
double dvmGetStaticFieldDouble(const StaticField* sfield)
{
    return sfield->value.d;
}
Object* dvmGetStaticFieldObject(const StaticField* sfield)
{
    return sfield->value.l;
}

s8 dvmGetStaticFieldLongVolatile(const StaticField* sfield)
{
	assert(0);
/*    const s8* addr = &sfield->value.j;
    s8 val = dvmQuasiAtomicRead64(addr);
    ANDROID_MEMBAR_FULL();
    return val;*/
    return 0; //fake to avoid warning.
}

#if 0
bool_t dvmGetStaticFieldBooleanVolatile(const StaticField* sfield)
{
    const s4* ptr = &(sfield->value.i);
    return (bool)android_atomic_acquire_load((s4*)ptr);
}
s1 dvmGetStaticFieldByteVolatile(const StaticField* sfield)
{
    const s4* ptr = &(sfield->value.i);
    return (s1)android_atomic_acquire_load((s4*)ptr);
}
s2 dvmGetStaticFieldShortVolatile(const StaticField* sfield)
{
    const s4* ptr = &(sfield->value.i);
    return (s2)android_atomic_acquire_load((s4*)ptr);
}
u2 dvmGetStaticFieldCharVolatile(const StaticField* sfield)
{
    const s4* ptr = &(sfield->value.i);
    return (u2)android_atomic_acquire_load((s4*)ptr);
}
s4 dvmGetStaticFieldIntVolatile(const StaticField* sfield)
{
    const s4* ptr = &(sfield->value.i);
    return android_atomic_acquire_load((s4*)ptr);
}
float dvmGetStaticFieldFloatVolatile(const StaticField* sfield)
{
    union { s4 ival; float fval; } alias;
    const s4* ptr = &(sfield->value.i);
    alias.ival = android_atomic_acquire_load((s4*)ptr);
    return alias.fval;
}

double dvmGetStaticFieldDoubleVolatile(const StaticField* sfield)
{
    union { s8 lval; double dval; } alias;
    const s8* addr = &sfield->value.j;
    alias.lval = dvmQuasiAtomicRead64(addr);
    ANDROID_MEMBAR_FULL();
    return alias.dval;
}
Object* dvmGetStaticFieldObjectVolatile(const StaticField* sfield)
{
    Object* const* ptr = &(sfield->value.l);
    return (Object*)android_atomic_acquire_load((int32_t*)ptr);
}
#endif

void dvmSetStaticFieldBoolean(StaticField* sfield, bool_t val)
{
    sfield->value.i = val;
}
void dvmSetStaticFieldByte(StaticField* sfield, s1 val)
{
    sfield->value.i = val;
}
void dvmSetStaticFieldShort(StaticField* sfield, s2 val)
{
    sfield->value.i = val;
}
void dvmSetStaticFieldChar(StaticField* sfield, u2 val)
{
    sfield->value.i = val;
}
void dvmSetStaticFieldInt(StaticField* sfield, s4 val)
{
    sfield->value.i = val;
}
void dvmSetStaticFieldFloat(StaticField* sfield, float val)
{
    sfield->value.f = val;
}
void dvmSetStaticFieldLong(StaticField* sfield, s8 val)
{
    sfield->value.j = val;
}
void dvmSetStaticFieldDouble(StaticField* sfield, double val)
{
    sfield->value.d = val;
}
void dvmSetStaticFieldObject(StaticField* sfield, Object* val)
{
    sfield->value.l = val;
    //if (val != NULL) {
    //    dvmWriteBarrierField(sfield->clazz, &sfield->value.l);
    //}
}

void dvmSetStaticFieldLongVolatile(StaticField* sfield, s8 val)
{
	assert(0);
 //   s8* addr = &sfield->value.j;
    //dvmQuasiAtomicSwap64Sync(val, addr);
}

#if 0
void dvmSetStaticFieldIntVolatile(StaticField* sfield, s4 val)
 {
    s4* ptr = &sfield->value.i;
    ANDROID_MEMBAR_STORE();
    *ptr = val;
    ANDROID_MEMBAR_FULL();
}

void dvmSetStaticFieldBooleanVolatile(StaticField* sfield, bool_t val)
{
    dvmSetStaticFieldIntVolatile(sfield, val);
}
void dvmSetStaticFieldByteVolatile(StaticField* sfield, s1 val)
{
    dvmSetStaticFieldIntVolatile(sfield, val);
}
void dvmSetStaticFieldShortVolatile(StaticField* sfield, s2 val)
{
    dvmSetStaticFieldIntVolatile(sfield, val);
}
void dvmSetStaticFieldCharVolatile(StaticField* sfield, u2 val)
{
    dvmSetStaticFieldIntVolatile(sfield, val);
}
void dvmSetStaticFieldFloatVolatile(StaticField* sfield, float val)
{
    union { s4 ival; float fval; } alias;
    alias.fval = val;
    dvmSetStaticFieldIntVolatile(sfield, alias.ival);
}
void dvmSetStaticFieldLongVolatile(StaticField* sfield, s8 val)
{
    s8* addr = &sfield->value.j;
    dvmQuasiAtomicSwap64Sync(val, addr);
}
void dvmSetStaticFieldDoubleVolatile(StaticField* sfield, double val)
{
    union { s8 lval; double dval; } alias;
    alias.dval = val;
    dvmSetStaticFieldLongVolatile(sfield, alias.lval);
}

void dvmSetStaticFieldObjectVolatile(StaticField* sfield, Object* val)
{
    Object** ptr = &(sfield->value.l);
    ANDROID_MEMBAR_STORE();
    *ptr = val;
    ANDROID_MEMBAR_FULL();
    if (val != NULL) {
        dvmWriteBarrierField(sfield->clazz, &sfield->value.l);
    }
}
#endif

/*
 * Find a field and return the byte offset from the object pointer.  Only
 * searches the specified class, not the superclass.
 *
 * Returns -1 on failure.
 */
int dvmFindFieldOffset(const ClassObject* clazz, const char* fieldName, const char* signature)
{
    InstField* pField = dvmFindInstanceField(clazz, fieldName, signature);
    if (pField == NULL)
        return -1;
    else
        return pField->byteOffset;
}

/*
 * Helpers.
 */
bool_t dvmIsPublicMethod(const Method* method)
{
    return (method->accessFlags & ACC_PUBLIC) != 0;
}
bool_t dvmIsPrivateMethod(const Method* method)
{
    return (method->accessFlags & ACC_PRIVATE) != 0;
}
bool_t dvmIsStaticMethod(const Method* method)
{
    return (method->accessFlags & ACC_STATIC) != 0;
}
bool_t dvmIsSynchronizedMethod(const Method* method)
{
    return (method->accessFlags & ACC_SYNCHRONIZED) != 0;
}
bool_t dvmIsDeclaredSynchronizedMethod(const Method* method)
{
    return (method->accessFlags & ACC_DECLARED_SYNCHRONIZED) != 0;
}
bool_t dvmIsFinalMethod(const Method* method)
{
    return (method->accessFlags & ACC_FINAL) != 0;
}
bool_t dvmIsNativeMethod(const Method* method)
{
    return (method->accessFlags & ACC_NATIVE) != 0;
}
bool_t dvmIsAbstractMethod(const Method* method)
{
    return (method->accessFlags & ACC_ABSTRACT) != 0;
}
bool_t dvmIsSyntheticMethod(const Method* method) 
{
    return (method->accessFlags & ACC_SYNTHETIC) != 0;
}
#if 0
bool_t dvmIsMirandaMethod(const Method* method)
{
    return (method->accessFlags & ACC_MIRANDA) != 0;
}
#endif
bool_t dvmIsConstructorMethod(const Method* method)
{
    return *method->name == '<';
}
/* Dalvik puts private, static, and constructors into non-virtual table */
bool_t dvmIsDirectMethod(const Method* method)
{
    return dvmIsPrivateMethod(method) ||
           dvmIsStaticMethod(method) ||
           dvmIsConstructorMethod(method);
}
/* Get whether the given method has associated bytecode. This is the
 * case for methods which are neither native nor abstract. */
bool_t dvmIsBytecodeMethod(const Method* method)
{
    return (method->accessFlags & (ACC_NATIVE | ACC_ABSTRACT)) == 0;
}

bool_t dvmIsProtectedField(const Field* field)
{
    return (field->accessFlags & ACC_PROTECTED) != 0;
}
bool_t dvmIsStaticField(const Field* field) {
    return (field->accessFlags & ACC_STATIC) != 0;
}
bool_t dvmIsFinalField(const Field* field)
{
    return (field->accessFlags & ACC_FINAL) != 0;
}
bool_t dvmIsVolatileField(const Field* field)
{
    return (field->accessFlags & ACC_VOLATILE) != 0;
}

bool_t dvmIsInterfaceClass(const ClassObject* clazz)
{
    return (clazz->accessFlags & ACC_INTERFACE) != 0;
}
bool_t dvmIsPublicClass(const ClassObject* clazz)
{
    return (clazz->accessFlags & ACC_PUBLIC) != 0;
}
bool_t dvmIsFinalClass(const ClassObject* clazz)
{
    return (clazz->accessFlags & ACC_FINAL) != 0;
}
bool_t dvmIsAbstractClass(const ClassObject* clazz)
{
    return (clazz->accessFlags & ACC_ABSTRACT) != 0;
}
bool_t dvmIsAnnotationClass(const ClassObject* clazz)
{
    return (clazz->accessFlags & ACC_ANNOTATION) != 0;
}
bool_t dvmIsPrimitiveClass(const ClassObject* clazz)
{
    return clazz->primitiveType != PRIM_NOT;
}

/* linked, here meaning prepared and resolved */
bool_t dvmIsClassLinked(const ClassObject* clazz)
{
    return clazz->status >= CLASS_RESOLVED;
}
/* has class been verified? */
bool_t dvmIsClassVerified(const ClassObject* clazz)
{
    return clazz->status >= CLASS_VERIFIED;
}

/*
 * Return whether the given object is an instance of Class.
 */
bool_t dvmIsClassObject(const Object* obj)
{
    //assert(obj != NULL);
    //assert(obj->clazz != NULL);
    return IS_CLASS_FLAG_SET(obj->clazz, CLASS_ISCLASS);
}

/*
 * Return whether the given object is the class Class (that is, the
 * unique class which is an instance of itself).
 */
bool_t dvmIsTheClassClass(const ClassObject* clazz) {
    //assert(clazz != NULL);
    return IS_CLASS_FLAG_SET(clazz, CLASS_ISCLASS);
}

/*
 * Determine if a class has been initialized.
 */
bool_t dvmIsClassInitialized(const ClassObject* clazz) {
    return (clazz->status == CLASS_INITIALIZED);
}


/*
 * Returns true if the class is being initialized by us (which means that
 * calling dvmInitClass will return immediately after fiddling with locks).
 * Returns false if it's not being initialized, or if it's being
 * initialized by another thread.
 *
 * The value for initThreadId is always set to "self->threadId", by the
 * thread doing the initializing.  If it was done by the current thread,
 * we are guaranteed to see "initializing" and our thread ID, even on SMP.
 * If it was done by another thread, the only bad situation is one in
 * which we see "initializing" and a stale copy of our own thread ID
 * while another thread is actually handling init.
 *
 * The initThreadId field is used during class linking, so it *is*
 * possible to have a stale value floating around.  We need to ensure
 * that memory accesses happen in the correct order.
 */
bool_t dvmIsClassInitializing(const ClassObject* clazz) {
    /* FIXME: doesn't check the thread IDs, may be needed to add in next step. */
    return (clazz->status == CLASS_INITIALIZING);
}

/*
 * Get the associated code struct for a method. This returns NULL
 * for non-bytecode methods.
 */
const DexCode* dvmGetMethodCode(const Method* meth)
{
    if (dvmIsBytecodeMethod(meth)) {
        /*
         * The insns field for a bytecode method actually points at
         * &(DexCode.insns), so we can subtract back to get at the
         * DexCode in front.
         */
        return (const DexCode*)
            (((const u1*) meth->insns) - CRTL_offsetof(DexCode, insns));
    } else {
        return NULL;
    }
}

/*
 * Get the size of the insns associated with a method. This returns 0
 * for non-bytecode methods.
 */
u4 dvmGetMethodInsnsSize(const Method* meth)
{
    const DexCode* pCode = dvmGetMethodCode(meth);
    return (pCode == NULL) ? 0 : pCode->insnsSize;
}

/*
 * Determine whether "sub" is a sub-class of "clazz".
 *
 * Returns 0 (false) if not, 1 (true) if so.
 */
int dvmIsSubClass(const ClassObject* sub, const ClassObject* clazz)
{
    do {
        /*printf("###### sub='%s' clazz='%s'\n", sub->name, clazz->name);*/
        if (sub == clazz)
            return 1;
        sub = sub->super;
    } while (sub != NULL);

    return 0;
}

/*
 * Store a single value in the array, and if the value isn't null,
 * note in the write barrier.
 */
void dvmSetObjectArrayElement(const ArrayObject* obj, int index,
                                     Object* val) {
    ((Object **)(void *)(obj)->contents)[index] = val;
}



/**======================================================================
/**    Original Object implementation
/**======================================================================*/


/*
 * Find a matching field, in the current class only.
 *
 * Returns NULL if the field can't be found.  (Does not throw an exception.)
 */
InstField* dvmFindInstanceField(const ClassObject* clazz, const char* fieldName, const char* signature)
{
    InstField* pField;
    int i;

    //assert(clazz != NULL);

    /*
     * Find a field with a matching name and signature.  The Java programming
     * language does not allow you to have two fields with the same name
     * and different types, but the Java VM spec does allow it, so we can't
     * bail out early when the name matches.
     */
    pField = clazz->ifields;
    for (i = 0; i < clazz->ifieldCount; i++, pField++) {
        if (CRTL_strcmp(fieldName, pField->field.name) == 0 &&
            CRTL_strcmp(signature, pField->field.signature) == 0)
        {
            return pField;
        }
    }

    return NULL;
}

/*
 * Find a matching field, in this class or a superclass.
 *
 * Searching through interfaces isn't necessary, because interface fields
 * are inherently public/static/final.
 *
 * Returns NULL if the field can't be found.  (Does not throw an exception.)
 */
InstField* dvmFindInstanceFieldHier(const ClassObject* clazz,
    const char* fieldName, const char* signature)
{
    InstField* pField;

    /*
     * Search for a match in the current class.
     */
    pField = dvmFindInstanceField(clazz, fieldName, signature);
    if (pField != NULL)
        return pField;

    if (clazz->super != NULL)
        return dvmFindInstanceFieldHier(clazz->super, fieldName, signature);
    else
        return NULL;
}


/*
 * Find a matching field, in this class or an interface.
 *
 * Returns NULL if the field can't be found.  (Does not throw an exception.)
 */
StaticField* dvmFindStaticField(const ClassObject* clazz,
    const char* fieldName, const char* signature)
{
    const StaticField* pField;
    int i;

    //assert(clazz != NULL);

    /*
     * Find a field with a matching name and signature.  As with instance
     * fields, the VM allows you to have two fields with the same name so
     * long as they have different types.
     */
    pField = &clazz->sfields[0];
    for (i = 0; i < clazz->sfieldCount; i++, pField++) {
        if (CRTL_strcmp(fieldName, pField->field.name) == 0 &&
            CRTL_strcmp(signature, pField->field.signature) == 0)
        {
            return (StaticField*) pField;
        }
    }

    return NULL;
}

/*
 * Find a matching field, in this class or a superclass.
 *
 * Returns NULL if the field can't be found.  (Does not throw an exception.)
 */
StaticField* dvmFindStaticFieldHier(const ClassObject* clazz,
    const char* fieldName, const char* signature)
{
    StaticField* pField;
    int i = 0;

    /*
     * Search for a match in the current class.
     */
    pField = dvmFindStaticField(clazz, fieldName, signature);
    if (pField != NULL)
        return pField;

    /*
     * See if it's in any of our interfaces.  We don't check interfaces
     * inherited from the superclass yet.
     *
     * (Note the set may have been stripped down because of redundancy with
     * the superclass; see notes in createIftable.)
     */
    if (clazz->super != NULL) {
        //assert(clazz->iftableCount >= clazz->super->iftableCount);
        i = clazz->super->iftableCount;
    }
    for ( ; i < clazz->iftableCount; i++) {
        ClassObject* iface = clazz->iftable[i].clazz;
        pField = dvmFindStaticField(iface, fieldName, signature);
        if (pField != NULL)
            return pField;
    }

    if (clazz->super != NULL)
        return dvmFindStaticFieldHier(clazz->super, fieldName, signature);
    else
        return NULL;
}

/*
 * Find a matching field, in this class or a superclass.
 *
 * We scan both the static and instance field lists in the class.  If it's
 * not found there, we check the direct interfaces, and then recursively
 * scan the superclasses.  This is the order prescribed in the VM spec
 * (v2 5.4.3.2).
 *
 * In most cases we know that we're looking for either a static or an
 * instance field and there's no value in searching through both types.
 * During verification we need to recognize and reject certain unusual
 * situations, and we won't see them unless we walk the lists this way.
 */
Field* dvmFindFieldHier(const ClassObject* clazz, const char* fieldName,
    const char* signature)
{
    Field* pField;
    int i = 0;

    /*
     * Search for a match in the current class.  Which set we scan first
     * doesn't really matter.
     */
    pField = (Field*) dvmFindStaticField(clazz, fieldName, signature);
    if (pField != NULL)
        return pField;
    pField = (Field*) dvmFindInstanceField(clazz, fieldName, signature);
    if (pField != NULL)
        return pField;

    /*
     * See if it's in any of our interfaces.  We don't check interfaces
     * inherited from the superclass yet.
     */
    if (clazz->super != NULL) {
        //assert(clazz->iftableCount >= clazz->super->iftableCount);
        i = clazz->super->iftableCount;
    }
    for ( ; i < clazz->iftableCount; i++) {
        ClassObject* iface = clazz->iftable[i].clazz;
        pField = (Field*) dvmFindStaticField(iface, fieldName, signature);
        if (pField != NULL)
            return pField;
    }

    if (clazz->super != NULL)
        return dvmFindFieldHier(clazz->super, fieldName, signature);
    else
        return NULL;
}


/*
 * Compare the given name, return type, and argument types with the contents
 * of the given method. This returns 0 if they are equal and non-zero if not.
 */
static int compareMethodHelper(Method* method, const char* methodName,
    const char* returnType, size_t argCount, const char** argTypes)
{
    DexParameterIterator iterator;
    const DexProto* proto;

    if (CRTL_strcmp(methodName, method->name) != 0) {
        return 1;
    }

    proto = &method->prototype;

    if (CRTL_strcmp(returnType, dexProtoGetReturnType(proto)) != 0) {
        return 1;
    }

    if (dexProtoGetParameterCount(proto) != argCount) {
        return 1;
    }

    dexParameterIteratorInit(&iterator, proto);

    for (/*argCount*/; argCount != 0; argCount--, argTypes++) {
        const char* argType = *argTypes;
        const char* paramType = dexParameterIteratorNextDescriptor(&iterator);

        if (paramType == NULL) {
            /* Param list ended early; no match */
            break;
        } else if (CRTL_strcmp(argType, paramType) != 0) {
            /* Types aren't the same; no match. */
            break;
        }
    }

    if (argCount == 0) {
        /* We ran through all the given arguments... */
        if (dexParameterIteratorNextDescriptor(&iterator) == NULL) {
            /* ...and through all the method's arguments; success! */
            return 0;
        }
    }

    return 1;
}

/*
 * Get the count of arguments in the given method descriptor string,
 * and also find a pointer to the return type.
 */
static size_t countArgsAndFindReturnType(const char* descriptor, const char** pReturnType)
{
    size_t count = 0;
    bool_t bogus = FALSE;
    bool_t done = FALSE;

    //assert(*descriptor == '(');
    descriptor++;

    while (!done) {
        switch (*descriptor) {
            case 'B': case 'C': case 'D': case 'F':
            case 'I': case 'J': case 'S': case 'Z': {
                count++;
                break;
            }
            case '[': {
                do {
                    descriptor++;
                } while (*descriptor == '[');
                /*
                 * Don't increment count, as it will be taken care of
                 * by the next iteration. Also, decrement descriptor
                 * to compensate for the increment below the switch.
                 */
                descriptor--;
                break;
            }
            case 'L': {
                do {
                    descriptor++;
                } while ((*descriptor != ';') && (*descriptor != '\0'));
                count++;
                if (*descriptor == '\0') {
                    /* Bogus descriptor. */
                    done = TRUE;
                    bogus = TRUE;
                }
                break;
            }
            case ')': {
                /*
                 * Note: The loop will exit after incrementing descriptor
                 * one more time, so it then points at the return type.
                 */
                done = TRUE;
                break;
            }
            default: {
                /* Bogus descriptor. */
                done = TRUE;
                bogus = TRUE;
                break;
            }
        }

        descriptor++;
    }

    if (bogus) {
        *pReturnType = NULL;
        return 0;
    }

    *pReturnType = descriptor;
    return count;
}

/*
 * Copy the argument types into the given array using the given buffer
 * for the contents.
 */
static void copyTypes(char* buffer, const char** argTypes, size_t argCount, const char* descriptor)
{
    size_t i;
    char c;

    /* Skip the '('. */
    descriptor++;

    for (i = 0; i < argCount; i++) {
        argTypes[i] = buffer;

        /* Copy all the array markers and one extra character. */
        do {
            c = *(descriptor++);
            *(buffer++) = c;
        } while (c == '[');

        if (c == 'L') {
            /* Copy the rest of a class name. */
            do {
                c = *(descriptor++);
                *(buffer++) = c;
            } while (c != ';');
        }

        *(buffer++) = '\0';
    }
}

/*
 * Look for a match in the given class. Returns the match if found
 * or NULL if not.
 */
static Method* findMethodInListByDescriptor(const ClassObject* clazz,
    bool_t findVirtual, bool_t isHier, const char* name, const char* descriptor)
{
#if 1
    const char* returnType;
    size_t argCount = countArgsAndFindReturnType(descriptor, &returnType);
    char *buffer = NULL;
    char** argTypes;
    Method* method = NULL;
    bool_t found = FALSE;

    if (returnType == NULL) {
        DVMTraceWar("Bogus method descriptor: %s", descriptor);
        return NULL;
    }

    /*
     * Make buffer big enough for all the argument type characters and
     * one '\0' per argument. The "- 2" is because "returnType -
     * descriptor" includes two parens.
     */
    //char buffer[argCount + (returnType - descriptor) - 2];
    //const char* argTypes[argCount];

    buffer = (char *)CRTL_malloc(argCount + (returnType - descriptor) - 2);
    if (buffer == NULL)
    {
        goto bail;
    }
    CRTL_memset(buffer, 0x0, argCount + (returnType - descriptor) - 2);

    argTypes = (char**)CRTL_malloc(argCount * sizeof(char*));
    if (argTypes == NULL)
    {
        goto bail;
    }
    CRTL_memset(argTypes, 0x0, argCount * sizeof(char*));

    copyTypes(buffer, argTypes, argCount, descriptor);

    while (clazz != NULL) {
        Method* methods;
        size_t methodCount;
        size_t i;

        if (findVirtual) {
            methods = clazz->virtualMethods;
            methodCount = clazz->virtualMethodCount;
        } else {
            methods = clazz->directMethods;
            methodCount = clazz->directMethodCount;
        }

        for (i = 0; i < methodCount; i++) {
            method = &methods[i];
            if (compareMethodHelper(method, name, returnType, argCount,
                            argTypes) == 0) {
                found = TRUE;
                goto bail;
            }
        }

        if (! isHier) {
            break;
        }

        clazz = clazz->super;
    }

bail:
    CRTL_freeif(buffer);
    CRTL_freeif(argTypes);
    
    if (found) {
        return method;
    }

    return NULL;
#else
    return NULL;
#endif
}

/*
 * Look for a match in the given clazz. Returns the match if found
 * or NULL if not.
 *
 * "wantedType" should be METHOD_VIRTUAL or METHOD_DIRECT to indicate the
 * list to search through.  If the match can come from either list, use
 * MATCH_UNKNOWN to scan both.
 */
static Method* findMethodInListByProto(const ClassObject* clazz,
    MethodType wantedType, bool_t isHier, const char* name, const DexProto* proto)
{
    while (clazz != NULL) {
        int i;

        /*
         * Check the virtual and/or direct method lists.
         */
        if (wantedType == METHOD_VIRTUAL || wantedType == METHOD_UNKNOWN) {
            for (i = 0; i < clazz->virtualMethodCount; i++) {
                Method* method = &clazz->virtualMethods[i];
                if (dvmCompareNameProtoAndMethod(name, proto, method) == 0) {
                    return method;
                }
            }
        }
        if (wantedType == METHOD_DIRECT || wantedType == METHOD_UNKNOWN) {
            for (i = 0; i < clazz->directMethodCount; i++) {
                Method* method = &clazz->directMethods[i];
                if (dvmCompareNameProtoAndMethod(name, proto, method) == 0) {
                    return method;
                }
            }
        }

        if (! isHier) {
            break;
        }

        clazz = clazz->super;
    }

    return NULL;
}

/*
 * Find a "virtual" method in a class.
 *
 * Does not chase into the superclass.
 *
 * Returns NULL if the method can't be found.  (Does not throw an exception.)
 */
Method* dvmFindVirtualMethodByDescriptor(const ClassObject* clazz,
    const char* methodName, const char* descriptor)
{
    return findMethodInListByDescriptor(clazz, TRUE, FALSE,
            methodName, descriptor);

    // TODO? - throw IncompatibleClassChangeError if a match is
    // found in the directMethods list, rather than NotFoundError.
    // Note we could have been called by dvmFindVirtualMethodHier though.
}


/*
 * Find a "virtual" method in a class, knowing only the name.  This is
 * only useful in limited circumstances, e.g. when searching for a member
 * of an annotation class.
 *
 * Does not chase into the superclass.
 *
 * Returns NULL if the method can't be found.  (Does not throw an exception.)
 */
Method* dvmFindVirtualMethodByName(const ClassObject* clazz,
    const char* methodName)
{
    Method* methods = clazz->virtualMethods;
    int methodCount = clazz->virtualMethodCount;
    int i;

    for (i = 0; i < methodCount; i++) {
        if (CRTL_strcmp(methods[i].name, methodName) == 0)
            return &methods[i];
    }

    return NULL;
}

/*
 * Find a "virtual" method in a class.
 *
 * Does not chase into the superclass.
 *
 * Returns NULL if the method can't be found.  (Does not throw an exception.)
 */
Method* dvmFindVirtualMethod(const ClassObject* clazz, const char* methodName,
    const DexProto* proto)
{
    return findMethodInListByProto(clazz, METHOD_VIRTUAL, FALSE, methodName,
            proto);
}

/*
 * Find a "virtual" method in a class.  If we don't find it, try the
 * superclass.  Does not examine interfaces.
 *
 * Returns NULL if the method can't be found.  (Does not throw an exception.)
 */
Method* dvmFindVirtualMethodHierByDescriptor(const ClassObject* clazz,
    const char* methodName, const char* descriptor)
{
    return findMethodInListByDescriptor(clazz, TRUE, TRUE,
            methodName, descriptor);
}

/*
 * Find a "virtual" method in a class.  If we don't find it, try the
 * superclass.  Does not examine interfaces.
 *
 * Returns NULL if the method can't be found.  (Does not throw an exception.)
 */
Method* dvmFindVirtualMethodHier(const ClassObject* clazz,
    const char* methodName, const DexProto* proto)
{
    return findMethodInListByProto(clazz, METHOD_VIRTUAL, TRUE, methodName,
            proto);
}

/*
 * Find a method in an interface.  Searches superinterfaces.
 *
 * Returns NULL if the method can't be found.  (Does not throw an exception.)
 */
Method* dvmFindInterfaceMethodHierByDescriptor(const ClassObject* iface,
    const char* methodName, const char* descriptor)
{
    Method* resMethod = dvmFindVirtualMethodByDescriptor(iface,
        methodName, descriptor);
    if (resMethod == NULL) {
        /* scan superinterfaces and superclass interfaces */
        int i;
        for (i = 0; i < iface->iftableCount; i++) {
            resMethod = dvmFindVirtualMethodByDescriptor(iface->iftable[i].clazz,
                methodName, descriptor);
            if (resMethod != NULL)
                break;
        }
    }
    return resMethod;
}

/*
 * Find a method in an interface.  Searches superinterfaces.
 *
 * Returns NULL if the method can't be found.  (Does not throw an exception.)
 */
Method* dvmFindInterfaceMethodHier(const ClassObject* iface,
    const char* methodName, const DexProto* proto)
{
    Method* resMethod = dvmFindVirtualMethod(iface, methodName, proto);
    if (resMethod == NULL) {
        /* scan superinterfaces and superclass interfaces */
        int i;
        for (i = 0; i < iface->iftableCount; i++) {
            resMethod = dvmFindVirtualMethod(iface->iftable[i].clazz,
                methodName, proto);
            if (resMethod != NULL)
                break;
        }
    }
    return resMethod;
}

/*
 * Find a "direct" method (static, private, or "<*init>").
 *
 * Returns NULL if the method can't be found.  (Does not throw an exception.)
 */
Method* dvmFindDirectMethodByDescriptor(const ClassObject* clazz,
    const char* methodName, const char* descriptor)
{
    return findMethodInListByDescriptor(clazz, FALSE, FALSE,
            methodName, descriptor);
}

/*
 * Find a "direct" method.  If we don't find it, try the superclass.  This
 * is only appropriate for static methods, but will work for all direct
 * methods.
 *
 * Returns NULL if the method can't be found.  (Does not throw an exception.)
 */
Method* dvmFindDirectMethodHierByDescriptor(const ClassObject* clazz,
    const char* methodName, const char* descriptor)
{
    return findMethodInListByDescriptor(clazz, FALSE, TRUE,
            methodName, descriptor);
}

/*
 * Find a "direct" method (static or "<*init>").
 *
 * Returns NULL if the method can't be found.  (Does not throw an exception.)
 */
Method* dvmFindDirectMethod(const ClassObject* clazz, const char* methodName,
    const DexProto* proto)
{
    return findMethodInListByProto(clazz, METHOD_DIRECT, FALSE, methodName,
            proto);
}

/*
 * Find a "direct" method in a class.  If we don't find it, try the
 * superclass.
 *
 * Returns NULL if the method can't be found.  (Does not throw an exception.)
 */
Method* dvmFindDirectMethodHier(const ClassObject* clazz,
    const char* methodName, const DexProto* proto)
{
    return findMethodInListByProto(clazz, METHOD_DIRECT, TRUE, methodName,
            proto);
}

/*
 * Find a virtual or static method in a class.  If we don't find it, try the
 * superclass.  This is compatible with the VM spec (v2 5.4.3.3) method
 * search order, but it stops short of scanning through interfaces (which
 * should be done after this function completes).
 *
 * In most cases we know that we're looking for either a static or an
 * instance field and there's no value in searching through both types.
 * During verification we need to recognize and reject certain unusual
 * situations, and we won't see them unless we walk the lists this way.
 *
 * Returns NULL if the method can't be found.  (Does not throw an exception.)
 */
Method* dvmFindMethodHier(const ClassObject* clazz, const char* methodName,
    const DexProto* proto)
{
    return findMethodInListByProto(clazz, METHOD_UNKNOWN, TRUE, methodName,
            proto);
}


/*
 * We have a method pointer for a method in "clazz", but it might be
 * pointing to a method in a derived class.  We want to find the actual entry
 * from the class' vtable.  If "clazz" is an interface, we have to do a
 * little more digging.
 *
 * For "direct" methods (private / constructor), we just return the
 * original Method.
 *
 * (This is used for reflection and JNI "call method" calls.)
 */
const Method* dvmGetVirtualizedMethod(const ClassObject* clazz,
    const Method* meth)
{
    Method* actualMeth;
    int methodIndex;

    if (dvmIsDirectMethod(meth)) {
        /* no vtable entry for these */
        //assert(!dvmIsStaticMethod(meth));
        return meth;
    }

    /*
     * If the method was declared in an interface, we need to scan through
     * the class' list of interfaces for it, and find the vtable index
     * from that.
     *
     * TODO: use the interface cache.
     */
    if (dvmIsInterfaceClass(meth->clazz)) {
        int i;

        for (i = 0; i < clazz->iftableCount; i++) {
            if (clazz->iftable[i].clazz == meth->clazz)
                break;
        }
        if (i == clazz->iftableCount) {
            //dvmThrowIncompatibleClassChangeError(
            //    "invoking method from interface not implemented by class");
            //TODO: Throw Class error here?
            return NULL;
        }

        methodIndex = clazz->iftable[i].methodIndexArray[meth->methodIndex];
    } else {
        methodIndex = meth->methodIndex;
    }

    //assert(methodIndex >= 0 && methodIndex < clazz->vtableCount);
    actualMeth = clazz->vtable[methodIndex];

    /*
     * Make sure there's code to execute.
     */
    if (dvmIsAbstractMethod(actualMeth)) {
        //dvmThrowAbstractMethodError(NULL);
        //TODO: throw class error here?
        return NULL;
    }
    //assert(!dvmIsMirandaMethod(actualMeth));

    return actualMeth;
}

/*
 * Get the source file for a method.
 */
const char* dvmGetMethodSourceFile(const Method* meth)
{
    /*
     * TODO: A method's debug info can override the default source
     * file for a class, so we should account for that possibility
     * here.
     */
    return meth->clazz->sourceFile;
}

/*
 * Dump some information about an object.
 */
void dvmDumpObject(const Object* obj)
{
    //nothing to do until now.
}


/*
 * Get the method ID for a static method in a class.
 */
Method * dvmGetStaticMethodID(ClassObject * clazz, const char* name, const char* sig) 
{
	Method* meth = NULL; 
	if (!dvmIsClassInitialized(clazz) && !dvmInitClass(clazz)) {

		return NULL;
    }

    meth = dvmFindDirectMethodHierByDescriptor(clazz, name, sig);

    /* make sure it's static, not virtual+private */
    if (meth != NULL && !dvmIsStaticMethod(meth)) {

        meth = NULL;
    }
    return meth;
}