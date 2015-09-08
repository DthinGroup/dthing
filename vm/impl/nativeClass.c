/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 *
 * Created:         $Date: 2013/10/17 $
 * Last modified:	$Date: 2013/10/17 $
 * Version:         $ID: nativeClass.c#1
 */


#include <nativeClass.h>
#include <class.h>
#include <classmisc.h>
#include <typecheck.h>
#include <exception.h>
#include <array.h>
#include <dthread.h>
#include <interpStack.h>
#include <accesscheck.h>
#include <gc.h>
#include <interpApi.h>
#include <utfstring.h>
#include <opl_file.h>
#include <encoding.h>


/**
 * Class:     java_lang_Class
 * Method:    forName
 * Signature: (Ljava/lang/String;)Ljava/lang/Class;
 */
void Java_java_lang_Class_forName(const u4* args, JValue* pResult)
{
    StringObject* nameObj = (StringObject*) args[1];

    RETURN_PTR(dvmFindClassByName(nameObj));
}

/**
 * Class:     java_lang_Class
 * Method:    isInstance
 * Signature: (Ljava/lang/Object;)Z
 */
void Java_java_lang_Class_isInstance(const u4* args, JValue* pResult)
{
    ClassObject* thisPtr = (ClassObject*) args[0];
    Object* testObj = (Object*) args[1];

    if (testObj == NULL)
        RETURN_INT(FALSE);
    RETURN_INT(dvmInstanceof(testObj->clazz, thisPtr));
}

/**
 * Class:     java_lang_Class
 * Method:    isAssignableFrom
 * Signature: (Ljava/lang/Class;)Z
 */
void Java_java_lang_Class_isAssignableFrom(const u4* args, JValue* pResult)
{
    ClassObject* thisPtr = (ClassObject*) args[0];
    ClassObject* testClass = (ClassObject*) args[1];

    if (testClass == NULL) {
        dvmThrowException("Ljava/lang/NullPointerException;", NULL);
        RETURN_INT(FALSE);
    }
    RETURN_INT(dvmInstanceof(testClass, thisPtr));    
}

/**
 * Class:     java_lang_Class
 * Method:    isInterface
 * Signature: ()Z
 */
void Java_java_lang_Class_isInterface(const u4* args, JValue* pResult)
{
    ClassObject* thisPtr = (ClassObject*) args[0];

    RETURN_INT(dvmIsInterfaceClass(thisPtr));
}

/**
 * Class:     java_lang_Class
 * Method:    isArray
 * Signature: ()Z
 */
void Java_java_lang_Class_isArray(const u4* args, JValue* pResult)
{
    ClassObject* thisPtr = (ClassObject*) args[0];

    RETURN_INT(dvmIsArrayClass(thisPtr));
}

/**
 * Class:     java_lang_Class
 * Method:    newInstance0
 * Signature: ()Ljava/lang/Object;
 */
void Java_java_lang_Class_newInstance0(const u4* args, JValue* pResult)
{
    Thread* self = dthread_currentThread();
    ClassObject* clazz = (ClassObject*) args[0];
    Method* init;
    Object* newObj;
    ClassObject* callerClass;

    /* can't instantiate these */
    if (dvmIsPrimitiveClass(clazz) || dvmIsInterfaceClass(clazz)
        || dvmIsArrayClass(clazz) || dvmIsAbstractClass(clazz))
    {
        DVMTraceDbg("newInstance failed: p%d i%d [%d a%d",
            dvmIsPrimitiveClass(clazz), dvmIsInterfaceClass(clazz),
            dvmIsArrayClass(clazz), dvmIsAbstractClass(clazz));
        dvmThrowInstantiationException(clazz, clazz->descriptor);
        RETURN_VOID();
    }

    /* initialize the class if it hasn't been already */
    if (!dvmIsClassInitialized(clazz)) {
        if (!dvmInitClass(clazz)) {
            DVMTraceWar("Class init failed in newInstance call (%s)",
                clazz->descriptor);
            //assert(dvmCheckException(self));
            RETURN_VOID();
        }
    }

    /* find the "nullary" constructor */
    init = dvmFindDirectMethodByDescriptor(clazz, "<init>", "()V");
    if (init == NULL) {
        /* common cause: secret "this" arg on non-static inner class ctor */
        DVMTraceDbg("newInstance failed: no <init>()");
        dvmThrowInstantiationException(clazz, "no empty constructor");
        RETURN_VOID();
    }

    /*
     * Verify access from the call site.
     *
     * First, make sure the method invoking Class.newInstance() has permission
     * to access the class.
     *
     * Second, make sure it has permission to invoke the constructor.  The
     * constructor must be public or, if the caller is in the same package,
     * have package scope.
     */
    callerClass = dvmGetCaller2Class(self->interpSave.curFrame);

    if (!dvmCheckClassAccess(callerClass, clazz)) {
        DVMTraceDbg("newInstance failed: %s not accessible to %s",
            clazz->descriptor, callerClass->descriptor);
        dvmThrowIllegalAccessException("access to class not allowed");
        RETURN_VOID();
    }
    if (!dvmCheckMethodAccess(callerClass, init)) {
        DVMTraceDbg("newInstance failed: %s.<init>() not accessible to %s",
            clazz->descriptor, callerClass->descriptor);
        dvmThrowIllegalAccessException("access to constructor not allowed");
        RETURN_VOID();
    }

    newObj = dvmAllocObject(clazz, ALLOC_DEFAULT);

    /* invoke constructor; unlike reflection calls, we don't wrap exceptions */
    dvmCallInitMethod(init, newObj);
    //dvmCallMethod(self, init, newObj, &unused);
    //dvmReleaseTrackedAlloc(newObj, NULL);

    RETURN_PTR(newObj);
}

