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


#ifdef __cplusplus
}
#endif

#endif //_DALVIK_OO_CLASS_H_
