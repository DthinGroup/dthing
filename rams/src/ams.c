#include "std_global.h"
#include "vm_common.h"
#include "eventbase.h"
#include "opl_file.h"
#include "vm_app.h"
#include "ams.h"
#include "ams_remote.h"
#include "ams_sms.h"
#include "ams_at.h"
#include "ams_utils.h"

#ifdef ARCH_X86
    #pragma comment(lib, "Winmm.lib")
    #include <Windows.h>
    #include <Mmsystem.h>
#else
    #include "sci_types.h"
    #include "os_api.h"
    #include "priority_app.h"
#endif

#ifdef DVM_LOG
#undef DVM_LOG
#endif
#define DVM_LOG DVMTraceDbg
#define MAX_PATH_LENGTH   255
#define RCMD_CANCELALL_CFG "rcmd_cancelall_cfg" 

static AmsCrtlCBFunc amsCrtlCBFunc[ATYPE_MAX + 1] ;
//default is Native Ams
static int32_t s_cur_ams_crtl_platform = AMS_MODULE_NAMS;


static int32_t Ams_handleAmsEvent(Event *evt, void *userData);
/*----------------------tool funcs-------------------------*/
void Ams_init()
{
    AMS_TYPE_E i;
    for(i=0;i<=ATYPE_MAX;i++)
    {
        amsCrtlCBFunc[i] = NULL;
    }
}

void Ams_final()
{
    AMS_TYPE_E i;
    for(i=0;i<=ATYPE_MAX;i++)
    {
        amsCrtlCBFunc[i] = NULL;
    }
}

bool_t Ams_regModuleCallBackHandler(AMS_TYPE_E type, AmsCrtlCBFunc modFunc)
{
    if(type > ATYPE_MAX || type < ATYPE_MIN)
    {
        DVM_LOG("===AmsCrtlCBFunc type is invalid==\d");
        return FALSE;
    }
    if(modFunc ==NULL)
    {
        DVM_LOG("===AmsCrtlCBFunc can not be null==\d");
        return FALSE;
    }

    amsCrtlCBFunc[type] = modFunc;
    return TRUE;
}
bool_t Ams_unregModuleCallBackHandler(AMS_TYPE_E type)
{
    amsCrtlCBFunc[type] = NULL;
    return TRUE;
}
void Ams_setCurCrtlModule(AMS_TYPE_E type)
{
    DVM_LOG("===Ams_setCurCrtlPlatform:%d\n",type);
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
    DVM_LOG("==invalid ams type, to assert===\n");
    DVM_ASSERT(0);
}
AMS_TYPE_E Ams_getATypeByModule(int module)
{
    switch(module)
    {
        case AMS_MODULE_NAMS: return ATYPE_NAMS ;
        case AMS_MODULE_RAMS: return ATYPE_RAMS ;
        case AMS_MODULE_AAMS: return ATYPE_AAMS ;
        case AMS_MODULE_SAMS: return ATYPE_SAMS ;
    }
    DVM_LOG("==invalid ams module, to assert===\n");
    DVM_ASSERT(0);
}

void Ams_listApp(AMS_TYPE_E type)
{
    Event newEvt;
    switch(type)
    {
        case ATYPE_RAMS:
        case ATYPE_SAMS:
        case ATYPE_AAMS:
            newNormalEvent(AMS_MODULE_RAMS, AMS_FASM_STATE_GET_LIST, NULL, Ams_handleAmsEvent, &newEvt);
            ES_pushEvent(&newEvt);
            break;

        case ATYPE_NAMS:
            default:break;
    }

    Ams_setCurCrtlModule(type);
}

void Ams_auth(AMS_TYPE_E type)
{
    Event newEvt;
    switch(type)
    {
        case ATYPE_RAMS:
        case ATYPE_SAMS:
        case ATYPE_AAMS:
            newNormalEvent(AMS_MODULE_RAMS, AMS_FASM_STATE_GET_AUTH, NULL, Ams_handleAmsEvent, &newEvt);
            ES_pushEvent(&newEvt);
            break;

        case ATYPE_NAMS:
            default:break;
    }

    Ams_setCurCrtlModule(type);
}


