#include "std_global.h"
#include "vm_common.h"
#include "eventbase.h"
#include "opl_file.h"
#include "vm_app.h"
#include "ams.h"
#include "ams_remote.h"
#include "ams_sms.h"
#include "ams_at.h"

#ifdef ARCH_X86
    #pragma comment(lib, "Winmm.lib")
    #include <Windows.h>
    #include <Mmsystem.h>
#endif

#ifdef DVM_LOG
#undef DVM_LOG
#endif
#define DVM_LOG DVMTraceDbg
#define MAX_PATH_LENGTH   255


//default is Native Ams
static int32_t s_cur_ams_crtl_platform = AMS_MODULE_NAMS;


static int32_t Ams_handleAmsEvent(Event *evt, void *userData);
/*----------------------tool funcs-------------------------*/


void Ams_setCurCrtlModule(AMS_TYPE_E type)
{
    DVM_LOG("===Ams_setCurCrtlPlatform:%d\d",type);
    switch(type)
    {
        case ATYPE_NAMS: s_cur_ams_crtl_platform = AMS_MODULE_NAMS ; break;
        case ATYPE_RAMS: s_cur_ams_crtl_platform = AMS_MODULE_RAMS ; break;
        case ATYPE_AAMS: s_cur_ams_crtl_platform = AMS_MODULE_AAMS ; break;
        case ATYPE_SAMS: s_cur_ams_crtl_platform = AMS_MODULE_SAMS ; break;
    }
}

int Ams_getCurCrtlModule()
{
    return  s_cur_ams_crtl_platform;
}

int Ams_getCrtlModuleByType(AMS_TYPE_E type)
{
    switch(type)
    {
        case ATYPE_NAMS: return AMS_MODULE_NAMS ;
        case ATYPE_RAMS: return AMS_MODULE_RAMS ;
        case ATYPE_AAMS: return AMS_MODULE_AAMS ;
        case ATYPE_SAMS: return AMS_MODULE_SAMS ;
    }
}

void Ams_listApp(AMS_TYPE_E type,AmsCrtlCBFunc func)
{
    Event newEvt;
    switch(type)
    {
        case ATYPE_RAMS:
            newNormalEvent(AMS_MODULE_RAMS, AMS_FASM_STATE_GET_LIST, NULL, Ams_handleAmsEvent, &newEvt);
            ES_pushEvent(&newEvt);
            break;

        case ATYPE_AAMS:
            break;

        case ATYPE_SAMS:
            break;

        case ATYPE_NAMS:
            default:break;
    }

    Ams_setCurCrtlModule(type);
}

int Ams_runApp(int id,AMS_TYPE_E type,AmsCrtlCBFunc func)
{
    uint8_t idBuf[4] = {0x0,};
    uint8_t *pByte;
    SafeBuffer  *safeBuf;
    Event newEvt;
    switch(type)
    {
        case ATYPE_RAMS:
        {
            pByte = (uint8_t*)idBuf;
            writebeIU32(&pByte[0], id);
            safeBuf = CreateSafeBufferByBin(idBuf, sizeof(idBuf));
            newNormalEvent(AMS_MODULE_RAMS, AMS_FASM_STATE_GET_RUN, (void *)safeBuf, Ams_handleAmsEvent, &newEvt);
            ES_pushEvent(&newEvt);
        }
        break;

        case ATYPE_AAMS:
        break;

        case ATYPE_SAMS:
        break;

        case ATYPE_NAMS:
        default:break;
    }
    Ams_setCurCrtlModule(type);
    return 0;
}

int Ams_deleteApp(int id,AMS_TYPE_E type,AmsCrtlCBFunc func)
{
    uint8_t idBuf[4] = {0x0,};
    uint8_t *pByte;
    SafeBuffer  *safeBuf;
    Event newEvt;
    switch(type)
    {
        case ATYPE_RAMS:
        {
            pByte = (uint8_t*)idBuf;
            writebeIU32(&pByte[0], id);
            safeBuf = CreateSafeBufferByBin(idBuf, sizeof(idBuf));
            newNormalEvent(AMS_MODULE_RAMS, AMS_FASM_STATE_GET_DELETE, (void *)safeBuf, Ams_handleAmsEvent, &newEvt);
            ES_pushEvent(&newEvt);
        }
        break;

        case ATYPE_AAMS:
        break;

        case ATYPE_SAMS:
        break;

        case ATYPE_NAMS:
        default:break;
    }
    Ams_setCurCrtlModule(type);
    return 0;
}

