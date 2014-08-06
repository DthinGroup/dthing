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
    StringObject * nameObj = (StringObject *) args[0];
    const jchar* name = dvmGetStringData(nameObj);
//    const char* name = dvmCreateCstrFromString(nameObj);
    int nameLen = dvmGetStringLength(nameObj);
    jint ret = 0;
	jint handle =0;
	jint fioRes = FILE_RES_FAILURE;

	if (name == NULL || nameLen <= 0) {
		RETURN_INT(ret);
	}

	fioRes = file_open(name, nameLen, FILE_MODE_RD, &handle);
	if (fioRes != FILE_RES_SUCCESS || handle == NULL) {
		RETURN_INT(ret);
	}

    RETURN_INT(handle);
}

/**
 * Class:     java_io_FileInputStream
 * Method:    closeFile
 * Signature: (I)Z
 */
void Java_java_io_FileInputStream_closeFile(const u4* args, JValue* pResult) {
    jint handle = (jint) args[0];
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
    jint handle = (jint) args[0];
    ArrayObject * buffArr = (ArrayObject *)args[1];
    jbyte * buffArrPtr = (jbyte *)(KNI_GET_ARRAY_BUF(args[1]));
    int buffArrLen = KNI_GET_ARRAY_LEN(args[1]);
    jint off = (jint) args[2];
    jint count = (jint) args[3];
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
    jint handle = (jint) args[0];
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
    jint handle = (jint) args[0];
    jlong byteCount = (jlong) args[1];
    jlong ret = 0;

	ret = file_seekex(handle,byteCount,FILE_SEEK_CURRENT);
    
    RETURN_LONG(ret);
}

