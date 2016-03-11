#include <std_global.h>
#include <eventsystem.h>
#include <opl_file.h>
#include <jarparser.h>
#include <opl_rams.h>
#include <encoding.h>
#include <upcall.h>
#include <init.h>
#include <vm_common.h>
#include <vm_app.h>
#include <ams.h>
#include <ams_remote.h>
#include <ams_utils.h>

/* FSM state definitions of EVT_CMD_DECLARE */
#define DECLARE_FSM_STARTUP    0x01
#define DECLARE_FSM_AUTOSTART  0x02
#define DECLARE_FSM_CONNECT    0x03
#define DECLARE_FSM_READ       0x04
#define DECLARE_FSM_WRITE      0x05
#define DECLARE_FSM_CLOSE      0x06
#define DECLARE_FSM_SHUTDOWN   0x07

#define DATA_BUF_SIZE (256)

static int32_t servInstance;
static int16_t currentFsmState;

/**
 * RAMS command actions.
 */
static int32_t ams_remote_buildConnection(Event *evt, void *userData);

static void ams_remote_callbackHandler(AmsCBData * cbdata);

/**
 * Parse App ID from command data, and verify whether the data length is correct.
 * @param data, raw data from server side.
 * @param dataLen, data length.
 * @return App ID (positive) or error value in EvtSysResult_e (negative).
 */
static int32_t parseAndVerifyAppId(uint8_t* data, int dataLen)
{
    uint8_t* p = data;
    int32_t length = readbeIU32(p += 4);
    int32_t appId = readbeIU32(p += 4);

    if (dataLen != 16 + 4 + length)
    {
        DVMTraceErr("parseAndVerifyAppId: Error, wrong data length.\n");
        return EVT_RES_FAILURE;
    }
    return appId;
}

/**
 * Parse URL from command data, and verify whether the data length is correct.
 * The caller should free the returned buffer if it is not NULL.
 * @param data, raw data from server side.
 * @param dataLen, data length.
 * @return URL if success, or NULL otherwise.
 */
static uint8_t* parseAndVerifyUrl(uint8_t* data, int dataLen)
{
    uint8_t* p = data;
    int32_t length = readbeIU32(p += 4);
    uint8_t * url = NULL;

    if (dataLen != 16 + 4 + length)
    {
        DVMTraceErr("parseAndVerifyUrl: Error, wrong data length.\n");
        return NULL;
    }

    url = (uint8_t*)CRTL_malloc(length + 1);
    if (url == NULL)
    {
        DVMTraceErr("parseAndVerifyUrl: alloc fail\n");
        return NULL;
    }

    CRTL_memset(url, 0, length + 1);
    CRTL_memcpy(url, p + 4, length);
    return url;
}

/**
 * Parse server commands.
 * @param data, raw data from server side.
 * @param dataBytes, raw data size in bytes.
 */