/**
 * Class:     java_lang_Class
 * Method:    getName
 * Signature: ()Ljava/lang/String;
 */
void Java_java_lang_Class_getName(const u4* args, JValue* pResult)
{
    ClassObject* clazz = (ClassObject*) args[0];
    const char* descriptor = clazz->descriptor;
    StringObject* nameObj;

    if ((descriptor[0] != 'L') && (descriptor[0] != '[')) {
        /*
         * The descriptor indicates that this is the class for
         * a primitive type; special-case the return value.
         */
        const char* name;
        switch (descriptor[0]) {
            case 'Z': name = "boolean"; break;
            case 'B': name = "byte";    break;
            case 'C': name = "char";    break;
            case 'S': name = "short";   break;
            case 'I': name = "int";     break;
            case 'J': name = "long";    break;
            case 'F': name = "float";   break;
            case 'D': name = "double";  break;
            case 'V': name = "void";    break;
            default: {
                DVMTraceDbg("Unknown primitive type '%c'", descriptor[0]);
                //assert(false);
                RETURN_PTR(NULL);
            }
        }

        nameObj = dvmCreateStringFromCstr(name);
    }
    else
    {
    
        /*
         * Convert the UTF-8 name to a java.lang.String. The
         * name must use '.' to separate package components.
         *
         * TODO: this could be more efficient. Consider a custom
         * conversion function here that walks the string once and
         * avoids the allocation for the common case (name less than,
         * say, 128 bytes).
         */
        char* dotName = dvmDescriptorToDot(clazz->descriptor);
        nameObj = dvmCreateStringFromCstr(dotName);
        CRTL_free(dotName);
    }

    //dvmReleaseTrackedAlloc((Object*) nameObj, NULL);
    RETURN_PTR(nameObj);
}


#include <jarparser.h>
/**
 * Class:     java_lang_Class
 * Method:    getResourceAsStream
 * Signature: (Ljava/lang/String;)Ljava/io/InputStream;
 */
void Java_java_lang_Class_getResourceAsStream(const u4* args, JValue* pResult)
{
    ClassObject*  clazz = (ClassObject*) args[0];
    StringObject* resObj = (StringObject*)args[1];
	s4  i;
	s4  handle;
    u2  fUcs2Name[MAX_FILE_NAME_LEN] = {0x0,};
	u1  fUtf8Name[MAX_FILE_NAME_LEN] = {0x0,};
	u1* fResName = NULL;
    s4  srcBytes = 0;
    s4  dstChars = 0;
	u8* data = NULL;
	s4  dataLen = 0;
	ClassObject* isCls;
	Object* isObj;
	ArrayObject* baObj;
	Method* init;
    const u2* resName = NULL;

    ClassesEntry* pClsEntry = gDvm.pClsEntry;
    if (pClsEntry == NULL) {
        RETURN_PTR(NULL);
    }
	resName = dvmGetStringData(resObj);
	convertUcs2ToUtf8(resName, dvmGetStringLength(resObj), fUtf8Name, MAX_FILE_NAME_LEN);
	fResName = fUtf8Name[0] == '/' ? fUtf8Name+1 : fUtf8Name;

    for (i = 0; i < MAX_NUM_CLASSES_ENTRY; i++)
    {
        if (pClsEntry[i].kind == kCpeJar)
        {
        	CRTL_memset(fUcs2Name, 0x0, MAX_FILE_NAME_LEN);
			srcBytes = (s4)CRTL_strlen(pClsEntry[i].fileName);
		    dstChars = convertAsciiToUcs2(pClsEntry[i].fileName, srcBytes, fUcs2Name, MAX_FILE_NAME_LEN);
			handle = openJar(fUcs2Name);
			data = (u8 *)getJarContentByFileName(handle, fResName, &dataLen);
			if (data != NULL) {
				closeJar(handle);
				break;
			}
			closeJar(handle);
        }
    }
	if (data == NULL || dataLen == 0) {
		RETURN_PTR(NULL);
	}

	baObj = dvmAllocPrimitiveArray('B', dataLen, ALLOC_DEFAULT);
	if (baObj == NULL) {
		RETURN_PTR(NULL);
	}
	CRTL_memcpy((u1 *)baObj->contents, data, dataLen);
	CRTL_free(data);
	
	
	isCls = dvmFindClass("Ljava/io/ByteArrayInputStream;");
	if (isCls == NULL) {
		RETURN_PTR(NULL);
	}
	isObj = dvmAllocObject(isCls, ALLOC_DEFAULT);

    /* find the "nullary" constructor */
    init = dvmFindDirectMethodByDescriptor(isCls, "<init>", "([B)V");
    if (init == NULL) {
        /* common cause: secret "this" arg on non-static inner class ctor */
        DVMTraceDbg("newInstance failed: no <init>()");
        dvmThrowInstantiationException(clazz, "no empty constructor");
		RETURN_PTR(NULL);
    }
	dvmCallInitMethod(init, isObj, baObj);
	
	RETURN_PTR(isObj);
}
