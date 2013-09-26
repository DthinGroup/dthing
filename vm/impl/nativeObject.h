
#ifndef __NATIVE_OBJECT_H__
#define __NATIVE_OBJECT_H__

#include <dthing.h>
#include <kni.h>


/*Object.java*/
void Java_java_lang_Object_wait(const u4* args, JValue* pResult);

void Java_java_lang_Object_notify(const u4* args, JValue* pResult);

void Java_java_lang_Object_notifyAll(const u4* args, JValue* pResult);

#endif