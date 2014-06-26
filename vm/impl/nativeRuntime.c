/**
 * Copyright (C) 2013-2014 YarlungSoft. All Rights Reserved.
 *
 * Created:         $Date: 2014/04/03 $
 */

/**
 * The implementation of java.lang.Runtime.
 */

#include <nativeRuntime.h>
#include <gc.h>

/**
 * Class:     java_lang_Runtime
 * Method:    freeMemory
 * Signature: ()J
 */
void Java_java_lang_Runtime_freeMemory(const u4* args, JValue* pResult) {
    // TODO : implement
    DVMTraceInf("Java_java_lang_Runtime_freeMemory is not implemented..\n");
}

/**
 * Class:     java_lang_Runtime
 * Method:    totalMemory
 * Signature: ()J
 */
void Java_java_lang_Runtime_totalMemory(const u4* args, JValue* pResult) {
    // TODO : implement
    DVMTraceInf("Java_java_lang_Runtime_totalMemory is not implemented..\n");
}

/**
 * Class:     java_lang_Runtime
 * Method:    gc
 * Signature: ()V
 */
void Java_java_lang_Runtime_gc(const u4* args, JValue* pResult) {
    DVMTraceInf("Java_java_lang_Runtime_gc\n");
    dvmRunGC();
    RETURN_VOID();
}
