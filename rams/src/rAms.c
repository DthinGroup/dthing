#include <rAms.h>
#include <jarparser.h>
#include <opl_rams.h>
#include <opl_file.h>
#include <encoding.h>
#include <upcall.h>
#include <vm_common.h>
#include <init.h>


/**
 * Global varible, applets list.
 */
static AppletProps* appletsList;
/* for output event queue, means c->s */
static EventCmd*    eventCmdQueue;

/* Do we really need this? -_- */
static AppletProps* curActiveApp;

#define REC_TIMEOUT_VAL (2000)

//extern int32_t DVM_main(int32_t argc, char * argv[]);

/**
 * get default applet installed path.
 * it should be configurable in different project.
 * TODO: change this api to configurable.
 */
static uint16_t* getDefaultInstalledPath()
{
#if defined(ARCH_X86)
    //return L"D:\\nix.long\\ReDvmAll\\dvm\\appdb\\";
    return L"D:\\dvm\\appdb\\";
#elif defined(ARCH_ARM_SPD)
	return file_getDthingDir();
#endif
	return NULL;      
}

static uint8_t* trim(uint8_t* value)
{
    int32_t  len = CRTL_strlen(value);
    uint8_t* ps = value;
    uint8_t* pe = value + len -1;
    while (*ps == 0x20 || *ps == '\t') ps++;
    while (*pe == 0x20 || *pe == '\t') pe--;
    if (pe != (value + len -1))
        *pe = '\0';
    return ps;
}

static bool_t setPropsValue(uint8_t* key, AppletProps* appProp)
{
    int32_t  appNameLen    = (int32_t)CRTL_strlen(APP_NAME);
    int32_t  appVerionLen  = (int32_t)CRTL_strlen(APP_VERSION);
    int32_t  appVendorLen  = (int32_t)CRTL_strlen(APP_VENDOR);
    int32_t  appMainClsLen = (int32_t)CRTL_strlen(MAIN_CLASS);
    uint8_t* propVal;

    if (!CRTL_memcmp(key, APP_NAME, appNameLen) && key[appNameLen] == ':')
    {
        propVal = trim(&key[appNameLen+1]);
        CRTL_memcpy(appProp->name, propVal, CRTL_strlen(propVal));
    }
    else if (!CRTL_memcmp(key, APP_VERSION, appVerionLen) && key[appVerionLen] == ':')
    {
        propVal = trim(&key[appVerionLen+1]);
        CRTL_memcpy(appProp->version, propVal, CRTL_strlen(propVal));
    }
    else if (!CRTL_memcmp(key, APP_VENDOR, appVendorLen) && key[appVendorLen] == ':')
    {
        propVal = trim(&key[appVendorLen+1]);
        CRTL_memcpy(appProp->vendor, propVal, CRTL_strlen(propVal));
    }
    else if (!CRTL_memcmp(key, MAIN_CLASS, appMainClsLen) && key[appMainClsLen] == ':')
    {
        propVal = trim(&key[appMainClsLen+1]);
        CRTL_memcpy(appProp->mainCls, propVal, CRTL_strlen(propVal));
    }
    else
    {
        //Unsupported properties!
        return FALSE;
    }
    return TRUE;
}


/**
 * parse applet properties and save result into appProp.
 * @data, the raw data of applet properties which unzip from manifest.mf
 * @appProp, the ouput variable used to save the parsed properties.
 * NOTE:
 *      the properties including below items:
 *          Applet-Name:
 *          Applet-Vendor:
 *          Applet-Version:
 *          Main-Class:
 *      Others are be igo
 */
static bool_t parseAppletProps(uint8_t* data, int32_t dataBytes, AppletProps* appProp)
{
    int32_t  i;
    uint8_t* ps = data;

    if (data == NULL || appProp == NULL)
    {
        DVMTraceErr("parseAppletProps, Wrong arguments\n");
        return FALSE;
    }
    
    for (i = 0; i < dataBytes; i++)
    {
        if (data[i] != '\r')
        {
            continue;
        }
        data[i] = '\0';
        if (data[i+1] == '\n')
        {
            i++; //skip '\n'.
        }

        setPropsValue(ps, appProp);

        ps = &data[i+1];
    }
    
    if (ps < data + dataBytes)
    {
        //To handle the case there is no line ending in last line.
        setPropsValue(ps, appProp);
    }

    return TRUE;
}

