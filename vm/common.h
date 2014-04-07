/**
 * Copyright (C) 2013-2014 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2013/06/25 $
 * Last modified:   $Date: 2014/04/07 $
 * Version:         $ID: heap.c#1
 */


#ifndef __DTHING_COMMON_H__
#define __DTHING_COMMON_H__

#include <std_global.h>


/*
 * These match the definitions in the VM specification.
 */
#ifndef VM_BASETYPES
#define VM_BASETYPES
typedef uint8_t  u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;
typedef int8_t   s1;
typedef int16_t  s2;
typedef int32_t  s4;
typedef int64_t  s8;
#endif // VM_BASETYPES

typedef u1     jboolean;
typedef s1     jbyte;
typedef u2     jchar;
typedef s2     jshort;
typedef s4     jint;
typedef s8     jlong;
typedef float  jfload;
typedef double jdouble;

/*
 * Storage for primitive types and object references.
 *
 * Some parts of the code (notably object field access) assume that values
 * are "left aligned", i.e. given "JValue jv", "jv.i" and "*((s4*)&jv)"
 * yield the same result.  This seems to be guaranteed by gcc on big- and
 * little-endian systems.
 */

typedef union JValue_u {
    jboolean z;
    jbyte    b;
    jchar    c;
    jshort   s;
    jint     i;
    jlong    j;
    jfload   f;
    jdouble  d;
    void*    l;
} JValue;

#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))

#endif //__DTHING_COMMON_H__