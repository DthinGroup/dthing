/*=========================================================================*/
/*======= INCLUDES ========================================================*/
/*=========================================================================*/

/* Networking porting module prototypes */
#include <vm_common.h>
#include <ams_remote.h>
#include <ams_sms.h>
#include <ams.h>
#include <ams_utils.h>
#if defined(ARCH_ARM_SPD)
#include <dal_time.h>
#include <Sig_code.h>
#include <os_api.h>
#include <priority_app.h>
#include <sci_service.h>
#include <mn_api.h>
#include <mn_error.h>
#include <mn_type.h>
#include <tcpip_types.h>
#include <socket_api.h>
#include <socket_types.h>
#include <Mn_events.h>
#include <Mn_type.h>
#include <sfs.h>
/*=========================================================================*/
/*======= PRIVATE DEFINITIONS =============================================*/
/*=========================================================================*/

#ifdef SMSCLOG
#undef SMSCLOG
#endif

#define SMSC_LOG

#if defined(SMSC_LOG)
#define SMSCLOG SCI_TRACE_LOW
#else
#define SMSCLOG
#endif

/*=========================================================================*/
/*======= MACRO DEFINE ====================================================*/
/*=========================================================================*/
#define SMS_CMD_BUFF_LEN 256 //the cmd is very long
#define DATE_MIN_YEAR  1980 
#define SMS_TEXT_BUFF_LEN       (MN_SMS_MAX_USER_VALID_DATA_LENGTH*4+1)
#define  MAX_SMS_ADDRESS_LEN_FOR_TE   ((MN_MAX_ADDR_BCD_LEN*2)*4+1)
#define UNKNOWN_PLAN_UNKNOWN_TYPE 128
 // ISDN/telephony number plan, //unknow number
#define    ISDN_TELE_PLAN_UNKNOWN_TYPE      129
// ISDN/telephony number plan, // international number
#define    ISDN_TELE_PLAN_INTERNATIONAL_TYPE 145 
// ISDN/telephony number plan // national number
#define    ISDN_TELE_PLAN_NATIONAL_TYPE  161 
#define SMS_RECIEVER_NUM_MAX             20
#define SMS__MAX_UCS2_BYTE_LEN        134
#define SMS__MAX_UCS2_SPLIT_LEN        128

#define SMS__MAX_DEF_BYTE_LEN        154
#define SMS__MAX_DEF_SPLIT_LEN        147

#define SMS__MAX_8BIT_BYTE_LEN        134
#define SMS__MAX_8BIT_SPLIT_LEN        128

#define  ATC_CHSET_IRA  0
#define  ATC_CHSET_GSM 1  
#define  ATC_CHSET_HEX 2
#define  ATC_CHSET_UCS2 3  

#define RCMD_CANCELALL_CFG "rcmd_cancelall_cfg" 

#define PARTYNUM_2_MNCALLEDNUM( _MMI_PARTY_NUM, _MN_CALL_NUM_PTR )                            \
        {                                                                                        \
            _MN_CALL_NUM_PTR->number_type = _MMI_PARTY_NUM->number_type;                            \
            _MN_CALL_NUM_PTR->num_len      = MIN( _MMI_PARTY_NUM->num_len, MN_MAX_ADDR_BCD_LEN );    \
            SCI_MEMCPY(                                                                            \
                        (void*)_MN_CALL_NUM_PTR->party_num,                                        \
                        (void*)_MMI_PARTY_NUM->party_num ,                                            \
                        _MN_CALL_NUM_PTR->num_len                                                \
                        );                                                                        \
        }

/*=========================================================================*/
/*======= GLOBAL DATA =====================================================*/
/*=========================================================================*/
PUBLIC void Rmt_start(void);
PUBLIC uint32 getNetID(void);
PUBLIC int32 getSocketID(void);
/*=========================================================================*/
/*======= LOCAL DATA ======================================================*/
/*=========================================================================*/
static char sms_cmd[SMS_CMD_BUFF_LEN] = {0};
//extern  char server_address[];
//extern  char server_port[];
//extern  char init_data[];
static int sms_cmd_value = EVT_CMD_NONE;
MN_CALLED_NUMBER_T  m_reply_number;
/*=========================================================================*/
/*======= Declare ============================================================*/
/*=========================================================================*/

//extern void rmtc_writeConfigFile();
//extern void rmtc_parseInitData();
//extern void rmtc_resetInitData();
//extern void rmtc_cancelInitData(char* app);
extern BOOLEAN util_isNumber(const char* p);
//extern void RmtReConnect_Pro();
void smsc_ackSmsResultMsg(int cmd,int result);
void smsc_ackSmsResultListMsg(int cmd,int result,char* data,int datalen);
/*=========================================================================*/
/*======= CODE ============================================================*/
/*=========================================================================*/

BOOLEAN util_isNumber(const char* p)
{
	while(*p)
	{
		if(*p<48 || *p>57) return FALSE;
		*p++;
	}

	return TRUE;
}

void smsc_callBack(AmsCBData* data)
{
    SMSCLOG("==SMST== smsc_callBack cmd=%d result=%d buff=%s",data->cmd,data->result,data->exptr);    
    if(data->cmd == 6)
    {
       // if(sms_cmd_value == EVT_CMD_LIST)
        //{
       //     smsc_ackSmsResultListMsg(data->cmd,data->result,data->exptr,strlen(data->exptr));
       // }
       // else
       // { 
             if(data->result == 0){
                    smsc_ackSmsResultMsg(EVT_CMD_OTA,CMD_RESULT_OK);
             }else{
                    smsc_ackSmsResultMsg(EVT_CMD_OTA,CMD_RESULT_FAILED);
             }
             
            //smsc_ackSmsResultMsg(data->cmd,data->result);    
      //  }
    }
}

void smsc_ReceiveRemoteCmd(int cmd_id,int suite_id,char* pdata)
{
    SMSCLOG("==SMST== smsc_ReceiveRemoteCmd cmd=%d suite=%d buff=%s",cmd_id,suite_id,pdata);
    switch(cmd_id)
    {
        case EVT_CMD_DELETE:
            //Ams_deleteApp(suite_id,ATYPE_SAMS);
            if(vm_deleteApp(suite_id)){
                smsc_ackSmsResultMsg(EVT_CMD_DELETE,CMD_RESULT_OK);
            }else{
                smsc_ackSmsResultMsg(EVT_CMD_DELETE,CMD_RESULT_FAILED);
            }
        break;
        case EVT_CMD_RUN:
           // Ams_runApp(suite_id,ATYPE_SAMS);
	        if(vm_runApp(suite_id)){
			    smsc_ackSmsResultMsg(EVT_CMD_RUN,CMD_RESULT_OK);
		    }else{
			    smsc_ackSmsResultMsg(EVT_CMD_RUN,CMD_RESULT_FAILED);
		    }
        break;
        case EVT_CMD_LIST:
            Ams_listApp(ATYPE_SAMS);
        break;
        case EVT_CMD_OTA:
            //Ams_otaApp(pdata,ATYPE_SAMS);
            
            if(/*vm_otaApp(pdata)*/Ams_otaApp(pdata,ATYPE_SAMS) == 0){
		    smsc_ackSmsResultMsg(EVT_CMD_OTA,CMD_RESULT_DOWNLOAD);
	    }else{
		    smsc_ackSmsResultMsg(EVT_CMD_OTA,CMD_RESULT_FAILED);
	    }
        break;
        case EVT_CMD_DESTROY:
           // Ams_destoryApp(suite_id,ATYPE_SAMS);
            if(vm_destroyApp(suite_id)){
			    smsc_ackSmsResultMsg(EVT_CMD_DESTROY,CMD_RESULT_OK);
	        }else{
			    smsc_ackSmsResultMsg(EVT_CMD_DESTROY,CMD_RESULT_FAILED);
		    }
        break;
        case EVT_CMD_DELETEALL:
            if(Ams_deleteAllApp(0, ATYPE_SAMS)){
                smsc_ackSmsResultMsg(EVT_CMD_DELETEALL,CMD_RESULT_OK);
		    }else{
			    smsc_ackSmsResultMsg(EVT_CMD_DELETEALL,CMD_RESULT_FAILED);
		    }          
        break;
        case EVT_CMD_STATUS:
            smsc_ackSmsResultMsg(EVT_CMD_STATUS,CMD_RESULT_OK);
        break;
        case EVT_CMD_RESET:
            {
                char * temp = NULL;
                // smsc_ackSmsResultMsg(EVT_CMD_RESET,CMD_RESULT_OK);  
                DVMTraceErr("The fouction is not work now!");
                // SCI_Sleep(50000);
                 //is there other way to reset this dvm?             
                 //strlen(temp);
                break;
            }
        case EVT_CMD_INIT:   
            if(amsUtils_initConfigData(pdata)){
                smsc_ackSmsResultMsg(EVT_CMD_INIT,CMD_RESULT_OK);
		    }else{
			    smsc_ackSmsResultMsg(EVT_CMD_INIT,CMD_RESULT_FAILED);
		    }          
            break;
        case EVT_CMD_CANCEL:
            if( amsUtils_cancelDefaultApp(suite_id)){
                smsc_ackSmsResultMsg(EVT_CMD_CANCEL,CMD_RESULT_OK);
		    }else{
			    smsc_ackSmsResultMsg(EVT_CMD_CANCEL,CMD_RESULT_FAILED);
		    }  
            break;			
        case EVT_CMD_CANCELALL:
            if(amsUtils_initConfigData(RCMD_CANCELALL_CFG)){
                 smsc_ackSmsResultMsg(EVT_CMD_CANCELALL,CMD_RESULT_OK);
		    }else{
			     smsc_ackSmsResultMsg(EVT_CMD_CANCELALL,CMD_RESULT_FAILED);
		    }  
            break;
        case EVT_CMD_CFGURL:           
            if(amsUtils_configAddress(pdata)){
                smsc_ackSmsResultMsg(EVT_CMD_CFGURL,CMD_RESULT_OK);
		    }else{
			    smsc_ackSmsResultMsg(EVT_CMD_CFGURL,CMD_RESULT_FAILED);
		    }  
            break;
        case EVT_CMD_CFGACCOUNT:
            if(amsUtils_configAccount(pdata)){
                smsc_ackSmsResultMsg(EVT_CMD_CFGACCOUNT,CMD_RESULT_OK);
		    }else{
		        smsc_ackSmsResultMsg(EVT_CMD_CFGACCOUNT,CMD_RESULT_FAILED);
		    }  
            break;          
        default:
            smsc_ackSmsResultMsg(EVT_CMD_NONE,CMD_RESULT_OK);
    }
}


