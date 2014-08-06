#include <utfstring.h>
#include <vm_common.h>
#include <opl_file.h>
#include "nativeFileOutputStream.h"

/**
 * Class:     java_io_FileOutputStream
 * Method:    writeFile
 * Signature: (I[BII)I
 */
void Java_java_io_FileOutputStream_writeFile(const u4* args, JValue* pResult) {
    jint handle = (jint) args[1];
    ArrayObject * bArr = (ArrayObject *)args[2];
    jbyte * bArrPtr = (jbyte *)(KNI_GET_ARRAY_BUF(args[2]));
    int bArrLen = KNI_GET_ARRAY_LEN(args[2]);
    jint off = (jint) args[3];
    jint count = (jint) args[4];
    jint ret = 0;

    ret = file_write(handle,&bArrPtr[off],count);

    RETURN_INT(ret);
}

/**
 * Class:     java_io_FileOutputStream
 * Method:    openFile
 * Signature: (Ljava/lang/String;Z)I
 */
void Java_java_io_FileOutputStream_openFile(const u4* args, JValue* pResult) {
    StringObject * nameObj = (StringObject *) args[1];
    const jchar* name = dvmGetStringData(nameObj);
//    const char* name = dvmCreateCstrFromString(nameObj);
    int nameLen = dvmGetStringLength(nameObj);
    jboolean append = (jboolean) args[2];
    jint ret = 0;
	jint handle =0;
	jint fioRes = FILE_RES_FAILURE;

	if (name == NULL || nameLen <= 0) {
		RETURN_INT(ret);
	}

	if(!append){
		//file_truncate(handle,0);
		file_delete(name,nameLen);
	}

	fioRes = file_open(name, nameLen, FILE_MODE_WR, &handle);
	if (fioRes != FILE_RES_SUCCESS || handle == NULL) {
		RETURN_INT(ret);
	}

	if(append){
		file_seekex(handle, 0, FILE_SEEK_END);
	}

    RETURN_INT(handle);
}

/**
 * Class:     java_io_FileOutputStream
 * Method:    closeFile
 * Signature: (I)Z
 */
void Java_java_io_FileOutputStream_closeFile(const u4* args, JValue* pResult) {
    jint handle = (jint) args[1];
    jboolean ret = FALSE;

    if(file_close(handle) ==FILE_RES_SUCCESS)
		 ret = TRUE;
    RETURN_BOOLEAN(ret);
}

