/**
 * Copyright (C) 2013-2014 YarlungSoft. All Rights Reserved.
 */

#include <nativeFile.h>
#include <vm_common.h>
#include <utfstring.h>
#include <array.h>

/**
 * Class:     java_io_File
 * Method:    createFile0
 * Signature: (Ljava/lang/String;)Z
 */
void Java_java_io_File_createFile0(const u4* args, JValue* pResult) {
    StringObject* pathObj = (StringObject*) args[0];
    char* path = dvmCreateCstrFromString(pathObj);
    jboolean ret = false;

    // TODO : create file

    CRTL_free(path);
    RETURN_BOOLEAN(ret);
}

/**
 * Class:     java_io_File
 * Method:    delete0
 * Signature: (Ljava/lang/String;)Z
 */
void Java_java_io_File_delete0(const u4* args, JValue* pResult) {
    StringObject* pathObj = (StringObject*) args[0];
    char* path = dvmCreateCstrFromString(pathObj);
    jboolean ret = false;

    // TODO : delete file

    CRTL_free(path);
    RETURN_BOOLEAN(ret);
}

/**
 * Class:     java_io_File
 * Method:    exists0
 * Signature: (Ljava/lang/String;)Z
 */
void Java_java_io_File_exists0(const u4* args, JValue* pResult) {
    StringObject* pathObj = (StringObject*) args[0];
    char* path = dvmCreateCstrFromString(pathObj);
    jboolean ret = false;

    // TODO : check file existence

    CRTL_free(path);
    RETURN_BOOLEAN(ret);
}

/**
 * Class:     java_io_File
 * Method:    isDirectory0
 * Signature: (Ljava/lang/String;)Z
 */
void Java_java_io_File_isDirectory0(const u4* args, JValue* pResult) {
    StringObject* pathObj = (StringObject*) args[0];
    char* path = dvmCreateCstrFromString(pathObj);
    jboolean ret = false;

    // TODO : check whether path is a directory

    CRTL_free(path);
    RETURN_BOOLEAN(ret);
}

/**
 * Class:     java_io_File
 * Method:    isFile0
 * Signature: (Ljava/lang/String;)Z
 */
void Java_java_io_File_isFile0(const u4* args, JValue* pResult) {
    StringObject* pathObj = (StringObject*) args[0];
    char* path = dvmCreateCstrFromString(pathObj);
    jboolean ret = false;

    // TODO : check whether path is a file

    CRTL_free(path);
    RETURN_BOOLEAN(ret);
}

/**
 * Class:     java_io_File
 * Method:    lastModified0
 * Signature: (Ljava/lang/String;)J
 */
void Java_java_io_File_lastModified0(const u4* args, JValue* pResult) {
    StringObject* pathObj = (StringObject*) args[0];
    char* path = dvmCreateCstrFromString(pathObj);
    jlong ret = 0;

    // TODO : get last modification of file/directory

    CRTL_free(path);
    RETURN_LONG(ret);
}

/**
 * Class:     java_io_File
 * Method:    length0
 * Signature: (Ljava/lang/String;)J
 */
void Java_java_io_File_length0(const u4* args, JValue* pResult) {
    StringObject* pathObj = (StringObject*) args[0];
    char* path = dvmCreateCstrFromString(pathObj);
    jlong ret = 0;

    // TODO : fetch the file size

    CRTL_free(path);
    RETURN_LONG(ret);
}

/**
 * Class:     java_io_File
 * Method:    list0
 * Signature: (Ljava/lang/String;)[Ljava/lang/String;
 */
void Java_java_io_File_list0(const u4* args, JValue* pResult) {
    StringObject* pathObj = (StringObject*) args[0];
    StringObject* subPathObj = NULL;
    char* path = dvmCreateCstrFromString(pathObj);
    ClassObject* classArrayString = NULL;
    ArrayObject* ret = NULL;
    int i;
    int filesCount = 0;

    if (path == NULL) {
        RETURN_PTR(ret);
    }

    classArrayString = dvmFindArrayClass("[Ljava/lang/String;");
    if (classArrayString == NULL) {
        RETURN_PTR(ret);
    }

    // TODO : list files under specified path

    ret = dvmAllocArrayByClass(gDvm.classArrayLong, filesCount, ALLOC_DEFAULT);
    if (ret == NULL) {
        RETURN_PTR(ret);
    }

    for (i = 0; i < filesCount; i++) {
        subPathObj = dvmCreateStringFromCstr(/* TODO: concrete sub path */NULL);
        dvmSetObjectArrayElement(ret, i, subPathObj);
    }

    CRTL_free(path);
    RETURN_PTR(ret);
}
