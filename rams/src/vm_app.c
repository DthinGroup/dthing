#include <std_global.h>
#include <eventsystem.h>
//#include <dvmdex.h>
#include <opl_file.h>
#include <jarparser.h>
#include <opl_rams.h>
#include <encoding.h>
#include <upcall.h>
#include <init.h>
#include <vm_common.h>
#include <vm_app.h>
#include <ams.h>
#include <opl_es.h>

/* align up to 4 bytes. */
#define MEM_ALIGN(x) ((int32_t)((x)+3)&(~3))

/* Global varible, applets list.*/
static AppletProps* appletsList;

/* Do we really need this? -_- */
static AppletProps* curActiveApp;

/*Global varible, app id*/
static int AppletId = 0;

/* free safe buffer */
void SafeBufferFree(void *p)
{
    CRTL_freeif(p);
}

/**
 * Create a new safe buffer according to a given safe buffer.
 * The src safe buffer data will copy into the new safe buffer.
 */
SafeBuffer* CreateSafeBuffer(SafeBuffer* src)
{
    SafeBuffer *newSafe;
    int32_t safeBufSize = MEM_ALIGN(src->bytes) + sizeof(SafeBuffer);

    newSafe = (SafeBuffer *)CRTL_malloc(safeBufSize);
    if (newSafe == NULL)
    {
        return NULL;
    }

    CRTL_memcpy(newSafe, src, safeBufSize);
    newSafe->pBuf = (uint8_t*)(((uint8_t*)newSafe) + sizeof(SafeBuffer));

    return newSafe;
}

/**
 * Create a new safe buffer by binary buffer and buffer size.
 * @param buf, the original buffer.
 * @param buf_size, the buffer size in bytes of original buffer.
 */
SafeBuffer* CreateSafeBufferByBin(uint8_t *buf, int32_t buf_size)
{
    SafeBuffer *newSafe;
    int32_t safeBufSize = buf_size + sizeof(SafeBuffer);

    newSafe = (SafeBuffer *)CRTL_malloc(safeBufSize);
    if (newSafe == NULL)
    {
        return NULL;
    }
    newSafe->pBuf = (uint8_t*)(((uint8_t*)newSafe) + sizeof(SafeBuffer));

    CRTL_memcpy(newSafe->pBuf, buf, buf_size);
    newSafe->bytes = buf_size;
    newSafe->buffer_free = SafeBufferFree;

    return newSafe;
}

/**
 * Interpret bytes as a little-endian IU16
 * @param p bytes
 * @return little-endian IU16 read from p
 */
int16_t readleIU16(const uint8_t* p)
{
    return (int16_t)((p[1] << 8) | p[0]);
}

/**
 * Interpret bytes as a little-endian IU32
 * @param p bytes
 * @return little-endian IU32 read from p
 */
int32_t readleIU32(const uint8_t* p)
{
    return
        (((int32_t)p[3]) << 24) |
        (((int32_t)p[2]) << 16) |
        (((int32_t)p[1]) <<  8) |
        (((int32_t)p[0]));
}

/**************************************************************/
/* streaming util APIs                                        */
/**************************************************************/
/**
 * Interpret bytes as a big-endian IU16
 * @param p bytes
 * @return big-endian IU16 read from p
 */
int16_t readbeIU16(const uint8_t* p)
{
    return (int16_t)((p[0] << 8) | p[1]);
}

/**
 * Interpret bytes as a big-endian IU32
 * @param p bytes
 * @return big-endian IU32 read from p
 */
int32_t readbeIU32(const uint8_t* p)
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
void writebeIU16(uint8_t* p, uint16_t num)
{
    p[0] = (uint8_t)((num>>8)&0xff);
    p[1] = (uint8_t)(num&0xff);
}

/**
 * Write an unsigned int into a bytes memory.
 * @p, the memory pointer saves bytes.
 * @num, IU32 number will write to p.
 */
void writebeIU32(uint8_t* p, uint32_t num)
{
    p[0] = (uint8_t)((num>>24)&0xff);
    p[1] = (uint8_t)((num>>16)&0xff);
    p[2] = (uint8_t)((num>>8)&0xff);
    p[3] = (uint8_t)(num&0xff);
}

/**
 * get default applet installed path.
 * it should be configurable in different project.
 * TODO: change this api to configurable.
 */
uint16_t* getDefaultInstalledPath()
{
    return file_getDthingWDir();
}

uint8_t* trim(uint8_t* value)
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