int Ams_runApp(int id,AMS_TYPE_E type)
{
    uint8_t idBuf[4] = {0x0,};
    uint8_t *pByte;
    SafeBuffer  *safeBuf;
    Event newEvt;
    switch(type)
    {
        case ATYPE_RAMS:
        case ATYPE_SAMS:
        case ATYPE_AAMS:
        {
            pByte = (uint8_t*)idBuf;
            writebeIU32(&pByte[0], id);
            safeBuf = CreateSafeBufferByBin(idBuf, sizeof(idBuf));
            newNormalEvent(AMS_MODULE_RAMS, AMS_FASM_STATE_GET_RUN, (void *)safeBuf, Ams_handleAmsEvent, &newEvt);
            ES_pushEvent(&newEvt);
        }
        break;

        case ATYPE_NAMS:
        default:break;
    }
    Ams_setCurCrtlModule(type);
    return 0;
}

int Ams_deleteApp(int id,AMS_TYPE_E type)
{
    uint8_t idBuf[4] = {0x0,};
    uint8_t *pByte;
    SafeBuffer  *safeBuf;
    Event newEvt;
    switch(type)
    {
        case ATYPE_RAMS:
        case ATYPE_SAMS:
        case ATYPE_AAMS:
        {
            pByte = (uint8_t*)idBuf;
            writebeIU32(&pByte[0], id);
            safeBuf = CreateSafeBufferByBin(idBuf, sizeof(idBuf));
            newNormalEvent(AMS_MODULE_RAMS, AMS_FASM_STATE_GET_DELETE, (void *)safeBuf, Ams_handleAmsEvent, &newEvt);
            ES_pushEvent(&newEvt);
        }
        break;

        case ATYPE_NAMS:
        default:break;
    }
    Ams_setCurCrtlModule(type);
    return 0;
}

int Ams_deleteAllApp(int id,AMS_TYPE_E type){
	AppletProps *pap;
	int i = 0;
	pap = vm_getCurApplist(TRUE);
	while(pap != NULL){   
	    Ams_deleteApp(pap->id,type);
	    pap = pap->nextRunning;
        i++;
	}
    if(i > 0){
	    return 1;
    }else{
        return 0;
    }

}

int Ams_initApp(int id,AMS_TYPE_E type)
{
    uint8_t idBuf[4] = {0x0,};
    uint8_t *pByte;
    SafeBuffer  *safeBuf;
    Event newEvt;
    switch(type)
    {
        case ATYPE_RAMS:
        case ATYPE_SAMS:
        case ATYPE_AAMS:
        {
            pByte = (uint8_t*)idBuf;
            writebeIU32(&pByte[0], id);
            safeBuf = CreateSafeBufferByBin(idBuf, sizeof(idBuf));
            newNormalEvent(AMS_MODULE_RAMS, AMS_FASM_STATE_GET_INIT, (void *)safeBuf, Ams_handleAmsEvent, &newEvt);
            ES_pushEvent(&newEvt);
        }
        break;

        case ATYPE_NAMS:
        default:break;
    }
    Ams_setCurCrtlModule(type);
    return 0;
}

int Ams_cancelInitApp(int id,AMS_TYPE_E type)
{
    uint8_t idBuf[4] = {0x0,};
    uint8_t *pByte;
    SafeBuffer  *safeBuf;
    Event newEvt;
    switch(type)
    {
        case ATYPE_RAMS:
        case ATYPE_SAMS:
        case ATYPE_AAMS:
        {
            pByte = (uint8_t*)idBuf;
            writebeIU32(&pByte[0], id);
            safeBuf = CreateSafeBufferByBin(idBuf, sizeof(idBuf));
            newNormalEvent(AMS_MODULE_RAMS, AMS_FASM_STATE_GET_CANCEL, (void *)safeBuf, Ams_handleAmsEvent, &newEvt);
            ES_pushEvent(&newEvt);
        }
        break;

        case ATYPE_NAMS:
        default:break;
    }
    Ams_setCurCrtlModule(type);
    return 0;
}

