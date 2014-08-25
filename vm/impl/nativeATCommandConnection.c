#include <utfstring.h>
#include <vm_common.h>
#include "nativeATCommandConnection.h"

#if defined(ARCH_ARM_SPD)
#include "sci_types.h"
#include "os_api.h"
#include "atc.h"
#include "AsyncIO.h"

#undef  FORCE_ATC_HEADER
#define FORCE_ATC_END
#define FORMAT_RESPONSE
#define ASYNCIO_SUPPORT
#define MAX_CMD_LENGTH 255
static ASYNC_Notifier g_sendNotifier = NULL;
static int g_async_linkId = 0;

unsigned char generateLinkId(void);
void atc_callback(void);
#endif

/**
 * Class:     jp_co_cmcc_atcommand_ATCommandConnection
 * Method:    open0
 * Signature: ()I
 */
void Java_jp_co_cmcc_atcommand_ATCommandConnection_open0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = 0;

    // TODO: implementation

    RETURN_INT(ret);
}

/**
 * Class:     jp_co_cmcc_atcommand_ATCommandConnection
 * Method:    send0
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
void Java_jp_co_cmcc_atcommand_ATCommandConnection_send0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    StringObject * atcmdObj = (StringObject *) args[1];
    const jchar* atcmd = dvmGetStringData(atcmdObj);
//    const char* atcmd = dvmCreateCstrFromString(atcmdObj);
    int atcmdLen = dvmGetStringLength(atcmdObj);
    StringObject * retObj = NULL;

    // TODO: initialize retObj via dvmCreateStringFrom* methods:
    // retObj = dvmCreateStringFromCstr("");

#if defined(ARCH_ARM_SPD)
    int atCommandLength = 0;
    char *response = NULL;
    uint responseLen = 0;
    unsigned char linkId = 0;
    char atCommand[MAX_CMD_LENGTH] = {0};

    if (AsyncIO_firstCall())
    {
        atCommandLength = atcmdLen;
        memset(&atCommand[0], 0x0, MAX_CMD_LENGTH);

        char * utf8Str = dvmCreateCstrFromString(atcmdObj);
	    atCommandLength = (atcmdLen >= MAX_CMD_LENGTH)? MAX_CMD_LENGTH : atcmdLen;
	    memcpy(&atCommand[0], utf8Str, atCommandLength);

	    if (utf8Str != NULL)
	    {
	        free(utf8Str);
	    }

#ifdef FORCE_ATC_END
        //force convert the <CR> flag
        if (atCommand[atCommandLength - 1] != 0x0D)
        {
            atCommand[atCommandLength - 1] = 0x0D;
        }
#endif //FORCE_ATC_END

#ifdef FORCE_ATC_HEADER
        //force convert to AT command
        if (atCommandLength > 2)
        {
            atCommand[0] = 'A';
            atCommand[1] = 'T';
        }
#endif //FORCE_ATC_HEADER
        linkId = generateLinkId();
        g_sendNotifier = Async_getCurNotifier();
        ATC_RegisterCallback(atc_callback);

        //Call native api
        ATC_SendNewATInd(linkId, atCommandLength, &atCommand[0]);
        AsyncIO_callAgainWhenSignalled();
    }
    else
    {
        //Waiting response
        ATC_GetResultRsp(linkId, &response, &responseLen);
        g_sendNotifier = NULL;
        ATC_RegisterCallback(NULL);
    }

#ifdef FORMAT_RESPONSE
    if ((response != NULL) && (strstr(response, "OK") != NULL))
    {
        retObj = dvmCreateStringFromCstr("OK");
    }
#else
    retObj = ((response != NULL)? dvmCreateStringFromCstr(response) : NULL);
#endif
#endif

    RETURN_PTR(retObj);
}

/**
 * Class:     jp_co_cmcc_atcommand_ATCommandConnection
 * Method:    sendAsyn0
 * Signature: (Ljava/lang/String;)I
 */
void Java_jp_co_cmcc_atcommand_ATCommandConnection_sendAsyn0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    StringObject * atcmdObj = (StringObject *) args[1];
    const jchar* atcmd = dvmGetStringData(atcmdObj);
