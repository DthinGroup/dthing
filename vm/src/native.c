#include "vm_common.h"
#include "kni.h"
#include "nativeThread.h"
#include <nativeSystem.h>
#include <nativeString.h>
#include <nativeObject.h>

KNINativeMethod gNativeMthTab[] = 
{
     /*classpath, name, signature, funcPtr */
    {"LDThread;","activeCount0",   "()I",        (KniFunc)Java_java_lang_Thread_activeCount0},
	{"LDThread;","currentThread0" ,"()LDThread;",(KniFunc)Java_java_lang_Thread_currentThread0},
	{"LDThread;","sleep0" ,        "(J)V",       (KniFunc)Java_java_lang_Thread_sleep0},
	{"LDThread;","start0" ,        "()V",        (KniFunc)Java_java_lang_Thread_start0},
	{"LDThread;","isAlive0" ,      "()Z",        (KniFunc)Java_java_lang_Thread_isAlive0},
	{"LDThread;","printQ" ,        "(II)V",      (KniFunc)Java_java_lang_Thread_printQ},

	{"Ljava/lang/Object;","wait" ,          "(JI)V",      (KniFunc)Java_java_lang_Object_wait},
	{"Ljava/lang/Object;","notify" ,        "()V",        (KniFunc)Java_java_lang_Object_notify},
	{"Ljava/lang/Object;","notifyAll" ,     "(II)V",      (KniFunc)Java_java_lang_Object_notifyAll},
	


    /* java.lang.System native APIs */
    {"Ljava/lang/System;", "arraycopy",         "(Ljava/lang/Object;ILjava/lang/Object;II)V", (KniFunc)Java_java_lang_System_arrayCopy},
    {"Ljava/lang/System;", "currentTimeMillis", "()J",                                        (KniFunc)Java_java_lang_System_currentTimeMillis},
    {"Ljava/lang/System;", "identityHashCode",  "(Ljava/lang/Object;)I",                      (KniFunc)Java_java_lang_System_identityHashCode},

    /* java.lang.String native APIs */

    {"Ljava/lang/String;", "intern", "()Ljava/lang/String;", (KniFunc)Java_java_lang_String_intern},

};



KniFunc Kni_findFuncPtr(const Method * mthd)
{
    ClassObject * clazz = NULL;
    int i =0;
    KniFunc funcptr = NULL;
    int funcCount = sizeof(gNativeMthTab)/sizeof(gNativeMthTab[0]);
    
    DVM_ASSERT(mthd != NULL);
    clazz = mthd->clazz;
    if(clazz == NULL)
    {
    
    }
    
    for(i=0;i<funcCount;i++)
    {
        if((0 ==DVM_STRCMP(gNativeMthTab[i].classpath,clazz->descriptor)) && (0== DVM_STRCMP(gNativeMthTab[i].name,mthd->name)))
        {
            funcptr = (KniFunc)gNativeMthTab[i].fnPtr;
            break;
        }
    }
    
    return funcptr;
}
