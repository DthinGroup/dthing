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
 * Helper functions to access data fields in Objects.
 */
#ifndef DALVIK_OO_OBJECTINLINES_H_
#define DALVIK_OO_OBJECTINLINES_H_

#ifdef __cplusplus
#extern "C" {
#endif

/*
 * Store a single value in the array, and if the value isn't null,
 * note in the write barrier.
 */
void dvmSetObjectArrayElement(const ArrayObject* obj, int index, Object* val);


/*
 * Field access functions.  Pass in the word offset from Field->byteOffset.
 *
 * We guarantee that long/double field data is 64-bit aligned, so it's safe
 * to access them with ldrd/strd on ARM.
 *
 * The VM treats all fields as 32 or 64 bits, so the field set functions
 * write 32 bits even if the underlying type is smaller.
 *
 * Setting Object types to non-null values includes a call to the
 * write barrier.
 */
#define BYTE_OFFSET(_ptr, _offset)  ((void*) (((u1*)(_ptr)) + (_offset)))

JValue* dvmFieldPtr(const Object* obj, int offset);

bool_t dvmGetFieldBoolean(const Object* obj, int offset);

s1 dvmGetFieldByte(const Object* obj, int offset);

s2 dvmGetFieldShort(const Object* obj, int offset);

u2 dvmGetFieldChar(const Object* obj, int offset);

s4 dvmGetFieldInt(const Object* obj, int offset);

s8 dvmGetFieldLong(const Object* obj, int offset);

float dvmGetFieldFloat(const Object* obj, int offset);

double dvmGetFieldDouble(const Object* obj, int offset);

Object* dvmGetFieldObject(const Object* obj, int offset);

s8 dvmGetFieldLongVolatile(const Object* obj, int offset);

#if 0
bool_t dvmGetFieldBooleanVolatile(const Object* obj, int offset);

s1 dvmGetFieldByteVolatile(const Object* obj, int offset);

s2 dvmGetFieldShortVolatile(const Object* obj, int offset);

u2 dvmGetFieldCharVolatile(const Object* obj, int offset);

s4 dvmGetFieldIntVolatile(const Object* obj, int offset);

float dvmGetFieldFloatVolatile(const Object* obj, int offset);



double dvmGetFieldDoubleVolatile(const Object* obj, int offset);

Object* dvmGetFieldObjectVolatile(const Object* obj, int offset);
#endif

void dvmSetFieldBoolean(Object* obj, int offset, bool_t val);

void dvmSetFieldByte(Object* obj, int offset, s1 val);

void dvmSetFieldShort(Object* obj, int offset, s2 val);

void dvmSetFieldChar(Object* obj, int offset, u2 val);

void dvmSetFieldInt(Object* obj, int offset, s4 val);

void dvmSetFieldFloat(Object* obj, int offset, float val);

void dvmSetFieldLong(Object* obj, int offset, s8 val);

void dvmSetFieldDouble(Object* obj, int offset, double val);

void dvmSetFieldObject(Object* obj, int offset, Object* val);

void dvmSetFieldLongVolatile(Object* obj, int offset, s8 val);

#if 0
void dvmSetFieldIntVolatile(Object* obj, int offset, s4 val);

void dvmSetFieldBooleanVolatile(Object* obj, int offset, bool_t val);

void dvmSetFieldByteVolatile(Object* obj, int offset, s1 val);

void dvmSetFieldShortVolatile(Object* obj, int offset, s2 val);

void dvmSetFieldCharVolatile(Object* obj, int offset, u2 val);

void dvmSetFieldFloatVolatile(Object* obj, int offset, float val);



void dvmSetFieldDoubleVolatile(Object* obj, int offset, double val);

void dvmSetFieldObjectVolatile(Object* obj, int offset, Object* val);
#endif

/*
 * Static field access functions.
 */
JValue* dvmStaticFieldPtr(const StaticField* sfield);

bool_t dvmGetStaticFieldBoolean(const StaticField* sfield);

s1 dvmGetStaticFieldByte(const StaticField* sfield);

s2 dvmGetStaticFieldShort(const StaticField* sfield);

u2 dvmGetStaticFieldChar(const StaticField* sfield);

s4 dvmGetStaticFieldInt(const StaticField* sfield);

float dvmGetStaticFieldFloat(const StaticField* sfield);

s8 dvmGetStaticFieldLong(const StaticField* sfield);

double dvmGetStaticFieldDouble(const StaticField* sfield);

Object* dvmGetStaticFieldObject(const StaticField* sfield);

s8 dvmGetStaticFieldLongVolatile(const StaticField* sfield);
#if 0
bool_t dvmGetStaticFieldBooleanVolatile(const StaticField* sfield);

s1 dvmGetStaticFieldByteVolatile(const StaticField* sfield);

s2 dvmGetStaticFieldShortVolatile(const StaticField* sfield);