int Ams_otaApp(uint8_t* url,AMS_TYPE_E type,AmsCrtlCBFunc func)
{
    uint8_t idBuf[4] = {0x0,};
    uint8_t *pByte;
    SafeBuffer  *safeBuf;
    int32_t safeBufSize;
	int32_t length;
    Event newEvt;
    switch(type)
    {
        case ATYPE_RAMS:
        {
			length = CRTL_strlen(url);
            safeBufSize = sizeof(SafeBuffer) + length + 1;
            safeBuf = (SafeBuffer *)CRTL_malloc(safeBufSize);
            if (safeBuf == NULL)
            {
                DVMTraceErr("Ams_otaApp ATYPE_RAMS: alloc fail\n");
                break;
            }
            CRTL_memset(safeBuf, 0x0, safeBufSize);

            safeBuf->pBuf = ((uint8_t*)(safeBuf)+sizeof(SafeBuffer));
            safeBuf->bytes = length;
            safeBuf->buffer_free = SafeBufferFree;
            CRTL_memcpy(safeBuf->pBuf,url,length);
            newNormalEvent(AMS_MODULE_RAMS, AMS_FASM_STATE_GET_OTA, (void *)safeBuf, Ams_handleAmsEvent, &newEvt);
            ES_pushEvent(&newEvt);
        }
        break;

        case ATYPE_AAMS:
        break;

        case ATYPE_SAMS:
        break;

        case ATYPE_NAMS:
        default:break;
    }
    Ams_setCurCrtlModule(type);
    return 0;
}

int Ams_destoryApp(int id,AMS_TYPE_E type,AmsCrtlCBFunc func)
{
    uint8_t idBuf[4] = {0x0,};
    uint8_t *pByte;
    SafeBuffer  *safeBuf;
    Event newEvt;
    switch(type)
    {
        case ATYPE_RAMS:
        {
            pByte = (uint8_t*)idBuf;
            writebeIU32(&pByte[0], id);
            safeBuf = CreateSafeBufferByBin(idBuf, sizeof(idBuf));
            newNormalEvent(AMS_MODULE_RAMS, AMS_FASM_STATE_GET_DESTROY, (void *)safeBuf, Ams_handleAmsEvent, &newEvt);
            ES_pushEvent(&newEvt);
        }
        break;

        case ATYPE_AAMS:
        break;

        case ATYPE_SAMS:
        break;

        case ATYPE_NAMS:
        default:break;
    }
    Ams_setCurCrtlModule(type);
    return 0;
}

//handle event process

int32_t Ams_lifecycleProcess(Event *evt, void *userData)
{
    int32_t evtId = UNMARK_EVT_ID(evt->evtId);
    Event   newEvt;
    switch (evtId)
    {
    case EVT_SYS_INIT:
        file_startup();
        vm_getCurApplist(TRUE );
        break;

    case EVT_SYS_EXIT:
        vm_clearApplist();
        //file_shutdown();
        break;

    default:
        break;
    }
    return EVT_RES_SUCCESS;
}

void Ams_handleAck(int module,int cmd,void * data)
{
    Event newEvt;
    newNormalEvent(module, cmd, data, Ams_handleAmsEvent, &newEvt);
    ES_pushEvent(&newEvt);
}

int32_t Ams_handleAmsEvent(Event *evt, void *userData)
{
    int32_t ams_type = UNMARK_EVT_ID(evt->evtId);

    switch(ams_type)
    {
        case AMS_MODULE_NAMS:
            return Ams_handleNativeAmsEvent(evt,userData);
            break;

        case AMS_MODULE_RAMS:
            return Ams_handleRemoteAmsEvent(evt,userData);
            break;

        case AMS_MODULE_AAMS:
            return Ams_handleAtAmsEvent(evt,userData);
            break;

        case AMS_MODULE_SAMS:
            return Ams_handleSmsAmsEvent(evt,userData);
            break;
    }
}

