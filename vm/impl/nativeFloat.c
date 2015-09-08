/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2013/09/24 $
 * Last modified:	$Date: 2013/09/24 $
 * Version:         $ID: nativeFloat.c#1
 */

/**
 * The implementation of java.lang.Float
 */

#include <nativeFloat.h>

static int isnanf(float f)
{
    IEEEf2bits u;

    u.f = f;
    return (u.bits.exp == 255 && u.bits.man != 0);
}

/**
 * Class:     java_lang_Float
 * Method:    floatToIntBits
 * Signature: (F)I
 */
void Java_java_lang_Float_floatToIntBits(const u4* args, JValue* pResult)
{
    Convert32 convert;
    convert.arg = args[1];
    //according to the spec of java.lang.float.floatToIntBits;
    pResult->i = isnanf(convert.ff) ? 0x7fc00000 : convert.arg;
}

/**
 * Class:     java_lang_Float
 * Method:    floatToRawIntBits
 * Signature: (F)I
 */
void Java_java_lang_Float_floatToRawIntBits(const u4* args, JValue* pResult)
{
    pResult->i = args[1];
}

/**
 * Class:     java_lang_Float
 * Method:    intBitsToFloat
 * Signature: (I)F
 */
void Java_java_lang_Float_intBitsToFloat(const u4* args, JValue* pResult)
{
    Convert32 convert;
    convert.arg = args[1];
    pResult->f = convert.ff;
}