BOOLEAN  smsc_HandleReceivedSms(char* buff,int len)
{
    int index = 0 ;
    SMSCLOG("==SMST== smsc_HandleReceivedSms call start  buff=%s len = %d",buff,len); 
    //to check if it is SMS AMS message, due to the cmd shortest length is 9, so length should bigger than 8
    if(len<9)
    {	
        smsc_ackSmsResultMsg(EVT_CMD_NONE,CMD_RESULT_OK);
        return FALSE;
    }

    if(buff[0] !='S' || buff[1] !='M' || buff[2] !='S' || buff[3] !='A' || buff[4] !='M' || buff[5] !='S' )
    {     
        smsc_ackSmsResultMsg(EVT_CMD_NONE,CMD_RESULT_OK);
	    return FALSE;
    }
    SMSCLOG("==SMST== smsc_HandleReceivedSms 1");
    // due to SMSAMS length is 6,so we start from 6:
    index = 6;
    while(index<len)
    {
        SMSCLOG("==SMST== smsc_HandleReceivedSms 1.5 index=%d buff[index]=%c",index,buff[index]);
        if(buff[index]==32)
        {
            break;
        }
        index++;
    }
    SMSCLOG("==SMST== smsc_HandleReceivedSms 3 index=%d",index);
    SCI_MEMSET(sms_cmd,0x00,SMS_CMD_BUFF_LEN);
    SCI_MEMCPY(sms_cmd, buff, index);
    SMSCLOG("==SMST== smsc_HandleReceivedSms 2 index=%d",index);
    if(index != len)
    {
        index++;
    }

    if(!memcmp("SMSAMSOTA",sms_cmd,strlen("SMSAMSOTA"))
        || !memcmp("SMSAMSINSTALL",sms_cmd,strlen("SMSAMSINSTALL")))
    {
        char* url = NULL;
        int url_len =  0;
        if(!memcmp("SMSAMSOTA",sms_cmd,strlen("SMSAMSOTA")))
        {
            sms_cmd_value = EVT_CMD_OTA;
            url_len =  len - strlen("SMSAMSOTA");
            url = SCI_ALLOC(url_len+1);
            SCI_MEMSET(url,0,url_len+1);
            SCI_MEMCPY(url,&buff[strlen("SMSAMSOTA")],url_len);
        }
        else if(!memcmp("SMSAMSINSTALL",sms_cmd,strlen("SMSAMSINSTALL")))
        {
            sms_cmd_value = EVT_CMD_INSTALL;
            url_len =  len - strlen("SMSAMSINSTALL");
            url = SCI_ALLOC(url_len+1);
            SCI_MEMSET(url,0,url_len+1);
            SCI_MEMCPY(url,&buff[strlen("SMSAMSINSTALL")],url_len);
        }
        smsc_ReceiveRemoteCmd(sms_cmd_value,0,url);
    }
     else if(!memcmp("SMSAMSDELETEALL",sms_cmd,strlen("SMSAMSDELETEALL")))
    {
        SMSCLOG("==SMST== smsc_HandleReceivedSms  sms_cmd = CMD_DELETEALL %s",sms_cmd);
        sms_cmd_value = EVT_CMD_DELETEALL;
        smsc_ReceiveRemoteCmd(sms_cmd_value,0,0);
    }

    else if(!memcmp("SMSAMSDELETE",sms_cmd,strlen("SMSAMSDELETE")) ||
                !memcmp("SMSAMSRUN",sms_cmd,strlen("SMSAMSRUN")) ||
                !memcmp("SMSAMSDESTROY",sms_cmd,strlen("SMSAMSDESTROY")))
    {
        int buff_len = 0;
        char* data = NULL;
        int suiteID = 0;
        BOOLEAN isNum = FALSE;

        if(!memcmp("SMSAMSDELETE",sms_cmd,strlen("SMSAMSDELETE")) )
        {
            sms_cmd_value = EVT_CMD_DELETE;
            buff_len = len - strlen("SMSAMSDELETE");
            data = SCI_ALLOC(buff_len +1);
        }
        else if(!memcmp("SMSAMSRUN",sms_cmd,strlen("SMSAMSRUN")))
        {
            sms_cmd_value = EVT_CMD_RUN;
            buff_len = len - strlen("SMSAMSRUN");
            data = SCI_ALLOC(buff_len +1);	
        }
        else if(!memcmp("SMSAMSDESTROY",sms_cmd,strlen("SMSAMSDESTROY")) )
        {
            buff_len = len - strlen("SMSAMSDESTROY");
            data = SCI_ALLOC(buff_len +1);
            sms_cmd_value = EVT_CMD_DESTROY;
        }

        SMSCLOG("==SMST== smsc_HandleReceivedSms  sms_cmd = CMD_DELETE/RUN/DESTROY %s buff_len=%d",sms_cmd,buff_len);
        SCI_MEMSET(data,0,buff_len+1);
        SCI_MEMCPY(data,&buff[index-1],buff_len);
        isNum = util_isNumber(data);
        SMSCLOG("==SMST== smsc_HandleReceivedSms  sms_cmd = %s isNum=%d len=%d suiteID=%s",sms_cmd,isNum,buff_len,data);
        if(isNum == TRUE)
        {
            suiteID = atoi(data);
            SCI_FREE(data);
            smsc_ReceiveRemoteCmd(sms_cmd_value,suiteID,0);
        }
        else
        {
            smsc_ReceiveRemoteCmd(sms_cmd_value,-1,data);
        }
    }
    else if(!memcmp("SMSAMSLIST",sms_cmd,strlen("SMSAMSLIST")))
    {	
        SMSCLOG("==SMST== smsc_HandleReceivedSms  sms_cmd = CMD_LIST %s",sms_cmd);
        sms_cmd_value = EVT_CMD_LIST;
        SMSCLOG("==SMST== smsc_HandleReceivedSms  sms_cmd_value = CMD_LIST %d",sms_cmd_value);
        smsc_ReceiveRemoteCmd(sms_cmd_value,0,0);
	    smsc_ackSmsResultMsg(EVT_CMD_LIST,CMD_RESULT_OK);
    }

    else if(!memcmp("SMSAMSSTATUS",sms_cmd,strlen("SMSAMSSTATUS")))
    {
        SMSCLOG("==SMST== smsc_HandleReceivedSms  sms_cmd = CMD_STATUS %s",sms_cmd);
        sms_cmd_value = EVT_CMD_STATUS;
        smsc_ReceiveRemoteCmd(sms_cmd_value,0,0);
    }
    else if(!memcmp("SMSAMSCONFIG",sms_cmd,strlen("SMSAMSCONFIG")))
    {
        // char* server_address = NULL;
        //char* server_port = NULL;
        char* server = NULL;
        int url_len = len - strlen("SMSAMSCONFIG");
        int depart_index = 0;
        sms_cmd_value = EVT_CMD_CFGURL;
        SMSCLOG("==SMST== smsc_HandleReceivedSms  sms_cmd = CMD_CFGURL %s url_len=%d",sms_cmd,url_len);
       /* if(url_len > 0)
        {
            for(depart_index =0 ;depart_index < url_len;depart_index++)
            {
                if(buff[strlen("SMSAMSCONFIG")+depart_index] == ',') break;
            }
            if(depart_index>0)
            {
                SMSCLOG("==SMST== smsc_HandleReceivedSms CMD_CFGURL  buff value = %s",&buff[index]);
                SCI_MEMSET(server_address,0,depart_index+1);
                SCI_MEMSET(server_port,0,url_len - depart_index);
                SCI_MEMCPY(server_address,&buff[strlen("SMSAMSCONFIG")],depart_index);
                SCI_MEMCPY(server_port,&buff[strlen("SMSAMSCONFIG") + depart_index + 1],url_len - depart_index - 1);
                SMSCLOG("==SMST== smsc_HandleReceivedSms  sms_cmd_value = CMD_CFGURL address=%s port=%s",server_address,server_port);
                smsc_ackSmsResultMsg(EVT_CMD_CFGURL,CMD_RESULT_OK);
                smsc_ReceiveRemoteCmd(sms_cmd_value,0,0);
            }
            else
            {
                smsc_ackSmsResultMsg(EVT_CMD_CFGURL,CMD_RESULT_FAILED);
            }
            }*/
            for(depart_index =0 ;depart_index < url_len;depart_index++)
            {
                if(buff[strlen("SMSAMSCONFIG")+depart_index] == ',') {
                    buff[strlen("SMSAMSCONFIG")+depart_index] = '|';
                    break;
				}
            }
            server = SCI_ALLOC(url_len+1);
            SCI_MEMSET(server,0,url_len+1);
            SCI_MEMCPY(server,&buff[strlen("SMSAMSCONFIG")],url_len);
            smsc_ReceiveRemoteCmd(sms_cmd_value,0,server);
            SCI_FREE(server);
       
    }
    else if(!memcmp("SMSAMSCFGACCOUNT",sms_cmd,strlen("SMSAMSCFGACCOUNT"))){
        char* account = NULL;
        int account_len = len - strlen("SMSAMSCFGACCOUNT");
        int depart_index = 0;
        sms_cmd_value = EVT_CMD_CFGACCOUNT;
        for(depart_index =0 ;depart_index < account_len;depart_index++)
        {
             if(buff[strlen("SMSAMSCFGACCOUNT")+depart_index] == ',') {
                    buff[strlen("SMSAMSCFGACCOUNT")+depart_index] = '|';
                    break;}
        }
        if(depart_index>0)
        {                  
                account = SCI_ALLOC(account_len+1);
                SCI_MEMSET(account,0,account_len+1);
                SCI_MEMCPY(account,&buff[strlen("SMSAMSCFGACCOUNT")],account_len);
                smsc_ReceiveRemoteCmd(sms_cmd_value,0,account);
                SCI_FREE(account);
        }
        else
        {
                smsc_ackSmsResultMsg(sms_cmd_value,CMD_RESULT_FAILED);
        }

    }
    else if(!memcmp("SMSAMSINITCANCELALL",sms_cmd,strlen("SMSAMSINITCANCELALL")))
    {
        SMSCLOG("==SMST== smsc_HandleReceivedSms  sms_cmd = SMSAMSINITCANCELALL");
        sms_cmd_value = EVT_CMD_CANCELALL;
        smsc_ReceiveRemoteCmd(sms_cmd_value,0,NULL);
    }
    else if(!memcmp("SMSAMSINITCANCEL",sms_cmd,strlen("SMSAMSINITCANCEL")))
    {
        int suiteID = -1;
        BOOLEAN isNum = FALSE;
        char* data =  NULL;
        int buff_len = len - strlen("SMSAMSINITCANCEL");
        data = SCI_ALLOC(buff_len +1);
        sms_cmd_value = EVT_CMD_CANCEL;    
        SMSCLOG("==SMST== smsc_HandleReceivedSms  sms_cmd = CMD_DELETE/RUN/DESTROY %s buff_len=%d",sms_cmd,buff_len);
        SCI_MEMSET(data,0,buff_len+1);
        SCI_MEMCPY(data,&buff[strlen("SMSAMSINITCANCEL")],buff_len);
        isNum = util_isNumber(data);
        SMSCLOG("==SMST== smsc_HandleReceivedSms  sms_cmd = %s isNum=%d len=%d suiteID=%s",sms_cmd,isNum,buff_len,data);
        if(isNum == TRUE)
        {
            suiteID = atoi(data);
            SCI_FREE(data);
            smsc_ReceiveRemoteCmd(sms_cmd_value,suiteID,0);
        }
        else
        {
            smsc_ReceiveRemoteCmd(sms_cmd_value,suiteID,data);
        }
    }
     else if(!memcmp("SMSAMSINIT",sms_cmd,strlen("SMSAMSINIT")))
    {
        int buff_len = len - strlen("SMSAMSINIT");
        char* data = NULL;      
        sms_cmd_value = EVT_CMD_INIT;
        SMSCLOG("==SMST== smsc_HandleReceivedSms  sms_cmd = CMD_INIT %s url_len=%d",sms_cmd,buff_len);
        if(buff_len > 0)
        {
            data = SCI_ALLOC(buff_len +1);
            SCI_MEMSET(data,0,buff_len+1);
            SCI_MEMCPY(data,&buff[strlen("SMSAMSINIT")],buff_len); 
            smsc_ReceiveRemoteCmd(sms_cmd_value,0,data);
        }
        else
        {
            smsc_ackSmsResultMsg(sms_cmd_value,CMD_RESULT_FAILED);
        }
    }
    else if(!memcmp("SMSAMSRESET",sms_cmd,strlen("SMSAMSRESET")))//TODO : the function is not ok, fixme.
    {
        SMSCLOG("==SMST== smsc_HandleReceivedSms  sms_cmd = CMD_RESET %s",sms_cmd);
        sms_cmd_value = EVT_CMD_RESET;
        smsc_ReceiveRemoteCmd(sms_cmd_value,0,0);
        //RmtReConnect_Pro();
    }

    else
    {
        sms_cmd_value = EVT_CMD_NONE;
        smsc_ackSmsResultMsg(sms_cmd_value,CMD_RESULT_OK);
        return FALSE;
    }

    return TRUE;

}

