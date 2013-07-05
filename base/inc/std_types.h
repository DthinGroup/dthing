/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2013/06/10 $
 * Last modified:	$Date: 2013/06/25 $
 * Version:         $ID: std_types.h#1
 */

#ifndef __DVM_TYPES_H__
#define __DVM_TYPES_H__

#if defined(__cplusplus)
/* The standard requires that stdint only supplies the various macros
 * if these are defined.
 */
#ifndef __STDC_CONSTANT_MACROS
# define __STDC_CONSTANT_MACROS
#endif

#ifndef __STDC_LIMIT_MACROS
# define __STDC_LIMIT_MACROS
#endif

#endif /* __cplusplus */

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

/** Unused structure to perform compile-time size & sign checks.
 *
 * Any compilation errors produced by this structure indicate that there are
 * serious problems elsewhere with the size or sign of the underlying types
 * used by the Jbed sources. Do NOT delete or modify this structure if it does
 * not compile -- the types it is checking must be fixed.
 */
struct __unused_jbed_types_h_assertion___ {
    char check_size_int8_t[sizeof(int8_t) == 1 ? 1 : -1];
    char check_size_uint8_t[sizeof(uint8_t) == 1 ? 1 : -1];
    char check_size_int16_t[sizeof(int16_t) == 2 ? 1 : -1];
    char check_size_uint16_t[sizeof(uint16_t) == 2 ? 1 : -1];
    char check_size_int32_t[sizeof(int32_t) == 4 ? 1 : -1];
    char check_size_uint32_t[sizeof(uint32_t) == 4 ? 1 : -1];
    char check_size_int64_t[sizeof(int64_t) == 8 ? 1 : -1];
    char check_size_uint64_t[sizeof(uint64_t) == 8 ? 1 : -1];
    char check_sign_uint8_t[ ((uint8_t)-1) > 0 ? 1 : -1];
    char check_sign_int8_t[ ((int8_t)-1) < 0 ? 1 : -1];
    char check_sign_uint16_t[ ((uint16_t)-1) > 0 ? 1 : -1];
    char check_sign_int16_t[ ((int16_t)-1) < 0 ? 1 : -1];
    char check_sign_uint32_t[ ((uint32_t)-1) > 0 ? 1 : -1];
    char check_sign_int32_t[ ((int32_t)-1) < 0 ? 1 : -1];
};

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef unsigned int bool_t;

#if defined(__cplusplus)
}
#endif

#endif /* __DVM_TYPES_H__ */