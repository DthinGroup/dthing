#include <utfstring.h>
#include <vm_common.h>
#include "nativeMain.h"
#include "class.h"
#include "crtl.h"
#include "init.h"


//just support ascii convert
char * convertJcharToChar(jchar * path, int len)
{
	char * convert = NULL;
	char high, low;
	int i=0;
	if(path == NULL)
		return NULL;

	convert = CRTL_malloc(len+1);
	CRTL_memset(convert,0,len+1);

	for(i=0; i<len; i++)
	{
		high = (char)((path[i] & 0xff00) >> 8);
		low = (char)(path[i] & 0x00ff);
		convert[i] =  (high ==0x00) ? low : high;
	}
	return convert;
}

/**
 * Class:     com_yarlungsoft_ams_Main
 * Method:    loadClassByPath0
 * Signature: (Ljava/lang/String;)V
 */
void Java_com_yarlungsoft_ams_Main_loadClassByPath0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    StringObject * pathObj = (StringObject *) args[1];
    const jchar* path = dvmGetStringData(pathObj);
//    const char* path = dvmCreateCstrFromString(pathObj);
    int pathLen = dvmGetStringLength(pathObj);

	char * cpath = convertJcharToChar(path, pathLen);

    loadClassByPath(cpath);

	CRTL_free(cpath);

	RETURN_VOID();

    // return type : void
}

/**
 * Class:     com_yarlungsoft_ams_Main
 * Method:    setNextSchedulerState0
 * Signature: (I)V
 */
void Java_com_yarlungsoft_ams_Main_setNextSchedulerState0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint state = (jint) args[1];

    // TODO: implementation
	setNextSchedulerState(state);

    RETURN_VOID();
}