void smsc_TransformBcdToStr(
    uint8       *bcd_ptr,         // in the BCD array
    int32        bcd_length,      // in the length of BCD array
    uint8        *tel_str,        // out the telephone number string
    uint16       *tel_str_len_ptr  // out, the telephone number length
)
{
    int32        i, j;
    uint8        temp;
    uint16       str_len = 0;

    SCI_ASSERT(PNULL != tel_str);/*assert verified: check null pointer*/

    if(MN_MAX_ADDR_BCD_LEN < bcd_length)
    {
        SMSCLOG("==SMST== smsc_TransformBcdToStr Assert Failure bcd len.");
    }

    SMSCLOG("==SMST== smsc_TransformBcdToStr the BCD CODE length is %d", bcd_length);

    for(i = 0; i < bcd_length; i++)
    {
        for(j = 0; j < 2; j++)
        {
            if(0 == j)
            {
                temp = (uint8)(*bcd_ptr & 0x0f);
            }
            else
            {
                temp = ((*bcd_ptr & 0xf0) >> 4);
            }

            str_len++;

            switch(temp)
            {
                case 10:
                    *tel_str++ = '*';
                    break;
                case 11:
                    *tel_str++ = '#';
                    break;
                case 12:
                    *tel_str++ = 'a';
                    break;
                case 13:
                    *tel_str++ = 'b';
                    break;
                case 14:
                    *tel_str++ = 'c';
                    break;
                case 15:
                    /*current character is '\0'*/
                    str_len--;
                    break;
                default:
                    *tel_str++ = (uint8)(temp + '0');
                    break;
            }
        }

        bcd_ptr++;
    }

    if(NULL != tel_str_len_ptr)
    {
        *tel_str_len_ptr = str_len;
    }

    return;
}

