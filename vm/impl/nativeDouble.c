/**
* Copyright (C) 2013-2015 YarlungSoft. All Rights Reserved.
*
* Created:         $Date: 2013/09/24 $
* Last modified:   $Date: 2015/09/09 $
* Version:         $ID: nativeDouble.c#1
*/

/**
* The implementation of java.lang.Float
*/

#include <vm_common.h>
#include "nativeDouble.h"

#define LL

static int isnan(double d) {
    IEEEd2bits u;

    u.d = d;
    return (u.bits.exp == 2047 && (u.bits.manl != 0 || u.bits.manh != 0));
}

/**
 * Class:     java_lang_Double
 * Method:    doubleToLongBits
 * Signature: (D)J
 */
void Java_java_lang_Double_doubleToLongBits(const u4* args, JValue* pResult) {
    Convert64 convert;
    convert.arg[0] = args[1];
    convert.arg[1] = args[2];
    jlong ret = isnan(convert.dd) ? 0x7ff8000000000000 LL : convert.ll;
    RETURN_LONG(ret);
}

/**
 * Class:     java_lang_Double
 * Method:    doubleToRawLongBits
 * Signature: (D)J
 */
void Java_java_lang_Double_doubleToRawLongBits(const u4* args, JValue* pResult) {
    Convert64 convert;
    convert.arg[0] = args[1];
    convert.arg[1] = args[2];
    RETURN_LONG(convert.ll);
}

/**
 * Class:     java_lang_Double
 * Method:    longBitsToDouble
 * Signature: (J)D
 */
void Java_java_lang_Double_longBitsToDouble(const u4* args, JValue* pResult) {
    Convert64 convert;
    convert.arg[0] = args[1];
    convert.arg[1] = args[2];
    RETURN_LONG(convert.dd);
}
