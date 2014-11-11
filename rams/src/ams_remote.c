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

/* FSM state definitions of EVT_CMD_DECLARE */
#define DECLARE_FSM_STARTUP  0x01
#define DECLARE_FSM_CONNECT  0x02
#define DECLARE_FSM_READ     0x03
#define DECLARE_FSM_WRITE    0x04
#define DECLARE_FSM_CLOSE    0x05
#define DECLARE_FSM_SHUTDOWN 0x06

#define DATA_BUF_SIZE (256)



static int32_t servInstance;
static int16_t currentFsmState;





/**
 * RAMS command actions.
 */
static int32_t ams_remote_buildConnection(Event *evt, void *userData);


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
        Ams_listApp(ATYPE_RAMS,NULL);
        break;

    case EVT_CMD_DELETE:
        {
            int32_t length = readbeIU32(p+=4);
            int32_t appId  = readbeIU32(p+=4);
            SafeBuffer *safeBuf;
            int32_t safeBufSize;

            if (len != 16 + 4 + length)
            {
                DVMTraceErr("parseServerCommands: Error, wrong data length.\n");
                res = EVT_RES_FAILURE;
                break;
            }
            Ams_deleteApp(appId,ATYPE_RAMS,NULL);
        }
        break;

    case EVT_CMD_RUN:
        {
            int32_t length = readbeIU32(p+=4);
            int32_t appId  = readbeIU32(p+=4);
            SafeBuffer *safeBuf;
            int32_t safeBufSize;

            if (len != 16 + 4 + length)
            {
                DVMTraceErr("parseServerCommands: Error, wrong data length.\n");
                res = EVT_RES_FAILURE;
                break;
            }
            Ams_runApp(appId,ATYPE_RAMS,NULL);
        }
        break;

    case EVT_CMD_DESTROY:
        {
            int32_t length = readbeIU32(p+=4);
            int32_t appId  = readbeIU32(p+=4);
            SafeBuffer *safeBuf;
            int32_t safeBufSize;

            if (len != 16 + 4 + length)
            {
                DVMTraceErr("parseServerCommands: Error, wrong data length.\n");
                res = EVT_RES_FAILURE;
                break;
            }
            Ams_destoryApp(appId,ATYPE_RAMS,NULL);
        }
        break;

    case EVT_CMD_OTA:
        {
            int32_t length = readbeIU32(p+=4);
            uint8_t * url = NULL;
            
            if (len != 16 + 4 + length)
            {
                DVMTraceErr("parseServerCommands EVT_CMD_OTA: Error, wrong data length.\n");
                res = EVT_RES_FAILURE;
                break;
            }
            
            url = (uint8_t*)CRTL_malloc(length +1);
            if (url == NULL)
            {
                res = EVT_RES_FAILURE;
                DVMTraceErr("parseServerCommands EVT_CMD_OTA: alloc fail\n");
                break;
            }
            CRTL_memset(url,0,length+1);
            CRTL_memcpy(url,p+4,length);
            DVMTraceDbg("ota:%s\n",url);
            Ams_otaApp(url,ATYPE_RAMS,NULL);
            
            CRTL_freeif(url);
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
            if (rams_startupNetwork() == RAMS_RES_SUCCESS)
            {
                evt->fsm_state = DECLARE_FSM_CONNECT;
                ES_pushEvent(evt);
            }
            break;

        case DECLARE_FSM_CONNECT:
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

/* see ramsclient.h */
bool_t ams_remote_deleteAppletById(int32_t id)
{
    bool_t res = TRUE;
    AppletProps *pAppProp;
    uint16_t     fpath[MAX_FILE_NAME_LEN] = {0x0,};
    uint8_t      ackBuf[16] = {0x0,};
    uint8_t     *pByte;
    SafeBuffer  *safeBuf;
    Event        newEvt;

    do
    {
        if (id < 0)
        {
            DVMTraceWar("deleteAppletById: Invalid id (%d)", id);
            res = FALSE;
            break;
        }

        if ((pAppProp = getAppletPropById(id)) == NULL)
        {
            DVMTraceWar("deleteAppletById: Unknown id (%d)", id);
            res = FALSE;
            break;
        }

        if (pAppProp->isRunning)
        {
            //TODO: stop the running appliction.
            //remove this node from running linked list.
        }

        CRTL_wcscpy(fpath, getDefaultInstalledPath());
        CRTL_wcscat(fpath, pAppProp->fname);

        if (file_delete(fpath, CRTL_wcslen(fpath)) != FILE_RES_SUCCESS)
        {
            DVMTraceWar("deleteAppletById: remove application content failure");
            res = FALSE;
            break;
        }
        CRTL_memset(pAppProp, 0x0, sizeof(AppletProps)); //clear
        pAppProp->id = PROPS_UNUSED;

    } while(FALSE);

    pByte = (uint8_t*)ackBuf;

    writebeIU32(&pByte[0], sizeof(ackBuf));
    writebeIU32(&pByte[4], ACK_RECV_AND_EXEC);
    writebeIU32(&pByte[8], EVT_CMD_DELETE);
    writebeIU32(&pByte[12], (res ? 1 : 0));

    safeBuf = CreateSafeBufferByBin(ackBuf, sizeof(ackBuf));
    newNormalEvent(EVT_CMD_DECLARE, DECLARE_FSM_WRITE, (void *)safeBuf, ams_remote_buildConnection, &newEvt);
    ES_pushEvent(&newEvt);

    return res;
}

/* see ramsclient.h */
bool_t ams_remote_runApplet(int32_t id)
{
    AppletProps *pap;
    uint8_t      ackBuf[16] = {0x0};
    uint8_t     *pByte;
    bool_t       res = TRUE;
    static char *argv[3];
    SafeBuffer  *safeBuf;
    Event        newEvt;

    refreshInstalledApp();
    do
    {
        if ((pap = getAppletPropById(id)) == NULL)
        {
            DVMTraceErr("runApplet failure, no such id(%d)\n", id);
            res = FALSE;
            break;
        }
        pap->fpath = combineAppPath(pap->fname);

        argv[0] = "-run";
        argv[1] = pap->fpath;
        argv[2] = pap->mainCls;
        DVMTraceInf("argv=0x%x,argv-1:%s,argv-2:%s,argv-3:%s\n",(void*)argv,argv[0],argv[1],argv[2]);

        if (Ams_createVMThread(VMThreadProc, 3, argv) < 0)
        {
            DVMTraceErr("lauch VM thread failure\n");
            res = FALSE;
            break;
        }
        vm_setCurActiveApp(pap);
    }
    while(FALSE);
#ifdef NOT_LAUNCH_NET_TASK
    return FALSE;
#endif

    pByte = (uint8_t*)ackBuf;

    writebeIU32(&pByte[0], sizeof(ackBuf));

    if (res)
    {
        writebeIU32(&pByte[4], ACK_RECV_WITHOUT_EXEC);
        writebeIU32(&pByte[8], 0x1);
        writebeIU32(&pByte[12], 0x0);
    }
    else
    {
        writebeIU32(&pByte[4], ACK_RECV_AND_EXEC);
        writebeIU32(&pByte[8], EVT_CMD_RUN);
        writebeIU32(&pByte[12], 0x0);
    }

    safeBuf = CreateSafeBufferByBin(ackBuf, sizeof(ackBuf));
    newNormalEvent(EVT_CMD_DECLARE, DECLARE_FSM_WRITE, (void *)safeBuf, ams_remote_buildConnection, &newEvt);
    ES_pushEvent(&newEvt);

    return res;
}

/* see ramsclient.h */
bool_t ams_remote_destroyApplet(int32_t id)
{
    AppletProps *pap;
    uint8_t      ackBuf[16] = {0x0,};
    uint8_t     *pByte;
    SafeBuffer  *safeBuf;
    Event        newEvt;

    if ((pap = getAppletPropById(id)) == NULL || !pap->isRunning)
    {
        DVMTraceErr("destroyApplet, wrong app id(%d) or this app is not running\n");
        return FALSE;
    }
    CRTL_freeif(pap->fpath);
    upcallDestroyApplet(pap);

    pByte = (uint8_t*)ackBuf;

    writebeIU32(&pByte[0], sizeof(ackBuf));
    writebeIU32(&pByte[4], ACK_RECV_WITHOUT_EXEC);
    writebeIU32(&pByte[8], 0x1);
    writebeIU32(&pByte[12], 0x0);

    safeBuf = CreateSafeBufferByBin(ackBuf, sizeof(ackBuf));
    newNormalEvent(EVT_CMD_DECLARE, DECLARE_FSM_WRITE, (void *)safeBuf, ams_remote_buildConnection, &newEvt);
    ES_pushEvent(&newEvt);

    return TRUE;
}

bool_t ams_remote_ota(char * url)
{
    bool_t res = TRUE;
    uint8_t      ackBuf[16] = {0x0};
    uint8_t     *pByte;
    SafeBuffer  *safeBuf;
    Event        newEvt;
    static char* argv[3];
    char * otaUrl = CRTL_malloc(CRTL_strlen(url)+1);  //memery leak
    if(otaUrl ==NULL)
    {}

    if(!IsDvmRunning())
    {
        CRTL_memset(otaUrl,0,CRTL_strlen(url)+1);
        CRTL_memcpy(otaUrl,url,CRTL_strlen(url));

        argv[0] = "-ota";
        argv[1] = otaUrl;
        argv[2] = NULL;
        DVMTraceInf("argv=0x%x,argv-1:%s,argv-2:%s,argv-3:%s\n",(void*)argv,argv[0],argv[1],argv[2]);

        if (Ams_createVMThread(VMThreadProc, 2, argv) < 0)
        {
            DVMTraceErr("lauch VM thread failure\n");
            res = FALSE;
        }

        pByte = (uint8_t*)ackBuf;

        writebeIU32(&pByte[0], sizeof(ackBuf));
        writebeIU32(&pByte[4], ACK_RECV_WITHOUT_EXEC);
        writebeIU32(&pByte[8], 0x1);
        writebeIU32(&pByte[12], 0x0);
    }else{
        pByte = (uint8_t*)ackBuf;

        writebeIU32(&pByte[0], sizeof(ackBuf));
        writebeIU32(&pByte[4], ACK_RECV_AND_EXEC);
        writebeIU32(&pByte[8], 0x0);
        writebeIU32(&pByte[12], 0x0);
    }

    safeBuf = CreateSafeBufferByBin(ackBuf, sizeof(ackBuf));
    newNormalEvent(EVT_CMD_DECLARE, DECLARE_FSM_WRITE, (void *)safeBuf, ams_remote_buildConnection, &newEvt);
    ES_pushEvent(&newEvt);

    return res;
}

void ams_remote_sendOTAExeResult(int res)
{
    SafeBuffer  *safeBuf;
    Event        newEvt;
    uint8_t      ackBuf[16] = {0x0};
    uint8_t     *pByte;
    uint8_t  val = 0;
    if(res==0){
        val = 1;
    }

    pByte = (uint8_t*)ackBuf;

    writebeIU32(&pByte[0], sizeof(ackBuf));
    writebeIU32(&pByte[4], ACK_RECV_AND_EXEC);
    writebeIU32(&pByte[8], EVT_CMD_OTA);
    writebeIU32(&pByte[12], val);

    safeBuf = CreateSafeBufferByBin(ackBuf, sizeof(ackBuf));
    newNormalEvent(EVT_CMD_DECLARE, DECLARE_FSM_WRITE, (void *)safeBuf, ams_remote_buildConnection, &newEvt);
    ES_pushEvent(&newEvt);
}

/* see ramsclient.h */
bool_t ams_remote_sendBackExecResult(int32_t cmd, bool_t res)
{
    uint8_t     ackBuf[16] = {0x0,};
    uint8_t    *pByte;
    SafeBuffer *safeBuf;
    Event       newEvt;

    switch(cmd)
    {
        case EVT_CMD_RUN:
            if (vm_getCurActiveApp() == NULL)
            {
                DVMTraceWar("No Applet in launching state\n");
                return FALSE;
            }
            vm_setCurActiveAppState(res ? TRUE : FALSE);
            break;

        case EVT_CMD_DESTROY:
            if (vm_getCurActiveApp() == NULL)
            {
                DVMTraceWar("No Applet in launching state\n");
                return FALSE;
            }
            vm_setCurActiveAppState(res ? TRUE : FALSE);
            vm_setCurActiveApp(NULL);//destroyed success;
            break;

        case EVT_CMD_DELETE:
            break;
    }

    pByte = (uint8_t*)ackBuf;
    writebeIU32(&pByte[0], sizeof(ackBuf));
    writebeIU32(&pByte[4], ACK_RECV_AND_EXEC);
    writebeIU32(&pByte[8], cmd);
    writebeIU32(&pByte[12], (res ? 1 : 0));

    safeBuf = CreateSafeBufferByBin(ackBuf, sizeof(ackBuf));
    newNormalEvent(EVT_CMD_DECLARE, DECLARE_FSM_WRITE, (void *)safeBuf, ams_remote_buildConnection, &newEvt);
    ES_pushEvent(&newEvt);

    return TRUE;
}

bool_t ramsClient_isVMActive(void)
{
    return file_isFSRegistered();
}
