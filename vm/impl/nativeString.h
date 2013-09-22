/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2013/09/22 $
 * Last modified:	$Date: 2013/09/22 $
 * Version:         $ID: nativeString.h#1
 */

/**
 * The implementation of java.lang.String.
 */

#ifndef __NATIVE_STRING_H__
#define __NATIVE_STRING_H__
#include <dthing.h>
#include <kni.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Returns an interned string equal to this string. The VM maintains an internal set of
 * unique strings. All string literals found in loaded classes'
 * constant pools are automatically interned. Manually-interned strings are only weakly
 * referenced, so calling {@code intern} won't lead to unwanted retention.
 *
 * <p>Interning is typically used because it guarantees that for interned strings
 * {@code a} and {@code b}, {@code a.equals(b)} can be simplified to
 * {@code a == b}. (This is not true of non-interned strings.)
 *
 * <p>Many applications find it simpler and more convenient to use an explicit
 * {@link java.util.HashMap} to implement their own pools.
 */
void Java_java_lang_String_intern(const u4* args, JValue* pResult);

#ifdef __cplusplus
}
#endif

#endif //__NATIVE_STRING_H__