void Ams_cancelAllApp(AMS_TYPE_E type)
{
    uint8_t idBuf[4] = {0x0,};
    uint8_t *pByte;
    SafeBuffer  *safeBuf;
    Event newEvt;
    switch(type)
    {
        case ATYPE_RAMS:
        case ATYPE_SAMS:
        case ATYPE_AAMS:
        {
            pByte = (uint8_t*)idBuf;
            writebeIU32(&pByte[0], 0);
            safeBuf = CreateSafeBufferByBin(idBuf, sizeof(idBuf));
            newNormalEvent(AMS_MODULE_RAMS, AMS_FASM_STATE_GET_CANCEL, (void *)safeBuf, Ams_handleAmsEvent, &newEvt);
            ES_pushEvent(&newEvt);
        }
        break;

        case ATYPE_NAMS:
        default:
        break;
    }
    Ams_setCurCrtlModule(type);
}


int Ams_otaApp(uint8_t* url,AMS_TYPE_E type)
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
        case ATYPE_SAMS:
        case ATYPE_AAMS:
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

        case ATYPE_NAMS:
        default:break;
    }
    Ams_setCurCrtlModule(type);
    return 0;
}

int Ams_tckApp(uint8_t* url, AMS_TYPE_E type)
{
    uint8_t idBuf[4] = { 0x0, };
    uint8_t *pByte;
    SafeBuffer  *safeBuf;
    int32_t safeBufSize;
    int32_t length;
    Event newEvt;
    switch (type)
    {
        case ATYPE_RAMS:
        case ATYPE_SAMS:
        case ATYPE_AAMS:
            {
                length = CRTL_strlen(url);
                safeBufSize = sizeof(SafeBuffer) + length + 1;
                safeBuf = (SafeBuffer *)CRTL_malloc(safeBufSize);
                if (safeBuf == NULL)
                {
                    DVMTraceErr("Ams_tckApp ATYPE_RAMS: alloc fail\n");
                    break;
                }
                CRTL_memset(safeBuf, 0x0, safeBufSize);

                safeBuf->pBuf = ((uint8_t*)(safeBuf)+sizeof(SafeBuffer));
                safeBuf->bytes = length;
                safeBuf->buffer_free = SafeBufferFree;
                CRTL_memcpy(safeBuf->pBuf, url, length);
                newNormalEvent(AMS_MODULE_RAMS, AMS_FASM_STATE_GET_TCK, (void *)safeBuf, Ams_handleAmsEvent, &newEvt);
                ES_pushEvent(&newEvt);
            }
            break;

        case ATYPE_NAMS:
        default:break;
    }
    Ams_setCurCrtlModule(type);
    return 0;
}

int Ams_destoryApp(int id,AMS_TYPE_E type)
{
    uint8_t idBuf[4] = {0x0,};
    uint8_t *pByte;
    SafeBuffer  *safeBuf;
    Event newEvt;
    switch(type)
    {
        case ATYPE_RAMS:
        case ATYPE_SAMS:
        case ATYPE_AAMS:
        {
            pByte = (uint8_t*)idBuf;
            writebeIU32(&pByte[0], id);
            safeBuf = CreateSafeBufferByBin(idBuf, sizeof(idBuf));
            newNormalEvent(AMS_MODULE_RAMS, AMS_FASM_STATE_GET_DESTROY, (void *)safeBuf, Ams_handleAmsEvent, &newEvt);
            ES_pushEvent(&newEvt);
        }
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
        case AMS_MODULE_RAMS:
        case AMS_MODULE_AAMS:
        case AMS_MODULE_SAMS:
            Ams_handleAllAmsEvent(evt,userData);
            break;

        case AMS_MODULE_NAMS:
            //un support now!
            break;
    }
}

