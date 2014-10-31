#include <stdio.h>
#include <string.h>
#include <vm_common.h>
#include "opl_atc.h"

#ifdef ARCH_ARM_SPD
typedef struct TPALRequestObject
{
  int cmdId;
  int param;
  char *pData;
} TPALRequestObject;

typedef struct ATCommandData
{
  char *cmd;
  int cmdId;
  int hasParam;
} ATCommandData;

typedef enum _RemoteCommandType
{
    RCMD_ACK  = 1,
    RCMD_LOGIN,
    RCMD_CFGURL,
    RCMD_CFGACCOUNT,
    RCMD_INIT,
    RCMD_LIST,
    RCMD_INSTALL,
    RCMD_OTA,
    RCMD_DELETE,
    RCMD_DELETEALL,
    RCMD_RUN,
    RCMD_DESTROY,
    RCMD_STATUS,
    RCMD_RESET,
    RCMD_VIEW,
    RCMD_ACTIVATE,
    RCMD_REMOVE,
    RCMD_OSGI,
    RCMD_CANCEL,
    RCMD_CANCELALL,
    RCMD_MAX
} RemoteCommandType;

#define RCMD_OK (0)
#define RCMD_FAILURE (-1)

static ATCommandData atcommands[] =
{
  {"OTA", RCMD_OTA, 1},              /* ATAMS="OTA","[url]" */
  {"INSTALL", RCMD_INSTALL, 1},      /* ATAMS="INSTALL","[url] */
  {"OSGI", RCMD_OSGI, 1},            /* ATAMS="OSGI","[url] */
  {"DELETE", RCMD_DELETE, 1},        /* ATAMS="DELETE","[suiteid]" */
  {"DELETEALL", RCMD_DELETEALL, 0},  /* ATAMS="DELETEALL" */
  {"RUN", RCMD_RUN, 1},              /* ATAMS="RUN","[suiteid]" */
  {"LIST", RCMD_LIST, 0},            /* ATAMS="LIST" */
  {"DESTROY", RCMD_DESTROY, 1},      /* ATAMS="DESTROY","[suiteid]" */
  {"STATUS", RCMD_STATUS, 0},        /* ATAMS="STATUS" */
  {"RESET", RCMD_RESET, 0},          /* ATAMS="RESET" */
  {"CFGINIT", RCMD_INIT, 1},         /* ATAMS="CFGINIT","[url]" */
  {"CFGURL", RCMD_CFGURL, 1},        /* ATAMS="CFGINIT","[ip|port]" */
  {"CFGACCOUNT", RCMD_CFGACCOUNT, 1},/* ATAMS="CFGINIT","[user|password]" */
  {"CFGCANCEL", RCMD_CANCEL, 1},       /* ATAMS="CFGCANCEL","[suiteid]" */
  {"CFGCANCELALL", RCMD_CANCELALL, 0}     /* ATAMS="CFGCANCELALL" */
};

static int atcommandLen = sizeof(atcommands)/sizeof(atcommands[0]);

static TPALRequestObject* processATComand(char* atcommand, char* instr, int param);
static int executeTPALCommand(TPALRequestObject *request, char **outstr);
static void freeTPALRequestObject(TPALRequestObject *request);
static int hasStartWith(char *str, char *searchStr, int *size);

#endif //ARCH_ARM_SPD

int cpl_handleATRequest(char* atcommand, char* instr, char** outstr)
{
  int result = 0;
#ifdef ARCH_ARM_SPD
  static int s_hasInitJava = 0;
  char *arg[] = {"-native-ams"};
  TPALRequestObject *request = NULL;
  int retryCount = 3;

  while(RmtMain_isVMActive() == 0)
  {
    if (retryCount == 0)
    {
      int len = strlen("Starting VM...\r\nPlease retry it later");
      *outstr = malloc(len + 1);
      memset(*outstr, 0x0, len + 1);
      memcpy(*outstr, "Starting VM...\r\nPlease retry it later", len);
      return 1;
    }
    SCI_Sleep(1000);
    retryCount--;
  };

  request = processATComand(atcommand, instr, -1);
  result = executeTPALCommand(request, outstr);
  freeTPALRequestObject(request);
  request = NULL;
#endif //ARCH_ARM_SPD
  return result;
}

