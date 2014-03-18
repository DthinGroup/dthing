#ifndef __KNI_H__
#define __KNI_H__

//#include "vm_common.h"
#include <common.h>
#include <dvmdex.h>


#define RETURN_VOID()           do { (void)(pResult); return; } while(0)
#define RETURN_BOOLEAN(_val)    do { pResult->i = (_val); return; } while(0)
#define RETURN_INT(_val)        do { pResult->i = (_val); return; } while(0)
#define RETURN_LONG(_val)       do { pResult->j = (_val); return; } while(0)
#define RETURN_FLOAT(_val)      do { pResult->f = (_val); return; } while(0)
#define RETURN_DOUBLE(_val)     do { pResult->d = (_val); return; } while(0)
#define RETURN_PTR(_val)        do { pResult->l = (Object*)(_val); return; } while(0)

typedef void (/*__stdcall*/ *KniFunc)(const u4 * argv,JValue* pResult);

typedef struct
{
    const char* classpath;  
    const char* name;
    const char* signature;
    KniFunc     fnPtr;
}KNINativeMethod;


//if kni param is byte[] array

#define KNI_GET_ARRAY_BUF(idx)	(char*)((ArrayObject *)idx)->contents
#define KNI_GET_ARRAY_LEN(idx)	(int)((ArrayObject *)idx)->length  


//extern KNINativeMethod gNativeMthTab


KniFunc Kni_findFuncPtr(const Method * mthd);

#endif
