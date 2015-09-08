/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2013/09/22 $
 * Last modified:	$Date: 2013/09/22 $
 * Version:         $ID: nativeString.c#1
 */

/**
 * The implementation of java.lang.String.
 */
 
#include <nativeString.h>
#include <utfstring.h>
 

/**
 * Class:     java_lang_String
 * Method:    intern
 * Signature: ()Ljava/lang/String;
 */
void Java_java_lang_String_intern(const u4* args, JValue* pResult)
{
    //nothing to do until now!
    DVMTraceErr("CAUSTION: Java_java_lang_String_intern not implemented!\n");
}