static uint16_t* getFileNameFromPath(uint16_t* path)
{
    uint16_t* rec = NULL;
    uint16_t* p = path;

    while(p != NULL && *p != '\0')
    {
        if (*p == '\\' || *p == '/')
        {
            rec = p+1;
        }
        p++;
    }
    return rec;
}

/**
 * Get the applets list in the specified installed path.
 * if path is null, the default installed path will be used.
 *
 * @param path, the installed path.
 * @return the applets list with pre-defined struct.
 */
static AppletProps* listInstalledApplets(const uint16_t* path)
{
    uint8_t*     data = NULL;
    bool_t       res = TRUE;
    uint16_t*    ins_path = getDefaultInstalledPath();
    AppletProps* appList = NULL;

    if (path != NULL)
    {
        ins_path = (uint16_t*)path;
    }

    do
    {
        int32_t   index = 0;
        int32_t   handle = 0;
        int32_t   result = 0;
        int32_t   pathLen = CRTL_wcslen(ins_path);
        uint16_t  foundJarPath[MAX_FILE_NAME_LEN];

        if (file_listOpen(path, pathLen, &handle) != FILE_RES_SUCCESS)
        {
            res = FALSE;
            break;
        }

        appList = (AppletProps*)CRTL_malloc(sizeof(AppletProps) * MAX_APPS_NUM);
        if (appList == NULL)
        {
            res = FALSE;
            break;
        }
        CRTL_memset(appList, 0x0, sizeof(AppletProps) * MAX_APPS_NUM);
        for (index = 0; index < MAX_APPS_NUM; index++)
        {
            //set all props nodes to default value - PROPS_UNUSED.
            appList[index].id = PROPS_UNUSED;
        }
        index = 0;

        do
        {
            CRTL_memset(foundJarPath, 0x0, MAX_FILE_NAME_LEN);
            result = file_listNextEntry(ins_path, pathLen, foundJarPath, MAX_FILE_NAME_LEN<<1, &handle);
            
            if (!(result > 0))
            {
                if (result < 0) res = FALSE;
                break;
            }
            else if (foundJarPath[0] != '\0')
            {
                int32_t dataBytes;
                int32_t handle;

                if (index == MAX_APPS_NUM)
                {
                    //should we realloc memory here?
                    DVMTraceWar("listInstalledApplets: max app number is reached\n");
                    //res = FALSE;
                    break;
                }

                if ((handle = openJar(foundJarPath)) < 0)
                    continue;
                data = getJarContentByFileName(handle, MANIFEST_MF, &dataBytes);
                if (data == NULL)
                {
                    DVMTraceWar("listInstalledApplets: invalid data\n");
                    closeJar(handle);
                    continue;
                }
                appList[index].id = index; //set id to index
                {
                    /* save file name for delete */
                    int32_t  len;
                    uint16_t *p = getFileNameFromPath(foundJarPath);
                    len = CRTL_wcslen(p);
                    CRTL_memcpy(appList[index].fname, p, len*sizeof(uint16_t));
                    appList[index].fname[len] = '\0';
                }
                parseAppletProps(data, dataBytes, &appList[index++]);
                closeJar(handle);
            }

        } while(foundJarPath[0] != '\0' && result > 0);

        file_listclose(&handle);

    } while(FALSE);

    CRTL_freeif(data);
    if (!res)
    {
        CRTL_freeif(appList);
        return NULL;
    }
    return appList;
}

/**
 * Interpret bytes as a little-endian IU16
 * @param p bytes
 * @return little-endian IU16 read from p
 */
static int16_t readleIU16(const uint8_t* p)
{
    return (int16_t)((p[1] << 8) | p[0]);
}

/**
 * Interpret bytes as a little-endian IU32
 * @param p bytes
 * @return little-endian IU32 read from p
 */
static int32_t readleIU32(const uint8_t* p)
{
    return
        (((int32_t)p[3]) << 24) |
        (((int32_t)p[2]) << 16) |
        (((int32_t)p[1]) <<  8) |
        (((int32_t)p[0]));
}

/**
 * Interpret bytes as a big-endian IU16
 * @param p bytes
 * @return big-endian IU16 read from p
 */
static int16_t readbeIU16(const uint8_t* p)
{
    return (int16_t)((p[0] << 8) | p[1]);
}

/**
 * Interpret bytes as a big-endian IU32
 * @param p bytes
 * @return big-endian IU32 read from p
 */
static int32_t readbeIU32(const uint8_t* p)
{
    return
        (((int32_t)p[0]) << 24) |
        (((int32_t)p[1]) << 16) |
        (((int32_t)p[2]) <<  8) |
        (((int32_t)p[3]));
}

