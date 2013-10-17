#ifndef __NATIVE_ASYNCIO_H__
#define __NATIVE_ASYNCIO_H__



int  AsyncIO_getCurNotifierState(const u4* args, JValue* pResult);

void AsyncIO_setCurNotifierState(const u4* args, JValue* pResult);

void AsyncIO_waitSignalOrTimeOut(const u4* args, JValue* pResult);
#endif