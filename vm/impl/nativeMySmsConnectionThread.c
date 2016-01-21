#include <vm_common.h>
#include <common.h>
#include <dvmdex.h>
#include "nativeMySmsCommon.h"
#include "nativeMySmsConnectionThread.h"

#if defined(ARCH_ARM_SPD)
#include "sci_types.h"
#include "os_api.h"
#include "Sig_code.h"
#include "AsyncIO.h"
#include <ams_remote.h>



/* Stored SMS message (CPL_SmsMessage) */
struct CPL_SmsMessage_s
{
    struct CPL_SmsMessage_s *next; /* Must be first for linked-list operation*/
    SCISmsMessage msg;
};

/**
 * Opaque SMS message structure.
 *
 * The first entry must be a pointer sized field, to allow for chaining
 * a queue. This field is read and written by the PushRegistry.
 */
typedef struct CPL_SmsMessage_s CPL_SmsMessage;

CPL_SmsMessage* pMsgQueue = NULL;

/* Async io notifier to trigger to wake the reader thread,
 * or NULL if not waiting.
 */
static ASYNC_Notifier *notifier = NULL;

//the table is for changing from 03.38 alphabet table to ASCII alphabet table
//add by sunsome.ju
const unsigned char default_to_ascii_table[128] =
{
    0x40,0xA3,0x24,0xA5,0xE8,0xE9,0xF9,0xEC,0xF2,0xC7,0x0A,0xD8,0xF8,0x0D,0xC5,0xE5,
    0x20,0x5F,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0xC6,0xE6,0xDF,0xC9,
    0x20,0x21,0x22,0x23,0xA4,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
    0xA1,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
    0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0xC4,0xD6,0xD1,0xDC,0xA7,
    0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
    0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0xE4,0xF6,0xF1,0xFC,0xE0
};

const unsigned char default_to_ascii_extension_table[128] =
{
    0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
    0x20,0x20,0x20,0x20,0x5e,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
    0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x7B,0x7D,0x20,0x20,0x20,0x20,0x20,0x5c,
    0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x5B,0x7e,0x5D,0x20,
    0x7c,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
    0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
    0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
    0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20
};

/**---------------------------------------------------------------------------*
 **                         Macro Declaration                                 *
 **---------------------------------------------------------------------------*/
#define     MONTH_START             1

/**--------------------------------------------------------------------------*
 **                         STATIC DEFINITION                                *
 **--------------------------------------------------------------------------*/

const uint16 dayfromjan1[13] =
{0,31,59,90,120,151,181,212,243,273,304,334,365};

const uint16 dayfromjan1run[13] =
{0,31,60,91,121,152,182,213,244,274,305,335,366};

