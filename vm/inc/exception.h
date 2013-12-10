/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Exception handling.
 */
#ifndef DALVIK_EXCEPTION_H_
#define DALVIK_EXCEPTION_H_

#ifdef __cplusplus
extern "C" {
#endif


typedef enum InitKind_e{
    kInitUnknown,
    kInitNoarg,
    kInitMsg,
    kInitMsgThrow,
    kInitThrow
} InitKind;

void dvmThrowChainedException(ClassObject* excepClass, const char* msg, Object* cause);


/*
 * Create some "stock" exceptions.  These can be thrown when the system is
 * too screwed up to allocate and initialize anything, or when we don't
 * need a meaningful stack trace.
 *
 * We can't do this during the initial startup because we need to execute
 * the constructors.
 */
bool_t dvmCreateStockExceptions();

/*
 * Create a Throwable and throw an exception in the current thread (where
 * "throwing" just means "set the thread's exception pointer").
 *
 * "msg" and/or "cause" may be NULL.
 *
 * If we have a bad exception hierarchy -- something in Throwable.<init>
 * is missing -- then every attempt to throw an exception will result
 * in another exception.  Exceptions are generally allowed to "chain"
 * to other exceptions, so it's hard to auto-detect this problem.  It can
 * only happen if the system classes are broken, so it's probably not
 * worth spending cycles to detect it.
 *
 * We do have one case to worry about: if the classpath is completely
 * wrong, we'll go into a death spin during startup because we can't find
 * the initial class and then we can't find NoClassDefFoundError.  We have
 * to handle this case.
 *
 * [Do we want to cache pointers to common exception classes?]
 */
void dvmThrowException(const char* exceptionDescriptor, const char* msg);

/**
 * Throw an InstantiationException in the current thread, with
 * the human-readable form of the given class as the detail message,
 * with optional extra detail appended to the message.
 */
void dvmThrowInstantiationException(ClassObject* clazz, const char* extraDetail);


/**
 * Throw an IllegalAccessException in the current thread, with the
 * given detail message.
 */
void dvmThrowIllegalAccessException(const char* msg);


/**
 * Throw a NullPointerException in the current thread, with the given
 * detail message.
 */
void dvmThrowNullPointerException(const char* msg);

/**
 * Throw a ArrayStoreException in the current thread, with the given
 * detail message.
 */
void dvmThrowArrayStoreExceptionNotArray(ClassObject* actual, const char* label);
#ifdef __cplusplus
}
#endif

#endif