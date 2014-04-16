#ifndef __KNI_H__
#define __KNI_H__

//#include "vm_common.h"
#include <common.h>
#include <dvmdex.h>

#ifdef __cplusplus
extern "C" {
#endif


#define RETURN_VOID()           return
#define RETURN_BOOLEAN(_val)    do { pResult->z = (_val); return; } while(0)
#define RETURN_INT(_val)        do { pResult->i = (_val); return; } while(0)
#define RETURN_LONG(_val)       do { pResult->j = (_val); return; } while(0)
#define RETURN_FLOAT(_val)      do { pResult->f = (_val); return; } while(0)
#define RETURN_DOUBLE(_val)     do { pResult->d = (_val); return; } while(0)
#define RETURN_PTR(_val)        do { pResult->l = (Object*)(_val); return; } while(0)

typedef void (/*__stdcall*/*KniFunc)(const u4 * argv, JValue* pResult);

//if kni param is byte[] array

#define KNI_GET_ARRAY_BUF(idx)  (char*)((ArrayObject *)idx)->contents
#define KNI_GET_ARRAY_LEN(idx)  (int)((ArrayObject *)idx)->length  

KniFunc Kni_findFuncPtr(const Method * mthd);

StringObject* NewStringUTF(const char* bytes);

#ifdef __cplusplus
}
#endif


#endif