int32_t Ams_handleAllAmsEvent(Event *evt, void *userData)
{
    int32_t     appId;
    char buff[16]={0};
    SafeBuffer *data;
    int32_t fsm_state = FSM_UNMARK(evt->fsm_state);
    Event newEvt;
    bool_t res;
    uint8_t * url =NULL;
    AppletProps *curApp = NULL;
//  AmsCrtlCBFunc cbFunc = amsCrtlCBFunc[Ams_getATypeByModule(UNMARK_EVT_ID(evt->evtId))];
    AmsCrtlCBFunc cbFunc = amsCrtlCBFunc[Ams_getATypeByModule(Ams_getCurCrtlModule())];
    int i = 0;

    AmsCBData amsCbData;
    DVM_LOG("===Ams_handleRemoteAmsEvent: state:%d\n",fsm_state);
    switch (fsm_state)
    {
        case AMS_FASM_STATE_GET_LIST:
            newNormalEvent(AMS_MODULE_RAMS, AMS_FASM_STATE_ACK_LIST, NULL, Ams_handleAmsEvent, &newEvt);
            ES_pushEvent(&newEvt);
            break;

        case AMS_FASM_STATE_ACK_LIST:
            //ams_remote_list();
            if(cbFunc !=NULL)
            {
                amsCbData.cmd = RCMD_LIST;
                amsCbData.module = Ams_getCurCrtlModule();
                amsCbData.result = 1;
                amsCbData.exptr = NULL;
                cbFunc(&amsCbData);
            }
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
#if 1   //report in Java_com_yarlungsoft_ams_Scheduler_reportState
            *((int32_t*)(data->pBuf)) = (int32_t) res;
            newNormalEvent(AMS_MODULE_RAMS, AMS_FASM_STATE_ACK_RUN, userData, Ams_handleAmsEvent, &newEvt);
            ES_pushEvent(&newEvt);
#else
            data->buffer_free(data);
#endif
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
            if(cbFunc !=NULL)
            {
                amsCbData.cmd = RCMD_RUN;
                amsCbData.module =  Ams_getCurCrtlModule();
                amsCbData.result = res;	
                amsCbData.exptr = NULL;
                cbFunc(&amsCbData);
            }
            //ams_remote_sendBackExecResult(EVT_CMD_RUN,(bool_t)res);
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
            if(cbFunc !=NULL)
            {
                amsCbData.cmd = RCMD_DELETE;
                amsCbData.module = Ams_getCurCrtlModule();
                amsCbData.result = res;
                amsCbData.exptr = NULL;
                cbFunc(&amsCbData);
            }
            //ams_remote_sendBackExecResult(EVT_CMD_DELETE,(bool_t) res);
            break;

        case AMS_FASM_STATE_GET_INIT:
            if (userData != NULL)
            {
                data = (SafeBuffer *)userData;
            }
            else
            {
                data = (SafeBuffer *)evt->userData;
            }
            appId = readbeIU32(data->pBuf);
            CRTL_memset(buff, 0x0, 16);
            sprintf(buff,"%d",appId);
            res = amsUtils_initConfigData(buff);
            *((int32_t*)(data->pBuf)) = (int32_t) res;
            newNormalEvent(AMS_MODULE_RAMS, AMS_FASM_STATE_ACK_INIT, userData, Ams_handleAmsEvent, &newEvt);
            ES_pushEvent(&newEvt);
            break;

        case AMS_FASM_STATE_ACK_INIT:
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
            if(cbFunc !=NULL)
            {
                amsCbData.cmd = RCMD_INIT;
                amsCbData.module = Ams_getCurCrtlModule();
                amsCbData.result = res;
                amsCbData.exptr = NULL;
                cbFunc(&amsCbData);
            }
            break;

        case AMS_FASM_STATE_GET_CANCEL:
            if (userData != NULL)
            {
                data = (SafeBuffer *)userData;
            }
            else
            {
                data = (SafeBuffer *)evt->userData;
            }
            appId = readbeIU32(data->pBuf);
            CRTL_memset(buff, 0x0, 16);
            sprintf(buff,"%d",appId);	
            res = amsUtils_cancelDefaultApp(appId);
            *((int32_t*)(data->pBuf)) = (int32_t) res;
            newNormalEvent(AMS_MODULE_RAMS, AMS_FASM_STATE_ACK_CANCEL, userData, Ams_handleAmsEvent, &newEvt);
            ES_pushEvent(&newEvt);
            break;

        case AMS_FASM_STATE_ACK_CANCEL:
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
            if(cbFunc !=NULL)
            {
                amsCbData.cmd = RCMD_CANCEL;
                amsCbData.module = Ams_getCurCrtlModule();
                amsCbData.result = res;
                amsCbData.exptr = NULL;
                cbFunc(&amsCbData);
            }
            break;

        case AMS_FASM_STATE_GET_CANCELALL:
            if (userData != NULL)
            {
                data = (SafeBuffer *)userData;
            }
            else
            {
                data = (SafeBuffer *)evt->userData;
            }
            res = amsUtils_initConfigData(NULL);
            *((int32_t*)(data->pBuf)) = (int32_t) res;
            newNormalEvent(AMS_MODULE_RAMS, AMS_FASM_STATE_ACK_CANCELALL, userData, Ams_handleAmsEvent, &newEvt);
            ES_pushEvent(&newEvt);
            break;

        case AMS_FASM_STATE_ACK_CANCELALL:
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
            if(cbFunc !=NULL)
            {
                amsCbData.cmd = RCMD_CANCELALL;
                amsCbData.module = Ams_getCurCrtlModule();
                amsCbData.result = res;
                amsCbData.exptr = NULL;
                cbFunc(&amsCbData);
            }
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
            if(cbFunc !=NULL)
            {
                amsCbData.cmd = RCMD_DESTROY;
                amsCbData.module = Ams_getCurCrtlModule();
                amsCbData.result = res;
                amsCbData.exptr = NULL;
                cbFunc(&amsCbData);
            }
            //ams_remote_sendBackExecResult(EVT_CMD_DESTROY,res);
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
            if(res){
                 vm_getCurApplist(TRUE);
            }
            if(cbFunc !=NULL)
            {
                amsCbData.cmd = RCMD_OTA;
                amsCbData.module = Ams_getCurCrtlModule();
                amsCbData.result = (res == 0 ? 1: 0);
                amsCbData.exptr = NULL;
                cbFunc(&amsCbData);
            }
           // ams_remote_sendOTAExeResult(res);
            break;

        case AMS_FASM_STATE_GET_TCK:
            if (userData != NULL)
            {
                data = (SafeBuffer *)userData;
            }
            else
            {
                data = (SafeBuffer *)evt->userData;
            }
            url = (uint8_t *)data->pBuf;
            res = vm_tckApp(url);
            data->buffer_free(data);
            break;

        case AMS_FASM_STATE_ACK_TCK:
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
            if (cbFunc != NULL)
            {
                amsCbData.cmd = RCMD_TCK;
                amsCbData.module = Ams_getCurCrtlModule();
                amsCbData.result = (res == 0 ? 1 : 0);
                amsCbData.exptr = NULL;
                cbFunc(&amsCbData);
            }
            // ams_remote_sendTCKExeResult(res);
            break;

		case AMS_FASM_STATE_GET_AUTH:
            newNormalEvent(AMS_MODULE_RAMS, AMS_FASM_STATE_ACK_AUTH, NULL, Ams_handleAmsEvent, &newEvt);
            ES_pushEvent(&newEvt);			
			break;
			
		case AMS_FASM_STATE_ACK_AUTH:
			if(cbFunc !=NULL)
            {
                amsCbData.cmd = RCMD_AUTH;
                amsCbData.module = Ams_getCurCrtlModule();
                amsCbData.result = 1;
                amsCbData.exptr = NULL;
                cbFunc(&amsCbData);
            }
			break;

    }

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
uint8_t*   g_dthing_mem_space_ptr = NULL;

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

    #define DTHING_VM_THREAD_STACK_SIZE     (8*1024)
    #define DTHING_VM_THREAD_QUEUE_SIZE     (40*sizeof(uint32_t)*SCI_QUEUE_ITEM_SIZE)
    DVMTraceInf("===Ready to launch dthing vm thread!\n");
    //try to alloc java heap
//    cleanVmThread();
    g_dthing_mem_space_ptr = (uint8_t*)SCI_ALLOCA(DTHING_VM_THREAD_STACK_SIZE +DTHING_VM_THREAD_QUEUE_SIZE);

    DVMTraceInf("===Alloc g_dthing_mem_space_ptr = 0x%x\n",g_dthing_mem_space_ptr);
    if(NULL == g_dthing_mem_space_ptr)
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
                    SCI_DONT_START
                );
    if(PNULL != g_dthing_threadid)
    {
        SCI_ResumeThread(g_dthing_threadid);
    }
