#include "vm_common.h"
#include "kni.h"
#include <nativeThread.h>
#include <nativeSystem.h>
#include <nativeString.h>
#include <nativeFloat.h>
#include <nativeObject.h>

KNINativeMethod gNativeMthTab[] = 
{

#if 0

     /*classpath, name, signature, funcPtr */
    {"LDThread;","activeCount0",   "()I",        (KniFunc)Java_java_lang_Thread_activeCount0},
	{"LDThread;","currentThread0" ,"()LDThread;",(KniFunc)Java_java_lang_Thread_currentThread0},
	{"LDThread;","sleep0" ,        "(J)V",       (KniFunc)Java_java_lang_Thread_sleep0},
	{"LDThread;","start0" ,        "()V",        (KniFunc)Java_java_lang_Thread_start0},
	{"LDThread;","isAlive0" ,      "()Z",        (KniFunc)Java_java_lang_Thread_isAlive0},
	{"LDThread;","printQ" ,        "(II)V",      (KniFunc)Java_java_lang_Thread_printQ},
#else
    {"Ljava/lang/Thread;", "activeCount",   "()I",                   (KniFunc)Java_java_lang_Thread_activeCount0},
    {"Ljava/lang/Thread;", "currentThread", "()Ljava/lang/Thread;",  (KniFunc)Java_java_lang_Thread_currentThread0},
    {"Ljava/lang/Thread;", "interrupt",     "()V",                   (KniFunc)Java_java_lang_Thread_interrupt},
    {"Ljava/lang/Thread;", "interrupted",   "()Z",                   (KniFunc)Java_java_lang_Thread_interrupted},
    {"Ljava/lang/Thread;", "isAlive",       "()Z",                   (KniFunc)Java_java_lang_Thread_isAlive0},
    {"Ljava/lang/Thread;", "isInterrupted", "()Z",                   (KniFunc)Java_java_lang_Thread_isInterrupted},
    {"Ljava/lang/Thread;", "setPriority",   "(I)V",                  (KniFunc)Java_java_lang_Thread_setPriority0},
    {"Ljava/lang/Thread;", "sleep",         "(JI)V",                 (KniFunc)Java_java_lang_Thread_sleep0},
    {"Ljava/lang/Thread;", "start",         "()V",                   (KniFunc)Java_java_lang_Thread_start0},
    {"Ljava/lang/Thread;", "yield",         "()V",                   (KniFunc)Java_java_lang_Thread_yield},
    {"Ljava/lang/Thread;", "holdsLock",     "(Ljava/lang/Object;)V", (KniFunc)Java_java_lang_Thread_holdsLock},

#endif

    /* java.lang.Object native APIs */
	{"Ljava/lang/Object;","wait" ,          "(JI)V",      (KniFunc)Java_java_lang_Object_wait},
	{"Ljava/lang/Object;","notify" ,        "()V",        (KniFunc)Java_java_lang_Object_notify},
	{"Ljava/lang/Object;","notifyAll" ,     "(II)V",      (KniFunc)Java_java_lang_Object_notifyAll},


    /* java.lang.System native APIs */
    {"Ljava/lang/System;", "arraycopy",         "(Ljava/lang/Object;ILjava/lang/Object;II)V", (KniFunc)Java_java_lang_System_arrayCopy},
    {"Ljava/lang/System;", "currentTimeMillis", "()J",                                        (KniFunc)Java_java_lang_System_currentTimeMillis},
    {"Ljava/lang/System;", "identityHashCode",  "(Ljava/lang/Object;)I",                      (KniFunc)Java_java_lang_System_identityHashCode},

    /* com.yarlungsoft.util.SystemPrintSteam native APIs */
    {"Lcom/yarlungsoft/util/SystemPrintStream;", "write",  "(I)V", (KniFunc)Java_com_yarlungsoft_util_SystemPrintSteam_write},

    /* java.lang.String native APIs */
    {"Ljava/lang/String;", "intern", "()Ljava/lang/String;", (KniFunc)Java_java_lang_String_intern},

    /* java.lang.Float native APIs */
    {"Ljava/lang/Float;", "floatToIntBits",    "(F)I", (KniFunc)Java_java_lang_Float_floatToIntBits},
    {"Ljava/lang/Float;", "floatToRawIntBits", "(F)I", (KniFunc)Java_java_lang_Float_floatToRawIntBits},
    {"Ljava/lang/Float;", "intBitsToFloat",    "(I)F", (KniFunc)Java_java_lang_Float_intBitsToFloat},

    /* java.lang.Double native APIs */
    {"Ljava/lang/Double;", "doubleToLongBits",    "(D)J", (KniFunc)Java_java_lang_Double_doubleToLongBits},
    {"Ljava/lang/Double;", "doubleToRawLongBits", "(D)J", (KniFunc)Java_java_lang_Double_doubleToRawLongBits},
    {"Ljava/lang/Double;", "longBitsToDouble",    "(J)D", (KniFunc)Java_java_lang_Double_longBitsToDouble},
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
