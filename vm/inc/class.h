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
 * Class loader.
 */
#ifndef _DALVIK_OO_CLASS_H_
#define _DALVIK_OO_CLASS_H_

#include <common.h>
#include <object.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * usually, we will have 3 type entries:
 * 1. only 1 bootstrap classes entry.
 * 2. >= 0 third-party class library or java driver entries.
 * 3. only 1 java application entry.
 */
#define MAX_NUM_CLASSES_ENTRY  (10)

typedef struct ClassesEntry_s   ClassesEntry;
typedef enum ClassEntryKind_e   ClassEntryKind;

/*
 * The classpath and bootclasspath differ in that only the latter is
 * consulted when looking for classes needed by the VM.  When searching
 * for an arbitrary class definition, we start with the bootclasspath,
 * look for optional packages (a/k/a standard extensions), and then try
 * the classpath.
 *
 * In Dalvik, a class can be found in one of 3 ways:
 *  - in a .dex file
 *  - in a .dex file named specifically "classes.dex", which is held
 *    inside a jar file
 *  - in the rom. class is converted to binary buffer and compiled into rom.
 *    Only used for bootstrap classes.
 *
 * These two may be freely intermixed in a classpath specification.
 * Ordering is significant.
 */
enum ClassEntryKind_e {
    kCpeUnused = 0,
    kCpeJar,
    kCpeDex,
    KCpeRom,
    kCpeMax      /* used as sentinel at end of array */
};


struct ClassesEntry_s {
    ClassEntryKind kind;
    char*   fileName;   /* Only valid for kCpeJar and kCpeDex */
    int32_t fnameLen;   /* Only valid for kCpeJar and kCpeDex */
    DvmDex* pDvmDex;
} ;

/*
 * Utility functions.
 */
ClassObject* dvmLookupClass(const char* descriptor);


/*
 * Initialize the bootstrap class loader.
 *
 * Call this after the bootclasspath string has been finalized.
 */
bool_t dvmClassStartup();

void dvmClassShutdown();

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
ClassObject* dvmFindClass(const char* descriptor);

/*
 * Find the class object representing the primitive type with the
 * given descriptor. This returns NULL if the given type character
 * is invalid.
 */
ClassObject* dvmFindPrimitiveClass(char type);

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
ClassObject* dvmFindClassNoInit(const char* descriptor);


/*
 * Store a copy of the method prototype descriptor string
 * for the given method into the given DexStringCache, returning the
 * stored string for convenience.
 */
char* dvmCopyDescriptorStringFromMethod(const Method* method, DexStringCache *pCache);

/*
 * Compute the number of argument words (u4 units) required by the
 * given method's prototype. For example, if the method descriptor is
 * "(IJ)D", this would return 3 (one for the int, two for the long;
 * return value isn't relevant).
 */
int dvmComputeMethodArgsSize(const Method* method);


/*
 * Compare the two method prototypes. The two prototypes are compared
 * as if by strcmp() on the result of dexProtoGetMethodDescriptor().
 */
int dvmCompareMethodProtos(const Method* method1, const Method* method2);


/*
 * Compare the two method prototypes, considering only the parameters
 * (i.e. ignoring the return types). The two prototypes are compared
 * as if by strcmp() on the result of dexProtoGetMethodDescriptor().
 */
int dvmCompareMethodParameterProtos(const Method* method1, const Method* method2);

/*
 * Compare a method descriptor string with the prototype of a method,
 * as if by converting the descriptor to a DexProto and comparing it
 * with dexProtoCompare().
 */
int dvmCompareDescriptorAndMethodProto(const char* descriptor, const Method* method);

/*
 * Compare the two method names and prototypes, a la strcmp(). The
 * name is considered the "major" order and the prototype the "minor"
 * order. The prototypes are compared as if by dvmCompareMethodProtos().
 */
int dvmCompareMethodNamesAndProtos(const Method* method1, const Method* method2);


/*
 * Compare the two method names and prototypes, a la strcmp(), ignoring
 * the return value. The name is considered the "major" order and the
 * prototype the "minor" order. The prototypes are compared as if by
 * dvmCompareMethodArgProtos().
 */
int dvmCompareMethodNamesAndParameterProtos(const Method* method1, const Method* method2);


/*
 * Compare a (name, prototype) pair with the (name, prototype) of
 * a method, a la strcmp(). The name is considered the "major" order and
 * the prototype the "minor" order. The descriptor and prototype are
 * compared as if by dvmCompareDescriptorAndMethodProto().
 */
int dvmCompareNameProtoAndMethod(const char* name, const DexProto* proto, const Method* method);


/*
 * Compare a (name, method descriptor) pair with the (name, prototype) of
 * a method, a la strcmp(). The name is considered the "major" order and
 * the prototype the "minor" order. The descriptor and prototype are
 * compared as if by dvmCompareDescriptorAndMethodProto().
 */
int dvmCompareNameDescriptorAndMethod(const char* name, const char* descriptor, const Method* method);

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
bool_t dvmAddClassToHash(ClassObject* clazz);


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
bool_t dvmInitClass(ClassObject* clazz);

/*
 * Find the named class (by descriptor), searching for it in the
 * bootclasspath.
 *
 * On failure, this returns NULL with an exception raised.
 */
ClassObject* dvmFindSystemClassNoInit(const char* descriptor);

void loadClassByPath(const char * path);

#ifdef __cplusplus
}
#endif

#endif //_DALVIK_OO_CLASS_H_
