/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2013/06/25 $
 * Last modified:	$Date: 2013/06/28 $
 * Version:         $ID: heap.c#1
 */


#ifndef __DTHING_H__
#define __DTHING_H__

#include <std_global.h>
#include <common.h>
#include <object.h>
#include <hash.h>
#include <class.h>

#ifdef __cplusplus 
extern "C" {
#endif

typedef struct DVMGlobal_s
{
    /**
     * Class path string, used for dynamically load java libraries. There are
     * 2 situations: 
     * First, java driver libraries. Such as I2C, COM, embedded devices 
     *     drivers and so on.
     * Second, thrid-party extern libraries. Such as functional java libs.
     */
    char* classPathStr;

    /**
     * Application package path. Used for launch an applications.
     */
    char* appPathStr;

    /*
     * Loaded classes, hashed by class name.  Each entry is a ClassObject*,
     * allocated in Heap space.
     * Note:
     *      All classes, including bootstrap classes, dynamically classes and
     *      application classes are recorded in the hash table for quick looking up.
     */
    HashTable* loadedClasses;

    /**
     * classes entry. Used to record different class path entries.
     * 1. bootstrap classes.
     * 2. third-party/java driver class path.
     * 3. java application class path.
     */
    ClassesEntry* pClsEntry;

    /* the class Class */
    ClassObject* classJavaLangClass;

    /* public java.lang.object class */
    ClassObject* classJavaLangObject;

    /* synthetic classes representing primitive types */
    ClassObject* typeVoid;
    ClassObject* typeBoolean;
    ClassObject* typeByte;
    ClassObject* typeShort;
    ClassObject* typeChar;
    ClassObject* typeInt;
    ClassObject* typeLong;
    ClassObject* typeFloat;
    ClassObject* typeDouble;

    /* synthetic classes for arrays of primitives */
    ClassObject* classArrayBoolean;
    ClassObject* classArrayByte;
    ClassObject* classArrayShort;
    ClassObject* classArrayChar;
    ClassObject* classArrayInt;
    ClassObject* classArrayLong;
    ClassObject* classArrayFloat;
    ClassObject* classArrayDouble;

    /*
     * Quick lookups for popular classes used internally.
     */
    ClassObject* classJavaLangString;

    /*
     * Pre-allocated throwables.
     */
    Object*     outOfMemoryObj;
    Object*     internalErrorObj;
    Object*     noClassDefFoundErrorObj;

    /*
     * Compute some stats on loaded classes.
     */
    int  numLoadedClasses;
    int  numDeclaredMethods;
    int  numDeclaredInstFields;
    int  numDeclaredStaticFields;


} DVMGlobal;

extern GLOBAL DVMGlobal gDvm;

#ifdef __cplusplus 
}
#endif

#endif //__DTHING_H__