int32_t Ams_handleRemoteAmsEvent(Event *evt, void *userData)
{
    int32_t     appId;
    SafeBuffer *data;
    int32_t fsm_state = FSM_UNMARK(evt->fsm_state);
    Event newEvt;
    bool_t res;
	uint8_t * url =NULL;
    switch (fsm_state)
    {
        case AMS_FASM_STATE_GET_LIST:
            newNormalEvent(AMS_MODULE_RAMS, AMS_FASM_STATE_ACK_LIST, NULL, Ams_handleAmsEvent, &newEvt);
            ES_pushEvent(&newEvt);
            break;

        case AMS_FASM_STATE_ACK_LIST:
            ams_remote_list();
            break;

        case AMS_FASM_STATE_GET_RUN:
            if (userData != NULL)
            {
                data = (SafeBuffer *)userData;
            }
            else
            {
                data = (SafeBuffer *)evt->userData;
            }
            appId = readbeIU32(data->pBuf);
            res = vm_runApp(appId);
            *((int32_t*)(data->pBuf)) = (int32_t) res;
            newNormalEvent(AMS_MODULE_RAMS, AMS_FASM_STATE_ACK_RUN, userData, Ams_handleAmsEvent, &newEvt);
            ES_pushEvent(&newEvt);
            break;

        case AMS_FASM_STATE_ACK_RUN:
            if (userData != NULL)
            {
                data = (SafeBuffer *)userData;
            }
            else
            {
                data = (SafeBuffer *)evt->userData;
            }
            res = *((int32_t*)(data->pBuf));
            data->buffer_free(data);
            ams_remote_sendBackExecResult(EVT_CMD_RUN,(bool_t)res);
            break;

        case AMS_FASM_STATE_GET_DELETE:
            if (userData != NULL)
            {
                data = (SafeBuffer *)userData;
            }
            else
            {
                data = (SafeBuffer *)evt->userData;
            }
            appId = readbeIU32(data->pBuf);
            //appId = *((int32_t*)(data->pBuf));
            res = vm_deleteApp(appId);
            *((int32_t*)(data->pBuf)) = (int32_t) res;
            //data->buffer_free(data);
            newNormalEvent(AMS_MODULE_RAMS, AMS_FASM_STATE_ACK_DELETE, userData, Ams_handleAmsEvent, &newEvt);
            ES_pushEvent(&newEvt);
            break;

        case AMS_FASM_STATE_ACK_DELETE:
            if (userData != NULL)
            {
                data = (SafeBuffer *)userData;
            }
            else
            {
                data = (SafeBuffer *)evt->userData;
            }
            res = *((int32_t*)(data->pBuf));
            data->buffer_free(data);
            ams_remote_sendBackExecResult(EVT_CMD_DELETE,(bool_t) res);
            break;

        case AMS_FASM_STATE_GET_DESTROY:
            if (userData != NULL)
            {
                data = (SafeBuffer *)userData;
            }
            else
            {
                data = (SafeBuffer *)evt->userData;
            }
            appId = readbeIU32(data->pBuf);
            vm_destroyApp(appId);
            data->buffer_free(data);
            break;

        case AMS_FASM_STATE_ACK_DESTROY:
            if (userData != NULL)
            {
                data = (SafeBuffer *)userData;
            }
            else
            {
                data = (SafeBuffer *)evt->userData;
            }
            res = *((int32_t*)(data->pBuf));
            data->buffer_free(data);
            ams_remote_sendBackExecResult(EVT_CMD_DESTROY,res);
            break;

        case AMS_FASM_STATE_GET_OTA:        	
        	if (userData != NULL)
            {
                data = (SafeBuffer *)userData;
            }
            else
            {
                data = (SafeBuffer *)evt->userData;
            }
            url = (uint8_t *)data->pBuf;
        	res = vm_otaApp(url);
        	data->buffer_free(data);
            break;

        case AMS_FASM_STATE_ACK_OTA:
        	if (userData != NULL)
            {
                data = (SafeBuffer *)userData;
            }
            else
            {
                data = (SafeBuffer *)evt->userData;
            }
            url = (uint8_t *)data->pBuf;
            res = *((int32_t*)(data->pBuf));
            data->buffer_free(data);
            ams_remote_sendOTAExeResult(res);
            break;
    }
    DVM_LOG("===Ams_handleRemoteAmsEvent:not support for now\n");
    return 0;
}

int32_t Ams_handleAtAmsEvent(Event *evt, void *userData)
{
    DVM_LOG("===Ams_handleAtAmsEvent:not support for now\n");
    return 0;
}

int32_t Ams_handleSmsAmsEvent(Event *evt, void *userData)
{
    DVM_LOG("===Ams_handleSmsAmsEvent:not support for now\n");
    return 0;
}

int32_t Ams_handleNativeAmsEvent(Event *evt, void *userData)
{
    DVM_LOG("===Ams_handleNativeAmsEvent:not support for now\n");
    return 0;
}

int32_t VMThreadProc(int argc, char* argv[])
{
    DVMTraceInf("Enter dvm thread,argc=%d,argv=0x%x\n",argc,(void*)argv);
    DVMTraceInf("argv-0:%s,argv-1:%s,argv-2:%s\n",argv[0],argv[1],argv[2]);
    DVMTraceInf("Enter dvm thread,sleep over,sizeof(int)=%d,sizeof(int32_t)=%d\n",sizeof(int),sizeof(int32_t));
    DVM_main(argc, argv);
    return 0;
}


#ifdef ARCH_X86
static DWORD WINAPI VMThreadFunc(PVOID pvParam)
{
    uint32_t* params = (uint32_t*)pvParam;
    DVMThreadFunc vmProc = (DVMThreadFunc)params[0];
    int32_t argc = (int32_t)params[1];
    char** argv = (char**)params[2];

    vmProc(argc, argv);
    return 0;
}
#elif defined(ARCH_ARM_SPD)
uint32_t g_dthing_threadid = SCI_INVALID_BLOCK_ID;
uint8*   g_dthing_mem_space_ptr = NULL;