u2 dvmGetStaticFieldCharVolatile(const StaticField* sfield);

s4 dvmGetStaticFieldIntVolatile(const StaticField* sfield);

float dvmGetStaticFieldFloatVolatile(const StaticField* sfield);



double dvmGetStaticFieldDoubleVolatile(const StaticField* sfield);

Object* dvmGetStaticFieldObjectVolatile(const StaticField* sfield);
#endif

void dvmSetStaticFieldBoolean(StaticField* sfield, bool_t val);

void dvmSetStaticFieldByte(StaticField* sfield, s1 val);

void dvmSetStaticFieldShort(StaticField* sfield, s2 val);

void dvmSetStaticFieldChar(StaticField* sfield, u2 val);

void dvmSetStaticFieldInt(StaticField* sfield, s4 val);

void dvmSetStaticFieldFloat(StaticField* sfield, float val);

void dvmSetStaticFieldLong(StaticField* sfield, s8 val);

void dvmSetStaticFieldDouble(StaticField* sfield, double val);

void dvmSetStaticFieldObject(StaticField* sfield, Object* val);

void dvmSetStaticFieldLongVolatile(StaticField* sfield, s8 val);

#if 0
void dvmSetStaticFieldIntVolatile(StaticField* sfield, s4 val);

void dvmSetStaticFieldBooleanVolatile(StaticField* sfield, bool val);

void dvmSetStaticFieldByteVolatile(StaticField* sfield, s1 val);

void dvmSetStaticFieldShortVolatile(StaticField* sfield, s2 val);

void dvmSetStaticFieldCharVolatile(StaticField* sfield, u2 val);

void dvmSetStaticFieldFloatVolatile(StaticField* sfield, float val);



void dvmSetStaticFieldDoubleVolatile(StaticField* sfield, double val);

void dvmSetStaticFieldObjectVolatile(StaticField* sfield, Object* val);
#endif


bool_t dvmIsPublicMethod(const Method* method);

bool_t dvmIsPrivateMethod(const Method* method);

bool_t dvmIsStaticMethod(const Method* method);

bool_t dvmIsSynchronizedMethod(const Method* method);

bool_t dvmIsDeclaredSynchronizedMethod(const Method* method);

bool_t dvmIsFinalMethod(const Method* method);

bool_t dvmIsNativeMethod(const Method* method);

bool_t dvmIsAbstractMethod(const Method* method);

bool_t dvmIsSyntheticMethod(const Method* method) ;

#if 0
bool_t dvmIsMirandaMethod(const Method* method);

#endif
bool_t dvmIsConstructorMethod(const Method* method);

/* Dalvik puts private, static, and constructors into non-virtual table */
bool_t dvmIsDirectMethod(const Method* method);

/* Get whether the given method has associated bytecode. This is the
 * case for methods which are neither native nor abstract. */
bool_t dvmIsBytecodeMethod(const Method* method);


bool_t dvmIsProtectedField(const Field* field);

bool_t dvmIsStaticField(const Field* field);

bool_t dvmIsFinalField(const Field* field);

bool_t dvmIsVolatileField(const Field* field);


bool_t dvmIsInterfaceClass(const ClassObject* clazz);

bool_t dvmIsPublicClass(const ClassObject* clazz);

bool_t dvmIsFinalClass(const ClassObject* clazz);

bool_t dvmIsAbstractClass(const ClassObject* clazz);

bool_t dvmIsAnnotationClass(const ClassObject* clazz);

bool_t dvmIsPrimitiveClass(const ClassObject* clazz);


/* linked, here meaning prepared and resolved */
bool_t dvmIsClassLinked(const ClassObject* clazz);

/* has class been verified? */
bool_t dvmIsClassVerified(const ClassObject* clazz);


/*
 * Return whether the given object is an instance of Class.
 */
bool_t dvmIsClassObject(const Object* obj);


/*
 * Return whether the given object is the class Class (that is, the
 * unique class which is an instance of itself).
 */
bool_t dvmIsTheClassClass(const ClassObject* clazz);


/*
 * Determine if a class has been initialized.
 */
bool_t dvmIsClassInitialized(const ClassObject* clazz);

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
bool_t dvmIsClassInitializing(const ClassObject* clazz);

/*
 * Get the associated code struct for a method. This returns NULL
 * for non-bytecode methods.
 */
const DexCode* dvmGetMethodCode(const Method* meth);


/*
 * Get the size of the insns associated with a method. This returns 0
 * for non-bytecode methods.
 */
u4 dvmGetMethodInsnsSize(const Method* meth);

/*
 * Determine whether "sub" is a sub-class of "clazz".
 *
 * Returns 0 (false) if not, 1 (true) if so.
 */
int dvmIsSubClass(const ClassObject* sub, const ClassObject* clazz);

#ifdef __cplusplus
}
#endif


#endif  // DALVIK_OO_OBJECTINLINES_H_
