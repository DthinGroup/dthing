/******************************************************************************
** File Name:      atc_basic_cmd.c                                           *
** Author:         Xueli.lv                                                  *
** Date:           12/08/2002                                                *
** Copyright:      Copyright 2001-2010 by Spreadtrum Communications,Inc.     *
**                 All Rights Reserved.                                      *
**                 This software is supplied under the terms of a license    *
**                 agreement or non-disclosure agreement with Spreadtrum.    *
**                 Passing on and copying of this document,and communication *
**                 of its contents is not permitted without prior written    *
**                 authorization.                                            *
** Description:                                                              *
******************************************************************************

******************************************************************************
**                        Edit History                                       *
** --------------------------------------------------------------------------*
** DATE           NAME             Description:                              *
**---------------------------------------------------------------------------*
** 12/08/2002     Xueli.lv         Create.                                   *
******************************************************************************/
#include "atc_basic_cmd.h"
#ifndef ATC_SAT_ENABLE
#include "atc_stk.h"
#include "atc_eng.h"
#endif
#include "atc_gsm_sms.h"
#include "atc.h"
#include "sci_service.h"
#include "atc_common.h"
#include "mnclassmark_api.h"
#include "layer1_audio.h"
#include "record.h"
#include "version.h"

#include "aud_gen.h"

#include "efs.h"
#include "sio.h"
//#include "mn_type.h"

#include "nv_item_id.h"
//#include "env_atc_signal.h"
#include "dal_time.h"
#include "dal_chr.h"
#include "tasks_id.h"
#include "dal_power.h"
#include "nvitem.h"
#include "atc_gsm.h"
#include "custom_tone_adp.h"
#include "dtmf_tone_adp.h"
#include "atc_gsm_ss.h"
#ifndef ATC_SAT_ENABLE
#include "Wifi_drv.h"
#include "Bt_abs.h"
#include "Bt_cfg.h"
#include "layer1_engineering.h"

#ifndef WIN32
#include "gpio_drvapi.h"
#endif
#endif

#include "generic_tone_adp.h"
//#include "aud_enha_exp.h"
#include "admm_III.h"
#include "audio_config.h"


#ifdef WIN32
#include "arm_reg.h"
#endif

#include "prod_param.h"
#include "ref_outport.h"
#include "aud_proc_exp.h"

#ifdef ATC_SAT_ENABLE
#include "power.h"
#endif

#ifdef AUDIO_PARAM_ARM
#include "audio_nv_arm.h"
#include "audio_nv_dsp.h"
#include "aud_enha_exp.h"
#endif

#include "lvve_ctrl_param_nv.h"
#include "cpl_atcontrol.h"
#include "modem_public.h"

#ifdef WIN32
#define ATC_MODE          0x01
#define DATA_MODE         0x02
#define MIX_MODE    0x03
#define COM_USER          0
#define COM_DATA          0
#define COM_DEBUG         1
#endif

/**---------------------------------------------------------------------------*
**                         Compiler Flag                                     *
**---------------------------------------------------------------------------*/
#ifdef   __cplusplus
{
#endif
/**---------------------------------------------------------------------------*
 **                         Declare Variable                                  *
 **---------------------------------------------------------------------------*/
#define ATAMS_USAGE "[USAGE]\r\n\
ATAMSOTA=\"<URL>\"\r\n\
ATAMSINSTALL=\"<URL>\"\r\n\
ATAMSDL=\"<URL>\"\r\n\
ATAMSDELETE=<SuiteID>\r\n\
ATAMSDELETEALL\r\n\
ATAMSRUN=<SuiteID>\r\n\
ATAMSLIST\r\n\
ATAMSDESTROY=<SuiteID>\r\n\
ATAMSSTATUS\r\n\
ATAMSRESET\r\n\
ATAMSCFGINIT=\"<SuiteID>\"\r\n\
ATAMSCFGINIT=\"<SuiteID1,SuiteID2>\"\r\n\
ATAMSCFGCANCEL=<SuiteID>\r\n\
ATAMSCFGCANCELALL\r\n\
ATAMSCFGURL=\"<ip|port>\"\r\n\
ATAMSCFGACCOUNT=\"<user|password>\"\r\n"

static HAUDIO hAtcCustomRing = INVALID_HANDLE;
static HAUDIO hAtcDtmfRing = INVALID_HANDLE;
extern MN_DUAL_SYS_E       g_current_card_id;

static AUDIO_DEVICE_MODE_TYPE_E       s_aud_dev_mod = AUDIO_DEVICE_MODE_HANDHOLD;
static uint16                s_b_mic_aux_device = FALSE;
static uint16                s_b_spk_aux_device = FALSE;
static uint16                s_nv_item_id = 0;
static uint8                 s_atc_admm_index = 0;
static uint8                 s_enha_eqmode_index = 0;

#ifndef ATC_SAT_ENABLE
static int16                 s_dsp_ram_flag;
static int16                 s_dsp_data_length;
static int16                 s_dsp_dest_addr;

static uint16                s_atc_eq_type = AUD_EQ_NORMAL;
static uint8                 s_atc_euicc_cnt[MN_SYS_NUMBER] = {0};
extern char                 *g_stk_terminal_profile[];
extern ATC_POWER_INF_T       g_atc_power_info;
#endif

extern RSP_RESULT_STR        g_rsp_str;
extern const ATC_INFO_T      g_atc_info_table[];
extern const ATC_RSP_STR_T   g_atc_cpin_rsp[];
extern const ATC_RSP_STR_T   g_atc_ecpin2_rsp[];
extern BOOLEAN               g_atc_call_flag;
extern ATC_CALLS_STATE_T     g_calls_state[MN_SYS_NUMBER];
BOOLEAN                      g_is_ps_deactivate[MN_SYS_NUMBER] = {FALSE};
SCI_TMID                     T_ATC_POWEROFF_EXP_IND = PNULL;
BOOLEAN                      g_received_sim_ready[MN_SYS_NUMBER] = {FALSE};   //flag to indicate the end of initialized pin check course or not
BOOLEAN                      g_atc_pcm_flag = FALSE;

BOOLEAN               g_is_ps_act[MN_SYS_NUMBER] = {FALSE};
static OPER_STATUS_E                s_prev_oper_status = UNKNOWN;
static OPER_STATUS_E                s_prev_gprs_oper_status = UNKNOWN;

extern const char *g_atc_charset[ATC_CHSET_MAX_NUM];
uint8  s_mt_need_open_codec_flag = 0; //mt call, this flag==CC_TCH_RESOURCE_ASSIGNED|CC_MT_CALL_CONNECTING,open audio. ref:MMI

BOOLEAN  g_atc_ps_not_save_sms[MN_SYS_NUMBER] = {0};

#define MO_CALL_STATE_INIT              0x00
#define MO_CALL_STATE_SYNC              0x01
#define MO_CALL_STATE_SETUP_OR_PROGRESS 0x02
#define MO_CALL_STATE_DONE              (MO_CALL_STATE_SYNC|MO_CALL_STATE_SETUP_OR_PROGRESS)
#define ATC_NT_RING_CON_ID              3
#define ATC_AUDIO_TONE_DEFAULT_FREQ      2000
#define ATC_CC_3SECOND                  3000
#define ATC_1SECONDS                    1000  //1 second

#define RR_CELL_INFO_SIZE 6

#ifdef ATC_SAT_ENABLE
static uint8                 s_swversion[20]={"version1.01"};
static uint8                 s_cid[4] = {"cid"};
static uint8                 s_gmid[7] = {"gmid"};
static uint8                 s_seed[16] = {"seed"};

uint8                        g_atc_ciev_rang = 3;//default value
uint8                        g_atc_ciev_lev[4] = {102, 96, 90, 84};
uint16   g_rxlev_arr[MN_SYS_NUMBER] = {0};
extern BOOLEAN RM_GetCalibrationPostMode(void);

extern BOOLEAN    is_init_ps_power_on[MN_SYS_NUMBER];
#endif

static HAUDIO  hAtcGenericRing = INVALID_HANDLE;

static SCI_TMID           s_network_ring_timer = NULL;
static BOOLEAN            is_play_local_tone = FALSE;
static ATC_AUDIO_STATUS_T s_audio_status = {FALSE};
static uint8              s_is_downlnk_mute_mocalling_flag = 0; // mo call

extern ATC_PLMN_INFO_T g_cur_plmn_info[MN_SYS_NUMBER];
extern BOOLEAN            s_sms_exclusive_flag;


HAUDIO hVoiceRing = NULL;
/***********************************************************/

#ifdef _MUX_ENABLE_
extern uint32 MUX_Setbaud_rate(uint32 baud_rate);
#endif

#ifndef ATC_SAT_ENABLE
extern PUBLIC void POWER_BootReset(void);
#endif

LOCAL void ATC_SetDefaultParams(
    ATC_CONFIG_T   *atc_config_ptr,
    MN_DUAL_SYS_E   dual_sys
);

/*****************************************************************************/
//  Description : unsolicited service info which is supported by modem
//  Global resource dependence : none
//  Author: xiaonian.li
//  Note:
/*****************************************************************************/
LOCAL void ATC_UpSNWSTATInfo(
    ATC_CONFIG_T                *atc_config_ptr,
    APP_MN_NETWORK_STATUS_IND_T *sig_ptr
    );

/***********************************************************/

#if 0
const uint8 c_speaker_gain[AT_SPEAKER_MAX_GAIN + 1] =
{
    0, 1, 2, 3, 4, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6
};
#endif

const AT_BAUTRATE_CONTEXT_T c_baudrate_data[AT_IPR_MAX] =
{
    {BAUD_1200, AT_BAUD_1200}, {BAUD_2400, AT_BAUD_2400}, {BAUD_4800, AT_BAUD_4800},
    {BAUD_9600, AT_BAUD_9600}, {BAUD_19200, AT_BAUD_19200}, {BAUD_38400, AT_BAUD_38400},
    {BAUD_57600, AT_BAUD_57600}, {BAUD_115200, AT_BAUD_115200}, {BAUD_230400, AT_BAUD_230400},
    {BAUD_460800, AT_BAUD_460800}
};

static const AT_DATA_CONTEXT_T     c_wdtmf_data[] =
{
    {AUD_DTMF_Zero,  0x30},   // '0'
    {AUD_DTMF_One,   0x31},   // '1'
    {AUD_DTMF_Two,   0x32},   // '2'
    {AUD_DTMF_Three, 0x33},   // '3'
    {AUD_DTMF_Four,  0x34},   // '4'
    {AUD_DTMF_Five,  0x35},   // '5'
    {AUD_DTMF_Six,   0x36},   // '6'
    {AUD_DTMF_Seven, 0x37},   // '7'
    {AUD_DTMF_Eight, 0x38},   // '8'
    {AUD_DTMF_Nine,  0x39},   // '9'
    {AUD_DTMF_Star,  0x2A},   // '*'
    {AUD_DTMF_Pond,  0x23},   // '#'
    {AUD_DTMF_letterA, 0x41}, // 'A'
    {AUD_DTMF_letterB, 0X42}, // 'B'
    {AUD_DTMF_letterC, 0X43}, // 'C'
    {AUD_DTMF_letterD, 0X44}  // 'D'
};

#if 0
static const uint8          c_sidet_data[AT_MAX_SIDET_LEV] =
{4, 3, 2, 1, 0};
#endif

/*****************************************************************************/
//  Description : This function is used to chech the time string.
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
LOCAL BOOLEAN CheckTimeStr(
    uint8      length,
    uint8      *str,
    SCI_DATE_T   *date_ptr,
    SCI_TIME_T   *tm_ptr
);

/*****************************************************************************/
//  Description : This function converts the hex to bin
//  Global resource dependence :
//  Author:       hanjun.liu
//  Note:
/*****************************************************************************/
LOCAL BOOLEAN   ConvertInt16ToBin(    // FALSE: There is invalid char
    uint8        *hex_ptr,    // in: the hexadecimal format string of address
    uint16       *bin_ptr      // out: pointer to the address
);

/*****************************************************************************/
//  Description : This function is used to restore the configuration profile.
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
LOCAL void RestoreConfig(
    ATC_CONFIG_T   *atc_config_ptr,
    MN_DUAL_SYS_E   dual_sys
);

/*****************************************************************************/
//  Description : This function handle the AT command which has one parameter
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
LOCAL void HandleSingleCmd(
    ATC_CUR_INFO_LIST_T  *cur_cmd_ptr,   // point to the atc cmd info
    ATC_CONFIG_T         *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    uint8                default_val,     // the default value
    uint8                *cur_val         // the current value
);

/*****************************************************************************/
//  Description : play local network tone to simulate network tone
//  Global resource dependence : none
//  Author: xiaonian.li
//  Note:
/*****************************************************************************/
LOCAL void ATC_PlayLocalNetTone(
                uint32 tone_id,
                uint32 duration,
                uint32 freq
                );

/*****************************************************************************/
//  Description : tch assigned or call disconnected, mo stop playing
//                local ring to simulate the net ring
//  Global resource dependence :
//  Author:xiaonian.li
//  Note:
/*****************************************************************************/
LOCAL void ATC_StopLocalNetRing(void);

/*****************************************************************************/
//  Description : if MS is not attach user connection, ms start playing local
//                ring to simulate the net ring
//  Global resource dependence :
//  Author:xiaonian.li
//  Note:
/*****************************************************************************/
LOCAL void ATC_StartLocalNetRing(void);

#ifndef ATC_SAT_ENABLE
/*****************************************************************************/
//  Description : This function get BT mac address.
//  Global resource dependence : g_rsp_str
//  Author:winnie.zhu
/*****************************************************************************/
LOCAL BOOLEAN BTATC_GetMac(uint8 *bt_addr, uint8 max_data_len);

/*****************************************************************************/
//  Description : This function get WiFi mac address.
//  Global resource dependence : g_rsp_str
//  Author:winnie.zhu
/*****************************************************************************/
LOCAL BOOLEAN  WIFIATC_GetMac(uint8 *mac_addr, uint8 max_data_len);
#endif

/*****************************************************************************/
//  Description : This function initializes the static variables
//  Global resource dependence :
//  Author:       hanjun.liu
//  Note:
/*****************************************************************************/
void ATC_InitBasicVar(void)
{
    uint8    i = 0;

    for(i = MN_DUAL_SYS_1; i < MN_SYS_NUMBER; i++)
    {
        g_is_ps_deactivate[i] = FALSE;
        g_is_ps_act[i] = FALSE;
    }
}

/*****************************************************************************/
//  Description : This function set the static variables g_is_ps_act
//  Global resource dependence :
//  Author:
//  Note:
/*****************************************************************************/
void Atc_SetPsActState(
        MN_DUAL_SYS_E dual_sys,
        BOOLEAN is_act
        )
{
    g_is_ps_act[dual_sys] = is_act;
}


/*****************************************************************************/
//  Description : Audio dummy notify callback function.
//  Global resource dependence : none
//  Author: shujing.dong
//  Note:
/*****************************************************************************/
static void AudioDummyNotifyCallback(
    HAUDIO hAudio,
    uint32 notify_info,
    uint32 affix_info
)
{
    //do nothing
}

/*****************************************************************************/
//  Description : This function handle the ATAMS command
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessAMS)
{
#ifndef _ULTRA_LOW_CODE_
    int result = -1;
    char *paramStr = 0x0;
    char *outStr = 0x0;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:
            ATC_TRACE_LOW("ATC_ProcessAMS:ATC_CMD_TYPE_EXECUTE");
            ATC_BuildInfoRsp(atc_config_ptr, ATAMS_USAGE);
            break;

        default:
            ATC_TRACE_LOW("ATAMS:OPERATION NOT SUPPORTED");
            return ERR_OPERATION_NOT_SUPPORTED;
    }

    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the ATAMSOTA command
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessAMSOTA)
{
#ifndef _ULTRA_LOW_CODE_
    int result = -1;
    char *paramStr = 0x0;
    char *outStr = 0x0;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:
            ATC_TRACE_LOW("ATC_ProcessAMS:ATC_CMD_TYPE_EXECUTE");
            ATC_BuildInfoRsp(atc_config_ptr, ATAMS_USAGE);
            ATC_EnableModemFileProtocol(atc_config_ptr->current_link_id);
            break;

        case ATC_CMD_TYPE_SET:
            ATC_TRACE_LOW("ATC_ProcessAMS:ATC_CMD_TYPE_SET");
            if ((PARAM1_FLAG) && (PARAM1_TYPE == ATC_CMD_PARAM_TYPE_STRING))
            {
                ATC_TRACE_LOW("[Param] %s", PARA1.str_ptr->str_ptr);
                paramStr = PARA1.str_ptr->str_ptr;
            }

            result = cpl_handleATRequest("OTA", paramStr, &outStr);

            if (result > 0)
            {
                if (outStr != NULL)
                {
                    ATC_BuildInfoRsp(atc_config_ptr, outStr);
                    cpl_freeOutStr(&outStr);
                }
            }
            else
            {
                return ERR_UNKNOWN;
            }
            break;

        default:
            ATC_TRACE_LOW("ATAMS:OPERATION NOT SUPPORTED");
            return ERR_OPERATION_NOT_SUPPORTED;
    }

    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the ATAMSINSTALL command
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessAMSINSTALL)
{
#ifndef _ULTRA_LOW_CODE_
    int result = -1;
    char *paramStr = 0x0;
    char *outStr = 0x0;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:
            ATC_TRACE_LOW("ATC_ProcessAMS:ATC_CMD_TYPE_EXECUTE");
            ATC_BuildInfoRsp(atc_config_ptr, ATAMS_USAGE);
            break;
        case ATC_CMD_TYPE_SET:
            ATC_TRACE_LOW("ATC_ProcessAMS:ATC_CMD_TYPE_SET");
            if ((PARAM1_FLAG) && (PARAM1_TYPE == ATC_CMD_PARAM_TYPE_STRING))
            {
                ATC_TRACE_LOW("[Param] %s", PARA1.str_ptr->str_ptr);
                paramStr = PARA1.str_ptr->str_ptr;
            }

            result = cpl_handleATRequest("INSTALL", paramStr, &outStr);

            if (result > 0)
            {
                if (outStr != NULL)
                {
                    ATC_BuildInfoRsp(atc_config_ptr, outStr);
                    cpl_freeOutStr(&outStr);
                }
            }
            else
            {
                return ERR_UNKNOWN;
            }
            break;

        default:
            ATC_TRACE_LOW("ATAMS:OPERATION NOT SUPPORTED");
            return ERR_OPERATION_NOT_SUPPORTED;
    }
    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the ATAMSDL command
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessAMSDL)
{
#ifndef _ULTRA_LOW_CODE_
    int result = -1;
    char *paramStr = 0x0;
    char *outStr = 0x0;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:
            ATC_TRACE_LOW("ATC_ProcessAMS:ATC_CMD_TYPE_EXECUTE");
            ATC_BuildInfoRsp(atc_config_ptr, ATAMS_USAGE);
            break;
        case ATC_CMD_TYPE_SET:
            ATC_TRACE_LOW("ATC_ProcessAMS:ATC_CMD_TYPE_SET");
            if ((PARAM1_FLAG) && (PARAM1_TYPE == ATC_CMD_PARAM_TYPE_STRING))
            {
                ATC_TRACE_LOW("[Param] %s", PARA1.str_ptr->str_ptr);
                paramStr = PARA1.str_ptr->str_ptr;
            }

            result = cpl_handleATRequest("OSGI", paramStr, &outStr);

            if (result > 0)
            {
                if (outStr != NULL)
                {
                    ATC_BuildInfoRsp(atc_config_ptr, outStr);
                    cpl_freeOutStr(&outStr);
                }
            }
            else
            {
                return ERR_UNKNOWN;
            }
            break;

        default:
            ATC_TRACE_LOW("ATAMS:OPERATION NOT SUPPORTED");
            return ERR_OPERATION_NOT_SUPPORTED;
    }
    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the ATAMSDELETE command
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessAMSDELETE)
{
#ifndef _ULTRA_LOW_CODE_
    int result = -1;
    char *outStr = 0x0;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:
            ATC_TRACE_LOW("ATC_ProcessAMS:ATC_CMD_TYPE_EXECUTE");
            ATC_BuildInfoRsp(atc_config_ptr, ATAMS_USAGE);
            break;
        case ATC_CMD_TYPE_SET:
            ATC_TRACE_LOW("ATC_ProcessAMS:ATC_CMD_TYPE_SET");
            if ((PARAM1_FLAG) && (PARAM1_TYPE == ATC_CMD_PARAM_TYPE_NUMERIC))
            {
                ATC_TRACE_LOW("[Param] %d", PARA1.num);
                result = cpl_handleATRequestWithNumericParam("DELETE", PARA1.num, &outStr);
            }

            if (result > 0)
            {
                if (outStr != NULL)
                {
                    ATC_BuildInfoRsp(atc_config_ptr, outStr);
                    cpl_freeOutStr(&outStr);
                }
            }
            else
            {
                return ERR_UNKNOWN;
            }
            break;

        default:
            ATC_TRACE_LOW("ATAMS:OPERATION NOT SUPPORTED");
            return ERR_OPERATION_NOT_SUPPORTED;
    }

    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the ATAMSDELETEALL command
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessAMSDELETEALL)
{
#ifndef _ULTRA_LOW_CODE_
    int result = -1;
    char *paramStr = 0x0;
    char *outStr = 0x0;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:
            ATC_TRACE_LOW("ATC_ProcessAMS:ATC_CMD_TYPE_EXECUTE");
            result = cpl_handleATRequest("DELETEALL", 0x0, &outStr);

            if (result > 0)
            {
                if (outStr != NULL)
                {
                  ATC_BuildInfoRsp(atc_config_ptr, outStr);
                  cpl_freeOutStr(&outStr);
                }
            }
            else
            {
                return ERR_UNKNOWN;
            }
            break;

        default:
            ATC_TRACE_LOW("ATAMS:OPERATION NOT SUPPORTED");
            return ERR_OPERATION_NOT_SUPPORTED;
    }

    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the ATAMSRUN command
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessAMSRUN)
{
#ifndef _ULTRA_LOW_CODE_
    int result = -1;
    char *outStr = 0x0;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:
            ATC_TRACE_LOW("ATC_ProcessAMS:ATC_CMD_TYPE_EXECUTE");
            ATC_BuildInfoRsp(atc_config_ptr, ATAMS_USAGE);
            break;
        case ATC_CMD_TYPE_SET:
            ATC_TRACE_LOW("ATC_ProcessAMS:ATC_CMD_TYPE_SET");
            if ((PARAM1_FLAG) && (PARAM1_TYPE == ATC_CMD_PARAM_TYPE_NUMERIC))
            {
                ATC_TRACE_LOW("[Param] %d", PARA1.num);
                result = cpl_handleATRequestWithNumericParam("RUN", PARA1.num, &outStr);
            }

            if (result > 0)
            {
                if (outStr != NULL)
                {
                  ATC_BuildInfoRsp(atc_config_ptr, outStr);
                  cpl_freeOutStr(&outStr);
                }
            }
            else
            {
                return ERR_UNKNOWN;
            }
            break;

        default:
            ATC_TRACE_LOW("ATAMS:OPERATION NOT SUPPORTED");
            return ERR_OPERATION_NOT_SUPPORTED;
    }

    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the ATAMSLIST command
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessAMSLIST)
{
#ifndef _ULTRA_LOW_CODE_
    int result = -1;
    char *outStr = 0x0;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:
            ATC_TRACE_LOW("ATC_ProcessAMS:ATC_CMD_TYPE_EXECUTE");
            result = cpl_handleATRequest("LIST", 0x0, &outStr);

            if ((result > 0) && (outStr != NULL))
            {
                ATC_BuildInfoRsp(atc_config_ptr, outStr);
                cpl_freeOutStr(&outStr);
            }
            else
            {
                return ERR_UNKNOWN;
            }
            break;

        default:
            ATC_TRACE_LOW("ATAMS:OPERATION NOT SUPPORTED");
            return ERR_OPERATION_NOT_SUPPORTED;
    }

    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the ATAMSDESTROY command
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessAMSDESTROY)
{
#ifndef _ULTRA_LOW_CODE_
    int result = -1;
    char *outStr = 0x0;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:
            ATC_TRACE_LOW("ATC_ProcessAMS:ATC_CMD_TYPE_EXECUTE");
            ATC_BuildInfoRsp(atc_config_ptr, ATAMS_USAGE);
            break;
        case ATC_CMD_TYPE_SET:
             ATC_TRACE_LOW("ATC_ProcessAMS:ATC_CMD_TYPE_SET");
            if ((PARAM1_FLAG) && (PARAM1_TYPE == ATC_CMD_PARAM_TYPE_NUMERIC))
            {
                ATC_TRACE_LOW("[Param] %d", PARA1.num);
                result = cpl_handleATRequestWithNumericParam("DESTROY", PARA1.num, &outStr);
            }

            if (result > 0)
            {
                if (outStr != NULL)
                {
                  ATC_BuildInfoRsp(atc_config_ptr, outStr);
                  cpl_freeOutStr(&outStr);
                }
            }
            else
            {
                return ERR_UNKNOWN;
            }
            break;

        default:
            ATC_TRACE_LOW("ATAMS:OPERATION NOT SUPPORTED");
            return ERR_OPERATION_NOT_SUPPORTED;
    }

    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the ATAMSSTATUS command
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessAMSSTATUS)
{
#ifndef _ULTRA_LOW_CODE_
    int result = -1;
    char *outStr = 0x0;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:
            ATC_TRACE_LOW("ATC_ProcessAMS:ATC_CMD_TYPE_EXECUTE");
            result = cpl_handleATRequest("STATUS", 0x0, &outStr);

            if ((result > 0) && (outStr != NULL))
            {
                ATC_BuildInfoRsp(atc_config_ptr, outStr);
                cpl_freeOutStr(&outStr);
            }
            else
            {
                return ERR_UNKNOWN;
            }
            break;

        default:
            ATC_TRACE_LOW("ATAMS:OPERATION NOT SUPPORTED");
            return ERR_OPERATION_NOT_SUPPORTED;
    }

    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the ATAMSRESET command
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessAMSRESET)
{
#ifndef _ULTRA_LOW_CODE_
    int result = -1;
    char *outStr = 0x0;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:
            ATC_TRACE_LOW("ATC_ProcessAMS:ATC_CMD_TYPE_EXECUTE");
            result = cpl_handleATRequest("RESET", 0x0, &outStr);

            if (result > 0)
            {
                if (outStr != NULL)
                {
                    ATC_BuildInfoRsp(atc_config_ptr, outStr);
                    cpl_freeOutStr(&outStr);
                }
            }
            else
            {
                return ERR_UNKNOWN;
            }
            break;

        default:
            ATC_TRACE_LOW("ATAMS:OPERATION NOT SUPPORTED");
            return ERR_OPERATION_NOT_SUPPORTED;
    }

    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the ATAMSCFGINIT command
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessAMSCFGINIT)
{
#ifndef _ULTRA_LOW_CODE_
    int result = -1;
    char *paramStr = 0x0;
    char *outStr = 0x0;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:
            ATC_TRACE_LOW("ATC_ProcessAMS:ATC_CMD_TYPE_EXECUTE");
            ATC_BuildInfoRsp(atc_config_ptr, ATAMS_USAGE);
            break;
        case ATC_CMD_TYPE_SET:
            ATC_TRACE_LOW("ATC_ProcessAMS:ATC_CMD_TYPE_SET");
            if ((PARAM1_FLAG) && (PARAM1_TYPE == ATC_CMD_PARAM_TYPE_STRING))
            {
                ATC_TRACE_LOW("[Param] %s", PARA1.str_ptr->str_ptr);
                paramStr = PARA1.str_ptr->str_ptr;
            }

            result = cpl_handleATRequest("CFGINIT", paramStr, &outStr);

            if (result > 0)
            {
                if (outStr != NULL)
                {
                    ATC_BuildInfoRsp(atc_config_ptr, outStr);
                    cpl_freeOutStr(&outStr);
                }
            }
            else
            {
                return ERR_UNKNOWN;
            }
            break;

        default:
            ATC_TRACE_LOW("ATAMS:OPERATION NOT SUPPORTED");
            return ERR_OPERATION_NOT_SUPPORTED;
    }
    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the ATAMSCFGURL command
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessAMSCFGURL)
{
#ifndef _ULTRA_LOW_CODE_
    int result = -1;
    char *paramStr = 0x0;
    char *outStr = 0x0;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:
            ATC_TRACE_LOW("ATC_ProcessAMS:ATC_CMD_TYPE_EXECUTE");
            ATC_BuildInfoRsp(atc_config_ptr, ATAMS_USAGE);
            break;
        case ATC_CMD_TYPE_SET:
            ATC_TRACE_LOW("ATC_ProcessAMS:ATC_CMD_TYPE_SET");
            if ((PARAM1_FLAG) && (PARAM1_TYPE == ATC_CMD_PARAM_TYPE_STRING))
            {
                ATC_TRACE_LOW("[Param] %s", PARA1.str_ptr->str_ptr);
                paramStr = PARA1.str_ptr->str_ptr;
            }

            result = cpl_handleATRequest("CFGURL", paramStr, &outStr);

            if (result > 0)
            {
                if (outStr != NULL)
                {
                    ATC_BuildInfoRsp(atc_config_ptr, outStr);
                    cpl_freeOutStr(&outStr);
                }
            }
            else
            {
                return ERR_UNKNOWN;
            }
            break;

        default:
            ATC_TRACE_LOW("ATAMS:OPERATION NOT SUPPORTED");
            return ERR_OPERATION_NOT_SUPPORTED;
    }
    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the ATAMSCFGACCOUNT command
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessAMSCFGACCOUNT)
{
#ifndef _ULTRA_LOW_CODE_
    int result = -1;
    char *paramStr = 0x0;
    char *outStr = 0x0;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:
            ATC_TRACE_LOW("ATC_ProcessAMS:ATC_CMD_TYPE_EXECUTE");
            ATC_BuildInfoRsp(atc_config_ptr, ATAMS_USAGE);
            break;
        case ATC_CMD_TYPE_SET:
            ATC_TRACE_LOW("ATC_ProcessAMS:ATC_CMD_TYPE_SET");
            if ((PARAM1_FLAG) && (PARAM1_TYPE == ATC_CMD_PARAM_TYPE_STRING))
            {
                ATC_TRACE_LOW("[Param] %s", PARA1.str_ptr->str_ptr);
                paramStr = PARA1.str_ptr->str_ptr;
            }

            result = cpl_handleATRequest("CFGACCOUNT", paramStr, &outStr);

            if (result > 0)
            {
                if (outStr != NULL)
                {
                    ATC_BuildInfoRsp(atc_config_ptr, outStr);
                    cpl_freeOutStr(&outStr);
                }
            }
            else
            {
                return ERR_UNKNOWN;
            }
            break;

        default:
            ATC_TRACE_LOW("ATAMS:OPERATION NOT SUPPORTED");
            return ERR_OPERATION_NOT_SUPPORTED;
    }
    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the ATAMSCFGCANCEL command
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessAMSCFGCANCEL)
{
#ifndef _ULTRA_LOW_CODE_
    int result = -1;
    char *outStr = 0x0;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:
            ATC_TRACE_LOW("ATC_ProcessAMS:ATC_CMD_TYPE_EXECUTE");
            ATC_BuildInfoRsp(atc_config_ptr, ATAMS_USAGE);
            break;
        case ATC_CMD_TYPE_SET:
            ATC_TRACE_LOW("ATC_ProcessAMS:ATC_CMD_TYPE_SET");
            if ((PARAM1_FLAG) && (PARAM1_TYPE == ATC_CMD_PARAM_TYPE_NUMERIC))
            {
                ATC_TRACE_LOW("[Param] %d", PARA1.num);
                result = cpl_handleATRequestWithNumericParam("CFGCANCEL", PARA1.num, &outStr);
            }

            if (result > 0)
            {
                if (outStr != NULL)
                {
                    ATC_BuildInfoRsp(atc_config_ptr, outStr);
                    cpl_freeOutStr(&outStr);
                }
            }
            else
            {
                return ERR_UNKNOWN;
            }
            break;

        default:
            ATC_TRACE_LOW("ATAMS:OPERATION NOT SUPPORTED");
            return ERR_OPERATION_NOT_SUPPORTED;
    }

    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the ATAMSCFGCANCELALL command
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessAMSCFGCANCELALL)
{
#ifndef _ULTRA_LOW_CODE_
    int result = -1;
    char *paramStr = 0x0;
    char *outStr = 0x0;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:
            ATC_TRACE_LOW("ATC_ProcessAMS:ATC_CMD_TYPE_EXECUTE");
            result = cpl_handleATRequest("CFGCANCELALL", 0x0, &outStr);

            if (result > 0)
            {
                if (outStr != NULL)
                {
                  ATC_BuildInfoRsp(atc_config_ptr, outStr);
                  cpl_freeOutStr(&outStr);
                }
            }
            else
            {
                return ERR_UNKNOWN;
            }
            break;

        default:
            ATC_TRACE_LOW("ATAMS:OPERATION NOT SUPPORTED");
            return ERR_OPERATION_NOT_SUPPORTED;
    }

    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the ATQ command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessQ)
{
#ifndef _ULTRA_LOW_CODE_
    HandleSingleCmd(cur_cmd_info, atc_config_ptr, 0, &atc_config_ptr->q_value);
    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the ATV command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessV)
{
#ifndef _ULTRA_LOW_CODE_

    if(ATC_CMD_TYPE_EXECUTE == ATC_GET_CMD_TYPE)
    {
        uint8 cur_val;

        if(!PARAM1_FLAG)
        {
            ATC_TRACE_LOW("ATC: ATC_ProcessV EXE: cur_param_num:%d", cur_cmd_info->cur_param_num);
            atc_config_ptr->v_value = 0;
            return S_ATC_SUCCESS;
        }

        cur_val = PARA1.num;
        ATC_TRACE_LOW("ATC: ATC_ProcessV EXE: PARAM1_FLAG, %d", cur_val);

        if((0 != cur_val) && (1 != cur_val))
        {
            ATC_TRACE_LOW("ATC: ATC_ProcessV the param num is %d, invalid!", cur_val);
            return ERR_OPERATION_NOT_SUPPORTED;
        }
    }

    HandleSingleCmd(cur_cmd_info, atc_config_ptr, 1, &atc_config_ptr->v_value);
#endif /* _ULTRA_LOW_CODE_ */
    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handle the AT+CMEE command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCMEE)
{
#ifndef _ULTRA_LOW_CODE_
    HandleSingleCmd(cur_cmd_info, atc_config_ptr, 0, (uint8*)&atc_config_ptr->atc_config_nv.cmee);
    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handleS the AT+CPOF command. This command is used
//  to power off the module
//  AT+CPOF
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCPOF)
{
#ifndef _ULTRA_LOW_CODE_
    MN_DUAL_SYS_E dual_sys = ATC_CURRENT_SIM_ID_MUX;

    // power off the spreadtrum module
    MNPHONE_DeactivePsEx(dual_sys);

    ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr,
                      (uint16)APP_MN_DEACTIVE_PS_CNF, 0xff, ATC_NO_DOMAIN, dual_sys);

    // start the timer to periodic show the calling number
    SCI_SET_TIMER(P_ATC, ATC_POWEROFF_INTERVAL, ATC_POWEROFF_EXP_IND,
            &T_ATC_POWEROFF_EXP_IND, "T_ATC_POWEROFF_EXP_IND");

    return S_ATC_DEFAULT_HANDLE;

#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the AT+CREG command
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCREG)
{
#ifndef _ULTRA_LOW_CODE_
    MN_PHONE_CURRENT_PLMN_INFO_T   register_status;
    OPER_STATUS_E                  oper_status;
    ATC_STATUS                     status = S_ATC_SUCCESS;
    MN_DUAL_SYS_E                  dual_sys = ATC_CURRENT_SIM_ID_MUX;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:

            if(PARAM1_FLAG)
            {
                atc_config_ptr->atc_config_nv.creg = (uint8)PARA1.num;
            }
            else
            {
                status = ERR_INVALID_INDEX;
            }

            break;

        case ATC_CMD_TYPE_READ:
            register_status = MNPHONE_GetCurrentPLMNInfoEx(dual_sys);
            ATC_TRACE_LOW("ATC: ATC_ProcessCREG, plmn_status:%d,rat:%d,cell_cap:%d%d%d%d",
            register_status.plmn_status,
            register_status.rat,
            register_status.cell_capability.mbms_support,
            register_status.cell_capability.hsdpa_support,
            register_status.cell_capability.hsupa_support,
            register_status.cell_capability.edge_support);
            ATC_GetOperStatus(dual_sys, atc_config_ptr, &register_status, &oper_status, PNULL);
            ATC_UpCREGInfo(dual_sys, atc_config_ptr, &register_status, oper_status, FALSE);
            break;

        default:
            break;
    }

    return status;
#else
    return S_ATC_FAIL;
#endif
}

/*****************************************************************************/
//  Description : This function handle the AT+CR command
//                set if its GPRS type before show connect
//  Global resource dependence : none
//  Author:       xu.xiaogang
//  Note:
//  History:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCR)
{
#ifndef _ULTRA_LOW_CODE_
    HandleSingleCmd(cur_cmd_info, atc_config_ptr, 0, &atc_config_ptr->cr);
    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the AT+CRC command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCRC)
{
#ifndef _ULTRA_LOW_CODE_
    HandleSingleCmd(cur_cmd_info, atc_config_ptr, 0, &atc_config_ptr->crc);
    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the AT+CICB command
//  this command is used to set the type of incoming calls
//               AT+CICB=<mode>
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCICB)
{
#ifndef _ULTRA_LOW_CODE_

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:

            if(PARAM1_FLAG)
            {
                atc_config_ptr->cicb_val = PARA1.num;
            }
            else
            {
                return ERR_INVALID_INDEX;
            }

            break;

        case ATC_CMD_TYPE_READ:
            sprintf((char *)g_rsp_str, "%s: %d", g_atc_info_table[AT_CMD_CICB].cmd_name->str_ptr,
            atc_config_ptr->cicb_val);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;

        default:
            return ERR_OPERATION_NOT_ALLOWED;
    }

    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}
/*****************************************************************************/
//  Description : This function handle the AT+CSQ command
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCSQ)
{
    MN_PHONE_SCELL_MEAS_T cell_info; // the variable holds the cell information
    uint8                 rssi        = 0; // the received signal strength indication
    uint16                  rxqual_full = AT_INVALID_RXQUAL;
    MN_DUAL_SYS_E   dual_sys = ATC_CURRENT_SIM_ID_MUX;

    if (ATC_CMD_TYPE_EXECUTE != ATC_GET_CMD_TYPE)
    {
        return ERR_OPERATION_NOT_ALLOWED;
    }

    if (!MNPHONE_IsPsPowerOnEx(dual_sys))
    {
        rssi        = 0;
        rxqual_full = AT_INVALID_RXQUAL;
    }
    else
    {
        // get the RSSI information
        cell_info = MNPHONE_GetRSSIEx(dual_sys);

        rssi = atc_config_ptr->idle_rxlev[dual_sys]; //cell_info.rxlev;
                                                     //rxlev from MNPHONE_GetRSSIEx may be inaccurate

        if(!cell_info.is_traffic_mode)
        {
            rxqual_full = AT_INVALID_RXQUAL;
        }
        else
        {
            rxqual_full = cell_info.rxqual_full;
        }
    }

    sprintf((char *)g_rsp_str, "%s: %d,%d", g_atc_info_table[AT_CMD_CSQ].cmd_name->str_ptr,
              (rssi / 2), rxqual_full);
    ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
    return S_ATC_SUCCESS;
}

#ifndef ATC_SAT_ENABLE
/*****************************************************************************/
//  Description : This function handle the AT+ESQOPT command
//  Global resource dependence : g_rsp_str
//  Author:
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessESQOPT)
{
    ATC_STATUS       status = S_ATC_SUCCESS;
#ifndef _ULTRA_LOW_CODE_
    int Param2Num = -1;
    MN_DUAL_SYS_E dual_sys = ATC_CURRENT_SIM_ID_MUX;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:

            if(!PARAM1_FLAG)
            {
                ATC_TRACE_LOW("ATC: +ESQOPT no option!");
                return ERR_OPERATION_NOT_ALLOWED;
            }

            if((ATC_ESQOPT_OPTION_TYPE_E)PARA1.num == ATC_ESQOPT_STOP_INDICATION)
            {
                atc_config_ptr->esqopt_param.option = (ATC_ESQOPT_OPTION_TYPE_E)PARA1.num;

                if(SCI_IsTimerActive(atc_config_ptr->esqopt_timer))
                {
                    SCI_DeactiveTimer(atc_config_ptr->esqopt_timer);
                    ATC_TRACE_LOW("ATC: +ESQOPT set to deactivate timer while active");
                }

                return status;
            }
            else if((ATC_ESQOPT_OPTION_TYPE_E)PARA1.num == ATC_ESQOPT_TIMER_INTERVAL
            || (ATC_ESQOPT_OPTION_TYPE_E)PARA1.num == ATC_ESQOPT_RSSI_THRESHOLD)
            {
                if((!PARAM2_FLAG) || (PARAM2_TYPE != ATC_CMD_PARAM_TYPE_NUMERIC))
                {
                    ATC_TRACE_LOW("ATC: +ESQOPT valid value, use defaul value 10!");
                    Param2Num = 10;
                    //return ERR_OPERATION_NOT_ALLOWED;
                }
                else
                {
                    Param2Num = PARA2.num;
                }

                if((ATC_ESQOPT_OPTION_TYPE_E)PARA1.num == ATC_ESQOPT_TIMER_INTERVAL && Param2Num < 10)
                {
                    ATC_TRACE_LOW("ATC: +ESQOPT invalid ATC_ESQOPT_TIMER_INTERVAL!");
                    return ERR_OPERATION_NOT_SUPPORTED;
                }
                else if((ATC_ESQOPT_OPTION_TYPE_E)PARA1.num == ATC_ESQOPT_RSSI_THRESHOLD
                && (Param2Num <= 0 || Param2Num >= 63))
                {
                    ATC_TRACE_LOW("ATC: +ESQOPT invalid ATC_ESQOPT_RSSI_THRESHOLD!");
                    return ERR_OPERATION_NOT_SUPPORTED;
                }

                if(SCI_IsTimerActive(atc_config_ptr->esqopt_timer))
                {
                    SCI_DeactiveTimer(atc_config_ptr->esqopt_timer);
                    ATC_TRACE_LOW("ATC: +ESQOPT deactivate timer first while active");
                }

                atc_config_ptr->esqopt_param.option = (ATC_ESQOPT_OPTION_TYPE_E)PARA1.num;

                if(atc_config_ptr->esqopt_param.option == ATC_ESQOPT_TIMER_INTERVAL)
                {
                    atc_config_ptr->esqopt_param.timer_value = Param2Num;

                    SCI_ChangeTimer(atc_config_ptr->esqopt_timer, ATC_ProcessEsqoptTimeout, \
                    atc_config_ptr->esqopt_param.timer_value * ATC_ESQOPT_TIMER_UNIT);
                    SCI_ActiveTimer(atc_config_ptr->esqopt_timer);

                    ATC_TRACE_LOW("ATC: +ESQOPT activate timer");
                }
                else
                {
                    atc_config_ptr->esqopt_param.thres_value = Param2Num;
                }

                ATC_TRACE_LOW("ATC: +ESQOPT option=%d,timer_value=%d,thres_value=%d", \
                atc_config_ptr->esqopt_param.option, atc_config_ptr->esqopt_param.timer_value, \
                atc_config_ptr->esqopt_param.thres_value);
                {
                    MN_PHONE_SCELL_MEAS_T          rssi;

                    rssi = MNPHONE_GetRSSIEx(dual_sys);

                    ATC_TRACE_LOW("ATC: ATC_ProcessESQOPT, +ECSQ rxlev=%d,rxqual=%d", \
                    rssi.rxlev, rssi.rxqual_full);
                    sprintf((char *)g_rsp_str, "+ECSQ: %d,%d",
                    ATC_GetConvertedRSSIVlaueForECSQ(atc_config_ptr->idle_rxlev[dual_sys]), rssi.rxqual_full);
                    ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
                }
            }
            else
            {
                status = ERR_INVALID_INDEX;
            }

            break;

        case ATC_CMD_TYPE_READ:

            if(atc_config_ptr->esqopt_param.option == ATC_ESQOPT_TIMER_INTERVAL)
            {
                sprintf((char *)g_rsp_str, "%s: %d,%d",
                g_atc_info_table[AT_CMD_ESQOPT].cmd_name->str_ptr,
                atc_config_ptr->esqopt_param.option,
                (int)atc_config_ptr->esqopt_param.timer_value);
            }
            else if(atc_config_ptr->esqopt_param.option == ATC_ESQOPT_RSSI_THRESHOLD)
            {
                sprintf((char *)g_rsp_str, "%s: %d,%d",
                g_atc_info_table[AT_CMD_ESQOPT].cmd_name->str_ptr,
                atc_config_ptr->esqopt_param.option,
                (int)atc_config_ptr->esqopt_param.thres_value);
            }
            else
            {
                sprintf((char *)g_rsp_str, "%s: %d",
                g_atc_info_table[AT_CMD_ESQOPT].cmd_name->str_ptr,
                atc_config_ptr->esqopt_param.option);
            }

            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;

        default:
            ATC_TRACE_LOW("ATC: +ESQOPT no test command!");
            break;
    }

#endif/* _ULTRA_LOW_CODE_ */

    return status;
}

/*****************************************************************************/
//  Description : This function handleS the AT+SBCM command. This command is used
//  to manage battery charge.
//  AT+SBCM=<mode>,<chargeind>,<battLevMax>,<battLevMin>,<TPulseInCharge>,<TPulseOutCharge>,
//  <BattIntRes>
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSBCM)
{
#ifndef _ULTRA_LOW_CODE_
    ATC_STATUS       status = S_ATC_SUCCESS;
#ifndef _MODEM_MODE //@brand.cheng
    uint32           mode;     // holds the charge mode
    uint32           batt_vol;
    uint32           batt_state;
    uint32           batt_lev_min;
    uint32           batt_lev_max;
    uint32           time_pluse_in_charge;
    uint32           time_pluse_out_charge;
    uint32           batt_int_res;

    if(PARAM1_FLAG)
    {
        mode = PARA1.num;

        switch(mode)
        {
            case AT_STOP_CHR:
                // stop battery charge
                CHR_StopCharge();
                break;

            case AT_START_CHR:
                // start the battery charge
                CHR_StartCharge();
                break;

            case AT_GET_BATT_VOL:
                // get the battery volate
                batt_vol   = CHR_GetVoltage();
                batt_state = CHR_GetBattState();
                sprintf((char *)g_rsp_str, "+SBCI: %ld, %ld", batt_state, batt_vol);
                ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
                return S_ATC_SUCCESS;//break;

            case AT_SET_BATT_PARM:

                if(PARAM3_FLAG)
                {
                    batt_lev_max = PARA3.num;
                    CHR_SetBattLevelMax(batt_lev_max);
                }

                if(PARAM4_FLAG)
                {
                    batt_lev_min = PARA4.num;
                    CHR_SetBattLevelMin(batt_lev_min);
                }

                if(PARAM5_FLAG)
                {
                    time_pluse_in_charge = PARA5.num;
                    CHR_SetTPulseInCharge(time_pluse_in_charge);
                }

                if(PARAM6_FLAG)
                {
                    time_pluse_out_charge = PARA6.num;
                    CHR_SetTPulseOutCharge(time_pluse_out_charge);
                }

                if(PARAM7_FLAG)
                {
                    batt_int_res = PARA7.num;
                    CHR_SetBattIntRes(batt_int_res);
                }

                break;

            default:
                break;
        }

        // if parameter 2 is present then set the battery charge indicator
        if(PARAM2_FLAG)
        {
            atc_config_ptr->atc_config_nv.battery_indicator = (uint8)PARA2.num;
            // Save to the NV ram.
            ATC_WriteNVItem(atc_config_ptr, NV_ATC_CONFIG_ID);
        }
    }
    else
    {
        status = ERR_OPERATION_NOT_ALLOWED;
    }

#endif

    return status;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the AT+CBC command
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCBC)
{
#ifndef _ULTRA_LOW_CODE_
    uint32        bcs = 0;
    uint32        bcl = 0;

#ifndef _MODEM_MODE //@brand.cheng
    // call the api for the battery
    bcs = CHR_CheckBatteryStaus();

    switch(bcs)
    {
        case CHR_BATTERY_NONE_S:
            bcs = AT_CBC_NO;
            break;

        case CHR_BATTERY_NORMAL_S:
            bcs = AT_CBC_NO_CHARGE;
            break;

        case CHR_BATTERY_CHARGING_S:
            bcs = AT_CBC_CHARGE;
            break;

        default:
            ATC_TRACE_LOW("ATC: ATC_ProcessCBC bcs %d not supported", bcs);
            return ERR_OPERATION_NOT_SUPPORTED;
    }

    bcl = CHR_GetBatCapacity();
#else
    bcs = AT_CBC_NO_CHARGE;
    bcl = 100;
#endif

    sprintf((char *)g_rsp_str, "%s: %ld,%ld",
    g_atc_info_table[AT_CMD_CBC].cmd_name->str_ptr, bcs, bcl);
    ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
#endif /* _ULTRA_LOW_CODE_ */

    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handle the AT+SEQT command
//                 AT+SEQT=<value>
//                         <value>:0 - 3
//  Global resource dependence : g_rsp_str
//  Author:       Shijun.cui
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSEQT)
{
#ifdef _ULTRA_LOW_CODE_
    return S_ATC_SUCCESS;
#else
    ATC_STATUS  status = S_ATC_SUCCESS;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:

            if(!PARAM1_FLAG)
            {
                return ERR_OPERATION_NOT_SUPPORTED;
            }

            s_atc_eq_type = PARA1.num;
            break;

        case ATC_CMD_TYPE_READ:
            sprintf((char *)g_rsp_str, "%s: %d", g_atc_info_table[AT_CMD_SEQT].cmd_name->str_ptr,
            s_atc_eq_type);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;

        default:
            break;
    }

    return status;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the AT^DSCI command
//  Global resource dependence : none
//  Author:       peng.chen
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_Processxor_DSCI)
{
    ATC_STATUS         status = S_ATC_SUCCESS;
    ATC_TRACE_LOW("ATC:ATC_Processxor_DSCI");

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:

            if(!PARAM1_FLAG)
            {
                return ERR_OPERATION_NOT_ALLOWED;
            }

            if(1 == PARA1.num)
            {
                atc_config_ptr->dsci_flag = TRUE;
            }
            else
            {
                atc_config_ptr->dsci_flag = FALSE;
            }

            break;
        case ATC_CMD_TYPE_READ:
            sprintf((char *)g_rsp_str, "^DSCI: %d", atc_config_ptr->dsci_flag);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;
        case ATC_CMD_TYPE_TEST:
            sprintf((char *)g_rsp_str, "^DSCI: (0-1)");
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;
        default:
            status = ERR_OPERATION_NOT_SUPPORTED;
            break;
    }

    return status;
}

/*****************************************************************************/
//  Description : This function handleS the AT+SMUX command. This command is used
//  to active or deactive the multiplex mode(data/commands)
//  AT+SMUX=<mode>  0: deactive 1: active
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSMUX)
{
#ifndef _ULTRA_LOW_CODE_
    ATC_STATUS       status = S_ATC_SUCCESS;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:

            if(PARAM1_FLAG)
            {
                //PARA1.num define the multiplex mode
                if(PARA1.num)
                {
                    SIO_SetUserPortMode(MIX_MODE);
                }
                else
                {
                    SIO_SetUserPortMode(ATC_MODE);
                }

                atc_config_ptr->atc_config_nv.mux_mode = (BOOLEAN)(PARA1.num);
            }
            else
            {
                status = ERR_OPERATION_NOT_ALLOWED;
            }

            break;

        case ATC_CMD_TYPE_READ:

            if(MIX_MODE == SIO_GetUserPortMode())
            {
                sprintf((char *)g_rsp_str, "%s: %d", g_atc_info_table[AT_CMD_SMUX].cmd_name->str_ptr, AT_MIX_MODE);
            }
            else
            {
                sprintf((char *)g_rsp_str, "%s: %d", g_atc_info_table[AT_CMD_SMUX].cmd_name->str_ptr, AT_AT_MODE);
            }

            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;

        default:
            status = ERR_OPERATION_NOT_ALLOWED;
            break;
    }

    return status;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the ATS95 command
//  Global resource dependence : none
//  Author:       Ivan.Yin
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessS95)
{
#ifndef _ULTRA_LOW_CODE_
    if (ATC_CMD_TYPE_EXECUTE != ATC_GET_CMD_TYPE)
    {
        ATC_TRACE_LOW("ATC: ATC_ProcessS95 Command Type not match");
        return ERR_OPERATION_NOT_ALLOWED;
    }
#endif /* _ULTRA_LOW_CODE_ */
    // CALL THE sio api to set the ate parameters
    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handle the AT&K command
//  Global resource dependence : none
//  Author:       Ivan.Yin
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_Processand_K)
{
#ifndef _ULTRA_LOW_CODE_
    if (ATC_CMD_TYPE_EXECUTE != ATC_GET_CMD_TYPE)
    {
        ATC_TRACE_LOW("ATC: ATC_Processand_K Command Type not match");
        return ERR_OPERATION_NOT_ALLOWED;
    }
#endif /* _ULTRA_LOW_CODE_ */
    // ???? CALL THE sio api to set the ate parameters
    return S_ATC_SUCCESS;
}

#endif

/*****************************************************************************/
//  Description : This function handle the ATS0 command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessS0)
{
#ifndef _ULTRA_LOW_CODE_
    HandleSingleCmd(cur_cmd_info, atc_config_ptr, 0, &atc_config_ptr->s0_reg);
#endif /* _ULTRA_LOW_CODE_ */

    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handle the ATS2 command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessS2)
{
#ifndef _ULTRA_LOW_CODE_
    ATC_STATUS       status = S_ATC_SUCCESS;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:

            if(PARAM1_FLAG)
            {
                atc_config_ptr->s2_reg = (uint8)PARA1.num;
            }
            else
            {
                status = ERR_INVALID_INDEX;
            }

            break;

        default:
            status = ERR_OPERATION_NOT_SUPPORTED;
            break;
    }

    return status;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the ATS3 command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessS3)
{
#ifndef _ULTRA_LOW_CODE_
    HandleSingleCmd(cur_cmd_info, atc_config_ptr, 13, &atc_config_ptr->s3_reg);

    if(ATC_CMD_TYPE_SET == ATC_GET_CMD_TYPE)
    {
#ifdef _MUX_ENABLE_
        ATC_SetCmdLineTerminateChar(atc_config_ptr->current_link_id, atc_config_ptr->s3_reg, atc_config_ptr->s4_reg);
#else
        SIO_ATC_SetCmdLineTerminateChar(atc_config_ptr->s3_reg, atc_config_ptr->s4_reg);
#endif
    }

    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the ATS4 command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessS4)
{
#ifndef _ULTRA_LOW_CODE_
    HandleSingleCmd(cur_cmd_info, atc_config_ptr, 10, &atc_config_ptr->s4_reg);

    if(ATC_CMD_TYPE_SET == ATC_GET_CMD_TYPE)
    {
#ifdef _MUX_ENABLE_
        ATC_SetCmdLineTerminateChar(atc_config_ptr->current_link_id, atc_config_ptr->s3_reg, atc_config_ptr->s4_reg);
#else
        SIO_ATC_SetCmdLineTerminateChar(atc_config_ptr->s3_reg, atc_config_ptr->s4_reg);
#endif
    }

    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the ATS5 command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessS5)
{
#ifndef _ULTRA_LOW_CODE_
    HandleSingleCmd(cur_cmd_info, atc_config_ptr, 8, &atc_config_ptr->s5_reg);

    if(ATC_CMD_TYPE_SET == ATC_GET_CMD_TYPE)
    {
#ifdef _MUX_ENABLE_
        ATC_SetCmdLineBackSpaceChar(atc_config_ptr->current_link_id, PARA1.num);
#else
#ifndef WIN32
        SIO_ATC_SetCmdLineBackSpaceChar(PARA1.num);
#endif
#endif
    }

    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the ATS6 command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessS6)
{
#ifndef _ULTRA_LOW_CODE_
    HandleSingleCmd(cur_cmd_info, atc_config_ptr, 2, &atc_config_ptr->s6_reg);
    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the ATS7 command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessS7)
{
#ifndef _ULTRA_LOW_CODE_
    HandleSingleCmd(cur_cmd_info, atc_config_ptr, 50, &atc_config_ptr->s7_reg);
    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the ATS8 command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessS8)
{
#ifndef _ULTRA_LOW_CODE_
    HandleSingleCmd(cur_cmd_info, atc_config_ptr, 2, &atc_config_ptr->s8_reg);
    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the ATS10 command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessS10)
{
#ifndef _ULTRA_LOW_CODE_
    HandleSingleCmd(cur_cmd_info, atc_config_ptr, 2, &atc_config_ptr->s7_reg);
    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the AT+CGMI command
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCGMI)
{
#ifndef _ULTRA_LOW_CODE_
    char *manufactory_str = PNULL;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:
            // get the manufactory name
            manufactory_str = (char *)CLASSMARK_GetManufacterName();
            if (!manufactory_str)
            {
                ATC_TRACE_LOW("ATC: ATC_ProcessCGMI failed to get the manufactory name");
                return S_ATC_FAIL;
            }
            sprintf((char *)g_rsp_str, "%s", manufactory_str);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;

        case ATC_CMD_TYPE_TEST:
            break;

        default:
            return ERR_OPERATION_NOT_SUPPORTED;
    }

#endif /* _ULTRA_LOW_CODE_ */

    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handle the AT+CGSN command
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Modify:  Winnie.zhu
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCGSN)
{
#ifndef _ULTRA_LOW_CODE_
    MN_IMEI_T           imei_arr; // the imei array
    uint8               temp_str[20] = {0};
    MN_DUAL_SYS_E dual_sys =  ATC_CURRENT_SIM_ID_MUX;

#ifdef _NDS_EXT_CMD_
    uint8 rand_str[17] = {0};
    uint8 ndssign[24]  = {0};
    int ndssign_len = 24;
    int ret  = 0;
#endif
    // get the manufactory name
    MNNV_GetIMEIEx(dual_sys, imei_arr);
    ConvertBcdToDigitalStr(MN_MAX_IMEI_LENGTH, imei_arr, temp_str);

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:
        {
            sprintf((char *)g_rsp_str, "%s", temp_str);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;
        }

#ifdef _NDS_EXT_CMD_
        case ATC_CMD_TYPE_SET:
        {
            if (PARAM1_FLAG && (16 ==PARA1.str_ptr->str_len))
            {
                SCI_MEMCPY(rand_str,PARA1.str_ptr->str_ptr,PARA1.str_ptr->str_len);
                rand_str[PARA1.str_ptr->str_len] = '\0';

                ret = sarft_sign_data((unsigned char *)temp_str,strlen((char *)temp_str),rand_str,strlen((char *)rand_str),
                                    s_cid,s_gmid,s_seed,ndssign,&ndssign_len);
                SCI_TRACE_LOW("ATC: ndssign_len = %d", ndssign_len);

                if(0 != ret)
                {
                    SCI_TRACE_LOW("ATC: sarft sign data fail. ret=%d", ret);
                    return ERR_UNKNOWN;
                }

                sprintf((char*)g_rsp_str,"+CGSN: %s,\"%s\",\"%s\"",
                temp_str,rand_str,ndssign);
                ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);

            }
            else
            {
                return ERR_INVALID_INDEX;
            }
            break;
        }
#endif
        case ATC_CMD_TYPE_TEST:
        {
            break;
        }

        default:
        {
            SCI_TRACE_LOW("ATC: Assert CGSN,invalid type.");
            break;
        }
    }
    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the AT+CGMM command
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCGMM)
{
#ifndef _ULTRA_LOW_CODE_
    char                *module_str = PNULL;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:
            //if(ATC_CMD_TYPE_EXECUTE == ATC_GET_CMD_TYPE)
            {
                module_str = (char *)CLASSMARK_GetModelName();
                if (!module_str)
                {
                    ATC_TRACE_LOW("ATC: ATC_ProcessCGMI failed to get the model name");
                    return S_ATC_FAIL;
                }
                sprintf((char *)g_rsp_str, "%s", module_str);
                ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
                return S_ATC_SUCCESS;
            }

        case ATC_CMD_TYPE_TEST:
            break;

        default:
            return ERR_OPERATION_NOT_SUPPORTED;
    }

    return S_ATC_SUCCESS;
#else /* _ULTRA_LOW_CODE_ */

    return S_ATC_SUCCESS;
#endif
}

/*****************************************************************************/
//  Description : This function handle the AT+CGMR command
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCGMR)
{
#ifndef _ULTRA_LOW_CODE_
    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:
            {
    #ifndef WIN32
                uint8*    cur_str = PNULL;
                uint16    str_len = 0;

                sprintf((char*)g_rsp_str, "%s", COMMON_GetProjectVersionInfo());

                cur_str = g_rsp_str;
                while (*cur_str != '\0')
                {
                    if (*cur_str == '\n')
                    {
                        str_len = strlen((char *)g_rsp_str);
                        memmove((void *)(cur_str + 1), (void *)cur_str, strlen((char *)cur_str));
                        g_rsp_str[str_len + 1] = '\0';
                        *cur_str = '\r';
                        cur_str +=1;
                    }
                    cur_str++;
                }
    #endif
                ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
                return S_ATC_SUCCESS;
            }

        case ATC_CMD_TYPE_TEST:
            break;

        default:
            return ERR_OPERATION_NOT_SUPPORTED;
    }

    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif
}

/*****************************************************************************/
//  Description : This function handle the AT+CSCS command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCSCS)
{
#ifndef _ULTRA_LOW_CODE_
    MN_DUAL_SYS_E   dual_sys = ATC_CURRENT_SIM_ID_MUX;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:

            if(!PARAM1_FLAG)
            {
                /*default value is "IRA"*/
                atc_config_ptr->cscs[dual_sys] = ATC_CHSET_IRA;
            }
            else
            {
                uint32   i;
                BOOLEAN  is_matched = FALSE;

                for(i = ATC_CHSET_IRA; i < ATC_CHSET_MAX_NUM; i++)
                {
                    if(0 == strcmp((char *)PARA1.str_ptr->str_ptr, g_atc_charset[i]))
                    {
                        atc_config_ptr->cscs[dual_sys] = (uint8)i;
                        is_matched = TRUE;
                        break;
                    }
                }

                if(!is_matched)
                {
                    return ERR_OPERATION_NOT_SUPPORTED;
                }

            }

            break;

        case ATC_CMD_TYPE_READ:
            if (atc_config_ptr->cscs[dual_sys] >= ATC_CHSET_MAX_NUM)
            {
                ATC_TRACE_LOW("ATC: ATC_ProcessCSCS atc_config_ptr->cscs %d unknown", atc_config_ptr->cscs[dual_sys]);
                return S_ATC_FAIL;
            }
            sprintf((char *)g_rsp_str, "%s: \"%s\"",
                    g_atc_info_table[AT_CMD_CSCS].cmd_name->str_ptr,
                    g_atc_charset[atc_config_ptr->cscs[dual_sys]]);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;

        default:
            return ERR_OPERATION_NOT_SUPPORTED;
    }

#endif /* _ULTRA_LOW_CODE_ */

    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handle the AT+CPAS command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCPAS)
{
    if (ATC_GET_CMD_TYPE == ATC_CMD_TYPE_EXECUTE)
    {
        sprintf((char*)g_rsp_str, "%s: %d", g_atc_info_table[AT_CMD_CPAS].cmd_name->str_ptr,
            atc_config_ptr->cpas);
        ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);

        return S_ATC_SUCCESS;
    }
    else
    {
        return S_ATC_FAIL;
    }
}

/*****************************************************************************/
//  Description : This function handle the AT+CPIN command
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCPIN)
{
    ATC_STATUS            status    = S_ATC_SUCCESS;
#ifndef _ULTRA_LOW_CODE_
    MN_BLOCK_T            password;        // the pin
    MN_BLOCK_T            new_password;    // the new pin

    ATC_CPWD_PARAM_T      param[3];
    uint8                 param_len;
    uint8                 cmd_type;
    uint8                 *temp_ptr = PNULL;
    ERR_MNPHONE_CODE_E    err_code  = ERR_MNPHONE_NO_ERR;
    MN_DUAL_SYS_E   dual_sys = ATC_CURRENT_SIM_ID_MUX;

    SCI_MEMSET(param, 0, (3 * sizeof(ATC_CPWD_PARAM_T)));

    if(!PARAM1_FLAG || ATC_GET_CMD_TYPE != ATC_CMD_TYPE_EXECUTE)
    {
        ATC_TRACE_LOW("ATC: cpin the param is error");
        return ERR_OPERATION_NOT_ALLOWED;
    }

    temp_ptr  = PARA1.str_ptr->str_ptr;
    param_len = PARA1.str_ptr->str_len;
    ATC_GetCmdType(temp_ptr, (uint16)param_len, &cmd_type);

    switch(cmd_type)
    {
            // sends a password to ME
        case ATC_CMD_TYPE_SET:
            temp_ptr++;
            param_len--;

            if(ATC_GetParams(temp_ptr, param_len, param, 2))
            {
                if(param[0].param_len != 0)
                {
                    if(ATC_CheckPinStr(param[0].param, param[0].param_len))
                    {
                        if(param[0].param_len < AT_PSW_MIN_LEN || param[0].param_len > AT_PSW_MAX_LEN)
                        {
                            ATC_TRACE_LOW("ATC: Assert Failure pin len error.");
                            return ERR_OPERATION_NOT_ALLOWED;
                        }

                        SCI_MEMCPY(password.blockbuf, param[0].param, param[0].param_len);
                        password.blocklen = param[0].param_len;
                    }
                    else
                    {
                        return ERR_OPERATION_NOT_ALLOWED;
                    }
                }
                else
                {
                    return ERR_OPERATION_NOT_ALLOWED;
                }

                if(param[1].param_len != 0)
                {
                    if(ATC_CheckPinStr(param[1].param, param[1].param_len))
                    {
                        if(param[1].param_len < AT_PSW_MIN_LEN || param[1].param_len > AT_PSW_MAX_LEN)
                        {
                            ATC_TRACE_LOW("ATC: Assert Failure pin length error.");
                            return ERR_OPERATION_NOT_ALLOWED;
                        }

                        SCI_MEMCPY(new_password.blockbuf, param[1].param, param[1].param_len);
                        new_password.blocklen = param[1].param_len;
                    }
                    else
                    {
                        return ERR_OPERATION_NOT_ALLOWED;
                    }
                }
            }
            else
            {
                return ERR_OPERATION_NOT_ALLOWED;
            }

            ATC_TRACE_LOW("ATC:sim_flag=%d,cpin=%d,sim_ready=%d|%d",
                atc_config_ptr->sim_flag[dual_sys], atc_config_ptr->cpin[dual_sys],
                g_received_sim_ready[dual_sys], atc_config_ptr->sim_ready[dual_sys]);

            switch(atc_config_ptr->cpin[dual_sys])
            {
                case ATC_CPIN_CODE_READY:
                    if (param[0].param_len != 0 && param[1].param_len != 0)
                    {
                        err_code = MNPHONE_OperatePinEx(dual_sys, MNSIM_PIN_UNBLOCK,
                                                   MNSIM_PIN1, &password, &new_password);

                        if(err_code == ERR_MNPHONE_NO_ERR)
                        {
                            ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr,
                                              (uint16)APP_MN_SIM_PIN_FUNC_CNF, 0xff, ATC_NO_DOMAIN, dual_sys);
                            status = S_ATC_DEFAULT_HANDLE;
                        }
                        else
                        {
                            return ERR_OPERATION_NOT_SUPPORTED;
                        }
                    }
                    else
                    {
                        return ERR_OPERATION_NOT_ALLOWED;
                    }

                    break;
                    // ME is waiting SIM PIN to be given
                case ATC_CPIN_CODE_PIN1:

                    if(param[0].param_len != 0 &&
                        param[1].param_len == 0)
                    {
                    #if 1
                        err_code = MNPHONE_ResponsePinEx(dual_sys, MNSIM_PIN1, FALSE, &password, PNULL);

                        if(ERR_MNPHONE_NO_ERR == err_code)
                        {
                        #ifdef _MUX_ENABLE_
                            if(!atc_config_ptr->sim_ready[dual_sys])
                            {
                                ATC_Add_Expected_Event(atc_config_ptr,
                                                 (uint16)APP_MN_SIM_READY_IND, 0xff, ATC_NO_DOMAIN, dual_sys);
                            }
                            else
                            {
                                ATC_Add_Expected_Event(atc_config_ptr,
                                                 (uint16)APP_MN_SIM_PIN_FUNC_CNF, 0xff, ATC_NO_DOMAIN, dual_sys);
                            }
                        #endif
                            status = S_ATC_DEFAULT_HANDLE;
                        }
                        else
                        {
                            ATC_TRACE_LOW("ATC: ATC_ProcessCPIN, PIN1,failure in MNPHONE_ResponsePin");
                            return ERR_OPERATION_NOT_ALLOWED;
                        }
                    #else
                        if(!atc_config_ptr->sim_ready[dual_sys])
                        {
                            err_code = MNPHONE_ResponsePinEx(dual_sys, MNSIM_PIN1, FALSE, &password, PNULL);
                            if (ERR_MNPHONE_NO_ERR == err_code)
                            {
                                ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr,
                                            (uint16)APP_MN_SIM_READY_IND, 0xff, ATC_NO_DOMAIN, dual_sys);
                            }
                            else
                            {
                                ATC_TRACE_LOW("ATC: ATC_ProcessCPIN, PIN1, failure in MNPHONE_ResponsePinEx");
                                return ERR_OPERATION_NOT_ALLOWED;
                            }
                        }
                        else
                        {
                            err_code = MNPHONE_OperatePinEx(dual_sys, MNSIM_PIN_VERIFY, MNSIM_PIN1, &password, PNULL);
                            if (ERR_MNPHONE_NO_ERR == err_code)
                            {
                                ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr,
                                            (uint16)APP_MN_SIM_PIN_FUNC_CNF, 0xff, ATC_NO_DOMAIN, dual_sys);
                            }
                            else
                            {
                                ATC_TRACE_LOW("ATC: ATC_ProcessCPIN, PIN1, failure in MNPHONE_OperatePinEx");
                                return ERR_OPERATION_NOT_ALLOWED;
                            }
                        }
                        #endif
                        status = S_ATC_DEFAULT_HANDLE;
                    }
                    else
                    {
                        //when emergency, change pin with puk
                        if((!g_received_sim_ready[dual_sys]) &&
                            (param[0].param_len != 0) &&
                            (param[1].param_len != 0))
                        {
                            err_code = MNPHONE_ResponsePinEx(dual_sys, MNSIM_PIN1, FALSE, &new_password, &password);

                            if(ERR_MNPHONE_NO_ERR == err_code)
                            {
                                ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr,
                                                  (uint16)APP_MN_SIM_READY_IND, 0xff, ATC_NO_DOMAIN, dual_sys);
                                status = S_ATC_DEFAULT_HANDLE;
                            }
                            else
                            {
                                ATC_TRACE_LOW("ATC: ATC_ProcessCPIN, sim_flag:TRUE,PUK1, failure in MNPHONE_ResponsePin");
                                return ERR_OPERATION_NOT_ALLOWED;
                            }
                        }
                        else if (param[0].param_len != 0 && param[1].param_len != 0)
                        {
                            err_code = MNPHONE_OperatePinEx(dual_sys, MNSIM_PIN_UNBLOCK,
                                                       MNSIM_PIN1, &password, &new_password);

                            if(err_code == ERR_MNPHONE_NO_ERR)
                            {
                                ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr,
                                                  (uint16)APP_MN_SIM_PIN_FUNC_CNF, 0xff, ATC_NO_DOMAIN, dual_sys);
                                status = S_ATC_DEFAULT_HANDLE;
                            }
                            else
                            {
                                return ERR_OPERATION_NOT_SUPPORTED;
                            }
                        }
                        else
                        {
                            return ERR_OPERATION_NOT_ALLOWED;
                        }

                    }

                    break;

                    // ME is waiting SIM PIN2 to be given
                case ATC_CPIN_CODE_PIN2:
        #if 0    //  ECPIN2 command processing PIN2
                    if(param[0].param_len != 0 &&
                        param[1].param_len == 0)
                    {
                        if(!atc_config_ptr->sim_flag[dual_sys])
                        {
                            err_code = MNPHONE_ResponsePinEx(dual_sys, MNSIM_PIN2, FALSE, &password, PNULL);

                            if(ERR_MNPHONE_NO_ERR == err_code)
                            {
                                ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr,
                                                  (uint16)APP_MN_SIM_PIN_FUNC_CNF, 0xff, ATC_NO_DOMAIN, dual_sys);
                                status = S_ATC_DEFAULT_HANDLE;
                            }
                            else
                            {
                                ATC_TRACE_LOW("ATC: ATC_ProcessCPIN, PIN2, failure in MNPHONE_ResponsePin");
                                return ERR_OPERATION_NOT_ALLOWED;
                            }
                        }
                        else
                        {
                            err_code = MNPHONE_OperatePinEx(dual_sys, MNSIM_PIN_VERIFY, MNSIM_PIN2, &password, PNULL);

                            if(err_code == ERR_MNPHONE_NO_ERR)
                            {
                                ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr,
                                                  (uint16)APP_MN_SIM_PIN_FUNC_CNF, 0xff, ATC_NO_DOMAIN, dual_sys);
                                status = S_ATC_DEFAULT_HANDLE;
                            }
                            else
                            {
                                return ERR_OPERATION_NOT_SUPPORTED;
                            }
                        }
                    }
                    else
        #endif
                    {
                        return ERR_OPERATION_NOT_SUPPORTED;
                    }

                    //break;

                    // ME is waiting for SIM PUK1 to be given
                case ATC_CPIN_CODE_PUK1:

                    if(param[0].param_len != 0 &&
                        param[1].param_len != 0)
                    {
                        if(!atc_config_ptr->sim_flag[dual_sys])
                        {
                            err_code = MNPHONE_ResponsePinEx(dual_sys, MNSIM_PIN1, TRUE, &new_password, &password);

                            if(ERR_MNPHONE_NO_ERR == err_code)
                            {
                                ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr,
                                                  (uint16)APP_MN_SIM_READY_IND, 0xff, ATC_NO_DOMAIN, dual_sys);
                                status = S_ATC_DEFAULT_HANDLE;
                            }
                            else
                            {
                                ATC_TRACE_LOW("ATC: ATC_ProcessCPIN, sim_flag:TRUE, PUK1, failure in MNPHONE_ResponsePin");
                                return ERR_OPERATION_NOT_ALLOWED;
                            }
                        }
                        else
                        {
                            if(!g_received_sim_ready[dual_sys])
                            {
                                err_code = MNPHONE_ResponsePinEx(dual_sys, MNSIM_PIN1, TRUE, &new_password, &password);

                                if(ERR_MNPHONE_NO_ERR == err_code)
                                {
                                    ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr,
                                                      (uint16)APP_MN_SIM_READY_IND, 0xff, ATC_NO_DOMAIN, dual_sys);
                                    status = S_ATC_DEFAULT_HANDLE;
                                }
                                else
                                {
                                    ATC_TRACE_LOW("ATC: ATC_ProcessCPIN, sim_flag:TRUE,PUK1, failure in MNPHONE_ResponsePin");
                                    return ERR_OPERATION_NOT_ALLOWED;
                                }
                            }
                            else
                            {
                                err_code = MNPHONE_OperatePinEx(dual_sys, MNSIM_PIN_UNBLOCK, MNSIM_PIN1, &password, &new_password);

                                if(err_code == ERR_MNPHONE_NO_ERR)
                                {
                                    ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr,
                                                      (uint16)APP_MN_SIM_PIN_FUNC_CNF, 0xff, ATC_NO_DOMAIN, dual_sys);
                                    status = S_ATC_DEFAULT_HANDLE;
                                }
                                else
                                {
                                    return ERR_OPERATION_NOT_SUPPORTED;
                                }
                            }
                        }
                    }
                    else
                    {
                        return ERR_OPERATION_NOT_SUPPORTED;
                    }

                    break;

                    // ME is waiting SIM PUK2 to be given
                case ATC_CPIN_CODE_PUK2:
            #if 0    //  ECPIN2 command processing PIN2
                    if(param[0].param_len != 0 &&
                    param[1].param_len != 0)
                    {
                        if(!atc_config_ptr->sim_flag[dual_sys])
                        {
                            err_code = MNPHONE_ResponsePinEx(dual_sys, MNSIM_PIN2, TRUE, &new_password, &password);

                            if(ERR_MNPHONE_NO_ERR == err_code)
                            {
                                ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr,
                                                  (uint16)APP_MN_SIM_PIN_FUNC_CNF, 0xff, ATC_NO_DOMAIN, dual_sys);
                                status = S_ATC_DEFAULT_HANDLE;
                            }
                            else
                            {
                                ATC_TRACE_LOW("ATC: ATC_ProcessCPIN, PUK2, failure in MNPHONE_ResponsePin");
                                return ERR_OPERATION_NOT_ALLOWED;
                            }
                        }
                        else
                        {
                            err_code = MNPHONE_OperatePinEx(dual_sys, MNSIM_PIN_UNBLOCK, MNSIM_PIN2, &password, &new_password);

                            if(err_code == ERR_MNPHONE_NO_ERR)
                            {
                                ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr,
                                                  (uint16)APP_MN_SIM_PIN_FUNC_CNF, 0xff, ATC_NO_DOMAIN, dual_sys);
                                status = S_ATC_DEFAULT_HANDLE;
                            }
                            else
                            {
                                return ERR_OPERATION_NOT_SUPPORTED;
                            }
                        }
                    }
                    else
                #endif
                    {
                        return ERR_OPERATION_NOT_SUPPORTED;
                    }

                    //break;

                    // The SIM card is blocked for the user
                case ATC_CPIN_CODE_BLOCK:
                    return ERR_SIM_PUK_REQUIRED;

                default:
                    ATC_TRACE_LOW("ATC: Assert Failure unknow cpin state %d", atc_config_ptr->cpin[dual_sys]);
                    return ERR_OPERATION_NOT_SUPPORTED;
            }

            atc_config_ptr->cpin_flag[dual_sys] = TRUE;
            status = S_ATC_DEFAULT_HANDLE;
            break;

        case ATC_CMD_TYPE_READ:
            if(ATC_CPIN_CODE_MAX <= atc_config_ptr->cpin[dual_sys] &&
               atc_config_ptr->sim_err_reason[dual_sys] != MNSIM_ERROR_INSERTED_ERROR &&
               atc_config_ptr->sim_err_reason[dual_sys] != MNSIM_ERROR_REMOVED)
            {
                //At system power on, if no sim inserted, at should response ERR_SIM_NOT_INSERTED,
                //other error code indicates sim initialize not ready or sim error
                return ERR_SIM_BUSY;
            }

            if(atc_config_ptr->sim_flag[dual_sys] == TRUE)
            {
                sprintf((char *)g_rsp_str, g_atc_cpin_rsp[atc_config_ptr->cpin[dual_sys]].str);
            }
            else
            {
                return ERR_SIM_NOT_INSERTED;
            }

            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;

        case ATC_CMD_TYPE_TEST:
            break;

        default:
            return ERR_OPERATION_NOT_ALLOWED;
    }

#endif /* _ULTRA_LOW_CODE_ */

    return status;

}

#ifndef ATC_SAT_ENABLE
/*****************************************************************************/
//  Description : This function handle the AT+ECPIN2 command
//  Global resource dependence : none
//  Author:       Bin Li
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessECPIN2)
{
    ATC_STATUS                  status = S_ATC_SUCCESS;
#ifndef _ULTRA_LOW_CODE_
    MN_BLOCK_T                 password;        // the pin
    MN_BLOCK_T                  new_password;    // the new pin
    ERR_MNPHONE_CODE_E  err_code;
    MN_DUAL_SYS_E   dual_sys = ATC_CURRENT_SIM_ID_MUX;

    ATC_CPWD_PARAM_T      param[3];
    uint8                 param_len;
    uint8                 cmd_type;
    uint8                 *temp_ptr = PNULL;

    SCI_MEMSET(param, 0, (3 * sizeof(ATC_CPWD_PARAM_T)));
    if(!PARAM1_FLAG || ATC_CMD_PARAM_TYPE_STRING != PARAM1_TYPE)
    {
        ATC_TRACE_LOW("ATC: ecpin2 the param is error");
        return ERR_OPERATION_NOT_ALLOWED;
    }

    temp_ptr  = PARA1.str_ptr->str_ptr;
    param_len = PARA1.str_ptr->str_len;
    ATC_GetCmdType(temp_ptr, (uint16)param_len, &cmd_type);

    switch(cmd_type)
    {
            // sends a password to ME
        case ATC_CMD_TYPE_SET:
            temp_ptr++;
            param_len--;

            if(ATC_GetParams(temp_ptr, param_len, param, 2))
            {
                if(param[0].param_len != 0)
                {
                    if(ATC_CheckPinStr(param[0].param, param[0].param_len))
                    {
                        if(param[0].param_len < AT_PSW_MIN_LEN || param[0].param_len > AT_PSW_MAX_LEN)
                        {
                            ATC_TRACE_LOW("ATC: Assert Failure pin len error.");
                            return ERR_OPERATION_NOT_ALLOWED;
                        }

                        SCI_MEMCPY(password.blockbuf, param[0].param, param[0].param_len);
                        password.blocklen = param[0].param_len;
                    }
                    else
                    {
                        return ERR_OPERATION_NOT_ALLOWED;
                    }
                }
                else
                {
                    return ERR_OPERATION_NOT_ALLOWED;
                }

                if(param[1].param_len != 0)
                {
                    if(ATC_CheckPinStr(param[1].param, param[1].param_len))
                    {
                        if(param[1].param_len < AT_PSW_MIN_LEN || param[1].param_len > AT_PSW_MAX_LEN)
                        {
                            ATC_TRACE_LOW("ATC: Assert Failure pin length error.");
                            return ERR_OPERATION_NOT_ALLOWED;
                        }

                        SCI_MEMCPY(new_password.blockbuf, param[1].param, param[1].param_len);
                        new_password.blocklen = param[1].param_len;
                    }
                    else
                    {
                        return ERR_OPERATION_NOT_ALLOWED;
                    }
                }
            }
            else
            {
                return ERR_OPERATION_NOT_ALLOWED;
            }

            switch(atc_config_ptr->cpin2[dual_sys])
            {
                    // ME is not pending for any password
                case ATC_CPIN_CODE_READY:
                    ATC_TRACE_LOW("ATC_ProcessECPIN2  while sim ready!");

                    if((param[0].param_len != 0) && (param[1].param_len == 0))
                    {
                        err_code = MNPHONE_OperatePinEx(dual_sys, MNSIM_PIN_VERIFY, MNSIM_PIN2, &password, PNULL);

                        if(ERR_MNPHONE_NO_ERR == err_code)
                        {
                            ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr,
                                              (uint16)APP_MN_SIM_PIN_FUNC_CNF, 0xff, ATC_NO_DOMAIN, dual_sys);
                            status = S_ATC_DEFAULT_HANDLE;

                            break;
                        }
                        else
                        {
                            ATC_TRACE_LOW("ATC: ATC_ProcessCPIN, PIN2, failure in MNPHONE_OperatePin");
                            return ERR_OPERATION_NOT_ALLOWED;
                        }
                    }

                    if((param[0].param_len != 0) && (param[1].param_len != 0))
                    {
                        err_code = MNPHONE_OperatePinEx(dual_sys, MNSIM_PIN_UNBLOCK, MNSIM_PIN2, &password, &new_password);

                        if(err_code == ERR_MNPHONE_NO_ERR)
                        {
                            ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr,
                                              (uint16)APP_MN_SIM_PIN_FUNC_CNF, 0xff, ATC_NO_DOMAIN, dual_sys);
                            status = S_ATC_DEFAULT_HANDLE;
                        }
                        else
                        {
                            return ERR_OPERATION_NOT_SUPPORTED;
                        }
                    }
                    else
                    {
                        return ERR_OPERATION_NOT_SUPPORTED;
                    }

                    break;

                case ATC_CPIN_CODE_PIN1:
                case ATC_CPIN_CODE_PUK1:
                    return ERR_OPERATION_NOT_ALLOWED;

                    // ME is waiting SIM PIN2 to be given
                case ATC_CPIN_CODE_PIN2:

                    if((param[0].param_len != 0) && (param[1].param_len == 0))
                    {
                        err_code = MNPHONE_OperatePinEx(dual_sys, MNSIM_PIN_VERIFY, MNSIM_PIN2, &password, PNULL);

                        if(ERR_MNPHONE_NO_ERR == err_code)
                        {
                            ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr,
                                              (uint16)APP_MN_SIM_PIN_FUNC_CNF, 0xff, ATC_NO_DOMAIN, dual_sys);
                            status = S_ATC_DEFAULT_HANDLE;
                        }
                        else
                        {
                            ATC_TRACE_LOW("ATC: ATC_ProcessCPIN, PIN2, failure in MNPHONE_ResponsePin");
                            return ERR_OPERATION_NOT_ALLOWED;
                        }

                    }
                    else if((param[0].param_len != 0) && (param[1].param_len != 0))
                    {
                        err_code = MNPHONE_OperatePinEx(dual_sys, MNSIM_PIN_CHANGE, MNSIM_PIN2, &password, &new_password);

                        if(err_code == ERR_MNPHONE_NO_ERR)
                        {
                            ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr,
                                              (uint16)APP_MN_SIM_PIN_FUNC_CNF, 0xff, ATC_NO_DOMAIN, dual_sys);
                            status = S_ATC_DEFAULT_HANDLE;
                        }
                        else
                        {
                            return ERR_OPERATION_NOT_SUPPORTED;
                        }
                    }
                    else
                    {
                        return ERR_OPERATION_NOT_SUPPORTED;
                    }

                    break;

                    // ME is waiting SIM PUK2 to be given
                case ATC_CPIN_CODE_PUK2:

                    if((param[0].param_len != 0)  && (param[1].param_len != 0))
                    {
                        err_code = MNPHONE_OperatePinEx(dual_sys, MNSIM_PIN_UNBLOCK, MNSIM_PIN2, &password, &new_password);

                        if(err_code == ERR_MNPHONE_NO_ERR)
                        {
                            ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr,
                                              (uint16)APP_MN_SIM_PIN_FUNC_CNF, 0xff, ATC_NO_DOMAIN, dual_sys);
                            status = S_ATC_DEFAULT_HANDLE;
                        }
                        else
                        {
                            return ERR_OPERATION_NOT_SUPPORTED;
                        }
                    }
                    else
                    {
                        return ERR_OPERATION_NOT_SUPPORTED;
                    }

                    break;

                    // The PUK2 blocked, FDN function forbidden forever
                case ATC_CPIN_CODE_BLOCK:
                    return ERR_SIM_PUK2_BLOCKED;

                default:
                    ATC_TRACE_LOW("ATC: Assert Failure unknow ecpin2 state %d", atc_config_ptr->cpin2[dual_sys]);
                    return ERR_OPERATION_NOT_SUPPORTED;

            }

            atc_config_ptr->cpin_flag[dual_sys] = TRUE;
            status = S_ATC_DEFAULT_HANDLE;
            break;

        case ATC_CMD_TYPE_READ:
            if(ATC_CPIN_CODE_MAX <= atc_config_ptr->cpin2[dual_sys] &&
               atc_config_ptr->sim_err_reason[dual_sys] != MNSIM_ERROR_INSERTED_ERROR &&
               atc_config_ptr->sim_err_reason[dual_sys] != MNSIM_ERROR_REMOVED)
            {
                //At system power on, if no sim inserted, at should response ERR_SIM_NOT_INSERTED,
                //other error code indicates sim initialize not ready or sim error
                return ERR_OPERATION_NOT_ALLOWED;
            }

            if(atc_config_ptr->sim_flag[dual_sys] == TRUE)
            {
                sprintf((char *)g_rsp_str, g_atc_ecpin2_rsp[atc_config_ptr->cpin2[dual_sys]].str);
            }
            else
            {
                return ERR_SIM_NOT_INSERTED;
            }

            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;

        case ATC_CMD_TYPE_TEST:
            break;

        default:
            return ERR_OPERATION_NOT_ALLOWED;
    }

#endif /* _ULTRA_LOW_CODE_ */

    return status;
}

AT_CMD_FUNC(ATC_ProcessX)
{
    uint8 param_num = 0;

    if (ATC_CMD_TYPE_EXECUTE != ATC_GET_CMD_TYPE)
    {
        SCI_TRACE_LOW("ATC: ATC_ProcessE Command Type not match");
        return ERR_OPERATION_NOT_ALLOWED;
    }

    param_num = *(PARA1.str_ptr->str_ptr) - '0';

    SCI_TRACE_LOW("ATC: para1=%d,", param_num);

    if(4 == param_num)
    {
        //atc_config_ptr->connect_result_code_mode = ATC_CONNECT_WITH_BAUD;
    }

    return S_ATC_SUCCESS;
}

#endif

/*****************************************************************************/
//  Description : This function handle the AT+XX command
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessXX)
{
    ATC_STATUS          status = S_ATC_SUCCESS;
#ifndef _ULTRA_LOW_CODE_
    uint8               pin_type;
    uint16              remain_times;
    MN_DUAL_SYS_E   dual_sys = ATC_CURRENT_SIM_ID_MUX;

    if(PARAM1_FLAG)
    {
        if(atc_config_ptr->sim_flag[dual_sys])
        {
            pin_type = (uint8)PARA1.num;

            switch(pin_type)
            {
                case ATC_PIN1:
                    remain_times = MNSIM_GetPinFalseRemainEx(dual_sys, MNSIM_PIN1);
                    break;
                case ATC_PIN2:
                    remain_times = MNSIM_GetPinFalseRemainEx(dual_sys, MNSIM_PIN2);
                    break;
                case ATC_PUK1:
                    remain_times = MNSIM_GetPinOrPukFalseRemainEx(dual_sys, MNSIM_PIN1, TRUE);
                    break;
                case ATC_PUK2:
                    remain_times = MNSIM_GetPinOrPukFalseRemainEx(dual_sys, MNSIM_PIN2, TRUE);
                    break;
                default:
                    ATC_TRACE_LOW("ATC: unknow pin type %d.", pin_type);
                    return ERR_INVALID_INDEX;
            }

            sprintf((char *)g_rsp_str, "%s: %d",
            g_atc_info_table[AT_CMD_XX].cmd_name->str_ptr, remain_times);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
        }
        else
        {
            status = ERR_SIM_NOT_INSERTED;
        }
    }
    else
    {
        status = ERR_INVALID_INDEX;
    }

#endif /* _ULTRA_LOW_CODE_ */
    return status;
}


/*****************************************************************************/
//  Description : This function handles the AT+CIMI command. This command is used
//                to get the IMSI of the SIM card.
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCIMI)
{
    ATC_STATUS         status = S_ATC_SUCCESS;
#ifndef _ULTRA_LOW_CODE_
    MN_IMSI_T          imsi_str;
    uint8              temp_str[20];
    MN_DUAL_SYS_E   dual_sys = ATC_CURRENT_SIM_ID_MUX;


    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:

            if(atc_config_ptr->sim_flag[dual_sys])
            {
                // get the IMSI identity
                imsi_str = MNSIM_GetImsiEx(dual_sys);

                if(MN_MAX_IMSI_ARR_LEN >= imsi_str.imsi_len)
                {
                    ConvertBcdToDigitalStr(imsi_str.imsi_len, imsi_str.imsi_val, temp_str);

                    /* <IMSI>: International Mobile Subscriber Identity
                     * (string without double quotes) */
                    sprintf((char *)g_rsp_str, "%s", temp_str);

                    ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
                }
                else
                {
                    status = S_ATC_FAIL;
                }
            }
            else
            {
                if (g_received_sim_ready[dual_sys])
                {
                    switch(atc_config_ptr->sim_err_reason[dual_sys])
                    {
                        case MNSIM_ERROR_REMOVED:
                            status = ERR_SIM_NOT_INSERTED;
                            break;

                        case MNSIM_ERROR_SERVICE_NOT_AVAILABLE:
                            status = ERR_SIM_CARD_SERVICE_NOT_AVAILABLE;
                            break;

                        case MNSIM_ERROR_PIN1_UNINITIALIZED:
                        case MNSIM_ERROR_PIN2_UNINITIALIZED:
                            status = ERR_SIM_CARD_PIN_UNINITIALIZED;
                            break;

                        case MNSIM_ERROR_PIN1_BLOCKED:
                        case MNSIM_ERROR_PIN2_BLOCKED:
                            status = ERR_SIM_CARD_PIN_BLOCKED;
                            break;

                        case MNSIM_ERROR_PIN1_UNBLOCK_BLOCKED:
                        case MNSIM_ERROR_PIN2_UNBLOCK_BLOCKED:
                            status = ERR_SIM_CARD_PUK_BLOCKED;
                            break;

                        case MNSIM_ERROR_REJECT_BY_NET:
                            status = ERR_SIM_CARD_REJ_BY_NET;
                            break;

                        case MNSIM_ERROR_POWER_OFF:
                        case MNSIM_ERROR_NO_SECRETCODE_INITIALIZED:
                        case MNSIM_ERROR_INVALID_STATUS_RSP:
                        case MNSIM_ERROR_INSERTED_ERROR:
                        case MNSIM_ERROR_INITIALISE_FAILED:
                        case MNSIM_ERROR_MEMORY_PROBLEM:
                        case MNSIM_ERROR_INVALID_PARAMS:
                        case MNSIM_ERROR_FILE_INVALIDATED:
                        case MNSIM_ERROR_FILE_NOT_FOUND:
                        case MNSIM_ERROR_GENERAL_FAULT:
                        case MNSIM_ERROR_ALLOC_ERROR:
                        case MNSIM_ERROR_SM_FAULT:
                        case MNSIM_ERROR_SERIOUS:
                        case MNSIM_ERROR_UNKNOWN:
                        case MNSIM_ERROR_NULL:
                            status = ERR_SIM_FAILURE;
                            break;

                        default :
                            status = S_ATC_FAIL;
                            break;
                    }
                }
            }

            break;

        case ATC_CMD_TYPE_TEST:
            break;

        default:
            return ERR_OPERATION_NOT_SUPPORTED;
    }

#endif /* _ULTRA_LOW_CODE_ */

    return status;
}
/*****************************************************************************/
//  Description : This function handles the data of CCID
//  Global resource dependence : g_rsp_str
//  Author:       hanjun.liu
//  Note:
/*****************************************************************************/
LOCAL void HandleCcidData(
    uint8                *p_ccid,        //the input data
    uint8                *p_result  //the output data
)
{
    uint8              temp_val;
    int32              i;
    int32              j = 0;

    for(i = 0; i < MNSIM_ICCID_ID_NUM_LEN; i++)
    {
        // get the low 4 bits
        temp_val = (p_ccid[i] & 0x0f);

        if(temp_val >= 0x0A)
        {
            p_result[j] = (temp_val - 0x0A) + 'A';
        }
        else
        {
            p_result[j] = (temp_val) + '0' ;
        }

        j++;

        // get the high 4 bits
        temp_val = (p_ccid[i] & 0xf0) >> 4;

        if(temp_val >= 0x0A)
        {
            p_result[j] = (temp_val - 0x0A) + 'A';
        }
        else
        {
            p_result[j] = (temp_val) + '0' ;
        }

        j++;
    }
}

/*****************************************************************************/
//  Description : This function handles the AT+CCID command. This command is used
//  to read the EF-CCID file from SIM card.
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCCID)
{
#ifndef _ULTRA_LOW_CODE_
    MNSIM_ICCID_T      ccid;
    uint8              temp_str[2*MNSIM_ICCID_ID_NUM_LEN + 1];
    ATC_STATUS         status = S_ATC_SUCCESS;
    MN_DUAL_SYS_E   dual_sys = ATC_CURRENT_SIM_ID_MUX;

    SCI_MEMSET(temp_str, 0 , sizeof(temp_str));

    if(atc_config_ptr->sim_flag[dual_sys])
    {
        if(ATC_CMD_TYPE_EXECUTE == ATC_GET_CMD_TYPE || ATC_CMD_TYPE_READ == ATC_GET_CMD_TYPE)
        {
            MNSIM_GetICCIDEx(dual_sys, &ccid);
            HandleCcidData(ccid.id_num, temp_str);
            sprintf((char *)g_rsp_str, "%s: \"%s\"",
            g_atc_info_table[AT_CMD_CCID].cmd_name->str_ptr, temp_str);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
        }
        else
        {
            status = ERR_OPERATION_NOT_ALLOWED;
        }
    }
    else
    {
        if (g_received_sim_ready[dual_sys])
        {
            switch(atc_config_ptr->sim_err_reason[dual_sys])
            {
                case MNSIM_ERROR_REMOVED:
                    status = ERR_SIM_NOT_INSERTED;
                    break;

                case MNSIM_ERROR_SERVICE_NOT_AVAILABLE:
                    status = ERR_SIM_CARD_SERVICE_NOT_AVAILABLE;
                    break;

                case MNSIM_ERROR_PIN1_UNINITIALIZED:
                case MNSIM_ERROR_PIN2_UNINITIALIZED:
                    status = ERR_SIM_CARD_PIN_UNINITIALIZED;
                    break;

                case MNSIM_ERROR_PIN1_BLOCKED:
                case MNSIM_ERROR_PIN2_BLOCKED:
                    status = ERR_SIM_CARD_PIN_BLOCKED;
                    break;

                case MNSIM_ERROR_PIN1_UNBLOCK_BLOCKED:
                case MNSIM_ERROR_PIN2_UNBLOCK_BLOCKED:
                    status = ERR_SIM_CARD_PUK_BLOCKED;
                    break;

                case MNSIM_ERROR_REJECT_BY_NET:
                    status = ERR_SIM_CARD_REJ_BY_NET;
                    break;

                case MNSIM_ERROR_POWER_OFF:
                case MNSIM_ERROR_NO_SECRETCODE_INITIALIZED:
                case MNSIM_ERROR_INVALID_STATUS_RSP:
                case MNSIM_ERROR_INSERTED_ERROR:
                case MNSIM_ERROR_INITIALISE_FAILED:
                case MNSIM_ERROR_MEMORY_PROBLEM:
                case MNSIM_ERROR_INVALID_PARAMS:
                case MNSIM_ERROR_FILE_INVALIDATED:
                case MNSIM_ERROR_FILE_NOT_FOUND:
                case MNSIM_ERROR_GENERAL_FAULT:
                case MNSIM_ERROR_ALLOC_ERROR:
                case MNSIM_ERROR_SM_FAULT:
                case MNSIM_ERROR_SERIOUS:
                case MNSIM_ERROR_UNKNOWN:
                case MNSIM_ERROR_NULL:
                    status = ERR_SIM_FAILURE;
                    break;

                default :
                    status = S_ATC_FAIL;
                    break;
            }
        }
    }

    return status;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the APP_MN_SYNC_IND signal
//  Global resource dependence : none
//  Author:       hanjun.liu
//  Note:
/*****************************************************************************/
ATC_STATUS ATC_ProcessMnSyncInd(
    ATC_CONFIG_T         *atc_config_ptr,
    APP_MN_SYNC_IND_T    *sig_ptr
)
{
    MN_DUAL_SYS_E   dual_sys    = sig_ptr->dual_sys;
    uint8           call_index  = 0;

    for(call_index = 0; call_index < MN_CALL_MAX_CALL_NUM; call_index++)
    {
        if(ATC_CALL_OUTGOING_STATE == g_calls_state[dual_sys].call_context[call_index].call_entity_state ||
           ATC_CALL_ALERTING_STATE == g_calls_state[dual_sys].call_context[call_index].call_entity_state)
        {
            break;
        }
    }

    if(call_index >= MN_CALL_MAX_CALL_NUM)
    {
        ATC_TRACE_LOW("ATC: receive the APP_MN_SYNC_IND signal when no call in outgoing state !!!");
    }
    else
    {
        sprintf((char *)g_rsp_str, "^CONF: %d", g_calls_state[dual_sys].call_context[call_index].call_id + 1);
        ATC_BuildUnsolicitedInfo(dual_sys, atc_config_ptr, (uint8 *)g_rsp_str);
    }


    ATC_TRACE_LOW("ATC: receive the APP_MN_SYNC_IND signal");

    if(MN_CALL_SYNC_RESOURCE_ASSIGNMENT == ((APP_MN_SYNC_IND_T *)sig_ptr)->sync_cause)
    {
        if( (ATC_CALL_OUTGOING_STATE == g_calls_state[dual_sys].call_context[0].call_entity_state) ||
            (ATC_CALL_ALERTING_STATE == g_calls_state[dual_sys].call_context[0].call_entity_state))
        {
            if(g_atc_call_flag)
            {
                if(g_calls_state[dual_sys].call_context[g_calls_state[dual_sys].call_amount - 1].is_emergency_call)
                {
                    ATC_TRACE_LOW("ATC: ATC_ProcessMnSyncInd is_emergency_call:%d, call_amount: %d",
                                 1,
                                 g_calls_state[dual_sys].call_amount - 1
                                 );

                    sprintf((char *)g_rsp_str, "%s: 9", g_atc_info_table[AT_CMD_SIND].cmd_name->str_ptr);

                    ATC_BuildUnsolicitedInfo(dual_sys, atc_config_ptr, (uint8 *)g_rsp_str);
                }
                if(ATC_VOICE_MODE == g_calls_state[dual_sys].call_context[0].call_mode)
                {
                    ATC_HandleMoMuteAndLocalRingState(dual_sys, APP_MN_SYNC_IND, 0);
                }
           }
       }
       else if(ATC_CALL_INCOMING_STATE == g_calls_state[dual_sys].call_context[0].call_entity_state)
       {
            s_mt_need_open_codec_flag |= CC_TCH_RESOURCE_ASSIGNED ;

            if((s_mt_need_open_codec_flag ==  (CC_MT_CALL_CONNECTING|CC_TCH_RESOURCE_ASSIGNED)) &&
                (!atc_config_ptr->audio_flag))
            {
                AUDIO_SetDevMode((AUDIO_DEVICE_MODE_TYPE_E)(atc_config_ptr->audio_mode));
                AUDIO_SetVolume(atc_config_ptr->speaker_volume);
                ATC_EnableUplinkMute(FALSE);
                ATC_EnableRemoteMute(FALSE);

                ATC_EnableVoiceCodec(TRUE);
                atc_config_ptr->audio_flag = TRUE;
                s_mt_need_open_codec_flag = 0;
            }
        }
    }

    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handle the APP_MN_NETWORK_STATUS_IND signal and
//                send the unsolicited result code
//  Global resource dependence : none
//  Author:
//  Note:
/*****************************************************************************/
ATC_STATUS ATC_ProcessNetworkStatusInd(
    ATC_CONFIG_T                  *atc_config_ptr,
    APP_MN_NETWORK_STATUS_IND_T   *sig_ptr
)
{
    MN_PHONE_CURRENT_PLMN_INFO_T register_status;
    OPER_STATUS_E                oper_status = UNKNOWN;
    OPER_STATUS_E                gprs_oper_status = UNKNOWN;

    //print BASE version information.
    SCI_TRACE_LOW("ATC: Version: BASE_%s", SCI_GetATVersionInfo());

    if(!sig_ptr->plmn_is_selecting)
    {

        register_status = ConvertNWStatusToPLMNInfo(sig_ptr);

        ATC_TRACE_LOW("ATC: ATC_ProcessNetworkStatusInd, plmn_status:%d,rat:%d,cell_cap:%d%d%d%d",
                                   register_status.plmn_status,
                                   register_status.rat/*,
                                   register_status.cell_capability.mbms_support,
                                   register_status.cell_capability.hsdpa_support,
                                   register_status.cell_capability.hsupa_support,
                                   register_status.cell_capability.edge_support*/);

        switch(sig_ptr->plmn_status)
        {
            case PLMN_EMERGENCY_GPRS_ONLY:
                if(MANUAL_DEREGISTER == atc_config_ptr->cops[sig_ptr->dual_sys].mode)
                {
                    return S_ATC_DEFAULT_HANDLE;
                }

                g_is_ps_act[sig_ptr->dual_sys] = TRUE;
                break;

            case PLMN_NORMAL_GSM_ONLY:
            case PLMN_NORMAL_GSM_GPRS_BOTH:

                /* Start Synchronize Service, and check condition in case of
                 * more "APP_MN_NETWORK_STATUS_IND" signal's incoming. */
                if(ATC_SYNCHRONIZESERVICE_STAT_NULL == atc_config_ptr->atc_sync_srv.status)
                {
                    ATC_SynchronizeService(sig_ptr->dual_sys);
                }

                g_is_ps_act[sig_ptr->dual_sys] = TRUE;
                break;
            case PLMN_NO_SERVICE:
                atc_config_ptr->idle_rxlev[sig_ptr->dual_sys] = 0;

                #ifdef ATC_SAT_ENABLE
                SCI_MEMSET((void *)g_rxlev_arr,0,sizeof(g_rxlev_arr));
                #endif

                break;
            default:
                break;
        }

        /*Unsolicited Area*/
        ATC_GetOperStatus(sig_ptr->dual_sys, atc_config_ptr, &register_status, &oper_status, &gprs_oper_status);

        //Send the unsolicited +SIND
        if(sig_ptr->plmn_status == PLMN_NO_SERVICE)
        {
            //network lost
            ATC_UpSINDInfo(sig_ptr->dual_sys,
                atc_config_ptr,
                PNULL,
                ATC_WIND_BIT7,
                8,
                (BOOLEAN)(0 != atc_config_ptr->atc_config_nv.creg));
        }

        if((atc_config_ptr->atc_config_nv.spauto_flag & 0x04) && (sig_ptr->dual_sys == 0))
        {
            SCI_TRACE_LOW("ATC: set auto test flag for IQ cather.");
            gprs_oper_status = REGISTERED_HOME_PLMN;
        }

        //Send the unsolicited +CGREG
        ATC_UpCGREGInfo(sig_ptr->dual_sys, atc_config_ptr, &register_status, gprs_oper_status, TRUE);
        //Send the unsolicited +CREG
        ATC_UpCREGInfo(sig_ptr->dual_sys, atc_config_ptr, &register_status, oper_status, TRUE);

        if((oper_status != s_prev_oper_status) || (gprs_oper_status != s_prev_gprs_oper_status))
        {
            if(oper_status != s_prev_oper_status)
            {
                //Send the unsolicited +SIND
                switch(oper_status)
                {
                    case NOT_REGISTERED_NOT_SEARCHING:
                    case NOT_REGISTERED_AND_SEARCHING:
                    case REGISTRATION_DENIED:
                        //network register failure
                        ATC_UpSINDInfo(sig_ptr->dual_sys,
                                       atc_config_ptr,
                                       PNULL,
                                       ATC_WIND_BIT6,
                                       7,
                                       (BOOLEAN)(0 != atc_config_ptr->atc_config_nv.creg));
                        break;
                    case REGISTERED_ROAMING:
                    case REGISTERED_HOME_PLMN:
                        //network register success
                        ATC_UpSINDInfo(sig_ptr->dual_sys,
                                       atc_config_ptr,
                                       PNULL,
                                       ATC_WIND_BIT6,
                                       11,
                                       (BOOLEAN)(0 != atc_config_ptr->atc_config_nv.creg));
                        break;
                    default:
                        break;
                }

                //send the unsolicited +ECIND when oper_status changed
                ATC_UpECINDInfo(sig_ptr->dual_sys, atc_config_ptr, (xSignalHeaderRec *)sig_ptr, 3, 3, 0, 0, 0);
            }

            //Send the unsolicited  ^SYSINFO when cs or ps status changed
            if(0 != atc_config_ptr->atc_config_nv.creg)
            {
                ATC_UpSYSINFOInfo(sig_ptr->dual_sys, atc_config_ptr, TRUE);
            }

            ATC_UpSNWSTATInfo(atc_config_ptr, sig_ptr);
        }

        s_prev_oper_status = oper_status;
        s_prev_gprs_oper_status = gprs_oper_status;
        /*End of Unsolicited Area..................*/
    #ifndef ATC_SAT_ENABLE
        if(atc_config_ptr->psntrg_flag)
        {
            BuildPSNTRGResponse(sig_ptr->dual_sys, atc_config_ptr, &register_status);
        }
    #endif
    }

#if 0
    event_class = MNRSF_GetEventId(g_current_card_id);
    if((NW_CS_AUTH_REJECT_CLASS == event_class) || (NW_PS_AUTH_REJECT_CLASS == event_class))
    {
        ATC_UpSPERRORInfo(sig_ptr->dual_sys, atc_config_ptr, (ATC_ERROR_ITEM_CLASS)event_class, sig_ptr->mn_mm_cause);
    }
    if((sig_ptr->mn_mm_cause>=MN_MM_IMSI_UNKNOWN_IN_HLR) && (sig_ptr->mn_mm_cause<=MN_MM_MSG_NOT_COMPAT_WITH_PROTO_STATE))
    {
        event_class = MNRSF_GetEventId(g_current_card_id);
        if((LOCATION_UPDATE_FAIL_CLASS == event_class) || (GPRS_ATTACH_FAIL_CLASS == event_class) || (RAU_COMRAU_FAIL_CLASS == event_class))
        {
            ATC_UpSPERRORInfo(sig_ptr->dual_sys, atc_config_ptr, (ATC_ERROR_ITEM_CLASS)event_class, sig_ptr->mn_mm_cause);
        }
    }
#endif

    return S_ATC_DEFAULT_HANDLE;
}

/*****************************************************************************/
//  Description : unsolicited service info which is supported by modem
//  Global resource dependence : none
//  Author: xiaonian.li
//  Note:
/*****************************************************************************/
LOCAL void ATC_UpSNWSTATInfo(
    ATC_CONFIG_T                *atc_config_ptr,
    APP_MN_NETWORK_STATUS_IND_T *sig_ptr
    )
{
    BOOLEAN              is_emc_allowed = MNPHONE_GetScellEmcSupportCapEx(sig_ptr->dual_sys);
    ATC_SERVICE_STATE_E  srv_state      = ATC_SEVICE_NONE;

    switch(sig_ptr->plmn_status)
    {
        case PLMN_NO_SERVICE:
            // not support all service
            break;

        case PLMN_EMERGENCY_ONLY:
        case PLMN_EMERGENCY_SIM_INVALID:
            srv_state = (TRUE == is_emc_allowed) ? ATC_SERVICE_EMC : ATC_SEVICE_NONE;
            break;

        case PLMN_NORMAL_GSM_ONLY:
            srv_state = (TRUE == is_emc_allowed) ? ATC_SERVICE_CS_EMC : ATC_SERVICE_CS;
            break;

        case PLMN_NORMAL_GPRS_ONLY:
        case PLMN_EMERGENCY_GPRS_ONLY:
        case PLMN_EMERGENCY_SIM_INVALID_GPRS_ONLY:
            srv_state = (TRUE == is_emc_allowed) ? ATC_SERVICE_PS_EMC : ATC_SERVICE_PS;
            break;

        case PLMN_NORMAL_GSM_GPRS_BOTH:
            srv_state = (TRUE == is_emc_allowed) ? ATC_SERVICE_CS_PS_EMC : ATC_SERVICE_CS_PS;
            break;

        default:
            return;
    }
    sprintf((char*)g_rsp_str, "^SNWSTAT: %d",srv_state);
    ATC_BuildUnsolicitedInfo(sig_ptr->dual_sys, atc_config_ptr, g_rsp_str);
}

/*****************************************************************************/
//  Description : This function handle the APP_MN_NETWORK_INFO_IND signal and
//                send the unsolicited result code
//  Global resource dependence : none
//  Author:       Tianming Wu
//  Note:
/*****************************************************************************/
ATC_STATUS ATC_ProcessNetworkInfoInd(
    ATC_CONFIG_T                  *atc_config_ptr,
    APP_MN_NETWORK_INFO_IND_T     *sig_ptr
)
{
    if(TRUE == atc_config_ptr->tz_report_flag)
    {
        if((TRUE == sig_ptr->time_and_timezone.ut_lt_zone_flag) ||
           (TRUE == sig_ptr->time_and_timezone.local_time_zone_flag) ||
           (TRUE == sig_ptr->time_and_timezone.daylight_saving_time_flag))
        {
            /* pattern for time and time zone report: +CTZV: "yy/mm/dd,hh:mm:ss(+/-)tz,dt" */
            if(TRUE == sig_ptr->time_and_timezone.ut_lt_zone_flag)
            {
                uint8  year      = sig_ptr->time_and_timezone.ut_lt_zone.year;
                uint8  month     = sig_ptr->time_and_timezone.ut_lt_zone.month;
                uint8  day       = sig_ptr->time_and_timezone.ut_lt_zone.day;
                uint8  hour      = sig_ptr->time_and_timezone.ut_lt_zone.hour;
                uint8  minute    = sig_ptr->time_and_timezone.ut_lt_zone.minute;
                uint8  second    = sig_ptr->time_and_timezone.ut_lt_zone.second;
                int16  time_zone = sig_ptr->time_and_timezone.ut_lt_zone.time_zone;

                sprintf((char*)g_rsp_str, "+CTZV: \"%02d/%02d/%02d,%02d:%02d:%02d", year, month, day, hour, minute, second);

                if(time_zone >= 0)
                {
                    sprintf((char*)(g_rsp_str + strlen((char *)g_rsp_str)), "+%d,", time_zone);
                }
                else
                {
                    sprintf((char*)(g_rsp_str + strlen((char *)g_rsp_str)), "-%d,", (0 - time_zone));
                }
            }
            else
            {
                sprintf((char*)g_rsp_str, "+CTZV: \"//,::");

                if(TRUE == sig_ptr->time_and_timezone.local_time_zone_flag)
                {
                    int8 local_time_zone = sig_ptr->time_and_timezone.local_time_zone;

                    if(local_time_zone >= 0)
                    {
                        sprintf((char*)(g_rsp_str + strlen((char *)g_rsp_str)), "+%d,", local_time_zone);
                    }
                    else
                    {
                        sprintf((char*)(g_rsp_str + strlen((char *)g_rsp_str)), "-%d,", (0 - local_time_zone));
                    }
                }
                else
                {
                    sprintf((char*)(g_rsp_str + strlen((char *)g_rsp_str)), "+,");
                }
            }

            if(TRUE == sig_ptr->time_and_timezone.daylight_saving_time_flag)
            {
                uint8   daylight_saving_time = sig_ptr->time_and_timezone.daylight_saving_time;

                sprintf((char*)(g_rsp_str + strlen((char *)g_rsp_str)), "%d\"", daylight_saving_time);
            }
            else
            {
                sprintf((char*)(g_rsp_str + strlen((char *)g_rsp_str)), "\"");
            }

            ATC_BuildUnsolicitedInfo(sig_ptr->dual_sys, atc_config_ptr, (uint8*)g_rsp_str);
        }
    }

    //to update current plmn info
    g_cur_plmn_info[sig_ptr->dual_sys].cur_plmn = sig_ptr->cur_plmn;
    g_cur_plmn_info[sig_ptr->dual_sys].full_name = sig_ptr->full_name;
    g_cur_plmn_info[sig_ptr->dual_sys].short_name = sig_ptr->short_name;

    return S_ATC_DEFAULT_HANDLE;
}

/*****************************************************************************/
//  Description : This function handles the AT command which has one parameter
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
LOCAL void HandleSingleCmd(
    ATC_CUR_INFO_LIST_T  *cur_cmd_ptr,   // point to the atc cmd info
    ATC_CONFIG_T         *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    uint8                default_val,     // the default value
    uint8                *cur_val         // the current value
)
{
    switch(cur_cmd_ptr->cmd_type & 0X0F)
    {
            // Set command AT+XXX=...
        case ATC_CMD_TYPE_SET:
            //@Ivan.Yin for ATV0 etc
        case ATC_CMD_TYPE_EXECUTE:

            if(cur_cmd_ptr->param_used[0])
            {
                *cur_val = (uint8)(cur_cmd_ptr->cur_param_list_ptr->param_info.num);
            }
            else
            {
                *cur_val = default_val;
            }

            break;

            // Read command AT+XXX?
        case ATC_CMD_TYPE_READ:

            // Determine if the command is a extension command
            if('+' != g_atc_info_table[cur_cmd_ptr->cmd_index].cmd_name->str_ptr[0])
            {
                sprintf((char *)g_rsp_str, "%d", *cur_val);
            }
            else
            {
                sprintf((char *)g_rsp_str, "%s: %d",
                g_atc_info_table[cur_cmd_ptr->cmd_index].cmd_name->str_ptr, *cur_val);
            }

            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;

        default:
            ATC_TRACE_LOW("ATC: HandleSingleCmd Command Type not supported");
            break;
    }
}

/*****************************************************************************/
//  Description : This function handle the AT command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessAT)
{
    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handle the ATE command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessE)
{
    ATC_STATUS         status = S_ATC_SUCCESS;

    if (ATC_CMD_TYPE_EXECUTE != ATC_GET_CMD_TYPE)
    {
        ATC_TRACE_LOW("ATC: ATC_ProcessE Command Type not match");
        return ERR_OPERATION_NOT_ALLOWED;
    }

    if(!PARAM1_FLAG)
    {
        atc_config_ptr->atc_config_nv.e_value = 0;
    }
    else
    {
        atc_config_ptr->atc_config_nv.e_value = PARA1.num;
    }

    if(0 == atc_config_ptr->atc_config_nv.e_value)
    {
#ifdef _MUX_ENABLE_

        if(atc_config_ptr->current_link_id == ATC_DEBUG_LINK)
        {
            SIO_ATC_SetEcho(FALSE);
        }
        else
        {
            ATC_SetEcho(FALSE);
        }

#else
        SIO_ATC_SetEcho(FALSE);
#endif
    }
    else if(1 == atc_config_ptr->atc_config_nv.e_value)
    {
        atc_config_ptr->atc_config_nv.e_value = TRUE;
#ifdef _MUX_ENABLE_

        if(atc_config_ptr->current_link_id == ATC_DEBUG_LINK)
        {
            SIO_ATC_SetEcho(FALSE);
        }
        else
        {
            ATC_SetEcho(TRUE);
        }

#else
        SIO_ATC_SetEcho(TRUE);
#endif
    }
    else
    {
        return ERR_OPERATION_NOT_SUPPORTED;
    }

    return status;
}

/*****************************************************************************/
//  Description : This function handles the ATZ command
//  ATZ is used to restore the default value configuration and release the current
//  calls
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessZ)
{
    ATC_STATUS  status  = S_ATC_DEFAULT_HANDLE;
#ifndef _ULTRA_LOW_CODE_
    MN_CALL_DISCONNECT_CALL_CAUSE_E  cause = MN_CAUSE_NORMAL_CLEARING;
    ATC_CALLS_STATE_T * calls_state_ptr = PNULL;
    MN_DUAL_SYS_E   dual_sys = ATC_CURRENT_SIM_ID_MUX;
    uint32      i = 0;

    calls_state_ptr = &g_calls_state[dual_sys];
    if(1 <= calls_state_ptr->call_amount)
    {
        atc_config_ptr->ath_flag = TRUE;

        for(i = 0; i < calls_state_ptr->call_amount; i++)
        {
            if (ATC_CALL_INCOMING_STATE == calls_state_ptr->call_context[i].call_entity_state)
            {
                cause = MN_CAUSE_USER_BUSY;
            }
            MNCALL_DisconnectCallEx(dual_sys, calls_state_ptr->call_context[i].call_id, cause, PNULL);

        #ifdef _MUX_ENABLE_
            ATC_Clear_All_Exp_Result(calls_state_ptr->call_context[i].call_id, ATC_DOMAIN_CS, dual_sys);
            ATC_Add_Expected_Event(atc_config_ptr, APP_MN_CALL_DISCONNECTED_IND,
                            calls_state_ptr->call_context[i].call_id, ATC_DOMAIN_CS, dual_sys);
        #endif
        }
    }
    else
    {
        status = S_ATC_SUCCESS;
    }

    RestoreConfig(atc_config_ptr, dual_sys);
#endif /* _ULTRA_LOW_CODE_ */
    return status;
}

/*****************************************************************************/
//  Description : This function handle the AT&F command
//  Global resource dependence : none
//  Author:       Ivan.Yin
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_Processand_F)
{
#ifdef _MUX_ENABLE_
    uint8 i;
#endif
    MN_DUAL_SYS_E   dual_sys = ATC_CURRENT_SIM_ID_MUX;


#ifndef _ULTRA_LOW_CODE_
    if (ATC_CMD_TYPE_EXECUTE != ATC_GET_CMD_TYPE)
    {
        ATC_TRACE_LOW("ATC: ATC_Processand_F Command Type not match");
        return ERR_OPERATION_NOT_ALLOWED;
    }
#endif /* _ULTRA_LOW_CODE_ */

    if(PARAM1_FLAG)
    {
        if(0 == PARA1.num)
        {
            RestoreConfig(atc_config_ptr, dual_sys);

#ifdef _MUX_ENABLE_
            ATC_SetEcho(atc_config_ptr->atc_config_nv.e_value);

            for(i = 0; i < ATC_MAX_LINK_ID; i++)
            {
                ATC_SetCmdLineTerminateChar(i, DEF_S3, 0);
            }

            SIO_ATC_SetEcho(atc_config_ptr->atc_config_nv.e_value);
            SIO_ATC_SetCmdLineTerminateChar(DEF_S3, 0);
#else
            SIO_ATC_SetEcho(atc_config_ptr->atc_config_nv.e_value);
            SIO_ATC_SetCmdLineTerminateChar(DEF_S3, 0);
#endif
        }
        else
        {
            return ERR_OPERATION_NOT_SUPPORTED;
        }
    }
    else
    {
        return ERR_OPERATION_NOT_SUPPORTED;
    }

    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handle the AT&D command
//  Global resource dependence : none
//  Author:       Ivan.Yin
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_Processand_D)
{
#ifndef _ULTRA_LOW_CODE_
    if (ATC_CMD_TYPE_EXECUTE != ATC_GET_CMD_TYPE)
    {
        ATC_TRACE_LOW("ATC: ATC_Processand_D Command Type not match");
        return ERR_OPERATION_NOT_ALLOWED;
    }
#endif /* _ULTRA_LOW_CODE_ */
    // ???? CALL THE sio api to set the ate parameters
    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handle the AT&C command
//  Global resource dependence : none
//  Author:       Ivan.Yin
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_Processand_C)
{
#ifndef _ULTRA_LOW_CODE_
    if (ATC_CMD_TYPE_EXECUTE != ATC_GET_CMD_TYPE)
    {
        ATC_TRACE_LOW("ATC: ATC_Processand_C Command Type not match");
        return ERR_OPERATION_NOT_ALLOWED;
    }
#endif /* _ULTRA_LOW_CODE_ */
    // ???? CALL THE sio api to set the ate parameters
    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handle the ATW command
//  Global resource dependence : none
//  Author:       Ivan.Yin
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessW)
{
#ifndef _ULTRA_LOW_CODE_
    if (ATC_CMD_TYPE_EXECUTE != ATC_GET_CMD_TYPE)
    {
        ATC_TRACE_LOW("ATC: ATC_ProcessW Command Type not match");
        return ERR_OPERATION_NOT_ALLOWED;
    }
#endif /* _ULTRA_LOW_CODE_ */
    // ???? CALL THE sio api to set the ate parameters
    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handle the AT+CCLK command
//  Global resource dependence : none
//  Author:       Xueli.lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCCLK)
{
    uint32         status = S_ATC_SUCCESS;
#ifndef _ULTRA_LOW_CODE_
#if !defined(CHIP_VER_6810) && !defined(CHIP_VER_6820)
    SCI_DATE_T   cur_date;
    SCI_TIME_T   cur_time;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:

            if(PARAM1_FLAG)
            {
                if(PARAM1_TYPE != ATC_CMD_PARAM_TYPE_STRING)
                {
                    ATC_TRACE_LOW("ATC: ATC_ProcessCPBW, param1 is not string!");
                    status = S_ATC_FAIL;
                    break;
                }


                if(CheckTimeStr(PARA1.str_ptr->str_len, PARA1.str_ptr->str_ptr, &cur_date, &cur_time))
                {
                    if((ERR_TM_NONE == TM_SetSysDate(cur_date)) && (ERR_TM_NONE == TM_SetSysTime(cur_time)))
                    {
                        ATC_TRACE_LOW("ATC: $$ set timer success");
                        break;
                    }
                    else
                    {
                        ATC_TRACE_LOW("ATC: set timer fail");
                        status = S_ATC_FAIL;
                        break;
                    }
                }
                else
                {
                    ATC_TRACE_LOW("ATC: the parameter check fail");
                    status = ERR_INVALID_INDEX;
                    break;
                }
            }
            else
            {
                status = ERR_INVALID_INDEX;
                break;
            }

            //break;

        case ATC_CMD_TYPE_READ:
            TM_GetSysDate(&cur_date);
            TM_GetSysTime(&cur_time);
            sprintf((char *)g_rsp_str, "%s: \"%02d/%02d/%02d,%02d:%02d:%02d\"",
            g_atc_info_table[AT_CMD_CCLK].cmd_name->str_ptr,
            (cur_date.year % 100), cur_date.mon, cur_date.mday,
            cur_time.hour, cur_time.min, cur_time.sec);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;

        default:
            status = ERR_OPERATION_NOT_SUPPORTED;
            break;
    }

    ATC_TRACE_LOW("ATC: the status is %d", status);
#endif
#endif /* _ULTRA_LOW_CODE_ */

    return status;

}
/*****************************************************************************/
//  Description : This function handle the AT+VGR command
//                 AT+VGR=<value>
//                        <value>:0 -- 255
//  Global resource dependence : none
//  Author:       Xueli.lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessVGR)
{
    ATC_STATUS  status = S_ATC_SUCCESS;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:

            if(PARAM1_FLAG)
            {
                if(PARA1.num <= AT_AUD_VOL_SPEAKER_MAX)
                {
                    atc_config_ptr->speaker_volume = PARA1.num;
                }
                else
                {
                    status = ERR_INVALID_INDEX;
                    break;
                }

#ifdef AUDIO_PARAM_ARM
                AUDIO_SetVolumeAndTypeEx(
                    INVALID_HANDLE,
                    atc_config_ptr->speaker_volume,
                    AUDIO_ARM_APP_TYPE_0);
#else
                AUDIO_SetVolume(atc_config_ptr->speaker_volume);
#endif
                ATC_EnableRemoteMute(s_audio_status.is_remote_mute);
                ATC_EnableUplinkMute(s_audio_status.is_mute);
            }
            else
            {
                status = ERR_INVALID_INDEX;
            }

            break;

        case ATC_CMD_TYPE_READ:
            sprintf((char *)g_rsp_str, "%s: %d", g_atc_info_table[AT_CMD_VGR].cmd_name->str_ptr,
            atc_config_ptr->speaker_volume);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;

        default:
            status = ERR_OPERATION_NOT_SUPPORTED;
            break;
    }

    return status;
}

/*****************************************************************************/
//  Description : This function handle the AT+VGT command
//  Global resource dependence : none
//  Author:       Xueli.lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessVGT)
{
    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handle the AT+SPEAKER command
//                 AT+SVGT=<value>,<value>
//                        <value>:0 -- 1
//  Global resource dependence : g_rsp_str
//  Author:       Shijun.cui
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSPEAKER)
{
    ATC_STATUS  status = S_ATC_SUCCESS;
    uint32      result;

#ifndef _ULTRA_LOW_CODE_

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:

            if(PARAM1_FLAG)
            {
                s_b_mic_aux_device = PARA1.num;
            }

            if(PARAM2_FLAG)
            {
                s_b_spk_aux_device = PARA2.num;
            }

            if(!PARAM1_FLAG && !PARAM2_FLAG)
            {
                status = ERR_INVALID_INDEX;
            }

            // audio should provide setting mic/aux device and speaker pri/aux interfaces!
            result = LAYER1_SwitchSoundDevice(s_b_mic_aux_device, s_b_spk_aux_device, (AUDIO_DEVICE_MODE_TYPE_E)(atc_config_ptr->audio_mode));

            if(result != SCI_SUCCESS)
            {
                return ERR_OPERATION_NOT_SUPPORTED;
            }

            break;

        case ATC_CMD_TYPE_READ:
            sprintf((char *)g_rsp_str, "%s: %d,%d", g_atc_info_table[AT_CMD_SPEAKER].cmd_name->str_ptr,
            s_b_mic_aux_device, s_b_spk_aux_device);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;

        default:
            break;
    }

#endif /* _ULTRA_LOW_CODE_ */

    return status;
}


/*****************************************************************************/
//  Description : This function handle the AT+SSEA command
//                 AT+SVGR=<value>
//                        <value>:0 -- 8
//  Global resource dependence : none
//  Author:
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSSEA)
{
    ATC_STATUS     status = S_ATC_DEFAULT_HANDLE;

    if(PARAM1_FLAG)
    {
        s_aud_dev_mod = (AUDIO_DEVICE_MODE_TYPE_E)PARA1.num;
        // output the '>'
        ATC_SendAsyncInfo(atc_config_ptr, ATC_CTRLZ_SSEA);
    }
    else
    {
        status = ERR_INVALID_INDEX;
    }

    return status;

}
/*****************************************************************************/
//  Description : This function handles the data send from the SIO when AT+SSEA
//  Global resource dependence : g_rsp_str
//  Author:       hanjun.liu
//  Note:
/*****************************************************************************/
PUBLIC uint32 ATC_ProcessSioSSEAData(
    ATC_CONFIG_T    *atc_config_ptr,
    uint8           *data_ptr,  // pointer to the data
    uint16            param_len
)
{
    uint8       terminater_char;     // indicates ther terminater char
    BOOLEAN     flag = FALSE;
    uint32      i;
    AUDIO_TEST_DATA_T audio_data;

    SCI_ASSERT(PNULL != atc_config_ptr);/*assert verified: check null pointer*/
    SCI_ASSERT(PNULL != data_ptr);/*assert verified: check null pointer*/

    terminater_char      = data_ptr[param_len - 1];
    data_ptr[param_len - 1] = '\0';
    param_len             -= 1;
    SCI_MEMSET(&audio_data, 0, sizeof(AUDIO_TEST_DATA_T));
    ATC_TRACE_LOW("ATC:param_len = %d", param_len);

    switch(terminater_char)
    {
            // cancel the operation
        case ESC:
            flag = TRUE;
            break;

        case CTRL_Z:

            //Change to bin format.
            if((sizeof(AUDIO_TEST_DATA_T) * 2) == param_len)
            {
                for(i = 0; i < 33; i++)
                {
                    ConvertInt16ToBin(data_ptr, (uint16 *)(&(audio_data.UL_equalizer_coeff[i])));
                    ATC_TRACE_LOW("ATC:AT+SSEA,audio_data.UL_equalizer_coeff[%d]=%x", i, audio_data.UL_equalizer_coeff[i]);
                    data_ptr = data_ptr + 4;
                }

                for(i = 0; i < 33; i++)
                {
                    ConvertInt16ToBin(data_ptr, (uint16 *)(&(audio_data.DL_equalizer_coeff[i])));
                    ATC_TRACE_LOW("ATC:AT+SSEA,audio_data.DL_equalizer_coeff[%d]=%x", i, audio_data.DL_equalizer_coeff[i]);
                    data_ptr = data_ptr + 4;
                }

                ATC_TRACE_LOW("ATC:AT+SSEA,audio_data.DL_equalizer_coeff=%x,%x,%x", *audio_data.DL_equalizer_coeff,
                *(audio_data.DL_equalizer_coeff + 1), *(audio_data.DL_equalizer_coeff + 2));
                ConvertInt16ToBin(data_ptr, (uint16 *)(&(audio_data.UL_PGA_gain)));
                //ATC_TRACE_LOW("ATC:AT+SSEA,audio_data.UL_PGA_gain=%x", audio_data.UL_PGA_gain);
                data_ptr = data_ptr + 4;
                ConvertInt16ToBin(data_ptr, (uint16 *)(&(audio_data.DL_PGA_gain)));
                //ATC_TRACE_LOW("ATC:AT+SSEA,audio_data.DL_PGA_gain=%x", audio_data.DL_PGA_gain);
                data_ptr = data_ptr + 4;
                ConvertInt16ToBin(data_ptr, (uint16 *)(&(audio_data.UL_digital_gain)));
                //ATC_TRACE_LOW("ATC:AT+SSEA,audio_data.UL_digital_gain=%x", audio_data.UL_digital_gain);
                data_ptr = data_ptr + 4;
                ConvertInt16ToBin(data_ptr, (uint16 *)(&(audio_data.DL_digital_gain)));
                //ATC_TRACE_LOW("ATC:AT+SSEA,audio_data.UL_digital_gain=%x", audio_data.UL_digital_gain);
                data_ptr = data_ptr + 4;
                ConvertInt16ToBin(data_ptr, (uint16 *)(&(audio_data.UL_digital_scale)));
                //ATC_TRACE_LOW("ATC:AT+SSEA,audio_data.UL_digital_scale=%x", audio_data.UL_digital_scale);
                data_ptr = data_ptr + 4;
                ConvertInt16ToBin(data_ptr, (uint16 *)(&(audio_data.DL_digital_scale)));
                //ATC_TRACE_LOW("ATC:AT+SSEA,audio_data.DL_digital_scale=%x", audio_data.DL_digital_scale);
                data_ptr = data_ptr + 4;
                ConvertInt16ToBin(data_ptr, (uint16 *)(&(audio_data.midi_PGA_gain_base)));
                //ATC_TRACE_LOW("ATC:AT+SSEA,audio_data.midi_PGA_gain_base=%x", audio_data.midi_PGA_gain_base);
                data_ptr = data_ptr + 4;
                ConvertInt16ToBin(data_ptr, (uint16 *)(&(audio_data.digital_sidetone_gain)));
                //ATC_TRACE_LOW("ATC:AT+SSEA,audio_data.digital_sidetone_gain=%x", audio_data.digital_sidetone_gain);
                data_ptr = data_ptr + 4;

                for(i = 0; i < 6; i++)
                {
                    ConvertInt16ToBin(data_ptr, (uint16 *)(&(audio_data.DL_IIR1_coeff[i])));
                    ATC_TRACE_LOW("ATC:AT+SSEA,audio_data.DL_IIR1_coeff[%d]=%x", i, audio_data.DL_IIR1_coeff[i]);
                    data_ptr = data_ptr + 4;
                }

                for(i = 0; i < 6; i++)
                {
                    ConvertInt16ToBin(data_ptr, (uint16 *)(&(audio_data.DL_IIR2_coeff[i])));
                    ATC_TRACE_LOW("ATC:AT+SSEA,audio_data.DL_IIR2_coeff[%d]=%x", i, audio_data.DL_IIR2_coeff[i]);
                    data_ptr = data_ptr + 4;
                }

                flag = TRUE;
                // call the REF api to write set the audio
                AUD_SetEngineeringParameter(s_aud_dev_mod, audio_data);
            }

            break;

        default:
            break;
    }

    ATC_TRACE_LOW("ATC:flag = %d", flag);

    if(!flag)
    {
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_ERROR , ERR_UNKNOWN);
    }
    else
    {
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
    }

    return S_ATC_DEFAULT_HANDLE;
}

/*****************************************************************************/
//  Description : This function handle the AT+CIND command
//  Global resource dependence : none
//  Author:       Xueli.lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCIND)
{
#ifndef _ULTRA_LOW_CODE_
    uint8                battery_lev;
    uint8                service_flag;
    uint8                sounder;
    uint8                call_flag;
    uint8                roam_flag;
    uint8                sms_full_flag;
#ifndef _MODEM_MODE
    uint32               capacity = 0;
#endif
    BOOLEAN              message_flag;
    BOOLEAN              sim_message_flag;
    MN_SMS_RECORD_ID_T   sim_total_amount;
    MN_SMS_RECORD_ID_T   sim_used_amount;
    MN_SMS_RECORD_ID_T   me_total_amount;
    MN_SMS_RECORD_ID_T   me_used_amount;
    BOOLEAN              me_message_flag;
    MN_DUAL_SYS_E   dual_sys = ATC_CURRENT_SIM_ID_MUX;

    MN_PHONE_CURRENT_PLMN_INFO_T register_status;

    if(g_calls_state[dual_sys].call_amount > 0)
    {
        call_flag = TRUE;
    }
    else
    {
        call_flag = FALSE;
    }

    sounder = atc_config_ptr->cmut_flag;
    register_status = MNPHONE_GetCurrentPLMNInfoEx(dual_sys);

    switch(register_status.plmn_status)
    {
        case PLMN_NO_SERVICE:
        case PLMN_EMERGENCY_ONLY:
        case PLMN_EMERGENCY_SIM_INVALID:
            service_flag = 0;
            break;
        default:
            service_flag = 1;
            break;
    }

    roam_flag = register_status.plmn_is_roaming;
#ifndef _MODEM_MODE
    //get the battery level
    capacity = CHR_GetBatCapacity(); //the return value is 0,20,40,60,80,100

    ATC_TRACE_LOW("ATC: bat capacity %d.", capacity);
    battery_lev = capacity / 20;
#else
    battery_lev = 5;
#endif

    //Get the SMS information
    GetStorageInfo(dual_sys, MN_SMS_STORAGE_SIM, &sim_total_amount, &sim_used_amount, &sim_message_flag);
    GetStorageInfo(dual_sys, MN_SMS_STORAGE_ME, &me_total_amount, &me_used_amount, &me_message_flag);

    if((sim_total_amount == sim_used_amount)
    && (me_total_amount == me_used_amount)
    && (sim_total_amount != 0)
    && (me_total_amount != 0))
    {
        sms_full_flag = 0; /*full*/
    }
    else
    {
        sms_full_flag = 1;/*not full*/
    }

    message_flag = sim_message_flag || me_message_flag;
    sprintf((char *)g_rsp_str, "%s: %d,%d,%d,%d,%d,%d,%d,%d",
    g_atc_info_table[AT_CMD_CIND].cmd_name->str_ptr, battery_lev,
    ATC_GetConvertedRSSIVlaueForCSQ(atc_config_ptr->idle_rxlev[dual_sys]), service_flag, sounder, message_flag,
    call_flag, roam_flag, sms_full_flag);
    ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
#endif /* _ULTRA_LOW_CODE_ */

    return S_ATC_SUCCESS;
}
/*****************************************************************************/
//  Description : This function handles the data of CCID
//  Global resource dependence : g_rsp_str
//  Author:       hanjun.liu
//  Note:
/*****************************************************************************/
#ifndef _ULTRA_LOW_CODE_
LOCAL void HandleCcedOneShot(
    MN_DUAL_SYS_E   dual_sys,
    ATC_CONFIG_T    *atc_config_ptr
)
{
    MN_PHONE_CURRENT_PLMN_INFO_T   plmn_info;
    MN_PHONE_SCELL_MEAS_T          rssi;
    APP_MN_NCELL_RSSI_CNF_T        ncell;
    int32                          i;
    uint8                          *temp_str    = PNULL;


    plmn_info = MNPHONE_GetCurrentPLMNInfoEx(dual_sys);
    rssi      = MNPHONE_GetRSSIEx(dual_sys);

    switch(atc_config_ptr->cced_type[dual_sys])
    {
        case ATC_CCED_SCELL:

            // check the status of the phone
            if(PLMN_NO_SERVICE == plmn_info.plmn_status)
            {
                sprintf((char *)g_rsp_str, "%s: ", g_atc_info_table[AT_CMD_CCED].cmd_name->str_ptr);
                ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            }
            else
            {
                sprintf((char *)g_rsp_str, "%s: %d,%02d,%d,%d,%d,%d,%d,%d",
                g_atc_info_table[AT_CMD_CCED].cmd_name->str_ptr, plmn_info.mcc, plmn_info.mnc,
                plmn_info.lac, plmn_info.cell_id, rssi.bsic, rssi.arfcn,
                ATC_GetConvertedRSSIVlaueForCSQ(atc_config_ptr->idle_rxlev[dual_sys]), rssi.rxqual_full);
                ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            }

            break;

        case ATC_CCED_TIME_ADV:
            sprintf((char *)g_rsp_str, "%s: 0", g_atc_info_table[AT_CMD_CCED].cmd_name->str_ptr);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;

        case ATC_CCED_NCELL:

            MNPHONE_GetMeasReportEx(dual_sys, &ncell, PNULL);

            ATC_TRACE_LOW("ATC: get ncell info succ");

            sprintf((char *)g_rsp_str, "%s: ", g_atc_info_table[AT_CMD_CCED].cmd_name->str_ptr);
            temp_str = &g_rsp_str[strlen((char *)g_rsp_str)];

            for(i = 0; i < MN_MAX_NCELL_NUM; i++)
            {
                ATC_TRACE_LOW("ATC: the index: %d", i);

                if(ncell.ncell_power[i].cell_present && ncell.ncell_power[i].identity_present)
                {
                    sprintf((char *)temp_str, "%d,%02d,%d,%d,%d,%d,%d", ncell.ncell_power[i].lai.mcc,
                    ncell.ncell_power[i].lai.mnc, ncell.ncell_power[i].lai.lac,
                    ncell.ncell_power[i].cell_id, ncell.ncell_power[i].bsic,
                    ncell.ncell_power[i].arfcn, ATC_GetConvertedRSSIVlaueForCSQ(ncell.ncell_power[i].rxlev));

                    temp_str    = &g_rsp_str[strlen((char *)g_rsp_str)];
                    *temp_str   = ',';
                    temp_str++;
                    *temp_str   = '\0';
                    ATC_TRACE_LOW("ATC: the string length is %d", strlen((char *)g_rsp_str));
                }
            }

            g_rsp_str[strlen((char *)g_rsp_str) - 1] = '\0';
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;

        case ATC_CCED_CSQ:
            atc_config_ptr->csq_flag[dual_sys] = FALSE;

            sprintf((char *)g_rsp_str, "%s: %d,%d",
                      g_atc_info_table[AT_CMD_CCED].cmd_name->str_ptr,
                      ATC_GetConvertedRSSIVlaueForCSQ(rssi.rxlev), rssi.rxqual_full);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;

        default:
            break;
    }
}
#endif /* _ULTRA_LOW_CODE_ */

/*****************************************************************************/
//  Description : This function handle the AT+CMER command
//  Global resource dependence : none
//  Author:       Xueli.lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCMER)
{
    MN_DUAL_SYS_E   dual_sys = ATC_CURRENT_SIM_ID_MUX;

    if(ATC_GET_CMD_TYPE != ATC_CMD_TYPE_SET)
    {
        return ERR_OPERATION_NOT_SUPPORTED;
    }

    if(!PARAM1_FLAG || !PARAM4_FLAG)
    {
        return ERR_OPERATION_NOT_ALLOWED;
    }

    if(1 != PARA1.num)
    {
        return ERR_OPERATION_NOT_SUPPORTED;
    }

    if(0 == PARA4.num)
    {
        atc_config_ptr->cmer_flag[dual_sys] = FALSE;
    }
    else
    {
        atc_config_ptr->cmer_flag[dual_sys] = TRUE;

        if(atc_config_ptr->rel_former[dual_sys] != 127)
        {
            sprintf((char *)g_rsp_str, "+CIEV: 1,%d", atc_config_ptr->rel_former[dual_sys]);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
        }

    }

    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handle the AT+CCED command
//  Global resource dependence : none
//  Author:       Xueli.lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCCED)
{
#ifndef _ULTRA_LOW_CODE_
    uint16                         mode;
    MN_DUAL_SYS_E   dual_sys = ATC_CURRENT_SIM_ID_MUX;

    SCI_TraceLow("ATC: Enter the function of ATC_ProcessCCED()");

    if(!PARAM1_FLAG || ATC_GET_CMD_TYPE != ATC_CMD_TYPE_SET)
    {
        return ERR_OPERATION_NOT_ALLOWED;
    }

    mode      = PARA1.num;

    switch(mode)
    {
        case ATC_CCED_ONE_SHOT:

            if(PARAM2_FLAG)
            {
                atc_config_ptr->cced_type[dual_sys] = (ATC_CCED_TYPE_E)PARA2.num;
            }

            HandleCcedOneShot(dual_sys, atc_config_ptr);
            break;

        case ATC_CCED_AUTO_SHOT:

            if(PARAM2_FLAG)
            {
                atc_config_ptr->cced_type[dual_sys] = (ATC_CCED_TYPE_E)PARA2.num;

                if(ATC_CCED_CSQ == atc_config_ptr->cced_type[dual_sys])
                {
                    ATC_TRACE_LOW("ATC: Set csq true");
                    atc_config_ptr->csq_flag[dual_sys] = TRUE;
                }
            }

            break;

        case ATC_CCED_STOP_AUTO:

            if(PARAM2_FLAG)
            {
                if(ATC_CCED_CSQ == PARA2.num)
                {
                    atc_config_ptr->csq_flag[dual_sys] = FALSE;
                }
                else
                {
                    atc_config_ptr->cced_type[dual_sys] = ATC_CCED_INVALID;
                }
            }
            else
            {
                atc_config_ptr->cced_type[dual_sys] = ATC_CCED_INVALID;
            }

            break;
        default:
            return ERR_OPERATION_NOT_SUPPORTED;
    }

#endif /* _ULTRA_LOW_CODE_ */

    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handles the AT+SDTMF command
//              AT+SDTMF is used to play the DTMF tone on the current speaker
//               AT+SDTMF=<mode>,<dtmf>,<gain>,<duartion>
//  Global resource dependence : none
//  Author:       Xueli.lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSDTMF)
{
#ifdef _ULTRA_LOW_CODE_
    return S_ATC_FAIL;
#else
    uint8       dtmf_key;
    uint32      duration = AUD_PLAY_FOREVER;
    int32       i;
    uint8       mode_flag;

    AUD_DTMF_TONE_ID_E  dtmf_tone = AUD_DTMF_One;

    if(PARAM1_FLAG)
    {
        mode_flag = PARA1.num;

        if(!mode_flag)
        {
            // stop play the dtmf tone
            uint32 result;

            AUDIO_ExeCodecExtOpe(
                hAtcDtmfRing,
                ANSI2UINT16("EXT_STOP"),
                0,
                PNULL,
                &result
            );
            return S_ATC_SUCCESS;
        }
    }

    if(!PARAM2_FLAG)
    {
        return ERR_INVALID_INDEX;
    }

    dtmf_key = PARA2.str_ptr->str_ptr[0];

    // search the dtmf tone
    for(i = 0; i < AUD_DTMF_MAX_ID; i++)
    {
        if(c_wdtmf_data[i].val == dtmf_key)
        {
            dtmf_tone = (AUD_DTMF_TONE_ID_E)(c_wdtmf_data[i].lev);
            break;
        }
    }

    if(PARAM3_FLAG)
    {
        duration = PARA3.num;

        if(0 == duration)
        {
            duration = AUD_PLAY_FOREVER;
        }
        else
        {
            // uint of 100 ms
            duration *= AT_SPEAKER_UNIT_DURATION;
        }
    }

    {
        DTMF_TONE_ADP_EXT_OPE_PLAY_PARA_T tDtmfPara = {0};
        uint32 result;
        uint32 volume = 0;

        if(INVALID_HANDLE == hAtcDtmfRing) //need to create this audio handle only in the first playing.
        {
            hAtcDtmfRing = AUDIO_CreateNormalHandle(hDtmfCodec, SCI_NULL, hLAYER1, SCI_NULL, AudioDummyNotifyCallback);
            if (INVALID_HANDLE == hAtcDtmfRing)
            {
                ATC_TRACE_LOW("ATC: ATC_ProcessSDTMF  hAtcDtmfRing %d is an invalid handle", hAtcDtmfRing);
                return ERR_OPERATION_NOT_ALLOWED;
            }
        }

        // max volume is 0x4096=16534, divided by 7 is 2362
        if(PARAM4_FLAG)
        {
            volume = PARA4.num*2362;
        }
        else
        {
            volume = 16384;
        }

        tDtmfPara.tone_id = dtmf_tone;
        tDtmfPara.duration = duration;
        tDtmfPara.is_uplink = FALSE;
        tDtmfPara.hAudioHandle = hAtcDtmfRing;
        tDtmfPara.volume = volume;
        AUDIO_ExeCodecExtOpe(
            hAtcDtmfRing,
            ANSI2UINT16("EXT_PLAY"),
            DTMF_TONE_ADP_EXT_OPE_PLAY_PARA_COUNT,
            &tDtmfPara,
            &result
        );
    }

    return S_ATC_SUCCESS;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handles the AT+CRSL command.This command is used
//                to set or get the sound level of the ringer on incoming call.
//  AT+CRSL=<sound level>
//  Global resource dependence : none
//  Author:       Xueli.lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCRSL)
{
    ATC_STATUS     status = S_ATC_SUCCESS;

#ifndef _ULTRA_LOW_CODE_

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:

            if(PARAM1_FLAG)
            {
                atc_config_ptr->crsl_value = PARA1.num;
            }
            else
            {
                status = ERR_OPERATION_NOT_ALLOWED;
            }

            break;

        case ATC_CMD_TYPE_READ:
            sprintf((char *)g_rsp_str, "%s: %d", g_atc_info_table[AT_CMD_CRSL].cmd_name->str_ptr, atc_config_ptr->crsl_value);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;

        default:
            status = ERR_OPERATION_NOT_SUPPORTED;
            break;
    }

#endif /* _ULTRA_LOW_CODE_ */

    return status;
}

/*****************************************************************************/
//  Description : This function handles the AT+CMUT command.This command is used
//                to mute the micrphone input.
//  AT+CRSL=<sound level>
//  Global resource dependence : none
//  Author:       Xueli.lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCMUT)
{
    ATC_STATUS           status = S_ATC_SUCCESS;

#ifndef _ULTRA_LOW_CODE_

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:

            if(PARAM1_FLAG && g_calls_state[ATC_CURRENT_SIM_ID_MUX].call_amount != 0)
            {
                atc_config_ptr->cmut_flag = PARA1.num;
                AUDIO_SetUpLinkMute(atc_config_ptr->cmut_flag);

                SCI_SLEEP(50);
                LAYER1_SetVolume(atc_config_ptr->speaker_volume);
            }
            else
            {
                status = ERR_OPERATION_NOT_ALLOWED;
            }

            break;

        case ATC_CMD_TYPE_READ:
            sprintf((char *)g_rsp_str, "%s: %d", g_atc_info_table[AT_CMD_CMUT].cmd_name->str_ptr, atc_config_ptr->cmut_flag);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;

        default:
            status = ERR_OPERATION_NOT_SUPPORTED;
            break;
    }

#endif /* _ULTRA_LOW_CODE_ */

    return status;
}

/*****************************************************************************/
//  Description : This function handleS the AT&W command
//              AT&W is used to save the current configuration to the nv
//  Global resource dependence : none
//  Author:       Xueli.lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_Processand_W)
{
#ifndef _ULTRA_LOW_CODE_
    ATC_WriteNVItem(atc_config_ptr, NV_ATC_CONFIG_ID);
#endif /* _ULTRA_LOW_CODE_ */
    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handle the AT+CFUN command
//  Global resource dependence : none
//  Author:       Ivan.Yin
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCFUN)
{
#ifndef _ULTRA_LOW_CODE_
    ATC_STATUS    status = S_ATC_DEFAULT_HANDLE;
    MN_DUAL_SYS_E   dual_sys = ATC_CURRENT_SIM_ID_MUX;

    ATC_TRACE_LOW("ATC: ATC_ProcessCFUN is called");

    if(ATC_GET_CMD_TYPE == ATC_CMD_TYPE_SET)
    {
        //if parameter2 is 1,reset the modle
        if(PARAM2_FLAG)
        {
            if(PARA2.num)
            {
                ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
                POWER_Reset();
                status = S_ATC_SUCCESS;
            }
        }


        if(PARAM1_FLAG)
        {
            ATC_TRACE_LOW("ATC: ATC_ProcessCFUN: set  PARA1:%d  when cfun status %d", PARA1.num, g_is_ps_act[dual_sys]);

            if ((PARA1.num == 0) && !g_is_ps_act[dual_sys])
            {
                atc_config_ptr->cfun_flag[dual_sys] = PARA1.num;
                return S_ATC_SUCCESS;
            }

#ifdef _MUX_ENABLE_
            if(ATC_GetPending_Event(atc_config_ptr, APP_MN_SIM_POWER_ON_CNF, ATC_INVALID_LINK_ID, dual_sys))
            {
                ATC_TRACE_LOW("ATC: dual_sys = %d   APP_MN_SIM_POWER_ON_CNF  MSG pending", dual_sys);
                return ERR_OPERATION_NOT_ALLOWED;
            }

            if(ATC_GetPending_Event(atc_config_ptr, APP_MN_PS_POWER_ON_CNF, ATC_INVALID_LINK_ID, dual_sys))
            {
                ATC_TRACE_LOW("ATC: dual_sys = %d   APP_MN_PS_POWER_ON_CNF  MSG pending", dual_sys);
                return ERR_OPERATION_NOT_ALLOWED;
            }

            if(ATC_GetPending_Event(atc_config_ptr, APP_MN_SIM_POWER_OFF_CNF, ATC_INVALID_LINK_ID, dual_sys))
            {
                ATC_TRACE_LOW("ATC: dual_sys = %d   APP_MN_SIM_POWER_OFF_CNF  MSG pending", dual_sys);
                return ERR_OPERATION_NOT_ALLOWED;
            }

            if(ATC_GetPending_Event(atc_config_ptr, APP_MN_PS_POWER_OFF_CNF, ATC_INVALID_LINK_ID, dual_sys))
            {
                ATC_TRACE_LOW("ATC: dual_sys = %d   APP_MN_PS_POWER_OFF_CNF  MSG pending", dual_sys);
                return ERR_OPERATION_NOT_ALLOWED;
            }
#endif

           ATC_TRACE_LOW("ATC: ATC_ProcessCFUN  cfun_status: %d", atc_config_ptr->cfun_status[dual_sys]);

            if(PARA1.num)
            {
                switch(atc_config_ptr->cfun_status[dual_sys])
                {
                    case ATC_FUN_DEACT_SIM_PS:
                        MNPHONE_PowerOnSimEx(dual_sys, TRUE);
                        break;

                    case ATC_FUN_ACT_SIM_PS:
                        status = S_ATC_SUCCESS;
                        break;

                    case ATC_FUN_DEACT_SIM:
                        /*PS should have been deactived before*/
                        if (g_is_ps_act[dual_sys])
                        {
                            status = ERR_OPERATION_NOT_ALLOWED;
                        }
                        else
                        {
                            MNPHONE_StartupPsEx(dual_sys, MN_GMMREG_RAT_GPRS);
                        }
                        break;

                    case ATC_FUN_ACT_SIM:
                        if (g_is_ps_act[dual_sys])
                        {
                            status = S_ATC_SUCCESS;
                        }
                        else
                        {
                            MNPHONE_PowerOnPsEx(dual_sys);
                        }
                        break;

                    case ATC_FUN_DEACT_PS:
                        if (TRUE == atc_config_ptr->sim_flag[dual_sys])
                        {
                            MNPHONE_PowerOnPsEx(dual_sys);
                        }
                        else
                        {
                            MNPHONE_StartupPsEx(dual_sys, MN_GMMREG_RAT_GPRS);
                        }
                        break;

                    case ATC_FUN_ACT_PS:
                        if (TRUE != atc_config_ptr->sim_flag[dual_sys])
                        {
                            status = ERR_OPERATION_NOT_ALLOWED;
                        }
                        else
                        {
                            status = S_ATC_SUCCESS;
                        }
                        break;

                    default :
                        status = ERR_OPERATION_NOT_ALLOWED;
                        break;
                }

                if (ERR_OPERATION_NOT_ALLOWED == status)
                {
                    return ERR_OPERATION_NOT_ALLOWED;
                }

                if (S_ATC_DEFAULT_HANDLE == status)
                {
                    ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr,
                                    (uint16)APP_MN_PS_POWER_ON_CNF, 0xff, ATC_NO_DOMAIN, dual_sys);
                }

                g_is_ps_act[dual_sys] = TRUE;
                atc_config_ptr->cfun_flag[dual_sys] = PARA1.num;
                atc_config_ptr->cfun_status[dual_sys] = ATC_FUN_ACT_SIM_PS;

                return status;
            }
            else
            {
                ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr,
                                  (uint16)APP_MN_DEACTIVE_PS_CNF, 0xff, ATC_NO_DOMAIN, dual_sys);
                MNPHONE_DeactivePsEx(dual_sys);
                g_is_ps_act[dual_sys]   = FALSE;
                g_is_ps_deactivate[dual_sys] = TRUE;
                atc_config_ptr->cfun_flag[dual_sys] = PARA1.num;
                atc_config_ptr->cfun_status[dual_sys] = ATC_FUN_DEACT_SIM_PS;

                return  S_ATC_DEFAULT_HANDLE;
            }
        }
    }
    else
    {
        sprintf((char *)g_rsp_str, "%s: %d",
               g_atc_info_table[AT_CMD_CFUN].cmd_name->str_ptr, atc_config_ptr->cfun_flag[dual_sys]);
        ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
        status = S_ATC_SUCCESS;
    }

    return status;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the AT+ECHO command
//  Global resource dependence : none
//  Author:     Xueli.lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessECHO)
{
#ifndef _ULTRA_LOW_CODE_

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:
            //call the Audio function
            break;
        case ATC_CMD_TYPE_TEST:
            sprintf((char *)g_rsp_str, "%s: 0,1,0,3,10,7",
            g_atc_info_table[AT_CMD_ECHO].cmd_name->str_ptr);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            return S_ATC_SUCCESS;
            //break;
        case ATC_CMD_TYPE_READ:
            break;

        default:
            return ERR_OPERATION_NOT_SUPPORTED;
    }

#endif /* _ULTRA_LOW_CODE_ */

    return S_ATC_FAIL;
}


/*****************************************************************************/
//  Description : This function handle the AT+STONE command
//  Global resource dependence : none
//  Author:       Xueli.lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSTONE)
{
#ifdef _ULTRA_LOW_CODE_
    return S_ATC_FAIL;
#else
    BOOLEAN       mode_flag;
    uint16        freq1 = 0;
    uint16        freq2 = 0;
    uint16        freq3 = 0;
    uint32        duration = 0;


    if(!PARAM1_FLAG)
    {
        return ERR_INVALID_INDEX;
    }

    mode_flag = PARA1.num;

    if(!mode_flag)
    {
        uint32 result;
        AUDIO_ExeCodecExtOpe(
            hAtcCustomRing,
            ANSI2UINT16("EXT_STOP"),
            0,
            PNULL,
            &result
        );
        return S_ATC_SUCCESS;
    }

    // get the frequence 1,2,3
    if(PARAM2_FLAG)
    {
        freq1 = PARA2.num;
    }

    if(PARAM3_FLAG)
    {
        freq2 = PARA3.num;
    }

    if(PARAM4_FLAG)
    {
        freq3 = PARA4.num;
    }

    //set the volume

    if(PARAM5_FLAG)
    {
        duration = PARA5.num;
    }

    if(0 == duration)
    {
        duration = AUD_PLAY_FOREVER;
    }
    else
    {
        duration = duration * AT_SPEAKER_UNIT_DURATION;
    }

    // play the generic tone
    {
        CUSTOM_TONE_ADP_EXT_OPE_PLAY_PARA_T tCustomPara = {0};
        uint32 result = 0;

        if(INVALID_HANDLE == hAtcCustomRing) //need to create this audio handle only in the first playing.
        {
            hAtcCustomRing = AUDIO_CreateNormalHandle(hCustomCodec, SCI_NULL, hLAYER1, SCI_NULL, AudioDummyNotifyCallback);
        }

        tCustomPara.freq1 = freq1;
        tCustomPara.freq2 = freq1;
        tCustomPara.freq3 = freq1;
        tCustomPara.duration = duration;
        tCustomPara.hAudioHandle = hAtcCustomRing;
        tCustomPara.volume = 16384;

        AUDIO_ExeCodecExtOpe(
            hAtcCustomRing,
            ANSI2UINT16("EXT_PLAY"),
            CUSTOM_TONE_ADP_EXT_OPE_PLAY_PARA_COUNT,
            &tCustomPara,
            &result
        );
    }

    return     S_ATC_SUCCESS;
#endif /* _ULTRA_LOW_CODE_ */
}

#ifndef ATC_SAT_ENABLE
extern AUDIO_RESULT_E ATC_EnableBtVoiceCodec( BOOLEAN is_enable );
#endif
/*****************************************************************************/
//  Description : This function handle the AT+SSAM command
//  Global resource dependence : none
//  Author:  Xueli.lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSSAM)
{
    ATC_STATUS           status = S_ATC_SUCCESS;
#ifndef _ULTRA_LOW_CODE_

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:

            if(PARAM1_FLAG)
            {
            #ifndef ATC_SAT_ENABLE
                BOOLEAN  is_exist_call = FALSE;
                uint8 i;
            #endif

                atc_config_ptr->audio_mode = (uint8)PARA1.num;
            #ifndef ATC_SAT_ENABLE
                for(i=0; i<MN_SYS_NUMBER; i++)
                {
                    if(0 != g_calls_state[i].call_amount)
                    {
                        SCI_TRACE_LOW("ATC: SIM %d call amount %d, call_flag %d", i, g_calls_state[i].call_amount, g_atc_call_flag);
                        is_exist_call = TRUE;
                        break;
                    }
                }

                if(atc_config_ptr->audio_mode == AUDIO_DEVICE_MODE_BLUEPHONE)
                {
                    g_atc_pcm_flag = TRUE;
                    if(is_exist_call && g_atc_call_flag)
                    {
                        if(atc_config_ptr->audio_flag)
                        {
                            #ifdef AUDIO_PARAM_ARM
                                AUDIO_SetDevModeEx(INVALID_HANDLE, (AUDIO_DEVICE_MODE_TYPE_E)(atc_config_ptr->audio_mode));
                            #else
                                AUDIO_SetDevMode((AUDIO_DEVICE_MODE_TYPE_E)(atc_config_ptr->audio_mode));
                            #endif
                            ATC_EnableBtVoiceCodec(TRUE);
                        }
                        else
                        {
                            #ifdef AUDIO_PARAM_ARM
                                AUDIO_SetDevModeEx(INVALID_HANDLE, (AUDIO_DEVICE_MODE_TYPE_E)(atc_config_ptr->audio_mode));
                            #else
                                AUDIO_SetDevMode((AUDIO_DEVICE_MODE_TYPE_E)(atc_config_ptr->audio_mode));
                            #endif
                        }
                    }
                }
                else
                {
                    if(is_exist_call && g_atc_call_flag && g_atc_pcm_flag)
                    {
                        g_atc_pcm_flag = FALSE;
                        ATC_EnableBtVoiceCodec(FALSE);
                    }
                    #ifdef AUDIO_PARAM_ARM
                        AUDIO_SetDevModeEx(INVALID_HANDLE, (AUDIO_DEVICE_MODE_TYPE_E)(atc_config_ptr->audio_mode));
                    #else
                        AUDIO_SetDevMode((AUDIO_DEVICE_MODE_TYPE_E)(atc_config_ptr->audio_mode));
                    #endif

                }
            #endif/*ATC_SAT_ENABLE*/
            }
            else
            {
                status = ERR_INVALID_INDEX;
            }
            break;

        case ATC_CMD_TYPE_READ:
            // get the current sound mode
            sprintf((char *)g_rsp_str, "%s: %d",
                g_atc_info_table[AT_CMD_SSAM].cmd_name->str_ptr,atc_config_ptr->audio_mode);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;

        default:
            status = ERR_OPERATION_NOT_SUPPORTED;
            break;
    }

#endif /* _ULTRA_LOW_CODE_ */

    return status;
}

/*****************************************************************************/
//  Description : This function handle the AT+SSAP command
//  Global resource dependence : none
//  Author:  Shijun.cui
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSSAP)
{
#ifdef _ULTRA_LOW_CODE_
    return S_ATC_SUCCESS;
#else
    ATC_STATUS    status = S_ATC_SUCCESS;
    uint8         parameter_type;
    uint8         b_ul_flag;
    uint16        pga_gain;
    uint16        digital_gain;
    AUDIO_TEST_DATA_T audio_test_data = {0};
    uint8         *ptr;


    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:

            if(PARAM1_FLAG && PARAM2_FLAG && PARAM3_FLAG && PARAM4_FLAG)
            {
                parameter_type = (uint8)PARA1.num;
                b_ul_flag      = (uint8)PARA2.num;
                pga_gain       = (uint16)PARA3.num;
                digital_gain   = (uint16)PARA4.num;
                ATC_TRACE_LOW("ATC: pga_gain = %d, digital_gain = %d", pga_gain, digital_gain);
            }
            else
            {
                return ERR_INVALID_INDEX;
            }

            AUD_GetEngineeringParameter((AUDIO_DEVICE_MODE_TYPE_E)parameter_type, (AUDIO_TEST_DATA_T *)&audio_test_data);

            if(b_ul_flag) // set UL gain
            {
                audio_test_data.UL_PGA_gain = pga_gain;
                audio_test_data.UL_digital_gain = digital_gain;
            }
            else // set DL gain
            {
                audio_test_data.DL_PGA_gain = pga_gain;
                audio_test_data.DL_digital_gain = digital_gain;
            }

            AUD_SetEngineeringParameter((AUDIO_DEVICE_MODE_TYPE_E)parameter_type, audio_test_data);

            // this is a temporary code because of the DSP, or the parameter will not go into effect immediately
            SCI_SLEEP(50);
            LAYER1_SetVolume(atc_config_ptr->speaker_volume);


            break;
        case ATC_CMD_TYPE_READ:
            // get the current sound mode
            ptr = g_rsp_str;
            AUD_GetEngineeringParameter(AUDIO_DEVICE_MODE_HANDHOLD, &audio_test_data);
            sprintf((char *)ptr, "%s: %d,%d,%d,%d,%d\r\n", g_atc_info_table[AT_CMD_SSAP].cmd_name->str_ptr, AUDIO_DEVICE_MODE_HANDHOLD,
            audio_test_data.UL_PGA_gain, audio_test_data.UL_digital_gain,
            audio_test_data.DL_PGA_gain, audio_test_data.DL_digital_gain);
            ptr += strlen((char *)ptr);

            AUD_GetEngineeringParameter(AUDIO_DEVICE_MODE_HANDFREE, &audio_test_data);
            sprintf((char *)ptr, "%s: %d,%d,%d,%d,%d\r\n", g_atc_info_table[AT_CMD_SSAP].cmd_name->str_ptr, AUDIO_DEVICE_MODE_HANDFREE,
            audio_test_data.UL_PGA_gain, audio_test_data.UL_digital_gain,
            audio_test_data.DL_PGA_gain, audio_test_data.DL_digital_gain);
            ptr += strlen((char *)ptr);

            AUD_GetEngineeringParameter(AUDIO_DEVICE_MODE_EARPHONE, &audio_test_data);
            sprintf((char *)ptr, "%s: %d,%d,%d,%d,%d", g_atc_info_table[AT_CMD_SSAP].cmd_name->str_ptr, AUDIO_DEVICE_MODE_EARPHONE,
            audio_test_data.UL_PGA_gain, audio_test_data.UL_digital_gain,
            audio_test_data.DL_PGA_gain, audio_test_data.DL_digital_gain);
            ptr += strlen((char *)ptr);

            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;

        default:
            status = ERR_OPERATION_NOT_SUPPORTED;
            break;
    }

    return status;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the AT+SADM command
//  Global resource dependence : g_rsp_str
//  Author:  Shijun.cui
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSADM)
{
    ATC_STATUS        status = S_ATC_SUCCESS;
    ATC_AUD_MODE_OPT_TYPE_E    type = ATC_GET_AUD_MODE_NUM;
    uint32             mode_index = 0;
    uint32             dev_number = 0;
    AUDMOD_DEVMOD_T*  audmod_devmod_ptr = NULL;
#ifdef AUDIO_PARAM_ARM
    char *  mode_name_ptr = PNULL;
    AUDIO_NV_ARM_MODE_INFO_T* audmod_arm_devmod_ptr = PNULL;
    uint8  *tmp_rsp_str = PNULL;
    AUDIO_RESULT_E result;
#else
    AUDMOD_RESULT_E result;
#endif
    uint8* str_ptr = NULL;

    if (ATC_GET_CMD_TYPE != ATC_CMD_TYPE_SET)
    {
        return ERR_OPERATION_NOT_SUPPORTED;
    }

    if(PARAM1_FLAG)
    {
        type = (ATC_AUD_MODE_OPT_TYPE_E)PARA1.num;
    }
    else
    {
        return ERR_INVALID_INDEX;
    }

    if(type >= ATC_GET_AUD_MODE_NUM && type <=ATC_GET_AUD_MODE_NV_DATA)
    {
        dev_number = AUDMOD_GetDevModeNumber();
    }
#ifdef AUDIO_PARAM_ARM
    else
    {
        dev_number = AUDIONVARM_GetModeNumber();
    }
#endif/*AUDIO_PARAM_ARM*/

    switch(type)
    {
    case ATC_GET_AUD_MODE_NUM:
        sprintf((char*)g_rsp_str, "%s: %ld", g_atc_info_table[AT_CMD_SADM].cmd_name->str_ptr, dev_number);
        ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
        break;

    case ATC_GET_AUD_MODE_NAME:
        if(PARAM2_FLAG)
        {
            if ((PARAM2_TYPE == ATC_CMD_PARAM_TYPE_NUMERIC) && ((uint32)PARA2.num < dev_number))
            {
                mode_index = (uint32)PARA2.num;

                SCI_TRACE_LOW("ATC: sadm mode_index = %d, dev_number = %d", mode_index, dev_number);

                audmod_devmod_ptr = AUDMOD_GetFirstDevMode();


                while(mode_index)
                {
                    audmod_devmod_ptr = AUDMOD_GetNextDevMode(audmod_devmod_ptr);

                    mode_index--;
                }

                sprintf((char*)g_rsp_str, "%s: %ld,\"%s\"",
                    g_atc_info_table[AT_CMD_SADM].cmd_name->str_ptr,
                    mode_index,
                    audmod_devmod_ptr->ucModeName);

                ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            }
            else
            {
                status = ERR_INVALID_INDEX;
            }
        }
        else
        {
            status = ERR_INVALID_INDEX;
        }
        break;

    case ATC_GET_AUD_MODE_DATA:
        if(PARAM2_FLAG)
        {
            if ((PARAM2_TYPE == ATC_CMD_PARAM_TYPE_NUMERIC) && ((uint32)PARA2.num < dev_number))
            {
                mode_index = (uint32)PARA2.num;

                SCI_TRACE_LOW("ATC: sadm mode_index = %d, dev_number = %d", mode_index, dev_number);
                if(mode_index < dev_number)
                {
                    audmod_devmod_ptr = AUDMOD_GetFirstDevMode();


                    while(mode_index)
                    {
                        audmod_devmod_ptr = AUDMOD_GetNextDevMode(audmod_devmod_ptr);

                        mode_index--;
                    }

                    str_ptr = (uint8*)SCI_ALLOC_BASEZ((sizeof(AUDIO_STRUCT_T)+1)*2);

                    ConvertBinToHex((uint8*)&audmod_devmod_ptr->tAudioStruct, sizeof(AUDIO_STRUCT_T), str_ptr);

                    sprintf((char*)g_rsp_str, "%s: %ld,\"%s\",%s",
                        g_atc_info_table[AT_CMD_SADM].cmd_name->str_ptr,
                        mode_index,
                        audmod_devmod_ptr->ucModeName,
                        str_ptr);

                    SCI_FREE(str_ptr);
                    ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
                }
                else
                {
                    status = ERR_INVALID_INDEX;
                }
            }
            else
            {
                status = ERR_INVALID_INDEX;
            }
        }
        else
        {
            status = ERR_INVALID_INDEX;
        }
        break;

    case ATC_SET_AUD_MODE_DATA:
        if(PARAM2_FLAG)
        {
            if ((PARAM2_TYPE == ATC_CMD_PARAM_TYPE_NUMERIC) && ((uint32)PARA2.num < dev_number))
            {
                s_atc_admm_index = (uint8)PARA2.num;
                // output the '>'
                ATC_SendAsyncInfo(atc_config_ptr, ATC_CTRLZ_SADM);

                status = S_ATC_DEFAULT_HANDLE;
            }
            else
            {
                status = ERR_INVALID_INDEX;
            }
        }
        else
        {
            status = ERR_INVALID_INDEX;
        }
        break;

    case ATC_SET_AUD_DEV_MODE:
        if(PARAM2_FLAG)
        {
            if (PARAM2_TYPE == ATC_CMD_PARAM_TYPE_STRING)
            {
            #ifdef AUDIO_PARAM_ARM
                AUDIO_DEVICE_MODE_TYPE_E eType = AUDMOD_GetAudioDevModeFromName((char *)(PARA2.str_ptr->str_ptr));

                if(AUDIO_DEVICE_MODE_MAX == eType)
                {
                    SCI_TRACE_LOW("ATC: set dev mode, can't get valid mode type, mode name:%s.", PARA2.str_ptr->str_ptr);
                    status = ERR_INVALID_INDEX;
                }
                else
                {
                    result = AUDIO_SetDevModeEx(INVALID_HANDLE, eType);
                    if (AUDIO_NO_ERROR != result)
                    {
                        SCI_TRACE_LOW("ATC: set dev mode result = %d",result);
                        status = ERR_INVALID_INDEX;
                    }
                }
            #else
                result = AUDMOD_SetDevMode(PARA2.str_ptr->str_ptr, PNULL);
                if (AUDMOD_NO_ERROR != result)
                {
                    SCI_TRACE_LOW("ATC: set dev mode result = %d",result);
                    status = ERR_INVALID_INDEX;
                }
            #endif
            }
            else
            {
                SCI_TRACE_LOW("ATC: set aud dev param error.");
                status = ERR_INVALID_INDEX;
            }
        }
        else
        {
            status = ERR_INVALID_INDEX;
        }
        break;

   case ATC_SET_AUD_MODE_NV_DATA:
        if(PARAM2_FLAG)
        {
            if ((PARAM2_TYPE == ATC_CMD_PARAM_TYPE_NUMERIC) && ((uint32)PARA2.num < dev_number))
            {
                s_atc_admm_index = (uint8)PARA2.num;
                // output the '>'
                ATC_SendAsyncInfo(atc_config_ptr, ATC_CTRLZ_SADMNV);

                status = S_ATC_DEFAULT_HANDLE;
            }
            else
            {
                status = ERR_INVALID_INDEX;
            }
        }
        else
        {
            status = ERR_INVALID_INDEX;
        }
        break;

    case ATC_GET_AUD_MODE_NV_DATA:
        if ((!PARAM2_FLAG) ||
            (!(PARAM2_TYPE == ATC_CMD_PARAM_TYPE_NUMERIC && (uint32)PARA2.num < dev_number)))
        {
            status = ERR_INVALID_INDEX;
            break;
        }
        mode_index = (uint32)PARA2.num;
        SCI_TRACE_LOW("ATC: sadm mode_index = %d, dev_number = %d", mode_index, dev_number);

        {
            AUDMOD_DEVMOD_T *temp_aud_ptr = NULL;

            audmod_devmod_ptr = AUDMOD_GetFirstDevMode();


            while(mode_index)
            {
                audmod_devmod_ptr = AUDMOD_GetNextDevMode(audmod_devmod_ptr);

                mode_index--;
            }

            temp_aud_ptr = (AUDMOD_DEVMOD_T*)SCI_ALLOC_BASEZ(sizeof(AUDMOD_DEVMOD_T));

            str_ptr = (uint8*)SCI_ALLOC_BASEZ((sizeof(AUDIO_STRUCT_T)+1)*2);

    #ifndef WIN32
            if (AUDMOD_NO_ERROR != PROD_ReadAudioParamFromFlash(audmod_devmod_ptr->ucModeName,temp_aud_ptr))
            {
                SCI_TRACE_LOW("ATC: PROD_ReadAudioParamFromFlash failed.");
                status = ERR_UNKNOWN;
                break;
            }
    #endif

            ConvertBinToHex((uint8*)&temp_aud_ptr->tAudioStruct, sizeof(AUDIO_STRUCT_T),str_ptr);

            sprintf((char*)g_rsp_str, "%s: %ld,\"%s\",%s",
                g_atc_info_table[AT_CMD_SADM].cmd_name->str_ptr,
                mode_index,
                temp_aud_ptr->ucModeName,
                str_ptr);

            SCI_FREE(str_ptr);
            SCI_FREE(temp_aud_ptr);

            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
        }
        break;

#ifdef AUDIO_PARAM_ARM
    case ATC_GET_AUD_ARM_MODE_NUM:
        sprintf((char*)g_rsp_str, "%s: %ld", g_atc_info_table[AT_CMD_SADM].cmd_name->str_ptr, dev_number);
        ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
        break;

    case ATC_GET_AUD_ARM_MODE_NAME:
        if(PARAM2_FLAG)
        {
            if ((PARAM2_TYPE == ATC_CMD_PARAM_TYPE_NUMERIC) && ((uint32)PARA2.num < dev_number))
            {
                mode_index = (uint32)PARA2.num;
                SCI_TRACE_LOW("ATC: sadm mode_index = %d, dev_number = %d", mode_index, dev_number);
                mode_name_ptr = AUDIONVARM_GetAudioModeName(mode_index);

                sprintf((char*)g_rsp_str, "%s: %ld,\"%s\"",
                    g_atc_info_table[AT_CMD_SADM].cmd_name->str_ptr,
                    mode_index,
                    mode_name_ptr);
                ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            }
            else
            {
                status = ERR_INVALID_INDEX;
            }
        }
        else
        {
            status = ERR_INVALID_INDEX;
        }
        break;

    case ATC_GET_AUD_ARM_MODE_DATA:
        if(PARAM2_FLAG)
        {
            if ((PARAM2_TYPE == ATC_CMD_PARAM_TYPE_NUMERIC) && ((uint32)PARA2.num < dev_number))
            {
                mode_index = (uint32)PARA2.num;

                SCI_TRACE_LOW("ATC: sadm mode_index = %d, dev_number = %d, %d", mode_index, dev_number, sizeof(AUDIO_NV_ARM_MODE_INFO_T));
                if(mode_index < dev_number)
                {
                    audmod_arm_devmod_ptr = (AUDIO_NV_ARM_MODE_INFO_T*)SCI_ALLOC_BASEZ(sizeof(AUDIO_NV_ARM_MODE_INFO_T));
                    mode_name_ptr = AUDIONVARM_GetAudioModeName(mode_index);
                    if(PNULL == mode_name_ptr)
                    {
                        SCI_TRACE_LOW("AGC_Get_Param failed to get mode name, mode:%d.", mode_index);
                        return ERR_NOT_FOUND;
                    }
                    else
                    {
                        SCI_TRACE_LOW("ATC:mode_name_ptr: %s, 0x%x", mode_name_ptr,audmod_arm_devmod_ptr);
                        if(AUDIO_NV_ARM_NO_ERROR != AUDIONVARM_GetModeParam((const char *) mode_name_ptr, (AUDIO_NV_ARM_MODE_INFO_T *)(audmod_arm_devmod_ptr)))
                        {
                            SCI_TRACE_LOW("AGC_Get_Param failed to get mode para.");
                            return ERR_NOT_FOUND;
                        }
                        str_ptr = (uint8*)SCI_ALLOC_BASEZ((sizeof(AUDIO_NV_ARM_MODE_STRUCT_T) + 1) * 2);

                        ConvertBinToHex((uint8*)&(audmod_arm_devmod_ptr->tAudioNvArmModeStruct), sizeof(AUDIO_NV_ARM_MODE_STRUCT_T), str_ptr);
                        SCI_TRACE_LOW("ATC: audmod_arm_devmod_ptr = 0x%x, str_ptr = 0x%x", audmod_arm_devmod_ptr, str_ptr);

                        //need larger memory over MAX_ATC_RSP_LEN, less than MAX_ATC_RSP_LEN
                        tmp_rsp_str = (uint8*)SCI_ALLOC_BASEZ(MAX_ATC_RSP_LEN * 2);
                        sprintf((char*)tmp_rsp_str, "%s: %ld,\"%s\",%s",
                                g_atc_info_table[AT_CMD_SADM].cmd_name->str_ptr,
                                mode_index,
                                audmod_arm_devmod_ptr->ucModeName,
                                str_ptr);

                        SCI_FREE(audmod_arm_devmod_ptr);
                        SCI_FREE(str_ptr);

                        ATC_BuildInfoRsp(atc_config_ptr, tmp_rsp_str);
                        SCI_FREE(tmp_rsp_str);
                    }
                }
                else
                {
                    status = ERR_INVALID_INDEX;
                }
            }
            else
            {
                status = ERR_INVALID_INDEX;
            }
        }
        else
        {
            status = ERR_INVALID_INDEX;
        }
        break;

    case ATC_SET_AUD_ARM_MODE_DATA:
        if(PARAM2_FLAG)
        {
            if ((PARAM2_TYPE == ATC_CMD_PARAM_TYPE_NUMERIC) && ((uint32)PARA2.num < dev_number))
            {
                s_atc_admm_index = (uint8)PARA2.num;
                // output the '>'
                ATC_SendAsyncInfo(atc_config_ptr, ATC_CTRLZ_SADM_ARM);

                status = S_ATC_DEFAULT_HANDLE;
            }
            else
            {
                status = ERR_INVALID_INDEX;
            }
        }
        else
        {
            status = ERR_INVALID_INDEX;
        }
        break;

    case ATC_GET_AUD_ARM_MODE_NV_DATA:
        if ((!PARAM2_FLAG) ||
            (!(PARAM2_TYPE == ATC_CMD_PARAM_TYPE_NUMERIC && (uint32)PARA2.num < dev_number)))
        {
            status = ERR_INVALID_INDEX;
            break;
        }
        mode_index = (uint32)PARA2.num;
        SCI_TRACE_LOW("ATC: sadm mode_index = %d, dev_number = %d", mode_index, dev_number);

        audmod_arm_devmod_ptr = SCI_ALLOC_BASEZ(sizeof(AUDIO_NV_ARM_MODE_INFO_T));

        mode_name_ptr = AUDIONVARM_GetAudioModeName(mode_index);
        if(PNULL == mode_name_ptr)
        {
            SCI_TRACE_LOW("AGC_Get_Param failed to get mode name, mode:%d.", mode_index);
            return ERR_NOT_FOUND;
        }
        SCI_TRACE_LOW("AGC_Get_Param to get mode name, mode:%d, %s.", mode_index, mode_name_ptr);
        if (AUDMOD_NO_ERROR != AUDIONVARM_ReadModeParamFromFlash(mode_name_ptr, audmod_arm_devmod_ptr))
        {
            SCI_TRACE_LOW("ATC: AUDIONVARM_ReadModeParamFromFlash failed.");
            status = ERR_UNKNOWN;
            break;
        }
        str_ptr = (uint8*)SCI_ALLOC_BASEZ((sizeof(AUDIO_NV_ARM_MODE_STRUCT_T) + 1) * 2);

        ConvertBinToHex((uint8*)&audmod_arm_devmod_ptr->tAudioNvArmModeStruct, sizeof(AUDIO_NV_ARM_MODE_STRUCT_T), str_ptr);

        //need larger memory over MAX_ATC_RSP_LEN, less than MAX_ATC_RSP_LEN
        tmp_rsp_str = (uint8*)SCI_ALLOC_BASEZ(MAX_ATC_RSP_LEN * 2);
        sprintf((char*)tmp_rsp_str, "%s: %ld,\"%s\",%s",
                    g_atc_info_table[AT_CMD_SADM].cmd_name->str_ptr,
                    mode_index,
                    audmod_arm_devmod_ptr->ucModeName,
                    str_ptr);

        SCI_FREE(str_ptr);
        SCI_FREE(audmod_arm_devmod_ptr);

        ATC_BuildInfoRsp(atc_config_ptr, tmp_rsp_str);
        SCI_FREE(tmp_rsp_str);
        break;

    case ATC_SET_AUD_ARM_MODE_NV_DATA:
        if(PARAM2_FLAG)
        {
            if ((PARAM2_TYPE == ATC_CMD_PARAM_TYPE_NUMERIC) && ((uint32)(PARA2.num) < dev_number))
            {
                s_atc_admm_index = (uint8)PARA2.num;
                // output the '>'
                ATC_SendAsyncInfo(atc_config_ptr, ATC_CTRLZ_SADMNV_ARM);

                status = S_ATC_DEFAULT_HANDLE;
            }
            else
            {
                status = ERR_INVALID_INDEX;
            }
        }
        else
        {
            status = ERR_INVALID_INDEX;
        }
        break;
#else
    case ATC_GET_AUD_ARM_MODE_NUM:
    case ATC_GET_AUD_ARM_MODE_NAME:
    case ATC_GET_AUD_ARM_MODE_DATA:
    case ATC_SET_AUD_ARM_MODE_DATA:
    case ATC_SET_AUD_ARM_MODE_NV_DATA:
    case ATC_GET_AUD_ARM_MODE_NV_DATA:
        status = ERR_OPERATION_NOT_SUPPORTED;
        break;
#endif/*AUDIO_PARAM_ARM*/

    default:
        SCI_TRACE_LOW("ATC: Assert SADM,invalid type.");
        break;
    }

    return status;
}

#ifdef AUDIO_PARAM_ARM
/*****************************************************************************/
//  Description : This function handles the data send from the SIO when AT+SADM
//  Global resource dependence : g_rsp_str
//  Author:
//  Note:
/*****************************************************************************/
PUBLIC uint32 ATC_ProcessSioARMSADMData(
                ATC_CONFIG_T    *atc_config_ptr,
                uint8           *data_ptr,  // pointer to the data
                uint16          param_len
                )
{
    uint8             terminater_char;     // indicates ther terminater char
    BOOLEAN           flag = FALSE;
    uint32            dev_number = 0;

    AUDIO_NV_ARM_RESULT_E   result = AUDIO_NV_ARM_NO_ERROR;

    AUDIO_NV_ARM_MODE_INFO_T* audmod_arm_devmod_ptr = PNULL;
    char *name_ptr = PNULL;
    uint8  name_len = 0;

    SCI_ASSERT(PNULL != atc_config_ptr); /*assert verified*/

    terminater_char      = data_ptr[param_len - 1];
    data_ptr[param_len - 1] = '\0';
    param_len             -= 1;

    audmod_arm_devmod_ptr = (AUDIO_NV_ARM_MODE_INFO_T *)SCI_ALLOC_BASEZ(sizeof(AUDIO_NV_ARM_MODE_INFO_T)+1);

    SCI_TRACE_LOW("ATC: siosadm for arm, param_len = %d",param_len);

    switch (terminater_char)
    {
        // cancel the operation
    case ESC:
        flag = TRUE;
        break;

    case CTRL_Z:
        if(param_len == sizeof(AUDIO_NV_ARM_MODE_STRUCT_T)*2)
        {
            dev_number = AUDIONVARM_GetModeNumber();
            if(s_atc_admm_index < dev_number)
            {
                name_ptr = AUDIONVARM_GetAudioModeName((AUDIO_DEVICE_MODE_TYPE_E)s_atc_admm_index);

                ConvertHexToBin(data_ptr, param_len, (uint8*)&audmod_arm_devmod_ptr->tAudioNvArmModeStruct);

                SCI_TRACE_LOW("ATC:ATC_ProcessSioARMSADMData %s, %s, %d.",
                    audmod_arm_devmod_ptr->ucModeName, name_ptr, strlen(name_ptr));

                name_len = strlen(name_ptr);
                if (AUDIO_NV_ARM_MODE_NAME_MAX_LEN > name_len)
                {
                    SCI_MEMCPY(audmod_arm_devmod_ptr->ucModeName, name_ptr, name_len);
                    audmod_arm_devmod_ptr->ucModeName[name_len] = '\0';
                }
                else
                {
                    SCI_TRACE_LOW("ATC: Audio Mode Name too long: %d", name_len);
                    flag = FALSE;
                    break;
                }

                SCI_TRACE_LOW("ATC:ATC_ProcessSioARMSADMData 2 %s, %s, %d.",
                    audmod_arm_devmod_ptr->ucModeName, name_ptr, strlen(name_ptr));

                result = AUDIONVARM_SetModeParam(name_ptr, audmod_arm_devmod_ptr);

                if(result != AUDIO_NV_ARM_NO_ERROR)
                {
                    SCI_TRACE_LOW("ATC: siosadm for arm, setmode param result = %d",result);
                    flag = FALSE;
                }
                else
                {
                    AUDIO_DEVICE_MODE_TYPE_E eMode = AUDIONVARM_GetAudioModeType(name_ptr);
                    AUDIO_DEVICE_MODE_TYPE_E eOldCurMode = AUDIO_DEVICE_MODE_HANDHOLD;
                    AUDIO_GetDevMode(&eOldCurMode);
                    AUDIO_SetDevMode(eMode);
                    SCI_TRACE_LOW("ATC:ATC_ProcessSioARMSADMData: setModeInfo, Mode:0x%x, pucModeName:0x%x, mode:%d, oldMode:%d.",
                        audmod_arm_devmod_ptr, name_ptr, eMode, eOldCurMode);
                    flag = TRUE;
                }

            }
            else
            {
                SCI_TRACE_LOW("ATC: siosadm for arm , input data index error.");
            }
        }
        else
        {
            SCI_TRACE_LOW("ATC: siosadm for arm , input data length error.");
        }

        break;
    default:
           SCI_TRACE_LOW("Lost the escape character is %s is line %d",__FILE__,__LINE__);
        break;
    }

    SCI_FREE(audmod_arm_devmod_ptr);

    if (flag)
    {
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
    }
    else
    {
        ATC_BuildResultCodeRsp(atc_config_ptr,S_ATC_RESULT_CODE_ERROR ,ERR_UNKNOWN);
    }

    return S_ATC_DEFAULT_HANDLE;
}


/*****************************************************************************/
//  Description : This function handles the data send from the SIO when AT+SADM
//  Global resource dependence : g_rsp_str
//  Author:
//  Note: new added for the function to read&write audio parameters into NV
/*****************************************************************************/
PUBLIC uint32 ATC_ProcessSioARMSADMNVData(
                ATC_CONFIG_T    *atc_config_ptr,
                uint8           *data_ptr,  // pointer to the data
                uint16          param_len
                )
{
    uint8        terminater_char;     // indicates ther terminater char
    uint16       data_length;
    BOOLEAN           flag = FALSE;
    uint32            mode_index = 0;
    uint32            dev_number = 0;
    AUDIO_NV_ARM_RESULT_E   result = AUDIO_NV_ARM_NO_ERROR;
    AUDIO_NV_ARM_MODE_INFO_T* audmod_arm_devmod_ptr = PNULL;
    char *name_ptr = PNULL;
    uint8  name_len = 0;

    SCI_ASSERT(PNULL != atc_config_ptr); /*assert verified*/

    param_len  -= 1;
    data_length = strlen((char*)data_ptr);
    terminater_char = data_ptr[data_length - 1];
    mode_index = s_atc_admm_index;
    SCI_TRACE_LOW("ATC:SioARMSADMNVData:length:%d,terminater_char:%d,mode index::%d",
           param_len,terminater_char,mode_index);

    switch (terminater_char)
    {
    // cancel the operation
    case ESC:
        flag = TRUE;
        SCI_TRACE_LOW("ATC:cancel the snvm audio data!");
        break;

    case CTRL_Z:
        if(param_len == sizeof(AUDIO_NV_ARM_MODE_STRUCT_T)*2)
        {
            dev_number = AUDIONVARM_GetModeNumber();
            if(s_atc_admm_index < dev_number)
            {
                name_ptr = AUDIONVARM_GetAudioModeName((AUDIO_DEVICE_MODE_TYPE_E )s_atc_admm_index);

                audmod_arm_devmod_ptr = (AUDIO_NV_ARM_MODE_INFO_T *)SCI_ALLOC_BASEZ(sizeof(AUDIO_NV_ARM_MODE_INFO_T)+1);

                SCI_TRACE_LOW("ATC:SioARMSADMNVData %s, %s, %d.",
                    audmod_arm_devmod_ptr->ucModeName, name_ptr, strlen(name_ptr));

                name_len = strlen(name_ptr);
                if (AUDIO_NV_ARM_MODE_NAME_MAX_LEN >= name_len)
                {
                    SCI_MEMCPY(audmod_arm_devmod_ptr->ucModeName, name_ptr, name_len);
                    audmod_arm_devmod_ptr->ucModeName[name_len] = '\0';
                }
                else
                {
                    SCI_TRACE_LOW("ATC: Audio Mode Name too long: %d", name_len);
                    flag = FALSE;
                    break;
                }

                SCI_TRACE_LOW("ATC:SioARMSADMNVData 2 %s, %s, %d.",
                    audmod_arm_devmod_ptr->ucModeName, name_ptr, strlen(name_ptr));

                if (!ConvertHexToBin(data_ptr, param_len, (uint8*)&audmod_arm_devmod_ptr->tAudioNvArmModeStruct))
                {
                    SCI_TRACE_LOW("ATC:ConvertHexToBin failed.");
                    break;
                }

                result = AUDIONVARM_WriteModeParamToFlash((char *)audmod_arm_devmod_ptr->ucModeName, audmod_arm_devmod_ptr);

                if (AUDMOD_NO_ERROR != result)
                {
                    SCI_TRACE_LOW("ATC:SioARMSADMNVData failed. result:%d",result);
                    break;
                }
                flag = TRUE;
            }
            else
            {
                   SCI_TRACE_LOW("ATC: siosadm for arm , input data index error.");
            }
        }
        else
        {
            SCI_TRACE_LOW("ATC: siosadmnv for arm, input audio data length error.length:%d != %d",
                            param_len,sizeof(AUDIO_NV_ARM_MODE_STRUCT_T)*2);
        }
        break;

        default:
          SCI_TRACE_LOW("ATC: snvm audio data format is error.");
          break;
    }

    if (PNULL != audmod_arm_devmod_ptr)
    {
        SCI_FREE(audmod_arm_devmod_ptr);
    }

    if (flag)
    {
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
    }
    else
    {
        ATC_BuildResultCodeRsp(atc_config_ptr,S_ATC_RESULT_CODE_ERROR ,ERR_UNKNOWN);
    }

    return S_ATC_DEFAULT_HANDLE;

}

/*****************************************************************************/
//  Description : This function handles the data send from the SIO when AT+SADM
//  Global resource dependence : g_rsp_str
//  Author: longting.zhao
//  Note: new added for the function to read&write audio parameters into NV
/*****************************************************************************/
PUBLIC uint32 ATC_ProcessSioDSPSADMData(
                ATC_CONFIG_T    *atc_config_ptr,
                uint8           *data_ptr,  // pointer to the data
                uint16          param_len
                )
{
    uint8             terminater_char;     // indicates ther terminater char
    BOOLEAN           flag = FALSE;
    uint32            dev_number = 0;
    uint32            mode_index = s_atc_admm_index;

    AUDIO_NV_DSP_RESULT_E   result = AUDIO_NV_DSP_NO_ERROR;

    AUDIO_NV_DSP_MODE_INFO_T* audmod_dsp_devmod_ptr = PNULL;
    AUDIO_NV_DSP_MODE_INFO_T * audmod_devmod_ptr = NULL;
    uint8  name_len = 0;

    SCI_ASSERT(PNULL != atc_config_ptr); /*assert verified*/

    terminater_char      = data_ptr[param_len - 1];
    data_ptr[param_len - 1] = '\0';
    param_len             -= 1;

    audmod_dsp_devmod_ptr = (AUDIO_NV_DSP_MODE_INFO_T *)SCI_ALLOC_BASEZ(sizeof(AUDIO_NV_DSP_MODE_INFO_T)+1);

    SCI_TRACE_LOW("ATC: siosadm for dsp, param_len = %d,terminater_char=%d", param_len, terminater_char);

    switch (terminater_char)
    {
        // cancel the operation
        case ESC:
            flag = TRUE;
            break;

        case CTRL_Z:
        {
            SCI_TRACE_LOW("atc: sizeof(AUDIO_NV_DSP_MODE_STRUCT_T)=%d", sizeof(AUDIO_NV_DSP_MODE_STRUCT_T));
            if(param_len == sizeof(AUDIO_NV_DSP_MODE_STRUCT_T)*2)
            {
                dev_number = AUDIONVDSP_GetModeNumber();
                if(s_atc_admm_index < dev_number)
                {
                    audmod_devmod_ptr = AUDIONVDSP_GetFirstDevMode();

                    while(mode_index)
                    {
                        audmod_devmod_ptr =  AUDIONVDSP_GetNextDevMode(audmod_devmod_ptr);
                        mode_index--;
                    }

                    ConvertHexToBin(data_ptr, param_len, (uint8*)&audmod_dsp_devmod_ptr->tAudioNvDspModeStruct);

                    SCI_TRACE_LOW("ATC:ATC_ProcessSioDSPSADMData %s, %d.", audmod_dsp_devmod_ptr->ucModeName, strlen((char *)audmod_dsp_devmod_ptr->ucModeName));

                    name_len = strlen((char *)audmod_devmod_ptr->ucModeName);
                    if (name_len < AUDIO_NV_DSP_MODE_NAME_MAX_LEN)
                    {
                        SCI_MEMCPY(audmod_dsp_devmod_ptr->ucModeName, audmod_devmod_ptr->ucModeName, name_len);
                        audmod_dsp_devmod_ptr->ucModeName[name_len] = '\0';
                    }
                    else
                    {
                        SCI_TRACE_LOW("ATC: Audio Mode Name too long: %d", name_len);
                        flag = FALSE;
                        break;
                    }

                    SCI_TRACE_LOW("ATC:ATC_ProcessSioDSPSADMData 2, %s, %d.", audmod_dsp_devmod_ptr->ucModeName, strlen((char *)audmod_dsp_devmod_ptr->ucModeName));

                    result = AUDIONVDSP_SetModeParam((char *)audmod_dsp_devmod_ptr->ucModeName, audmod_dsp_devmod_ptr);
                    if(result != AUDIO_NV_DSP_NO_ERROR)
                    {
                        SCI_TRACE_LOW("ATC: siosadm for dsp, setmode param result = %d", result);
                        flag = FALSE;
                    }
                    else
                    {
                        AUDIO_DEVICE_MODE_TYPE_E eMode = AUDIONVDSP_GetAudioModeType((char *)audmod_dsp_devmod_ptr->ucModeName);
                        AUDIO_DEVICE_MODE_TYPE_E eOldCurMode = AUDIO_DEVICE_MODE_HANDHOLD;
                        AUDIO_GetDevMode(&eOldCurMode);
                        AUDIO_SetDevModeEx(INVALID_HANDLE, eMode);
                        SCI_TRACE_LOW("ATC:ATC_ProcessSioDSPSADMData: setModeInfo, Mode:0x%x, pucModeName:%s, mode:%d, oldMode:%d.", audmod_dsp_devmod_ptr, audmod_dsp_devmod_ptr->ucModeName, eMode, eOldCurMode);
                        flag = TRUE;
                    }

                }
                else
                {
                    SCI_TRACE_LOW("ATC: siosadm for dsp , input data index error.");
                }
            }
            else
            {
                SCI_TRACE_LOW("ATC: siosadm for dsp , input data length error.");
            }

            break;
        }

        default:
        {
            SCI_TRACE_LOW("Lost the escape character is %s is line %d");
            break;
        }
    }

    SCI_FREE(audmod_dsp_devmod_ptr);

    if ( flag )
    {
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
    }
    else
    {
        ATC_BuildResultCodeRsp(atc_config_ptr,S_ATC_RESULT_CODE_ERROR ,ERR_UNKNOWN);
    }

    return S_ATC_DEFAULT_HANDLE;
}


/*****************************************************************************/
//  Description : This function handles the data send from the SIO when AT+SADM
//  Global resource dependence : g_rsp_str
//  Author: longting.zhao
//  Note: new added for the function to read&write audio parameters into NV
/*****************************************************************************/
PUBLIC uint32 ATC_ProcessSioDSPSADMNVData(
                      ATC_CONFIG_T    *atc_config_ptr,
                uint8           *data_ptr,  // pointer to the data
                uint16          param_len
                )
{
    uint8        terminater_char;     // indicates ther terminater char
    uint16       data_length;
    BOOLEAN           flag = FALSE;
    uint32            mode_index = 0;
    uint32            dev_number = 0;
    AUDIO_NV_DSP_RESULT_E   result = AUDIO_NV_DSP_NO_ERROR;
    AUDIO_NV_DSP_MODE_INFO_T* audmod_dsp_devmod_ptr = PNULL;
    AUDIO_NV_DSP_MODE_INFO_T * audmod_devmod_ptr = NULL;
    int name_len = 0;

    SCI_ASSERT(PNULL != atc_config_ptr); /*assert verified*/

    param_len  -= 1;
    data_length = strlen((char*)data_ptr);
    terminater_char = data_ptr[data_length - 1];
    mode_index = s_atc_admm_index;
    SCI_TRACE_LOW("ATC:SioDSPSADMNVData:length:%d,terminater_char:%d,mode index::%d", param_len,terminater_char,mode_index);

    switch (terminater_char)
    {
        // cancel the operation
        case ESC:
        {
            flag = TRUE;
            SCI_TRACE_LOW("ATC:cancel the snvm audio data!");
            break;
        }

        case CTRL_Z:
        {
            if(param_len == sizeof(AUDIO_NV_DSP_MODE_STRUCT_T)*2)
            {
                dev_number = AUDIONVDSP_GetModeNumber();
                if(s_atc_admm_index < dev_number)
                {
                    audmod_devmod_ptr = AUDIONVDSP_GetFirstDevMode();

                    while(mode_index)
                    {
                        audmod_devmod_ptr =  AUDIONVDSP_GetNextDevMode(audmod_devmod_ptr);
                        mode_index--;
                    }

                    audmod_dsp_devmod_ptr = (AUDIO_NV_DSP_MODE_INFO_T *)SCI_ALLOC_BASEZ(sizeof(AUDIO_NV_DSP_MODE_INFO_T)+1);

                    SCI_TRACE_LOW("ATC:SioDSPSADMNVData %s, %d.", audmod_devmod_ptr->ucModeName, strlen((char *)audmod_devmod_ptr->ucModeName));

                    name_len = strlen((char *)audmod_devmod_ptr->ucModeName);
                    if (name_len < AUDIO_NV_DSP_MODE_NAME_MAX_LEN)
                    {
                        SCI_MEMCPY(audmod_dsp_devmod_ptr->ucModeName, audmod_devmod_ptr->ucModeName, name_len);
                        audmod_devmod_ptr->ucModeName[name_len] = '\0';
                    }
                    else
                    {
                        SCI_TRACE_LOW("ATC: Audio Mode Name too long: %d", name_len);
                        flag = FALSE;
                        break;
                    }

                    if (!ConvertHexToBin(data_ptr, param_len, (uint8*)&audmod_dsp_devmod_ptr->tAudioNvDspModeStruct))
                    {
                        SCI_TRACE_LOW("ATC:ConvertHexToBin failed.");
                        break;
                    }

                    result = AUDIONVDSP_WriteModeParamToFlash((char *)audmod_devmod_ptr->ucModeName, audmod_dsp_devmod_ptr);
                    if (AUDMOD_NO_ERROR != result)
                    {
                        SCI_TRACE_LOW("ATC:SioDSPSADMNVData failed. result:%d", result);
                        break;
                    }
                    flag = TRUE;
                }
                else
                {
                    SCI_TRACE_LOW("ATC: siosadm for dsp , input data index error.");
                }
            }
            else
            {
                SCI_TRACE_LOW("ATC: siosadmnv for dsp, input audio data length error.length:%d != %d", param_len,sizeof(AUDIO_NV_DSP_MODE_STRUCT_T)*2);
            }
            break;
        }

        default:
        {
            SCI_TRACE_LOW("ATC: snvm audio data format is error.");
            break;
        }
    }

    if (PNULL != audmod_dsp_devmod_ptr)
    {
        SCI_FREE(audmod_dsp_devmod_ptr);
    }

    if (flag)
    {
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
    }
    else
    {
        ATC_BuildResultCodeRsp(atc_config_ptr,S_ATC_RESULT_CODE_ERROR ,ERR_UNKNOWN);
    }

    return S_ATC_DEFAULT_HANDLE;

}
#endif/*AUDIO_PARAM_ARM*/

/*****************************************************************************/
//  Description : This function handles the data send from the SIO when AT+SADM
//  Global resource dependence : g_rsp_str
//  Author:
//  Note:
/*****************************************************************************/
PUBLIC uint32 ATC_ProcessSioSADMData(
    ATC_CONFIG_T    *atc_config_ptr,
    uint8           *data_ptr,  // pointer to the data
    uint16          param_len
)
{
    uint8             terminater_char;     // indicates ther terminater char
    BOOLEAN           flag = FALSE;
    uint32            dev_number = 0;
    AUDMOD_DEVMOD_T   *audio_data_ptr;
    AUDMOD_DEVMOD_T  *audmod_devmod_ptr = NULL;
    AUDMOD_RESULT_E   result;

    if (!atc_config_ptr)
    {
        ATC_TRACE_LOW("ATC: ATC_ProcessSioSADMData  atc_config_ptr pointer is PNULL");
        return S_ATC_FAIL;
    }

    if (!data_ptr)
    {
        ATC_TRACE_LOW("ATC: ATC_ProcessSioSADMData  data_ptr pointer is PNULL");
        return S_ATC_FAIL;
    }

    terminater_char      = data_ptr[param_len - 1];
    data_ptr[param_len - 1] = '\0';
    param_len             -= 1;

    audio_data_ptr = (AUDMOD_DEVMOD_T *)SCI_ALLOC_BASE(sizeof(AUDMOD_DEVMOD_T) + 1);

    SCI_MEMSET((uint8 *)audio_data_ptr, 0, (sizeof(AUDMOD_DEVMOD_T) + 1));
    ATC_TRACE_LOW("ATC: siosadm param_len = %d", param_len);

    switch(terminater_char)
    {
            // cancel the operation
        case ESC:
            flag = TRUE;
            break;

        case CTRL_Z:

            if(param_len == sizeof(AUDIO_STRUCT_T) * 2)
            {
                dev_number = AUDMOD_GetDevModeNumber();

                if(s_atc_admm_index < dev_number)
                {
                    flag = TRUE;
                    audmod_devmod_ptr = AUDMOD_GetFirstDevMode();
                    if (!audmod_devmod_ptr)
                    {
                        ATC_TRACE_LOW("ATC: ATC_ProcessSioSADMData  audmod_devmod_ptr pointer is PNULL");
                        return ERR_OPERATION_NOT_SUPPORTED;
                    }

                    while(s_atc_admm_index)
                    {
                        audmod_devmod_ptr = AUDMOD_GetNextDevMode(audmod_devmod_ptr);
                        s_atc_admm_index--;
                    }

                    ConvertHexToBin(data_ptr, param_len, (uint8 *)&audio_data_ptr->tAudioStruct);
                #ifdef AUDIO_PARAM_ARM
                    SCI_MEMCPY(audio_data_ptr->ucModeName, audmod_devmod_ptr->ucModeName, AUDMOD_NAME_MAX_LEN);
                #else
                    SCI_MEMCPY(audio_data_ptr->ucModeName, audmod_devmod_ptr->ucModeName, NAME_MAX_LEN);
                #endif

                    result = AUDMOD_SetModeParam(audmod_devmod_ptr->ucModeName, audio_data_ptr);

                    if(result != AUDMOD_NO_ERROR)
                    {
                        ATC_TRACE_LOW("ATC: siosadm setmode param result = %d", result);
                        flag = FALSE;
                    }
                }
            }
            else
            {
                ATC_TRACE_LOW("ATC: siosadm input data length error.");
            }

            break;
        default:
            ATC_TRACE_LOW("Lost the escape character is %s is line %d", __FILE__, __LINE__);
            break;
    }

    SCI_FREE(audio_data_ptr);

    if(flag)
    {
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
    }
    else
    {
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_ERROR , ERR_UNKNOWN);
    }

    return S_ATC_DEFAULT_HANDLE;
}

/*****************************************************************************/
//  Description : This function handles the data send from the SIO when AT+SADM
//  Global resource dependence : g_rsp_str
//  Author:
//  Note: new added for the function to read&write audio parameters into NV
/*****************************************************************************/
PUBLIC uint32 ATC_ProcessSioSADMNVData(
    ATC_CONFIG_T    *atc_config_ptr,
    uint8           *data_ptr,  // pointer to the data
    uint16          param_len
)
{
    uint8        terminater_char;     // indicates ther terminater char
    uint16       data_length;

    BOOLEAN           flag = FALSE;
    uint32            mode_index = 0;
    AUDMOD_DEVMOD_T   *audio_data_ptr = NULL;
    AUDMOD_DEVMOD_T   *audio_temp_ptr = NULL;
    AUDMOD_RESULT_E   result;

    if (!atc_config_ptr)
    {
        ATC_TRACE_LOW("ATC: ATC_ProcessSioSADMNVData  atc_config_ptr pointer is PNULL");
        return S_ATC_FAIL;
    }

    if (!data_ptr)
    {
        ATC_TRACE_LOW("ATC: ATC_ProcessSioSADMNVData  data_ptr pointer is PNULL");
        return S_ATC_FAIL;
    }

    param_len             -= 1;
    data_length = strlen((char *)data_ptr);
    terminater_char = data_ptr[data_length - 1];
    mode_index = s_atc_admm_index;
    ATC_TRACE_LOW("ATC: ATC_ProcessSioSADMNVData: length:%d,terminater_char:%d,mode index:%d",
                                    param_len, terminater_char, mode_index);

    switch(terminater_char)
    {
            // cancel the operation
        case ESC:
            flag = TRUE;
            ATC_TRACE_LOW("ATC:cancel the snvm audio data!");
            break;

        case CTRL_Z:

            if(param_len == sizeof(AUDIO_STRUCT_T) * 2)
            {
                /*get audio name according to audio mode index*/
                audio_temp_ptr = AUDMOD_GetFirstDevMode();

                while(mode_index)
                {
                    audio_temp_ptr = AUDMOD_GetNextDevMode(audio_temp_ptr);
                    mode_index--;
                }

                /*get audio data*/
                audio_data_ptr = (AUDMOD_DEVMOD_T *)SCI_ALLOC_BASE(sizeof(AUDMOD_DEVMOD_T) + 1);
                SCI_MEMSET((uint8 *)audio_data_ptr, 0, (sizeof(AUDMOD_DEVMOD_T) + 1));

                //ATC_TRACE_LOW("ATC:process the command!");
                if(!ConvertHexToBin(data_ptr, param_len, (uint8 *)&audio_data_ptr->tAudioStruct))
                {
                    ATC_TRACE_LOW("ATC:ConvertHexToBin failed.");
                    break;
                }
                #ifdef AUDIO_PARAM_ARM
                SCI_MEMCPY(audio_data_ptr->ucModeName, audio_temp_ptr->ucModeName, AUDMOD_NAME_MAX_LEN);
                #else
                SCI_MEMCPY(audio_data_ptr->ucModeName, audio_temp_ptr->ucModeName, NAME_MAX_LEN);
                #endif

                /*write audio param data to flash*/
#ifndef WIN32
                result = PROD_WriteAudioParamToFlash(audio_temp_ptr->ucModeName, audio_data_ptr);
#else
                //windows simulator
                break;
#endif
                if(AUDMOD_NO_ERROR != result)
                {
                    ATC_TRACE_LOW("ATC:PROD_WriteAudioParamToFlash failed. result:%d", result);
                    break;
                }

                flag = TRUE;
            }
            else
            {
                ATC_TRACE_LOW("ATC: siosadmnv input audio data length error.length:%d != %d",
                param_len, sizeof(AUDIO_STRUCT_T) * 2);
            }

            break;

        default:
            ATC_TRACE_LOW("ATC: snvm audio data format is error.");
            break;
    }

    if(NULL != audio_data_ptr)
    {
        SCI_FREE(audio_data_ptr);
    }

    if(flag)
    {
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
    }
    else
    {
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_ERROR , ERR_UNKNOWN);
    }

    return S_ATC_DEFAULT_HANDLE;
}

/*****************************************************************************/
//Description : This function handles the data send from the SIO when AT+SPADLVVE
//Global resource dependence : g_rsp_str
//  Author: chong.liu
//Note:
/*****************************************************************************/
PUBLIC uint32 ATC_ProcessSioSADLVVEData(
                ATC_CONFIG_T    *atc_config_ptr,
                uint8           *data_ptr,  // pointer to the data
                uint16          param_len
                )
{
#ifndef WIN32
    uint8             terminater_char;     // indicates ther terminater char
    BOOLEAN           flag = FALSE;
    AUDMOD_RESULT_E   result;

    LVVE_CTRL_PARAM_NV_T *lvve_ctrl_param_nv_ptr = PNULL;
    uint8 *name_ptr = PNULL;

    SCI_ASSERT(PNULL != atc_config_ptr); /*assert verified:null pointer*/
    SCI_ASSERT(PNULL != data_ptr); /*assert verified:null pointer*/

    terminater_char      = data_ptr[param_len - 1];
    data_ptr[param_len - 1] = '\0';
    param_len             -= 1;

    lvve_ctrl_param_nv_ptr = (LVVE_CTRL_PARAM_NV_T *)SCI_ALLOC_BASEZ(sizeof(LVVE_CTRL_PARAM_NV_T)+1);

    SCI_TRACE_LOW("ATC: siosadm for arm, param_len = %d", param_len);

    switch (terminater_char)
    {
        case ESC:
        {
            flag = TRUE;
            break;
        }

        case CTRL_Z:
        {
            uint8 i = 0;
            SCI_TRACE_LOW("atc:ctrl_z sizeof(LVVE_CTRL_PARAM_NV_T)*2=%d", sizeof(LVVE_CTRL_PARAM_NV_T)*2);
            if(param_len == sizeof(LVVE_CTRL_PARAM_NV_T)*2)
            {
                name_ptr =(uint8 *)LVVE_CTRL_PARAM_GetDevModeName((AUDIO_DEVICE_MODE_TYPE_E)s_atc_admm_index);
                SCI_TRACE_LOW("ATC:ATC_ProcessSioSADLVVEData %s, %d", name_ptr, strlen((char *)name_ptr));

                SCI_ASSERT(name_ptr != NULL);/*assert verified:null pointer*/

                ConvertHexToBin(data_ptr, param_len, (uint8*)lvve_ctrl_param_nv_ptr);

                lvve_ctrl_param_nv_ptr->ucModeName[strlen((char *)name_ptr)] = '\0';

                SCI_TRACE_LOW("atc: lvve_ctrl_param_nv_ptr->name len=%d", strlen((char *)(lvve_ctrl_param_nv_ptr->ucModeName)));
                for(i=0; i<strlen((char *)(lvve_ctrl_param_nv_ptr->ucModeName)); i++)
                {
                    SCI_TRACE_LOW("atc: ->name [%d]=%d ", i,  (lvve_ctrl_param_nv_ptr->ucModeName[i]));
                }


                result = LVVE_CTRL_PARAM_SetDevModeParam(name_ptr, lvve_ctrl_param_nv_ptr);

                if(result != AUDIO_NV_ARM_NO_ERROR)
                {
                    SCI_TRACE_LOW("ATC: LVVE_CTRL_PARAM_SetDevModeParam , result = %d", result);
                    flag = FALSE;
                }
                else
                {
                    flag = TRUE;
                }
            }
            else
            {
                SCI_TRACE_LOW("ATC: ATC_ProcessSioSADLVVEData, input data length error.");
            }

            break;
        }

        default:
        {
            SCI_TRACE_LOW("Lost the escape character is %s is line %d", __FILE__,__LINE__);
            break;
        }
    }

    SCI_FREE(lvve_ctrl_param_nv_ptr);

    if (flag)
    {
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
    }
    else
    {
        ATC_BuildResultCodeRsp(atc_config_ptr,S_ATC_RESULT_CODE_ERROR ,ERR_UNKNOWN);
    }
#endif
    return S_ATC_DEFAULT_HANDLE;
}

/*****************************************************************************/
//Description : This function handles the data send from the SIO when AT+SPADLVVE
//Global resource dependence : g_rsp_str
//  Author: chong.liu
//Note:
/*****************************************************************************/
PUBLIC uint32 ATC_ProcessSioSADLVVENVData(
                ATC_CONFIG_T    *atc_config_ptr,
                uint8           *data_ptr,  // pointer to the data
                uint16          param_len
                )
{
#ifndef WIN32
    uint8             terminater_char;     // indicates ther terminater char
    BOOLEAN           flag = FALSE;
    AUDMOD_RESULT_E   result;

    LVVE_CTRL_PARAM_NV_T *lvve_ctrl_param_nv_ptr = PNULL;
    uint8 *name_ptr = PNULL;

    SCI_ASSERT(PNULL != atc_config_ptr); /*assert verified:null pointer*/
    SCI_ASSERT(PNULL != data_ptr); /*assert verified:null pointer*/

    terminater_char      = data_ptr[param_len - 1];
    data_ptr[param_len - 1] = '\0';
    param_len  = param_len - 1;

    lvve_ctrl_param_nv_ptr = (LVVE_CTRL_PARAM_NV_T *)SCI_ALLOC_BASEZ(sizeof(LVVE_CTRL_PARAM_NV_T)+1);

    SCI_TRACE_LOW("ATC: siosadm for arm, param_len = %d", param_len);

    switch (terminater_char)
    {
        case ESC:
        flag = TRUE;
        break;

        case CTRL_Z:
        {
            SCI_TRACE_LOW("atc: ctrl_z, sizeof(LVVE_CTRL_PARAM_NV_T)*2=%d", sizeof(LVVE_CTRL_PARAM_NV_T)*2);
            if(param_len == sizeof(LVVE_CTRL_PARAM_NV_T)*2)
            {
                name_ptr = (uint8*)LVVE_CTRL_PARAM_GetDevModeName((AUDIO_DEVICE_MODE_TYPE_E)s_atc_admm_index);

                SCI_ASSERT(name_ptr != NULL);/*assert verified:null pointer*/

                ConvertHexToBin(data_ptr, param_len, (uint8*)lvve_ctrl_param_nv_ptr);

                SCI_TRACE_LOW("ATC:ATC_ProcessSioSADLVVENVData %s, %d", name_ptr, strlen((char *)name_ptr));

                lvve_ctrl_param_nv_ptr->ucModeName[strlen((char *)name_ptr)] = '\0';

                result = LVVE_CTRL_PARAM_WriteModeParamToFlash(name_ptr, lvve_ctrl_param_nv_ptr);
                if(result != AUDIO_NV_ARM_NO_ERROR)
                {
                    SCI_TRACE_LOW("ATC: ATC_ProcessSioSADLVVENVData , result = %d", result);
                    flag = FALSE;
                }
                else
                {
                    flag = TRUE;
                }
            }
            else
            {
                SCI_TRACE_LOW("ATC: ATC_ProcessSioSADLVVEData, input data length error.");
            }

            break;
        }

        default:
        {
            SCI_TRACE_LOW("Lost the escape character is %s is line %d", __FILE__,__LINE__);
            break;
        }
    }

    SCI_FREE(lvve_ctrl_param_nv_ptr);

    if (flag)
    {
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
    }
    else
    {
        ATC_BuildResultCodeRsp(atc_config_ptr,S_ATC_RESULT_CODE_ERROR ,ERR_UNKNOWN);
    }
#endif
    return S_ATC_DEFAULT_HANDLE;
}

/*****************************************************************************/
//  Description : This function handle the AT+IPR command, this command is used
//  to set or read the bandrate of the uart.
//  Global resource dependence : none
//  Author:       Xueli.lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessIPR)
{
    uint32        baud_rate;
    uint32        status = ERR_OPERATION_NOT_ALLOWED;
    int32         i;

#ifndef _ULTRA_LOW_CODE_

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:

            if(PARAM1_FLAG)
            {
                baud_rate = PARA1.num;

                for(i = 0; i < AT_IPR_MAX; i++)
                {
                    if(c_baudrate_data[i].atc_val == baud_rate)
                    {
                        baud_rate = c_baudrate_data[i].reg_val;

                        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
                        SCI_SLEEP(100);
#ifdef _MUX_ENABLE_
                        MUX_Setbaud_rate(baud_rate);
#else
                        SIO_SetBaudRate(COM_USER, baud_rate);
#endif
                        status = S_ATC_DEFAULT_HANDLE;
                        break;
                    }
                }
            }
            else
            {
                status = ERR_INVALID_INDEX;
            }

            break;

        case ATC_CMD_TYPE_READ:
#ifdef _MUX_ENABLE_
            baud_rate = MUX_Getbaud_rate();
#else
            baud_rate = SIO_GetBaudRate(COM_USER);
#endif

            for(i = 0; i < AT_IPR_MAX; i++)
            {
                if(baud_rate == c_baudrate_data[i].reg_val)
                {
                    baud_rate = c_baudrate_data[i].atc_val;
                    sprintf((char *)g_rsp_str, "%s: %ld", g_atc_info_table[AT_CMD_IPR].cmd_name->str_ptr,    baud_rate);
                    ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
                    status = S_ATC_SUCCESS;
                    break;
                }
            }

            break;

        default:
            status = ERR_OPERATION_NOT_SUPPORTED;
            break;
    }

#endif /* _ULTRA_LOW_CODE_ */

    return status;
}

/*****************************************************************************/
//  Description : This function handles the APP_MN_SIM_GET_PIN_IND signal
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
ATC_STATUS ATC_ProcessPinInd(
    ATC_CONFIG_T                   *atc_config_ptr,
    APP_MN_SIM_GET_PIN_IND_T       *sig_ptr)
{
    MN_DUAL_SYS_E     dual_sys = sig_ptr->dual_sys;

    ATC_UpECINDInfo(dual_sys, atc_config_ptr, (xSignalHeaderRec *)sig_ptr, 3, 3, 0, 0, 0);

    atc_config_ptr->sim_flag[dual_sys] = TRUE;

    switch(sig_ptr->pin_num)
    {
        case MNSIM_PIN1:

            if(sig_ptr->pin_is_blocked)
            {
                atc_config_ptr->cpin[dual_sys] = ATC_CPIN_CODE_PUK1;
                if(atc_config_ptr->cpin_flag[dual_sys])
                {
                    ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_ERROR , ERR_SIM_PUK_REQUIRED);
                    atc_config_ptr->cpin_flag[dual_sys] = FALSE;
                }
            }
            else
            {
                atc_config_ptr->cpin[dual_sys] = ATC_CPIN_CODE_PIN1;

                if(atc_config_ptr->cpin_flag[dual_sys])
                {
                    /*ATC_BuildResultCodeRsp(atc_config_ptr,S_ATC_RESULT_CODE_ERROR ,ERR_SIM_PIN_REQUIRED);*/
                    ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_ERROR , ERR_INCORRECT_PASSWORD);
                    atc_config_ptr->cpin_flag[dual_sys] = FALSE;
                }
            }

            break;

        case MNSIM_PIN2:

            if(sig_ptr->pin_is_blocked)
            {
                atc_config_ptr->cpin2[dual_sys] = ATC_CPIN_CODE_PUK2;

                if(atc_config_ptr->cpin_flag[dual_sys])
                {
                    ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_ERROR , ERR_SIM_PUK2_REQUIRED);
                    atc_config_ptr->cpin_flag[dual_sys] = FALSE;
                }
            }
            else
            {
                atc_config_ptr->cpin2[dual_sys] = ATC_CPIN_CODE_PIN2;

                if(atc_config_ptr->cpin_flag[dual_sys])
                {
                    ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_ERROR , ERR_SIM_PIN2_REQUIRED);
                    atc_config_ptr->cpin_flag[dual_sys] = FALSE;
                }
            }

            break;

        default:
            break;
    }

    return S_ATC_SUCCESS;
}
/*****************************************************************************/
//  Description : This function handles the APP_MN_AOC_INCREASE_ACM_IND signal
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
ATC_STATUS ATC_ProcessIncreaseAcmInd(
    ATC_CONFIG_T                   *atc_config_ptr,
    APP_MN_AOC_MODIFIED_IND_T      *sig_ptr
)
{
    uint32            ccm_val;

    ccm_val = sig_ptr->acm_unit;
    atc_config_ptr->ccm_val = sig_ptr->acm_unit;

    switch(atc_config_ptr->aoc_mode)
    {
        case ATC_AOC_QUERY_MODE:
        case ATC_AOC_DEACTIVE_MODE:
            break;

        case ATC_AOC_ACTIVE_MODE:
            sprintf((char *)g_rsp_str, "+CCCM: %lx", ccm_val);
            ATC_BuildUnsolicitedInfo(sig_ptr->dual_sys, atc_config_ptr, g_rsp_str);
            break;
        default:
            return ERR_OPERATION_NOT_SUPPORTED;
    }

    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handles the APP_MN_SIM_NOT_READY_IND signal
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
ATC_STATUS ATC_ProcessNotReadyInd(
    ATC_CONFIG_T                    *atc_config_ptr,
    APP_MN_SIM_NOT_READY_IND_T      *sig_ptr
)
{
    MN_DUAL_SYS_E     dual_sys = sig_ptr->dual_sys;
    ATC_TRACE_LOW("ATC:No sim reason = %d", sig_ptr->reason);
    atc_config_ptr->sim_err_reason[dual_sys] = sig_ptr->reason;

    if(atc_config_ptr->cpin_flag[dual_sys])
    {
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_ERROR , ERR_SIM_FAILURE);
        atc_config_ptr->cpin_flag[dual_sys] = FALSE;
    }

    switch(sig_ptr->reason)
    {
        case MNSIM_ERROR_REJECT_BY_NET:
            atc_config_ptr->sim_flag[dual_sys] = TRUE;
            ATC_UpECINDInfo(dual_sys, atc_config_ptr, (xSignalHeaderRec *)sig_ptr, 3, 0, 0, 0, 0);
            break;

        case MNSIM_ERROR_PIN1_BLOCKED:
            atc_config_ptr->cpin[dual_sys]     = ATC_CPIN_CODE_PUK1;
            atc_config_ptr->sim_flag[dual_sys] = TRUE;
            atc_config_ptr->sim_ready[dual_sys] = FALSE;
            ATC_UpECINDInfo(dual_sys, atc_config_ptr, (xSignalHeaderRec *)sig_ptr, 3, 1, 0, 0, 0);
            break;

        case MNSIM_ERROR_PIN2_BLOCKED:
            atc_config_ptr->cpin2[dual_sys]     = ATC_CPIN_CODE_PUK2;
            ATC_UpECINDInfo(dual_sys, atc_config_ptr, (xSignalHeaderRec *)sig_ptr, 3, 3, 0, 0, 0);
            break;

        case MNSIM_ERROR_PIN2_UNBLOCK_BLOCKED:
            atc_config_ptr->cpin2[dual_sys] = ATC_CPIN_CODE_BLOCK;
            ATC_UpECINDInfo(dual_sys, atc_config_ptr, (xSignalHeaderRec *)sig_ptr, 3, 3, 0, 0, 0);
            break;

        case MNSIM_ERROR_PIN1_UNBLOCK_BLOCKED:
            atc_config_ptr->sim_flag[dual_sys] = TRUE;
            atc_config_ptr->sim_ready[dual_sys] = FALSE;
            atc_config_ptr->cpin[dual_sys] = ATC_CPIN_CODE_BLOCK;
            ATC_UpECINDInfo(dual_sys, atc_config_ptr, (xSignalHeaderRec *)sig_ptr, 3, 1, 0, 0, 0);
            break;

        case MNSIM_ERROR_REMOVED:
        case MNSIM_ERROR_SERVICE_NOT_AVAILABLE:

        case MNSIM_ERROR_PIN1_UNINITIALIZED:
        case MNSIM_ERROR_PIN2_UNINITIALIZED:

        case MNSIM_ERROR_POWER_OFF:
        case MNSIM_ERROR_NO_SECRETCODE_INITIALIZED:
        case MNSIM_ERROR_INVALID_STATUS_RSP:
        case MNSIM_ERROR_INSERTED_ERROR:
        case MNSIM_ERROR_INITIALISE_FAILED:
        case MNSIM_ERROR_MEMORY_PROBLEM:
        case MNSIM_ERROR_INVALID_PARAMS:
        case MNSIM_ERROR_FILE_INVALIDATED:
        case MNSIM_ERROR_FILE_NOT_FOUND:
        case MNSIM_ERROR_GENERAL_FAULT:
        case MNSIM_ERROR_ALLOC_ERROR:
        case MNSIM_ERROR_SM_FAULT:
        case MNSIM_ERROR_SERIOUS:
        case MNSIM_ERROR_UNKNOWN:
        case MNSIM_ERROR_NULL:
        default :
            atc_config_ptr->sim_flag[dual_sys] = FALSE;
            atc_config_ptr->sim_ready[dual_sys] = FALSE;
            atc_config_ptr->cpin[dual_sys] = ATC_CPIN_CODE_MAX;
            ATC_SmsSimRemovedInd(dual_sys, atc_config_ptr);
            ATC_UpECINDInfo(dual_sys, atc_config_ptr, (xSignalHeaderRec *)sig_ptr, 3, 1, 0, 0, 0);
            break;
    }

    //Send the unsolicited +SIND: 0 and 10
    ATC_UpSINDInfo(dual_sys,
        atc_config_ptr,
        (xSignalHeaderRec *)sig_ptr,
        ATC_WIND_BIT0,
        0,
        (BOOLEAN)(0 != atc_config_ptr->atc_config_nv.creg));
    ATC_UpSINDInfo(dual_sys,
        atc_config_ptr,
        (xSignalHeaderRec *)sig_ptr,
        ATC_WIND_BIT9,
        10,
        (BOOLEAN)(0 != atc_config_ptr->atc_config_nv.creg));

    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handles the APP_MN_SIM_READY_IND signal
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
ATC_STATUS ATC_ProcessReadyInd(
    ATC_CONFIG_T                  *atc_config_ptr,
    APP_MN_SIM_READY_IND_T        *sig_ptr)
{
    MN_DUAL_SYS_E     dual_sys = sig_ptr->dual_sys;


    atc_config_ptr->sim_ready[dual_sys] = TRUE;
    g_received_sim_ready[dual_sys] = TRUE;
    atc_config_ptr->sim_flag[dual_sys] = TRUE;

    if(sig_ptr->pin1_is_enabled)
    {
        atc_config_ptr->pin1_enable_flag[dual_sys] = TRUE;
    }
    atc_config_ptr->cpin[dual_sys]     = ATC_CPIN_CODE_READY;


    if (!sig_ptr->pin2_status.pin_blocked)
    {
        atc_config_ptr->cpin2[dual_sys] = ATC_CPIN_CODE_READY;  //pin2 code exits always
    }
    else if (!sig_ptr->unblock_pin2_Status.pin_blocked)
    {
        atc_config_ptr->cpin2[dual_sys] = ATC_CPIN_CODE_PUK2;
    }
    else
    {
        atc_config_ptr->cpin2[dual_sys] = ATC_CPIN_CODE_BLOCK;
    }


    //Send the unsolicited +SIND: 1
    ATC_UpSINDInfo(dual_sys,
                   atc_config_ptr,
                   (xSignalHeaderRec *)sig_ptr,
                   ATC_WIND_BIT0,
                   1,
                   (BOOLEAN)(0 != atc_config_ptr->atc_config_nv.creg)
                   );

    //Send the unsolicited +SIND: 10
    ATC_UpSINDInfo(dual_sys,
                   atc_config_ptr,
                   (xSignalHeaderRec *)sig_ptr,
                   ATC_WIND_BIT9,
                   10,
                   (BOOLEAN)(0 != atc_config_ptr->atc_config_nv.creg)
                   );

    ATC_UpECINDInfo(dual_sys, atc_config_ptr, (xSignalHeaderRec *)sig_ptr, 3, 3, 0, 0, 0);

    return S_ATC_SUCCESS;
}

#ifndef ATC_SAT_ENABLE
/*****************************************************************************/
//  Description : This function handles the AT+ECIND command
//  Global resource dependence : g_rsp_str
//  Author:       binli
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessECIND)
{
    uint32  return_value = S_ATC_SUCCESS;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_READ:
            sprintf((char *)g_rsp_str, "%s: %s ",
            g_atc_info_table[AT_CMD_ECIND].cmd_name->str_ptr,  "(0-5),(0-5),(0-255),(0-255),(0-255)");

            ATC_BuildInfoRsp(atc_config_ptr, (uint8 *)g_rsp_str);
            break;
        default:
            return ERR_OPERATION_NOT_ALLOWED;

    }

    return return_value;
}
#endif

/*****************************************************************************/
//  Description : This function handles the APP_MN_SCELL_RSSI_IND signal
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
ATC_STATUS ATC_ProcessRssiInd(
    ATC_CONFIG_T                  *atc_config_ptr,
    APP_MN_SCELL_RSSI_IND_T       *sig_ptr)
{
    uint8 rel;
    uint16      rxqual_full;

#ifdef ATC_SAT_ENABLE
    rel = sig_ptr->rxlev;
    if(GPIO_IsAllowedReportRssiToAP())
    {
        if(110 - rel < g_atc_ciev_lev[3])
        {
            rel = 4;
        }
        else if(110 - rel < g_atc_ciev_lev[2])
        {
            rel = 3;
        }
        else if(110 - rel < g_atc_ciev_lev[1])
        {
            rel = 2;
        }
        else if(110 - rel < g_atc_ciev_lev[0])
        {
            rel = 1;
        }
        else
        {
            rel = 0;
        }

        if((MN_DUAL_SYS_1 <= sig_ptr->dual_sys) && (sig_ptr->dual_sys < MN_DUAL_SYS_MAX))
        {
            if(g_rxlev_arr[sig_ptr->dual_sys] != rel)
            {
                g_rxlev_arr[sig_ptr->dual_sys] = rel;
                sprintf((char*)g_rsp_str, "+CIEV: %d", rel);
               ATC_BuildUnsolicitedInfo(sig_ptr->dual_sys, atc_config_ptr, g_rsp_str);
            }
        }
    }

    atc_config_ptr->idle_rxlev[sig_ptr->dual_sys] = sig_ptr->rxlev;
    if (atc_config_ptr->csq_flag[sig_ptr->dual_sys])
    {
        rel = sig_ptr->rxlev / 2;
        if(sig_ptr->is_traffic_mode)
        {
            rxqual_full = sig_ptr->rxqual_full;
        }
        else
        {
            rxqual_full = AT_INVALID_RXQUAL;
        }

        sprintf((char*)g_rsp_str, "%s: %d,%d", g_atc_info_table[AT_CMD_CSQ].cmd_name->str_ptr, rel, sig_ptr->rxqual_full);
        ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
    }
#else

    if(PNULL == atc_config_ptr || PNULL == sig_ptr)
    {
        ATC_TRACE_LOW("ATC: ATC_ProcessRssiInd,at least one pointer is invalid");
        return S_ATC_FAIL;
    }

    ATC_TRACE_LOW("ATC: ATC_ProcessRssiInd,+CSQ rxlev=%d,rxqual=%d,csq_flag[%d]:%d", \
                                    sig_ptr->rxlev, sig_ptr->rxqual_full, sig_ptr->dual_sys, atc_config_ptr->csq_flag[sig_ptr->dual_sys]);

    if(atc_config_ptr->csq_flag[sig_ptr->dual_sys])
    {
        if(sig_ptr->is_traffic_mode)
        {
            rxqual_full = sig_ptr->rxqual_full;
        }
        else
        {
            rxqual_full = AT_INVALID_RXQUAL;
        }
        sprintf((char *)g_rsp_str, "%s: %d,%d",
                    g_atc_info_table[AT_CMD_CSQ].cmd_name->str_ptr,
                    ATC_GetConvertedRSSIVlaueForCSQ(sig_ptr->rxlev), rxqual_full);
        ATC_BuildUnsolicitedInfo(sig_ptr->dual_sys, atc_config_ptr, g_rsp_str);
    }

    if(0 == sig_ptr->rxlev)
    {
        rel = 0;
    }
    else if(sig_ptr->rxlev < 6)
    {
        rel = 1;
    }
    else if(sig_ptr->rxlev < 11)
    {
        rel = 2;
    }
    else if(sig_ptr->rxlev < 21)
    {
        rel = 3;
    }
    else
    {
        rel = 4;
    }

    atc_config_ptr->idle_rxlev[sig_ptr->dual_sys] = sig_ptr->rxlev;

    if((atc_config_ptr->cmer_flag[sig_ptr->dual_sys]) && (atc_config_ptr->rel_former[sig_ptr->dual_sys] != rel))
    {
        sprintf((char *)g_rsp_str, "+CIEV: 1,%d", rel);
        ATC_BuildUnsolicitedInfo(sig_ptr->dual_sys, atc_config_ptr, g_rsp_str);
    }

    atc_config_ptr->rel_former[sig_ptr->dual_sys] = rel;
#endif

    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handles the APP_MN_SCELL_RSSI_IND signal
//  Global resource dependence : g_rsp_str
//  Author:      sunny.shen
//  Note:
/*****************************************************************************/
ATC_STATUS ATC_ProcessRssiIndEx(
    ATC_CONFIG_T                  *atc_config_ptr,
    APP_MN_SCELL_RSSI_IND_T       *sig_ptr)
{
    static uint8   old_rxlev = 0;       //range:0...63

    if(PNULL == atc_config_ptr || PNULL == sig_ptr)
    {
        ATC_TRACE_LOW("ATC: ATC_ProcessRssiIndEx,at least one pointer is invalid");
        return S_ATC_FAIL;
    }

    if(atc_config_ptr->esqopt_param.option == ATC_ESQOPT_RSSI_THRESHOLD)
    {
        ATC_TRACE_LOW("ATC: ATC_ProcessRssiIndEx, +ECSQ old_rxlev=%d,rxlev=%d,rxqual=%d,esqopt thres:%d", \
        old_rxlev, sig_ptr->rxlev, sig_ptr->rxqual_full, atc_config_ptr->esqopt_param.thres_value);

        if(sig_ptr->rxlev >= (old_rxlev + atc_config_ptr->esqopt_param.thres_value)
           || sig_ptr->rxlev <= (old_rxlev - atc_config_ptr->esqopt_param.thres_value))
        {
            old_rxlev = sig_ptr->rxlev;
            sprintf((char *)g_rsp_str, "+ECSQ: %d,%d",
            ATC_GetConvertedRSSIVlaueForECSQ(old_rxlev), sig_ptr->rxqual_full);
            ATC_BuildUnsolicitedInfo(sig_ptr->dual_sys, atc_config_ptr, g_rsp_str);
        }
    }

    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handles the APP_MN_DEACTIVE_PS_CNF signal
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
ATC_STATUS ATC_ProcessPowerOffCnf(
    ATC_CONFIG_T                  *atc_config_ptr,
    APP_MN_DEACTIVE_PS_CNF_T     *sig_ptr
)
{
    #ifdef ATC_SAT_ENABLE
    SCI_MEMSET((void *)g_rxlev_arr,0,sizeof(g_rxlev_arr));
    #endif

    ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);

    //If it deactivate the PS,dont power off
    if(TRUE == g_is_ps_deactivate[sig_ptr->dual_sys])
    {
        g_is_ps_deactivate[sig_ptr->dual_sys] = FALSE;
    }

#ifdef  _ATC_ONLY
    else
    {
        SCI_KILL_TIMER(P_ATC, ATC_POWEROFF_EXP_IND, &T_ATC_POWEROFF_EXP_IND, "T_ATC_POWEROFF_EXP_IND");
#ifndef _SUPPORT_IPC_
        POWER_PowerOff();
#endif
  }
#endif

    //ATC_BuildResultCodeRsp(atc_config_ptr,S_ATC_RESULT_CODE_OK,0);
    g_is_ps_act[sig_ptr->dual_sys] = FALSE;
    SCI_MEMSET((void *)&g_cur_plmn_info[sig_ptr->dual_sys], 0, sizeof(ATC_PLMN_INFO_T));

    return S_ATC_DEFAULT_HANDLE;
}
/*****************************************************************************/
//  Description : This function handles the ATC_CHR_CAP_IND signal.And inform the
//  DTE the current battery level
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
ATC_STATUS ATC_ProcessChrCapInd(
    ATC_CONFIG_T                  *atc_config_ptr,
    xSignalHeaderRec              *sig_ptr
)
{
#ifndef ATC_SAT_ENABLE
    uint32              batt_vol;
    uint32              batt_state;

    if(atc_config_ptr->atc_config_nv.battery_indicator)
    {
#ifndef _MODEM_MODE //@brand.cheng
        batt_vol   = CHR_GetVoltage();
        batt_state = CHR_GetBattState();
#else
        batt_vol = 4200;
        batt_state = CHR_BATT_OUT_OF_CHARGE;
#endif //@brand.cheng
        sprintf((char *)g_rsp_str, "+SBCI: %ld,%ld", batt_state, batt_vol);
        ATC_BuildUnsolicitedInfo(g_current_card_id, atc_config_ptr, g_rsp_str);
    }
#endif /*ATC_SAT_ENABLE*/
    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handles the ATC_CHR_CHARGE_START_IND signal.Inform
//  that the battery starts charge.
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
ATC_STATUS ATC_ProcessChrStartInd(
    ATC_CONFIG_T                  *atc_config_ptr,
    xSignalHeaderRec              *sig_ptr
)
{
#ifndef ATC_SAT_ENABLE
    uint32              batt_vol;

    if(atc_config_ptr->atc_config_nv.battery_indicator)
    {
#ifndef _MODEM_MODE //@brand.cheng
        batt_vol   = CHR_GetVoltage();
#else
        batt_vol = 4200;
#endif //@brand.cheng
        sprintf((char *)g_rsp_str, "+SBCI: %d,%ld", CHR_BATT_IN_CHARGE, batt_vol);
        ATC_BuildUnsolicitedInfo(g_current_card_id, atc_config_ptr, g_rsp_str);
    }
#endif
    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handles the ATC_CHR_CHARGE_END_IND signal.Inform
//  that the battery starts charge.
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
ATC_STATUS ATC_ProcessChrEndInd(
    ATC_CONFIG_T                  *atc_config_ptr,
    xSignalHeaderRec              *sig_ptr
)
{
#ifndef ATC_SAT_ENABLE
    uint32              batt_vol;

    if(atc_config_ptr->atc_config_nv.battery_indicator)
    {
#ifndef _MODEM_MODE
        batt_vol   = CHR_GetVoltage();
#else
        batt_vol = 4200;
#endif
        sprintf((char *)g_rsp_str, "+SBCI: %d,%ld", CHR_BATT_OUT_OF_CHARGE, batt_vol);
        ATC_BuildUnsolicitedInfo(g_current_card_id, atc_config_ptr, g_rsp_str);
    }
#endif /*ATC_SAT_ENABLE*/
    return S_ATC_SUCCESS;
}
/*****************************************************************************/
//  Description : This function handle the AT## command
//  This command is called by Public phone module
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSHARP_SHARP)
{
    return S_ATC_DEFAULT_HANDLE;
}

/*****************************************************************************/
//  Description : This function handle the AT%%% command
//  Global resource dependence : none
//  Author:       Ivan.Yin
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessTHREE_PERCENT)
{
#ifndef _ULTRA_LOW_CODE_
    if (ATC_CMD_TYPE_EXECUTE != ATC_GET_CMD_TYPE)
    {
        ATC_TRACE_LOW("ATC: ATC_ProcessTHREE_PERCENT Command Type not match");
        return S_ATC_FAIL;
    }
#endif /* _ULTRA_LOW_CODE_ */
    return S_ATC_SUCCESS;
}
/*****************************************************************************/
//  Description : This function handle the AT+ASSERT command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessASSERT)
{
    SCI_ASSERT(FALSE); /*assert verified*/

    //return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handles the APP_MN_SET_BAND_CNF at the
//                 AT+SBAND
//  Global resource dependence : none
//  Author:       Liuhan
//  Note:
/*****************************************************************************/
ATC_STATUS ATC_ProcessSetBandCnf(
    ATC_CONFIG_T                *atc_config_ptr,
    APP_MN_SET_BAND_CNF_T       *sig_ptr
)
{
    if(sig_ptr->is_ok)
    {
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
    }
    else
    {
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_ERROR, 0);
    }

    return S_ATC_DEFAULT_HANDLE;
}

/*****************************************************************************/
//  Description : This function handles the AT+SBAND command, This command is used
//  to select the band in EGSM900/DCS1800/PCS1900.
//  Global resource dependence : g_rsp_str
//  Author:       Liuhan
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSBAND)
{
    ATC_STATUS     status = S_ATC_SUCCESS;
    int32          band = 0;
    MN_DUAL_SYS_E   dual_sys = ATC_CURRENT_SIM_ID_MUX;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:

            if(PARAM1_FLAG)
            {
                band = PARA1.num;

                switch(band)
                {
                    case 0:
                        band = MN_PHONE_BAND_GSM;
                        break;
                    case 1:
                        band = MN_PHONE_BAND_DCS;
                        break;
                    case 2:
                        band = MN_PHONE_BAND_PCS;
                        break;
                    case 3:
                        band = MN_PHONE_BAND_GSM850;
                        break;
                    case 4:
                        band = MN_PHONE_BAND_GSM_DCS;
                        break;
                    case 5:
                        band = MN_PHONE_BAND_GSM850_GSM;
                        break;
                    case 6:
                        band = MN_PHONE_BAND_GSM850_DCS;
                        break;
                    case 7:
                        band = MN_PHONE_BAND_GSM850_PCS;
                        break;
                    case 8:
                        band = MN_PHONE_BAND_GSM_PCS;
                        break;
                    case 9:
                        band = MN_PHONE_BAND_GSM850_GSM_DCS;
                        break;
                    case 10:
                        band = MN_PHONE_BAND_GSM850_GSM_PCS;
                        break;
                    case 11:
                        band = MN_PHONE_BAND_DCS_PCS;
                        break;
                    case 12:
                        band = MN_PHONE_BAND_GSM850_DCS_PCS;
                        break;
                    case 13:
                        band = MN_PHONE_BAND_GSM_DCS_PCS;
                        break;
                    case 14:
                        band = MN_PHONE_BAND_GSM850_GSM_DCS_PCS;
                        break;
                    default:
                        break;
                }

                if(ERR_MNPHONE_NO_ERR != MNPHONE_SelectBandEx(dual_sys, (MN_PHONE_BAND_TYPE_E)band))
                {
                    ATC_TRACE_LOW("ATC:error MNPHONE_SelectBand");
                    status = S_ATC_FAIL;
                    return status;
                }

                ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr,
                                  (uint16)APP_MN_SET_BAND_CNF, 0xff, ATC_NO_DOMAIN, dual_sys);
                status = S_ATC_DEFAULT_HANDLE;
            }
            else
            {
                status = ERR_INVALID_INDEX;
            }

            break;

        case ATC_CMD_TYPE_READ:
            //read band from the NV.
            band = MNPHONE_GetBandEx(dual_sys);

            switch(band)
            {
                case MN_PHONE_BAND_GSM:
                    band = 0;
                    break;
                case MN_PHONE_BAND_DCS:
                    band = 1;
                    break;
                case MN_PHONE_BAND_PCS:
                    band = 2;
                    break;
                case MN_PHONE_BAND_GSM850:
                    band = 3;
                    break;
                case MN_PHONE_BAND_GSM_DCS:
                    band = 4;
                    break;
                case MN_PHONE_BAND_GSM850_GSM:
                    band = 5;
                    break;
                case MN_PHONE_BAND_GSM850_DCS:
                    band = 6;
                    break;
                case MN_PHONE_BAND_GSM850_PCS:
                    band = 7;
                    break;
                case MN_PHONE_BAND_GSM_PCS:
                    band = 8;
                    break;
                case MN_PHONE_BAND_GSM850_GSM_DCS:
                    band = 9;
                    break;
                case MN_PHONE_BAND_GSM850_GSM_PCS:
                    band = 10;
                    break;
                case MN_PHONE_BAND_DCS_PCS:
                    band = 11;
                    break;
                case MN_PHONE_BAND_GSM850_DCS_PCS:
                    band = 12;
                    break;
                case MN_PHONE_BAND_GSM_DCS_PCS:
                    band = 13;
                    break;
                case MN_PHONE_BAND_GSM850_GSM_DCS_PCS:
                    band = 14;
                    break;
                default:
                    ATC_TRACE_LOW("ATC: unknow band %d", band);
                    break;
            }

            sprintf((char *)g_rsp_str, "%s: %ld",
            g_atc_info_table[AT_CMD_SBAND].cmd_name->str_ptr, band);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;

        default:
            status = ERR_OPERATION_NOT_SUPPORTED;
            break;
    }

    return status;

}

/*****************************************************************************/
//  Description : This function convert the string of IMEI to BCD code.
//  Global resource dependence : g_rsp_str
//  Author:       Liuhan
/*****************************************************************************/
BOOLEAN Atc_ConImeiStrToBin(    // FALSE: There is invalid char
    uint8        *hex_ptr,     // in: the hexadecimal format string
    uint16       length,       // in: the length of hexadecimal string
    uint8        *bin_ptr      // out: pointer to the binary format string
)
{
    uint8        *dest_ptr = bin_ptr;
    int32        i = 0;
    uint8        ch;


    SCI_ASSERT(PNULL != hex_ptr);/*assert verified: check null pointer*/
    SCI_ASSERT(PNULL != bin_ptr);/*assert verified: check null pointer*/

    //Handle the first value.
    ch = hex_ptr[0];

    // digital 0 - 9
    if(ch >= '0' && ch <= '9')
    {
        *dest_ptr = (uint8)((ch - '0') << 4);
    }
    // a - f
    else if(ch >= 'a' && ch <= 'f')
    {
        *dest_ptr = (uint8)((ch - 'a' + 10) << 4);
    }
    // A - F
    else if(ch >= 'A' && ch <= 'F')
    {
        *dest_ptr = (uint8)((ch - 'A' + 10) << 4);
    }
    else
    {
        return FALSE;
    }

    *dest_ptr += 0x0A;

    dest_ptr++;

    for(i = 1; i < length; i += 2)
    {
        // the bit 8,7,6,5
        ch = hex_ptr[i];

        // digital 0 - 9
        if(ch >= '0' && ch <= '9')
        {
            *dest_ptr = (uint8)(ch - '0');
        }
        // a - f
        else if(ch >= 'a' && ch <= 'f')
        {
            *dest_ptr = (uint8)(ch - 'a' + 10);
        }
        // A - F
        else if(ch >= 'A' && ch <= 'F')
        {
            *dest_ptr = (uint8)(ch - 'A' + 10) ;
        }
        else
        {
            return FALSE;
        }

        // the bit 1,2,3,4
        ch = hex_ptr[i+1];

        // digtial 0 - 9
        if(ch >= '0' && ch <= '9')
        {
            *dest_ptr |= (uint8)((ch - '0') << 4);
        }
        // a - f
        else if(ch >= 'a' && ch <= 'f')
        {
            *dest_ptr |= (uint8)((ch - 'a' + 10) << 4);
        }
        // A - F
        else if(ch >= 'A' && ch <= 'F')
        {
            *dest_ptr |= (uint8)((ch - 'A' + 10) << 4);
        }
        else
        {
            return FALSE;
        }

        dest_ptr++;
    }

    *dest_ptr = '\0';
    return TRUE;

}
/*****************************************************************************/
//  Description : This function handles the AT+SNVM command, This command is used
//  to manage the NV items.
//  Global resource dependence : g_rsp_str
//  Author:       Liuhan
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSNVM)
{
#ifndef _ULTRA_LOW_CODE_
#ifdef _ATC_ONLY

    ATC_STATUS      status = S_ATC_SUCCESS;
    NVITEM_ERROR_E  nv_err = NVERR_NONE;
    uint16          nv_item_id;
    uint16          nv_item_id_end;
    uint16          mode;
    uint16          nv_item_length;
    /*uint16          count;*/
    uint8           *buf_ptr;
    uint8           *temp_ptr;

    // Set the nv item valuve
    if(PARAM1_FLAG && PARAM2_FLAG)
    {
        mode = (uint16)PARA1.num;
        nv_item_id = (uint16)PARA2.num + MN_NV_USER_BASE; // id from 0 - 1199

        // set the related NV value
        switch(mode)
        {
            case ATC_SNVM_READ:
                nv_item_length = NVITEM_GetLength(nv_item_id);

                if(nv_item_length > MAX_ATC_RSP_LEN / 2 - 12) // exclude +SNVM: <length>,
                {
                    ATC_TRACE_LOW("ATC: nv item length = %d", nv_item_length);
                    status =  ERR_INVALID_INDEX;
                    break;
                }

                buf_ptr = (uint8 *)SCI_ALLOC_BASE(nv_item_length * 2 + 1);

                SCI_MEMSET(buf_ptr, 0, (nv_item_length * 2 + 1));

                nv_err = EFS_NvitemRead(nv_item_id, nv_item_length, (uint8 *)buf_ptr);

                if(nv_err != NVERR_NONE)
                {
                    ATC_TRACE_LOW("ATC: SNVM read nv item id = %d fail, err = %d", nv_item_id, nv_err);
                    SCI_FREE(buf_ptr);
                    status = ERR_INVALID_INDEX;
                    break;
                }

                sprintf((char *)g_rsp_str, "%s: %d,", g_atc_info_table[AT_CMD_SNVM].cmd_name->str_ptr, nv_item_length);
                temp_ptr = g_rsp_str + strlen((char *)g_rsp_str);

                ConvertBinToHex(buf_ptr, nv_item_length, temp_ptr);

                //qing.xi cr53411
                temp_ptr[nv_item_length * 2] = '\0';
                //qing.xi


                SCI_FREE(buf_ptr);
                ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
                break;
            case ATC_SNVM_ADD:
            case ATC_SNVM_REPLACE:
                s_nv_item_id = nv_item_id;
                // output the '>'
                ATC_SendAsyncInfo(atc_config_ptr, ATC_CTRLZ_SNVM);
                status = S_ATC_DEFAULT_HANDLE;
                break;

            case ATC_SNVM_GETSIZE:
                nv_item_length = NVITEM_GetLength(nv_item_id);
                sprintf((char *)g_rsp_str, "%s: %d", g_atc_info_table[AT_CMD_SNVM].cmd_name->str_ptr, nv_item_length);
                ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
                break;

            case ATC_SNVM_DELETE:

                if(PARAM3_FLAG)
                {
                    nv_item_id_end = (uint16)PARA3.num + MN_NV_USER_BASE;

                    if(nv_item_id_end < nv_item_id)
                    {
                        status = ERR_OPERATION_NOT_ALLOWED;
                        break;
                    }
                }
                else
                {
                    nv_item_id_end = nv_item_id;
                }

                nv_err = EFS_NvitemDelete(nv_item_id, nv_item_id_end);

                if(nv_err != NVERR_NONE)
                {
                    ATC_TRACE_LOW("ATC: SNVM read nv item id = %d fail, err = %d", nv_item_id, nv_err);
                    status = ERR_INVALID_INDEX;
                }

                break;
            default :
                break;
        }


    }
    else
    {
        status = ERR_OPERATION_NOT_SUPPORTED;
    }

    return status;
#else
    return S_ATC_SUCCESS;
#endif // END _ATC_ONLY

#else
    return S_ATC_SUCCESS;
#endif
}

/*****************************************************************************/
//  Description : This function handles the data send from the SIO when AT+SNVM
//  Global resource dependence :
//  Author:       SHIJUN
//  Note:
/*****************************************************************************/
PUBLIC uint32 ATC_ProcessSioSNVMData(
    ATC_CONFIG_T    *atc_config_ptr,
    uint8           *data_ptr  // pointer to the data
)
{
    uint16          data_length;
    uint8           terminator;        // holds the terminal charactor
    NVITEM_ERROR_E  nv_err = NVERR_NONE;

    if (!atc_config_ptr)
    {
        ATC_TRACE_LOW("ATC: ATC_ProcessSioSNVMData  atc_config_ptr pointer is PNULL");
        return S_ATC_FAIL;
    }

    if (!data_ptr)
    {
        ATC_TRACE_LOW("ATC: ATC_ProcessSioSNVMData  data_ptr pointer is PNULL");
        return S_ATC_FAIL;
    }

    data_length = strlen((char *)data_ptr);
    terminator = data_ptr[data_length - 1];

    switch(terminator)
    {
        case CTRL_Z:
            ConvertHexToBin(data_ptr, data_length, data_ptr);
            EFS_NvitemDelete(s_nv_item_id, s_nv_item_id);
            nv_err = EFS_NvitemWrite(s_nv_item_id, data_length / 2, data_ptr, FALSE);

            if(NVERR_NONE == nv_err)
            {
                ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
            }
            else
            {
                ATC_TRACE_LOW("ATC: write snvm data fail, err = %d.", nv_err);
                ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_ERROR , ERR_UNKNOWN);
            }

            break;
        case ESC:
            ATC_TRACE_LOW("ATC: cancel the snvm data.");
            ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
            break;
        default :
            ATC_TRACE_LOW("ATC: snvm data format is error.");
            ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_ERROR , ERR_UNKNOWN);
            break;
    }

    return S_ATC_DEFAULT_HANDLE;
}

/*****************************************************************************/
//  Description : This function converts the hex to bin
//  Global resource dependence :
//  Author:       hanjun.liu
//  Note:
/*****************************************************************************/
LOCAL BOOLEAN   ConvertInt16ToBin(    // FALSE: There is invalid char
    uint8        *hex_ptr,    // in: the hexadecimal format string of address
    uint16       *bin_ptr      // out: pointer to the address
)
{
    uint16        *dest_ptr = bin_ptr;
    int32        i = 0;
    uint8        ch;

    SCI_ASSERT(PNULL != hex_ptr);/*assert verified: check null pointer*/
    SCI_ASSERT(PNULL != bin_ptr);/*assert verified: check null pointer*/

    *bin_ptr = 0;

    for(i = 0; i < 4; i++)
    {
        ch = hex_ptr[i];
        (*dest_ptr) = (*dest_ptr) * 16;

        // digital 0 - 9
        if(ch >= '0' && ch <= '9')
        {
            *dest_ptr += (uint8)(ch - '0');
        }
        // a - f
        else if(ch >= 'a' && ch <= 'f')
        {
            *dest_ptr += (uint8)(ch - 'a' + 10) ;
        }
        // A - F
        else if(ch >= 'A' && ch <= 'F')
        {
            *dest_ptr += (uint8)(ch - 'A' + 10);
        }
        else
        {
            return FALSE;
        }

        ATC_TRACE_LOW("ATC:dest_Addr=%x,ch=%x", *dest_ptr, ch);
    }

    return TRUE;
}

#ifndef ATC_SAT_ENABLE
/*****************************************************************************/
//  Description : This function handles the AT+SBRA command, This command is used
//  to enable the ATC
//  Global resource dependence : g_rsp_str
//  Author:       Liuhan
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSBRA)
{
    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handles the ATCMD_READW_CNF signal and send the
//                result code to SIO
//  Global resource dependence :
//  Author:       hanjun.liu
//  Note:
/*****************************************************************************/
PUBLIC ATC_STATUS ATC_ProcessReadDSPCnf(
    ATC_CONFIG_T             *atc_config_ptr,
    APP_MN_READ_DSP_MEM_CNF_T    *sig_ptr
)
{
#ifndef _ULTRA_LOW_CODE_
    uint8 length = sig_ptr->length;

    if(0 == length)
    {
        //Return the result
        sprintf((char *)g_rsp_str, "%s: 0x%x", g_atc_info_table[AT_CMD_SRWD].cmd_name->str_ptr, sig_ptr->result);
        ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
    }
    else
    {
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_ERROR , ERR_UNKNOWN);
    }

#endif
    return S_ATC_DEFAULT_HANDLE;
}

#ifndef _ULTRA_LOW_CODE_
/*****************************************************************************/
//  Description : This function handles the data send from the SIO when AT+SRWD
//  Global resource dependence : g_rsp_str
//  Author:       hanjun.liu
//  Note:
/*****************************************************************************/
PUBLIC uint32 ATC_ProcessSioSRWDData(
    ATC_CONFIG_T    *atc_config_ptr,
    uint8           *data_ptr  // pointer to the data
)
{
    uint8       terminater_char;     // indicates ther terminater char
    uint16      data_len;          // holds the gived pdu length
    BOOLEAN     flag = FALSE;
    uint32      i;
    int16       *value_or_source_address; // If the length is 0,the value will be put into the destine address directly.
    // Otherwise a memcpy is performed.

    if (!atc_config_ptr)
    {
        ATC_TRACE_LOW("ATC: ATC_ProcessSioSRWDData  atc_config_ptr pointer is PNULL");
        return S_ATC_FAIL;
    }
    if (!data_ptr)
    {
        ATC_TRACE_LOW("ATC: ATC_ProcessSioSRWDData  data_ptr pointer is PNULL");
        return S_ATC_FAIL;
    }

    data_len              = (uint16)strlen((char *)data_ptr);
    terminater_char      = data_ptr[data_len - 1];
    data_ptr[data_len - 1] = '\0';
    data_len             -= 1;

    switch(terminater_char)
    {
            // cancel the operation
        case ESC:
            flag = TRUE;
            break;

        case CTRL_Z:

            if(0 == s_dsp_data_length)
            {
                //allocate one word memory
                value_or_source_address  = SCI_ALLOC_BASE(2);

                if(!ConvertInt16ToBin(data_ptr, (uint16 *)value_or_source_address))
                {
                    ATC_TRACE_LOW("ATC:AT+SRWD,ConvertHexToBin=FALSE");
                    break;
                }

                ATC_TRACE_LOW("ATC:AT+SRWD,value_or_source_address=%d", *value_or_source_address);

            }
            else
            {
                //uint16 *temp_ptr = PNULL;
                value_or_source_address  = SCI_ALLOC_BASE(data_len / 2);
                SCI_MEMSET(value_or_source_address, 0, (data_len / 2));

                //Change to bin format.
                for(i = 0; i < data_len / 4; i++)
                {
                    ConvertInt16ToBin((data_ptr + i * 4), (uint16 *)(value_or_source_address + i));
                }

                ATC_TRACE_LOW("ATC:AT+SRWD,value_or_source_address=%x,%x,%x", *value_or_source_address,
                *(value_or_source_address + 1), *(value_or_source_address + 1));
            }

            flag = TRUE;
            // call the process to write DSP
            L1API_WriteDspMemory(s_dsp_ram_flag, s_dsp_dest_addr, s_dsp_data_length, value_or_source_address);
            break;

        default:
            break;
    }

    ATC_TRACE_LOW("ATC:flag = %d", flag);

    if(!flag)
    {
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_ERROR , ERR_UNKNOWN);
    }
    else
    {
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
    }

    return S_ATC_DEFAULT_HANDLE;
}
#endif

/*****************************************************************************/
//  Description : This function handles the AT+SSGF command.This command is used
//  to set or get GPIO function.
//  Global resource dependence : g_rsp_str
//  Author:       Shijun.cui
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSSGF)
{
#ifndef _ULTRA_LOW_CODE_
    ATC_STATUS    status = S_ATC_SUCCESS;
    uint32        gpio_id;
    uint32        type; // 0 - set direction, 1 - get Direction
    BOOLEAN       value = FALSE;
    BOOLEAN       result = FALSE;
    uint32        gpio_num;

    switch(ATC_GET_CMD_TYPE)
    {

        case ATC_CMD_TYPE_SET:
            // check the type

            if(PARAM1_FLAG && PARAM2_FLAG)
            {
                //PARA1.num define the GPIO id
                gpio_id = PARA1.num;
                //PARA2.num define the operation type, 0 - set gpio direction, 1 - get gpio direction
                type = PARA2.num;
            }
            else
            {
                status = ERR_OPERATION_NOT_ALLOWED;
                break;
            }

#ifndef WIN32
            gpio_num = GPIO_GetMaxNumber();

            if(gpio_id >= gpio_num)
            {
                status = ERR_INVALID_INDEX;
                break;
            }

#endif

            if((type == AT_GPIO_SET_DIRECTION) && !PARAM3_FLAG)
            {
                ATC_TRACE_LOW("ATC: need gpio direction parameter!");
                status = ERR_OPERATION_NOT_ALLOWED;
                break;
            }

            if(PARAM3_FLAG)
            {
                value = PARA3.num;
            }

            ATC_TRACE_LOW("ATC: set direction gpio id = %d, type = %d, value = %d", gpio_id, type, value);

            switch(type)
            {
                case AT_GPIO_SET_DIRECTION:
#ifndef WIN32
                    GPIO_SetDirection(gpio_id, value); // set input direction
#endif
                    break;
                case AT_GPIO_GET_DIRECTION:
#ifndef WIN32
                    result = GPIO_GetDirection(gpio_id); // get direction
#endif
                    sprintf((char *)g_rsp_str, "%s: %d", g_atc_info_table[AT_CMD_SSGF].cmd_name->str_ptr, result);
                    ATC_BuildInfoRsp(atc_config_ptr, (uint8 *)g_rsp_str);
                    break;

                default :
                    ATC_TRACE_LOW("ATC: Assert Failure receive gpio type = %d", type);
                    break;
            }

            break;

        default:
            status = ERR_OPERATION_NOT_SUPPORTED;
            break;
    }

    return status;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

#endif

/*****************************************************************************/
//  Description : This function handles the AT+SIND command
//  Global resource dependence : g_rsp_str
//  Author:       hanjun.liu
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSIND)
{
    ATC_STATUS    status = S_ATC_SUCCESS;

    switch(ATC_GET_CMD_TYPE)
    {
            //set command AT+SIND=IndLevel
        case ATC_CMD_TYPE_SET:

            // check the index
            if(!PARAM1_FLAG)
            {
                return ERR_INVALID_INDEX;
            }

            if((uint16)PARA1.num != atc_config_ptr->atc_config_nv.wind_ind_level)
            {
                atc_config_ptr->atc_config_nv.wind_ind_level = (uint16)PARA1.num;

                //save in the NV.
                if(!ATC_WriteNVItem(atc_config_ptr, NV_ATC_CONFIG_ID))
                {
                    ATC_TRACE_LOW("ATC: ATC_WriteNVItem failed");
                    return ERR_MEMORY_FAILURE;
                }
            }

            break;

            //Test command AT+SIND?
        case ATC_CMD_TYPE_READ:
            sprintf((char *)g_rsp_str, "%s: %d", g_atc_info_table[AT_CMD_SIND].cmd_name->str_ptr,
            atc_config_ptr->atc_config_nv.wind_ind_level);
            ATC_BuildInfoRsp(atc_config_ptr, (uint8 *)g_rsp_str);
            break;

        default:
            status = ERR_OPERATION_NOT_SUPPORTED;
            break;
    }

    return status;
}

/*****************************************************************************/
//  Description : This function handles the AT+SBOOT command.This command is used
//  to set the spreadtrum module to BOOT mode.
//  Global resource dependence : g_rsp_str
//  Author:       Xueli.lv
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSBOOT)
{
#ifdef _ULTRA_LOW_CODE_
    return S_ATC_SUCCESS;
#else
    ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
    POWER_BootReset();
    return S_ATC_DEFAULT_HANDLE;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handles the AT+SLOG command.This command is used
//  to enable the log output.
//  Global resource dependence : g_rsp_str
//  Author:       Shijun.cui
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSLOG)
{
    ATC_STATUS    status = S_ATC_SUCCESS;

#ifndef _ULTRA_LOW_CODE_

    switch(ATC_GET_CMD_TYPE)
    {
            //set command AT+SLOG=type
        case ATC_CMD_TYPE_SET:

            // check the type
            if(PARAM1_FLAG)
            {
                //PARA1.num define the log switch
                if(PARA1.num)
                {
                    SCI_InitLogSwitch(SCI_TRUE);
                }
                else
                {
                    SCI_InitLogSwitch(SCI_FALSE);
                }
            }
            else
            {
                status = ERR_OPERATION_NOT_ALLOWED;
            }

            break;

        default:
            status = ERR_OPERATION_NOT_SUPPORTED;
            break;
    }

#endif /* _ULTRA_LOW_CODE_ */

    return status;
}

/*****************************************************************************/
//  Description : This function handles the AT+SUPS command.This command is used
//  to enable the log output.
//  Global resource dependence : g_rsp_str
//  Author:       Shijun.cui
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSUPS)
{
#ifdef _ULTRA_LOW_CODE_
    return S_ATC_SUCCESS;
#else
    ATC_STATUS    status = S_ATC_SUCCESS;

    switch(ATC_GET_CMD_TYPE)
    {

        case ATC_CMD_TYPE_SET:

            // check the type
            if(PARAM1_FLAG)
            {
                //PARA1.num define the uart switch
                if(PARA1.num)
                {
                    SIO_ATC_SetMixMode(TRUE);
                }
                else
                {
                    SIO_ATC_SetMixMode(FALSE);
                }

            }
            else
            {
                status = ERR_OPERATION_NOT_ALLOWED;
            }

            break;

        default:
            status = ERR_OPERATION_NOT_SUPPORTED;
            break;
    }

    return status;
#endif /* _ULTRA_LOW_CODE_ */
}

#ifndef ATC_SAT_ENABLE
/*****************************************************************************/
//  Description : This function handles the AT+SSGS command.This command is used
//  to set or get GPIO value.
//  Global resource dependence : g_rsp_str
//  Author:       Shijun.cui
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSSGS)
{
#ifndef _ULTRA_LOW_CODE_
    ATC_STATUS    status = S_ATC_SUCCESS;
    uint32        gpio_id;
    uint32        type; // 0 - set gpio value, 1 - get gpio value
    BOOLEAN       value = FALSE;
    BOOLEAN       result = FALSE;
    uint32        gpio_num;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:
            // check the type

            if(PARAM1_FLAG && PARAM2_FLAG)
            {
                //PARA1.num define the GPIO id
                gpio_id = PARA1.num;
                //PARA2.num define the operation type, 0 - set gpio value, 1 - get gpio value
                type = PARA2.num;
            }
            else
            {
                status = ERR_OPERATION_NOT_ALLOWED;
                break;
            }

#ifndef WIN32
            gpio_num = GPIO_GetMaxNumber();

            if(gpio_id >= gpio_num)
            {
                status = ERR_INVALID_INDEX;
                break;
            }
#endif

            if((AT_GPIO_SET_VALUE == type) && !PARAM3_FLAG)
            {
                ATC_TRACE_LOW("ATC: need gpio set parameter!");
                status = ERR_OPERATION_NOT_ALLOWED;
                break;
            }

            if(PARAM3_FLAG)
            {
                value = PARA3.num;
            }

#ifndef WIN32
            if(!GPIO_GetDataMask(gpio_id))
            {
                /*Gpio not enabled*/
                ATC_TRACE_LOW("ATC: gpio:%d not enabled!", gpio_id);
                status = ERR_OPERATION_NOT_ALLOWED;
                break;
            }
#endif

            ATC_TRACE_LOW("ATC: set value gpio id = %d type = %d, value = %d", gpio_id, type, value);

            switch(type)
            {
                case AT_GPIO_SET_VALUE:
#ifndef WIN32
                    result = GPIO_GetDirection(gpio_id); // get direction
#endif

                    if(!result)
                    {
                        ATC_TRACE_LOW("ATC: gpio %d set to input directiong, set value not permit.", gpio_id);
                        status = ERR_OPERATION_NOT_ALLOWED;
                        break;
                    }

#ifndef WIN32
                    GPIO_SetValue(gpio_id, value); // set value
#endif
                    break;
                case AT_GPIO_GET_VALUE:
#ifndef WIN32
                    result = GPIO_GetDirection(gpio_id); // get direction
#endif

                    if(!result)
                    {
                        ATC_TRACE_LOW("ATC: gpio %d set to input directiong, get value not permit.", gpio_id);
                        status = ERR_OPERATION_NOT_ALLOWED;
                        break;
                    }

#ifndef WIN32
                    result = GPIO_GetValue(gpio_id); // get value
#endif
                    sprintf((char *)g_rsp_str, "%s: %d", g_atc_info_table[AT_CMD_SSGS].cmd_name->str_ptr, result);
                    ATC_BuildInfoRsp(atc_config_ptr, (uint8 *)g_rsp_str);
                    break;
                default :
                    ATC_TRACE_LOW("ATC: Assert Failure receive gpio type = %d", type);
                    break;
            }

            break;

        default:
            status = ERR_OPERATION_NOT_SUPPORTED;
            break;
    }

    return status;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handles the AT+SRWD command, This command is used
//  to read or write the DSP.
//  Global resource dependence : g_rsp_str
//  Author:       hanjun.liu
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSRWD)
{
#ifdef _ULTRA_LOW_CODE_
    return S_ATC_SUCCESS;
#else
    ATC_STATUS    status = S_ATC_SUCCESS;
    int16         temp_address;

    //check the validity of parameter
    if((!PARAM1_FLAG) || (!PARAM2_FLAG) || (!PARAM3_FLAG) || (!PARAM4_FLAG))
    {
        ATC_TRACE_LOW("ATC:AT+SRWD,error in parameters");
        status = ERR_INVALID_INDEX;
        return status;
    }

    //handle the parameter
    s_dsp_ram_flag = PARA2.num;
    s_dsp_data_length   = PARA3.num;

    if(PARAM4_TYPE != ATC_CMD_PARAM_TYPE_STRING)
    {
        ATC_TRACE_LOW("ATC: ATC_ProcessSRWD, param4 is not string!");
        status = ERR_INVALID_INDEX;
        return status;
    }

    if(4 != PARA4.str_ptr->str_len)
    {
        ATC_TRACE_LOW("ATC:Length of string is error!");
        status = ERR_INVALID_INDEX;
        return status;
    }

    //Contert the ascci format of address to bin.
    temp_address = 0;

    if(!ConvertInt16ToBin(PARA4.str_ptr->str_ptr, (uint16 *)&temp_address))
    {
        ATC_TRACE_LOW("ATC:AT+SRWD,ConvertInt16ToBin=FALSE");
        status = ERR_INVALID_INDEX;
        return status;
    }

    //write the DSP
    if(PARA1.num)
    {
        s_dsp_dest_addr = temp_address;
        // output the '>'
        ATC_SendAsyncInfo(atc_config_ptr, ATC_CTRLZ_SRWD);
        return S_ATC_DEFAULT_HANDLE;
    }
    //read the DSP
    else
    {
        //call the function to read data from DSP
        L1API_ReadDspMemory(s_dsp_ram_flag, temp_address, s_dsp_data_length, P_ATC);
        ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr,
                          (uint16)APP_MN_READ_DSP_MEM_CNF, 0xff, ATC_NO_DOMAIN, ATC_CURRENT_SIM_ID_MUX);
        return S_ATC_DEFAULT_HANDLE;
    }

#endif
}

/*****************************************************************************/
//  Description : This function handle the AT+SFUN command
//  Global resource dependence : none
//  Author:
//  Note:
/*****************************************************************************/
AT_CMD_FUNC( ATC_ProcessSFUN )
{
    ATC_STATUS    status = S_ATC_DEFAULT_HANDLE;
    MN_DUAL_SYS_E   dual_sys = ATC_CURRENT_SIM_ID_MUX;

    switch (ATC_GET_CMD_TYPE)
    {
    case ATC_CMD_TYPE_SET:
        if (PARAM1_FLAG)
        {
            switch(PARA1.num)
            {
            case ATC_POWER_OFF_SIM_PS:
                if (!g_is_ps_act[dual_sys])
                {
                    return S_ATC_SUCCESS;
                }

#ifndef ATC_SAT_ENABLE
                //Initialise the variable of STK
                ATC_InitSimStkVar(dual_sys);
#endif

                MNPHONE_DeactivePsEx(dual_sys);
                ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr,
                                  (uint16)APP_MN_DEACTIVE_PS_CNF, 0xff, ATC_NO_DOMAIN, dual_sys);
                g_is_ps_act[dual_sys]   = FALSE;
                g_is_ps_deactivate[dual_sys] = TRUE;
                atc_config_ptr->cfun_status[dual_sys] = ATC_FUN_DEACT_SIM_PS;
                break;
            case ATC_POWER_ON_PS:
                if (g_is_ps_act[dual_sys])
                {
                    return S_ATC_SUCCESS;
                }
                if ((ATC_FUN_DEACT_SIM_PS == atc_config_ptr->cfun_status[dual_sys]) ||
                     (ATC_FUN_DEACT_SIM == atc_config_ptr->cfun_status[dual_sys]))
                {
                    return ERR_OPERATION_NOT_ALLOWED;
                }

                if( s_sms_exclusive_flag )
                {
                    s_sms_exclusive_flag = FALSE;
                }

                MNPHONE_PowerOnPsEx(dual_sys);
                ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr,
                                  (uint16)APP_MN_PS_POWER_ON_CNF, 0xff, ATC_NO_DOMAIN, dual_sys);
                g_is_ps_act[dual_sys]   = TRUE;
                atc_config_ptr->cfun_status[dual_sys] = ATC_FUN_ACT_PS;
                break;
            case ATC_POWER_OFF_PS:
                if (!g_is_ps_act[dual_sys])
                {
                    return S_ATC_SUCCESS;
                }
                MNPHONE_PowerOffPsEx(dual_sys);
                ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr,
                                  (uint16)APP_MN_PS_POWER_OFF_CNF, 0xff, ATC_NO_DOMAIN, dual_sys);
                g_is_ps_act[dual_sys]   = FALSE;
                atc_config_ptr->cfun_status[dual_sys] = ATC_FUN_DEACT_PS;
                break;
            case ATC_POWER_ON_SIM_ONLY:
                MNPHONE_PowerOnSimEx(dual_sys, FALSE);
                ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr,
                                  (uint16)APP_MN_SIM_POWER_ON_CNF, 0xff, ATC_NO_DOMAIN, dual_sys);
                atc_config_ptr->cfun_status[dual_sys] = ATC_FUN_ACT_SIM;
                break;
            case ATC_POWER_ON_SIM_NORMAL:
                MNPHONE_PowerOnSimEx(dual_sys, TRUE);// init sms and pb
                ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr,
                                  (uint16)APP_MN_SIM_POWER_ON_CNF, 0xff, ATC_NO_DOMAIN, dual_sys);
                atc_config_ptr->cfun_status[dual_sys] = ATC_FUN_ACT_SIM;
                break;
            case ATC_POWER_OFF_SIM:
                if (g_is_ps_act[dual_sys])
                {
                    return ERR_OPERATION_NOT_ALLOWED;
                }

#ifndef ATC_SAT_ENABLE
                //Initialise the variable of STK
                ATC_InitSimStkVar(dual_sys);
#endif

                MNPHONE_PowerOffSimEx(dual_sys);
                ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr,
                                  (uint16)APP_MN_SIM_POWER_OFF_CNF, 0xff, ATC_NO_DOMAIN, dual_sys);
                atc_config_ptr->cfun_status[dual_sys] = ATC_FUN_DEACT_SIM;
                break;
            default:
                status = ERR_INVALID_INDEX;
                break;
            }
        }
        else
        {
            status = ERR_INVALID_INDEX;
        }
        break;

    case ATC_CMD_TYPE_READ:
        sprintf((char *)g_rsp_str, "%s: %d", g_atc_info_table[AT_CMD_SFUN].cmd_name->str_ptr,
                atc_config_ptr->cfun_status[dual_sys]);
        ATC_BuildInfoRsp(atc_config_ptr, (uint8 *)g_rsp_str);
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
        break;

    default:
        return ERR_OPERATION_NOT_ALLOWED;
    }

    return status;
}

/*****************************************************************************/
//  Description : This function handles the AT+EUICC command.This command is used
//  to set or get uicc info.
//  Global resource dependence : g_rsp_str
//  Author:       bin.li
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessEUICC)
{
#ifdef _ULTRA_LOW_CODE_
    return S_ATC_SUCCESS;
#else
    ATC_STATUS    status = S_ATC_SUCCESS;
    MN_DUAL_SYS_E   dual_sys = ATC_CURRENT_SIM_ID_MUX;
    MN_CARD_TYPE_E  card_type = MN_CARD_TYPE_NUM;
    uint8 sim_status = 0;
    uint8 pin_status = 0;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_READ:
            if(atc_config_ptr->sim_type[dual_sys] != MN_CARD_TYPE_NUM)
            {
                ATC_GetUiccInfo(dual_sys, atc_config_ptr, &sim_status, &pin_status, &card_type);
                sprintf((char *)g_rsp_str, "%s: %d,%d,%d",
                        g_atc_info_table[AT_CMD_EUICC].cmd_name->str_ptr,
                        sim_status, pin_status, card_type);
                ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
                status = S_ATC_SUCCESS;
            }
            else
            {
                s_atc_euicc_cnt[dual_sys] += 1;
                MNSIM_GetSimTypeEx(dual_sys);
                ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr,
                                      (uint16)APP_MN_GET_SIM_TYPE_CNF, 0xff, ATC_NO_DOMAIN, dual_sys);

                status = S_ATC_DEFAULT_HANDLE;
            }
            break;
        case ATC_CMD_TYPE_TEST:
            break;

        default:
            return ERR_OPERATION_NOT_ALLOWED;
    }

    return status;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the AT+SPCM command
//  Global resource dependence : g_rsp_str
//  Author:  Shijun.cui
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSPCM)
{
#ifdef _ATC_ONLY
    BOOLEAN  is_exist_call = FALSE;
    uint8  i;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:

            if(PARAM1_FLAG)
            {
                if(g_atc_pcm_flag != (uint8)PARA1.num)
                {
                    g_atc_pcm_flag = (uint8)PARA1.num;

                    for(i=0; i<MN_SYS_NUMBER; i++)
                    {
                        SCI_TRACE_LOW("ATC%d: call amount = %d, call_flag = %d", i, g_calls_state[i].call_amount, g_atc_call_flag);
                        if(0 != g_calls_state[i].call_amount)
                        {
                            is_exist_call = TRUE;
                            break;
                        }
                    }

                    if(is_exist_call && g_atc_call_flag)
                    {
                        if(atc_config_ptr->audio_flag)
                        {
                            atc_config_ptr->audio_mode = AUDIO_DEVICE_MODE_BLUEPHONE;
                            AUDIO_SetDevMode((AUDIO_DEVICE_MODE_TYPE_E)(atc_config_ptr->audio_mode));
                            AUDIO_SetVolume(atc_config_ptr->speaker_volume);
                            ATC_EnableUplinkMute(s_audio_status.is_mute);
                            ATC_EnableRemoteMute(s_audio_status.is_remote_mute);

                            ATC_EnableBtVoiceCodec(TRUE);
                        }
                        else
                        {
                            atc_config_ptr->audio_mode = AUDIO_DEVICE_MODE_BLUEPHONE;
                            AUDIO_SetDevMode((AUDIO_DEVICE_MODE_TYPE_E)(atc_config_ptr->audio_mode));
                        }
                    }
                }
            }
            else
            {
                return ERR_INVALID_INDEX;
            }
            break;
        case ATC_CMD_TYPE_READ:
            sprintf((char *)g_rsp_str, "%s: %ld",
                g_atc_info_table[AT_CMD_SPCM].cmd_name->str_ptr,
                g_atc_pcm_flag);

            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;

        default:
            SCI_ASSERT(FALSE);/* assert verified: NULL_PTR */
            break;
    }
#endif
    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handles the AT+CLVL command
//  Global resource dependence : g_rsp_str
//  Author:       peng.chen
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCLVL)
{
    ATC_STATUS    status = S_ATC_SUCCESS;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:

            if(!PARAM1_FLAG)
            {
                return S_ATC_FAIL;
            }

            if(AUDIO_NO_ERROR != AUDIO_SetVolume(PARA1.num))
            {
                return S_ATC_FAIL;
            }

            if((uint8)PARA1.num != atc_config_ptr->atc_config_nv.earphone_vol)
            {
                atc_config_ptr->atc_config_nv.earphone_vol = (uint8)PARA1.num;
                // Save to the NV ram.
                ATC_WriteNVItem(atc_config_ptr, NV_ATC_CONFIG_ID);
            }

            break;
        case ATC_CMD_TYPE_READ:
            sprintf((char *)g_rsp_str, "%s:%d",
            g_atc_info_table[AT_CMD_CLVL].cmd_name->str_ptr,
            atc_config_ptr->atc_config_nv.earphone_vol);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;
        default:
            break;
    }

    return status;
}

/*****************************************************************************/
//  Description : This function handles the AT+GMM command
//  Global resource dependence : g_rsp_str
//  Author:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessGMM)
{
    ATC_STATUS       status = S_ATC_SUCCESS;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:
            //status = S_ATC_DEFAULT_HANDLE;
            sprintf((char *)g_rsp_str, "H.324 video-ready rev. 1.0");
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;

        case ATC_CMD_TYPE_READ:
            break;

        default:
            break;
    }

    return status;
}

/*****************************************************************************/
//  Description : This function handles the AT_CLS command
//  Global resource dependence : g_rsp_str
//  Author:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSHARP_CLS)
{
    ATC_STATUS       status = S_ATC_SUCCESS;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:
            break;

        case ATC_CMD_TYPE_READ:
            status = S_ATC_DEFAULT_HANDLE;
            ATC_BuildResultErrCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_ERROR, 0);
            break;

        case ATC_CMD_TYPE_TEST:
            status = S_ATC_DEFAULT_HANDLE;
            ATC_BuildResultErrCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_ERROR, 0);
            break;

        default:
            break;
    }

    return status;
}

/*****************************************************************************/
//  Description : This function handles the AT+GCI command
//  Global resource dependence : g_rsp_str
//  Author:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessGCI)
{
    ATC_STATUS       status = S_ATC_SUCCESS;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:
            break;

        case ATC_CMD_TYPE_READ:
            status = S_ATC_DEFAULT_HANDLE;
            ATC_BuildResultErrCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_ERROR, 0);
            break;

        case ATC_CMD_TYPE_TEST:
            status = S_ATC_DEFAULT_HANDLE;
            ATC_BuildResultErrCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_ERROR, 0);
            break;

        default:
            break;
    }

    return status;
}

/*****************************************************************************/
//  Description : This function handles the AT+I1 command
//  Global resource dependence : g_rsp_str
//  Author:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessI1)
{
    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:
            break;

        case ATC_CMD_TYPE_READ:
            break;

        default:
            break;
    }

    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handles the AT+I2 command
//  Global resource dependence : g_rsp_str
//  Author:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessI2)
{
    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:
            break;

        case ATC_CMD_TYPE_READ:
            break;

        default:
            break;
    }

    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handles the AT+I3 command
//  Global resource dependence : g_rsp_str
//  Author:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessI3)
{
    ATC_STATUS       status = S_ATC_SUCCESS;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:
            //status = S_ATC_DEFAULT_HANDLE;
            sprintf((char *)g_rsp_str, "Spreadtrum SoftModem Version 1.0");
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;

        case ATC_CMD_TYPE_READ:
            break;

        default:
            break;
    }

    return status;
}

/*****************************************************************************/
//  Description : This function handles the AT+I4 command
//  Global resource dependence :
//  Author:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessI4)
{
    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:
            break;

        case ATC_CMD_TYPE_READ:
            break;

        default:
            break;
    }

    ATC_BuildResultErrCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_ERROR, 0);
    return S_ATC_DEFAULT_HANDLE;
}

/*****************************************************************************/
//  Description : This function handles the AT+I52 command
//  Global resource dependence :
//  Author:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessI5)
{
    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:
            break;

        case ATC_CMD_TYPE_READ:
            break;

        default:
            break;
    }

    ATC_BuildResultErrCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_ERROR, 0);
    return S_ATC_DEFAULT_HANDLE;
}

/*****************************************************************************/
//  Description : This function handles the AT+I6 command
//  Global resource dependence :
//  Author:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessI6)
{
    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:
            break;

        case ATC_CMD_TYPE_READ:
            break;

        default:
            break;
    }

    ATC_BuildResultErrCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_ERROR, 0);
    return S_ATC_DEFAULT_HANDLE;
}

/*****************************************************************************/
//  Description : This function handles the AT+I7 command
//  Global resource dependence :
//  Author:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessI7)
{
    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:
            break;

        case ATC_CMD_TYPE_READ:
            break;

        default:
            break;
    }


    ATC_BuildResultErrCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_ERROR, 0);
    return S_ATC_DEFAULT_HANDLE;
}

/*****************************************************************************/
//  Description : check whether a data call present or not
//  Global resource dependence :
//  Author:  Elvis.xu
//  Note:
/*****************************************************************************/
BOOLEAN ATC_IsDataConnectPresent(MN_DUAL_SYS_E dual_sys)
{
    MN_GPRS_CON_STATE_T *pdp_con_state_arr  = PNULL;
    ERR_MNGPRS_CODE_E    ret     = ERR_MNGPRS_NO_ERR;
    uint32               pdp_con_num = 0;
    uint32               i;

    ret = MNGPRS_ReadPdpContextStateEx(dual_sys, &pdp_con_state_arr, &pdp_con_num);
    if(ret != ERR_MNGPRS_NO_ERR)
    {
        if(pdp_con_state_arr != PNULL)
        {
            SCI_FREE(pdp_con_state_arr);
            pdp_con_state_arr = PNULL;
        }
        SCI_TRACE_LOW("ATC: no pdp is activated ret %d", ret);
        return FALSE;
    }

    for(i = 0; i < pdp_con_num; i++)
    {
        if(pdp_con_state_arr[i].pdp_state == MN_CONTEXT_ACTIVATED)
        {
            break;
        }
    }

    if(pdp_con_state_arr != PNULL)
    {
        SCI_FREE(pdp_con_state_arr);
        pdp_con_state_arr = PNULL;
    }

    if (i == pdp_con_num)
    {
        SCI_TRACE_LOW("ATC: no pdp is activated");
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*****************************************************************************/
//  Description : check whether a call present or not
//  Global resource dependence : g_calls_state
//  Author:  Elvis.xu
//  Note:
/*****************************************************************************/
BOOLEAN ATC_IsVoiceConnectPresent(MN_DUAL_SYS_E dual_sys)
{
    return ((g_calls_state[dual_sys].call_amount)?(TRUE):(FALSE));
}

/*****************************************************************************/
//  Description : This function handle the ATO command to recover data mode
//  Global resource dependence : g_rsp_str
//  Author:  Elvis.xu
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessO)
{
    MN_DUAL_SYS_E       dual_sys = ATC_CURRENT_SIM_ID_MUX;
#ifdef _MUX_ENABLE_
    uint32 link_id = atc_config_ptr->current_link_id;
#endif

    if (ATC_GET_CMD_TYPE != ATC_CMD_TYPE_EXECUTE)
    {
        return ERR_OPERATION_NOT_SUPPORTED;
    }

    if (ATC_IsDataConnectPresent(dual_sys) != TRUE)
    {
        SCI_TRACE_LOW("ATC: no pdp is available");
        return ERR_GPRS_SERVICES_NOT_ALLOWED;
    }

#ifdef _MUX_ENABLE_
    if(MUX_DLC_AT_MODE == g_mux_dlc_info[link_id].work_mode)
    {
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
        SCI_TRACE_LOW("ATC: set data mode link %d", MUX_DLC_AT_MODE);
        g_mux_dlc_info[link_id].work_mode = MUX_DLC_DATA_MODE;
    }
    else
    {
        SCI_TRACE_LOW("ATC: already enter into data mode");
        return S_ATC_SUCCESS;
    }
#else
    if (DATA_MODE == SIO_GetUserPortMode())
    {
        SCI_TRACE_LOW("ATC: already enter into data mode");
        return S_ATC_SUCCESS;
    }
    else
    {
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
        SIO_ATC_SetDataMode(DATA_MODE);
    }
#endif

    return S_ATC_DEFAULT_HANDLE;
}

/*****************************************************************************/
//  Description : This function handle the AT+FCLASS command
//  Global resource dependence : g_rsp_str
//  Author:  Elvis.xu
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessFCLASS)
{
    ATC_STATUS           status = S_ATC_SUCCESS;
    MN_DUAL_SYS_E        dual_sys = ATC_CURRENT_SIM_ID_MUX;
#ifdef _MUX_ENABLE_
    uint32 link_id = atc_config_ptr->current_link_id;
#endif

    switch(ATC_GET_CMD_TYPE)
    {
    case ATC_CMD_TYPE_SET:
        if(PARAM1_FLAG)
        {
            if (PARA1.num == 8)
            {
                /*if in MUX or SIO data mode, should escape with +++ firstly, then AT command can be received*/
                /*default in voice mode, so return ok directly*/
                return S_ATC_SUCCESS;
            }
            else if(PARA1.num == 0)
            {
                if (ATC_IsDataConnectPresent(dual_sys) == TRUE)
                {
                    SCI_TRACE_LOW("ATC: pdp is available");
                #ifdef _MUX_ENABLE_
                    if(MUX_DLC_AT_MODE == g_mux_dlc_info[link_id].work_mode)
                    {
                        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
                        SCI_TRACE_LOW("ATC: set data mode link %d", MUX_DLC_AT_MODE);
                        g_mux_dlc_info[link_id].work_mode = MUX_DLC_DATA_MODE;
                        return S_ATC_DEFAULT_HANDLE;
                    }
                 #else
                    if (ATC_MODE == SIO_GetUserPortMode())
                    {
                        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
                        SIO_ATC_SetDataMode(DATA_MODE);
                        return S_ATC_DEFAULT_HANDLE;
                    }
                 #endif
                }
                else
                {
                    SCI_TRACE_LOW("ATC: pdp is not available");
                }
            }
            else
            {
                status = ERR_OPERATION_NOT_SUPPORTED;
            }
        }
        else
        {
            status = ERR_OPERATION_NOT_SUPPORTED;
        }
        break;

    case ATC_CMD_TYPE_READ:
        if(ATC_IsVoiceConnectPresent(dual_sys))
        {
            sprintf((char*)g_rsp_str, "+FCLASS: 8");
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
        }

        if(ATC_IsDataConnectPresent(dual_sys))
        {
            sprintf((char*)g_rsp_str, "+FCLASS: 0");
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
        }
        break;

    case ATC_CMD_TYPE_TEST:
        sprintf((char *)g_rsp_str, "%s:0,8", g_atc_info_table[AT_CMD_FCLASS].cmd_name->str_ptr);
        ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
        break;

    default:
        status = ERR_OPERATION_NOT_SUPPORTED;
        break;
    }

    return status;
}

/*****************************************************************************/
//  Description : This function handle the ATM command
//  Global resource dependence : none
//  Author:       minqian.qian
//  Note:      Monitor speaker mode(ITU V.25 (subclause: 6.3.14))
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessM)
{
    if(PARAM1_FLAG)
    {
        if(PARAM1_TYPE != ATC_CMD_PARAM_TYPE_NUMERIC)
        {
            return ERR_INVALID_INDEX;
        }

        if(PARA1.num < 0 || PARA1.num > 2)
        {
            return ERR_OPERATION_NOT_ALLOWED;
        }
    }

    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handle the ATM command
//  Global resource dependence : none
//  Author:
//  Note:     modify the related nvram
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessNVOPERATE)
{
    return S_ATC_FAIL;
}

/******************************************************************************/
//  DESCRIPTION:
//           get wifi/bluetooth mac address
//  Global: atc_config_ptr
//  Author: winnie.zhu
//  Note :
//  History:
/******************************************************************************/
ATC_STATUS ATC_ProcessSPHAR( // return the handle result
    ATC_CONFIG_T    *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
)
{
    int32  state = 0;
    BOOLEAN ret = 0;
    uint8  temp_str[6];

    ATC_TRACE_LOW("ATC: ATC_ProcessSPHAR");

    SCI_ASSERT(PNULL != atc_config_ptr);/*assert verified: check null pointer*/
    SCI_ASSERT(PNULL != cur_cmd_info);/*assert verified: check null pointer*/

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:
            //param1--state
            if(cur_cmd_info->param_used[0])
            {
                state = PARA1.num;
            }
            else
            {
                return ERR_INVALID_INDEX;
            }

            if(1 == state)
            {
               //get BT mac addr
              SCI_MEMSET(temp_str,0,6);
              ret = BTATC_GetMac(temp_str,6);
            }
            else if(2 == state)
            {
              //get wifi mac addr
              SCI_MEMSET(temp_str,0,6);
              ret = WIFIATC_GetMac(temp_str,6);
            }
            else
            {
                //Other values will result in an ERROR
                return ERR_INVALID_INDEX;
            }
            if(!ret)
            {
                return ERR_UNKNOWN;
            }
            sprintf((char *)g_rsp_str, "%s: \"%x-%x-%x-%x-%x-%x\"", g_atc_info_table[AT_CMD_SPHAR].cmd_name->str_ptr, temp_str[0], temp_str[1], temp_str[2], temp_str[3], temp_str[4], temp_str[5]);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            return S_ATC_SUCCESS;

        case ATC_CMD_TYPE_TEST:
            sprintf((char *)g_rsp_str, "%s:(1-2)", g_atc_info_table[AT_CMD_SPHAR].cmd_name->str_ptr);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;

        default:
            return ERR_UNKNOWN;

    }

    return S_ATC_SUCCESS;
}

/******************************************************************************/
//  Description : This function handles the AT+CTZR command.This command is used
//                to set or get time and time zone report config information .
//  Global resource dependence : g_rsp_str
//  Author:
/******************************************************************************/
AT_CMD_FUNC( ATC_ProcessCTZR)
{
    ATC_STATUS    status = S_ATC_SUCCESS;

    switch(ATC_GET_CMD_TYPE)
    {

    case ATC_CMD_TYPE_SET:
        if (PARAM1_FLAG)
        {
            atc_config_ptr->tz_report_flag = (BOOLEAN)(PARA1.num);
        }
        else
        {
            status = ERR_OPERATION_NOT_ALLOWED;
        }
        break;
    case ATC_CMD_TYPE_READ:
        sprintf((char*)g_rsp_str, "%s: %d",g_atc_info_table[AT_CMD_CTZR].cmd_name->str_ptr,
            atc_config_ptr->tz_report_flag);
        ATC_BuildInfoRsp(atc_config_ptr, (uint8*)g_rsp_str);
        break;
    default:
        status = ERR_OPERATION_NOT_SUPPORTED;
        break;
    }
    return status;
}


/*****************************************************************************/
//  Description : This function handle the AT+SPENHA command
//  Global resource dependence : g_rsp_str
//  Author:
//  Note:
/*****************************************************************************/
AT_CMD_FUNC( ATC_ProcessSPENHA )
{
    ATC_STATUS        status = S_ATC_SUCCESS;
    uint8             type = 0;
    uint8             enha_type = 0;
    uint32            mode_index = 0;
    uint32            dev_number = 0;
    AUDIO_ENHA_EQ_STRUCT_T*  audenha_eq_ptr = NULL;

    AUDIO_ENHA_TUNABLE_EQ_STRUCT_T* audenha_tun_eq_ptr = PNULL;
    uint8   *str_ptr = PNULL;

    if (ATC_GET_CMD_TYPE != ATC_CMD_TYPE_SET)
    {
        return ERR_OPERATION_NOT_SUPPORTED;
    }

    if(PARAM1_FLAG)
    {
        type = (uint8)PARA1.num;
    }
    else
    {
        return ERR_INVALID_INDEX;
    }

    if(PARAM2_FLAG)
    {
        enha_type = (uint8)PARA2.num;
    }
    else
    {
        return ERR_INVALID_INDEX;
    }

    dev_number = AUDENHA_EXP_GetEqParaSetNum((BOOLEAN)type);

    switch(enha_type)
    {
    case ATC_GET_ENHA_MODE_NUM:
        sprintf((char*)g_rsp_str, "%s: %ld", g_atc_info_table[AT_CMD_SPENHA].cmd_name->str_ptr, dev_number);
        ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
        break;
    case ATC_GET_ENHA_MODE_DATA:
        if(PARAM3_FLAG)
        {
            if((PARAM3_TYPE == ATC_CMD_PARAM_TYPE_NUMERIC) && ((uint32)PARA3.num <= dev_number) && ((uint32)PARA3.num > 0))
            {

                mode_index = (uint32)PARA3.num;

                if(0 == type)
                {
                    SCI_TRACE_LOW("ATC: spenha eq mode_index = %d, dev_number = %d, %d", mode_index, dev_number, sizeof(AUDIO_ENHA_EQ_STRUCT_T));
                    if(mode_index <= dev_number)
                    {
                        audenha_eq_ptr = (AUDIO_ENHA_EQ_STRUCT_T*)SCI_ALLOC_BASEZ(sizeof(AUDIO_ENHA_EQ_STRUCT_T));
                        SCI_ASSERT(audenha_eq_ptr != PNULL); /*assert verified*/
                        str_ptr = (uint8*)SCI_ALLOC_BASEZ((sizeof(AUDIO_ENHA_EQ_STRUCT_T)+1)*2);
                        SCI_ASSERT(str_ptr != PNULL); /*assert verified*/

                        if(AUDIO_NO_ERROR != AUDENHA_EXP_GetEqSetFromRam(mode_index, audenha_eq_ptr))
                        {
                            SCI_TRACE_LOW("AGC_Get_Param failed to get mode_index, mode_index :%d.", mode_index);
                            SCI_FREE(audenha_eq_ptr);
                            SCI_FREE(str_ptr);
                            return ERR_NOT_FOUND;
                        }

                        ConvertBinToHex((uint8*)(audenha_eq_ptr), sizeof(AUDIO_ENHA_EQ_STRUCT_T), str_ptr);

                        sprintf((char*)g_rsp_str, "%s: %ld,%s",
                                g_atc_info_table[AT_CMD_SPENHA].cmd_name->str_ptr,
                                mode_index,
                                str_ptr);
                        SCI_FREE(audenha_eq_ptr);
                        SCI_FREE(str_ptr);
                        ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);

                    }
                }
                else
                {
                    SCI_TRACE_LOW("ATC: spenha tun_eq mode_index = %d, dev_number = %d, %d", mode_index, dev_number, sizeof(AUDIO_ENHA_TUNABLE_EQ_STRUCT_T));
                    if(mode_index <= dev_number)
                    {
                        audenha_tun_eq_ptr = (AUDIO_ENHA_TUNABLE_EQ_STRUCT_T*)SCI_ALLOC_BASEZ(sizeof(AUDIO_ENHA_TUNABLE_EQ_STRUCT_T));
                        SCI_ASSERT(audenha_tun_eq_ptr != PNULL); /*assert verified*/

                        str_ptr = (uint8*)SCI_ALLOC_BASEZ((sizeof(AUDIO_ENHA_TUNABLE_EQ_STRUCT_T)+1)*2);
                        SCI_ASSERT(str_ptr != PNULL); /*assert verified*/

                        if(AUDIO_NO_ERROR != AUDENHA_EXP_GetTunableEqSetFromRam(mode_index, audenha_tun_eq_ptr))
                        {
                            SCI_TRACE_LOW("AGC_Get_Param failed to get mode_index, mode_index :%d.", mode_index);
                            SCI_FREE(audenha_tun_eq_ptr);
                            SCI_FREE(str_ptr);
                            return ERR_NOT_FOUND;
                        }

                        ConvertBinToHex((uint8*)(audenha_tun_eq_ptr), sizeof(AUDIO_ENHA_TUNABLE_EQ_STRUCT_T), str_ptr);

                        sprintf((char*)g_rsp_str, "%s: %ld,%s",
                                g_atc_info_table[AT_CMD_SPENHA].cmd_name->str_ptr,
                                mode_index,
                                str_ptr);
                        SCI_FREE(audenha_tun_eq_ptr);
                        SCI_FREE(str_ptr);
                        ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);

                    }
                }
            }
            else
            {
                status = ERR_INVALID_INDEX;
            }
        }
        else
        {
            status = ERR_INVALID_INDEX;
        }
        break;
    case ATC_SET_ENHA_MODE_DATA:
        if(PARAM3_FLAG && PARAM4_FLAG)
        {
            if (((PARAM3_TYPE == ATC_CMD_PARAM_TYPE_NUMERIC) && ((uint32)PARA3.num <= dev_number)&& ((uint32)PARA3.num > 0))
                  &&
                ((PARAM4_TYPE == ATC_CMD_PARAM_TYPE_NUMERIC) && ((uint32)PARA4.num <= AUD_ENHA_EQMODE_SEL_SOFTROCK) && ((uint32)PARA4.num > 0))
                )
            {
                s_atc_admm_index = (uint8)PARA3.num;
                s_enha_eqmode_index = (uint8)PARA4.num;
                /* output the '>' */
                if(0 == type)
                {
                    ATC_SendAsyncInfo(atc_config_ptr, ATC_CTRLZ_ENHA);
                }
                else
                {
                    ATC_SendAsyncInfo(atc_config_ptr, ATC_CTRLZ_ENHA_TUN);
                }
                status = S_ATC_DEFAULT_HANDLE;
            }
            else
            {
                status = ERR_INVALID_INDEX;
            }
        }
        else
        {
            status = ERR_INVALID_INDEX;
        }
        break;

    case ATC_GET_ENHA_MODE_NV_DATA:
        if ((!PARAM3_FLAG) ||
            (!(PARAM3_TYPE == ATC_CMD_PARAM_TYPE_NUMERIC && (uint32)PARA3.num <= dev_number && (uint32)PARA3.num > 0)))
        {
            status = ERR_INVALID_INDEX;
            break;
        }
        mode_index = (uint32)PARA3.num;

        if(0 == type)
        {
            SCI_TRACE_LOW("ATC: spenha eq mode_index = %d, dev_number = %d, %d", mode_index, dev_number, sizeof(AUDIO_ENHA_EQ_STRUCT_T));

            audenha_eq_ptr = (AUDIO_ENHA_EQ_STRUCT_T*)SCI_ALLOC_BASEZ(sizeof(AUDIO_ENHA_EQ_STRUCT_T));
            SCI_ASSERT(audenha_eq_ptr != PNULL); /*assert verified*/
            str_ptr = (uint8*)SCI_ALLOC_BASEZ((sizeof(AUDIO_ENHA_EQ_STRUCT_T)+1)*2);
            SCI_ASSERT(str_ptr != PNULL); /*assert verified*/

            if(AUDIO_NO_ERROR != AUDENHA_EXP_GetEqSetFromFlash(mode_index, audenha_eq_ptr))
            {
                SCI_TRACE_LOW("AGC_Get_Param failed to get mode_index, mode_index:%d.", mode_index);
                SCI_FREE(audenha_eq_ptr);
                SCI_FREE(str_ptr);
                return ERR_NOT_FOUND;
            }

            ConvertBinToHex((uint8*)(audenha_eq_ptr), sizeof(AUDIO_ENHA_EQ_STRUCT_T), str_ptr);

            sprintf((char*)g_rsp_str, "%s: %ld,%s",
                    g_atc_info_table[AT_CMD_SPENHA].cmd_name->str_ptr,
                    mode_index,
                    str_ptr);
            SCI_FREE(audenha_eq_ptr);
            SCI_FREE(str_ptr);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
        }
        else
        {
            SCI_TRACE_LOW("ATC: spenha tun_eq mode_index = %d, dev_number = %d, %d", mode_index, dev_number, sizeof(AUDIO_ENHA_TUNABLE_EQ_STRUCT_T));

            audenha_tun_eq_ptr = (AUDIO_ENHA_TUNABLE_EQ_STRUCT_T*)SCI_ALLOC_BASEZ(sizeof(AUDIO_ENHA_TUNABLE_EQ_STRUCT_T));
            SCI_ASSERT(audenha_tun_eq_ptr != PNULL); /*assert verified*/

            str_ptr = (uint8*)SCI_ALLOC_BASEZ((sizeof(AUDIO_ENHA_TUNABLE_EQ_STRUCT_T)+1)*2);
            SCI_ASSERT(str_ptr != PNULL); /*assert verified*/

            if(AUDIO_NO_ERROR != AUDENHA_EXP_GetTunableEqSetFromFlash(mode_index, audenha_tun_eq_ptr))
            {
                SCI_TRACE_LOW("AGC_Get_Param failed to get mode_index, mode_index:%d.", mode_index);
                SCI_FREE(audenha_tun_eq_ptr);
                SCI_FREE(str_ptr);
                return ERR_NOT_FOUND;
            }

            ConvertBinToHex((uint8*)(audenha_tun_eq_ptr), sizeof(AUDIO_ENHA_TUNABLE_EQ_STRUCT_T), str_ptr);

            sprintf((char*)g_rsp_str, "%s: %ld,%s",
                    g_atc_info_table[AT_CMD_SPENHA].cmd_name->str_ptr,
                    mode_index,
                    str_ptr);
            SCI_FREE(audenha_tun_eq_ptr);
            SCI_FREE(str_ptr);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);

        }
        break;
    case ATC_SET_ENHA_MODE_NV_DATA:
        if(PARAM3_FLAG)
        {
            if ((PARAM3_TYPE == ATC_CMD_PARAM_TYPE_NUMERIC) && ((uint32)(PARA3.num) <= dev_number) && ((uint32)(PARA3.num) > 0))
            {
                s_atc_admm_index = (uint8)PARA3.num;
                /* output the '>'*/
                if(0 == type)
                {
                    ATC_SendAsyncInfo(atc_config_ptr, ATC_CTRLZ_ENHANV);
                }
                else
                {
                    ATC_SendAsyncInfo(atc_config_ptr, ATC_CTRLZ_ENHANV_TUN);
                }

                status = S_ATC_DEFAULT_HANDLE;
            }
            else
            {
                status = ERR_INVALID_INDEX;
            }
        }
        else
        {
            status = ERR_INVALID_INDEX;
        }
        break;
    default:
        SCI_ASSERT(FALSE); /*assert verified*/
        break;
    }

    return status;
}

/*****************************************************************************/
//  Description : This function handles ^CURC
//  Global resource dependence : g_rsp_str
//  Author:
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_Processxor_CURC)
{
    ATC_STATUS       status = S_ATC_SUCCESS;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:
            atc_config_ptr->urc_switch = PARA1.num;
            break;
        case ATC_CMD_TYPE_READ:
            sprintf((char*)g_rsp_str, "%s: %d",
                    "^CURC",atc_config_ptr->urc_switch);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;
        case ATC_CMD_TYPE_TEST:
            sprintf((char*)g_rsp_str, "%s: %s",
                "^CURC", "(0-1)");
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;
        default:
            status = ERR_OPERATION_NOT_SUPPORTED;
            break;
    }
    return status;
}

/*****************************************************************************/
//  Description : This function handles AT+SSRVT, set ms to service type
//  Global resource dependence : g_rsp_str
//  Author:
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSSRVT)
{
    ATC_STATUS       status = S_ATC_SUCCESS;
    MN_DUAL_SYS_E    dual_sys = ATC_CURRENT_SIM_ID_MUX;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:
            MNPHONE_SetServiceTypeEx(dual_sys, PARA1.num);
            break;
        case ATC_CMD_TYPE_READ:
            status = ERR_OPERATION_NOT_SUPPORTED;
            break;
        default:
            status = ERR_OPERATION_NOT_SUPPORTED;
            break;
    }
    return status;
}

/*****************************************************************************/
//  Description : This function handles AT+SSCR for atc audio record
//  Global resource dependence : atc_record_codec
//  Author:
//  Note:
/*****************************************************************************/
//extern HAUDIODEV hLAYER1;
//extern HAUDIODEV hMixDev;
AT_CMD_FUNC( ATC_ProcessSSCR)
{
    ATC_STATUS           status = S_ATC_SUCCESS;

#ifdef _MUX_ENABLE_
    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:

            if (PARAM1_TYPE != ATC_CMD_PARAM_TYPE_NUMERIC)
            {
                return ERR_OPERATION_NOT_ALLOWED;
            }

            switch (PARA1.num)
            {
                case 0:
                    atc_config_ptr->audio_rec = ATC_AUDIO_REC_STOP;
                    #ifdef _ATC_WAIT_L4
                    if( AUDIO_ERROR==VOICE_RECORD_Stop(hLAYER1))
                        #endif
                    {
                        SCI_TRACE_LOW("ATC_ProcessSSCR, audio record stop error");
                        status = ERR_INVALID_INDEX;
                    }
                    break;

                case 1:
                    atc_config_ptr->audio_rec = ATC_AUDIO_REC_START;
                    #ifdef _ATC_WAIT_L4
                    if( AUDIO_ERROR==VOICE_RECORD_Start(hLAYER1))
                    #endif
                    {
                        SCI_TRACE_LOW("ATC_ProcessSSCR, audio record start error");
                        status = ERR_INVALID_INDEX;
                    }
                    break;

                case 2:
                #ifdef BT_MIX_SUPPORT
                    atc_config_ptr->audio_rec = ATC_AUDIO_BT_REC_START;
                #ifdef _ATC_WAIT_L4
                    if( AUDIO_ERROR==VOICE_RECORD_Start(hMixDev))
                #endif
                    {
                        SCI_TRACE_LOW("ATC_ProcessSSCR, audio bt record start error");
                        status = ERR_INVALID_INDEX;
                    }
                    #else
                        status = ERR_OPERATION_NOT_SUPPORTED;
                #endif
                    break;

                default :
                    status = ERR_OPERATION_NOT_SUPPORTED;
                    break;
            }
            break;

        case ATC_CMD_TYPE_READ:
            sprintf((char*)g_rsp_str,"+SSCR: %d", atc_config_ptr->audio_rec);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;

        default:
            status = ERR_OPERATION_NOT_SUPPORTED;
            break;
    }

#endif
    return status;

}
#endif

/*****************************************************************************/
//  Description : This function is used to chech the time string.
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
LOCAL BOOLEAN CheckTimeStr(
    uint8      length,
    uint8      *str,
    SCI_DATE_T   *date_ptr,
    SCI_TIME_T   *tm_ptr
)
{
    int32            i;


    if(length != AT_CCLK_STR_LEN)
    {
        ATC_TRACE_LOW("ATC: the timer string length is error %d", length);
        return FALSE;
    }

    // YY/MM/DD,HH:MM:SS
    if(str[2]  != '/' ||
    str[5]  != '/' ||
    str[8]  != ',' ||
    str[11] != ':' ||
    str[14] != ':')
    {
        ATC_TRACE_LOW("ATC: The timer string format error");
        return FALSE;
    }

    // Check year string
    for(i = 0; i < 2; i++)
    {
        if(str[i] > '9' || str[i] < '0')
        {
            ATC_TRACE_LOW("ATC: the value of year is error");
            return FALSE;
        }
    }

    date_ptr->year = 2000 + (str[0] - '0') * 10 + (str[1] - '0');

    // check month string

    if((str[3] > '1' || str[3] < '0') &&
    (str[4] > '9' || str[4] < '0'))
    {
        ATC_TRACE_LOW("ATC: the month digital is error");
        return FALSE;
    }

    date_ptr->mon = (str[3] - '0') * 10 + (str[4] - '0');

    if(date_ptr->mon > AT_MAX_MONTH)
    {
        ATC_TRACE_LOW("ATC: the month value is error");
        return FALSE;
    }

    // Check day string
    date_ptr->mday = (str[6] - '0') * 10 + (str[7] - '0');

    if(date_ptr->mday > AT_MAX_DAY)
    {
        ATC_TRACE_LOW("ATC: the day value is error");
        return FALSE;
    }

    // check hour string
    tm_ptr->hour = (str[9] - '0') * 10 + (str[10] - '0');

    if(tm_ptr->hour > AT_MAX_HOUR)
    {
        ATC_TRACE_LOW("ATC: the hour value is error");
        return FALSE;
    }

    // check minute string
    tm_ptr->min = (str[12] - '0') * 10 + (str[13] - '0');

    if(tm_ptr->min > AT_MAX_MINUTE)
    {
        ATC_TRACE_LOW("ATC: the minute value is error");
        return FALSE;
    }

    // check second string
    // check minute string
    tm_ptr->sec = (str[15] - '0') * 10 + (str[16] - '0');

    if(tm_ptr->sec > AT_MAX_MINUTE)
    {
        ATC_TRACE_LOW("ATC: the second value is error");
        return FALSE;
    }

    ATC_TRACE_LOW("ATC: the timer parameter is right");
    return TRUE;
}


/*****************************************************************************/
//  Description : This function is used to restore the configuration profile.
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
LOCAL void RestoreConfig(
    ATC_CONFIG_T   *atc_config_ptr,
    MN_DUAL_SYS_E   dual_sys
)
{
    // set the following parameter to the defualt value
    atc_config_ptr->atc_config_nv.creg       = DEF_CREG;
    atc_config_ptr->atc_config_nv.e_value    = FALSE;
    atc_config_ptr->atc_config_nv.clip_flag  = TRUE;
    atc_config_ptr->atc_config_nv.clir_flag  = FALSE;
    atc_config_ptr->atc_config_nv.colp_flag  = FALSE;
    atc_config_ptr->atc_config_nv.cmee       = NUMERIC_ERR_VALUE_MODE;
    atc_config_ptr->atc_config_nv.cmgf_flag  = FALSE;
    atc_config_ptr->atc_config_nv.sidet_flag = FALSE;
    atc_config_ptr->atc_config_nv.wind_ind_level            = ATC_SIND_DEFAULT_VAL;
    atc_config_ptr->ccwa_flag[dual_sys] = TRUE;
    atc_config_ptr->cscs[dual_sys] = ATC_CHSET_IRA;

    //The following is about STK
    atc_config_ptr->atc_config_nv.stk_actflag   = TRUE;
    atc_config_ptr->atc_config_nv.stk_time      = ATC_STKTIME_DEFAULT_VAL;

    ATC_SetDefaultParams(atc_config_ptr, dual_sys);

#ifndef _ULTRA_LOW_CODE_
#ifndef ATC_SAT_ENABLE
    ATC_CheckAndHandleConfig(dual_sys, atc_config_ptr, g_stk_terminal_profile[AT_STK_TRP_MAX_VAL]);
#endif /*atc_sat_EANBLE*/
#endif /* _ULTRA_LOW_CODE_ */

    ATC_RestoreSmsConfig(dual_sys, atc_config_ptr);
    ATC_RestoreGsmConfig(atc_config_ptr, dual_sys);
    ATC_RestoreGsmSSConfig(dual_sys);
    //write  nv item
    ATC_WriteNVItem(atc_config_ptr, NV_ATC_CONFIG_ID);

#ifdef _MUX_ENABLE_
    ATC_SetEcho(atc_config_ptr->atc_config_nv.e_value);
    SIO_ATC_SetEcho(atc_config_ptr->atc_config_nv.e_value);
#else
    SIO_ATC_SetEcho(atc_config_ptr->atc_config_nv.e_value);
#endif
}

LOCAL void ATC_SetDefaultParams(
    ATC_CONFIG_T   *atc_config_ptr,
    MN_DUAL_SYS_E   dual_sys
)
{
    SCI_ASSERT(PNULL != atc_config_ptr);/*assert verified: check null pointer*/
    atc_config_ptr->s0_reg = DEF_S0;
    atc_config_ptr->s2_reg = DEF_S2;
    atc_config_ptr->s3_reg = DEF_S3;
    atc_config_ptr->s4_reg = DEF_S4;
    atc_config_ptr->s5_reg = DEF_S5;
    atc_config_ptr->s6_reg = DEF_S6;
    atc_config_ptr->s7_reg = DEF_S7;
    atc_config_ptr->s8_reg = DEF_S8;
    atc_config_ptr->s10_reg = DEF_S10;

    atc_config_ptr->cr = DEF_CR;
    atc_config_ptr->crc = DEF_CRC;

    atc_config_ptr->cgreg = DEF_CGREG;

    atc_config_ptr->l_value = DEF_L;
    atc_config_ptr->m_value = DEF_M;
    atc_config_ptr->p_value = DEF_P;
    atc_config_ptr->q_value = DEF_Q;
    atc_config_ptr->v_value = DEF_V;
    atc_config_ptr->x_value = DEF_X;

    atc_config_ptr->cmod = DEF_CMOD;

    atc_config_ptr->cbst.speed = DEF_CBST_SPEED;
    atc_config_ptr->cbst.name = DEF_CBST_NAME;
    atc_config_ptr->cbst.ce = DEF_CBST_CE;

    atc_config_ptr->cops[dual_sys].mode = (PLMN_SELECTION_MODE_E)DEF_COPS_MODE;
    atc_config_ptr->cops[dual_sys].format = (ATC_PLMN_FORMAT_E)DEF_COPS_FORMAT;
}

/*****************************************************************************/
//  Description : This function check the password of Clck.
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
#if 0 //@Hyman wu

LOCAL BOOLEAN CheckClckPsw(
    uint8            len,
    uint8            *psw_ptr
)
{
    int32               i;

    for(i = 0; i < len; i++)
    {
        if(*psw_ptr > '9' || *psw_ptr < '0')
        {
            return FALSE;
        }
    }

    return TRUE;
}
/*****************************************************************************/
//  Description : This function convert the ATC Clck to the MNSS's code
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
LOCAL void ConvertClckToSsCode(
    uint8         clck_code,
    MN_SS_CODE_E  *ss_code_ptr
)
{
    SCI_ASSERT(PNULL != ss_code_ptr);

    switch(clck_code)
    {
        case AT_CLCK_ALL_BAR_SVR:
            *ss_code_ptr = MN_SS_ALL_BAR_SERVICE;
            break;
        case AT_CLCK_BAR_OUTGOING:
            *ss_code_ptr = MN_SS_BAR_OUTGOING;
            break;
        case AT_CLCK_BAOC:
            *ss_code_ptr = MN_SS_BAOC;
            break;
        case AT_CLCK_BOIC:
            *ss_code_ptr = MN_SS_BOIC;
            break;
        case AT_CLCK_BOIC_EXHC:
            *ss_code_ptr = MN_SS_BOIC_EXHC;
            break;
        case AT_CLCK_BAR_INCOMING:
            *ss_code_ptr = MN_SS_BAR_INCOMING;
            break;
        case AT_CLCK_BAIC :
            *ss_code_ptr = MN_SS_BAIC;
            break;
        case AT_CLCK_BICROAM:
            *ss_code_ptr = MN_SS_BICROAM;
            break;
        default:
            ATC_TRACE_LOW("ATC: Assert Failure unknow clck_code.");
            break;
    }
}
#endif /*0*/

/******************************************************************************/
//  Description : This function convert APP_MN_NETWORK_STATUS_IND_T infos to MN_PHONE_CURRENT_PLMN_INFO_T
//  Global resource dependence : None
//  Author : minqian.qian
//  Note : None
/******************************************************************************/
MN_PHONE_CURRENT_PLMN_INFO_T ConvertNWStatusToPLMNInfo(
    APP_MN_NETWORK_STATUS_IND_T *nw_plmn_status
)
{
    MN_PHONE_CURRENT_PLMN_INFO_T    info;

    SCI_ASSERT(PNULL != nw_plmn_status);/*assert verified: check null pointer*/

    info.plmn_status = nw_plmn_status->plmn_status;

    if(PLMN_NO_SERVICE != nw_plmn_status->plmn_status)
    {
        info.mnc = nw_plmn_status->mnc;
        info.mcc = nw_plmn_status->mcc;
        info.mnc_digit_num = nw_plmn_status->mnc_digit_num;
        info.cell_id = nw_plmn_status->cell_id;
        info.lac = nw_plmn_status->lac;
        info.plmn_is_roaming = nw_plmn_status->plmn_is_roaming;
        info.rat = nw_plmn_status->rat;
        //info.cell_capability.mbms_support = nw_plmn_status->mbms_support;
        //info.cell_capability.hsdpa_support = nw_plmn_status->hsdpa_support;
        //info.cell_capability.hsupa_support = nw_plmn_status->hsupa_support;
        //info.cell_capability.edge_support = nw_plmn_status->edge_support;
    }
    else
    {
        info.mnc = 0;
        info.mcc = 0;
        info.mnc_digit_num = 0;
        info.cell_id = 0;
        info.lac = 0;
        info.plmn_is_roaming = FALSE;
        info.rat = MN_GMMREG_RAT_UNKNOWN;
        //info.cell_capability.mbms_support = FALSE;
        //info.cell_capability.hsdpa_support = FALSE;
        //info.cell_capability.hsupa_support = FALSE;
        //info.cell_capability.edge_support = FALSE;
    }

    return info;
}

/*****************************************************************************/
//  Description : This function handles the AT+SSYS command.This command is used
//                to set or get the operated sim id .
//  Global resource dependence : g_rsp_str
//  Author:
/*****************************************************************************/
AT_CMD_FUNC( ATC_ProcessSSYS )
{
    ATC_STATUS    status = S_ATC_SUCCESS;

    switch(ATC_GET_CMD_TYPE)
    {
    case ATC_CMD_TYPE_SET:
        if (PARAM1_FLAG)
        {
#ifdef _MUX_ENABLE_
           if(g_mux_sys_info[MUX_1].is_start!=MUX_STATUS_OFF)
           {
                status = ERR_OPERATION_NOT_ALLOWED;
                break;
           }
#endif
            if (PARA1.num < MN_SYS_NUMBER)
            {
                g_current_card_id = (MN_DUAL_SYS_E)(PARA1.num);
            }
            else
            {
                status = ERR_OPERATION_NOT_ALLOWED;
            }
        }
        else
        {
            status = ERR_OPERATION_NOT_ALLOWED;
        }
        break;
    case ATC_CMD_TYPE_READ:
        sprintf((char*)g_rsp_str, "%s: %d",g_atc_info_table[AT_CMD_SSYS].cmd_name->str_ptr,
            g_current_card_id);
        ATC_BuildInfoRsp(atc_config_ptr, (uint8*)g_rsp_str);
        break;
    default:
        SCI_ASSERT(FALSE);/* assert verified: NULL_PTR */
        break;
    }
    return status;
}

#ifndef ATC_SAT_ENABLE
/*****************************************************************************/
//  Description : This function get BT mac address.
//  Global resource dependence : g_rsp_str
//  Author:winnie.zhu
/*****************************************************************************/
LOCAL BOOLEAN BTATC_GetMac(uint8 *bt_addr, uint8 max_data_len)
{
    NVITEM_ERROR_E  status  = NVERR_NONE;
    BT_NV_PARAM    nv_param;

    status = EFS_NvitemRead(NV_BT_CONFIG, sizeof(BT_NV_PARAM), (uint8*)&nv_param);
    if(NVERR_NONE != status)
    {
        return FALSE;
    }

    SCI_MEMCPY(bt_addr,(uint8*)&(nv_param.bd_addr),  sizeof(BT_ADDRESS));
    SCI_TRACE_LOW("ATC_BT:info:GetMAC is %x-%x-%x-%x-%x-%x", bt_addr[0], bt_addr[1], bt_addr[2], bt_addr[3], bt_addr[4], bt_addr[5]);

    return TRUE;
}

/*****************************************************************************/
//  Description : This function get WiFi mac address.
//  Global resource dependence : g_rsp_str
//  Author:winnie.zhu
/*****************************************************************************/
LOCAL BOOLEAN  WIFIATC_GetMac(uint8 *mac_addr, uint8 max_data_len)
{
    NVITEM_ERROR_E  status  = NVERR_NONE;
    WIFI_NV_PARAM_T nv_param = {0};

    status = EFS_NvitemRead(NV_WIFI_CONFIG, sizeof(WIFI_NV_PARAM_T), (uint8*)&nv_param);
    if(NVERR_NONE != status)
    {
         return FALSE;
    }

    SCI_MEMCPY(mac_addr, nv_param.addr.address, sizeof(nv_param.addr.address));

    SCI_TRACE_LOW("ATC_WIFI:info:GetMAC is %x-%x-%x-%x-%x-%x", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    return TRUE;
}
#endif

/*********************************************************************audio func ***********************/

/*****************************************************************************/
//  Description : play local network tone to simulate network tone
//  Global resource dependence : none
//  Author: xiaonian.li
//  Note:
/*****************************************************************************/
LOCAL void ATC_PlayLocalNetTone(
                uint32 tone_id,
                uint32 duration,
                uint32 freq
                )
{
    GENERIC_TONE_ADP_EXT_OPE_PLAY_PARA_T tGenericPara = {0};

    uint32 result = 0;

    SCI_TRACE_LOW("ATC:ATC_PlayLocalNetTone()");

    if(NULL == hAtcGenericRing)
    {
        hAtcGenericRing  = AUDIO_CreateNormalHandle(hGenericCodec,  SCI_NULL, hLAYER1,     SCI_NULL, AudioDummyNotifyCallback);
        SCI_ASSERT(NULL != hAtcGenericRing); /* assert verified: invalid value */
    }

    tGenericPara.tone_id = (AUD_GENERIC_TONE_ID_E)tone_id;

    tGenericPara.duration = duration;

    tGenericPara.hAudioHandle = hAtcGenericRing;

    tGenericPara.volume = 16384;

    AUDIO_ExeCodecExtOpe(
        hAtcGenericRing,
        ANSI2UINT16("EXT_PLAY"),
        GENERIC_TONE_ADP_EXT_OPE_PLAY_PARA_COUNT,
        &tGenericPara,
        &result
        );
}

/*****************************************************************************/
//  Description : This function handles the ATC_NET_RING_EXP_IND signal
//  Global resource dependence : none
//  Author:       xiaonian.li
//  Note:
/*****************************************************************************/
void ATC_ProcessNetRingExpInd(ATC_CONFIG_T  *atc_config_ptr)
{
    if(is_play_local_tone)
    {
        ATC_PlayLocalNetTone(ATC_NT_RING_CON_ID, ATC_1SECONDS,ATC_AUDIO_TONE_DEFAULT_FREQ);
        SCI_SET_TIMER(P_ATC,
                  ATC_CC_3SECOND,
                  ATC_NET_RING_EXP_IND,
                  &s_network_ring_timer,
                  "ATC_NET_RING_EXP_IND");
    }
}

/*****************************************************************************/
//  Description : if MS is not attach user connection, ms start playing local
//                ring to simulate the net ring
//  Global resource dependence :
//  Author:xiaonian.li
//  Note:
/*****************************************************************************/
LOCAL void ATC_StartLocalNetRing(void)
{
    SCI_TRACE_LOW("ATC: StartLocalNetRing");
    ATC_PlayLocalNetTone(ATC_NT_RING_CON_ID, ATC_1SECONDS,ATC_AUDIO_TONE_DEFAULT_FREQ);

    if (0 != is_play_local_tone)
    {
        SCI_KILL_TIMER(P_ATC,
                   ATC_NET_RING_EXP_IND,
                   &s_network_ring_timer,
                   "ATC_NET_RING_EXP_IND");

        is_play_local_tone = FALSE;
    }

    SCI_SET_TIMER(P_ATC,
                  ATC_CC_3SECOND,
                  ATC_NET_RING_EXP_IND,
                  &s_network_ring_timer,
                  "ATC_NET_RING_EXP_IND");

    is_play_local_tone = TRUE;
}

/*****************************************************************************/
//  Description : tch assigned or call disconnected, mo stop playing
//                local ring to simulate the net ring
//  Global resource dependence :
//  Author:xiaonian.li
//  Note:
/*****************************************************************************/
LOCAL void ATC_StopLocalNetRing(void)
{
    uint32 result = 0;
    if (is_play_local_tone)
    {
        SCI_TRACE_LOW("ATC:StopLocalNetRing");

        SCI_KILL_TIMER(P_ATC,
                   ATC_NET_RING_EXP_IND,
                   &s_network_ring_timer,
                   "ATC_NET_RING_EXP_IND");

        is_play_local_tone = FALSE;

        AUDIO_ExeCodecExtOpe(
                hAtcGenericRing,
                ANSI2UINT16("EXT_STOP"),
                0,
                PNULL,
                &result
                );
    }
}

/*****************************************************************************/
//  Description : open/close downlink mute
//  Global resource dependence :
//  Author: xiaonian.li
//  Note:
/*****************************************************************************/
BOOLEAN ATC_EnableRemoteMute ( BOOLEAN is_enable )
{
    AUDIO_RESULT_E result = AUDIO_NO_ERROR;
    BOOLEAN        audio_downlink_mute_mode   = FALSE;

    AUDIO_GetDownLinkMute(&audio_downlink_mute_mode);

    SCI_TRACE_LOW("ATC: ATC_EnableRemoteMute old mute:%d, new mute:%d",
        audio_downlink_mute_mode, is_enable);

    if (is_enable != audio_downlink_mute_mode)
    {
        result = AUDIO_SetDownLinkMute(is_enable);
    }

    if(result == AUDIO_NO_ERROR)
    {
        s_audio_status.is_remote_mute = is_enable;
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************/
//  Description : open/close Uplink mute
//  Global resource dependence :
//  Author: xiaonian.li
//  Note:
/*****************************************************************************/
BOOLEAN ATC_EnableUplinkMute ( BOOLEAN is_enable )
{
    AUDIO_RESULT_E result = AUDIO_NO_ERROR;
    BOOLEAN        audio_uplink_mute_mode   = FALSE;

    AUDIO_GetDownLinkMute(&audio_uplink_mute_mode);

    SCI_TRACE_LOW("ATC: ATC_EnableUplinkMute old mute:%d, new mute:%d",
        audio_uplink_mute_mode, is_enable);

    if (is_enable != audio_uplink_mute_mode)
    {
        result = AUDIO_SetUpLinkMute(is_enable);
    }

    if(result == AUDIO_NO_ERROR)
    {
        s_audio_status.is_mute = is_enable;
        return TRUE;
    }
    return FALSE;
}


/*****************************************************************************/
//  Description : HandleMoMuteAndLocalRingState
//  Global resource dependence :
//  Author:xiaonian.li
//  Note: open downlink mute if MS has received APP_MN_SYNC_IND,but not received
//        APP_MN_SETUP_COMPLETE_CNF(APP_MN_PROGRESS_IND);
//        close downlink mute after receiving APP_MN_PROGRESS_IND or
//        APP_MN_SETUP_COMPLETE_CNF
/*****************************************************************************/
BOOLEAN  ATC_HandleMoMuteAndLocalRingState(MN_DUAL_SYS_E dual_sys, uint16 msg_id, uint8 call_index)
{
    // just handle during the first call
    if(g_calls_state[dual_sys].call_amount > 1)
    {
        if (APP_MN_PROGRESS_IND == msg_id)
        {
            // network maybe not send alerting or connect message, directly sends progress message to inform
            // ms listenning the vocie notification, so set the call state to ATC_CALL_ALERTING_STATE
            if (ATC_CALL_OUTGOING_STATE == g_calls_state[dual_sys].call_context[call_index].call_entity_state)
            {
                g_calls_state[dual_sys].call_context[call_index].call_entity_state = ATC_CALL_ALERTING_STATE;
            }
        }
        return FALSE;
    }

    ATC_TRACE_LOW("ATC: ATC_HandleMoMuteAndLocalRingState(): msg_id=%d,s_is_downlnk_mute_mocalling_flag= %d",
            msg_id, s_is_downlnk_mute_mocalling_flag);

    switch(msg_id)
    {
    case APP_MN_CALL_START_IND:
        // s_is_downlnk_mute_mocalling_flag must be MO_CALL_STATE_INIT;
        s_is_downlnk_mute_mocalling_flag = MO_CALL_STATE_INIT;
        ATC_EnableRemoteMute(TRUE);
        break;

    case APP_MN_SYNC_IND:
        s_is_downlnk_mute_mocalling_flag |= MO_CALL_STATE_SYNC;
        break;

    case APP_MN_SETUP_COMPLETE_CNF:
    case APP_MN_PROGRESS_IND:
        s_is_downlnk_mute_mocalling_flag |= MO_CALL_STATE_SETUP_OR_PROGRESS;
        break;

    // reset initial value
    case APP_MN_CALL_DISCONNECTED_IND:
    case APP_MN_CALL_ERR_IND:
        ATC_EnableRemoteMute(FALSE);
        s_is_downlnk_mute_mocalling_flag = MO_CALL_STATE_INIT;
        ATC_StopLocalNetRing();
        break;

    case APP_MN_ALERTING_IND:
        if (MO_CALL_STATE_DONE != s_is_downlnk_mute_mocalling_flag)
        {
            ATC_StartLocalNetRing();
        }
        break;

    default:
        SCI_ASSERT(FALSE); // assert verified, shouldn't be present
        break;
    }

    if(MO_CALL_STATE_DONE == s_is_downlnk_mute_mocalling_flag)
    {
        ATC_EnableRemoteMute(FALSE);
        ATC_StopLocalNetRing();
        // network maybe not send alerting or connect message, directly sends progress message to inform
        // ms listenning the vocie notification, so set the call state to ATC_CALL_ALERTING_STATE
        if ((APP_MN_SYNC_IND == msg_id) || (APP_MN_PROGRESS_IND == msg_id))
        {
            if(ATC_CALL_OUTGOING_STATE == g_calls_state[dual_sys].call_context[call_index].call_entity_state)
            {
                 g_calls_state[dual_sys].call_context[call_index].call_entity_state = ATC_CALL_ALERTING_STATE;
            }
        }
    }
    return TRUE;
}

/*****************************************************************************/
//  Description : handle APP_MN_GET_SIM_TYPE_CNF signal
//  Global resource dependence :
//  Author:
//  Note:
/*****************************************************************************/
void ATC_ProcessSimTypeCnf(
    ATC_CONFIG_T *atc_config_ptr,
    APP_MN_GET_SIM_TYPE_CNF_T *sig_ptr
    )
{
#ifndef ATC_SAT_ENABLE
    MN_CARD_TYPE_E card_type = MN_CARD_TYPE_NUM;
    MN_DUAL_SYS_E dual_sys = sig_ptr->dual_sys;
    uint8 sim_status = 0;
    uint8 pin_status = 0;

    switch(sig_ptr->cause)
    {
    case MN_SIM_TYPE_SUCCESS:
        switch(sig_ptr->card_type)
        {
        case MN_CARD_TYPE_SIM:
            card_type = MN_CARD_TYPE_SIM; //sim
            break;
        case MN_CARD_TYPE_USIM:
            card_type = MN_CARD_TYPE_USIM; //usim
            break;
        case MN_CARD_TYPE_DUAL:
            card_type = MN_CARD_TYPE_DUAL; //GSM/CDMA dual sim
            break;
        case MN_CARD_TYPE_UIM:
            card_type = MN_CARD_TYPE_UIM; //wcdma card
            break;
        default:
            break;
        }
        atc_config_ptr->sim_type[dual_sys] = card_type;
        break;
    case MN_SIM_TYPE_NOT_READY:
        break;
    case MN_SIM_TYPE_NO_SIM:
        break;
    case MN_SIM_TYPE_FAILED:
        break;
    default:
        break;
    }

    if(s_atc_euicc_cnt[dual_sys] > 0)
    {
        s_atc_euicc_cnt[dual_sys] -= 1;
        ATC_GetUiccInfo(dual_sys, atc_config_ptr, &sim_status, &pin_status, &card_type);
        sprintf((char *)g_rsp_str, "%s: %d,%d,%d",
                g_atc_info_table[AT_CMD_EUICC].cmd_name->str_ptr,
                sim_status, pin_status, card_type);
        ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
    }
#endif
}

/*****************************************************************************/
//  Description : This function handles the data send from the SIO when AT+SPENHA
//  Global resource dependence : g_rsp_str
//  Author:
//  Note:
/*****************************************************************************/
PUBLIC uint32 ATC_ProcessSioENHA_NVData(
    ATC_CONFIG_T    *atc_config_ptr,
    uint8           *data_ptr,  // pointer to the data
    uint16          param_len
)
{
    uint8             terminater_char;     // indicates ther terminater char
    BOOLEAN           flag = FALSE;
    uint32            dev_number = 0;
    uint32            mode_index = 0;

    AUDIO_RESULT_E   result;
    AUDIO_ENHA_EQ_STRUCT_T*  audenha_eq_ptr = PNULL;


    SCI_ASSERT(PNULL != atc_config_ptr); /*assert verified*/
    SCI_ASSERT(PNULL != data_ptr); /*assert verified*/

    terminater_char      = data_ptr[param_len - 1];
    data_ptr[param_len - 1] = '\0';
    param_len             -= 1;

    audenha_eq_ptr = (AUDIO_ENHA_EQ_STRUCT_T *)SCI_ALLOC_BASEZ(sizeof(AUDIO_ENHA_EQ_STRUCT_T)+1);
    SCI_ASSERT(NULL != audenha_eq_ptr); /*assert verified*/

    SCI_TRACE_LOW("ATC: siospenha for enha, param_len = %d",param_len);

    switch (terminater_char)
    {
    case ESC:
        flag = TRUE;
        break;

    case CTRL_Z:
        if(param_len == sizeof(AUDIO_ENHA_EQ_STRUCT_T)*2)
        {
            dev_number = AUDENHA_EXP_GetEqParaSetNum(FALSE);
            if(s_atc_admm_index <= dev_number)
            {
                ConvertHexToBin(data_ptr, param_len, (uint8*)audenha_eq_ptr);

                mode_index = s_atc_admm_index;

                result = AUDENHA_EXP_SetEqSetToFlash(mode_index, audenha_eq_ptr);

                if(result != AUDIO_NO_ERROR)
                {
                    SCI_TRACE_LOW("ATC: siospenha for enha, set result = %d",result);
                    flag = FALSE;
                }
                else
                {
                    flag = TRUE;
                }
            }
            else
            {
                SCI_TRACE_LOW("ATC: siospenha for enha, input data index error.");
            }
        }
        else
        {
            SCI_TRACE_LOW("ATC: siospenha for enha, input data length error.");
        }

        break;
    default:
           SCI_TRACE_LOW("Lost the escape character is %s is line %d",__FILE__,__LINE__);
        break;
    }

    SCI_FREE(audenha_eq_ptr);

    if(flag)
    {
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
    }
    else
    {
        ATC_BuildResultCodeRsp(atc_config_ptr,S_ATC_RESULT_CODE_ERROR ,ERR_UNKNOWN);
    }

    return S_ATC_DEFAULT_HANDLE;

}

/*****************************************************************************/
//  Description : This function handles the data send from the SIO when AT+SPENHA
//  Global resource dependence : g_rsp_str
//  Author:
//  Note:
/*****************************************************************************/
PUBLIC uint32 ATC_ProcessSioENHA_Tun_NVData(
                ATC_CONFIG_T    *atc_config_ptr,
                uint8           *data_ptr,  // pointer to the data
                uint16          param_len
                )
{
    uint8             terminater_char;     // indicates ther terminater char
    BOOLEAN           flag = FALSE;
    uint32            dev_number = 0;
    uint32            mode_index = 0;
    AUDIO_RESULT_E   result;

    AUDIO_ENHA_TUNABLE_EQ_STRUCT_T* audenha_tun_eq_ptr = PNULL;


    SCI_ASSERT(PNULL != atc_config_ptr); /*assert verified*/
    SCI_ASSERT(PNULL != data_ptr); /*assert verified*/

    terminater_char      = data_ptr[param_len - 1];
    data_ptr[param_len - 1] = '\0';
    param_len             -= 1;


    audenha_tun_eq_ptr = (AUDIO_ENHA_TUNABLE_EQ_STRUCT_T *)SCI_ALLOC_BASEZ(sizeof(AUDIO_ENHA_TUNABLE_EQ_STRUCT_T)+1);
    SCI_ASSERT(NULL != audenha_tun_eq_ptr); /*assert verified*/

    SCI_TRACE_LOW("ATC: siospenha for enha tun, param_len = %d",param_len);

    switch (terminater_char)
    {
    case ESC:
        flag = TRUE;
        break;

    case CTRL_Z:
        if(param_len == sizeof(AUDIO_ENHA_TUNABLE_EQ_STRUCT_T)*2)
        {
            dev_number = AUDENHA_EXP_GetEqParaSetNum(TRUE);
            if(s_atc_admm_index <= dev_number)
            {
                ConvertHexToBin(data_ptr, param_len, (uint8*)audenha_tun_eq_ptr);

                mode_index = s_atc_admm_index;

                result = AUDENHA_EXP_SetTunableEqSetToFlash(mode_index, audenha_tun_eq_ptr);

                if(result != AUDIO_NO_ERROR)
                {
                    SCI_TRACE_LOW("ATC: siospenha for enha tun, set result = %d",result);
                    flag = FALSE;
                }
                else
                {
                    flag = TRUE;
                }

            }
            else
            {
                SCI_TRACE_LOW("ATC: siospenha for enha tun, input data index error.");
            }
        }
        else
        {
            SCI_TRACE_LOW("ATC: siospenha for enha tun, input data length error.");
        }

        break;
    default:
           SCI_TRACE_LOW("Lost the escape character is %s is line %d",__FILE__,__LINE__);
        break;
    }

    SCI_FREE(audenha_tun_eq_ptr);

    if(flag)
    {
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
    }
    else
    {
        ATC_BuildResultCodeRsp(atc_config_ptr,S_ATC_RESULT_CODE_ERROR ,ERR_UNKNOWN);
    }

    return S_ATC_DEFAULT_HANDLE;
}

/*****************************************************************************/
//  Description : This function handles the data send from the SIO when AT+SPENHA
//  Global resource dependence : g_rsp_str
//  Author:
//  Note:
/*****************************************************************************/
PUBLIC uint32 ATC_ProcessSioENHA_Data(
                ATC_CONFIG_T    *atc_config_ptr,
                uint8           *data_ptr,  // pointer to the data
                uint16          param_len
                )
{
    uint8             terminater_char;     // indicates ther terminater char
    BOOLEAN           flag = FALSE;
    uint32            dev_number = 0;
    AUDIO_RESULT_E   result;
    AUDIO_ENHA_EQ_STRUCT_T*  audenha_eq_ptr = NULL;
    uint32            enha_mode_index = 0;
    uint32            mode_index = 0;


    SCI_ASSERT(PNULL != atc_config_ptr); /*assert verified*/
    SCI_ASSERT(PNULL != data_ptr); /*assert verified*/

    terminater_char      = data_ptr[param_len - 1];
    data_ptr[param_len - 1] = '\0';
    param_len             -= 1;


    audenha_eq_ptr = (AUDIO_ENHA_EQ_STRUCT_T *)SCI_ALLOC_BASEZ(sizeof(AUDIO_ENHA_EQ_STRUCT_T)+1);
    SCI_ASSERT(NULL != audenha_eq_ptr); /*assert verified*/

    SCI_TRACE_LOW("ATC: siospenha for enha, param_len = %d",param_len);

    switch (terminater_char)
    {
    case ESC:
        flag = TRUE;
        break;

    case CTRL_Z:
        if(param_len == sizeof(AUDIO_ENHA_EQ_STRUCT_T)*2)
        {
            dev_number = AUDENHA_EXP_GetEqParaSetNum(FALSE);
            if(s_atc_admm_index <= dev_number)
            {
                ConvertHexToBin(data_ptr, param_len, (uint8*)audenha_eq_ptr);

                mode_index = s_atc_admm_index;
                enha_mode_index = s_enha_eqmode_index;

                result = AUDENHA_EXP_SetEqSetToRam(mode_index, audenha_eq_ptr, enha_mode_index);

                if(result != AUDIO_NO_ERROR)
                {
                    SCI_TRACE_LOW("ATC: siospenha for enha, set result = %d",result);
                    flag = FALSE;
                }
                else
                {
                    flag = TRUE;
                }
            }
            else
            {
                SCI_TRACE_LOW("ATC: siospenha for enha, input data index error.");
            }
        }
        else
        {
            SCI_TRACE_LOW("ATC: siospenha for enha, input data length error.");
        }

        break;
    default:
           SCI_TRACE_LOW("Lost the escape character is %s is line %d",__FILE__,__LINE__);
        break;
    }

    SCI_FREE(audenha_eq_ptr);

    if (flag)
    {
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
    }
    else
    {
        ATC_BuildResultCodeRsp(atc_config_ptr,S_ATC_RESULT_CODE_ERROR ,ERR_UNKNOWN);
    }

    return S_ATC_DEFAULT_HANDLE;

}

/*****************************************************************************/
//  Description : This function handles the data send from the SIO when AT+SPENHA
//  Global resource dependence : g_rsp_str
//  Author:
//  Note:
/*****************************************************************************/
PUBLIC uint32 ATC_ProcessSioENHA_Tun_Data(
                ATC_CONFIG_T    *atc_config_ptr,
                uint8           *data_ptr,  // pointer to the data
                uint16          param_len
                )
{
    uint8             terminater_char;     // indicates ther terminater char
    BOOLEAN           flag = FALSE;
    uint32            dev_number = 0;
    uint32            mode_index = 0;
    uint32            enha_mode_index = 0;

    AUDIO_RESULT_E   result;

    AUDIO_ENHA_TUNABLE_EQ_STRUCT_T* audenha_tun_eq_ptr = PNULL;


    SCI_ASSERT(PNULL != atc_config_ptr); /*assert verified*/
    SCI_ASSERT(PNULL != data_ptr); /*assert verified*/

    terminater_char      = data_ptr[param_len - 1];
    data_ptr[param_len - 1] = '\0';
    param_len             -= 1;


    audenha_tun_eq_ptr = (AUDIO_ENHA_TUNABLE_EQ_STRUCT_T *)SCI_ALLOC_BASEZ(sizeof(AUDIO_ENHA_TUNABLE_EQ_STRUCT_T)+1);
    SCI_ASSERT(NULL != audenha_tun_eq_ptr); /*assert verified*/

    SCI_TRACE_LOW("ATC: siospenha for enha tun, param_len = %d",param_len);

    switch (terminater_char)
    {
    case ESC:
        flag = TRUE;
        break;

    case CTRL_Z:
        if(param_len == sizeof(AUDIO_ENHA_TUNABLE_EQ_STRUCT_T)*2)
        {
            dev_number = AUDENHA_EXP_GetEqParaSetNum(TRUE);
            if(s_atc_admm_index <= dev_number)
            {
                ConvertHexToBin(data_ptr, param_len, (uint8*)audenha_tun_eq_ptr);

                mode_index = s_atc_admm_index;
                enha_mode_index = s_enha_eqmode_index;

                result = AUDENHA_EXP_SetTunableEqSetToRam(mode_index, audenha_tun_eq_ptr, enha_mode_index);

                if(result != AUDIO_NO_ERROR)
                {
                    SCI_TRACE_LOW("ATC: siospenha for enha tun, set result = %d",result);
                    flag = FALSE;
                }
                else
                {
                    flag = TRUE;
                }

            }
            else
            {
                SCI_TRACE_LOW("ATC: siospenha for enha tun, input data index error.");
            }
        }
        else
        {
            SCI_TRACE_LOW("ATC: siospenha for enha tun, input data length error.");
        }

        break;
    default:
           SCI_TRACE_LOW("Lost the escape character is %s is line %d",__FILE__,__LINE__);
        break;
    }

    SCI_FREE(audenha_tun_eq_ptr);

    if(flag)
    {
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
    }
    else
    {
        ATC_BuildResultCodeRsp(atc_config_ptr,S_ATC_RESULT_CODE_ERROR ,ERR_UNKNOWN);
    }

    return S_ATC_DEFAULT_HANDLE;

}

/*****************************************************************************/
//  Description : This function handle the AT+SLOOPTEST command, to do loop back test
//  Global resource dependence : g_rsp_str
//  Author:       longting.zhao
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSLOOPTEST)
{
    uint8                     operation_mode = 0;
    ATC_STATUS                          status = S_ATC_SUCCESS;

     if(NULL == hVoiceRing)
    {
        hVoiceRing = AUDIO_CreateNormalHandle(hVoiceCodec, SCI_NULL, hLAYER1, SCI_NULL, AudioDummyNotifyCallback);
    }

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:
        {
            operation_mode = PARA1.num;

            if(1 == operation_mode)
            {
                // start loopback
                #ifdef _ATC_WAIT_L4
                AUD_EnableVoiceLoopback(TRUE, 100);
                #endif

                // set device mode
                AUDIO_SetDevMode(AUDIO_DEVICE_MODE_HANDHOLD);

                // set volume
                AUDIO_SetVolume(7);
            }
            else if(0 == operation_mode)
            {
            #ifdef _ATC_WAIT_L4
                AUD_EnableVoiceLoopback(FALSE, 100);
            #endif
            }
            else
            {
                status = ERR_OPERATION_NOT_SUPPORTED;
            }

            break;
        }

        default:
        {
            status = ERR_OPERATION_NOT_SUPPORTED;
            break;
        }
    }

    return status;
}

/*****************************************************************************/
//  Description : This function is used to get the parameters (AT+CRSM)
//
//  Global resource dependence : none
//  Author:       peng chen
//  Note:
/*****************************************************************************/
LOCAL BOOLEAN ATC_GetCRSMParams(
    uint8            *in_ptr,
    uint32             in_len,
    ATC_CPWD_PARAM_T *param_ptr,
    ATC_CRSM_PARAM_T  *param6_ptr
)
{
    uint32            i;
    uint32            j = 0;
    uint16           k = 0;
    BOOLEAN          flag = TRUE;

    for(i = 0; i < in_len; i++)
    {
        if(in_ptr[i] == ',')
        {
            k++;
            j = 0;

            if(k > 5)
            {
                flag = FALSE;
                break;
            }
        }
        else
        {
            if(5 == k)
            {
                if(j < ATC_CRSM_DATA_MAE_LEN)
                {
                    if(in_ptr[i] != '"')
                    {
                        param6_ptr[0].param[j++] = in_ptr[i];
                        param6_ptr[0].param_len += 1;
                    }
                }
                else if(in_ptr[i] != '"')
                {
                    flag = FALSE;
                    break;
                }
            }
            else
            {
                if(j < ATC_PWD_MAX_LEN)
                {
                    if(in_ptr[i] != '"')
                    {
                        param_ptr[k].param[j++] = in_ptr[i];
                        param_ptr[k].param_len += 1;
                    }
                }
                else if(in_ptr[i] != '"')
                {
                    flag = FALSE;
                    break;
                }
            }

        }
    }

    ATC_TRACE_LOW("ATC: get the CRSM parameters %d", flag);

    return flag;

}

/*****************************************************************************/
//  Description : This function handles the AT+CRSM command.This command is used
//  to set or get GPIO value.
//  Global resource dependence : g_rsp_str
//  Author:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCRSM)
{
#ifndef _ULTRA_LOW_CODE_
    uint32   status = S_ATC_DEFAULT_HANDLE;
    MNSIM_ERROR_E           error_code   = MNSIM_ERROR_POWER_OFF;
    MN_CRSM_COMMAND_TYPE_E  atc_sim_cmd  = MN_CRSM_COMMAND_TYPE_NUM;
    uint16  file_id = 0;
    uint8    P1 = 0;
    uint8    P2 = 0;
    uint8    P3 = 0;
    uint8  data[MN_SYS_NUMBER][255] = {0};

    MN_DUAL_SYS_E   dual_sys = ATC_CURRENT_SIM_ID_MUX;
    uint8    path_len = 0;
    uint8    pathid[MN_MAX_CRSM_PATHID_LEN] = {0};

    SCI_TRACE_LOW("ATC:ATC_ProcessCRSM");

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:

            if(PARAM1_FLAG)
            {
                atc_sim_cmd = (MN_CRSM_COMMAND_TYPE_E)PARA1.num;
            }

            switch(atc_sim_cmd)
            {
                case MN_CRSM_UPDATE_BINARY:
                case MN_CRSM_UPDATE_RECORD:

                    if(PARAM2_FLAG && PARAM3_FLAG && PARAM4_FLAG && PARAM5_FLAG && PARAM6_FLAG)
                    {
                        file_id = PARA2.num;
                        P1      = PARA3.num;
                        P2      = PARA4.num;
                        P3      = PARA5.num;

                        SCI_MEMSET(data[dual_sys], 0, sizeof(data[dual_sys]));
                        if(ATC_CMD_PARAM_TYPE_STRING == PARAM6_TYPE && PARA6.str_ptr->str_len > 0)
                        {
                            ConvertHexToBin(PARA6.str_ptr->str_ptr, PARA6.str_ptr->str_len, data[dual_sys]);
                        }
                        else
                        {
                            return ERR_OPERATION_NOT_ALLOWED;
                        }

                        if (PARAM7_FLAG)
                        {
                            if ((PARA7.str_ptr->str_len > 0) && (PARA7.str_ptr->str_len <= (MN_MAX_CRSM_PATHID_LEN*2)))
                            {
                                ConvertHexToBin(PARA7.str_ptr->str_ptr, PARA7.str_ptr->str_len, pathid);
                                path_len = PARA7.str_ptr->str_len/2;//ASCii to Hex bytes
                            }
                            else
                            {
                                return ERR_OPERATION_NOT_ALLOWED;
                            }
                        }
                    }
                    else
                    {
                        return ERR_INVALID_INDEX;
                    }

                    break;

                case MN_CRSM_READ_BINARY:
                case MN_CRSM_READ_RECORD:

                    if(PARAM2_FLAG && PARAM3_FLAG && PARAM4_FLAG && PARAM5_FLAG)
                    {
                        file_id = PARA2.num;
                        P1      = PARA3.num;
                        P2      = PARA4.num;
                        P3      = PARA5.num;

                        if (PARAM7_FLAG)
                        {
                            if ((PARA7.str_ptr->str_len > 0) && (PARA7.str_ptr->str_len <= (MN_MAX_CRSM_PATHID_LEN*2)))
                            {
                                ConvertHexToBin(PARA7.str_ptr->str_ptr, PARA7.str_ptr->str_len, pathid);
                                path_len = PARA7.str_ptr->str_len/2;//ASCii to Hex bytes
                            }
                            else
                            {
                                return ERR_OPERATION_NOT_ALLOWED;
                            }
                        }
                    }
                    else
                    {
                        return ERR_INVALID_INDEX;
                    }

                    break;

                case MN_CRSM_GET_RESPONSE:

                    if(PARAM2_FLAG)
                    {
                        file_id = PARA2.num;

                        if (PARAM7_FLAG)
                        {
                            if ((PARA7.str_ptr->str_len > 0) && (PARA7.str_ptr->str_len <= (MN_MAX_CRSM_PATHID_LEN*2)))
                            {
                                ConvertHexToBin(PARA7.str_ptr->str_ptr, PARA7.str_ptr->str_len, pathid);
                                path_len = PARA7.str_ptr->str_len/2;//ASCii to Hex bytes
                            }
                            else
                            {
                                return ERR_OPERATION_NOT_ALLOWED;
                            }
                        }
                    }
                    else
                    {
                        return ERR_INVALID_INDEX;
                    }

                    break;

                case MN_CRSM_GET_STATUS:
                    break;

                default:
                    return ERR_INVALID_INDEX;
            }

            MNSIM_GetSimStatusEx(dual_sys, &error_code);

            if((MNSIM_ERROR_REMOVED == error_code) || (MNSIM_ERROR_INSERTED_ERROR == error_code))
            {
                status = ERR_SIM_NOT_INSERTED;
            }
            else
            {
                if(ERR_MNDATAMAG_NO_ERR == MNSIM_CrsmEx(dual_sys, atc_sim_cmd, P1, P2, P3, file_id, data[dual_sys], path_len, pathid))
                {
                    ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr, (uint16)APP_MN_CRSM_CNF, 0xff, ATC_NO_DOMAIN, dual_sys);
                    status = S_ATC_DEFAULT_HANDLE;
                }
                else
                {
                    status = S_ATC_FAIL;
                }
            }

            break;

        case ATC_CMD_TYPE_READ:
        default:
            return ERR_OPERATION_NOT_ALLOWED;
    }

    return status;
#else
    return S_ATC_FAIL;
#endif/*_ULTRA_LOW_CODE_*/
}

/*****************************************************************************/
//  Description : This function handles the APP_MN_CRSM_CNF signal in
//  deleting only one item.
//  Global resource dependence :
//  Author:       WangHui
//  Note:
/*****************************************************************************/
ATC_STATUS ATC_ProcessCrsmCnf(
    ATC_CONFIG_T             *atc_config_ptr,
    APP_MN_CRSM_CNF_T    *sig_ptr
)
{
#ifndef _ULTRA_LOW_CODE_
    uint8 data_response[MAX_SIM_DATALEN+1] = "\0";

    ATC_TRACE_LOW("ATC:ATC_ProcessCrsmCnf");


    if(sig_ptr->rsp_data.response == TRUE)
    {
        ATC_TRACE_LOW("ATC:ATC_ProcessCrsmCnf,true");

        if(sig_ptr->rsp_data.length != 0)
        {
            ATC_TranUint8IntoChar(sig_ptr->rsp_data.data, sig_ptr->rsp_data.length, data_response);
            sprintf((char *)g_rsp_str, "+CRSM: %d,%d,%s", sig_ptr->rsp_data.sw1, sig_ptr->rsp_data.sw2, data_response);
        }
        else
        {
            sprintf((char *)g_rsp_str, "+CRSM: %d,%d", sig_ptr->rsp_data.sw1, sig_ptr->rsp_data.sw2);
        }

        ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
    }
    else
    {
        ATC_TRACE_LOW("ATC:ATC_ProcessCrsmCnf,FAIL");
        sprintf((char *)g_rsp_str, "+CRSM: %d,%d", sig_ptr->rsp_data.sw1, sig_ptr->rsp_data.sw2);
        ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
    }

#endif /* _ULTRA_LOW_CODE_ */

    return S_ATC_DEFAULT_HANDLE;
}

/*****************************************************************************/
//  Description : This function handle the AT+SPSETSAVESMS command
//  Global resource dependence : none
//  Author:   bin.li
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSPSETSAVESMS)
{
    ATC_STATUS     status = S_ATC_SUCCESS;
//#ifdef _ATC_WAIT_
    MN_DUAL_SYS_E   dual_sys = ATC_CURRENT_SIM_ID_MUX;

    SCI_TRACE_LOW("ATC: ATC_ProcessSPSETSAVESMS.");
    switch (ATC_GET_CMD_TYPE)
    {
    case ATC_CMD_TYPE_SET:
        if (!PARAM1_FLAG)
        {
            return ERR_INVALID_INDEX;
        }
        MNSMS_SetSaveSMSEx(dual_sys,PARA1.num);
        g_atc_ps_not_save_sms[dual_sys] = (PARA1.num) ?0:1;
        SCI_TRACE_LOW("ATC: current card is %d,ps_not_save_sms is %d.",dual_sys,g_atc_ps_not_save_sms[dual_sys]);

        break;
    case ATC_CMD_TYPE_READ:
    case ATC_CMD_TYPE_TEST:
    case ATC_CMD_TYPE_EXECUTE:
    default:
        status = ERR_OPERATION_NOT_SUPPORTED;
        break;
    }

//#endif
    return status;
}

#ifdef ATC_SAT_ENABLE
/*****************************************************************************/
//  Description : This function handle the AT+SIME command
//  Global resource dependence : g_rsp_str
//  Author:       Feng.shi
//  Note:
/*****************************************************************************/
AT_CMD_FUNC( ATC_ProcessSIME )
{
#ifndef _ULTRA_LOW_CODE_
    MN_DUAL_SYS_E dual_sys =  ATC_CURRENT_SIM_ID_MUX;

    if(ATC_CMD_TYPE_EXECUTE == ATC_GET_CMD_TYPE)
    {
        sprintf((char*)g_rsp_str, "%s: %d",
                g_atc_info_table[AT_CMD_SIME].cmd_name->str_ptr, atc_config_ptr->sim_flag[dual_sys]);
        ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
        return S_ATC_SUCCESS;
    }
    else
        return S_ATC_FAIL;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the AT+SPLO command£¬Preferred operator
//  list
//  Global resource dependence : g_rsp_str
//  Author:  Shijun.cui
//  Note:
/*****************************************************************************/
AT_CMD_FUNC( ATC_ProcessSPLO )
{
#ifndef _ULTRA_LOW_CODE_
    ATC_STATUS        status = S_ATC_SUCCESS;
    uint8             index = 0;
    MN_PHONE_FPLMN_LIST_T    fplmn_list;
    MN_PLMN_T         plmn;
    uint8*               ptr;
    uint32                  i = 0;

    MN_DUAL_SYS_E dual_sys =  ATC_CURRENT_SIM_ID_MUX;

    SCI_MEMSET(&plmn, 0, sizeof(plmn));
    SCI_MEMSET(&fplmn_list, 0, sizeof(fplmn_list));

    if( MNPHONE_GetFplmnListEx(dual_sys,&fplmn_list) != ERR_MNPHONE_NO_ERR)
    {
        SCI_TRACE_LOW("ATC: SPLO get fplmn fail!\n");

        return ERR_UNKNOWN;
    }

    SCI_TRACE_LOW("ATC: SPLO get fplmn success!\n");

    switch (ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:
        {
            if( PARAM1_FLAG )
            {
                index = (uint8)PARA1.num;
            }
            else
            {
                index = 0;
            }

            if(MN_PHONE_FPLMN_LIST_NUM < index)//<index> is not the right value, show error
            {
                return ERR_OPERATION_NOT_SUPPORTED;
            }

            SCI_TRACE_LOW("ATC: SPLO index = %d",index);

            if(PARAM3_FLAG)
            {

                if(ATC_CMD_PARAM_TYPE_NUMERIC == PARAM3_TYPE)
                {
                    SCI_TRACE_LOW("ATC: SPLO plmn = %d",PARA3.num);

                    plmn.mnc_digit_num = (uint16)(((uint16)PARA3.num) % 10);
                    plmn.mnc = (uint16)(((uint16)(PARA3.num / 10)) % 1000);
                    plmn.mcc = (uint16)(PARA3.num / 10000);

                    if((plmn.mnc_digit_num != 0x02)&&(plmn.mnc_digit_num != 0x03))
                    {
                        SCI_TRACE_LOW("+CMS ERROR: mnc_digit_num can only be 2 or 3\n");
                        return ERR_OPERATION_NOT_SUPPORTED;
                    }
                }
                else//(ATC_CMD_PARAM_TYPE_STRING == PARAM3_TYPE)
                {
                    SCI_TRACE_LOW("+CMS ERROR: cannot input string in para3\n");
                    return ERR_OPERATION_NOT_SUPPORTED;
                }

                if(0 == index)//If<oper> is given but <index> is left out, <oper> is put in the next free location.
                {
                    if(fplmn_list.plmn_num < MN_PHONE_FPLMN_LIST_NUM)
                    {
                        fplmn_list.plmn_arr[fplmn_list.plmn_num].mcc = plmn.mcc;
                        fplmn_list.plmn_arr[fplmn_list.plmn_num].mnc = plmn.mnc;
                        fplmn_list.plmn_arr[fplmn_list.plmn_num].mnc_digit_num = plmn.mnc_digit_num;
                        fplmn_list.plmn_num ++;

                    }
                    else if(MN_PHONE_FPLMN_LIST_NUM == fplmn_list.plmn_num)
                    {
                        for(i=0;i<MN_PHONE_FPLMN_LIST_NUM - 1;i++)
                        {
                            fplmn_list.plmn_arr[i].mcc = fplmn_list.plmn_arr[i + 1].mcc;
                            fplmn_list.plmn_arr[i].mnc = fplmn_list.plmn_arr[i + 1].mnc;
                            fplmn_list.plmn_arr[i].mnc_digit_num = fplmn_list.plmn_arr[i + 1].mnc_digit_num;
                        }

                        fplmn_list.plmn_arr[MN_PHONE_FPLMN_LIST_NUM - 1].mcc = plmn.mcc;
                        fplmn_list.plmn_arr[MN_PHONE_FPLMN_LIST_NUM - 1].mnc = plmn.mnc;
                        fplmn_list.plmn_arr[MN_PHONE_FPLMN_LIST_NUM - 1].mnc_digit_num = plmn.mnc_digit_num;
                    }

                    if( MNPHONE_SetFplmnListEx(dual_sys,&fplmn_list) != ERR_MNPHONE_NO_ERR)
                    {
                        SCI_TRACE_LOW("+CMS ERROR: set FPLMN list fail\n");
                        return ERR_UNKNOWN;
                    }

                }
                else if((index >= 1)&&(index <= fplmn_list.plmn_num))//update the <oper> in the <index> position
                {
                    fplmn_list.plmn_arr[index - 1].mcc = plmn.mcc;
                    fplmn_list.plmn_arr[index - 1].mnc = plmn.mnc;
                    fplmn_list.plmn_arr[index - 1].mnc_digit_num = plmn.mnc_digit_num;

                    if( MNPHONE_SetFplmnListEx(dual_sys,&fplmn_list) != ERR_MNPHONE_NO_ERR)
                    {
                        SCI_TRACE_LOW("+CMS ERROR: set FPLMN list fail\n");
                        return ERR_UNKNOWN;
                    }
                }
                else
                {
                    return ERR_UNKNOWN;
                }
            }
            else
            {
                if((index >= 1)&&(index <= fplmn_list.plmn_num))//If <index> is given but <oper> is left out, entry is deleted
                {

                    fplmn_list.plmn_arr[index - 1].mcc = (uint16)0xFFFF;
                    fplmn_list.plmn_arr[index - 1].mnc = (uint16)0xFFFF;
                    fplmn_list.plmn_num--;

                    for(i=index - 1;i<MN_PHONE_FPLMN_LIST_NUM - 1;i++)
                    {

                        fplmn_list.plmn_arr[i].mcc = fplmn_list.plmn_arr[i + 1].mcc;
                        fplmn_list.plmn_arr[i].mnc = fplmn_list.plmn_arr[i + 1].mnc;
                        fplmn_list.plmn_arr[i].mnc_digit_num = fplmn_list.plmn_arr[i + 1].mnc_digit_num;
                    }

                    if( MNPHONE_SetFplmnListEx(dual_sys,&fplmn_list) != ERR_MNPHONE_NO_ERR)
                    {
                        SCI_TRACE_LOW("+CMS ERROR: set FPLMN list fail\n");
                        return ERR_UNKNOWN;
                    }

                }
                else//error <index> and error <oper>, show error
                {
                    return ERR_UNKNOWN;
                }
            }
            break;
        }

        case ATC_CMD_TYPE_READ:
        {
            // list the current fplmn list
            ptr = g_rsp_str;

            for(i=0; i< fplmn_list.plmn_num; i++)
            {
                    sprintf((char*)ptr, "%s: index %d, mcc is %d, mnc is %d, mnc_digit_num is %d\r\n",g_atc_info_table[AT_CMD_SPLO].cmd_name->str_ptr,i+1,fplmn_list.plmn_arr[i].mcc,fplmn_list.plmn_arr[i].mnc,fplmn_list.plmn_arr[i].mnc_digit_num);
                    ptr = g_rsp_str + strlen((char*)g_rsp_str);
            }

            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;
        }

        default:
        {
            SCI_TRACE_LOW("ATC: Assert SPLO,invalid type.");
            break;
        }
    }

    return status;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
// Description : This function handle the AT+POSI command
//                 Automatic downloader
// Global resource dependence : none
// Author:winnie.zhu
// Note:
/*****************************************************************************/
AT_CMD_FUNC( ATC_ProcessPOSI )
{
    MN_PHONE_CURRENT_PLMN_INFO_T   plmn_info;
    MN_PHONE_SCELL_MEAS_T          rssi;
    APP_MN_NCELL_RSSI_CNF_T        ncell = {0};
    MN_PHONE_CELL_MEAS_T    cell_power[MN_MAX_CELL_NUM * 2] = {0}; //include scell
    uint8           cell_id[MN_MAX_CELL_NUM * 2]={0};
    MN_DUAL_SYS_E   dual_sys = ATC_CURRENT_SIM_ID_MUX;

    MN_PHONE_CELL_MEAS_T    scell = {0};


    uint16 mode = 0;
    uint8 end = 0;
    uint8 rand_str[40] = {0};
    uint8 ndssign[30] = {0};
    //uint8 rand_nds_data[16] = {0};
    //uint8 ndssign_str[50]      = {0};
 #ifdef _NDS_EXT_CMD_
    int ndssign_len = 30;
#endif
    //uint16 rand_nds_len = 0;
    uint8 data_str[255] = {0};
    int32 i,j;
    uint8 *temp_str = PNULL;
    uint8 cell_num = 0;
    uint8 temp1 = 0;
    //int    sign_errno = 0;

    switch (ATC_GET_CMD_TYPE)
    {
    case ATC_CMD_TYPE_SET:
        if (!PARAM1_FLAG || !PARAM2_FLAG ||(32 !=PARA2.str_ptr->str_len))
        {
            return ERR_INVALID_INDEX;
        }
        mode  = (uint16)(PARA1.num);
        SCI_MEMCPY(rand_str,PARA2.str_ptr->str_ptr,PARA2.str_ptr->str_len);
        rand_str[PARA2.str_ptr->str_len] = '\0';

        switch (mode)
        {
            case 0:
            {
                end = 1;
                SCI_MEMSET(&plmn_info, 0, sizeof(MN_PHONE_CURRENT_PLMN_INFO_T));
                SCI_MEMSET(&rssi, 0, sizeof(MN_PHONE_SCELL_MEAS_T));
                plmn_info = MNPHONE_GetCurrentPLMNInfoEx(dual_sys);
                rssi = MNPHONE_GetRSSIEx(dual_sys);
                // check the status of the phone
                if (PLMN_NO_SERVICE == plmn_info.plmn_status)
                {
                    sprintf((char*)data_str, "%d,%d", mode,end);
                #ifdef _NDS_EXT_CMD_
                    if(!sarft_sign_data(data_str,strlen((char *)data_str),rand_str,strlen((char *)rand_str),
                                        s_cid,s_gmid,s_seed,ndssign,&ndssign_len))
                    {
                        SCI_TRACE_LOW("ATC: sarft sign data fail.");
                        return ERR_UNKNOWN;
                    }
                #endif
                    sprintf((char*)g_rsp_str, "%s: %d,%d,\"%s\",\"%s\"",
                    g_atc_info_table[AT_CMD_POSI].cmd_name->str_ptr,mode,end,rand_str,ndssign);

                    ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
                }
                else
                {
                    MNPHONE_GetMeasReportEx(dual_sys, &ncell, &scell);

                    if(scell.cell_present)
                    {
                        sprintf((char*)data_str, "%d,%d,%02d,%d,%d,%d,%d,%d",mode,plmn_info.mcc,plmn_info.mnc,
                        plmn_info.lac,plmn_info.cell_id,rssi.bsic,((rssi.rxlev+1) / 2),end);
                    #ifdef _NDS_EXT_CMD_
                        if(!sarft_sign_data(data_str,strlen((char *)data_str),rand_str,strlen((char *)rand_str),
                                            s_cid,s_gmid,s_seed,ndssign,&ndssign_len))
                        {
                            SCI_TRACE_LOW("ATC: sarft sign data fail.");
                            return ERR_UNKNOWN;
                        }
                    #endif
                        sprintf((char*)g_rsp_str, "%s: %d,%d,%02d,%d,%d,%d,%d,%d,\"%s\",\"%s\"",
                            g_atc_info_table[AT_CMD_POSI].cmd_name->str_ptr,mode,plmn_info.mcc,plmn_info.mnc,
                            plmn_info.lac,plmn_info.cell_id,rssi.bsic,((rssi.rxlev+1) / 2),end,rand_str,ndssign);
                        ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
                    }
                    else //sometimes MN is normal but RR is doing power sweep, so return no service at this time
                    {
                        sprintf((char*)data_str, "%d,%d", mode,end);
                    #ifdef _NDS_EXT_CMD_
                        if(!sarft_sign_data(data_str,strlen((char *)data_str),rand_str,strlen((char *)rand_str),
                                        s_cid,s_gmid,s_seed,ndssign,&ndssign_len))
                        {
                            SCI_TRACE_LOW("ATC: sarft sign data fail.");
                            return ERR_UNKNOWN;
                        }
                    #endif
                        sprintf((char*)g_rsp_str, "%s: %d,%d,\"%s\",\"%s\"",
                           g_atc_info_table[AT_CMD_POSI].cmd_name->str_ptr,mode,end,rand_str,ndssign);
                        ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
                    }
                }
                break;
            }

            case 1:
            {
                end = 0;
                cell_num = 0;
                sprintf((char*)g_rsp_str, "%s:%d,", g_atc_info_table[AT_CMD_POSI].cmd_name->str_ptr, mode);
                temp_str = &g_rsp_str[strlen((char*)g_rsp_str)];

                for (dual_sys = MN_DUAL_SYS_1; dual_sys <= MN_DUAL_SYS_2; dual_sys++)
                {
                    if (DMSIM_SimIsReady(MN_DUAL_SYS_1) && (dual_sys == MN_DUAL_SYS_2))
                    {
                        break;
                    }
                    SCI_MEMSET(&plmn_info, 0, sizeof(MN_PHONE_CURRENT_PLMN_INFO_T));
                    SCI_MEMSET(&rssi, 0, sizeof(MN_PHONE_SCELL_MEAS_T));
                    SCI_MEMSET(&ncell, 0, sizeof(APP_MN_NCELL_RSSI_CNF_T));

                    plmn_info = MNPHONE_GetCurrentPLMNInfoEx(dual_sys);
                    rssi = MNPHONE_GetRSSIEx(dual_sys);

                    MNPHONE_GetMeasReportEx(dual_sys, &ncell, &scell);

                    for (i = 0; i < MN_MAX_NCELL_NUM; i++)
                    {
                        //remove the lock of mobile operator
                        if (ncell.ncell_power[i].cell_present && ncell.ncell_power[i].identity_present)
                        {
                            SCI_MEMCPY(&cell_power[cell_num], &ncell.ncell_power[i], sizeof(MN_PHONE_CELL_MEAS_T));
                            cell_id[cell_num] = cell_num;
                            SCI_TRACE_LOW("ATC: sim_id=%d, ncell_index=%d, arfcn=%d, bsic=%d, rxlev=%d, mcc=%d, mnc=%d, lac=%d, mnc_digit_num=%d, cell_id=%d",
                                               dual_sys, cell_num, cell_power[cell_num].arfcn, cell_power[cell_num].bsic,
                                               cell_power[cell_num].rxlev, cell_power[cell_num].lai.mcc,
                                               cell_power[cell_num].lai.mnc, cell_power[cell_num].lai.lac,
                                               cell_power[cell_num].lai.mnc_digit_num, cell_power[cell_num].cell_id);
                            cell_num++;
                        }
                    }

                    //add scell
                    if(PLMN_NO_SERVICE != plmn_info.plmn_status)
                    {
                        BOOLEAN scell_present = FALSE;
                        for(i = 0; i < cell_num; i++)
                        {
                            //check ncell including scell or not
                            if(cell_power[i].cell_id == plmn_info.cell_id)
                            {
                                scell_present = TRUE;
                                break;
                            }
                        }
                        if(!scell_present)
                        {
                            cell_power[cell_num].lai.mcc = plmn_info.mcc;
                            cell_power[cell_num].lai.mnc = plmn_info.mnc;
                            cell_power[cell_num].lai.lac = plmn_info.lac;
                            cell_power[cell_num].cell_id = plmn_info.cell_id;
                            cell_power[cell_num].lai.mnc_digit_num= plmn_info.mnc_digit_num;
                            cell_power[cell_num].bsic    = rssi.bsic;
                            cell_power[cell_num].rxlev   = rssi.rxlev;
                            SCI_TRACE_LOW("ATC: sim_id=%d, scell_index=%d, arfcn=%d, bsic=%d, rxlev=%d, mcc=%d, mnc=%d, lac=%d, mnc_digit_num=%d, cell_id=%d",
                                               dual_sys, cell_num, cell_power[cell_num].arfcn, cell_power[cell_num].bsic,
                                               cell_power[cell_num].rxlev, cell_power[cell_num].lai.mcc,
                                               cell_power[cell_num].lai.mnc, cell_power[cell_num].lai.lac,
                                               cell_power[cell_num].lai.mnc_digit_num, cell_power[cell_num].cell_id);

                            cell_id[cell_num] = cell_num;
                            cell_num++;
                        }

                        SCI_TRACE_LOW("ATC_ProcessPOSI sim %d, ncell_has_scell %d", dual_sys, scell_present);
                    }
                }


                //return OK without any cell
                if(cell_num == 0)
                {
                    SCI_TRACE_LOW("ATC_ProcessPOSI ncell_num = 0");
                    return S_ATC_SUCCESS;
                }

                for(i = 1; i < cell_num; i++)
                {
                    temp1 = cell_id[i];

                    for(j = i - 1; j >= 0 && cell_power[cell_id[j]].rxlev < cell_power[temp1].rxlev; j--)
                    {
                        cell_id[j + 1] = cell_id[j];
                    }
                    cell_id[j + 1] = temp1;
                }


                for(j = 0; j < cell_num; j++ )
                {
                    i = cell_id[j];
                    SCI_TRACE_LOW("ATC: cell_num=%d, cell_power[%d].rxlev=%d ",cell_num, i, cell_power[i].rxlev);
                    if(cell_power[i].lai.mcc == 0 || cell_power[i].cell_id == 0 || cell_power[i].rxlev == 0)
                    {
                        SCI_TRACE_LOW("ATC_ProcessPOSI invalid cell mnc %d,cell_id %x, rxlev %d",
                                cell_power[i].lai.mcc,
                                cell_power[i].cell_id, cell_power[i].rxlev);
                        continue;
                    }

                    sprintf((char*)temp_str,"%d,%02d,%x,%x,%x,%d,%x", cell_power[i].lai.mcc,
                        cell_power[i].lai.mnc, cell_power[i].lai.lac,
                        cell_power[i].cell_id, cell_power[i].bsic,
                        cell_power[i].rxlev, end);
                    SCI_TRACE_LOW("ATC POSI: %s", temp_str);
                    temp_str = &g_rsp_str[strlen((char*)g_rsp_str)];
                    *temp_str = ',';
                    temp_str++;
                    *temp_str = '\0';
                }
                sprintf((char *)data_str, "%d,%s,1", mode, temp_str);

                g_rsp_str[strlen((char*)g_rsp_str) - 2] = '1'; //end
                g_rsp_str[strlen((char*)g_rsp_str) - 1] = ',';
                sprintf((char *)&g_rsp_str[strlen((char*)g_rsp_str)],"\"%s\"", rand_str);
         #if 0
                rand_nds_len = ConvertStrToNDSData(rand_str, strlen(rand_str), rand_nds_data);

                sign_errno = sarft_sign_data(g_rsp_str, strlen(g_rsp_str), rand_nds_data, rand_nds_len,
                          g_cid_nds_data, g_mid_nds_data, g_seed_nds_data, ndssign, &ndssign_len);
                if(0 != sign_errno)
                {
                    SCI_TRACE_LOW("ATC: sarft sign data fail.");

                    SCI_TRACE_LOW("ATC_ProcessPOSI sign str %s", sarft_error_str(sarft_sign_errno));
                    return ERR_UNKNOWN;
                }

                ConvertNDSDataToStr(ndssign, ndssign_len, ndssign_str);
                sprintf((char *)&g_rsp_str[strlen((char*)g_rsp_str)],",\"%s\"", ndssign_str);
         #else
             #ifdef _NDS_EXT_CMD_
                if(!sarft_sign_data(data_str,strlen((char *)data_str),rand_str,strlen((char *)rand_str),
                    s_cid,s_gmid,s_seed,ndssign,&ndssign_len))
                {
                    SCI_TRACE_LOW("ATC: sarft sign data fail.");
                    return ERR_UNKNOWN;
                }
            #endif

                sprintf((char *)&g_rsp_str[strlen((char*)g_rsp_str)],"\"%s\"", ndssign);

         #endif
                g_rsp_str[strlen((char*)g_rsp_str)] = '\0';
                ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
                SCI_TRACE_LOW("ATC_ProcessPOSI: %s",g_rsp_str);
                SCI_TRACE_LOW("ATC: the string length is %d,cell_num=%d", strlen((char*)g_rsp_str), cell_num);
                break;
            }

            default:
                break;
        }
        break;

    default:
        break;
    }
    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handles the AT+SCPFRE command.This command is used
//  to set or get frequency value.
//  Global resource dependence : g_rsp_str
//  Author:       king.li
/*****************************************************************************/
AT_CMD_FUNC( ATC_ProcessSCPFRE )
{
    ATC_STATUS    status = S_ATC_SUCCESS;
    uint8        band;
    uint16        frequency, freq; // 0 - set gpio value, 1 - get gpio value
    BOOLEAN   ismode = FALSE;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:
        {
            // check the type
            if (PARAM1_FLAG && PARAM2_FLAG)
            {
                //PARA1.num define the band
                band = (uint8)PARA1.num;
                //PARA2.num define the frequency
                frequency = (uint16)PARA2.num;
            }
            else
            {
                status = ERR_OPERATION_NOT_ALLOWED;
                break;
            }
            /*in CALIBRATION_POST_MODE, should set the frequency parameter to global variable
               msg_head->seq_num higher 2 bytes carry the frequency information; big endian
               frequency information includes frequency(11bit) and band(4bit);
               bit    0   1   2   3   4   5   6   7    8   9   10  11  12  13  14  15
                       |                                |                                         |
                               seq_num[0]                       seq_num[1]
                       |                                     |   |    |    |    |    |    |   |
                       |                                       frequency(low 8 bit)
                       |   |   |        |   |   |   |
             frequency(high 3 bit)      band(4 bits)
                */
            freq = (frequency&0xff)|((frequency&0x700)<<5)|((band&0xf)<<8);
            POWER_SetCalibrationPostModeFreq(freq);

            ATC_BuildResultCodeRsp(atc_config_ptr,S_ATC_RESULT_CODE_OK,0);
            sprintf((char*)g_rsp_str,"%s: 0x%x", g_atc_info_table[AT_CMD_SCPFRE].cmd_name->str_ptr,freq);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;
        }

        case ATC_CMD_TYPE_READ:
        {
#ifndef WIN32
            ismode = RM_GetCalibrationPostMode();
#endif
            if(ismode)
            {
                  freq = POWER_GetCalibrationPostModeFreq();
                 frequency = (freq&0xff)|((freq&0xe000)>>5);
                 band = (freq&0xf00)>>8;
                  sprintf((char*)g_rsp_str,"%s: %d,%d",g_atc_info_table[AT_CMD_SCPFRE].cmd_name->str_ptr,
                     frequency, band);
                 ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            }
            else
            {
                status = ERR_OPERATION_NOT_ALLOWED;
                break;
            }
             break;
        }

        default:
        {
            break;
        }
    }
    return  status;
}

/*****************************************************************************/
//  Description : This function handles the AT+SCINF command.This command is used
//                      to get some information about the serving and neighbour cells .
//  Global resource dependence : g_rsp_str
//  Author:       elvis.xu
/*****************************************************************************/
AT_CMD_FUNC( ATC_ProcessSCINF )
{
    ATC_STATUS    status = S_ATC_SUCCESS;
#if 0
    MN_DUAL_SYS_E dual_sys =  ATC_CURRENT_SIM_ID_MUX;
    RR_CELLS_INFO_T *rr_cells_info_bsic_ptr = PNULL;
    RR_CELLS_INFO_TRAFFIC_T *rr_cells_info_bsic_traffic_ptr = PNULL;
    RR_CELLS_INFO_EX_T *rr_cells_info_ex_ptr = PNULL;
    BOOLEAN is_traffic = FALSE;
    BOOLEAN is_present = FALSE;
    uint8 * rsp_buf_ptr = PNULL;
    int16 i, ncell_num = 0;

    RR_CELL_INFO_TRAFFIC_T * t_sc_ptr = PNULL;
    RR_CELL_INFO_T  * t_ncs_ptr = PNULL;

    RR_CELL_INFO_T * i_sc_ptr = PNULL;
    RR_CELL_INFO_T * i_ncs_ptr = PNULL;

    RR_CELL_INFO_EX_T * i_sc_e_ptr = PNULL;
    RR_CELL_INFO_EX_T * i_ncs_e_ptr = PNULL;

    SCI_TRACE_LOW("ATC:ATC_ProcessSCINF");

    switch(ATC_GET_CMD_TYPE)
    {
        //case ATC_CMD_TYPE_READ:
        case ATC_CMD_TYPE_EXECUTE:
        {
            rr_cells_info_bsic_ptr = (RR_CELLS_INFO_T *)SCI_ALLOC_BASEZ(sizeof(RR_CELLS_INFO_T));

            rr_cells_info_bsic_traffic_ptr = (RR_CELLS_INFO_TRAFFIC_T *)SCI_ALLOC_BASEZ(sizeof(RR_CELLS_INFO_TRAFFIC_T));

            rr_cells_info_ex_ptr = (RR_CELLS_INFO_EX_T *)SCI_ALLOC_BASEZ(sizeof(RR_CELLS_INFO_EX_T));

            //to get cell parameters and information from RR
            PS_GetRRCellsInfoBsicAndEx(dual_sys,
                                                              rr_cells_info_bsic_ptr,
                                                              rr_cells_info_bsic_traffic_ptr,
                                                              rr_cells_info_ex_ptr,
                                                              &is_traffic);

            //to check cell params and info
            rsp_buf_ptr = &g_rsp_str[0];
            if(is_traffic)
            {
                is_present = rr_cells_info_bsic_traffic_ptr->scell.present;
                if(is_present)
                {
                    sprintf((char *)rsp_buf_ptr,"+SCINF: %d,", ATC_TRAFFIC);
                    rsp_buf_ptr += strlen((char *)rsp_buf_ptr);

                    //to send serving cell params
                    t_sc_ptr = &(rr_cells_info_bsic_traffic_ptr->scell);
                    sprintf((char *)rsp_buf_ptr, "%d,%d,%d,",
                                  t_sc_ptr->arfcn, t_sc_ptr->bsic, t_sc_ptr->rxlev_full);
                    rsp_buf_ptr += strlen((char *)rsp_buf_ptr);

                    sprintf((char *)rsp_buf_ptr, "%d,%d,%d;",
                                  t_sc_ptr->rxlev_sub, t_sc_ptr->rxqual_full,t_sc_ptr->rxqual_sub);
                    rsp_buf_ptr += strlen((char *)rsp_buf_ptr);

                    //to count the number of neighbour cells
                    t_ncs_ptr = rr_cells_info_bsic_traffic_ptr->ncells;
                    for (i = 0; i < RR_CELL_INFO_SIZE; i++)
                    {
                        if(FALSE == t_ncs_ptr[i].present)
                        {
                            continue;
                        }
                        ncell_num++;
                    }

                    //to send neighbour cells' info
                    for (i = 0; i < RR_CELL_INFO_SIZE; i++)
                    {
                        if(FALSE == t_ncs_ptr[i].present)
                        {
                            continue;
                        }
                        sprintf((char *)rsp_buf_ptr, "%d,%d,%d,%d;",
                                      ncell_num--, t_ncs_ptr[i].arfcn, t_ncs_ptr[i].bsic, t_ncs_ptr[i].rxlev);
                        rsp_buf_ptr += strlen((char *)rsp_buf_ptr);
                    }

                    ATC_BuildInfoRsp(atc_config_ptr, (uint8*)g_rsp_str);
                    status = S_ATC_SUCCESS;
                }
                else
                {
                    SCI_TRACE_LOW("ATC: no network");
                    status = ERR_NO_NETWORK_SERVICE;
                }
            }
            else
            {
                is_present = rr_cells_info_bsic_ptr->scell.present;
                if(is_present)
                {
                    sprintf((char *)rsp_buf_ptr,"+SCINF: %d,", ATC_IDLE);
                    rsp_buf_ptr += strlen((char *)rsp_buf_ptr);

                    //to send serving cell params
                    i_sc_ptr = &(rr_cells_info_bsic_ptr->scell);
                    sprintf((char *)rsp_buf_ptr, "%d,%d,%d,",
                                 i_sc_ptr->arfcn, i_sc_ptr->bsic, i_sc_ptr->rxlev);
                    rsp_buf_ptr += strlen((char *)rsp_buf_ptr);

                    sprintf((char *)rsp_buf_ptr, "%d,%d,",
                                 i_sc_ptr->C1, i_sc_ptr->C2);
                    rsp_buf_ptr += strlen((char *)rsp_buf_ptr);

                    i_sc_e_ptr = &(rr_cells_info_ex_ptr->scell);
                    sprintf((char *)rsp_buf_ptr, "%d,%d,%d,",
                                  i_sc_e_ptr->ms_txpwr_max_ccch, i_sc_e_ptr->rxlev_access_min, i_sc_e_ptr->CBA);
                    rsp_buf_ptr += strlen((char *)rsp_buf_ptr);

                    sprintf((char *)rsp_buf_ptr, "%d,%d,%d,",
                                  i_sc_e_ptr->CBQ, i_sc_e_ptr->CRO, i_sc_e_ptr->TempOffset);
                    rsp_buf_ptr += strlen((char *)rsp_buf_ptr);

                    sprintf((char *)rsp_buf_ptr, "%d,%d;",
                                  i_sc_e_ptr->PT, i_sc_e_ptr->CRH);
                    rsp_buf_ptr += strlen((char *)rsp_buf_ptr);

                    //to count the number of neighbour cells
                    i_ncs_ptr = &(rr_cells_info_bsic_ptr->ncells[0]);
                    i_ncs_e_ptr = &(rr_cells_info_ex_ptr->ncells[0]);

                    for (i = 0; i < RR_CELL_INFO_SIZE; i++)
                    {
                        if(FALSE == i_ncs_ptr[i].present)
                        {
                            continue;
                        }
                        ncell_num++;
                    }

                    //to send neighbour cells' info
                    for (i = 0; i < RR_CELL_INFO_SIZE; i++)
                    {
                        if(FALSE == i_ncs_ptr[i].present)
                        {
                            continue;
                        }

                        sprintf((char *)rsp_buf_ptr, "%d,%d,%d,",
                                      ncell_num--, i_ncs_ptr[i].arfcn, i_ncs_ptr[i].bsic);
                        rsp_buf_ptr += strlen((char *)rsp_buf_ptr);

                        sprintf((char *)rsp_buf_ptr, "%d,%d,%d,",
                                      i_ncs_ptr[i].rxlev, i_ncs_ptr[i].C1, i_ncs_ptr[i].C2);
                        rsp_buf_ptr += strlen((char *)rsp_buf_ptr);

                        i_ncs_e_ptr = rr_cells_info_ex_ptr->ncells;
                        sprintf((char *)rsp_buf_ptr, "%d,%d,%d,",
                                      i_ncs_e_ptr[i].ms_txpwr_max_ccch, i_ncs_e_ptr[i].rxlev_access_min, i_ncs_e_ptr[i].CBA);
                        rsp_buf_ptr += strlen((char *)rsp_buf_ptr);

                        sprintf((char *)rsp_buf_ptr, "%d,%d,%d,",
                                      i_ncs_e_ptr[i].CBQ, i_ncs_e_ptr[i].CRO, i_ncs_e_ptr[i].TempOffset);

                        sprintf((char *)rsp_buf_ptr, "%d,%d;",
                                      i_ncs_e_ptr[i].PT, i_ncs_e_ptr[i].CRH);
                        rsp_buf_ptr += strlen((char *)rsp_buf_ptr);
                    }

                    ATC_BuildInfoRsp(atc_config_ptr, (uint8*)g_rsp_str);
                    status = S_ATC_SUCCESS;
                }
                else
                {
                    SCI_TRACE_LOW("ATC: no network");
                    status = ERR_NO_NETWORK_SERVICE;
                }

            }

            SCI_FREE(rr_cells_info_bsic_ptr);
            SCI_FREE(rr_cells_info_bsic_traffic_ptr);
            SCI_FREE(rr_cells_info_ex_ptr);
            break;
        }

        case ATC_CMD_TYPE_TEST:
        {
            SCI_TRACE_LOW("ATC: Test Scinf CMD");
            status = S_ATC_SUCCESS;
            break;
        }

        default:
        {
            SCI_TRACE_LOW("ATC: Assert SCINF,invalid type.");
            break;
        }
     }
#endif
     return status;
}

/*****************************************************************************/
//  Description : This function handles the AT+SPEQPARA command
//                      Get(Set) eq para from(to) RAM or FALSH
//  Global resource dependence :
//  Author:     elvis.xu
//  Note:
/*****************************************************************************/
AT_CMD_FUNC( ATC_ProcessSPEQPARA)
{
#if defined( ATC_DISABLE_SPEQPARA)
    return S_ATC_SUCCESS;
#else
    AUDIO_EQ_STRUCT_T   *eq_para_ptr = NULL;
    uint8       *buffer_ptr = NULL;
    AUDIO_RESULT_E      set_result;

    if (ATC_CMD_TYPE_SET == ATC_GET_CMD_TYPE)
    {
        if (!PARAM1_FLAG)
        {
            SCI_TRACE_LOW("ATC: ATC_ProcessSPEQPARA: <n> is invalid.");
            return ERR_INVALID_INDEX;
        }

        /*PARA1.num is <n>*/
        switch (PARA1.num)
        {
            case 0:
            {
#if !defined(WIN32)
                //get the maximum count of eq para type
                sprintf((char *)g_rsp_str, "%s: %lu",
                        g_atc_info_table[AT_CMD_SPEQPARA].cmd_name->str_ptr,
                        AUDPROC_EXP_GetEqParaSetNum());
                ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
#endif
                break;
            }

            case 1:   //get eq para structure info
            {
                if ((!PARAM2_FLAG) || (!PARAM3_FLAG) || PARAM4_FLAG)
                {
                    SCI_TRACE_LOW("ATC: ATC_ProcessSPEQPARA: <type> or <dest> is invalid.");
                    return ERR_INVALID_INDEX;
                }

                /*PARA2.num is <type>;
                   PARA3.num is <dest>*/
                #if !defined(WIN32)
                if (0 == PARA3.num)
                {
                    /*get eq info from ram*/
                    eq_para_ptr = AUDPROC_EXP_GetEqSetFromRam((AUD_PROC_EQPARA_SET_E)PARA2.num);
                }
                else
                {
                    /*get eq info from flash*/
                    eq_para_ptr = AUDPROC_EXP_GetEqSetFromFlash((AUD_PROC_EQPARA_SET_E)PARA2.num);
                }
            #else
            eq_para_ptr = NULL;
            #endif
                if (NULL == eq_para_ptr)
                {
                    SCI_TRACE_LOW("ATC: ATC_ProcessSPEQPARA: Get EQ PARA failed.");
                    return ERR_OPERATION_NOT_ALLOWED;
                }

                buffer_ptr = (uint8*)SCI_ALLOC_BASEZ((sizeof(AUDIO_EQ_STRUCT_T)+1)*2);

                /*convert eq structure data to hex string*/
                ConvertBinToHex((uint8*)eq_para_ptr, sizeof(AUDIO_EQ_STRUCT_T), buffer_ptr);

                sprintf((char*)g_rsp_str, "%s: %s",
                              g_atc_info_table[AT_CMD_SPEQPARA].cmd_name->str_ptr,
                              buffer_ptr);

                SCI_FREE(buffer_ptr);
                ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
                break;
            }

            case 2:     //set eq para info
            {
                if ((!PARAM2_FLAG) || (!PARAM3_FLAG) || (!PARAM4_FLAG))
                {
                    SCI_TRACE_LOW("ATC: ATC_ProcessSPEQPARA: <type> or <dest> or <data> is invalid.");
                    return ERR_INVALID_INDEX;
                }

                /*<data> type must be string*/
                if (ATC_CMD_PARAM_TYPE_STRING != PARAM4_TYPE)
                {
                    SCI_TRACE_LOW("ATC: ATC_ProcessSPEQPARA: <data> type is not string.");
                    return ERR_INVALID_INDEX;
                }
                /*check <data> string length, <data> must be hex string*/
                if (PARA4.str_ptr->str_len != (sizeof(AUDIO_EQ_STRUCT_T)*2))
                {
                    SCI_TRACE_LOW("ATC: ATC_ProcessSPEQPARA: <data> length:%d != %d*2.", PARA4.str_ptr->str_len, sizeof(AUDIO_EQ_STRUCT_T));
                    return ERR_INVALID_INDEX;
                }
                eq_para_ptr = (AUDIO_EQ_STRUCT_T*)SCI_ALLOC_BASEZ(sizeof(AUDIO_EQ_STRUCT_T));

                /*convert hex string to bin*/
                if (!ConvertHexToBin(PARA4.str_ptr->str_ptr,(uint16)PARA4.str_ptr->str_len, (uint8*)eq_para_ptr))
                {
                    SCI_TRACE_LOW("ATC: ATC_ProcessSPEQPARA: ConvertHexToBin failed.");
                    SCI_FREE(eq_para_ptr);
                    return ERR_OPERATION_NOT_ALLOWED;
                }

                /*PARA2.num is <type>;
                   PARA3.num is <dest>*/
                #if !defined(WIN32)
                if (0 == PARA3.num)
                {
                    /*set eq info to ram*/
                    set_result = AUDPROC_EXP_SetEqSetToRam((AUD_PROC_EQPARA_SET_E)PARA2.num, eq_para_ptr);
                }
                else
                {
                    /*set eq info to flash*/
                    set_result = AUDPROC_EXP_SetEqSetToFlash((AUD_PROC_EQPARA_SET_E)PARA2.num, eq_para_ptr);
                }
            #else
            set_result = AUDIO_NO_ERROR;
                #endif
                SCI_FREE(eq_para_ptr);
                if (AUDIO_NO_ERROR != set_result)
                {
                    SCI_TRACE_LOW("ATC: ATC_ProcessSPEQPARA: Set EQ PARA failed.");
                    return ERR_OPERATION_NOT_ALLOWED;
                }
                break;
            }

            default:
            {
                return ERR_OPERATION_NOT_SUPPORTED;
            }
        }
    }
    else
    {
        return ERR_OPERATION_NOT_SUPPORTED;
    }
    return S_ATC_SUCCESS;
#endif
}

/*****************************************************************************/
//  Description : This function handle the AT+SPREF command
//                 Automatic downloader
//  Global resource dependence : none
//  Author:
//  Note:
/*****************************************************************************/
AT_CMD_FUNC( ATC_ProcessSPREF)
{

    if(ATC_CMD_TYPE_SET != ATC_GET_CMD_TYPE)
    {
        return ERR_OPERATION_NOT_ALLOWED;
    }

    if(PARAM1_FLAG && (PARAM1_TYPE == ATC_CMD_PARAM_TYPE_STRING))
    {
        if(!REF_HandleATCommand(PARA1.str_ptr->str_ptr))
        {
            return ERR_OPERATION_NOT_ALLOWED;
        }
    }
    else
    {
        return ERR_OPERATION_NOT_ALLOWED;
    }

    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handles the AT+PSORGMOD command.This command is used
//  to set audio mode. This is used for AP+BB scenario. If AP connect only with BB's
//  receiver, and AP want to use this physical channel as handon/handfree/BT earphone,
//  it will use this function to set audio param to DSP without change physical channel.
//  handon/handfree/BT should have the same device name in the NV item.
//  Global resource dependence : g_rsp_str
//  Author:
/*****************************************************************************/
AT_CMD_FUNC( ATC_ProcessPSORGMOD)
{
#ifndef _ULTRA_LOW_CODE_
    ATC_STATUS           status = S_ATC_SUCCESS;

    switch (ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:
        {
            if (PARAM1_FLAG)
            {
                atc_config_ptr->audio_mode = (uint8)PARA1.num;
                AUDIO_SetDevMode((AUDIO_DEVICE_MODE_TYPE_E)atc_config_ptr->audio_mode);
            }
            else
            {
                status = ERR_INVALID_INDEX;
            }
            break;
        }

        case ATC_CMD_TYPE_READ:
        {
            // get the current sound mode
            sprintf((char*)g_rsp_str, "%s: %d", g_atc_info_table[AT_CMD_PSORGMOD].cmd_name->str_ptr, atc_config_ptr->audio_mode);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;
        }

        default:
        {
            break;
        }
    }

    return status;
#else
    return S_ATC_FAIL;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the AT+SCREG command
//  Global resource dependence : g_rsp_str
//  Author:       Feng.shi
//  Note:
/*****************************************************************************/
AT_CMD_FUNC( ATC_ProcessSCREG )
{
#ifdef _ULTRA_LOW_CODE_
    return S_ATC_FAIL;
#else
    ATC_STATUS                     status = S_ATC_SUCCESS;
#if 0
    MN_PHONE_CURRENT_PLMN_INFO_T   register_status;

    MN_DUAL_SYS_E dual_sys =  ATC_CURRENT_SIM_ID_MUX;

    switch (ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_READ:
        {
            // call the MN API functions
            register_status = MNPHONE_GetCurrentPLMNInfoEx(dual_sys);

            if (register_status.plmn_status != PLMN_NO_SERVICE)
            {
                sprintf((char*)g_rsp_str,"%s: %d,%d,%04X,%04X", g_atc_info_table[AT_CMD_SCREG].cmd_name->str_ptr,
                    register_status.mcc, register_status.mnc, register_status.lac, register_status.cell_id);
            }
            else
            {
                sprintf((char*)g_rsp_str, "%s: 0,0,0,0", g_atc_info_table[AT_CMD_SCREG].cmd_name->str_ptr);
            }

            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            status = S_ATC_SUCCESS;
            break;
        }

        default:
        {
            break;
        }
    }
#endif
    return status;
#endif
}


/*****************************************************************************/
// Description : This function handle the AT+GSMSTATUS command
//                 Automatic downloader
// Global resource dependence : none
// Author:winnie.zhu
// Note:
/*****************************************************************************/
AT_CMD_FUNC( ATC_ProcessGSMSTATUS)
{
    const char *state_str[]={"OK","INITING","SIMERR","NETERR","OTHERERR"};
    uint8 state = 4;
    MN_DUAL_SYS_E dual_sys = ATC_CURRENT_SIM_ID_MUX;
    MN_PHONE_CURRENT_PLMN_INFO_T       register_status;

    SCI_MEMSET(&register_status, 0x0, sizeof(MN_PHONE_CURRENT_PLMN_INFO_T));

    switch (ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_EXECUTE:
        {
            if(POWER_MS_STATUS_INIT == POWER_GetMsStatus())
            {
                state = 1;
            }
            else if (!DMSIM_SimIsReady(dual_sys))
            {
                state = 2;
            }

            else
            {
                register_status = MNPHONE_GetCurrentPLMNInfoEx(dual_sys);
                switch(register_status.plmn_status)
                {
                    case PLMN_NORMAL_GSM_ONLY:
                    case PLMN_NORMAL_GPRS_ONLY:
                    case PLMN_NORMAL_GSM_GPRS_BOTH:
                    {
                        state = 0;
                        break;
                    }

                    case PLMN_EMERGENCY_ONLY:
                    case PLMN_EMERGENCY_SIM_INVALID:
                    case PLMN_EMERGENCY_GPRS_ONLY:
                    case PLMN_EMERGENCY_SIM_INVALID_GPRS_ONLY:
                    case PLMN_NO_SERVICE:
                    {
                        state = 3;
                        break;
                    }

                    default :
                    {
                        state = 4;
                        break;
                    }
                }
            }

            sprintf((char*)g_rsp_str, "%s",state_str[state]);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;
        }

        default:
            break;
    }
    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handle the AT+CIEV command
//  Global resource dependence : none
//  Author:
//  Note: this command set the rssi report level
/*****************************************************************************/
AT_CMD_FUNC( ATC_ProcessCIEV )
{
#ifdef _ULTRA_LOW_CODE_
    return S_ATC_FAIL;
#else
    ATC_STATUS    status = S_ATC_SUCCESS;

    switch (ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:
        {
            if(!PARAM1_FLAG || !PARAM2_FLAG || !PARAM3_FLAG || !PARAM4_FLAG || !PARAM5_FLAG)
            {
                status = ERR_OPERATION_NOT_ALLOWED;
                break;
            }
            if(PARA2.num < PARA3.num || PARA3.num < PARA4.num || PARA4.num < PARA5.num)
            {
                status = ERR_OPERATION_NOT_ALLOWED;
                break;
            }

            g_atc_ciev_rang   = PARA1.num;
            g_atc_ciev_lev[0] = PARA2.num;
            g_atc_ciev_lev[1] = PARA3.num;
            g_atc_ciev_lev[2] = PARA4.num;
            g_atc_ciev_lev[3] = PARA5.num;

            break;
        }

        case ATC_CMD_TYPE_READ:
        {
            sprintf((char*)g_rsp_str, "%s: %d, %d, %d, %d, %d",
                g_atc_info_table[AT_CMD_CIEV].cmd_name->str_ptr,
                g_atc_ciev_rang,
                g_atc_ciev_lev[0],
                g_atc_ciev_lev[1],
                g_atc_ciev_lev[2],
                g_atc_ciev_lev[3]);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;
        }

        default:
        {
            SCI_ASSERT(FALSE); /*assert verified check default case*/
            break;
        }
    }
    return status;
#endif /* _ULTRA_LOW_CODE_ */
}

/*****************************************************************************/
//  Description : This function handle the AT+FIRBOOT command
//                 Automatic downloader
//  Global resource dependence : none
//  Author:winnie.zhu
//  Note:
/*****************************************************************************/
#define NV_FIRBOOT_CONFIG_ID    (MN_ATNV_USER_BASE+5)

AT_CMD_FUNC( ATC_ProcessFIRBOOT )
{
    uint8 seed_str[21];
    uint8 seed_str1[21];
    switch (ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:
        {
            // check the index
            if (PARAM1_FLAG && (20 ==PARA1.str_ptr->str_len))
            {
                SCI_MEMCPY(seed_str,PARA1.str_ptr->str_ptr,PARA1.str_ptr->str_len);
                seed_str[PARA1.str_ptr->str_len] = '\0';

                //save in the NV.
                if (NVERR_NONE != EFS_NvitemWrite(NV_FIRBOOT_CONFIG_ID,
                                                sizeof(FIRBOOT_SEED_NV_T),
                                                PARA1.str_ptr->str_ptr,
                                                FALSE))
                {
                    SCI_TRACE_LOW("FIRBOOT:EFS_NvitemWrite error");
                    return S_ATC_FAIL;
                }

                EFS_NvitemRead(NV_FIRBOOT_CONFIG_ID,sizeof(FIRBOOT_SEED_NV_T),seed_str1);
                seed_str1[PARA1.str_ptr->str_len] = '\0';
                SCI_TRACE_LOW("winnie:read seed:%s",seed_str1);
            }
            else
            {
                return ERR_INVALID_INDEX;
            }
            break;
        }

        default:
            break;
    }
    return S_ATC_SUCCESS;
}

/*****************************************************************************/
// Description : This function handle the AT+VGSMSW command
//                 Automatic downloader
// Global resource dependence : none
// Author:winnie.zhu
// Note:
/*****************************************************************************/
AT_CMD_FUNC( ATC_ProcessVGSMSW )
{
#ifdef _NDS_EXT_CMD_
    uint8 rand_str[17] = {0};
    uint8 ndssign[24]  = {0};
    int ndssign_len     = 24;
    int ret = 0;

    switch (ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:
        {
            if (PARAM1_FLAG && (16 ==PARA1.str_ptr->str_len))
            {
                SCI_MEMCPY(rand_str,PARA1.str_ptr->str_ptr,PARA1.str_ptr->str_len);
                rand_str[PARA1.str_ptr->str_len] = '\0';
                SCI_MEMSET(ndssign,0,sizeof(ndssign));

                ret = sarft_sign_data(s_swversion,strlen((char *)s_swversion),rand_str,strlen((char *)rand_str),
                                    s_cid,s_gmid,s_seed,ndssign,&ndssign_len);
                if(0 != ret)
                {
                    SCI_TRACE_LOW("ATC: sarft sign data fail. ret= %d", ret);
                    return ERR_UNKNOWN;
                }

                sprintf((char*)g_rsp_str,"+VGSMSW: \"%s\",\"%s\",\"%s\"",rand_str,s_swversion,ndssign);
                ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            }
            else
            {
                return ERR_INVALID_INDEX;
            }

            break;
        }

        default:
            break;
    }
    return S_ATC_SUCCESS;
#else
    return S_ATC_FAIL;
#endif /* _NDS_EXT_CMD_ */
}

#endif /*ATC_SAT_ENABLE*/

/*****************************************************************************/
//  Description : This function handles the ATC_ProcessPowerOnCnf signal
//  Global resource dependence : g_rsp_str
//  Author:       Feng.shi
//  Note:
/*****************************************************************************/
ATC_STATUS ATC_ProcessPsPowerOnCnf(
            ATC_CONFIG_T *atc_config_ptr,
            MN_DUAL_SYS_E    dual_sys
            )
{
#ifdef ATC_SAT_ENABLE
    if(is_init_ps_power_on[dual_sys])
    {
        is_init_ps_power_on[dual_sys] = FALSE;
    }
    else
    {
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
    }
#else
    if(g_atc_power_info.cmd_id != AT_CMD_STPTEST)
    {
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);
        //to restore sms cell broadcast configurement
        ATC_InitSmsCellBroadcastPara(atc_config_ptr, FALSE, dual_sys);
    }
    else
    {
        SCI_MEMSET(&g_atc_power_info, 0, sizeof(ATC_POWER_INF_T));
    }
#endif

    if(atc_config_ptr->reset_flag[dual_sys] == TRUE)
    {
        g_is_ps_act[dual_sys]   = TRUE;
        atc_config_ptr->cfun_status[dual_sys] = ATC_FUN_ACT_PS;
        atc_config_ptr->reset_flag[dual_sys] = FALSE;
    }

    if (TRUE != atc_config_ptr->cfun_flag[dual_sys])
    {
        atc_config_ptr->cfun_flag[dual_sys] = TRUE;
    }

    return S_ATC_SUCCESS;
}


/*****************************************************************************/
//  Description : This function handles the ATC_ProcessPsPowerOffCnf signal
//  Global resource dependence : g_rsp_str
//  Author:       Feng.shi
//  Note:
/*****************************************************************************/
ATC_STATUS ATC_ProcessPsPowerOffCnf(
            ATC_CONFIG_T *atc_config_ptr,
            MN_DUAL_SYS_E    dual_sys
            )
{
#ifndef ATC_SAT_ENABLE
    if (g_atc_power_info.cmd_id == AT_CMD_STPTEST)
    {
        ATC_StartGsmTxPwrTest(atc_config_ptr);
        return S_ATC_SUCCESS;
    }
#endif

    atc_config_ptr->rel_former[dual_sys] = 127;

    ATC_TRACE_LOW("ATC: APP_MN_PS_POWER_OFF_CNF: reset_flag=%d", atc_config_ptr->reset_flag[dual_sys]);

    ATC_Reset(dual_sys);

    if (TRUE == atc_config_ptr->cfun_flag[dual_sys])
    {
        atc_config_ptr->cfun_flag[dual_sys] = FALSE;
    }

    if(atc_config_ptr->reset_flag[dual_sys] == TRUE)
    {
        MNPHONE_StartupPsEx(dual_sys, MN_GMMREG_RAT_GPRS);

        ATC_ADD_EXPECTED_EVENT_MUX(atc_config_ptr,
                          (uint16)APP_MN_PS_POWER_ON_CNF, 0xff, ATC_NO_DOMAIN, dual_sys);
    }
    else
    {
        ATC_BuildResultCodeRsp(atc_config_ptr, S_ATC_RESULT_CODE_OK, 0);

        if(TRUE != atc_config_ptr->cfun_flag[dual_sys])
        {
            if(ATC_16BIT1 == ((atc_config_ptr->atc_config_nv.wind_ind_level) | ATC_WIND_BIT7))
            {
                ATC_TRACE_LOW("ATC: cfun_flag:%d,APP_MN_PS_POWER_ON_CNF,URC:+SIND: 8", atc_config_ptr->cfun_flag[dual_sys]);
            }

            ATC_UpSINDInfo(dual_sys,
                           atc_config_ptr,
                           PNULL,
                           ATC_WIND_BIT7,
                           8,
                           (BOOLEAN)(0 != atc_config_ptr->atc_config_nv.creg));
        }
    }

    return S_ATC_SUCCESS;
}

/*****************************************************************************/
//  Description : This function handle the AT+SADMDSP command
//  Global resource dependence : g_rsp_str
//  Author:  Shijun.cui
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSADMDSP)
{
    ATC_STATUS        status = S_ATC_SUCCESS;
    ATC_AUD_MODE_OPT_TYPE_E    type = ATC_GET_AUD_MODE_NUM;
    uint32             mode_index = 0;
    uint32             dev_number = 0;
    AUDIO_NV_DSP_MODE_INFO_T*  audmod_devmod_ptr = NULL;
    #ifdef AUDIO_PARAM_ARM
    AUDIO_NV_DSP_MODE_INFO_T* audmod_dsp_devmod_ptr = PNULL;
    #endif
    uint8* str_ptr = NULL;

    if (ATC_GET_CMD_TYPE != ATC_CMD_TYPE_SET)
    {
        return ERR_OPERATION_NOT_SUPPORTED;
    }

    if(PARAM1_FLAG)
    {
        type = (ATC_AUD_MODE_OPT_TYPE_E)PARA1.num;
    }
    else
    {
        return ERR_INVALID_INDEX;
    }

    if(PARAM2_FLAG)
    {
        mode_index = (uint32)PARA2.num;
    }

    SCI_TRACE_LOW("ATC: ATC_ProcessSADMDSP");

    dev_number = AUDIONVDSP_GetModeNumber();

    switch(type)
    {
        case ATC_GET_AUD_MODE_NUM:
        {
            sprintf((char*)g_rsp_str, "%s: %ld", g_atc_info_table[AT_CMD_SADMDSP].cmd_name->str_ptr, dev_number);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;
        }

        case ATC_GET_AUD_MODE_NAME:
        {
            if(PARAM2_FLAG)
            {
                if ((ATC_CMD_PARAM_TYPE_NUMERIC==PARAM2_TYPE) && ((uint32)PARA2.num < dev_number))
                {

                    audmod_devmod_ptr = AUDIONVDSP_GetFirstDevMode();
                    SCI_TRACE_LOW("ATC: mode_index=%d", mode_index);
                    while(mode_index)
                    {
                        audmod_devmod_ptr =  AUDIONVDSP_GetNextDevMode(audmod_devmod_ptr);
                        mode_index--;
                    }
                    SCI_TRACE_LOW("ATC: audmod_devmod_ptr=0x%x, name %s", audmod_devmod_ptr, audmod_devmod_ptr->ucModeName);
                    sprintf((char*)g_rsp_str, "%s: %d,\"%s\"",
                            g_atc_info_table[AT_CMD_SADMDSP].cmd_name->str_ptr,
                            PARA2.num,
                            audmod_devmod_ptr->ucModeName);

                    ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
                }
                else
                {
                    status = ERR_INVALID_INDEX;
                }
            }
            else
            {
                status = ERR_INVALID_INDEX;
            }
            break;
        }

        case ATC_GET_AUD_MODE_DATA:
        {
            if(PARAM2_FLAG)
            {
                if ((ATC_CMD_PARAM_TYPE_NUMERIC==PARAM2_TYPE) && ((uint32)PARA2.num < dev_number))
                {
                    int str_rsp_len = 0;
                    char str_result_tmp[30] = {0};

                    if(mode_index < dev_number)
                    {
                        audmod_devmod_ptr =  AUDIONVDSP_GetFirstDevMode();

                        while(mode_index)
                        {
                            audmod_devmod_ptr =  AUDIONVDSP_GetNextDevMode(audmod_devmod_ptr);

                            mode_index--;
                        }

                        str_ptr = (uint8*)SCI_ALLOC_BASEZ((sizeof(AUDIO_NV_DSP_MODE_STRUCT_T))*2+1);

                        ConvertBinToHex((uint8*)&audmod_devmod_ptr->tAudioNvDspModeStruct, sizeof(AUDIO_NV_DSP_MODE_STRUCT_T), str_ptr);

                        sprintf(str_result_tmp, "%s: %d,\"%s\",", g_atc_info_table[AT_CMD_SADMDSP].cmd_name->str_ptr,
                                        PARA2.num,
                                        audmod_devmod_ptr->ucModeName);

                        str_rsp_len = strlen(str_result_tmp)+strlen((char*)str_ptr);
                        SCI_TRACE_LOW("ATC: str_rsp_len=%d ", str_rsp_len);

                        if(str_rsp_len>= MAX_ATC_RSP_LEN)
                        {
                            char * str_rsp = NULL;

                            str_rsp = (char *)SCI_ALLOC_BASEZ(str_rsp_len +1);

                            sprintf(str_rsp, "%s%s", str_result_tmp, str_ptr);

                            SCI_TRACE_LOW("ATC: str_len_rsp=%d ", strlen(str_rsp));

                            ATC_BuildInfoRsp(atc_config_ptr, (uint8*)str_rsp);
                            SCI_FREE(str_rsp);
                        }
                        else
                        {
                            sprintf((char*)g_rsp_str, "%s%s", str_result_tmp, str_ptr);
                            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
                        }

                        SCI_FREE(str_ptr);
                    }
                    else
                    {
                        status = ERR_INVALID_INDEX;
                    }
                }
                else
                {
                    status = ERR_INVALID_INDEX;
                }
            }
            else
            {
                status = ERR_INVALID_INDEX;
            }
            break;
        }

        case ATC_SET_AUD_MODE_DATA:
        {
            // input 246 bytes
            if(PARAM2_FLAG)
            {
                if ((ATC_CMD_PARAM_TYPE_NUMERIC==PARAM2_TYPE) && ((uint32)PARA2.num < dev_number))
                {
                    s_atc_admm_index = (uint8)PARA2.num;
                    // output the '>'
                    ATC_SendAsyncInfo(atc_config_ptr, ATC_CTRLZ_SADMDSP);
                    status = S_ATC_DEFAULT_HANDLE;
                }
                else
                {
                    status = ERR_INVALID_INDEX;
                }
            }
            else
            {
                status = ERR_INVALID_INDEX;
            }
            break;
        }

        case ATC_SET_AUD_DEV_MODE:
        {
            // get audio control mode
            uint16 audio_ctrl_mode = 0;

            AUDIONVDSP_GetAudioDspCodecInfoFromRam(&audio_ctrl_mode);
            SCI_TRACE_LOW("ATC: audio_ctrl_mode=%d", audio_ctrl_mode);
            sprintf((char*)g_rsp_str, "%s: %d",
                            g_atc_info_table[AT_CMD_SADMDSP].cmd_name->str_ptr,
                            audio_ctrl_mode);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;
        }

        case ATC_SET_AUD_MODE_NV_DATA:
        {
            if(PARAM2_FLAG)
            {
                if ((ATC_CMD_PARAM_TYPE_NUMERIC==PARAM2_TYPE) && ((uint32)PARA2.num < dev_number))
                {
                    s_atc_admm_index = (uint8)PARA2.num;
                    // output the '>'
                    ATC_SendAsyncInfo(atc_config_ptr, ATC_CTRLZ_SADMNV_DSP);
                    status = S_ATC_DEFAULT_HANDLE;
                }
                else
                {
                    status = ERR_INVALID_INDEX;
                }
            }
            else
            {
                status = ERR_INVALID_INDEX;
            }
            break;
        }
       #ifdef AUDIO_PARAM_ARM
        case ATC_GET_AUD_MODE_NV_DATA:
        {
            if ((!PARAM2_FLAG)
                || (!(PARAM2_TYPE == ATC_CMD_PARAM_TYPE_NUMERIC && (uint32)PARA2.num < dev_number)))
            {
                status = ERR_INVALID_INDEX;
                break;
            }
            mode_index = (uint32)PARA2.num;

            audmod_dsp_devmod_ptr = SCI_ALLOC_BASEZ(sizeof(AUDIO_NV_DSP_MODE_INFO_T));

            audmod_devmod_ptr = AUDIONVDSP_GetFirstDevMode();

            while(mode_index)
            {
                audmod_devmod_ptr =  AUDIONVDSP_GetNextDevMode(audmod_devmod_ptr);
                mode_index--;
            }

            //mode_name_ptr = AUDIONVDSP_GetAudioModeName(mode_index);
            if(PNULL == audmod_devmod_ptr)
            {
                SCI_TRACE_LOW("ATC: AUDIONVDSP_GetAudioModeName failed mode:%d.", mode_index);
                return ERR_NOT_FOUND;
            }

            if (AUDMOD_NO_ERROR != AUDIONVDSP_ReadModeParamFromFlash((char *)audmod_devmod_ptr->ucModeName, audmod_dsp_devmod_ptr))
            {
                SCI_TRACE_LOW("ATC: AUDIONVDSP_ReadModeParamFromFlash failed.");
                status = ERR_UNKNOWN;
                break;
            }
            str_ptr = (uint8*)SCI_ALLOC_BASEZ(sizeof(AUDIO_NV_DSP_MODE_STRUCT_T)*2+1);

            ConvertBinToHex((uint8*)&audmod_dsp_devmod_ptr->tAudioNvDspModeStruct, sizeof(AUDIO_NV_DSP_MODE_STRUCT_T),str_ptr);
            sprintf((char*)g_rsp_str, "%s: %ld,\"%s\",%s",
                        g_atc_info_table[AT_CMD_SADMDSP].cmd_name->str_ptr,
                        mode_index,
                        audmod_dsp_devmod_ptr->ucModeName,
                        str_ptr);

            SCI_FREE(str_ptr);
            SCI_FREE(audmod_dsp_devmod_ptr);

            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;
        }
        #endif

        default:
        {
            SCI_TRACE_LOW("ATC: Assert SADMDSP,invalid type.");
            status = ERR_INVALID_INDEX;
            break;
        }
    }

    return status;
}

/*****************************************************************************/
//Description : This function handle the AT+SPADLVVE command
//Global resource dependence : g_rsp_str
// Author:  chong.liu
//Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSPADLVVE)
{
    ATC_STATUS        status = S_ATC_SUCCESS;
#ifndef WIN32
    uint8             type = 0;
    uint32            mode_index = 0;
    uint32            dev_number = 0;
    LVVE_CTRL_PARAM_NV_T *lvve_ctrl_param_nv_ptr = PNULL;
    char *  mode_name_ptr = PNULL;
    uint8   *str_ptr = PNULL;

    SCI_TRACE_LOW("ATC: ATC_ProcessSPADLVVE");

    if (ATC_GET_CMD_TYPE != ATC_CMD_TYPE_SET)
    {
        return ERR_OPERATION_NOT_SUPPORTED;
    }

    if((PARAM1_FLAG ) &&(PARAM1_TYPE == ATC_CMD_PARAM_TYPE_NUMERIC))
    {
        type = (uint8)PARA1.num;
    }
    else
    {
        SCI_TRACE_LOW("ATC: param_1 is wrong");
        return ERR_INVALID_INDEX;
    }

    dev_number =LVVE_CTRL_PARAM_GetDevModeCount();
    if(type >= ATC_GET_LEVE_MODE_NAME)
    {
        if((PARAM2_FLAG) &&(PARAM2_TYPE == ATC_CMD_PARAM_TYPE_NUMERIC))
        {
            mode_index = (uint32)PARA2.num;
            if(mode_index >= dev_number)
            {
                SCI_TRACE_LOW("ATC: param_2 is wrong");
                return ERR_INVALID_INDEX;
            }
        }
        else
        {
            SCI_TRACE_LOW("ATC: param_2 is wrong");
            return ERR_INVALID_INDEX;
        }
    }

    switch(type)
    {
        case ATC_GET_LEVE_MODE_NUM:
        {
            sprintf((char*)g_rsp_str, "%s: %d", g_atc_info_table[AT_CMD_SPADLVVE].cmd_name->str_ptr, dev_number);
            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;
        }

        case ATC_GET_LEVE_MODE_NAME:
        {
            SCI_TRACE_LOW("ATC: SADLVVE type = %d,mode_index = %d", type, mode_index);

            mode_name_ptr = LVVE_CTRL_PARAM_GetDevModeName(mode_index);
            if(PNULL == mode_name_ptr)
            {
                SCI_TRACE_LOW("ATC: LVVE_CTRL_PARAM_GetDevModeName failed to get mode name, mode:%d.", mode_index);
                return ERR_NOT_FOUND;
            }

            sprintf((char*)g_rsp_str, "%s: %d,\"%s\"",
                        g_atc_info_table[AT_CMD_SPADLVVE].cmd_name->str_ptr,
                        mode_index,
                        mode_name_ptr);

            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;
        }

        case ATC_GET_LEVE_MODE_DATA:
        {
            SCI_TRACE_LOW("ATC: SADLVVE type = %d,mode_index = %d, %d", type, mode_index,  sizeof(LVVE_CTRL_PARAM_NV_T));

            lvve_ctrl_param_nv_ptr = (LVVE_CTRL_PARAM_NV_T*)SCI_ALLOC_BASEZ(sizeof(LVVE_CTRL_PARAM_NV_T));

            mode_name_ptr = LVVE_CTRL_PARAM_GetDevModeName(mode_index);
            if(PNULL == mode_name_ptr)
            {
                SCI_TRACE_LOW("LVVE_CTRL_PARAM_GetDevModeName failed to get mode name, mode:%d.", mode_index);
                SCI_FREE(lvve_ctrl_param_nv_ptr);
                return ERR_NOT_FOUND;
            }
            else
            {
                SCI_TRACE_LOW("ATC:mode_name_ptr: %s, 0x%x", mode_name_ptr, lvve_ctrl_param_nv_ptr);
                if(AUDIO_NV_ARM_NO_ERROR != LVVE_CTRL_PARAM_GetDevModeParam((uint8 *) mode_name_ptr, (LVVE_CTRL_PARAM_NV_T *)(lvve_ctrl_param_nv_ptr)))
                {
                    SCI_TRACE_LOW("LVVE_CTRL_PARAM_GetDevModeParam failed to get mode para.");
                    SCI_FREE(lvve_ctrl_param_nv_ptr);
                    return ERR_NOT_FOUND;
                }

                str_ptr = (uint8*)SCI_ALLOC_BASEZ((sizeof(LVVE_CTRL_PARAM_NV_T)+1)*2);

                ConvertBinToHex((uint8*)lvve_ctrl_param_nv_ptr, sizeof(LVVE_CTRL_PARAM_NV_T), str_ptr);

                sprintf((char*)g_rsp_str, "%s: %d,\"%s\",%s",
                            g_atc_info_table[AT_CMD_SPADLVVE].cmd_name->str_ptr,
                            mode_index,
                            mode_name_ptr,
                            str_ptr);

                SCI_FREE(lvve_ctrl_param_nv_ptr);
                SCI_FREE(str_ptr);
                ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            }
            break;
        }

        case ATC_SET_LEVE_MODE_DATA:
        {
            s_atc_admm_index = (uint8)PARA2.num;
            // output the '>'
            ATC_SendAsyncInfo(atc_config_ptr, ATC_CTRLZ_LVVE);
            status = S_ATC_DEFAULT_HANDLE;

            break;
        }

        case ATC_GET_LEVE_MODE_NV_DATA:
        {
            SCI_TRACE_LOW("ATC: LVVE type = %d, mode_index = %d", type, mode_index);

            lvve_ctrl_param_nv_ptr = SCI_ALLOC_BASEZ(sizeof(LVVE_CTRL_PARAM_NV_T));

            mode_name_ptr = LVVE_CTRL_PARAM_GetDevModeName(mode_index);
            if(PNULL == mode_name_ptr)
            {
                SCI_TRACE_LOW("ATC: LVVE_CTRL_PARAM_GetDevModeName failed to get mode name, mode:%d.", mode_index);
                SCI_FREE(lvve_ctrl_param_nv_ptr);
                return ERR_NOT_FOUND;
            }

            SCI_TRACE_LOW("ATC: AGC_Get_Param to get mode name, mode:%d, %s.", mode_index, mode_name_ptr);
            if (AUDMOD_NO_ERROR != LVVE_CTRL_PARAM_ReadModeParamFromFlash((uint8 *)mode_name_ptr, lvve_ctrl_param_nv_ptr))
            {
                SCI_TRACE_LOW("ATC: LVVE_CTRL_PARAM_ReadModeParamFromFlash failed.");
                SCI_FREE(lvve_ctrl_param_nv_ptr);
                status = ERR_UNKNOWN;
                break;
            }
            str_ptr = (uint8*)SCI_ALLOC_BASEZ((sizeof(LVVE_CTRL_PARAM_NV_T)+1)*2);

            ConvertBinToHex((uint8*)lvve_ctrl_param_nv_ptr, sizeof(LVVE_CTRL_PARAM_NV_T),str_ptr);
            sprintf((char*)g_rsp_str, "%s: %d,\"%s\",%s",
                        g_atc_info_table[AT_CMD_SPADLVVE].cmd_name->str_ptr,
                        mode_index,
                        mode_name_ptr,
                        str_ptr);

            SCI_FREE(str_ptr);
            SCI_FREE(lvve_ctrl_param_nv_ptr);

            ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
            break;
        }

        case ATC_SET_LEVE_MODE_NV_DATA:
        {
            s_atc_admm_index = (uint8)PARA2.num;
            // output the '>'
            ATC_SendAsyncInfo(atc_config_ptr, ATC_CTRLZ_LVVENV);
            status = S_ATC_DEFAULT_HANDLE;
            break;
        }

        default:
        {
            SCI_ASSERT(FALSE);/*assert verified:invalid type*/
            break;
        }
    }
#endif
    return status;
}

/*****************************************************************************/
//  Description : This function handle the AT+SPAUDVOLTYPE command
//  Global resource dependence : g_rsp_str
//  Author:  chong.liu
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSPAUDVOLTYPE)
{
    ATC_STATUS        status = S_ATC_SUCCESS;
    uint32            audio_type = 0;
    uint32            audio_volume = 0;

    switch(ATC_GET_CMD_TYPE)
    {
        case ATC_CMD_TYPE_SET:
        {
            if((PARAM1_FLAG && (PARAM1_TYPE == ATC_CMD_PARAM_TYPE_NUMERIC))
                &&
               (PARAM2_FLAG && (PARAM2_TYPE == ATC_CMD_PARAM_TYPE_NUMERIC)))
            {
                audio_volume = PARA1.num;
                audio_type = PARA2.num;

                #ifdef AUDIO_PARAM_ARM
                    if( 0 != AUDIO_SetVolumeAndTypeEx(INVALID_HANDLE, audio_volume, audio_type))
                    {
                        return ERR_OPERATION_NOT_ALLOWED;
                    }
                #else
                    if(0 != AUDIO_SetVolume(audio_volume))
                    {
                        return ERR_OPERATION_NOT_ALLOWED;
                    }
                #endif
            }
            else
            {
                return ERR_INVALID_INDEX;
            }
            break;
        }

        default:
            break;
    }

    return status;
}

AT_CMD_FUNC(ATC_ProcessSFSTEST)
{
	return S_ATC_SUCCESS;
}
/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
}
#endif