/** 
 * Write an unsigned short into a bytes memory.
 * @p, the memory pointer saves bytes.
 * @num, IU16 number will write to p.
 */
static void writebeIU16(uint8_t* p, uint16_t num)
{
    p[0] = (uint8_t)((num>>8)&0xff);
    p[1] = (uint8_t)(num&0xff);
}

/** 
 * Write an unsigned int into a bytes memory.
 * @p, the memory pointer saves bytes.
 * @num, IU32 number will write to p.
 */
static void writebeIU32(uint8_t* p, uint32_t num)
{
    p[0] = (uint8_t)((num>>24)&0xff);
    p[1] = (uint8_t)((num>>16)&0xff);
    p[2] = (uint8_t)((num>>8)&0xff);
    p[3] = (uint8_t)(num&0xff);
}


static EventCmd* getAvailable()
{
    int32_t i;
    EventCmd* pec = eventCmdQueue;
    for (i = 0; i < EVT_QUEUE_SIZE; i++)
    {
        if (pec[i].length == 0)
        {
            return &pec[i];
        }
    }
    return NULL;
}

/**
 * Package the client acknowledgement data.
 * @ack_magic, command magic nubmer for response. seed definintions in rAms.h
 * @packBuf, the data need to be packaged.
 * @bufSize, the packBuf size in bytes.
 * @return the packaged data size; or 0 means no data packaged.
 */
static int32_t packageClinetACK(int32_t ack_magic, uint8_t* packBuf, int32_t bufSize)
{
    int32_t   dataSize = 0;
    EventCmd* pEC;
    uint8_t*  pb = NULL;

    switch (ack_magic)
    {
    case ACK_LIST:
        {
            AppletProps* p = appletsList;
            int32_t appNum;
            int32_t i;

            if (p == NULL) return 0;
            UNUSED(packBuf);
            UNUSED(bufSize)

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
                    writebeIU32(&pb[0], dataSize);
                    writebeIU32(&pb[4], ack_magic);
                    writebeIU32(&pb[8], appNum);
                    writebeIU32(&pb[12], 0); //reserved bytes
                    break;
                }
                else
                {
                    pb = (uint8_t*)CRTL_malloc(dataSize);
                    if (pb == NULL)
                    {
                        DVMTraceErr("packageClinetACK, not enough memory!\n");
                        return 0;
                    }
                    CRTL_memset(pb, 0x0, dataSize);
                }
            } while(TRUE);
        }
        break;

    case ACK_RECV_WITHOUT_EXEC:
        {
            int32_t res;
            res = ((int32_t*)packBuf)[0];
            dataSize = 16;
            pb = (uint8_t*)CRTL_malloc(16);
            if (pb == NULL)
            {
                DVMTraceErr("packageClinetACK, not enough memory!\n");
                return 0;
            }
            CRTL_memset(pb, 0x0, dataSize);

            writebeIU32(&pb[0], dataSize);
            writebeIU32(&pb[4], ack_magic);
            writebeIU32(&pb[8], res);
            writebeIU32(&pb[12], 0);
        }
        break;

    case ACK_RECV_EXEC:
        {
            int32_t   cmd;
            int32_t   res;

            cmd = ((int32_t*)packBuf)[0];
            res = ((int32_t*)packBuf)[1];
            dataSize = 16;

            pb = (uint8_t*)CRTL_malloc(16);
            if (pb == NULL)
            {
                DVMTraceErr("packageClinetACK, not enough memory!\n");
                return 0;
            }
            CRTL_memset(pb, 0x0, dataSize);

            writebeIU32(&pb[0], dataSize);
            writebeIU32(&pb[4], ack_magic);
            writebeIU32(&pb[8], cmd);
            writebeIU32(&pb[12], res);
        }
        break;

    case ACK_SHUTDOWN_NW:
        break;
    }

    /* save packaged data into event queue */
    pEC = getAvailable();
    if (pEC == NULL)
    {
        DVMTraceErr("parseServerCommands: Error, The QUEUE is full.\n");
	 CRTL_freeif(pb);
        return 0;        
    }
    pEC->length = dataSize;
    pEC->evtData = pb;

    return dataSize;
}



