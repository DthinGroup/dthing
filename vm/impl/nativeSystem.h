/**
 * Copyright (C) 2013-2014 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2013/09/22 $
 * Last modified:   $Date: 2014/04/03 $
 * Version:         $ID: nativeSystem.h#1
 */

#include <dthing.h>
#include <kni.h>

/**
 * Header for the following classes:
 *     java.lang.System
 *     com.yarlungsoft.util.SystemInputStream
 *     com.yarlungsoft.util.SystemPrintStream
 */

#ifndef __NATIVE_SYSTEM_H__
#define __NATIVE_SYSTEM_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Class:     java_lang_System
 * Method:    arraycopy
 * Signature: (Ljava/lang/Object;ILjava/lang/Object;II)V
 *
 * Copies {@code length} elements from the array {@code src},
 * starting at offset {@code srcPos}, into the array {@code dst},
 * starting at offset {@code dstPos}.
 *
 * @param src
 *            the source array to copy the content.
 * @param srcPos
 *            the starting index of the content in {@code src}.
 * @param dst
 *            the destination array to copy the data into.
 * @param dstPos
 *            the starting index for the copied content in {@code dst}.
 * @param length
 *            the number of elements to be copied.
 */
void Java_java_lang_System_arraycopy(const u4* args, JValue* pResult);

/**
 * Class:     java_lang_System
 * Method:    currentTimeMillis
 * Signature: ()J
 *
 * Returns the current system time in milliseconds since January 1, 1970
 * 00:00:00 UTC. This method shouldn't be used for measuring timeouts or
 * other elapsed time measurements, as changing the system time can affect
 * the results.
 *
 * @return the local system time in milliseconds.
 */
void Java_java_lang_System_currentTimeMillis(const u4* args, JValue* pResult);

/**
 * Class:     java_lang_System
 * Method:    identityHashCode
 * Signature: (Ljava/lang/Object;)I
 *
 * Returns an integer hash code for the parameter. The hash code returned is
 * the same one that would be returned by the method {@code
 * java.lang.Object.hashCode()}, whether or not the object's class has
 * overridden hashCode(). The hash code for {@code null} is {@code 0}.
 *
 * @param anObject
 *            the object to calculate the hash code.
 * @return the hash code for the given object.
 * @see java.lang.Object#hashCode
 */
void Java_java_lang_System_identityHashCode(const u4* args, JValue* pResult);

/**
 * Class:     java_lang_System
 * Method:    getProperty
 * Signature: (Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 */
void Java_java_lang_System_getProperty(const u4* args, JValue* pResult);

/**
 * Class:     com_yarlungsoft_util_SystemInputStream
 * Method:    readN
 * Signature: ([BII)I
 */
void Java_com_yarlungsoft_util_SystemInputStream_readN(const u4* args, JValue* pResult);

/**
 * Class:     com_yarlungsoft_util_SystemPrintStream
 * Method:    flush
 * Signature: ()V
 */
void Java_com_yarlungsoft_util_SystemPrintStream_flush(const u4* args, JValue* pResult);

/**
 * Class:     com_yarlungsoft_util_SystemPrintStream
 * Method:    write
 * Signature: (I)V
 *
 * Output a single byte.
 * @param b byte to write
 */
void Java_com_yarlungsoft_util_SystemPrintStream_write(const u4* args, JValue* pResult);

#ifdef __cplusplus
}
#endif
#endif // __NATIVE_SYSTEM_H__