BOOLEAN smsc_ConvertIRAToUCS2(
    uint8      *ira_str_ptr,    //in, points to the ira string
    uint16      ira_str_len,    //in, ira string length
    uint8      *ucs2_str_ptr,    //out, points to buffer used for storing the ucs2 string
    uint16     *ucs2_str_len_ptr   //in, maximum length of buffer which pointed by ucs2_str_ptr
    //out, the length of the ucs2 string converted from ira string
)
{
    uint32       i;
    uint8        semi_octet;
    uint8        *dest_ptr = NULL;

    SCI_ASSERT(PNULL != ira_str_ptr);/*assert verified: check null pointer*/
    SCI_ASSERT(PNULL != ucs2_str_ptr && NULL != ucs2_str_len_ptr);/*assert verified: check null pointer*/

    /*NOTES: 16-bit universal multiple-octet coded character set (ISO/IEC10646 [32]);
    UCS2 character strings are converted to hexadecimal numbers from 0000 to FFFF;
    e.g. "004100620063" equals three 16-bit characters with decimal values 65, 98 and 99.*/

    /*check, whether the maximum length of the ucs2 buffer is enough for storing the ucs2 string
    converted from IRA string.*/
    if((*ucs2_str_len_ptr - 1) < 4 * ira_str_len)
    {
        *ucs2_str_len_ptr = 0;
        return FALSE;
    }

    dest_ptr = ucs2_str_ptr;

    for(i = 0; i < ira_str_len; i++)
    {
        /*set 0 as ucs2 high 8bits character, which should be converted to
            hexadecimal numbers "00"*/
        *dest_ptr = (uint8)48;
        dest_ptr++;
        *dest_ptr = (uint8)48;
        dest_ptr++;

        /*set ira character(e.g. decimal value:65) as ucs2 low 8bits character
        which should be converted to hexadecimal numbers(e.g. "41")*/
        semi_octet = (uint8)((ira_str_ptr[i] & 0xF0) >> 4);

        if(semi_octet <= 9)
        {
            *dest_ptr = (uint8)(semi_octet + '0');
        }
        else
        {
            *dest_ptr = (uint8)(semi_octet + 'A' - 10);
        }

        dest_ptr++;

        semi_octet = (uint8)(ira_str_ptr[i] & 0x0f);

        if(semi_octet <= 9)
        {
            *dest_ptr = (uint8)(semi_octet + '0');
        }
        else
        {
            *dest_ptr = (uint8)(semi_octet + 'A' - 10);
        }

        dest_ptr++;
    }

    *ucs2_str_len_ptr = 4 * ira_str_len;
    return TRUE;
}


BOOLEAN smsc_TransformIRAToOtherChset(
    uint8    *src_str_ptr,           //in, the origination character set string
    uint16    src_str_length,        //in, the origination character set string length
    uint8     dest_chset_type,//in, the destination character set type
    uint16    dest_str_max_len,   //in, the destination character set string maximum length
    uint8     *dest_str_ptr,      //out, the destination character set string
    uint16    *dest_str_len_ptr   //out, the destination character set string length
)
{
    uint16    dest_str_len = 0;
    BOOLEAN   result = TRUE;

    uint8     *temp_buffer_ptr = NULL;
    uint16    temp_str_len = 0;
    uint8     seven_bit_offset = 0;

    SCI_ASSERT(NULL != src_str_ptr);/*assert verified: check null pointer*/
    SCI_ASSERT(NULL != dest_str_ptr);/*assert verified: check null pointer*/

    if(0 == src_str_length || 0 == dest_str_max_len)
    {
        /*the originator string length and destination string maximum length must not be zero */
        SMSCLOG("ATC:transform chset IRA,WARNING,invalid string length. src_str_length:%d,dest_str_max_len:%d",
                      src_str_length, dest_str_max_len);
        return FALSE;
    }

    //SMSCLOG("==SMST==smsc_TransformIRAToOtherChset.%d",dest_chset_type);
    switch(dest_chset_type)
    {
        case ATC_CHSET_IRA:

            /*check the dest buffer size is valid*/
            if(src_str_length >= dest_str_max_len)
            {
                SMSCLOG("==SMST==:transform chset IRA=>IRA,WARNING,dest string buffer overflow.");
                return FALSE;
            }

            /*Originator chset is the same to destination chset. Do not need to transform*/
            SMSCLOG("==SMST==:originator chset and destination chset are both IRA.");
            SCI_MEMCPY(dest_str_ptr, src_str_ptr, src_str_length);
            dest_str_len = src_str_length;
            break;
        case ATC_CHSET_GSM:
            /*e.g. "13800210500" => "B1190E0693C56035180C"*/

            //Firset step, pack 8bits to 7bits(refer to 3GPP 23.038)
            /*Get the maximum length of 7bits string according to the IRA chset string length*/
            temp_str_len = (src_str_length * 7) / 8;

            if((src_str_length * 7) % 8 != 0)
            {
                temp_str_len += 1;
            }

            temp_str_len += 1;    //the last character stores '\0'
            temp_buffer_ptr = (uint8 *)SCI_ALLOC_BASE(temp_str_len);
            SCI_ASSERT(NULL != temp_buffer_ptr);/*assert verified: check null pointer*/
            SCI_MEMSET(temp_buffer_ptr, 0, temp_str_len);

            /*transform 8bit string to 7bit string*/
            if(SCI_SUCCESS != SCI_Pack8bitsTo7bits(src_str_ptr, (int16)src_str_length, temp_buffer_ptr, (int16 *)&temp_str_len, &seven_bit_offset))
            {
                result = FALSE;
                break;
            }

            SMSCLOG("==SMST==:transform chset IRA,GSM string length:%d,sevent bit offset:%d.", temp_str_len, seven_bit_offset);

            /*check, whether the destination buffer length is enough or not*/
            if(temp_str_len * 2 >= dest_str_max_len)
            {
                result = FALSE;
                SMSCLOG("==SMST==:transform chset IRA=>GSM,WARNING,dest string buffer overflow.");
                break;
            }

            /*transform 8 bit characters to hexadecimal values
            e.g. "123" => "313233"*/
            ConvertBinToHex(temp_buffer_ptr, temp_str_len, dest_str_ptr);
            dest_str_len = temp_str_len * 2;
            break;
        case ATC_CHSET_HEX:

            /*check, whether the destination buffer length is enough or not*/
            if(src_str_length * 2 >= dest_str_max_len)
            {
                result = FALSE;
                SMSCLOG("==SMST==:transform chset IRA=>HEX,WARNING,dest string buffer overflow.");
                break;
            }

            /*transform 8 bit characters to hexadecimal string
            e.g. "123" => "313233"*/
            ConvertBinToHex(src_str_ptr, src_str_length, dest_str_ptr);
            dest_str_len = src_str_length * 2;
            break;
        case ATC_CHSET_UCS2:

            /*check, whether the destination buffer length is enough or not*/
            if(src_str_length * 4 >= dest_str_max_len)
            {
                result = FALSE;
                SMSCLOG("==SMST==:transform chset IRA=>UCS2,WARNING,dest string buffer overflow.");
                break;
            }

            /*transform 8 bit characters to ucs2 string(in hexadecimal format)
            e.g. "123" => "003100320033"*/
            dest_str_len = dest_str_max_len;

            if(!smsc_ConvertIRAToUCS2(src_str_ptr, src_str_length, dest_str_ptr, &dest_str_len))
            {
                result = FALSE;
                break;
            }

            break;
        default:
            result = FALSE;
            break;
    }

    if(NULL != dest_str_len_ptr)
    {
        *dest_str_len_ptr = dest_str_len;
    }

    if(NULL != temp_buffer_ptr)
    {
        SCI_FREE(temp_buffer_ptr);
    }

    return result;
}