static int32_t parseAndActionServerCommands(uint8_t* data, int32_t dataBytes)
{
    uint8_t* p  = data;
    int32_t len = readbeIU32(p);
    int32_t cmd = readbeIU32(p+=4);
    int32_t reserve1 = readbeIU32(p+=4);
    int32_t reserve2 = readbeIU32(p+=4);
    int32_t res = cmd;

	DVMTraceErr("parseAndActionServerCommands:cmd - %d\n",cmd);
    switch (cmd)
    {
    case CMD_INSTALL:
        //not supported, ignore.
        break;

    case CMD_LIST:
        {
            if (len != 16)
            {
                DVMTraceErr("parseServerCommands: Error, wrong data length.\n");
                res = -1;
                break;
            }
            packageClinetACK(ACK_LIST, NULL, 0);

        }
        break;

    case CMD_DELETE:
    case CMD_RUN:
    case CMD_DESTROY:
        {
            int32_t length = readbeIU32(p+=4);
            int32_t appId  = readbeIU32(p+=4);
            if (len != 16 + 4 + length)
            {
                DVMTraceErr("parseServerCommands: Error, wrong data length.\n");
                res = -1;
                break;
            }
            if (cmd == CMD_DELETE)
            {
                deleteAppletById(appId);
            }
            else if (cmd == CMD_RUN)
            {
                runApplet(appId);
            }
            else if (cmd == CMD_DESTROY)
            {
                destroyApplet(appId);
            }
        }
        break;

    case CMD_OTA:
        break;

    case CMD_NONE:
    default:
        break;
    }
    
    return res;
}

/**
 * Thread process function. used to handle commands/events which are
 * sent from remote server. 
 * @argc, argv are reserved for the moment.
 * @return the exection result.
 */
static int32_t remoteAMSThreadProc(int argc, void* argv[])
{
    /* 1024 bytes should be enough for recieving data. */
    #define REC_BUF_SIZE 1024
    int32_t rsHandle;
    uint8_t recvBuf[REC_BUF_SIZE];
    UNUSED(argc);
    UNUSED(argv);
    
    rsHandle = getRemoteServerInstance(RS_ADDRESS, RS_PORT);
    if (rsHandle <= 0)
    {
        DVMTraceErr("remoteAMSThreadProc: create remote server instance failure\n");
        return -1;
    }
    
    eventCmdQueue = (EventCmd*)CRTL_malloc(sizeof(EventCmd) * EVT_QUEUE_SIZE);
    if (eventCmdQueue == NULL)
    {
        DVMTraceErr("remoteAMSThreadProc: no enough memory\n");
        destroyRemoteServerInstance(rsHandle);
        return -1;
    }
    CRTL_memset(eventCmdQueue, 0x0, sizeof(EventCmd) * EVT_QUEUE_SIZE);

    do
    {
        int32_t i;
        int32_t gotBytes;
        int32_t res;
        CRTL_memset(recvBuf, 0x0, REC_BUF_SIZE);
        gotBytes = recvDataWithTimeout(rsHandle, recvBuf, REC_BUF_SIZE, REC_TIMEOUT_VAL);
        if (gotBytes <= 0 || (res = parseAndActionServerCommands(recvBuf, gotBytes)) < 0)
        {
            //unexpected result, re-loop;
            //DVMTraceWar("unexpected result, re-loop?");
            //continue;
        }

        /* should handle cmd_exit here if any. */
        /* if (res == CMD_EXIT) break; */
        
        /* loop event command queue to send response to server */
        for (i = 0; i < EVT_QUEUE_SIZE; i++)
        {
            if (eventCmdQueue[i].length != 0)
            {
            	DVMTraceErr("cmd ack!\n");
                sendData(rsHandle, eventCmdQueue[i].evtData, eventCmdQueue[i].length);
                eventCmdQueue[i].length = 0;
                CRTL_freeif(eventCmdQueue[i].evtData);
            }
        }

        
    } while(TRUE);

    destroyRemoteServerInstance(rsHandle);

    return 0;
}

/* see rAms.h */
bool_t launchRemoteAMSClient(bool_t ramsIsolate, int32_t argc, uint8_t* argv[])
{
    int32_t threadHandle;
    file_startup();
    appletsList = listInstalledApplets(NULL);
	DVMTraceInf("App list ptr:0x%x\n",appletsList);
    if (ramsIsolate)
    {
        threadHandle = ramsCreateClientThread(remoteAMSThreadProc, argc, argv);
        if (threadHandle < 0)
            return FALSE;
    }

    remoteAMSThreadProc(argc, argv);
    return TRUE;
}

static AppletProps* getAppletPropById(int32_t id)
{
    int32_t i;
    AppletProps *pap;
    for (i = 0, pap = appletsList; i < MAX_APPS_NUM; i++)
    {
        if (pap[i].id == id)
        {
            return pap;
        }
    }
    return NULL;
}