AppletProps* getAppletPropById(int32_t id)
{
    int32_t i;
    AppletProps *pap;
    for (i = 0, pap = appletsList; i < MAX_APPS_NUM; i++)
    {	 
        if (pap[i].id == id && id > -1)//id > -1 is to make sure value right
        {     
            return &pap[i];
        }
    }
    return NULL;
}

AppletProps* getAppletPropByName(char *name)
{
    int32_t i;
    AppletProps *pap;
    char *papname = NULL;
    for (i = 0, pap = appletsList; i < MAX_APPS_NUM; i++)
    {
	papname = pap[i].frealName;
	if(strcmp(papname,name)==0)    
        {
            return &pap[i];
        }
    }
    return NULL;
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
bool_t parseAppletProps(uint8_t* data, int32_t dataBytes, AppletProps* appProp)
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

uint8_t* combineAppPath(uint16_t* appName)
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

uint16_t* getFileNameFromPath(uint16_t* path)
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
    AppletProps *pAppProp = NULL;
   
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
      //if(appletsList != NULL){   //should we clear the applist when we repite the malloc size below?;  
      //    vm_clearApplist();
      //}
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
            DVMTraceWar("vm_app.c: file_listNextEntry[%d] index[%d]\n", result, index);
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
                {
                    DVMTraceWar("vm_app.c: failed to openJar! continue\n");
                    continue;
                }
                data = getJarContentByFileName(handle, MANIFEST_MF, &dataBytes);
                if (data == NULL)
                {
                    DVMTraceWar("listInstalledApplets: invalid data\n");
                    closeJar(handle);
                    continue;
                }
                appList[index].id = index; //set id to index                     
                if (index > 0)
                {
                    appList[index - 1].nextRunning = &appList[index];
                }
                {
                    /* save file name for delete */
                    int32_t  len = 0;
                    uint16_t *p = getFileNameFromPath(foundJarPath);
                    uint8_t*  fpath;
                    uint8_t* jarSuffix = ".jar";
                    int32_t   len_name = CRTL_wcslen(p );
                    fpath = CRTL_malloc((len_name+1)*3);//utf8 encoding.                   
                    if (fpath != NULL)
                    {                       
                        CRTL_memset(fpath, 0x0, (len_name+1)*3);
                        convertUcs2ToUtf8(p, len_name, fpath, len_name*3);
                        amsUtils_del(fpath,jarSuffix);
                        CRTL_memcpy(appList[index].frealName, fpath, len_name*3);                    
                    }
                    CRTL_free(fpath);
                    if ((pAppProp = getAppletPropByName(appList[index].frealName)) != NULL)
                    {
                        //appList[index].fname = pAppProp->fname;
                        appList[index].fpath= pAppProp->fpath;
                        //appList[index].id= pAppProp->id;
                        appList[index].isRunning= pAppProp->isRunning;
                        //appList[index].mainCls= pAppProp->mainCls;
                        //appList[index].version= pAppProp->version;                 
                    }
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

//attention: not be protected by mutext,it's thread-unsafe
void refreshInstalledApp(void)
{
    appletsList = listInstalledApplets(NULL);
}

AppletProps* vm_getCurApplist(bool_t refresh)
{
    if(refresh)
        refreshInstalledApp();
    return  appletsList;
}

AppletProps* vm_getCurActiveApp(void)
{
    return curActiveApp;
}

void vm_clearApplist(void)
{
    AppletProps *currentApp = appletsList;
    AppletProps *nextApp = NULL;
    do
    {
        nextApp = currentApp->nextRunning;
        CRTL_freeif(currentApp->fpath);
        CRTL_freeif(currentApp);
        currentApp = nextApp;
    } while(nextApp);
}

void vm_setCurActiveApp(AppletProps * app)
{
    curActiveApp = app;
}

void vm_setCurActiveAppState(bool_t state)
{
    if(curActiveApp !=NULL)
        curActiveApp->isRunning = state;
}


/*----------------------vm op APIs-------------------------------*/
bool_t vm_runApp(int id)
{
    AppletProps *pap;
    uint8_t      ackBuf[16] = {0x0};
    uint8_t     *pByte;
    bool_t       res = TRUE;
    static char *argv[3];
    SafeBuffer  *safeBuf;
    Event        newEvt;

    do
    {
        if ((pap = getAppletPropById(id)) == NULL)
        {
            DVMTraceErr("runApplet failure, no such id(%d)\n", id);
            res = FALSE;
            break;
        }
    if(pap->isRunning){
            DVMTraceErr("This app is running, you can't run it again! \n");
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
        pap->isRunning = TRUE;
        vm_setCurActiveApp(pap);
    }
    while(FALSE);

    return res;
}

bool_t vm_deleteApp(int id)
{
    bool_t res = TRUE;
    AppletProps *pAppProp;
    char *pAppName;
    uint16_t     fpath[MAX_FILE_NAME_LEN] = {0x0,};
    Event        newEvt;
    int32_t i = -1;
    
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
        pAppName = pAppProp->frealName;
        if (pAppProp->isRunning)
        {
            //TODO: stop the running appliction.
            //remove this node from running linked list.
            DVMTraceErr("Destroy this applet first !");
            res = FALSE;
            break;
        }

        CRTL_wcscpy(fpath, getDefaultInstalledPath());
        CRTL_wcscat(fpath, pAppProp->fname);

        if (file_delete(fpath, CRTL_wcslen(fpath)) != FILE_RES_SUCCESS)
        {
            DVMTraceWar("deleteAppletById: remove application content failure");
            res = FALSE;
            break;
        }  
        //pAppProp->id = PROPS_UNUSED;
     if(amsUtils_checkConfigData(pAppName)){
        //TODO
       }else{
        //TODO
       }
       CRTL_memset(pAppProp, 0x0, sizeof(AppletProps)); //clear
    } while(FALSE);

    return res;
}

bool_t vm_destroyApp(int id)
{
    AppletProps *pap;
    Event        newEvt;

    if ((pap = getAppletPropById(id)) == NULL || !pap->isRunning)
    {
        DVMTraceErr("destroyApplet, wrong app id(%d) or this app is not running\n",id);
        return FALSE;
    }
    pap->isRunning = FALSE;
    vm_setCurActiveApp(pap);
    CRTL_freeif(pap->fpath);
    upcallDestroyApplet(pap);

    return TRUE;
}

bool_t vm_otaApp(char * url)
{
    bool_t res = TRUE;
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
        DVMTraceInf("===argv=0x%x,argv-1:%s,argv-2:%s,argv-3:%s\n",(void*)argv,argv[0],argv[1],argv[2]);

        if (Ams_createVMThread(VMThreadProc, 2, argv) < 0)
        {
            DVMTraceErr("lauch VM thread failure\n");
            res = FALSE;
        }
    }else{
        DVMTraceDbg("===vm is running,ota:%s\n",url);
        //DVM_ASSERT(0);
        vm_ota_set(TRUE,url);
    }

    return res;
}

bool_t vm_tckApp(char * url)
{
    bool_t res = TRUE;
    Event        newEvt;
    static char* argv[3];
    char * tckUrl = CRTL_malloc(CRTL_strlen(url) + 1);  //memery leak
    if (tckUrl == NULL)
    {
    }

    if (!IsDvmRunning())
    {
        CRTL_memset(tckUrl, 0, CRTL_strlen(url) + 1);
        CRTL_memcpy(tckUrl, url, CRTL_strlen(url));

        argv[0] = "-tck";
        argv[1] = tckUrl;
        argv[2] = NULL;
        DVMTraceInf("===argv=0x%x,argv-1:%s,argv-2:%s,argv-3:%s\n", (void*)argv, argv[0], argv[1], argv[2]);

        if (Ams_createVMThread(VMThreadProc, 2, argv) < 0)
        {
            DVMTraceErr("lauch VM thread failure\n");
            res = FALSE;
        }
    }
    else{
        DVMTraceDbg("===vm is running,tck:%s\n", url);
        //DVM_ASSERT(0);
        vm_tck_set(TRUE, url);
    }

    return res;
}

/*----------Special ota Handle--------------*/
//OTA
static bool_t s_ota_hang_flag;
static char   s_ota_addr[128];
static ES_Mutex * s_ota_mutex;
void vm_ota_init()
{
    s_ota_mutex = mutex_init();
    DVMTraceDbg("===s_ota_mutex:0x%x\n",s_ota_mutex);
    DVM_ASSERT(s_ota_mutex !=NULL);
    CRTL_memset(s_ota_addr,0,128);
    s_ota_hang_flag = FALSE;
}

void vm_ota_final()
{
    mutex_destory(s_ota_mutex);
    CRTL_memset(s_ota_addr,0,128);
    s_ota_hang_flag = FALSE;
}

void vm_ota_set(bool_t flag,char * url)
{
    mutex_lock(s_ota_mutex);
    if(flag)
    {
        CRTL_memset(s_ota_addr,0,128);
        CRTL_memcpy(s_ota_addr,url,CRTL_strlen(url));
        s_ota_hang_flag = TRUE;
    }
    else
    {
        CRTL_memset(s_ota_addr,0,128);
        s_ota_hang_flag = FALSE;
    }
    mutex_unlock(s_ota_mutex);
}

bool_t vm_ota_get()
{
    bool_t ret;
    mutex_lock(s_ota_mutex);
    ret = s_ota_hang_flag;
    mutex_unlock(s_ota_mutex);
    return ret;
}

void vm_create_otaTask()
{
    if(vm_ota_get() !=TRUE)
    {
        //nothing
    }
    else
    {
        uint8_t** newArgv = NULL;
        int32_t   newArgc = 0;
        ClassObject* otaClass = NULL;
        Method*      startMeth = NULL;
        ClassObject* dummyThreadCls = NULL;
        Object*      dummyThreadObj = NULL;
        ClassObject* strCls = NULL;
        ArrayObject* params = NULL;
        StringObject* strObj= NULL;

        otaClass = dvmFindClass("Ljava/net/ota/OTADownload;");
        startMeth = dvmGetStaticMethodID(otaClass, "OTA", "([Ljava/lang/String;)V");
        dummyThreadCls = dvmFindClass("Ljava/lang/Thread;");
        dummyThreadObj = dvmAllocObject(dummyThreadCls, 0);

        strCls = dvmFindClass("[Ljava/lang/String;");
        params = dvmAllocArrayByClass(strCls, 1, 0);

        strObj = NewStringUTF(s_ota_addr);
        dvmSetObjectArrayElement(params, 0, strObj);

        dthread_create_params(startMeth, dummyThreadObj, params);

        vm_ota_set(FALSE,NULL);
    }
}

/*----------Special tck Handle--------------*/
//TCK
static bool_t s_tck_hang_flag;
static char   s_tck_addr[128];
static ES_Mutex * s_tck_mutex;
void vm_tck_init()
{
    s_tck_mutex = mutex_init();
    DVMTraceDbg("===s_tck_mutex:0x%x\n", s_tck_mutex);
    DVM_ASSERT(s_tck_mutex != NULL);
    CRTL_memset(s_tck_addr, 0, 128);
    s_tck_hang_flag = FALSE;
}

void vm_tck_final()
{
    mutex_destory(s_tck_mutex);
    CRTL_memset(s_tck_addr, 0, 128);
    s_tck_hang_flag = FALSE;
}

void vm_tck_set(bool_t flag, char * url)
{
    mutex_lock(s_tck_mutex);
    if (flag)
    {
        CRTL_memset(s_tck_addr, 0, 128);
        CRTL_memcpy(s_tck_addr, url, CRTL_strlen(url));
        s_tck_hang_flag = TRUE;
    }
    else
    {
        CRTL_memset(s_tck_addr, 0, 128);
        s_tck_hang_flag = FALSE;
    }
    mutex_unlock(s_tck_mutex);
}

bool_t vm_tck_get()
{
    bool_t ret;
    mutex_lock(s_tck_mutex);
    ret = s_tck_hang_flag;
    mutex_unlock(s_tck_mutex);
    return ret;
}

void vm_create_tckTask()
{
    if (vm_tck_get() != TRUE)
    {
        //nothing
    }
    else
    {
        uint8_t** newArgv = NULL;
        int32_t   newArgc = 0;
        ClassObject* tckClass = NULL;
        Method*      startMeth = NULL;
        ClassObject* dummyThreadCls = NULL;
        Object*      dummyThreadObj = NULL;
        ClassObject* strCls = NULL;
        ArrayObject* params = NULL;
        StringObject* strObj = NULL;

        tckClass = dvmFindClass("Lcom/yarlungsoft/ams/TCKRunner;");
        startMeth = dvmGetStaticMethodID(tckClass, "launchFromNative", "([Ljava/lang/String;)V");
        dummyThreadCls = dvmFindClass("Ljava/lang/Thread;");
        dummyThreadObj = dvmAllocObject(dummyThreadCls, 0);

        strCls = dvmFindClass("[Ljava/lang/String;");
        params = dvmAllocArrayByClass(strCls, 1, 0);

        strObj = NewStringUTF(s_tck_addr);
        dvmSetObjectArrayElement(params, 0, strObj);

        dthread_create_params(startMeth, dummyThreadObj, params);

        vm_tck_set(FALSE, NULL);
    }
}