static int32_t parseServerCommands(uint8_t* data, int32_t dataBytes)
{
    uint8_t* p  = data;
    int32_t len = readbeIU32(p);
    int32_t cmd = readbeIU32(p+=4);
    int32_t reserve1 = readbeIU32(p+=4);
    int32_t reserve2 = readbeIU32(p+=4);
    int32_t res = cmd;
    Event   newEvt;

    DVMTraceErr("parseServerCommands:cmd - %d\n",cmd);
    switch (cmd)
    {
    case EVT_CMD_INSTALL:
        //not supported, ignore.
        break;

    case EVT_CMD_LIST:
        if (len != 16)
        {
            DVMTraceErr("parseServerCommands: Error, wrong data length.\n");
            res = EVT_RES_FAILURE;
            break;
        }
        Ams_listApp(ATYPE_RAMS);
        break;

    case EVT_CMD_CANCELALL:
        if (len != 16)
        {
            DVMTraceErr("parseServerCommands: Error, wrong data length.\n");
            res = EVT_RES_FAILURE;
            break;
        }
        Ams_cancelAllApp(ATYPE_RAMS);
        break;


    case EVT_CMD_DELETE:
        {
            int32_t appId = parseAndVerifyAppId(p, len);

            if (appId < 0)
            {
                res = EVT_RES_FAILURE;
            }
            else
            {
                Ams_deleteApp(appId, ATYPE_RAMS);
            }
        }
        break;

    case EVT_CMD_RUN:
        {
            int32_t appId = parseAndVerifyAppId(p, len);

            if (appId < 0)
            {
                res = EVT_RES_FAILURE;
            }
            else
            {
                Ams_runApp(appId, ATYPE_RAMS);
            }
        }
        break;

    case EVT_CMD_DESTROY:
        {
            int32_t appId = parseAndVerifyAppId(p, len);

            if (appId < 0)
            {
                res = EVT_RES_FAILURE;
            }
            else
            {
                Ams_destoryApp(appId, ATYPE_RAMS);
            }
        }
        break;

        case EVT_CMD_INIT:
        {
            int32_t appId = parseAndVerifyAppId(p, len);

            if (appId < 0)
            {
                res = EVT_RES_FAILURE;
            }
            else
            {
                  Ams_initApp(appId, ATYPE_RAMS);
            }
        }
        break;

        case EVT_CMD_CANCEL:
        {
            int32_t appId = parseAndVerifyAppId(p, len);

            if (appId < 0)
            {
                res = EVT_RES_FAILURE;
            }
            else
            {
                Ams_cancelInitApp(appId, ATYPE_RAMS);
            }
        }
        break;

    case EVT_CMD_OTA:
        {
            uint8_t * url = parseAndVerifyUrl(p, len);

            if (url == NULL)
            {
                res = EVT_RES_FAILURE;
            }
            else
            {
                DVMTraceDbg("ota:%s\n", url);
                Ams_otaApp(url, ATYPE_RAMS);
                CRTL_freeif(url);
            }
        }
        break;

    case EVT_CMD_TCK:
        {
            uint8_t * url = parseAndVerifyUrl(p, len);

            if (url == NULL)
            {
                res = EVT_RES_FAILURE;
            }
            else
            {
                DVMTraceDbg("tck:%s\n", url);
                Ams_tckApp(url, ATYPE_RAMS);
                CRTL_freeif(url);
            }
        }
        break;

    case EVT_CMD_NONE:
    default:
        break;
    }

    return res;
}

/**
 * It's a state machine for RAMS communication with server.
 * @param evt, current event.
 * @param userData, user data.
 */