int cpl_handleATRequestWithNumericParam(char* atcommand, int param, char** outstr)
{
  int result = 0;
#ifdef ARCH_ARM_SPD
  TPALRequestObject *request = processATComand(atcommand, NULL, param);
  result = executeTPALCommand(request, outstr);
  freeTPALRequestObject(request);
  request = NULL;
#endif
  return result;
}

#ifdef ARCH_ARM_SPD
static TPALRequestObject* processATComand(char* atcommand, char* instr, int param)
{
  int i = 0;
  TPALRequestObject *request = NULL;

  if (atcommand)
  {
    request = malloc(sizeof(TPALRequestObject));
    memset(request, 0x0, sizeof(TPALRequestObject));
    request->param = -1;

    for (i = 0; i < atcommandLen; i++)
    {
      if (strcmp(atcommand, atcommands[i].cmd) == 0)
      {
        request->cmdId = atcommands[i].cmdId;
        request->pData = NULL;
        if (atcommands[i].hasParam > 0)
        {
          if (instr)
          {
            int len = strlen(instr);
            request->pData = malloc(len + 1);
            memset(request->pData, 0x0, len + 1);
            memcpy(request->pData, instr, len);
          }
          else if (param >= 0)
          {
            request->param = param;
          }
          else
          {
            //Invalid params
            freeTPALRequestObject(request);
            request = NULL;
            break;
          }
        }
        break;
      }
    }
  }

  return request;
}

static void freeTPALRequestObject(TPALRequestObject *request)
{
  if (request != NULL)
  {
    if (request->pData != NULL)
    {
      free(request->pData);
    }
    free(request);
  }
}

static int executeTPALCommand(TPALRequestObject *request, char **outstr)
{
  int result = 0;
  int suiteId = -1;
  int len = 0;
  char *dataPtr = NULL;
  char buf[255] = {0};
  int retryCount = 3;

  if ((request == NULL) || (outstr == NULL))
  {
    return -1;
  }

  if (request->pData != NULL)
  {
    len = strlen(request->pData);
    dataPtr = malloc(len + 1);
    memset(dataPtr, 0x0, len + 1);
    memcpy(dataPtr, request->pData, len);
  }
  else if (request->param > 0)
  {
    memset(buf, 255, 0x0);
    sprintf(buf, "%d", request->param);
    len = strlen(buf);
    dataPtr = malloc(len + 1);
    memset(dataPtr, 0x0, len + 1);
    memcpy(dataPtr, buf, len);
  }
  else
  {
    dataPtr = NULL;
  }

  switch(request->cmdId)
  {
  case RCMD_DELETE:
  case RCMD_RUN:
  case RCMD_DESTROY:
  case RCMD_CANCEL:
    suiteId = request->param;
    //TODO:result = ReceiveRemoteCmdEx(request->cmdId, suiteId, NULL, NULL);
    break;
  case RCMD_LIST:
  case RCMD_STATUS:
    //TODO:result = ReceiveRemoteCmdEx(request->cmdId, suiteId, dataPtr, outstr);
    break;
  case RCMD_OTA:
  case RCMD_OSGI:
    while(RmtMain_isGPRSActive() == 0)
    {
      if (retryCount == 0)
      {
        int len = strlen("Network Connecting...\r\nPlease retry it later");
        *outstr = malloc(len + 1);
        memset(*outstr, 0x0, len + 1);
        memcpy(*outstr, "Network Connecting...\r\nPlease retry it later", len);
        return 1;
      }
      SCI_Sleep(1000);
      retryCount--;
    };
  default:
    //TODO: result = ReceiveRemoteCmdEx(request->cmdId, suiteId, dataPtr, NULL);
    break;
  }

  if ((outstr != NULL) && ((*outstr == NULL) || strlen(*outstr) == 0))
  {
    if (suiteId >= 0)
    {
      memset(buf, 255, 0x0);
      sprintf(buf, "sid:%d", suiteId);
      len = strlen(buf);
      dataPtr = malloc(len + 1);
      memset(dataPtr, 0x0, len + 1);
      memcpy(dataPtr, buf, len);
      *outstr = dataPtr;
    }
  }
  return result;
}
#endif //ARCH_ARM_SPD

void cpl_freeOutStr(char** outstr)
{
#ifdef ARCH_ARM_SPD
  if (*outstr != NULL)
  {
    free(*outstr);
    *outstr = NULL;
  }
#endif
}
