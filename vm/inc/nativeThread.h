
#ifndef __NATIVE_THREAD_H__
#define __NATIVE_THREAD_H__

void Java_java_lang_Thread_activeCount(void);

void Java_java_lang_Thread_currentThread(void);

void Java_java_lang_Thread_yield(void);

void Java_java_lang_Thread_sleep(void);

void Java_java_lang_Thread_start(void);

void Java_java_lang_Thread_isAlive(void);

void Java_java_lang_Thread_setPriority0(void);

void Java_java_lang_Thread_interrupt0(void);

#endif