static int32_t ams_remote_buildConnection(Event *evt, void *userData)
{
    int32_t evtId = UNMARK_EVT_ID(evt->evtId);

    if (evtId == EVT_CMD_DECLARE)
    {
        int32_t fsm_state = FSM_UNMARK(evt->fsm_state);

        if (fsm_state)

        switch (fsm_state)
        {
        case DECLARE_FSM_STARTUP:
            //DVMTraceErr("===DVM_command:---DECLARE_FSM_STARTUP");
            //TODISCUSS: Is it necessary to remove network check before app startup?
            //           For most of autostart apps needs network. And this change will
            //           demand all apps to add auto-connect function!
            //if (rams_startupNetwork() == RAMS_RES_SUCCESS)
            {
                evt->fsm_state = DECLARE_FSM_AUTOSTART;
                ES_pushEvent(evt);
            }
            break;

        case DECLARE_FSM_AUTOSTART:
            {
                RMTConfig *pp_cfg;
		   AppletProps *curApp;
		     char *name;
		   char ** appNames[MAX_APPS_NUM];
		   char *defaultSplitChar = "#";
		    int32_t id = -1;
                if (amsUtils_readConfigData(&pp_cfg))
                {
                    int appId1 = -1, appId2 = -1;

					name = malloc(strlen(pp_cfg->appName)+1);
                     memset(name, 0x0, strlen(pp_cfg->appName)+1);
					strcpy(name,pp_cfg->appName);
                    sscanf(pp_cfg->initData, "%*[s:]%i,%i", &appId1, &appId2);
                    if (appId1 >= 0 || appId2>=0)
                    {
                    		int i = 0;
						memset(appNames, 0x0, sizeof(appNames));
			       		amsRemote_split(appNames, name, defaultSplitChar);
				for (i=0; i < MAX_APPS_NUM; i++)
				{
					if (appNames[i] == NULL){
  						break;
					}
			 		  curApp = getAppletPropByName(appNames[i]);
					 if(curApp != NULL)
					 	{
					 		id = curApp->id;
					      		 Ams_runApp(id, ATYPE_RAMS);
						 }
				}

			 }
			 CRTL_freeif(name);
			amsUtils_releaseConfigData(&pp_cfg);
                }
                evt->fsm_state = DECLARE_FSM_CONNECT;
                ES_pushEvent(evt);
            }
            break;

        case DECLARE_FSM_CONNECT:
            //Better to check network status before connect
            if (rams_startupNetwork() != RAMS_RES_SUCCESS) break;
            if (rams_connectServer(RS_ADDRESS, RS_PORT, &servInstance) == RAMS_RES_SUCCESS)
            {
                evt->fsm_state = DECLARE_FSM_READ;
                ES_pushEvent(evt);
            }
            break;


        case DECLARE_FSM_READ:
            {
                SafeBuffer *recvBuf = NULL;
                if (userData != NULL)
                {
                    recvBuf = (SafeBuffer *)userData;
                }
                else
                {
                    int32_t bufSize = (DATA_BUF_SIZE * sizeof(uint8_t) + sizeof(SafeBuffer));
                    recvBuf = (SafeBuffer*)CRTL_malloc(bufSize);
                    if (recvBuf == NULL)
                    {
                        return EVT_RES_FAILURE;
                    }
                    CRTL_memset(recvBuf, 0x0, bufSize);
                    recvBuf->pBuf = (uint8_t*)(((uint8_t*)recvBuf) + sizeof(SafeBuffer));
                    recvBuf->buffer_free = SafeBufferFree;
                }
                evt->userData = (void *)recvBuf; //saved into userData;
                recvBuf->bytes = rams_recvData(servInstance, recvBuf->pBuf, DATA_BUF_SIZE);

                if (recvBuf->bytes > 0)
                {
                    Event newEvt;
                    newNormalEvent(EVT_CMD_PARSER, FSM_STATE_UNSET, (void *)CreateSafeBuffer(recvBuf),
                                   ams_remote_buildConnection, &newEvt);
                    ES_pushEvent(&newEvt);
                }
                if (recvBuf->bytes != EVT_RES_WOULDBLOCK)
                {
                    /* try to get new commands from sever after 2000ms */
                    ES_scheduleAgainAsFirstTimeWithTimeout(2000);
                }
            }
            break;

        case DECLARE_FSM_WRITE:
            {
                SafeBuffer *sendBuf = NULL;
                if (userData != NULL)
                {
                    sendBuf = (SafeBuffer *)userData;
                }
                else
                {
                    sendBuf = (SafeBuffer *)evt->userData;
                }
                rams_sendData(servInstance, sendBuf->pBuf, sendBuf->bytes);
                sendBuf->buffer_free(sendBuf);
            }
            break;

        case DECLARE_FSM_CLOSE:
            rams_closeConnection(servInstance);
            break;

        case DECLARE_FSM_SHUTDOWN:
            rams_shutdownNetwork(servInstance);
            break;

        default:
            break;
        }
    }
    else if (evtId == EVT_CMD_PARSER)
    {
        SafeBuffer *data;
        if (userData != NULL)
        {
            data = (SafeBuffer *)userData;
        }
        else
        {
            data = (SafeBuffer *)evt->userData;
        }

        if (data == NULL)
        {
            /* no data */
            return EVT_RES_FAILURE;
        }

        parseServerCommands(data->pBuf, data->bytes);

        data->buffer_free(data);
    }

    return EVT_RES_SUCCESS;
}

/* see amsclient.h */
int32_t ams_remote_lifecycleProcess(Event *evt, void *userData)
{
    int32_t evtId = UNMARK_EVT_ID(evt->evtId);
    Event   newEvt;
    switch (evtId)
    {
    case EVT_SYS_INIT:

        Ams_regModuleCallBackHandler(ATYPE_RAMS,ams_remote_callbackHandler);
        //file_startup();
        //appletsList = listInstalledApplets(NULL);
        /* push connection event to queue */
        currentFsmState = DECLARE_FSM_STARTUP;
        newNormalEvent(EVT_CMD_DECLARE, currentFsmState, NULL, ams_remote_buildConnection, &newEvt);
        newEvt.priority = EP_HIGH;
        ES_pushEvent(&newEvt);
        break;

    case EVT_SYS_EXIT:
        //CRTL_freeif(appletsList);
        //file_shutdown();
        break;

    default:
        break;
    }
    return EVT_RES_SUCCESS;
}

