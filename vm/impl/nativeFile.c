/**
 * Copyright (C) 2013-2014 YarlungSoft. All Rights Reserved.
 */

#include <nativeFile.h>
#include <vm_common.h>
#include <utfstring.h>
#include <array.h>
#include <opl_file.h>

/**
 * Class:     java_io_File
 * Method:    createFile0
 * Signature: (Ljava/lang/String;)Z
 */
void Java_java_io_File_createFile0(const u4* args, JValue* pResult) {
	ClassObject* thisObj = (ClassObject*) args[0];
    StringObject * pathObj = (StringObject *) args[1];
	const jchar* path = dvmGetStringData(pathObj);
	int pathLen = dvmGetStringLength(pathObj);
	int32_t fioRes = FILE_RES_FAILURE;
	int32_t handle = NULL;

	if (path == NULL || pathLen <= 0) {
		RETURN_BOOLEAN(false);
	}

	fioRes = file_open(path, pathLen, FILE_MODE_WR, &handle);
	if (fioRes != FILE_RES_SUCCESS || handle == NULL) {
		RETURN_BOOLEAN(false);
	}

	fioRes = file_close(handle);

    RETURN_BOOLEAN(fioRes == FILE_RES_SUCCESS);
}

/**
 * Class:     java_io_File
 * Method:    delete0
 * Signature: (Ljava/lang/String;)Z
 */
void Java_java_io_File_delete0(const u4* args, JValue* pResult) {
	ClassObject* thisObj = (ClassObject*) args[0];
    StringObject * pathObj = (StringObject *) args[1];
	const jchar* path = dvmGetStringData(pathObj);
	int pathLen = dvmGetStringLength(pathObj);
	int32_t fioRes = FILE_RES_FAILURE;

	if (path == NULL || pathLen <= 0) {
		RETURN_BOOLEAN(false);
	}

	fioRes = file_delete(path, pathLen);

	RETURN_BOOLEAN(fioRes == FILE_RES_SUCCESS);
}

/**
 * Class:     java_io_File
 * Method:    exists0
 * Signature: (Ljava/lang/String;)Z
 */
void Java_java_io_File_exists0(const u4* args, JValue* pResult) {
	ClassObject* thisObj = (ClassObject*) args[0];
    StringObject * pathObj = (StringObject *) args[1];
	const jchar* path = dvmGetStringData(pathObj);
	int pathLen = dvmGetStringLength(pathObj);
	int32_t fioRes = FILE_RES_FAILURE;

	if (path == NULL || pathLen <= 0) {
		RETURN_BOOLEAN(false);
	}

	fioRes = file_exists(path, pathLen);

	RETURN_BOOLEAN(fioRes == FILE_RES_ISDIR || fioRes == FILE_RES_ISREG);
}

/**
 * Class:     java_io_File
 * Method:    isDirectory0
 * Signature: (Ljava/lang/String;)Z
 */
void Java_java_io_File_isDirectory0(const u4* args, JValue* pResult) {
	ClassObject* thisObj = (ClassObject*) args[0];
    StringObject * pathObj = (StringObject *) args[1];
	const jchar* path = dvmGetStringData(pathObj);
	int pathLen = dvmGetStringLength(pathObj);
	int32_t fioRes = FILE_RES_FAILURE;

	if (path == NULL || pathLen <= 0) {
		RETURN_BOOLEAN(false);
	}

	fioRes = file_exists(path, pathLen);

	RETURN_BOOLEAN(fioRes == FILE_RES_ISDIR);
}

/**
 * Class:     java_io_File
 * Method:    isFile0
 * Signature: (Ljava/lang/String;)Z
 */
void Java_java_io_File_isFile0(const u4* args, JValue* pResult) {
	ClassObject* thisObj = (ClassObject*) args[0];
    StringObject * pathObj = (StringObject *) args[1];
	const jchar* path = dvmGetStringData(pathObj);
	int pathLen = dvmGetStringLength(pathObj);
	int32_t fioRes = FILE_RES_FAILURE;

	if (path == NULL || pathLen <= 0) {
		RETURN_BOOLEAN(false);
	}

	fioRes = file_exists(path, pathLen);

	RETURN_BOOLEAN(fioRes == FILE_RES_ISREG);
}

/**
 * Class:     java_io_File
 * Method:    lastModified0
 * Signature: (Ljava/lang/String;)J
 */
void Java_java_io_File_lastModified0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    StringObject * pathObj = (StringObject *) args[1];
	const jchar* path = dvmGetStringData(pathObj);
	int pathLen = dvmGetStringLength(pathObj);
    jlong ret = 0;

    // TODO : get last modification of file/directory

    RETURN_LONG(ret);
}

/**
 * Class:     java_io_File
 * Method:    length0
 * Signature: (Ljava/lang/String;)J
 */
void Java_java_io_File_length0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    StringObject * pathObj = (StringObject *) args[1];
	const jchar* path = dvmGetStringData(pathObj);
	int pathLen = dvmGetStringLength(pathObj);
	int32_t fioRes = FILE_RES_FAILURE;
	int32_t handle = NULL;
    jlong ret = 0;

	if (path == NULL || pathLen <= 0) {
		RETURN_BOOLEAN(0);
	}

	fioRes = file_open(path, pathLen, FILE_MODE_RD, &handle);
	if (fioRes != FILE_RES_SUCCESS || handle == NULL) {
		RETURN_BOOLEAN(0);
	}

	ret = file_getLengthByFd(handle);

	fioRes = file_close(handle);
    RETURN_LONG(ret);
}

/**
 * Class:     java_io_File
 * Method:    list0
 * Signature: (Ljava/lang/String;)[Ljava/lang/String;
 */
void Java_java_io_File_list0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    StringObject * pathObj = (StringObject *) args[1];
    StringObject* subPathObj = NULL;
    char* path = dvmCreateCstrFromString(pathObj);
    ClassObject* classArrayString = NULL;
    ArrayObject* ret = NULL;
    int i;
    int filesCount = 0;

    if (path == NULL) {
        RETURN_PTR(NULL);
    }

    classArrayString = dvmFindArrayClass("[Ljava/lang/String;");
    if (classArrayString == NULL) {
        RETURN_PTR(NULL);
    }

    // TODO : list files under specified path

    ret = dvmAllocArrayByClass(gDvm.classArrayLong, filesCount, ALLOC_DEFAULT);
    if (ret == NULL) {
        RETURN_PTR(NULL);
    }

    for (i = 0; i < filesCount; i++) {
        subPathObj = dvmCreateStringFromCstr(/* TODO: concrete sub path */NULL);
        dvmSetObjectArrayElement(ret, i, subPathObj);
    }

    CRTL_free(path);
    RETURN_PTR(ret);
}
