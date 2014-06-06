/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2013/09/24 $
 * Last modified:	$Date: 2013/09/24 $
 * Version:         $ID: nativeFloat.c#1
 */

/**
 * The implementation of java.lang.Double and java.lang.Float.
 */

#include <nativeFloat.h>


static int isnan (double d)
{
	IEEEd2bits u;

	u.d = d;
	return (u.bits.exp == 2047 && (u.bits.manl != 0 || u.bits.manh != 0));
}

static int isnanf(float f)
{
	IEEEf2bits u;

	u.f = f;
	return (u.bits.exp == 255 && u.bits.man != 0);
}


/* see nativeFloat.h */
void Java_java_lang_Float_floatToIntBits(const u4* args, JValue* pResult)
{
    Convert32 convert;
    convert.arg = args[1];
    //according to the spec of java.lang.float.floatToIntBits;
    pResult->i = isnanf(convert.ff) ? 0x7fc00000 : convert.arg;
}

/* see nativeFloat.h */
void Java_java_lang_Float_floatToRawIntBits(const u4* args, JValue* pResult)
{
    pResult->i = args[1];
}

/* see nativeFloat.h */
void Java_java_lang_Float_intBitsToFloat(const u4* args, JValue* pResult)
{
    Convert32 convert;
    convert.arg = args[1];
    pResult->f = convert.ff;
}

#define LL
/* see nativeFloat.h */
void Java_java_lang_Double_doubleToLongBits(const u4* args, JValue* pResult)
{
    Convert64 convert;
    convert.arg[0] = args[1];
    convert.arg[1] = args[2];
    pResult->j = isnan(convert.dd) ? 0x7ff8000000000000 LL : convert.ll;
}

/* see nativeFloat.h */
void Java_java_lang_Double_doubleToRawLongBits(const u4* args, JValue* pResult)
{
    Convert64 convert;
    convert.arg[0] = args[1];
    convert.arg[1] = args[2];
    pResult->j = convert.ll;
}

/* see nativeFloat.h */
void Java_java_lang_Double_longBitsToDouble(const u4* args, JValue* pResult)
{
    Convert64 convert;
    convert.arg[0] = args[1];
    convert.arg[1] = args[2];
    pResult->d = convert.dd;
}




