/**
 * Copyright (C) 2013-2014 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2013/09/24 $
 * Last modified:   $Date: 2014/04/03 $
 * Version:         $ID: nativeFloat.h#1
 */

#include <dthing.h>
#include <kni.h>

/* Header for class java.lang.Double and java.lang.Float. */

#ifndef __NATIVE_FLOAT_H__
#define __NATIVE_FLOAT_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef union Convert32_u {
    u4 arg;
    float ff;
} Convert32;

typedef union Convert64_u {
    u4 arg[2];
    s8 ll;
    double dd;
} Convert64;

typedef union IEEEf2bits_u {
    float f;
    struct {
#ifdef DVM_BIG_ENDIAN
        unsigned int sign :1;
        unsigned int exp :8;
        unsigned int man :23;
#else /* _BIG_ENDIAN */
        unsigned int man :23;
        unsigned int exp :8;
        unsigned int sign :1;
#endif
    } bits;
} IEEEf2bits;

typedef union IEEEd2bits_u {
    double d;
    struct {
#ifdef  DVM_BIG_ENDIAN
        unsigned int sign :1;
        unsigned int exp :11;
        unsigned int manh :20;
        unsigned int manl :32;
#else
        unsigned int manl :32;
        unsigned int manh :20;
        unsigned int exp :11;
        unsigned int sign :1;
#endif
    } bits;
} IEEEd2bits;

/**
 * Class:     java_lang_Float
 * Method:    floatToIntBits
 * Signature: (F)I
 *
 * Returns an integer corresponding to the bits of the given
 * <a href="http://en.wikipedia.org/wiki/IEEE_754-1985">IEEE 754</a> single precision
 * float {@code value}. All <em>Not-a-Number (NaN)</em> values are converted to a single NaN
 * representation ({@code 0x7fc00000}) (compare to {@link #floatToRawIntBits}).
 */
void Java_java_lang_Float_floatToIntBits(const u4* args, JValue* pResult);

/**
 * Class:     java_lang_Float
 * Method:    floatToRawIntBits
 * Signature: (F)I
 *
 * Returns an integer corresponding to the bits of the given
 * <a href="http://en.wikipedia.org/wiki/IEEE_754-1985">IEEE 754</a> single precision
 * float {@code value}. <em>Not-a-Number (NaN)</em> values are preserved (compare
 * to {@link #floatToIntBits}).
 */
void Java_java_lang_Float_floatToRawIntBits(const u4* args, JValue* pResult);

/**
 * Class:     java_lang_Float
 * Method:    intBitsToFloat
 * Signature: (I)F
 *
 * Returns the <a href="http://en.wikipedia.org/wiki/IEEE_754-1985">IEEE 754</a>
 * single precision float corresponding to the given {@code bits}.
 */
void Java_java_lang_Float_intBitsToFloat(const u4* args, JValue* pResult);

/**
 * Class:     java_lang_Double
 * Method:    doubleToLongBits
 * Signature: (D)J
 *
 * Returns an integer corresponding to the bits of the given
 * <a href="http://en.wikipedia.org/wiki/IEEE_754-1985">IEEE 754</a> double precision
 * {@code value}. All <em>Not-a-Number (NaN)</em> values are converted to a single NaN
 * representation ({@code 0x7ff8000000000000L}) (compare to {@link #doubleToRawLongBits}).
 */
void Java_java_lang_Double_doubleToLongBits(const u4* args, JValue* pResult);

/**
 * Class:     java_lang_Double
 * Method:    doubleToRawLongBits
 * Signature: (D)J
 *
 * Returns an integer corresponding to the bits of the given
 * <a href="http://en.wikipedia.org/wiki/IEEE_754-1985">IEEE 754</a> double precision
 * {@code value}. <em>Not-a-Number (NaN)</em> values are preserved (compare
 * to {@link #doubleToLongBits}).
 */
void Java_java_lang_Double_doubleToRawLongBits(const u4* args, JValue* pResult);

/**
 * Class:     java_lang_Double
 * Method:    longBitsToDouble
 * Signature: (J)D
 *
 * Returns the <a href="http://en.wikipedia.org/wiki/IEEE_754-1985">IEEE 754</a>
 * double precision float corresponding to the given {@code bits}.
 */
void Java_java_lang_Double_longBitsToDouble(const u4* args, JValue* pResult);

#ifdef __cplusplus
}
#endif
#endif // __NATIVE_FLOAT_H__