bool_t ams_remote_list()
{
    AppletProps* p = NULL;
    int32_t   appNum;
    int32_t   i;
    int32_t     dataSize;
    uint8_t    *pb = NULL;
    SafeBuffer *safeBuf;
    int32_t     safeBufSize;

    p = vm_getCurApplist(TRUE);
    if (p == NULL) return 0;

    do
    {
        dataSize = 16;
        for (i = 0, appNum = 0; i < MAX_APPS_NUM; i++)
        {
            if (p[i].id != PROPS_UNUSED)
            {
                uint16_t len = (uint16_t)CRTL_strlen(p[i].name);
                if (pb != NULL)
                {
                    writebeIU16(&pb[dataSize], len+2/*2 bytes of appid*/);
                    writebeIU16(&pb[dataSize+2], (uint16_t)p[i].id);
                    CRTL_memcpy(&pb[dataSize+4], p[i].name, len);
                    appNum++;
                }
                dataSize += 4; //length + app id
                dataSize += len;
            }
        }
        if (pb != NULL)
        {
            Event newEvt;
            writebeIU32(&pb[0], dataSize);
            writebeIU32(&pb[4], ACK_CMD_LIST);
            writebeIU32(&pb[8], appNum);
            writebeIU32(&pb[12], 0); //reserved bytes

            newNormalEvent(EVT_CMD_DECLARE, DECLARE_FSM_WRITE, (void *)safeBuf, ams_remote_buildConnection, &newEvt);
            ES_pushEvent(&newEvt);
            break;
        }
        else
        {
            safeBufSize = dataSize + sizeof(SafeBuffer);
            safeBuf = CRTL_malloc(safeBufSize);
            if (safeBuf == NULL)
            {
                return EVT_RES_FAILURE;
            }
            CRTL_memset(safeBuf, 0x0, safeBufSize);
            safeBuf->pBuf = ((uint8_t*)(safeBuf)+sizeof(SafeBuffer));
            safeBuf->bytes = dataSize;
            safeBuf->buffer_free = SafeBufferFree;
            pb = safeBuf->pBuf;
        }
    } while(TRUE);

    return EVT_RES_SUCCESS;
}

bool_t ramsClient_isVMActive(void)
{
    return file_isFSRegistered();
}

void amsRemote_split( char **arr, char *str, const char *del)
{
 char *s =NULL; 

 s=strtok(str,del);
 while(s != NULL)
 {
  *arr++ = s;
  s = strtok(NULL,del);
 }
}


