/**
 * Copyright (C) 2013-2014 YarlungSoft. All Rights Reserved.
 *
 * Created:         $Date: 2013/10/10 $
 * Last modified:   $Date: 2014/04/03 $
 * Version:         $ID: nativeThrowable.h#1
 */

#include <dthing.h>
#include <kni.h>

/* Header for class java.lang.Throwable */

#ifndef __NATIVE_THROWABLE_H__
#define __NATIVE_THROWABLE_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Class:     java_lang_Throwable
 * Method:    printStackTrace0
 * Signature: (Ljava/lang/Object;)V
 *
 * Writes a printable representation of this {@code Throwable}'s stack trace
 * to the specified print stream. If the {@code Throwable} contains a
 * {@link #getCause() cause}, the method will be invoked recursively for
 * the nested {@code Throwable}.
 *
 * @param err
 *            the stream to write the stack trace on.
 */
void Java_java_lang_Throwable_printStackTrace0(const u4* args, JValue* pResult);

#ifdef __cplusplus
}
#endif
#endif // __NATIVE_THROWABLE_H__