static void VMThreadFunc(uint32_t argc,void * argv)
{
    DVMTraceInf("===Enter dvm thread,argc=%d,argv=0x%x\n",argc,(void*)argv);
    DVMTraceInf("===argv-0:%s,argv-1:%s,argv-2:%s\n",((char*) argv)[0],((char*) argv)[1],((char*) argv)[2]);
    DVMTraceInf("===Enter dvm thread,sleep over,sizeof(int)=%d,sizeof(int32_t)=%d\n",sizeof(int),sizeof(int32_t));
    DVM_main(argc, argv);
}
#endif

int32_t Ams_createVMThread(DVMThreadFunc pDvmThreadProc, int argc, void* argv[])
{
#ifdef ARCH_X86
    HANDLE handle;
    static uint32_t params[3] = {0x0,};//static variable is better, any side effect?
    params[0] = (uint32_t)pDvmThreadProc;
    params[1] = argc;
    params[2] = (uint32_t)argv;

    handle = CreateThread(NULL, 0, VMThreadFunc, (LPVOID)params, 0, 0);
    if (handle > 0)
        return (int32_t)handle;

#elif defined (ARCH_ARM_SPD)

    #define DTHING_VM_THREAD_STACK_SIZE     (16*1024)
    #define DTHING_VM_THREAD_QUEUE_SIZE     (40*sizeof(uint32)*SCI_QUEUE_ITEM_SIZE)
    DVMTraceInf("===Ready to launch dthing vm thread!\n");
    //try to alloc java heap
//    cleanVmThread();
    g_dthing_mem_space_ptr = (uint8*)SCI_ALLOCA(DTHING_VM_THREAD_STACK_SIZE +DTHING_VM_THREAD_QUEUE_SIZE);

    DVMTraceInf("===Alloc g_dthing_mem_space_ptr = 0x%x\n",g_dthing_mem_space_ptr);
    if(PNULL == g_dthing_mem_space_ptr)
    {
        DVMTraceErr("===Error:alloc dthing vm memory fail!\n");
        return -1;
    }
#if 0
    g_dthing_threadid = SCI_CreateAppThread(
                    "DthingVmTask",
                    "DthingVmQueue",
                    VMThreadFunc,
                    argc,
                    (void *)argv,
                    DTHING_VM_THREAD_STACK_SIZE,
                    20,
                    30,//PRI_DTHING_TASK,
                    SCI_PREEMPT,
                    SCI_AUTO_START
                    );
#endif
#if 1
    g_dthing_threadid = SCI_CreateAppThreadEx
                (
                    "DthingVmTask",
                    VMThreadFunc,
                    argc,
                    (void*)argv,
                    g_dthing_mem_space_ptr,
                    DTHING_VM_THREAD_STACK_SIZE,
                    PRI_DTHING_TASK,//30,//PRI_DTHING_TASK,
                    SCI_PREEMPT,
                    "DthingVmQueue",
                    (g_dthing_mem_space_ptr+DTHING_VM_THREAD_STACK_SIZE),
                    DTHING_VM_THREAD_QUEUE_SIZE,
                    SCI_AUTO_START
                );
#endif
    DVMTraceInf("===Create dthing vm thread success! thread id = %d \n",g_dthing_threadid);
    return g_dthing_threadid;
#endif
    return -1;
}

/**
 * AMS Remote Control API
 */
int Ams_handleRemoteCmdSync(int cmdId, AMS_TYPE_E cmdType, int suiteId, char *data, char** ppout)
{
    int result = -1;

    *ppout = NULL;

    switch(cmdId)
    {
    case RCMD_CFGURL:
        //TODO:
        break;
    case RCMD_CFGACCOUNT:
        //TODO:
        break;
    case RCMD_INIT:
        break;
    case RCMD_LIST:
        //TODO:
        break;
    case RCMD_OTA:
        if (NULL != data)
        {
            result = Ams_otaApp(data, cmdType, NULL);
        }
        break;
    case RCMD_DELETE:
        result = Ams_deleteApp(suiteId, cmdType, NULL);
        break;
    case RCMD_DELETEALL:
        //TODO:
        break;
    case RCMD_RUN:
        result = Ams_runApp(suiteId, cmdType, NULL);
        break;
    case RCMD_DESTROY:
        result = Ams_destoryApp(suiteId, cmdType, NULL);
        break;
    case RCMD_STATUS:
        //TODO:
        break;
    default:
        break;
    }
    return result;
}