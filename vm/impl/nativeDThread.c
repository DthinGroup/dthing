#include <vm_common.h>
#include "nativeDThread.h"

/**
 * Class:     java_lang_DThread
 * Method:    start0
 * Signature: ()V
 */
void Java_java_lang_Thread_start(const u4* args, JValue* pResult) {

    // TODO: implementation

    // return type : void
}

/**
 * Class:     java_lang_DThread
 * Method:    sleep0
 * Signature: (J)V
 */
void Java_java_lang_Thread_sleep(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jlong ms = (jlong) args[1];

    // TODO: implementation

    // return type : void
}

/**
 * Class:     java_lang_DThread
 * Method:    activeCount0
 * Signature: ()I
 */
void Java_java_lang_Thread_activeCount(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     java_lang_DThread
 * Method:    currentThread0
 * Signature: ()Ljava/lang/DThread;
 */
void Java_java_lang_Thread_currentThread(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];

    // TODO: implementation

    // return type : DThread
}

/**
 * Class:     java_lang_DThread
 * Method:    isAlive0
 * Signature: ()Z
 */
void Java_java_lang_Thread_isAlive(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jboolean ret = FALSE;

    // TODO: implementation
    RETURN_BOOLEAN(ret);
}

