#ifndef __KNI_H__
#define __KNI_H__

#include "vm_common.h"
#include "dvmdex.h"

typedef void (/*__stdcall*/ *KniFunc)(const u4 * argv,JValue* pRet);

typedef struct
{
    const char* classpath;  
    const char* name;
    const char* signature;
    KniFunc     fnPtr;
}KNINativeMethod;


//extern KNINativeMethod gNativeMthTab


KniFunc Kni_findFuncPtr(const Method * mthd);

#endif