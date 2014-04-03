#include <common.h>

/* Header for class java.lang.AsyncIO */

#ifndef __NATIVE_ASYNCIO_H__
#define __NATIVE_ASYNCIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Class:     java_lang_AsyncIO
 * Method:    getCurNotifierState
 * Signature: ()I
 */
void Java_java_lang_AsyncIO_getCurNotifierState(const u4* args, JValue* pResult);

/**
 * Class:     java_lang_AsyncIO
 * Method:    setCurNotifierState
 * Signature: (I)I
 */
void Java_java_lang_AsyncIO_setCurNotifierState(const u4* args, JValue* pResult);

/**
 * Class:     java_lang_AsyncIO
 * Method:    waitSignalOrTimeOut
 * Signature: ()V
 */
void Java_java_lang_AsyncIO_waitSignalOrTimeOut(const u4* args, JValue* pResult);

#ifdef __cplusplus
}
#endif
#endif // __NATIVE_ASYNCIO_H__