//    const char* atcmd = dvmCreateCstrFromString(atcmdObj);
    int atcmdLen = dvmGetStringLength(atcmdObj);
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    int atCommandLength = 0;
    unsigned char linkId = 0;
    char atCommand[MAX_CMD_LENGTH] = {0};

    memset(&atCommand[0], 0x0, MAX_CMD_LENGTH);
    char * utf8Str = dvmCreateCstrFromString(atcmdObj);
	atCommandLength = (atcmdLen >= MAX_CMD_LENGTH)? MAX_CMD_LENGTH : atcmdLen;
	memcpy(&atCommand[0], utf8Str, atCommandLength);

	if (utf8Str != NULL)
    {
	    free(utf8Str);
	}

#ifdef FORCE_ATC_END
    //force convert the <CR> flag
    if (atCommand[atCommandLength - 1] != 0x0D)
    {
      atCommand[atCommandLength - 1] = 0x0D;
    }
#endif //FORCE_ATC_END

#ifdef FORCE_ATC_HEADER
    //force convert to AT command
    if (atCommandLength > 2)
    {
      atCommand[0] = 'A';
      atCommand[1] = 'T';
    }
#endif //FORCE_ATC_HEADER

    linkId = generateLinkId();
    g_async_linkId = linkId;

#ifdef ASYNCIO_SUPPORT
    ATC_RegisterCallback(atc_callback);
#endif

    //Call native api
    ATC_SendNewATInd(linkId, atCommandLength, &atCommand[0]);
    ret = linkId;
#endif

    RETURN_INT(ret);
}

/**
 * Class:     jp_co_cmcc_atcommand_ATCommandConnection
 * Method:    get0
 * Signature: ()Ljava/lang/String;
 */
void Java_jp_co_cmcc_atcommand_ATCommandConnection_get0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    StringObject * retObj = NULL;

    // TODO: initialize retObj via dvmCreateStringFrom* methods:
    // retObj = dvmCreateStringFromCstr("");

#if defined(ARCH_ARM_SPD)
    char *response = NULL;
    uint responseLen = 0;

    if ((g_async_linkId >= 0)
#ifdef ASYNCIO_SUPPORT
        && AsyncIO_firstCall()
#endif
    )
    {
        ATC_GetResultRsp(g_async_linkId, &response, &responseLen);

#ifdef ASYNCIO_SUPPORT
        if (response == NULL)
        {
            g_sendNotifier = Async_getCurNotifier();
            AsyncIO_callAgainWhenSignalled();
        }
#endif
    }
#ifdef ASYNCIO_SUPPORT
    else
    {
        ATC_GetResultRsp(g_async_linkId, &response, &responseLen);
        //clear flags for next async call
        g_async_linkId = -1;
        g_sendNotifier = NULL;
        ATC_RegisterCallback(NULL);
    }
#else
    if (response != NULL)
    {
        g_async_linkId = -1;
        ATC_RegisterCallback(NULL);
    }
#endif

#ifdef FORMAT_RESPONSE
    if ((response != NULL) && (strstr(response, "OK") != NULL))
    {
        retObj = dvmCreateStringFromCstr("OK");
    }
#else
    retObj = ((response != NULL)? dvmCreateStringFromCstr(response) : NULL);
#endif
#endif

    RETURN_PTR(retObj);
}

/**
 * Class:     jp_co_cmcc_atcommand_ATCommandConnection
 * Method:    close0
 * Signature: ()I
 */
void Java_jp_co_cmcc_atcommand_ATCommandConnection_close0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint ret = 0;
    
	//TODO:

    RETURN_INT(ret);
}

#if defined(ARCH_ARM_SPD)
unsigned char generateLinkId(void)
{
  return ATC_GetMuxLinkIdFromSimId(MN_DUAL_SYS_1);
}

void atc_callback(void)
{
  if (g_sendNotifier != NULL)
  {
    AsyncIO_notify(g_sendNotifier);
  }
}
#endif