void smsc_ConvertCalledNumToStr(
    MN_CALLED_NUMBER_T     *called_number_ptr,  //input, call number structure
    int        *tel_type_ptr,       //output,telephone number type
    uint8                  *tel_str_ptr,        //output,telephone number string
    uint16                 *tel_str_len_ptr     //output, telephone number length
)
{
    uint8     *temp_str = PNULL;

    temp_str = tel_str_ptr;

    SCI_ASSERT(PNULL != called_number_ptr);/*assert verified: check null pointer*/
    SMSCLOG("==SMST==: smsc_ConvertCalledNumToStr,Convert called number={%d,%d,%d}....",
                  called_number_ptr->number_type, called_number_ptr->number_plan,
                  called_number_ptr->num_len);

    if(NULL != tel_type_ptr)
    {
        /* bit     8  7  6  5  4  3  2  1
        **     ext| number   |  number    |
        **        | type     |   plan     |
            */
        /*tel type=(called_number_ptr->number_plan +
        (called_number_ptr->number_type << 4))|0x80*/
        if(MN_NUM_PLAN_ISDN_TELE == called_number_ptr->number_plan)
        {
            switch(called_number_ptr->number_type)
            {
                case MN_NUM_TYPE_INTERNATIONAL:
                    *tel_type_ptr = ISDN_TELE_PLAN_INTERNATIONAL_TYPE;
                    break;
                case MN_NUM_TYPE_NATIONAL:
                    *tel_type_ptr = ISDN_TELE_PLAN_NATIONAL_TYPE;
                    break;
                default:
                    *tel_type_ptr = ISDN_TELE_PLAN_UNKNOWN_TYPE;
                    break;
            }
        }
        else
        {
            *tel_type_ptr = UNKNOWN_PLAN_UNKNOWN_TYPE;
        }

        SMSCLOG("==SMST==: smsc_ConvertCalledNumToStr,Get telephone type:%d", *tel_type_ptr);
    }

    if(NULL != temp_str)
    {
        uint8    num_type_len = 0;

        // This telphone number is international number
        if(MN_NUM_TYPE_INTERNATIONAL == called_number_ptr->number_type)
        {
            *temp_str = '+';
            temp_str++;
            num_type_len = 1;
        }

        // bcd code to digital string
        smsc_TransformBcdToStr((uint8 *)called_number_ptr->party_num,
                              called_number_ptr->num_len,
                              temp_str,
                              tel_str_len_ptr);

        if(NULL == tel_str_len_ptr)
        {
            SMSCLOG("==SMST==: smsc_ConvertCalledNumToStr,Get telephone num:%s", tel_str_ptr);
        }
        else
        {
            *tel_str_len_ptr += num_type_len;

            SMSCLOG("==SMST==:smsc_ConvertCalledNumToStr,Get telephone num:%s,the length:%d", tel_str_ptr, *tel_str_len_ptr);
        }

    }

}

