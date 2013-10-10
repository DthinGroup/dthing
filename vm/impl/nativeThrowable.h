#ifndef __NATIVE_THROWABLE_H__
#define __NATIVE_THROWABLE_H__

#include <dthing.h>
#include <kni.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
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


#endif //__NATIVE_THROWABLE_H__