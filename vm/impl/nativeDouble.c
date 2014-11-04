#include <vm_common.h>
#include "nativeDouble.h"

/**
 * Class:     java_lang_Double
 * Method:    doubleToLongBits
 * Signature: (D)J
 */
void Java_java_lang_Double_doubleToLongBits(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jdouble value = (jdouble) args[1];
    jlong ret = 0;

    // TODO: implementation

    RETURN_LONG(ret);
}

/**
 * Class:     java_lang_Double
 * Method:    doubleToRawLongBits
 * Signature: (D)J
 */
void Java_java_lang_Double_doubleToRawLongBits(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jdouble value = (jdouble) args[1];
    jlong ret = 0;

    // TODO: implementation

    RETURN_LONG(ret);
}

/**
 * Class:     java_lang_Double
 * Method:    longBitsToDouble
 * Signature: (J)D
 */
void Java_java_lang_Double_longBitsToDouble(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jlong bits = (jlong) args[1];
    jdouble ret = 0.0;

    // TODO: implementation

    RETURN_DOUBLE(ret);
}

