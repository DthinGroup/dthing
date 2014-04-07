/**
 * Copyright (C) 2013-2014 YarlungSoft. All Rights Reserved.
 */

#include <nativeTimeZone.h>

/**
 * Class:     java_util_TimeZone
 * Method:    getDefaultId
 * Signature: ()Ljava/lang/String;
 */
void Java_java_util_TimeZone_getDefaultId(const u4* args, JValue* pResult) {
    char* ret = "GMT+8";
    // TODO: get system default GMT time zone id.
    RETURN_PTR(dvmCreateStringFromCstr(ret));
}
