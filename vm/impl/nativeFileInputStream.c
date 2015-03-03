#include <utfstring.h>
#include <vm_common.h>
#include <opl_file.h>
#include "nativeFileInputStream.h"

/**
 * Class:     java_io_FileInputStream
 * Method:    openFile
 * Signature: (Ljava/lang/String;)I
 */
void Java_java_io_FileInputStream_openFile(const u4* args, JValue* pResult) {
    StringObject * nameObj = (StringObject *) args[1];
    const jchar* name = dvmGetStringData(nameObj);
//    const char* name = dvmCreateCstrFromString(nameObj);
    int nameLen = dvmGetStringLength(nameObj);
	jint handle = 0;
	jint fioRes = FILE_RES_FAILURE;

	DVMTraceDbg("Java_java_io_FileInputStream_openFile name=0x%08X, nameLen=%d\n",name, nameLen);

	if (name == NULL || nameLen <= 0) {
		RETURN_INT(0);
	}

	fioRes = file_open(name, nameLen, FILE_MODE_RD, &handle);
	DVMTraceDbg("Java_java_io_FileInputStream_openFile fioRes=%d handle=%d\n",fioRes, handle);
	if (fioRes != FILE_RES_SUCCESS || handle == NULL) {
		RETURN_INT(0);
	}

    RETURN_INT(handle);
}

/**
 * Class:     java_io_FileInputStream
 * Method:    closeFile
 * Signature: (I)Z
 */
void Java_java_io_FileInputStream_closeFile(const u4* args, JValue* pResult) {
    jint handle = (jint) args[1];
    jboolean ret = FALSE;

    if(file_close(handle) ==FILE_RES_SUCCESS)
		 ret = TRUE;
    RETURN_BOOLEAN(ret);
}

/**
 * Class:     java_io_FileInputStream
 * Method:    readFile
 * Signature: (I[BII)I
 */
void Java_java_io_FileInputStream_readFile(const u4* args, JValue* pResult) {
    jint handle = (jint) args[1];
    ArrayObject * buffArr = (ArrayObject *)args[2];
    jbyte * buffArrPtr = (jbyte *)(KNI_GET_ARRAY_BUF(args[2]));
    int buffArrLen = KNI_GET_ARRAY_LEN(args[2]);
    jint off = (jint) args[3];
    jint count = (jint) args[4];
    jint ret = 0;

    ret = file_read(handle, &buffArrPtr[off],count);

    RETURN_INT(ret);
}

/**
 * Class:     java_io_FileInputStream
 * Method:    available0
 * Signature: (I)I
 */
void Java_java_io_FileInputStream_available0(const u4* args, JValue* pResult) {
    jint handle = (jint) args[1];
	jint size =0;

    if(file_getsize(handle,&size)==FILE_RES_SUCCESS)
		RETURN_INT(size);

    RETURN_INT(-1);
}

/**
 * Class:     java_io_FileInputStream
 * Method:    skip0
 * Signature: (IJ)J
 */
void Java_java_io_FileInputStream_skip0(const u4* args, JValue* pResult) {
    jint handle = (jint) args[1];
    jlong byteCount = (jlong) args[2];
    jlong ret = 0;

	ret = file_seekex(handle,byteCount,FILE_SEEK_CURRENT);
    
    RETURN_LONG(ret);
}