void ams_remote_callbackHandler(AmsCBData * cbdata)
{
    uint8_t     ackBuf[16] = {0x0,};
    uint8_t    *pByte;
    SafeBuffer *safeBuf;
    Event       newEvt;
    uint32_t res;

    if(cbdata ==NULL)
    {
        return;
    }
    if(cbdata->module != AMS_MODULE_RAMS)
    {
        return;
    }

    res = cbdata->result;
    switch(cbdata->cmd)
    {
    case RCMD_LIST:
        ams_remote_list();
        break;

    case RCMD_RUN:
        {
        AppletProps *curApp = vm_getCurActiveApp();

        if (curApp == NULL)
        {
            DVMTraceWar("No Applet in launching state\n");
            return ;
        }
        vm_setCurActiveAppState(res ? TRUE : FALSE);		
	curApp->isRunning = (res ? TRUE : FALSE);
	vm_setCurActiveApp(curApp);
	//curApp = vm_getCurActiveApp();

        pByte = (uint8_t*)ackBuf;
        writebeIU32(&pByte[0], sizeof(ackBuf));
        writebeIU32(&pByte[4], ACK_RECV_AND_EXEC);
        writebeIU32(&pByte[8], EVT_CMD_RUN);
        writebeIU32(&pByte[12], (res ? 1 : 0));

        safeBuf = CreateSafeBufferByBin(ackBuf, sizeof(ackBuf));
        newNormalEvent(EVT_CMD_DECLARE, DECLARE_FSM_WRITE, (void *)safeBuf, ams_remote_buildConnection, &newEvt);
        ES_pushEvent(&newEvt);
        }
        break;

    case RCMD_INIT:
        pByte = (uint8_t*)ackBuf;
        writebeIU32(&pByte[0], sizeof(ackBuf));
        writebeIU32(&pByte[4], ACK_RECV_AND_EXEC);
        writebeIU32(&pByte[8], EVT_CMD_INIT);
        writebeIU32(&pByte[12], (res ? 1 : 0));

        safeBuf = CreateSafeBufferByBin(ackBuf, sizeof(ackBuf));
        newNormalEvent(EVT_CMD_DECLARE, DECLARE_FSM_WRITE, (void *)safeBuf, ams_remote_buildConnection, &newEvt);
        ES_pushEvent(&newEvt);
        break;

    case RCMD_CANCEL:
        pByte = (uint8_t*)ackBuf;
        writebeIU32(&pByte[0], sizeof(ackBuf));
        writebeIU32(&pByte[4], ACK_RECV_AND_EXEC);
        writebeIU32(&pByte[8], EVT_CMD_CANCEL);
        writebeIU32(&pByte[12], (res ? 1 : 0));

        safeBuf = CreateSafeBufferByBin(ackBuf, sizeof(ackBuf));
        newNormalEvent(EVT_CMD_DECLARE, DECLARE_FSM_WRITE, (void *)safeBuf, ams_remote_buildConnection, &newEvt);
        ES_pushEvent(&newEvt);
        break;

    case RCMD_CANCELALL:
        pByte = (uint8_t*)ackBuf;
        writebeIU32(&pByte[0], sizeof(ackBuf));
        writebeIU32(&pByte[4], ACK_RECV_AND_EXEC);
        writebeIU32(&pByte[8], EVT_CMD_CANCELALL);
        writebeIU32(&pByte[12], (res ? 1 : 0));

        safeBuf = CreateSafeBufferByBin(ackBuf, sizeof(ackBuf));
        newNormalEvent(EVT_CMD_DECLARE, DECLARE_FSM_WRITE, (void *)safeBuf, ams_remote_buildConnection, &newEvt);
        ES_pushEvent(&newEvt);
        break;

    case RCMD_OTA:
        if(res==0){
            res = 1;
        }

        pByte = (uint8_t*)ackBuf;

        writebeIU32(&pByte[0], sizeof(ackBuf));
        writebeIU32(&pByte[4], ACK_RECV_AND_EXEC);
        writebeIU32(&pByte[8], EVT_CMD_OTA);
        writebeIU32(&pByte[12], res);

        safeBuf = CreateSafeBufferByBin(ackBuf, sizeof(ackBuf));
        newNormalEvent(EVT_CMD_DECLARE, DECLARE_FSM_WRITE, (void *)safeBuf, ams_remote_buildConnection, &newEvt);
        ES_pushEvent(&newEvt);
        break;

    case RCMD_TCK:
        if (res == 0){
            res = 1;
        }

        pByte = (uint8_t*)ackBuf;

        writebeIU32(&pByte[0], sizeof(ackBuf));
        writebeIU32(&pByte[4], ACK_RECV_AND_EXEC);
        writebeIU32(&pByte[8], EVT_CMD_TCK);
        writebeIU32(&pByte[12], res);

        safeBuf = CreateSafeBufferByBin(ackBuf, sizeof(ackBuf));
        newNormalEvent(EVT_CMD_DECLARE, DECLARE_FSM_WRITE, (void *)safeBuf, ams_remote_buildConnection, &newEvt);
        ES_pushEvent(&newEvt);
        break;

    case RCMD_DESTROY:
        if (vm_getCurActiveApp() == NULL)
        {
            DVMTraceWar("No Applet in launching state\n");
            return;
        }
        vm_setCurActiveAppState(res ? TRUE : FALSE);
        vm_setCurActiveApp(NULL);//destroyed success;

        pByte = (uint8_t*)ackBuf;
        writebeIU32(&pByte[0], sizeof(ackBuf));
        writebeIU32(&pByte[4], ACK_RECV_AND_EXEC);
        writebeIU32(&pByte[8], EVT_CMD_DESTROY);
        writebeIU32(&pByte[12], (res ? 1 : 0));

        safeBuf = CreateSafeBufferByBin(ackBuf, sizeof(ackBuf));
        newNormalEvent(EVT_CMD_DECLARE, DECLARE_FSM_WRITE, (void *)safeBuf, ams_remote_buildConnection, &newEvt);
        ES_pushEvent(&newEvt);
        break;

    case RCMD_DELETE:
        pByte = (uint8_t*)ackBuf;
        writebeIU32(&pByte[0], sizeof(ackBuf));
        writebeIU32(&pByte[4], ACK_RECV_AND_EXEC);
        writebeIU32(&pByte[8], EVT_CMD_DELETE);
        writebeIU32(&pByte[12], (res ? 1 : 0));

        safeBuf = CreateSafeBufferByBin(ackBuf, sizeof(ackBuf));
        newNormalEvent(EVT_CMD_DECLARE, DECLARE_FSM_WRITE, (void *)safeBuf, ams_remote_buildConnection, &newEvt);
        ES_pushEvent(&newEvt);
        break;
    }
}

