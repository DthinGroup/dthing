/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2013/06/10 $
 * Last modified:	$Date: 2013/06/25 $
 * Version:         $ID: stdint.h#1
 */

#ifndef __DVM_STDINT_H__
#define __DVM_STDINT_H__

// signed fixed-width integers

#ifdef __cplusplus
extern "C" {
#endif

typedef signed char int8_t;
typedef short       int16_t;
typedef long        int32_t;
typedef __int64     int64_t;

#define INT8_C(x)  (x)
#define INT16_C(x) (x)
#define INT32_C(x) (x ## L)
#define INT64_C(x) (x ## I64)

#define INT8_MAX  INT8_C( 0x7f)
#define INT16_MAX INT16_C(0x7fff)
#define INT32_MAX INT32_C(0x7fffffff)
#define INT64_MAX INT64_C(0x7fffffffffffffff)

#define INT8_MIN  (-128)
#define INT16_MIN (-32768)
#define INT32_MIN (-INT32_MAX-1)
#define INT64_MIN (-INT64_MAX-1)


// unsigned fixed-width integers

typedef unsigned char    uint8_t;
typedef unsigned short   uint16_t;
typedef unsigned long    uint32_t;
typedef unsigned __int64 uint64_t;

#define UINT8_C(x)  (x ## U)
#define UINT16_C(x) (x ## U)
#define UINT32_C(x) (x ## UL)
#define UINT64_C(x) (x ## UI64)

#define UINT8_MAX  UINT8_C( 0xff)
#define UINT16_MAX UINT16_C(0xffff)
#define UINT32_MAX UINT32_C(0xffffffff)
#define UINT64_MAX UINT64_C(0xffffffffffffffff)

// signed minimum-width integers

typedef int8_t  int_least8_t;
typedef int16_t int_least16_t;
typedef int32_t int_least32_t;
typedef int64_t int_least64_t;

#define INT_LEAST8_MIN  INT8_MIN
#define INT_LEAST16_MIN INT16_MIN
#define INT_LEAST32_MIN INT32_MIN
#define INT_LEAST64_MIN INT64_MIN

#define INT_LEAST8_MAX  INT8_MAX
#define INT_LEAST16_MAX INT16_MAX
#define INT_LEAST32_MAX INT32_MAX
#define INT_LEAST64_MAX INT64_MAX

// unsigned minimum-width integers

typedef uint8_t  uint_least8_t;
typedef uint16_t uint_least16_t;
typedef uint32_t uint_least32_t;
typedef uint64_t uint_least64_t;

#define UINT_LEAST8_MAX  UINT8_MAX
#define UINT_LEAST16_MAX UINT16_MAX
#define UINT_LEAST32_MAX UINT32_MAX
#define UINT_LEAST64_MAX UINT64_MAX

// signed fast integers with minimum width

typedef int32_t int_fast8_t;
typedef int32_t int_fast16_t;
typedef int32_t int_fast32_t;
typedef int64_t int_fast64_t;

#define INT_FAST8_MIN  INT32_MIN
#define INT_FAST16_MIN INT32_MIN
#define INT_FAST32_MIN INT32_MIN
#define INT_FAST64_MIN INT64_MIN

#define INT_FAST8_MAX  INT32_MAX
#define INT_FAST16_MAX INT32_MAX
#define INT_FAST32_MAX INT32_MAX
#define INT_FAST64_MAX INT64_MAX

// unsigned fast integers with minimum width

typedef uint32_t uint_fast8_t;
typedef uint32_t uint_fast16_t;
typedef uint32_t uint_fast32_t;
typedef uint64_t uint_fast64_t;

#define UINT_FAST8_MAX  UINT32_MAX
#define UINT_FAST16_MAX UINT32_MAX
#define UINT_FAST32_MAX UINT32_MAX
#define UINT_FAST64_MAX UINT64_MAX

// integers capable of holding pointers

#ifndef _INTPTR_T_DEFINED
typedef int32_t  intptr_t;
// Avoid redefinitions in MSC 7.x headers
#define _INTPTR_T_DEFINED
#endif

#ifndef _UINTPTR_T_DEFINED
typedef uint32_t uintptr_t;
// Avoid redefinitions in MSC 7.x headers
#define _UINTPTR_T_DEFINED

#define INTPTR_MIN  INT32_MIN
#define INTPTR_MAX  INT32_MAX
#define UINTPTR_MAX UINT32_MAX
#endif

// signed maximum-width integer

typedef int64_t intmax_t;

#define INTMAX_MIN INT64_MIN
#define INTMAX_MAX INT64_MAX

#define INTMAX_C(x) INT64_C(x)

// signed maximum-width integer

typedef uint64_t uintmax_t;

#define UINTMAX_MAX UINT64_MAX

#define UINTMAX_C(x) UINT64_C(x)

// limits of types defined elsewhere

#ifndef PTRDIFF_MIN
#define PTRDIFF_MIN INT32_MIN
#define PTRDIFF_MAX INT32_MAX
#endif

#ifndef SIG_ATOMIC_MIN
#define SIG_ATOMIC_MIN INT32_MIN
#endif
#ifndef SIG_ATOMIC_MAX
#define SIG_ATOMIC_MAX INT32_MAX
#endif

#ifndef SIZE_MAX
#define SIZE_MAX UINT32_MAX
#endif

#ifndef WCHAR_MIN
#define WCHAR_MIN (0)
#define WCHAR_MAX UINT16_MAX
#endif

#ifndef WINT_MIN
#define WINT_MIN INT32_MIN
#define WINT_MAX INT32_MAX
#endif

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

#ifdef __cplusplus
}
#endif

#endif