BOOLEAN smsc_EncodeCalledNumToTE(
    MN_CALLED_NUMBER_T     *called_number_ptr,  //input, call number structure
    uint8                   dest_chset,       //in, character set of telephone number
    int        *tel_type_ptr,       //output,telephone number type
    uint8                  *tel_str_ptr,        //output,telephone number string buffer
    //the max length must be MAX_SMS_ADDRESS_LEN_FOR_TE
    uint16                 *tel_str_len_ptr     //output, telephone number length
)
{
    uint8     tel_num_arr[MN_MAX_ADDR_BCD_LEN*2+1] = {0};
    uint16    tel_num_len = 0;

    if(NULL == called_number_ptr || NULL == tel_str_ptr)
    {
        SMSCLOG("==SMST==:smsc_EncodeCalledNumToTE, invalid parameters.");
        return FALSE;
    }

    /*first, convert bcd number to ira string*/
    smsc_ConvertCalledNumToStr(called_number_ptr, tel_type_ptr, tel_num_arr, &tel_num_len);

    /*then, convert ira string to specific character set*/
    if(!smsc_TransformIRAToOtherChset(tel_num_arr, tel_num_len,
                                     dest_chset,
                                     MAX_SMS_ADDRESS_LEN_FOR_TE,
                                     tel_str_ptr, tel_str_len_ptr))
    {
        SCI_MEMSET(tel_str_ptr, 0, MAX_SMS_ADDRESS_LEN_FOR_TE);
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}


PUBLIC void smsc_StrToBcdMf(
                    char *pStr,   // String的头指针
                    uint8 strLen, // String的长度
                    uint8 *pBcd   // 转换后BCD码的头指针
                   )
{
    uint8 bcdCode = 0;
    uint8 ascCode = 0;
    int32 i = 0;

  /*memset(pBcd,0xFF,(strLen&1) ? (strLen+1)/2 : strLen/2+1); */

    for(i=0;i < strLen;i++)
    {
        ascCode = pStr[i];
        /*bcdCode = 0xFF;*/

        if(!((ascCode >= '0' && ascCode <= '9')||ascCode == '*'||
              ascCode == '#'||(ascCode == 'P'||ascCode == 'p')||
              ascCode == 'W'||ascCode == 'w'))
            break;
        /*
        bcdCode = ( ascCode == '*' ) ? DIALBCD_STAR:
                  ( ascCode == '#' ) ? DIALBCD_HASH:
                  ( ascCode == 'P'||ascCode == 'p') ? DIALBCD_PAUSE:
                  ( ascCode == 'W' ) ? DIALBCD_WILD:(ascCode - '0');
        */
        switch( ascCode )
        {
            case '*':
                bcdCode = DIALBCD_STAR;
                break;
            case '#':
                bcdCode = DIALBCD_HASH;
                break;
            case 'P':
            case 'p':
                bcdCode = DIALBCD_PAUSE;
                break;
            case 'W':
            case 'w':
                bcdCode = DIALBCD_WILD;
                break;
            default:
                bcdCode = ascCode - '0';
                break;
        }
        pBcd[i/2] = ((i & 1) ? pBcd[i/2] : 0) +
                    (bcdCode << ((i+1 & 1) ? 4 : 0));
    }
    if(i&1)
    {
       pBcd[(i-1)/2] |= 0x0f;
    }

}

/*****************************************************************************/
//  Description : convert the string to the bcd code(PACKED_LSB_FIRST)
//  Global resource dependence :
//  Author: Great.Tian
//  Note:
/*****************************************************************************/
LOCAL void smsc_StrToBcdLf(
                    char *pStr,   // String的头指针
                    uint8 strLen, // String的长度
                    uint8 *pBcd   // 转换后BCD码的头指针
                   )
{
    uint8 bcdCode = 0;
    uint8 ascCode = 0;
    int32 i = 0;

    /*memset(pBcd,0xFF,(strLen&1) ? (strLen+1)/2 : strLen/2+1);*/

    for(i=0;i < strLen;i++)
    {
        ascCode = pStr[i];
        /*bcdCode = 0xFF;*/

        if(!((ascCode >= '0' && ascCode <= '9')||ascCode == '*'||
              ascCode == '#'||(ascCode == 'P'||ascCode == 'p')||
              ascCode == 'W'||ascCode == 'w'||ascCode=='+'))
            break;
        /*
        bcdCode = ( ascCode == '*' ) ? DIALBCD_STAR:
                  ( ascCode == '#' ) ? DIALBCD_HASH:
                  ( ascCode == 'P'||ascCode == 'p') ? DIALBCD_PAUSE:
                  ( ascCode == 'W' ) ? DIALBCD_WILD:(ascCode - '0');
        */
        switch( ascCode )
        {
            case '*':
                bcdCode = DIALBCD_STAR;
                break;
            case '#':
                bcdCode = DIALBCD_HASH;
                break;
            case 'P':
            case 'p':
                bcdCode = DIALBCD_PAUSE;
                break;
            case 'W':
            case 'w':
                bcdCode = DIALBCD_WILD;
                break;
            case '+':
            bcdCode=DIALBCD_EXPANSION;
            break;
            default:
                bcdCode = ascCode - '0';
                break;
        }
        pBcd[i/2] = ((i & 1) ? pBcd[i/2] : 0) +
                    (bcdCode << ((i & 1) ? 4 : 0));
    }
    if(i&1)
    {
       pBcd[(i-1)/2] |= 0xf0;
    }

}

/*****************************************************************************/
//  Description : convert the string to the bcd code(UNPACKED)
//  Global resource dependence :
//  Author: Great.Tian
//  Note:
/*****************************************************************************/
LOCAL void smsc_StrToBcdUp(
                    char *pStr,   // String的头指针
                    uint8 strLen, // String的长度
                    uint8 *pBcd   // 转换后BCD码的头指针
                   )
{
    uint8 bcdCode = 0;
    uint8 ascCode = 0;
    int32 i = 0;

    /*memset(pBcd,0xFF,strLen+1);*/

    for(i=0;i < strLen;i++)
    {
        ascCode = pStr[i];

        if(!((ascCode >= '0' && ascCode <= '9')||ascCode == '*'||
              ascCode == '#'||(ascCode == 'P'||ascCode == 'p')||
              ascCode == 'W'||ascCode == 'w'))
            break;
        /*
        bcdCode = ( ascCode == '*' ) ? DIALBCD_STAR:
                  ( ascCode == '#' ) ? DIALBCD_HASH:
                  ( ascCode == 'P'||ascCode == 'p') ? DIALBCD_PAUSE:
                  ( ascCode == '+' ) ? DIALBCD_WILD:(ascCode - '0');
        */

        switch( ascCode )
        {
            case '*':
                bcdCode = DIALBCD_STAR;
                break;
            case '#':
                bcdCode = DIALBCD_HASH;
                break;
            case 'P':
            case 'p':
                bcdCode = DIALBCD_PAUSE;
                break;
            case 'W':
            case 'w':
                bcdCode = DIALBCD_WILD;
                break;
            default:
                bcdCode = ascCode - '0';
                break;
        }
        pBcd[i] = bcdCode;
    }
}

LOCAL void smsc_StrToBcd(
                  uint8 bcdFmt, // BCD码的类型，目前有三种: PACKED_LSB_FIRST, PACKED_MSB_FIRST, UNPACKED
                  char *pStr,   // String的头指针
                  uint8 *pBcd   // 转换后BCD码的头指针
                 )
{
    uint8 strLen = 0;

    strLen = strlen(pStr);

    switch(bcdFmt)
    {
        case 0://PACKED_LSB_FIRST:
            smsc_StrToBcdLf(pStr,strLen,pBcd);
            break;
        case 1://PACKED_MSB_FIRST:
            smsc_StrToBcdMf(pStr,strLen,pBcd);
            break;
        case 2://UNPACKED        :
            smsc_StrToBcdUp(pStr,strLen,pBcd);
            break;
        default:
            // DevFail("Wrong bcd format");
            break;
    }
}

BOOLEAN smsc_EncodeSMSDataToTE(
    uint8        *data_str,        //in, sms data
    uint16        data_len,        //in, sms data length
    MN_SMS_ALPHABET_TYPE_E      dcs,   //in, the Data Coding Scheme of sms data
    uint8         dest_chset,       //in, character set of sms <data>
    uint8        *data_to_TE_ptr,     //out, sms data(reported to TE)
    uint16       *data_to_TE_len_ptr  //out, sms data(reported to TE) length
)
{
    if(NULL == data_str || 0 == data_len || NULL == data_to_TE_ptr)
    {
        SMSCLOG("===SMST==smsc_EncodeSMSDataToTE, invalid parameters.");
        return FALSE;
    }

    if(MN_SMS_DEFAULT_ALPHABET == dcs)
    {
        /*sms data is decoded first(from GSM to IRA) by calling MNSMS_DecodeUserData*/
        /*ME/TA converts IRA to current TE character set*/
        return smsc_TransformIRAToOtherChset(data_str, data_len,
                                            dest_chset,
                                            SMS_TEXT_BUFF_LEN,
                                            data_to_TE_ptr, data_to_TE_len_ptr);
    }
    else
    {
        /*ME/TA converts each 8-bit octet into two IRA character long hexadecimal number*/
        ConvertBinToHex(data_str, data_len, data_to_TE_ptr);

        if(NULL != data_to_TE_len_ptr)
        {
            *data_to_TE_len_ptr = data_len * 2;
        }

        return TRUE;
    }
}

LOCAL BOOLEAN smsc_GenPartyNumber(
                                        uint8               *tele_num_ptr,
                                        int16               tele_len,
                                        MN_CALLED_NUMBER_T *party_num
                                        )
{
    uint8   offset   = 0;
    uint8*  tele_num = PNULL;

    //check the param
    if (PNULL == tele_num_ptr || PNULL == party_num)
    {
        SMSCLOG("==SMST==smsc_GenPartyNumber PNULL == tele_num_ptr || PNULL == party_num");
        return FALSE;
    }
    if (0 == tele_len)
    {
        // An empty string is given. The length of the BCD number is set to 0
        party_num->num_len = 0;
        SMSCLOG("==SMST==smsc_GenPartyNumber  length = 0\n");
        return FALSE;
    }

    // @Great.Tian, CR12051, begin
    tele_len = MIN(40, tele_len);
    tele_num = (uint8*)SCI_ALLOC_APP(tele_len + 1);
    SCI_MEMSET(tele_num, 0, (tele_len + 1));
    // SCI_MEMCPY(tele_num, tele_num_ptr, tele_len);
    SCI_MEMCPY(
        tele_num,
        tele_num_ptr,
        tele_len);
    // @Great.Tian, CR12051, end

    if('+' == tele_num[0])
    {
        party_num->number_type = MN_NUM_TYPE_INTERNATIONAL;
        offset++;
    }
    else
    {
        party_num->number_type = MN_NUM_TYPE_UNKNOW;
    }

    party_num->num_len = ( tele_len - offset + 1 ) /2;
    SCI_ASSERT( party_num->num_len <=  MN_MAX_ADDR_BCD_LEN);/*assert verified*/
    smsc_StrToBcd( 0 /*PACKED_LSB_FIRST*/,
                (char*)( tele_num + offset),
                party_num->party_num );

    // @Great.Tian, CR12051, begin
    SCI_FREE(tele_num);
    // @Great.Tian, CR12051, end

    return TRUE;

}

LOCAL void smsc_GetMnCalledNum(
                                            uint8 *dest_ptr,
                                            uint8 dest_len,
                                            MN_CALLED_NUMBER_T *party_number_ptr,    //OUT:
                                            MN_CALLED_NUMBER_T *dest_number_ptr     //OUT:
                                            )
{
    uint8 max_dest_num[SMS_RECIEVER_NUM_MAX] = {0};

    if (PNULL == dest_ptr || PNULL == party_number_ptr || PNULL == dest_number_ptr)
    {
        SMSCLOG("===SMST==:smsc_GetMnCalledNum dest_ptr party_number_ptr dest_number_ptr = PNULL");
        return;
    }
    // init the destination number
    dest_number_ptr->num_len     = 0;
    dest_number_ptr->number_plan = MN_NUM_PLAN_UNKNOW;
    dest_number_ptr->number_type = MN_NUM_TYPE_UNKNOW;

    if(dest_len > SMS_RECIEVER_NUM_MAX)
    {
        dest_len = SMS_RECIEVER_NUM_MAX;
    }

    SCI_MEMSET(max_dest_num, 0x00, sizeof(max_dest_num));
    SCI_MEMCPY(max_dest_num, dest_ptr, dest_len);

    if (dest_len > 0)
    {
        smsc_GenPartyNumber((uint8*)max_dest_num, dest_len, party_number_ptr);
        PARTYNUM_2_MNCALLEDNUM(party_number_ptr, dest_number_ptr);
    }
}

BOOLEAN smsc_GetDestNumber(
                                    uint8 *addr_ptr,  //IN
                                    uint8 addr_len,
                                    MN_CALLED_NUMBER_T    *dest_number_ptr    //OUT:
                                    )
{
    BOOLEAN   ret_val = FALSE;
    uint8 addr_id = 0;
    uint8 *dest_ptr = PNULL;
    uint8 dest_len = 0;
    MN_CALLED_NUMBER_T party_number = {MN_NUM_TYPE_UNKNOW,0,0};

    if (PNULL == dest_number_ptr)
    {
        SMSCLOG( "===SMST==:smsc_GetDestNumber dest_number_ptr = PNULL");
        return FALSE;
    }

    // init the destination number
    dest_number_ptr->num_len     = 0;
    dest_number_ptr->number_plan = MN_NUM_PLAN_UNKNOW;
    dest_number_ptr->number_type = MN_NUM_TYPE_UNKNOW;

    dest_ptr = addr_ptr;
    dest_len = addr_len;

    if (dest_len > 0)
    {
        smsc_GetMnCalledNum(dest_ptr, dest_len, &party_number, dest_number_ptr);
        ret_val = TRUE;
    }

    return (ret_val);
}

void smsc_handleSMSInd(void *param)
{
    APP_MN_SMS_IND_T *sig_ptr = (APP_MN_SMS_IND_T *)param;
    MN_SMS_USER_DATA_HEAD_T m_user_data_head = {0};
    MN_SMS_USER_VALID_DATA_T m_user_valid_data = {0};
    BOOLEAN ret = FALSE;
    uint8    data_to_TE[SMS_TEXT_BUFF_LEN] = {0};
    uint16  data_to_TE_len = 0;
    uint8       data_len;
    uint8      *data_str = PNULL;
    uint8      origin_addr_arr[MAX_SMS_ADDRESS_LEN_FOR_TE] = {0};
    uint8      sc_addr_arr[MAX_SMS_ADDRESS_LEN_FOR_TE] = {0};
    int          origin_tele_type = UNKNOWN_PLAN_UNKNOWN_TYPE;
    int          sc_tele_type = UNKNOWN_PLAN_UNKNOWN_TYPE;
    int          origin_tele_len = 0;
    int          sc_tele_len = 0;

    if (NULL == sig_ptr)
    {
        SMSCLOG("==SMST==smsc_handleSMSInd warning: sig_ptr is null.");
        return;
    }

    SMSCLOG("==SMST==smsc_handleSMSInd user_head_is_exist = %d, record_id = %d, storage = %d",
             sig_ptr->sms_t.user_head_is_exist, sig_ptr->record_id, sig_ptr->storage);

    // need call MN API to decode user head and data.
    smsc_EncodeCalledNumToTE(&sig_ptr->sms_t.origin_addr_t,
                                                                ATC_CHSET_IRA,        
                                                                &origin_tele_type,
                                                                origin_addr_arr,
                                                                &origin_tele_len
                                                                );
    SMSCLOG("==SMST==smsc_handleSMSInd warning: oring_len = %d,type=%d,value=%s",origin_tele_len,origin_tele_type,origin_addr_arr);
    smsc_EncodeCalledNumToTE(&sig_ptr->sms_t.sc_addr_t,
                                                                ATC_CHSET_IRA,        
                                                                &sc_tele_type,
                                                                sc_addr_arr,
                                                                &sc_tele_len
                                                                );
    SMSCLOG("==SMST==smsc_handleSMSInd warning: sc_len = %d,type=%d,value=%s",sc_tele_len,sc_tele_type,sc_addr_arr);  

    smsc_GetDestNumber(origin_addr_arr, origin_tele_len, &m_reply_number);

    MNSMS_DecodeUserDataEx(sig_ptr->dual_sys,
                                                        sig_ptr->sms_t.user_head_is_exist,
                                                        &sig_ptr->sms_t.dcs,
                                                        &sig_ptr->sms_t.user_data_t,
                                                        &m_user_data_head,
                                                        &m_user_valid_data);
                     
        // get the data content
    data_len = m_user_valid_data.length;          //mt_sms_ptr->user_data_t.length;
    data_str = m_user_valid_data.user_valid_data_arr;   //mt_sms_ptr->user_data_t.user_data_arr;
    data_str[data_len] = '\0';

    smsc_EncodeSMSDataToTE(data_str, (uint16)data_len, sig_ptr->sms_t.dcs.alphabet_type,
                          ATC_CHSET_IRA, data_to_TE, &data_to_TE_len);
                                                        
    ret = smsc_HandleReceivedSms(data_to_TE,data_to_TE_len);

    //if(ret == TRUE)
    if(1) // here we trade as all sms should be handled
    {
        // call MN API to update SMS status.
        MNSMS_UpdateSmsStatusEx(sig_ptr->dual_sys, sig_ptr->storage, sig_ptr->record_id, MN_SMS_FREE_SPACE);
    }

    MNSMS_FreeUserDataHeadSpaceEx(sig_ptr->dual_sys, &m_user_data_head);
 }
 
void smsc_smsEventHandle(uint32 id, uint32 argc, void *argv)
{
     SMSCLOG("==SMST==smsc_smsEventHandle argc = %d ", argc);

    switch (argc)
    {
        case EV_MN_APP_SMS_IND_F:
            smsc_handleSMSInd(argv);
            break;
        default:
            break;
    }
}

int smsc_getCurrentSmsCmd()
{
    SMSCLOG("==SMST==smsc_getCurrentSmsCmd sms_cmd_value = %d ", sms_cmd_value);
    return sms_cmd_value;
}

void smsc_ackSmsResultMsg(int cmd,int result)
{
    MN_SMS_MO_SMS_T mo_sms = {0};
    MN_CALLED_NUMBER_T src_num = {MN_NUM_TYPE_UNKNOW, MN_NUM_PLAN_UNKNOW, 0, 0};
    MN_SMS_DCS_T   m_dcs  = {0};
    MN_SMS_USER_VALID_DATA_T m_user_data = {0};
    MN_SMS_USER_DATA_T m_out_user_data = {0};
    MN_SMS_USER_DATA_HEAD_T   m_user_data_header = {0};
    SCI_TIME_T time = {0};
    SCI_DATE_T date = {0};
    MN_SMS_TIME_STAMP_T m_time_stamp = {0};
    ERR_MNSMS_CODE_E mn_err_code = ERR_MNSMS_NONE;
    SMSCLOG("==SMST==smsc_ackSmsResultMsg cmd = %d  result=%d", cmd,result);
    //set user data
    switch(cmd)
    {
        case EVT_CMD_OTA:
        {
            SCI_MEMCPY(m_user_data.user_valid_data_arr,"OTA",strlen("OTA"));
            m_user_data.length = strlen("OTA");
        }
        break;
        case EVT_CMD_INSTALL:
        {
            SCI_MEMCPY(m_user_data.user_valid_data_arr,"INSTALL",strlen("INSTALL"));
            m_user_data.length = strlen("INSTALL");
        }
        break;
	
        case EVT_CMD_INIT:
        {
            SCI_MEMCPY(m_user_data.user_valid_data_arr,"INIT",strlen("INIT"));
            m_user_data.length = strlen("INIT");
        }
        break;
        case EVT_CMD_CANCEL:
        {
            SCI_MEMCPY(m_user_data.user_valid_data_arr,"INITCANCEL",strlen("INITCANCEL"));
            m_user_data.length = strlen("INITCANCEL");
        }
        break;
        case EVT_CMD_CANCELALL:
        {
            SCI_MEMCPY(m_user_data.user_valid_data_arr,"INITCANCELALL",strlen("INITCANCELALL"));
            m_user_data.length = strlen("INITCANCELALL");
        }
        break;
        case EVT_CMD_DELETE:
        {
             SCI_MEMCPY(m_user_data.user_valid_data_arr,"DELETE",strlen("DELETE"));
            m_user_data.length = strlen("DELETE");
        }
        break;

        case EVT_CMD_DELETEALL:
        {
            SCI_MEMCPY(m_user_data.user_valid_data_arr,"DELETEALL",strlen("DELETEALL"));
            m_user_data.length = strlen("DELETEALL");
        }
        break;
        
        case EVT_CMD_RUN:
        {
            SCI_MEMCPY(m_user_data.user_valid_data_arr,"RUN",strlen("RUN"));
            m_user_data.length = strlen("RUN");
        }
        break;
        case EVT_CMD_DESTROY:
        {
            SCI_MEMCPY(m_user_data.user_valid_data_arr,"DESTROY",strlen("DESTROY"));
            m_user_data.length = strlen("DESTROY");
        }
        break;
	
        case EVT_CMD_RESET:
        {
            SCI_MEMCPY(m_user_data.user_valid_data_arr,"RESET",strlen("RESET"));
            m_user_data.length = strlen("RESET");
            
        }
        break;

        case EVT_CMD_CFGURL:
        {
            SCI_MEMCPY(m_user_data.user_valid_data_arr,"CFGURL",strlen("CFGURL"));
            m_user_data.length = strlen("CFGURL");
        }
        break;
        case EVT_CMD_CFGACCOUNT:
        {
            SCI_MEMCPY(m_user_data.user_valid_data_arr,"CFGACCOUNT",strlen("CFGACCOUNT"));
            m_user_data.length = strlen("CFGACCOUNT");
        }
        break;
        
        case EVT_CMD_STATUS:
        {
            char *re = NULL;
	     re = amsUtils_getAppletList(TRUE);    
            SCI_MEMCPY(m_user_data.user_valid_data_arr,"STATUS:\n",strlen("STATUS:\n"));
            m_user_data.length = strlen("STATUS:\n");    
            SCI_MEMCPY(&m_user_data.user_valid_data_arr[m_user_data.length],re,strlen(re));
            m_user_data.length = m_user_data.length + strlen(re)-2;
            m_user_data.user_valid_data_arr[m_user_data.length] = 0;


         
            /*SCI_MEMCPY(m_user_data.user_valid_data_arr,"STATUS\n",strlen("STATUS\n"));
            m_user_data.length = strlen("STATUS\n");
            SCI_MEMCPY(&m_user_data.user_valid_data_arr[m_user_data.length+1],re,strlen(re));
            m_user_data.length = m_user_data.length + 1 + strlen(re); */
        }
        break;

        case EVT_CMD_LIST:
        {
	        char *re = NULL;
	        re = amsUtils_getAppletList(FALSE);
            SCI_MEMCPY(m_user_data.user_valid_data_arr,"LIST:\n",strlen("LIST:\n"));
            m_user_data.length = strlen("LIST:\n");    
            SCI_MEMCPY(&m_user_data.user_valid_data_arr[m_user_data.length],re,strlen(re));
            m_user_data.length = m_user_data.length + strlen(re);
            m_user_data.user_valid_data_arr[m_user_data.length] = 0;
        }
        break;        
        case EVT_CMD_NONE:
        default:
        {
            SCI_MEMCPY(m_user_data.user_valid_data_arr,"UNKOWN COMMAND",strlen("UNKOWN COMMAND"));
            m_user_data.length = strlen("UNKOWN COMMAND");
            cmd = EVT_CMD_NONE;
        }
    }

   m_user_data.user_valid_data_arr[m_user_data.length] = 32;

    if(cmd != EVT_CMD_NONE)
    {
        if(result == CMD_RESULT_OK)
        {
            if( cmd == EVT_CMD_LIST)
            {
              //  SCI_MEMCPY(&m_user_data.user_valid_data_arr[m_user_data.length+1],"No App Installed",strlen("No App Installed"));
              //  m_user_data.length = m_user_data.length + 1 + strlen("No App Installed");
           // }
            }
            else if(cmd == EVT_CMD_STATUS)
            {
                //SCI_MEMCPY(&m_user_data.user_valid_data_arr[m_user_data.length+1],"No Running App",strlen("No Running App"));
                //m_user_data.length = m_user_data.length + 1 + strlen("No Running App");
            }
           else
           {
               SCI_MEMCPY(&m_user_data.user_valid_data_arr[m_user_data.length+1],"SUCCESS",strlen("SUCCESS"));
               m_user_data.length = m_user_data.length + 1 + strlen("SUCCESS");  
            }
        }
        else if (result == CMD_RESULT_DOWNLOAD)
        {
            if (cmd == EVT_CMD_OTA)
            {
                 SCI_MEMCPY(&m_user_data.user_valid_data_arr[m_user_data.length+1],"Start download",strlen("Start download"));
                 m_user_data.length = m_user_data.length + 1 + strlen("Start download");
            }
        }
        else
        {
            if( cmd == EVT_CMD_LIST )
            {
                SCI_MEMCPY(&m_user_data.user_valid_data_arr[m_user_data.length+1],"Failed to get App list",strlen("Failed to get App list"));
                m_user_data.length = m_user_data.length + 1 + strlen("Failed to get App list");
            }
			
            else if(cmd == EVT_CMD_STATUS)
            {
                SCI_MEMCPY(&m_user_data.user_valid_data_arr[m_user_data.length+1],"Failed to get Running App list",strlen("Failed to get Running App list"));
                m_user_data.length = m_user_data.length + 1 + strlen("Failed to get Running App list");
            }
            else
            {
                SCI_MEMCPY(&m_user_data.user_valid_data_arr[m_user_data.length+1],"FAILED",strlen("FAILED"));
                m_user_data.length = m_user_data.length + 1 + strlen("FAILED");
            }
        }
    }

   SMSCLOG("==SMST==smsc_ackSmsResultMsg m_user_data = %s  length=%d", m_user_data.user_valid_data_arr,m_user_data.length);
    //smsc_GetUserData(m_user_data.user_valid_data_arr,m_user_data.length,MN_SMS_DEFAULT_ALPHABET,&m_out_user_data);        
   
   MNSMS_EncodeUserDataEx( 
             0,
              FALSE,
              MN_SMS_DEFAULT_ALPHABET,          //IN: the Data Coding Scheme structure
              &m_user_data_header,//IN: the user data header structure
              &m_user_data, //IN: the valid user data structure
              &m_out_user_data
              );
   
    //set time stamp
    TM_GetSysTime(&time);
    TM_GetSysDate(&date);
    m_time_stamp.day = date.mday;
    m_time_stamp.hour = time.hour;
    m_time_stamp.minute = time.min;
    m_time_stamp.month = date.mon;
    m_time_stamp.second = time.sec;
    m_time_stamp.timezone = 0;
    m_time_stamp.year = date.year - DATE_MIN_YEAR;

    //to fill mo sms data
    mo_sms.user_head_is_exist = FALSE;
    mo_sms.reply_path_is_request = FALSE;
    mo_sms.status_report_is_request = FALSE;
    mo_sms.time_format_e = MN_SMS_TP_VPF_RELATIVE_FORMAT;
    mo_sms.time_valid_period.time_second = 50000000;
    mo_sms.tp_mr = 0;
    mo_sms.dest_addr_present  = TRUE;
    mo_sms.dest_addr_t = m_reply_number;
    mo_sms.sc_addr_present = FALSE;
    mo_sms.sc_addr_t = src_num;
    mo_sms.pid_present = TRUE;
    mo_sms.pid_e = MN_SMS_PID_DEFAULT_TYPE;
    mo_sms.dcs = m_dcs;
    mo_sms.user_data_t = m_out_user_data;
    mo_sms.time_stamp_t = m_time_stamp;

    // call the MN API to send request
    mn_err_code = MNSMS_AppSendSmsEx(
        MN_DUAL_SYS_1,
        &mo_sms,
        MN_SMS_NO_STORED,
        MN_SMS_GSM_PATH,
        FALSE
        );

    SMSCLOG("==SMST==smsc_ackSmsResultMsg mn_err_code = %d", mn_err_code);
  /* if(sms_cmd_value == EVT_CMD_RESET && mn_err_code ==0){
        while(true){
            char * temp ;
            sms_cmd_value = EVT_CMD_NONE;
            *temp = malloc(1024 * 256 * 1);
			}  
      }
		  */
    sms_cmd_value = EVT_CMD_NONE;
}

void smsc_ackSmsResultListMsg(int cmd,int result,char* data,int datalen)
{
    MN_SMS_MO_SMS_T mo_sms = {0};
    MN_CALLED_NUMBER_T src_num = {MN_NUM_TYPE_UNKNOW, MN_NUM_PLAN_UNKNOW, 0, 0};
    MN_SMS_DCS_T   m_dcs  = {0};
    MN_SMS_USER_VALID_DATA_T m_user_data = {0};
    MN_SMS_USER_DATA_T m_out_user_data = {0};
    MN_SMS_USER_DATA_HEAD_T   m_user_data_header = {0};
    SCI_TIME_T time = {0};
    SCI_DATE_T date = {0};
    MN_SMS_TIME_STAMP_T m_time_stamp = {0};
    ERR_MNSMS_CODE_E mn_err_code = ERR_MNSMS_NONE;
    SMSCLOG("==SMST==smsc_ackSmsResultListMsg cmd = %d  result=%d", cmd,result);
    
    if(cmd == EVT_CMD_LIST)
    {
        SCI_MEMCPY(m_user_data.user_valid_data_arr,"LIST:\n",strlen("LIST:\n"));
        m_user_data.length = strlen("LIST:\n");
    }
    else
    {
        SCI_MEMCPY(m_user_data.user_valid_data_arr,"STATUS:\n",strlen("STATUS:\n"));
        m_user_data.length = strlen("STATUS:\n");
    }
    
    SCI_MEMCPY(&m_user_data.user_valid_data_arr[m_user_data.length],data,datalen-2);
    m_user_data.length = m_user_data.length + datalen-2;
    m_user_data.user_valid_data_arr[m_user_data.length] = 0;

  MNSMS_EncodeUserDataEx( 
             0,
              FALSE,
              MN_SMS_DEFAULT_ALPHABET,          //IN: the Data Coding Scheme structure
              &m_user_data_header,//IN: the user data header structure
              &m_user_data, //IN: the valid user data structure
              &m_out_user_data
              );
   
    //set time stamp
    TM_GetSysTime(&time);
    TM_GetSysDate(&date);
    m_time_stamp.day = date.mday;
    m_time_stamp.hour = time.hour;
    m_time_stamp.minute = time.min;
    m_time_stamp.month = date.mon;
    m_time_stamp.second = time.sec;
    m_time_stamp.timezone = 0;
    m_time_stamp.year = date.year - DATE_MIN_YEAR;

    //to fill mo sms data
    mo_sms.user_head_is_exist = FALSE;
    mo_sms.reply_path_is_request = FALSE;
    mo_sms.status_report_is_request = FALSE;
    mo_sms.time_format_e = MN_SMS_TP_VPF_RELATIVE_FORMAT;
    mo_sms.time_valid_period.time_second = 50000000;
    mo_sms.tp_mr = 0;
    mo_sms.dest_addr_present  = TRUE;
    mo_sms.dest_addr_t = m_reply_number;
    mo_sms.sc_addr_present = FALSE;
    mo_sms.sc_addr_t = src_num;
    mo_sms.pid_present = TRUE;
    mo_sms.pid_e = MN_SMS_PID_DEFAULT_TYPE;
    mo_sms.dcs = m_dcs;
    mo_sms.user_data_t = m_out_user_data;
    mo_sms.time_stamp_t = m_time_stamp;

    // call the MN API to send request
    mn_err_code = MNSMS_AppSendSmsEx(
        MN_DUAL_SYS_1,
        &mo_sms,
        MN_SMS_NO_STORED,
        MN_SMS_GSM_PATH,
        FALSE
        );

    SMSCLOG("==SMST==smsc_ackSmsResultListMsg mn_err_code = %d", mn_err_code);

    sms_cmd_value = EVT_CMD_NONE;
}
void smsc_registerEvent()
{
	SCI_RegisterMsg( MN_APP_SMS_SERVICE,
		(uint8)(EV_MN_APP_SMS_READY_IND_F),
		(uint8)(MAX_MN_APP_SMS_EVENTS_NUM - 1),
		smsc_smsEventHandle);
	ATC_SendNewATInd(0,11,"AT+CMGF=1\r\n");
}

#endif

