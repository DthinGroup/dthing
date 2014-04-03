
#include <Object.h>

/* Header for class java.lang.Thread */

#ifndef __NATIVE_THREAD_H__
#define __NATIVE_THREAD_H__

#ifdef __cplusplus
extern "C" {
#endif

#undef  DThread_MAX_PRIORITY
#define DThread_MAX_PRIORITY 10L
#undef  DThread_MIN_PRIORITY
#define DThread_MIN_PRIORITY 1L
#undef  DThread_NORM_PRIORITY
#define DThread_NORM_PRIORITY 5L
#undef  DThread_MAX_SLEEP
#define DThread_MAX_SLEEP 9223372036854775LL


Method * GetMethodID(ClassObject * clazz, const char* name, const char* sig);

/**
 * Class:     java_lang_Thread
 * Method:    activeCount
 * Signature: ()I
 */
void Java_java_lang_Thread_activeCount(const u4* args, JValue* pResult);

/**
 * Class:     java_lang_Thread
 * Method:    currentThread
 * Signature: ()Ljava/lang/Thread;
 */
void Java_java_lang_Thread_currentThread(const u4* args, JValue* pResult);

/**
 * Class:     java_lang_Thread
 * Method:    interrupt
 * Signature: ()V
 */
void Java_java_lang_Thread_interrupt(const u4* args, JValue* pResult);

/**
 * Class:     java_lang_Thread
 * Method:    interrupted
 * Signature: ()Z
 */
void Java_java_lang_Thread_interrupted(const u4* args, JValue* pResult);

/**
 * Class:     java_lang_Thread
 * Method:    isAlive
 * Signature: ()Z
 */
void Java_java_lang_Thread_isAlive(const u4* args, JValue* pResult);

/**
 * Class:     java_lang_Thread
 * Method:    isInterrupted
 * Signature: ()Z
 */
void Java_java_lang_Thread_isInterrupted(const u4* args, JValue* pResult);

/**
 * Class:     java_lang_Thread
 * Method:    setPriority
 * Signature: (I)V
 */
void Java_java_lang_Thread_setPriority(const u4* args, JValue* pResult);

/**
 * Class:     java_lang_Thread
 * Method:    sleep
 * Signature: (JI)V
 */
void Java_java_lang_Thread_sleep(const u4* args, JValue* pResult);

/**
 * Class:     java_lang_Thread
 * Method:    start
 * Signature: ()V
 */
void Java_java_lang_Thread_start(const u4* args, JValue* pResult);

/**
 * Class:     java_lang_Thread
 * Method:    yield
 * Signature: ()V
 */
void Java_java_lang_Thread_yield(const u4* args, JValue* pResult);

/**
 * Class:     java_lang_Thread
 * Method:    holdsLock
 * Signature: (Ljava/lang/Object;)Z
 */
void Java_java_lang_Thread_holdsLock(const u4* args, JValue* pResult);

#ifdef __cplusplus
}
#endif
#endif // __NATIVE_THREAD_H__