const uint16 day_per_mon[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

#define     MMICOM_DATE_MAX_YEAR                2099   //时间设置最大年份
#define     MMICOM_DATE_MIN_YEAR                1980   //时间设置最小年份
#define     MMICOM_SEC_PER_DAY                  86400
#define     MMICOM_SEC_PER_HOUR                 3600
#define     MMICOM_SEC_PER_MIN                  60
#define     MMICOM_DAY_PER_YEAR                 365


/*****************************************************************************/
//     Description : decode and modify sms data
//    Global resource dependence :
//  Author: liming.deng
//    Note:
/*****************************************************************************/
PUBLIC void JAVASMS_DecodeUserHeadAndContent(
                                     MN_DUAL_SYS_E           dual_sys,           //IN:
                                     BOOLEAN                 user_head_is_exist, //IN:
                                     MN_SMS_DCS_T            *dcs_ptr_t,         //IN:
                                     MN_SMS_USER_DATA_T      *user_data_ptr_t,     //IN:
                                     JAVA_SMS_USER_DATA_T     *decode_data_ptr     //OUT:
                                     )
{
    uint8 temp_valid_data[MN_SMS_MAX_USER_VALID_DATA_LENGTH] = {0};
    BOOLEAN is_long_sms_16bit_ref = FALSE;
    BOOLEAN is_long_sms_8bit_ref = FALSE;
    BOOLEAN is_vcard_ref = FALSE;
    BOOLEAN is_java_ref = FALSE;
    BOOLEAN is_mms_ref  = FALSE;

    if(dual_sys >= MN_DUAL_SYS_MAX)
    {
        DVMTraceDbg("mmisms decode dual_sys error, dual_sys = %d", dual_sys);
        return;
    }

    MNSMS_DecodeUserDataEx(
        dual_sys,
        user_head_is_exist,
        dcs_ptr_t,
        user_data_ptr_t,
        &(decode_data_ptr->user_data_head_t),
        &(decode_data_ptr->user_valid_data_t)
        );

    is_long_sms_16bit_ref = (decode_data_ptr->user_valid_data_t.length >= decode_data_ptr->user_valid_data_t.user_valid_data_arr[0] + 1)                    // 最小为消息头部分的长度
        && ((0x06 == decode_data_ptr->user_valid_data_t.user_valid_data_arr[0])    //header length
            && (MN_SMS_CONCAT_16_BIT_REF_NUM == decode_data_ptr->user_valid_data_t.user_valid_data_arr[1])    // 表示为16bit reference number
            && (0x04 == decode_data_ptr->user_valid_data_t.user_valid_data_arr[2]));

    is_long_sms_8bit_ref = (decode_data_ptr->user_valid_data_t.length >= decode_data_ptr->user_valid_data_t.user_valid_data_arr[0] + 1)                    // 最小为消息头部分的长度
        && ((0x05 == decode_data_ptr->user_valid_data_t.user_valid_data_arr[0])     //header length
            && (MN_SMS_CONCAT_8_BIT_REF_NUM == decode_data_ptr->user_valid_data_t.user_valid_data_arr[1])    // 表示为8bit reference number
            && (0x03 == decode_data_ptr->user_valid_data_t.user_valid_data_arr[2]));

    is_vcard_ref = (decode_data_ptr->user_valid_data_t.length >= decode_data_ptr->user_valid_data_t.user_valid_data_arr[0] + 1)                    // 最小为消息头部分的长度
        && ((JAVASMS_HEAD_IEI_FOR_PORT == decode_data_ptr->user_valid_data_t.user_valid_data_arr[1])
            && (JAVASMS_HEAD_IEL_FOR_PORT == decode_data_ptr->user_valid_data_t.user_valid_data_arr[2])
            && (JAVASMS_VCARD_PORT_NUM == ((decode_data_ptr->user_valid_data_t.user_valid_data_arr[3] << 8) | (decode_data_ptr->user_valid_data_t.user_valid_data_arr[4])))
            && (0x00 == decode_data_ptr->user_valid_data_t.user_valid_data_arr[5])
            && (0x00 == decode_data_ptr->user_valid_data_t.user_valid_data_arr[6]));

    is_java_ref = (decode_data_ptr->user_valid_data_t.length >= decode_data_ptr->user_valid_data_t.user_valid_data_arr[0] + 1)                    // 最小为消息头部分的长度
        && (((6 == decode_data_ptr->user_valid_data_t.user_valid_data_arr[0])
                    &&(MN_SMS_APP_PORT_16_BIT_ADDR == decode_data_ptr->user_valid_data_t.user_valid_data_arr[1]))//16Bit Address
                ||
                ((4 == decode_data_ptr->user_valid_data_t.user_valid_data_arr[0])
                    &&(MN_SMS_APP_PORT_8_BIT_ADDR == decode_data_ptr->user_valid_data_t.user_valid_data_arr[1])));//8Bit Address
    is_mms_ref = (decode_data_ptr->user_valid_data_t.length >= decode_data_ptr->user_valid_data_t.user_valid_data_arr[0] + 1)                    // 最小为消息头部分的长度
        && (((0x0B == decode_data_ptr->user_valid_data_t.user_valid_data_arr[0])
                    &&(MN_SMS_APP_PORT_16_BIT_ADDR == decode_data_ptr->user_valid_data_t.user_valid_data_arr[1])));//16Bit Address
    //短信内容修正
    // 本身是长短消息，但是没有设置相应的标志位，当成了一般短消息的
    if (!user_head_is_exist
        && (is_long_sms_16bit_ref
        || is_long_sms_8bit_ref
        || is_vcard_ref
        || is_java_ref
        || is_mms_ref
                ))
    {
        MN_SMS_USER_VALID_DATA_T    temp_data = {0};

        //save header
        SCI_MEMCPY(
                                &decode_data_ptr->user_data_head_t,
                                &decode_data_ptr->user_valid_data_t.user_valid_data_arr[0],
                                (decode_data_ptr->user_valid_data_t.user_valid_data_arr[0] + 1)
                                );

        temp_data.length = decode_data_ptr->user_valid_data_t.length - (decode_data_ptr->user_valid_data_t.user_valid_data_arr[0] + 1);
        SCI_MEMCPY(
            temp_data.user_valid_data_arr,
            &(decode_data_ptr->user_valid_data_t.user_valid_data_arr[decode_data_ptr->user_valid_data_t.user_valid_data_arr[0] + 1]),
            temp_data.length
            );

        decode_data_ptr->user_valid_data_t = temp_data;
    }

    //if is UCS2 code,and valid data length is odd, delete the first byte data
    if (MN_SMS_UCS2_ALPHABET == dcs_ptr_t->alphabet_type
        && (0 != decode_data_ptr->user_valid_data_t.length % MMISMS_VALID_DATA_LEN_DIV))
    {
        //Store the valid data
        SCI_MEMCPY(temp_valid_data,decode_data_ptr->user_valid_data_t.user_valid_data_arr,decode_data_ptr->user_valid_data_t.length);
        decode_data_ptr->user_valid_data_t.length--;

        //re- store the valid data.delete the first byte or last byte.
        SCI_MEMSET(decode_data_ptr->user_valid_data_t.user_valid_data_arr, 0,sizeof(decode_data_ptr->user_valid_data_t.user_valid_data_arr));
        SCI_MEMCPY(decode_data_ptr->user_valid_data_t.user_valid_data_arr,temp_valid_data,decode_data_ptr->user_valid_data_t.length);
    }
}

/*****************************************************************************/
//     Description : is MT for java
//    Global resource dependence :
/*****************************************************************************/
static BOOLEAN JAVASMS_IsMtForJava(
                              MN_SMS_USER_HEAD_T user_head_ptr_t,
                              int32                    *des_port_ptr,
                              int32                    *org_port_ptr
                              )
{
    uint8 i = 0;
    int32 index= -1;
    int32 des_port_num = 0,org_port_num = 0;
    BOOLEAN is_for_java = FALSE;
    uint8 iei_num = user_head_ptr_t.iei_num;

    for (i = 0; i<iei_num;i++)
    {
        DVMTraceDbg("JAVASMS_IsMtForJava: i value is = %d  .iei=%d",
            i, user_head_ptr_t.iei_arr[i].iei);

        if((user_head_ptr_t.iei_arr[i].iei == JAVASMS_HEAD_IEI_FOR_PORT)
            ||(user_head_ptr_t.iei_arr[i].iei == JAVASMS_HEAD_IEL_FOR_PORT))
        {
            index = i;
            break;
        }
        else
        {
            index = -1;
        }
    }

    if (-1 == index)
    {
        DVMTraceDbg("JAVASMS_IsMtForJava: this is commom SMS which is received by java as well!");
        *des_port_ptr = 0;
        *org_port_ptr = 0;
        return TRUE;
    }
    else
    {
        if (user_head_ptr_t.iei_arr[index].iei == MN_SMS_APP_PORT_8_BIT_ADDR)
        {
            //destination  port  number
            des_port_num = user_head_ptr_t.iei_arr[index].iei_data_t[0];
            org_port_num = user_head_ptr_t.iei_arr[index].iei_data_t[1];

        }
        else if (user_head_ptr_t.iei_arr[index].iei == MN_SMS_APP_PORT_16_BIT_ADDR)
        {
            des_port_num = (user_head_ptr_t.iei_arr[index].iei_data_t[0] << 8) | (user_head_ptr_t.iei_arr[index].iei_data_t[1]);
            org_port_num = (user_head_ptr_t.iei_arr[index].iei_data_t[2] << 8) |(user_head_ptr_t.iei_arr[index].iei_data_t[3]);

        }

        *des_port_ptr = des_port_num;
        *org_port_ptr = org_port_num;

        DVMTraceDbg("JAVASMS_IsMtForJava: des_port_num = %d, org_port_num = %d ", des_port_num,org_port_num);
        //is_for_java = CheckSmsIsForJava(des_port_num);
        // when the value of des_port_num is large than zero, we regard it as an java message.
        is_for_java = (des_port_num > 0);
        DVMTraceDbg("JAVASMS_IsMtForJava: the port result is = %d", is_for_java);

        return is_for_java;
    }
}


jboolean RetrieveSmsContent(SCISmsMessage* pMsg)
{
    SCISmsMessage* msg = pMsg;
    CPL_SmsMessage* nodeMsg = NULL;
    uint8_t* smsdata = NULL;

    nodeMsg = (CPL_SmsMessage*)CRTL_malloc(sizeof(CPL_SmsMessage));
    smsdata = (uint8_t*)CRTL_malloc(msg->length);

    if(nodeMsg == NULL || smsdata == NULL)
    {
        DVMTraceDbg(("===Native JAVASMS: RetrieveSmsContent malloc failed."));
        if (nodeMsg) {
            CRTL_free(nodeMsg);
        }
        
        if (smsdata) {
            CRTL_free(smsdata);
        }
        return FALSE;
    }

    CRTL_memset(nodeMsg, 0, sizeof(CPL_SmsMessage));
    CRTL_memset(smsdata, 0, msg->length);
    CRTL_memcpy(nodeMsg->msg.addr, msg->addr, JAVA_PHONE_NUM_MAX_SIZE);
    CRTL_memcpy(smsdata, msg->data, msg->length);
    nodeMsg->msg.srcPort = msg->srcPort;
    nodeMsg->msg.dstPort = msg->dstPort;
    nodeMsg->msg.encoding = msg->encoding;
    nodeMsg->msg.timestamp = msg->timestamp;
    nodeMsg->msg.seq_id = msg->seq_id;
    nodeMsg->msg.seq_idx = msg->seq_idx;
    nodeMsg->msg.seq_len = msg->seq_len;
    nodeMsg->msg.length = msg->length;
    nodeMsg->msg.data = smsdata;
    nodeMsg->next = NULL;

    if (pMsgQueue == NULL)
    {
        pMsgQueue = nodeMsg;
    }
    else
    {
        while(pMsgQueue->next != NULL)
        {
            DVMTraceDbg(("===Native JAVASMS : RetrieveSmsContent pMsgQueue->next != NULL"));
            pMsgQueue = pMsgQueue->next;
        }
        nodeMsg->next = pMsgQueue;
        pMsgQueue = nodeMsg;
    }

    if(notifier != NULL)
    {
        DVMTraceDbg(("===Native JAVASMS RetrieveSmsContent IO notify."));
        AsyncIO_notify(notifier);
        notifier = NULL;
        /* Wakeup interate loop in case it is blocked on event */
    }
}

LOCAL JAVA_LONG_SMS_T* InitLongsmsStruct(void)
{
    uint32 i = 0;
    JAVA_LONG_SMS_T* long_myriadsms_ptr = PNULL;

    DVMTraceDbg(("Native JAVASMS InitLongsmsStruct()"));

    long_myriadsms_ptr = (JAVA_LONG_SMS_T *)SCI_ALLOC(sizeof(JAVA_LONG_SMS_T));

    if (PNULL != long_myriadsms_ptr)
    {
        SCI_MEMSET(long_myriadsms_ptr, 0, sizeof(JAVA_LONG_SMS_T));

        for(i = 0;i < JAVA_MAX_LONG_SMS_NUM; i++)
        {
            long_myriadsms_ptr->myriad_sms[i].seq_id = JAVA_SMS_INVAILD_INDEX;
        }
    }
    return long_myriadsms_ptr;
}


/*****************************************************************************/
//     Description : to get the length according the alpha of message
//    Global resource dependence : none
//  Author:louis.wei
//    Note:
/*****************************************************************************/
PUBLIC void JAVASMS_GetLenByAlpha(
                                 MN_SMS_ALPHABET_TYPE_E    alpha,            //IN:
                                 uint8                     *split_len_ptr,    //OUT: the len to split message
                                 uint8                     *limit_len_ptr    //OUT: the limit len
                                 )
{
    if (PNULL == split_len_ptr || PNULL == limit_len_ptr)
    {
        DVMTraceDbg( "JAVASMS_GetLenByAlpha split_len_ptr num_ptr = PNULL");
        return;
    }

    switch (alpha)
    {
    case MN_SMS_UCS2_ALPHABET:
        (*limit_len_ptr) = MMISMS_JAVA_MAX_UCS2_BYTE_LEN;
        (*split_len_ptr) = MMISMS_JAVA_MAX_UCS2_SPLIT_LEN;
        break;

    case MN_SMS_DEFAULT_ALPHABET:
         (*limit_len_ptr) = MMISMS_JAVA_MAX_DEF_BYTE_LEN;
         (*split_len_ptr) = MMISMS_JAVA_MAX_DEF_SPLIT_LEN;
        break;

    case MN_SMS_8_BIT_ALPHBET:

       (*limit_len_ptr) = MMISMS_JAVA_MAX_8BIT_BYTE_LEN;
       (*split_len_ptr) = MMISMS_JAVA_MAX_8BIT_SPLIT_LEN;
        break;

    default:
        DVMTraceDbg("JAVASMS_GetLenByAlpha alpha = %d", alpha);
        break;
    }
}

/*****************************************************************************/
//  Description : Change from 03.38 alphabet table to ASCII alphabet table
/*****************************************************************************/
LOCAL uint16 MMIAPICOM_Default2Ascii(                          //RETURN:
                         uint8      *default_ptr,   //IN:
                         uint8      *ascii_ptr,     //OUT:
                         uint16     str_len         //IN:
                         )
{
    uint16 ascii_len = 0;
    int32 i      = 0;

    if (PNULL == ascii_ptr || PNULL == default_ptr)
    {
        DVMTraceDbg("JAVASMS MMIAPICOM_Default2Ascii PNULL == ascii_ptr || PNULL == default_ptr!");
        return 0;
    }

    for (i=0; i<str_len; i++)
    {
        if (default_ptr[i] >= 0x80)
        {
            break;
        }
        else
        {
            if (default_ptr[i] == 0x1b)
            {
                i++;
                if (default_ptr[i] >= 0x80)
                {
                    break;
                }
                else
                {
                    ascii_ptr[ascii_len] = default_to_ascii_extension_table[default_ptr[i]];
                    ascii_len++;
                }
            }
            else
            {
                 ascii_ptr[ascii_len] = default_to_ascii_table[default_ptr[i]];
                 ascii_len++;
            }
        }
    }

    return (ascii_len);
}


/*****************************************************************************/
//  Description : Generate to the string number
/*****************************************************************************/
LOCAL uint16 MMIAPICOM_GenDispNumber(
                                     MN_NUMBER_TYPE_E  number_type,
                                     uint8             party_len,
                                     uint8             *party_num,
                                     uint8             *tele_num,
                                     uint16            max_tele_len//!!!!!!!!!可显示的号码的最大长度 + 2(/0 字符串结束符号)
                                     )
{
    uint8   offset = 0;
    uint8   len = 0;
  //mdy by qing.yu@for cr238786
    if (PNULL == tele_num || PNULL == party_num || max_tele_len<=2 || party_len == 0)
    {
        DVMTraceDbg("JAVASMS MMIAPICOM_GenDispNumber max_tele_len = %d, party_len = %d", max_tele_len, party_len);
        return 0;
    }
    if(MN_NUM_TYPE_ALPHANUMERIC == number_type)
    {
        // because the system does not support default character, the number need convert
        len = MIN(max_tele_len, party_len);
        len = MMIAPICOM_Default2Ascii(
                                party_num,
                                tele_num,
                                len
                                );
        return (len);
    }

    //check the tele_num is internation tele
    if( MN_NUM_TYPE_INTERNATIONAL == number_type )
    {
        offset = 1;
        tele_num[0] = '+';
    }


    len = party_len << 1;

    if (len <= (max_tele_len - offset - 1))
    {
        MMIAPICOM_BcdToStr(0 ,party_num, len, (char*)( tele_num + offset ) );
    }
    else
    {
        return 0;
    }
    //SCI_PASSERT(len <= (max_tele_len - offset - 1) ,(("len = %d ,offset =%d"),len,offset)); /*assert to do*/

    return (uint8)strlen((char*)tele_num);
}

PUBLIC BOOLEAN MMIAPICOM_IsLeapYear(uint32 year)
{
  BOOLEAN ret_val = FALSE;

  ret_val = (BOOLEAN)((!(year % 4) && (year % 100)) || !(year % 400));

  return ret_val;
}

/*****************************************************************************/
//    Description :  to translate the time to second
//    Global resource dependence :
//    Author:
//    Note:
/*****************************************************************************/
LOCAL uint32 MMIAPICOM_Tm2Second(
                      uint32 tm_sec,
                      uint32 tm_min,
                      uint32 tm_hour,
                      uint32 tm_mday,
                      uint32 tm_mon,
                      uint32 tm_year
                      )
{
    uint16 i = 0;
    uint32 no_of_days = 0;
    uint32 utc_time = 0;

    //check if the date value range is valid
    if ( (tm_mday > 31) || (tm_mon > 12)
        ||(tm_year < MMICOM_DATE_MIN_YEAR)
        ||(tm_mon < MONTH_START)
        )
    {
        DVMTraceDbg("JAVASMS MMI_COMMON:tm_mday = %d,tm_mon = %d,tm_year = %dn",tm_mday ,tm_mon ,tm_year);
        return 0;
    }

    /* year */
    for (i = MMICOM_DATE_MIN_YEAR; i < tm_year; i++)
    {
        no_of_days += (MMICOM_DAY_PER_YEAR + MMIAPICOM_IsLeapYear(i));
    }

    if(MMIAPICOM_IsLeapYear(tm_year))
    {
        no_of_days +=
            dayfromjan1run[tm_mon-MONTH_START]
            +tm_mday - 1;
    }
    else
    {
        no_of_days +=
            dayfromjan1[tm_mon-MONTH_START]
            +tm_mday - 1;
    }

    /* sec */
    utc_time =
         no_of_days *MMICOM_SEC_PER_DAY +  tm_hour * MMICOM_SEC_PER_HOUR +
                                                                tm_min * MMICOM_SEC_PER_MIN + tm_sec;

    return utc_time;
}


/*****************************************************************************/
//     Description : operate MT ind as for java
//    Global resource dependence :
/*****************************************************************************/
LOCAL JAVA_SMS_ERROR_ID_E RetrieveSmsContentForJava(                                    //RETURN:
                                                                    JAVA_SMS_USER_DATA_T    *user_data_ptr,    //IN:
                                                                    MN_SMS_USER_HEAD_T*  user_head_ptr,        //IN:
                                                                    MN_SMS_STORAGE_E    storage,        //IN:
                                                                    MN_SMS_RECORD_ID_T    record_id,        //IN:
                                                                    MN_SMS_MT_SMS_T        sms_t,
                                                                    int32 des_port,
                                                                    int32 org_port
                                                                    )
{
    BOOLEAN ret = FALSE;
    //JAVA_SMS_ERROR_ID_E sms_error_id = JAVA_SMS_ERROR_NO;
    uint8 iei_num = 0;
    int32 i = 0, index = -1;
    int32 ref_num = 0, seq_num = 0,max_num = 0;
    BOOLEAN is_long_sms = FALSE;

    SCISmsMessage*     sci_sms_java_ptr = PNULL;
    JAVA_SMS_CONTENT_T *valid_date_ptr = PNULL;
    uint8 limit_bytes = 0;
    uint8 split_bytes = 0;
    uint8* data_ptr = PNULL;
    uint8 receive_num = 1;
    uint8 sms_index[JAVA_MAX_LONG_SMS_NUM]={0};
    int32 empty_pos = -1;

    //SCI_ASSERT(PNULL != user_data_ptr && PNULL != user_head_ptr);
    if(PNULL == user_data_ptr || PNULL == user_head_ptr)
    {
        DVMTraceDbg("JAVASMS RetrieveSmsContentForJava return directly.");
        return JAVA_SMS_ERROR_ALLOC;
    }

    DVMTraceDbg("JAVASMS RetrieveSmsContentForJava enter.");

    iei_num = user_head_ptr->iei_num;
    for (i = 0; i <iei_num; i++)
    {
        if((user_head_ptr->iei_arr[i].iei == MN_SMS_CONCAT_8_BIT_REF_NUM)
            ||(user_head_ptr->iei_arr[i].iei == MN_SMS_CONCAT_16_BIT_REF_NUM))
        {
            index = i;
            break;
        }
        else
        {
            index = -1;
        }
    }

    if (index != -1)
    {
        if (user_head_ptr->iei_arr[index].iei == MN_SMS_CONCAT_8_BIT_REF_NUM)
        {
            ref_num = user_head_ptr->iei_arr[index].iei_data_t[0];
            max_num = user_head_ptr->iei_arr[index].iei_data_t[1];
            seq_num = user_head_ptr->iei_arr[index].iei_data_t[2];
        }
        else if (user_head_ptr->iei_arr[index].iei == MN_SMS_CONCAT_16_BIT_REF_NUM)
        {
            ref_num = user_head_ptr->iei_arr[index].iei_data_t[0];
            ref_num += user_head_ptr->iei_arr[index].iei_data_t[1];
            max_num = user_head_ptr->iei_arr[index].iei_data_t[2];
            seq_num = user_head_ptr->iei_arr[index].iei_data_t[3];
        }
    }
    else
    {
        max_num = 1;
        ref_num  = record_id;
        seq_num = 1;
    }

    if (1 == max_num)
    {
        is_long_sms = FALSE;
    }
    else
    {
        is_long_sms = TRUE;
    }
    //set CPL_SmsMessage info
    //设置基本信息
    JAVASMS_GetLenByAlpha(sms_t.dcs.alphabet_type, &split_bytes, &limit_bytes);
    sci_sms_java_ptr = (SCISmsMessage*)SCI_ALLOC_APP(sizeof(SCISmsMessage));

    if(PNULL == sci_sms_java_ptr)
    {
        return JAVA_SMS_ERROR_ALLOC;
    }
    SCI_MEMSET(sci_sms_java_ptr, 0, sizeof(SCISmsMessage));

    MMIAPICOM_GenDispNumber(
                            sms_t.origin_addr_t.number_type,
                            MIN(MN_MAX_ADDR_BCD_LEN, sms_t.origin_addr_t.num_len),
                            sms_t.origin_addr_t.party_num,
                            sci_sms_java_ptr->addr,
                            JAVA_PHONE_NUM_MAX_SIZE
                        );
    sci_sms_java_ptr->dstPort = des_port;
    sci_sms_java_ptr->srcPort = org_port;

    sci_sms_java_ptr->timestamp = (uint64)MMIAPICOM_Tm2Second(
                                            sms_t.time_stamp_t.second,
                                            sms_t.time_stamp_t.minute,
                                            sms_t.time_stamp_t.hour,
                                            sms_t.time_stamp_t.day,
                                            sms_t.time_stamp_t.month,
                                            sms_t.time_stamp_t.year + JAVASMS_MT_OFFSERT_YEAR
                                        );
    sci_sms_java_ptr->seq_id = ref_num;
    sci_sms_java_ptr->seq_idx = seq_num;
    sci_sms_java_ptr->seq_len = max_num;
    sci_sms_java_ptr->encoding = sms_t.dcs.alphabet_type;

    data_ptr = (uint8*)SCI_ALLOC_APP(user_data_ptr->user_valid_data_t.length+1);
    if(PNULL == data_ptr)
    {
        SCI_FREE(sci_sms_java_ptr);
        return JAVA_SMS_ERROR_ALLOC;
    }
    SCI_MEMSET(data_ptr,0,user_data_ptr->user_valid_data_t.length + 1);
    SCI_MEMCPY(data_ptr,user_data_ptr->user_valid_data_t.user_valid_data_arr, user_data_ptr->user_valid_data_t.length);

    if (is_long_sms)
    {
        LOCAL JAVA_LONG_SMS_T* s_long_myriadsms_ptr = PNULL;
        if (PNULL == s_long_myriadsms_ptr)
        {
            s_long_myriadsms_ptr = InitLongsmsStruct();

            if (PNULL == s_long_myriadsms_ptr)
            {
                SCI_FREE(data_ptr);
                SCI_FREE(sci_sms_java_ptr)
                return JAVA_SMS_ERROR_ALLOC;
            }
        }

        for(i = 0;i < JAVA_MAX_LONG_SMS_NUM;i++)
        {
            if (ref_num == s_long_myriadsms_ptr->myriad_sms[i].seq_id)
            {
                if (seq_num != s_long_myriadsms_ptr->myriad_sms[i].seq_idx)
                {
                    sms_index[receive_num] = i;
                    receive_num++;
                }
                else
                {

                    SCI_FREE(data_ptr);
                    SCI_FREE(sci_sms_java_ptr)
                    return JAVA_SMS_ERROR_REPEAT;
                }
            }
        }


        if (receive_num < max_num)
        {

            for(i = 0;i < JAVA_MAX_LONG_SMS_NUM;i++)
            {
                if (s_long_myriadsms_ptr->myriad_sms[i].timestamp == 0)
                {
                    empty_pos = i;
                    break;
                }
            }

            if ((empty_pos < JAVA_MAX_LONG_SMS_NUM) && (empty_pos != -1))
            {
                sci_sms_java_ptr->length = user_data_ptr->user_valid_data_t.length;
                sci_sms_java_ptr->data = data_ptr;
                SCI_MEMCPY(&(s_long_myriadsms_ptr->myriad_sms[empty_pos]),
                    sci_sms_java_ptr,
                    sizeof(SCISmsMessage));
            }
            //SCI_FREE(data_ptr);
            SCI_FREE(sci_sms_java_ptr);
            return JAVA_SMS_ERROR_NO;
        }
        else
        {

            int32 j = 1;
            int32 copy_len = 0;
            valid_date_ptr = (JAVA_SMS_CONTENT_T*)SCI_ALLOC_APP(sizeof(JAVA_SMS_CONTENT_T));

            if(PNULL == valid_date_ptr)
            {
                if(PNULL != data_ptr)
                {
                    SCI_FREE(data_ptr)
                }
                if(PNULL != sci_sms_java_ptr)
                {
                    SCI_FREE(sci_sms_java_ptr)
                }
                return JAVA_SMS_ERROR_ALLOC;
            }
            SCI_MEMSET(valid_date_ptr, 0, sizeof(JAVA_SMS_CONTENT_T));

            while(j <= receive_num)
            {
                if(JAVA_SMS_CONTENT_MAX_LEN <= valid_date_ptr->length)
                {
                    valid_date_ptr->length = JAVA_SMS_CONTENT_MAX_LEN;
                    break;
                }
                for(i = 1;i < receive_num;i++)
                {
                    if (s_long_myriadsms_ptr->myriad_sms[sms_index[i]].seq_idx== j)
                    {
                        copy_len = MIN(JAVA_SMS_CONTENT_MAX_LEN - valid_date_ptr->length, s_long_myriadsms_ptr->myriad_sms[sms_index[i]].length);

                        SCI_MEMCPY(valid_date_ptr->data + valid_date_ptr->length, s_long_myriadsms_ptr->myriad_sms[sms_index[i]].data, copy_len);

                        valid_date_ptr->length += copy_len;

                        SCI_FREE(s_long_myriadsms_ptr->myriad_sms[sms_index[i]].data);

                        SCI_MEMSET(&(s_long_myriadsms_ptr->myriad_sms[sms_index[i]]), 0, sizeof(SCISmsMessage));

                        s_long_myriadsms_ptr->myriad_sms[sms_index[i]].seq_id = JAVA_SMS_INVAILD_INDEX;

                        break;
                    }
                    else if (seq_num == j)
                    {
                        copy_len = MIN(JAVA_SMS_CONTENT_MAX_LEN - valid_date_ptr->length, user_data_ptr->user_valid_data_t.length);

                        SCI_MEMCPY(valid_date_ptr->data + valid_date_ptr->length, data_ptr,copy_len);

                        valid_date_ptr->length += copy_len;

                        break;
                    }
                }
                j++;
            }
            sci_sms_java_ptr->data = valid_date_ptr->data;
            sci_sms_java_ptr->length = valid_date_ptr->length;
            sci_sms_java_ptr->seq_idx = 1;
            sci_sms_java_ptr->seq_len = 1;
        }


        for(i = 0;i < JAVA_MAX_LONG_SMS_NUM; i++)
        {
            if (JAVA_SMS_INVAILD_INDEX != s_long_myriadsms_ptr->myriad_sms[i].seq_id)
            {
                break;
            }
        }

        if (JAVA_MAX_LONG_SMS_NUM == i)
        {
            SCI_FREE(s_long_myriadsms_ptr);
            s_long_myriadsms_ptr = PNULL;
        }
        else
        {
            DVMTraceDbg("RetrieveSmsContentForJava exist vcard sms");
        }
    }
    else
    {
        sci_sms_java_ptr->data = data_ptr;
        sci_sms_java_ptr->length = user_data_ptr->user_valid_data_t.length;
    }

    ret = RetrieveSmsContent(sci_sms_java_ptr);
    SCI_FREE(data_ptr);
    SCI_FREE(sci_sms_java_ptr);
    if(PNULL != valid_date_ptr)
    {
        SCI_FREE(valid_date_ptr);
    }

    DVMTraceDbg("JAVASMS RetrieveSmsContentForJava exit.");

    return JAVA_SMS_ERROR_NO;
}


static void JAVASMS_HandleSMSMTInd(void *param)
{
    APP_MN_SMS_IND_T *sig_ptr = (APP_MN_SMS_IND_T *)param;
    JAVA_SMS_USER_DATA_T sms_user_data_t = {0};
    MN_SMS_USER_HEAD_T    user_head_ptr_t = {0};
    BOOLEAN is_new_java = FALSE;
    int32 des_port = 0,org_port = 0;
    MN_DUAL_SYS_E dual_sys;
    MN_SMS_STORAGE_E storage;
    MN_SMS_RECORD_ID_T record_id;
    MN_SMS_MT_SMS_T sms_t;

    if (NULL == sig_ptr)
    {
        DVMTraceDbg("JAVASMS_HandleSMSMTInd warning: sig_ptr is null.");
        return;
    }
    else
    {
        DVMTraceDbg("JAVASMS_HandleSMSMTInd enter.");
    }

    dual_sys = sig_ptr->dual_sys;
    storage = sig_ptr->storage;
    record_id = sig_ptr->record_id;
    sms_t = sig_ptr->sms_t;


    SCI_MEMSET( &sms_user_data_t, 0x00, sizeof( JAVA_SMS_USER_DATA_T ) );

    DVMTraceDbg("HandleSMSMTInd user_head_is_exist = %d, record_id = %d, storage = %d",
             sms_t.user_head_is_exist, record_id, storage);

    // need call MN API to decode user head and data.
    JAVASMS_DecodeUserHeadAndContent(
        dual_sys,
        sms_t.user_head_is_exist,
        &(sms_t.dcs),
        &(sms_t.user_data_t),
        &sms_user_data_t
        );

    MNSMS_DecodeUserDataHeadEx(
                        dual_sys,
                        &sms_user_data_t.user_data_head_t,
                        &user_head_ptr_t
                        );

    // 判断是否为java sms
    is_new_java = JAVASMS_IsMtForJava(user_head_ptr_t,&des_port,&org_port);
    if (is_new_java)
    {
        RetrieveSmsContentForJava(
                        &sms_user_data_t,
                        &user_head_ptr_t,
                        storage,
                        record_id,
                        sms_t,
                        des_port,
                        org_port);

        // call MN API to update SMS status.
        MNSMS_UpdateSmsStatusEx(dual_sys, storage, record_id, MN_SMS_FREE_SPACE);
    }

    MNSMS_FreeUserDataHeadSpaceEx(dual_sys, &user_head_ptr_t);
}

static void sms_msgCallbck(uint32 id, uint32 argc, void *argv)
{
    DVMTraceDbg("sms_msgCallbck argc = %d ", argc);

    switch (argc)
    {
        case EV_MN_APP_SMS_IND_F:
            JAVASMS_HandleSMSMTInd(argv);
            break;

        default:
            break;
    }
}
#endif // ARCH_ARM_SPD

/**
 * Class:     jp_co_cmcc_message_sms_MySmsConnectionThread
 * Method:    nReadMessage
 * Signature: ()Z
 */
void Java_jp_co_cmcc_message_sms_MySmsConnectionThread_nReadMessage(const u4* args, JValue* pResult) {
    jboolean ret = FALSE;
    ClassObject* thisObj = (ClassObject*) args[0];
    jbyte * bufferPtr = (jbyte *)(KNI_GET_ARRAY_BUF(args[1]));
    jint bufferLen = (jint) args[2];

#if defined(ARCH_ARM_SPD)
    CPL_SmsMessage *message = NULL;

    if (pMsgQueue)
    {
        message = pMsgQueue;
        pMsgQueue = pMsgQueue->next;
    }

    if (message != NULL)
    {
        SCISmsMessage *sciMsg = &(message->msg);
        uint8_t *msgBuffer = (uint8_t *)bufferPtr;
        int type = JAVA_ENC_GSM_7BIT;
        int addlen = strlen(sciMsg->addr);
        int txtlen = 0;
        int64_t ts = (int64_t)(sciMsg->timestamp * ((int64_t)1000L));
        int32_t timeHigh = (int32_t)((ts >> 32) & 0xffffffff);
        int32_t timeLow = (int32_t)(ts & 0xffffffff);

        DVMTraceDbg(("java native MySmsConnectionThread: nReadMessage - get data. encoding = %d", sciMsg->encoding));
        switch(sciMsg->encoding)
        {
            case MN_SMS_8_BIT_ALPHBET:
                type = JAVA_ENC_8BIT_BIN;
                break;

            case MN_SMS_UCS2_ALPHABET:
                type = JAVA_ENC_UCS_2;
                break;

            case MN_SMS_DEFAULT_ALPHABET:
            default:
                type = JAVA_ENC_GSM_7BIT;
                break;
        }

        /* Wrap data into msg buffer as below format:
         * srcPort(4bytes) + dstPort(4bytes) + type(4bytes) + time_high(4bytes) + time_high(4bytes) +
         * addlen(4bytes) + address(addlen bytes) + txtlen(4bytes) + text(txtlen bytes).
         */
        writebeIU32(&msgBuffer[0], sciMsg->srcPort);
        writebeIU32(&msgBuffer[4], sciMsg->dstPort);
        writebeIU32(&msgBuffer[8], type);
        writebeIU32(&msgBuffer[12], timeHigh);
        writebeIU32(&msgBuffer[16], timeLow);
        writebeIU32(&msgBuffer[20], addlen);
        memcpy(&msgBuffer[24], sciMsg->addr, addlen);
        txtlen = (sciMsg->length >= (bufferLen - 28 - addlen) ? (bufferLen - 28 - addlen) : sciMsg->length);
        writebeIU32(&msgBuffer[24+addlen], txtlen);
        memcpy(&msgBuffer[28+addlen], sciMsg->data, txtlen);

        DVMTraceDbg(("java native MySmsConnectionThread: nReadMessage - sms reading address = %s, srcPort =%d,desport=%d,type=%d.",
            sciMsg->addr, sciMsg->srcPort, sciMsg->dstPort, type));
         DVMTraceDbg(("java native MySmsConnectionThread: nReadMessage timeHigh = %d, timeLow = %d ", timeHigh, timeLow));

        CRTL_free(message->msg.data);
        CRTL_free(message);

        ret = TRUE;
    }
    else
    {
        DVMTraceDbg(("===java message: nReadMessage - no messages pending - blocking==="));

        notifier = Async_getCurNotifier();
        DVMTraceDbg(("===java  message nReadMessage get notifier = 0x%x===",notifier));
        AsyncIO_callAgainWhenSignalled();
    }

#endif

    RETURN_BOOLEAN(ret);
}

/**
 * Class:     jp_co_cmcc_message_sms_MySmsConnectionThread
 * Method:    nDeleteMessage
 * Signature: ()V
 */
void Java_jp_co_cmcc_message_sms_MySmsConnectionThread_nDeleteMessage(const u4* args, JValue* pResult) {

    DVMTraceDbg(("java native MySmsConnectionThread nDeleteMessage enter."));
}

/**
 * Class:     jp_co_cmcc_message_sms_MySmsConnectionThread
 * Method:    nRegister
 * Signature: ()Z
 */
void Java_jp_co_cmcc_message_sms_MySmsConnectionThread_nRegister(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jboolean ret = FALSE;

    DVMTraceDbg("java native MySmsConnectionThread_nRegister enter.");

#if defined(ARCH_ARM_SPD)
    {
    //Register sms event
    uint32 sciRet = SCI_RegisterMsg( MN_APP_SMS_SERVICE,
            (uint8)(EV_MN_APP_SMS_READY_IND_F & 0xff), /*lint !e778*/
            (uint8)((MAX_MN_APP_SMS_EVENTS_NUM - 1) & 0xff),
            sms_msgCallbck);
    notifier = NULL;
    ret = (sciRet == SCI_SUCCESS ? TRUE : FALSE);
    }
#endif

    DVMTraceDbg("java native MySmsConnectionThread_nRegister ret = %d", ret);

    RETURN_BOOLEAN(ret);
}

/**
 * Class:     jp_co_cmcc_message_sms_MySmsConnectionThread
 * Method:    nUnregister
 * Signature: ()V
 */
void Java_jp_co_cmcc_message_sms_MySmsConnectionThread_nUnregister(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];

    DVMTraceDbg("java native MySmsConnectionThread_unbind enter.");

#if defined(ARCH_ARM_SPD)
    notifier = NULL;
    //Unregister sms event
    SCI_UnregisterMsg( MN_APP_SMS_SERVICE,
        (uint8)(EV_MN_APP_SMS_READY_IND_F & 0xff), /*lint !e778*/
        (uint8)((MAX_MN_APP_SMS_EVENTS_NUM - 1) & 0xff),
        sms_msgCallbck);
#endif
}