/* see rAms.h */
bool_t deleteAppletById(int32_t id)
{
    bool_t res = TRUE;
    AppletProps* pAppProp;
    uint16_t fpath[MAX_FILE_NAME_LEN] = {0x0,};
    uint8_t  ackBuf[8] = {0x0,};
    int32_t* pint;

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

    pint = (int32_t*)ackBuf;
    pint[0] = CMD_DELETE;
    pint[1] = res ? 1 : 0;
    packageClinetACK(ACK_RECV_EXEC, ackBuf, 8);

    return res;
}

static uint8_t* combineAppPath(uint16_t* appName)
{
    int32_t   len = CRTL_wcslen(getDefaultInstalledPath()) + CRTL_wcslen(appName);
    uint16_t  fUcsPath[MAX_FILE_NAME_LEN] = {0x0,};
    uint8_t*  fpath;

    CRTL_wcscpy(fUcsPath, getDefaultInstalledPath());
    CRTL_wcscat(fUcsPath, appName);

    fpath = CRTL_malloc((len+1)*3);//utf8 encoding.
    if (fpath != NULL) 
    {
        CRTL_memset(fpath, 0x0, (len+1)*3);
        convertUcs2ToUtf8(fUcsPath, len, fpath, len*3);
    }

    return fpath;//this space will be freed in destroyApplet.
}

static int32_t VMThreadProc(int argc, char* argv[])
{
	DVMTraceInf("Enter dvm thread,argc=%d,argv=0x%x\n",argc,(void*)argv);
	DVMTraceInf("argv-0:%s,argv-1:%s,argv-2:%s\n",argv[0],argv[1],argv[2]);
    DVMTraceInf("Enter dvm thread,sleep over,sizeof(int)=%d,sizeof(int32_t)=%d\n",sizeof(int),sizeof(int32_t));    
    DVM_main(argc, argv);
    return 0;
}

bool_t destroyApplet(int32_t id)
{
    AppletProps *pap;
    uint8_t  ackBuf[8] = {0x0,};
    int32_t* pint;

    if ((pap = getAppletPropById(id)) == NULL || !pap->isRunning)
    {
        DVMTraceErr("destroyApplet, wrong app id(%d) or this app is not running\n");
        return FALSE;
    }
    CRTL_freeif(pap->fpath);
    //TODO, how to destroy this APP?
    upcallDestroyApplet(pap);

    pint = (int32_t*)ackBuf;
    pint[0] = CMD_DESTROY;
    pint[1] = 1;
    packageClinetACK(ACK_RECV_WITHOUT_EXEC, ackBuf, 8);

    return TRUE;
}

/* see rAms.h */
bool_t runApplet(int32_t id)
{
    AppletProps *pap;
    uint8_t  ackBuf[8] = {0x0};
    int32_t* pint;
    bool_t   res = TRUE;
    static char* argv[3];

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
        if (ramsCreateVMThread(VMThreadProc, 3, (void**)argv) < 0)
        {
            DVMTraceErr("lauch VM thread failure\n");
            res = FALSE;
            break;
        }
        curActiveApp = pap;
    }
    while(FALSE);

    pint = (int32_t*)ackBuf;
    pint[0] = CMD_RUN;
    if (res)
    {
        pint[1] = 1;
        packageClinetACK(ACK_RECV_EXEC, ackBuf, 8);
    }
    else
    {
        pint[1] = 0;
        packageClinetACK(ACK_RECV_WITHOUT_EXEC, ackBuf, 8);
    }
    DVMTraceInf("App run\n");
    return res;
}

/* see rAms.h */
bool_t sendBackExecResult(int32_t cmd, bool_t res)
{
    uint8_t  ackBuf[8] = {0x0,};
    int32_t* pint;

    if (curActiveApp == NULL)
    {
        DVMTraceWar("No Applet in launching state\n");
        return FALSE;
    }
    pint = (int32_t*)ackBuf;
    pint[0] = cmd;
    if (cmd == CMD_RUN)
    {
        pint[1] = res ? 1 : 0;
        curActiveApp->isRunning = res ? TRUE : FALSE;
    }
    else if (cmd == CMD_DESTROY)
    {
        pint[1] = res ? 1 : 0;
        curActiveApp->isRunning = res ? FALSE : TRUE;
        curActiveApp = FALSE; //destroyed success;
    }
    packageClinetACK(ACK_RECV_EXEC, ackBuf, 8);

    return TRUE;
}
