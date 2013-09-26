
#ifndef __NATIVE_THREAD_H__
#define __NATIVE_THREAD_H__

#include <Object.h>

#undef  DThread_MAX_PRIORITY
#define DThread_MAX_PRIORITY 10L
#undef  DThread_MIN_PRIORITY
#define DThread_MIN_PRIORITY 1L
#undef  DThread_NORM_PRIORITY
#define DThread_NORM_PRIORITY 5L
#undef  DThread_MAX_SLEEP
#define DThread_MAX_SLEEP 9223372036854775LL


Method * GetMethodID(ClassObject * clazz, const char* name, const char* sig);

/*Thread.java*/
void Java_java_lang_Thread_activeCount0(const u4* args, JValue* pResult);

void Java_java_lang_Thread_currentThread0(const u4* args, JValue* pResult);

void Java_java_lang_Thread_sleep0(const u4* args, JValue* pResult);

void Java_java_lang_Thread_start0(const u4* args, JValue* pResult);

void Java_java_lang_Thread_isAlive0(const u4* args, JValue* pResult);

void Java_java_lang_Thread_setPriority0(const u4* args, JValue* pResult);

void Java_java_lang_Thread_printQ(const u4* args, JValue* pResult);

#endif