#endif
    DVMTraceInf("===Create dthing vm thread success! thread id = %d \n",g_dthing_threadid);
    return g_dthing_threadid;
#endif
    return -1;
}

/**
 * AMS Remote Control API
 */

/**
 * @brief request ams with remote command
 * @params cmdId int defined in <RemoteCommandType>
 * @params cmdType AMS_TYPE_E
 * @params suiteId int suiteId if exist or -1 when not specified
 * @params data char*
 * @params[OUT] ppout char** pointer to output result or NULL when no output
 *
 * @return int result for request, negative for failed, otherwise success
 */
int Ams_handleRemoteCmdSync(int cmdId, AMS_TYPE_E cmdType, int suiteId, char *data, char** ppout)
{
    int result = -1;

    if (ppout)
    {
      *ppout = NULL;
    }

    switch(cmdId)
    {
    case RCMD_CFGURL:
        if (amsUtils_configAddress(data))
        {
            result = 0;
        }
        break;
    case RCMD_CFGACCOUNT:
        if (amsUtils_configAccount(data))
        {
            result = 0;
        }
        break;
    case RCMD_INIT:
        if (amsUtils_initConfigData(data))
        {
            result = 0;
        }
        break;
    case RCMD_CANCEL:		
        if (amsUtils_cancelDefaultApp(suiteId))
        {
          result = 0;
        }
        break;
    case RCMD_CANCELALL:
        if (amsUtils_initConfigData(RCMD_CANCELALL_CFG  ))
        {
          result = 0;
        }
        break;
    case RCMD_LIST:
        if (ppout)
        {
            *ppout = amsUtils_getAppletList(FALSE);      
             result = 0;
        }
        break;
    case RCMD_OTA:
        if (NULL != data)
        {
            result = Ams_otaApp(data, cmdType);
        }
        break;
    case RCMD_TCK:
        if (NULL != data)
        {
            result = Ams_tckApp(data, cmdType);
        }
        break;
    case RCMD_DELETE:
        result = Ams_deleteApp(suiteId, cmdType);
        break;
    case RCMD_DELETEALL:
        result = Ams_deleteAllApp(suiteId, cmdType);
        break;
    case RCMD_RUN:
        result = Ams_runApp(suiteId, cmdType);
        break;
    case RCMD_DESTROY:
        result = Ams_destoryApp(suiteId, cmdType);
        break;
    case RCMD_STATUS:
        if (ppout)
        {
          *ppout = amsUtils_getAppletList(TRUE);
          result = 0;
        }
        break;
   	case RCMD_RESET:
        {
        //FIXME: Not the best way to reset power
        int i = 0;
        result = 0;
        while(true){
            char * temp ;
            *temp = malloc(1024 * 1024 * 1);
            }
        }
        break;
		
    default:
        DVMTraceDbg("=== Unknown RemoteCmd %d\n", cmdId);
        break;
    }
    return result;
}

