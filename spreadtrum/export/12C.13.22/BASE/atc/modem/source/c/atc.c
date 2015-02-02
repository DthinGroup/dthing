/******************************************************************************
 ** File Name:      atc.c                                                     *
 ** Author:         Steven & Ivan                                             *
 ** Date:           05/24/2002                                                *
 ** Copyright:      Copyright 2001-2010 by Spreadtrum Communications,Inc.     *
 **                 All Rights Reserved.                                      *
 **                 This software is supplied under the terms of a license    *
 **                 agreement or non-disclosure agreement with Spreadtrum.    *
 **                 Passing on and copying of this document,and communication *
 **                 of its contents is not permitted without prior written    *
 **                 authorization.                                            *
 ** Description:    This file is the main file of ATC module. It contains     *
 **                 most of the functions which control the ATC line read,    *
 **                 process, error handle and others.                         *
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                       *
 ** --------------------------------------------------------------------------*
 ** DATE           NAME             Description:                              *
 **---------------------------------------------------------------------------*
 ** 05/24/2002     Steven           Create.                                   *
 ** 06/20/2002     Ivan             start to edit and modity                  *
 ** 08/20/2002     Xueli            Modify and add                            *
 ******************************************************************************/

/*******************************************************************************
 **                        ATC Header Files                                    *
 ******************************************************************************/
#include "mux_all.h"

#include "atc.h"
#include "atc_common.h"
#include "atc_malloc.h"
#include "atc_gsm_ss.h"
#include "atc_gsm.h"
#include "atc_gprs_modem.h"
#include "atc_gsm_sms.h"
#include "atc_phonebook.h"
#include "atc_basic_cmd.h"
#ifndef ATC_SAT_ENABLE
#include "atc_mbbms.h"
#endif
#include "dal_chr.h"
#include "mnclassmark_api.h"
//#include "env_atc_signal.h"
#include "sig_code.h"
#include "atc_port.h"
#include "atc_ppp.h"
#include "atc_tcp_if.h"
#include "dal_audio.h"
#ifdef _MUX_ENABLE_
#include "atc_mux.h"
#endif
#ifdef _SUPPORT_GPRS_
#include "mn_api.h"
#endif

#include "sio.h"
#include "mux_api.h"

#ifdef _RTOS
#ifdef WIN32
#else
#endif //end WIN32
#include "sci_service.h"
#include "dal_power.h"
#endif //end _RTOS

#include "arm_reg.h"

#ifndef ATC_SAT_ENABLE
#include "atc_stk.h"
#include "atc_eng.h"
#endif

#include "atc_gsm_sms.h"
#include "sci_codec.h"
#include "nv_classmark.h"

#ifdef WIN32
#define ATC_MODE          0x01
#define DATA_MODE         0x02
#define MIX_MODE    0x03
#define COM_USER          0
#define COM_DATA          0
#define COM_DEBUG         1
#endif

#define S_CONFIG_INFO_PTR       &s_config_info

/**---------------------------------------------------------------------------*
 **                         Declare Variable                                  *
 **---------------------------------------------------------------------------*/
extern  void ATC_ProcessSocketEvent(ATC_CONFIG_T *atc_config_ptr,xSignalHeaderRec *sig_ptr);

MN_DUAL_SYS_E               g_current_card_id = MN_DUAL_SYS_1;
#ifdef _MUX_ENABLE_
extern MN_DUAL_SYS_E        g_atc_modem_dial_sys;
uint32                      g_cur_atc_cmd_size[MN_SYS_NUMBER] = {0};
#endif
extern const ATC_INFO_T     g_atc_info_table[];
extern ATC_Func_Handle_F    g_atc_handle_table[];
extern const uint8         *g_atc_result_code_text[];

extern SCI_TMID             T_ATC_CALL_EXP_IND;
extern SCI_TMID             T_ATC_POWEROFF_EXP_IND;
extern BOOLEAN              g_atc_call_flag;
extern BOOLEAN              g_received_sim_ready[];

#ifndef ATC_SAT_ENABLE
extern BOOLEAN              g_is_stk_sendsms[];
extern BOOLEAN              g_is_stk_sendussd[];
extern BOOLEAN              g_is_stk_sendss[];
extern BOOLEAN              g_is_stk_refresh_reset[];
#ifndef _MUX_ENABLE_
extern BOOLEAN              g_is_diag_cmd;
#endif
#else
extern uint16               g_rxlev_arr[MN_SYS_NUMBER];
extern uint16               g_auto_send_len;
#endif

extern ATC_PLMN_INFO_T      g_cur_plmn_info[];

extern BOOLEAN              g_atc_ps_not_save_sms[MN_SYS_NUMBER];


#ifdef _MUX_ENABLE_
#ifdef ATC_SAT_ENABLE
    BOOLEAN                 g_is_mux_single_sys_mod = FALSE;
#else
    BOOLEAN                 g_is_mux_single_sys_mod = TRUE;
#endif

uint8                       atc_urc_link_id = 0;
TCPIP_PACKET_TYPE_E         g_packet_type[MN_GPRS_MAX_PDP_CONTEXT_COUNT+1] = {TCPIP_PKTTYPE_NULL};
#endif
const ATC_RSP_STR_T         g_atc_cpin_rsp[] =
{
    "+CPIN: READY",         // No farther entry needed
    "+CPIN: SIM PIN",       // ME is waiting for SIM PIN
    "+CPIN: SIM PIN2",      // ME is waiting for SIM PUK
    "+CPIN: SIM PUK",      // ME is waiting for SIM2
    "+CPIN: SIM PUK2",      // ME is waiting for SIM PUK2
    "+CPIN: BLOCK"          // The SIM card is blocked for the user
};

const ATC_RSP_STR_T         g_atc_ecpin2_rsp[] =
{
    "+ECPIN2: READY",         // No farther entry needed
    "+ECPIN2: INVALID", 
    "+ECPIN2: SIM PIN2",      // ME is waiting for SIM PIN2
    "+ECPIN2: INVALID", 
    "+ECPIN2: SIM PUK2",      // ME is waiting for SIM PUK2
    "+ECPIN2: BLOCK"          // PUK2 is blocked for the user
};

static const ATC_SMS_RSP_STR_T s_sms_verbos_info[] =
{
    {300,   "ME failure"},
    {301,   "SMS service of ME reserved"},
    {302,   "Operation not allowed"},
    {303,   "Operation not supported"},
    {304,   "Invalid PDU mode parameter"},
    {305,   "Invalid text mode parameter"},
    {310,   "SIM not inserted"},
    {311,   "SIM PIN required"},
    {312,   "PH-SIM PIN required"},
    {313,   "SIM failure"},
    {315,   "SIM wrong"},
    {316,   "SIM PUK required"},
    {317,   "SIM PIN2 required"},
    {318,   "SIM PUK2 required"},
    {320,   "memory failure"},
    {321,   "Invalid memory index"},
    {322,   "SIM memory full"},
    {330,   "SC address unknown"},
    {331,   "no network service"},
    {332,   "network timeout"},
    {340,   "no +CNMA acknowledgement expected"},
    {500,   "unknown error"},
    {512,   "sms conflict with mt call"}
};

RSP_RESULT_STR                  g_rsp_str = {0};
static ATC_GLOBAL_INFO_T        s_atc_global_info;
ATC_CONFIG_T                    s_config_info;
BOOLEAN                         is_close_atc_queue = FALSE;
ATC_CALLS_STATE_T               g_calls_state[MN_SYS_NUMBER] = {0};
ATC_EMG_NUM_LIST_IND_T         *g_emg_num_list_ptr[MN_SYS_NUMBER] = {PNULL};

static BOOLEAN                  s_ps_service_is_activate = FALSE;

static BOOLEAN                  atc_err_protected_flag = FALSE;

const ATC_CEER_ERROR_STR_T atc_ceer_error_verbose[] =
{
    { CEER_ERR_0_NO_ERROR, "no error" },
    { CEER_ERR_1_UNASSIGNED_NUMBER, "unassigned (unallocated) number" },
    { CEER_ERR_3_NO_ROUTE_TO_DESTINATION, "no route to destination" },
    { CEER_ERR_6_UNACCEPTABLE_CHANNEL, "unacceptable channel" },
    { CEER_ERR_8_OPERATOR_DETERMINATE_BARRING, "operator determinate barring" },
    { CEER_ERR_16_NORMAL_CLEARING, "normal clearing" },
    { CEER_ERR_17_USER_BUSY, "user busy" },
    { CEER_ERR_18_NO_USER_RESPONDING, "no user responding" },
    { CEER_ERR_19_ALERTING_NO_ANSWER, "alerting no answer" },
    { CEER_ERR_21_CALL_REJECTED, "call rejected" },
    { CEER_ERR_22_NUMBER_CHANGED, "number changed" },
    { CEER_ERR_26_NONSELECT_USER_CLEARING, "nonselect user clearing" },
    { CEER_ERR_27_DESTINATION_OUT_OF_ORDER, "destination out of order" },
    { CEER_ERR_28_INVALID_NUMBER_FORMAT, "invalid number format" },
    { CEER_ERR_29_FACILITY_REJECTED, "facility rejected" },
    { CEER_ERR_30_RESPONSE_TO_STATUS_QUERY, "response to status query" },
    { CEER_ERR_31_NORMAL_UNSPECIFIED, "normal unspecified" },
    { CEER_ERR_34_NO_CIRCUIT_CHANNEL_AVAILABLE, "no circuit channel available" },
    { CEER_ERR_38_NET_OUT_OF_ORDER, "net out of order" },
    { CEER_ERR_41_TEMPORARY_FAILURE, "temporary failure" },
    { CEER_ERR_42_SWITCH_CONGESTION, "switch congestion" },
    { CEER_ERR_43_ACCESS_INFORMATION_DISCARDED, "access information discarded" },
    { CEER_ERR_44_REQUEST_CIRCUIT_CHANNEL_UNAVAILABLE, "request circuit channel unavailable" },
    { CEER_ERR_47_RESOURCES_UNAVAILABLE, "resources unavailable" },
    { CEER_ERR_49_QOS_UNAVAILABLE, "quality of service unavailable" },
    { CEER_ERR_50_REQUEST_FACILITY_NOT_SUBSCRIBE, "request facility not subscribe" },
    { CEER_ERR_55_CUG_INCOMING_BARRED, "CUG incoming barred" },
    { CEER_ERR_57_BEAR_CAPABILITY_NOT_AUTHORIZATION, "bear capability not authorization" },
    { CEER_ERR_58_BEAR_CAPABILITY_UNAVAILABLE, "bear capability unavailable" },
    { CEER_ERR_63_SERVICE_UNAVAILABLE, "service unavailable" },
    { CEER_ERR_65_BEAR_SERVICE_NOT_IMPLEMENT, "bear service not implement" },
    { CEER_ERR_68_ACM_EQUAL_OR_GREAT_ACMMAX, "ACM equal to or greater than ACMmax" },
    { CEER_ERR_69_REQUEST_FACILITY_NOT_IMPLEMENT, "request facility not implement" },
    { CEER_ERR_70_ONLY_RESTRICT_DIGITAL_AVAILABLE, "only restrict digital available" },
    { CEER_ERR_79_SERVICE_OPTION_NOT_IMPLEMENT, "service option not implement" },
    { CEER_ERR_81_INVALID_TI, "invalid ti" },
    { CEER_ERR_87_USER_NOT_IN_CUG, "user not in CUG" },
    { CEER_ERR_88_INCOMPATIBILITY_DESTINATION, "incompatibility destination" },
    { CEER_ERR_91_INVALID_TRANSIT_NET, "invalid transit net" },
    { CEER_ERR_95_INVALID_MESSAGE_SEMANTIC, "invalid message semantic" },
    { CEER_ERR_96_MANDATORY_IE_ERROR, "mandatory IE error" },
    { CEER_ERR_97_MESSAGE_NONEXISTENT, "message nonexistent" },
    { CEER_ERR_98_MESSAGE_UNCOMPATBILITY_ERROR, "message uncompatbility error" },
    { CEER_ERR_99_IE_NONEXISTENT, "IE nonexistent" },
    { CEER_ERR_100_INVALID_CONDITION_IE, "invalid condition IE" },
    { CEER_ERR_101_MESSAGE_INCOMPATIBILITY_STATE, "message incompatibility state" },
    { CEER_ERR_102_RECOVER_ON_TIMER, "recover on timer" },
    { CEER_ERR_111_PROTOCOL_ERROR, "protocol error" },
    { CEER_ERR_127_INTERWORKING, "interworking" },
    { CEER_ERR_150_AUTHENTICATION_REJECTED, "authentication rejected" },
    { CEER_ERR_151_EMERGENCY_CALL_ONLY, "emergency call only" },
    { CEER_ERR_152_IMSI_DETACH, "IMSI detach" },
    { CEER_ERR_153_T3230_EXPIRY, "T3230 expiry" },
    { CEER_ERR_154_RR_CONNECTION_ERROR, "rr connection error" },
    { CEER_ERR_171_NO_NETWORK_SERVICE, "no network service" },
    { CEER_ERR_172_EMERGENCY_CALL_ONLY, "emergency call only" },
    { CEER_ERR_173_NORMAL_DISCONNECT, "normal disconnect" },
    { CEER_ERR_174_REMOTE_DISCONNECT, "remote disconnect" },
    { CEER_ERR_175_LOW_FAILURE, "low failure" },
    { CEER_ERR_176_NETWORK_REJECT, "network reject" },
    { CEER_ERR_177_NO_CELL, "no cell" },
    { CEER_ERR_180_NOMAL_DETACH_GPRS, "GPRS detached" },
    { CEER_ERR_181_NOMAL_DEACTIVE_PDP, "PDP deactive" },
    { CEER_ERR_202_SUPPLEMENT_NOT_PROVIDE, "supplement not provide" },
    { CEER_ERR_241_FDN_RESTRICTION, "FDN restriction" }
};

#ifndef WIN32
extern BOOLEAN RM_GetCalibrationPostMode(void);
#endif

#ifndef ATC_SAT_ENABLE
extern BOOLEAN        is_xor_mbau_cmd;
#else
BOOLEAN               is_init_ps_power_on[MN_SYS_NUMBER] = {0};//0: FALSE,1:TRUE, only in atc initialization process
#endif


#ifdef _MUX_ENABLE_

extern uint8          g_atc_expire_call_id;
extern BOOLEAN        is_cmux_recv;
extern MUX_SYS_PARA_T cmux_sys_para;

ATC_BUF_T             atc_buffer[MN_SYS_NUMBER] = {0};

static BOOLEAN        atc_is_echo;

static char           atc_end_ch1[ATC_MAX_LINK_ID];
static char           atc_end_ch2[ATC_MAX_LINK_ID] = {'\0'};
static char           atc_bs_ch[ATC_MAX_LINK_ID];       // Backspace char of ATC

static uint32         s_dicard_count = 0;

#ifndef ATC_SAT_ENABLE
#ifdef _DEBUG
static struct
{
    uint16      event;
    const char *name;
} expect_event_name[] =
{
#define EVENT_DEF(a) { a,  #a }

    EVENT_DEF(APP_MN_NETWORK_STATUS_IND),
    EVENT_DEF(APP_MN_PLMN_LIST_CNF),
    EVENT_DEF(APP_MN_PLMN_SELECT_CNF),
#ifndef MMI_SIMULATOR
    EVENT_DEF(APP_MN_PLMN_READ_CNF),
    EVENT_DEF(APP_MN_PLMN_WRITE_CNF),
    EVENT_DEF(APP_MN_PREFER_PLMN_CHANGE_CNF),
#endif
    EVENT_DEF(APP_MN_SIM_GET_PIN_IND),
    EVENT_DEF(APP_MN_SIM_READY_IND),
    EVENT_DEF(APP_MN_SIM_NOT_READY_IND),
    EVENT_DEF(APP_MN_SIM_PIN_FUNC_CNF),
    EVENT_DEF(APP_MN_GET_SIM_TYPE_CNF),
    EVENT_DEF(APP_MN_GET_UIM_IMSI_CNF),
    EVENT_DEF(APP_MN_SCELL_RSSI_IND),
    EVENT_DEF(APP_MN_MS_POWERING_OFF_IND),
    EVENT_DEF(APP_MN_DEACTIVE_PS_CNF),
    EVENT_DEF(APP_MN_NCELL_RSSI_CNF),
    EVENT_DEF(APP_MN_AOC_MODIFIED_IND),
    EVENT_DEF(APP_MN_FDN_SERVICE_IND),
    EVENT_DEF(APP_MN_GPRS_ATTACH_RESULT),
    EVENT_DEF(APP_MN_GPRS_DETACH_RESULT),
    EVENT_DEF(APP_MN_DETACH_GSM_CNF),
    EVENT_DEF(APP_MN_PUCT_INFO_CNF),
    EVENT_DEF(APP_MN_PS_READY_IND),
    EVENT_DEF(APP_MN_SET_PREF_MODE_CNF),
    EVENT_DEF(APP_MN_SET_BAND_CNF),
    EVENT_DEF(APP_MN_SET_SERVICE_TYPE_CNF),
    EVENT_DEF(APP_MN_SET_ROAM_MODE_CNF),
    EVENT_DEF(APP_MN_DISPLAY_PLMN_IND),
    EVENT_DEF(APP_MN_READ_GID_CNF),

    EVENT_DEF(APP_MN_SIM_POWER_ON_CNF),
    EVENT_DEF(APP_MN_SIM_POWER_OFF_CNF),
    EVENT_DEF(APP_MN_PS_POWER_ON_CNF),
    EVENT_DEF(APP_MN_PS_POWER_OFF_CNF),

    EVENT_DEF(APP_MN_READ_SIM_FILE_BUF_CNF),

    /* be used for phonebook service */
    EVENT_DEF(APP_MN_PHONEBOOK_SIGNAL_START),
    EVENT_DEF(APP_MN_MSISDN_UPDATE_CNF),
    EVENT_DEF(APP_MN_ADN_UPDATE_CNF),
    EVENT_DEF(APP_MN_FDN_UPDATE_CNF),

    EVENT_DEF(APP_MN_GET_FREE_EXT_INFO_CNF),
    EVENT_DEF(APP_MN_CRSM_CNF),
    EVENT_DEF(APP_MN_CPHS_INFO_IND),
    EVENT_DEF(APP_MN_OPER_NAME_IND),
    EVENT_DEF(APP_MN_CFF_IND),
    EVENT_DEF(APP_MN_DCS_UPDATE_CNF),
    EVENT_DEF(APP_MN_CSP_UPDATE_CNF),
    EVENT_DEF(APP_MN_PHONEBOOK_SIGNAL_END),

    /* be used for call service */
    EVENT_DEF(APP_MN_CALL_SIGNAL_START),
    EVENT_DEF(APP_MN_CALL_READY_IND),
    EVENT_DEF(APP_MN_CALL_START_IND),
    EVENT_DEF(APP_MN_ALERTING_IND),
    EVENT_DEF(APP_MN_SETUP_COMPLETE_IND),
    EVENT_DEF(APP_MN_SETUP_COMPLETE_CNF),
    EVENT_DEF(APP_MN_CALL_DISCONNECTED_IND),
    EVENT_DEF(APP_MN_CALL_ERR_IND),
    EVENT_DEF(APP_MN_SETUP_IND),
    EVENT_DEF(APP_MN_START_DTMF_CNF),
    EVENT_DEF(APP_MN_STOP_DTMF_CNF),
    EVENT_DEF(APP_MN_CALL_HELD_CNF),
    EVENT_DEF(APP_MN_CALL_HELD_IND),
    EVENT_DEF(APP_MN_CALL_HOLDING_IND),
    EVENT_DEF(APP_MN_CALL_RETRIEVED_CNF),
    EVENT_DEF(APP_MN_CALL_RETRIEVED_IND),
    EVENT_DEF(APP_MN_CALL_RETRIEVING_IND),
    EVENT_DEF(APP_MN_BUILDING_MPTY_IND),
    EVENT_DEF(APP_MN_SPLITTING_MPTY_IND),
    EVENT_DEF(APP_MN_ECT_INVOKING_IND),
    EVENT_DEF(APP_MN_CALL_SWAP_IND),
    EVENT_DEF(APP_MN_CALL_WAIT_IND),
    EVENT_DEF(APP_MN_CALL_NOTIFY_IND),
    EVENT_DEF(APP_MN_CALL_MODIFY_CNF),
    EVENT_DEF(APP_MN_CALL_MODIFY_IND),
    EVENT_DEF(APP_MN_CALL_BARRED_IND),
    EVENT_DEF(APP_MN_BUILD_MPTY_CNF),
    EVENT_DEF(APP_MN_BUILD_MPTY_IND),
    EVENT_DEF(APP_MN_SPLIT_MPTY_CNF),
    EVENT_DEF(APP_MN_ECT_INVOKE_CNF),
    EVENT_DEF(APP_MN_ECT_INVOKE_IND),
    EVENT_DEF(APP_MN_ACMMAX_IND),
    EVENT_DEF(APP_MN_TMPTY_EXP_IND),
    EVENT_DEF(APP_MN_SYNC_IND),
    EVENT_DEF(APP_MN_PROGRESS_IND),
    EVENT_DEF(APP_MN_USER_TO_USER_IND),
    EVENT_DEF(APP_MN_DISCONNECTING_IND),
    EVENT_DEF(APP_MN_CALL_FORWARD_IND),
    EVENT_DEF(APP_MN_FORWARD_STATUS_IND),
    EVENT_DEF(APP_MN_CONNECTING_IND),
    EVENT_DEF(APP_MN_DISC_INFO_IND),
    EVENT_DEF(APP_MN_CALL_SIGNAL_END),

    /* be used for gprs service */
    EVENT_DEF(APP_MN_GPRS_SIGNAL_START),
    EVENT_DEF(APP_MN_MTACT_RESPOND_MODE_CNF),
    EVENT_DEF(APP_MN_ACTIVATE_PDP_CONTEXT_CNF),
    EVENT_DEF(APP_MN_SET_QOS_CNF),
    EVENT_DEF(APP_MN_SET_MIN_QOS_CNF),
    EVENT_DEF(APP_MN_SET_PDP_CONTEXT_CNF),
    EVENT_DEF(APP_MN_DEACTIVATE_PDP_CONTEXT_CNF),
    EVENT_DEF(APP_MN_DELETE_PDP_CONTEXT_CNF),
    EVENT_DEF(APP_MN_DELETE_MIN_QOS_CNF),
    EVENT_DEF(APP_MN_DELETE_QOS_CNF),
    EVENT_DEF(APP_MN_PUSH_GPRS_DATA_CNF),
    EVENT_DEF(APP_MN_DEACTIVATE_PDP_CONTEXT_IND),
    EVENT_DEF(APP_MN_GPRS_SIGNAL_END),

    /* be used for sms service */
    EVENT_DEF(APP_MN_SMS_SIGNAL_START),
    EVENT_DEF(APP_MN_SMS_READY_IND),
    EVENT_DEF(APP_MN_SMS_MEM_FULL_IND),
#ifndef MMI_SIMULATOR
    EVENT_DEF(APP_MN_SMS_MEM_OVER_FLOW_IND),
#endif
    EVENT_DEF(APP_MN_READ_SMS_CNF),
    EVENT_DEF(APP_MN_UPDATE_SMS_STATE_CNF),
    EVENT_DEF(APP_MN_READ_SMS_STATUS_REPORT_CNF),
    EVENT_DEF(APP_MN_SEND_SMS_CNF),
    EVENT_DEF(APP_MN_WRITE_SMS_CNF),
    EVENT_DEF(APP_MN_EDIT_SMS_CNF),
    EVENT_DEF(APP_MN_SMS_IND),
    EVENT_DEF(APP_MN_SMS_STATUS_REPORT_IND),
    EVENT_DEF(APP_MN_READ_SMS_PARAM_CNF),
    EVENT_DEF(APP_MN_WRITE_SMS_PARAM_CNF),
    EVENT_DEF(APP_MN_DELETE_ALL_SMS_CNF),
    EVENT_DEF(APP_MN_SET_ASSERT_SMS_DESTADDR_CNF),
    EVENT_DEF(APP_MN_SET_SMS_RECEIVE_FUNC_CNF),
    EVENT_DEF(APP_MN_VMW_FALG_IND),
    EVENT_DEF(APP_MN_GET_VMW_FLAG_CNF),
    EVENT_DEF(APP_MN_SET_VMW_FLAG_CNF),
    EVENT_DEF(APP_MN_SET_VMW_ADDR_CNF),
    EVENT_DEF(APP_MN_GET_VMW_ADDR_CNF),
    EVENT_DEF(APP_MN_SMS_SIGNAL_END),

    /* be used for smscb service */
    EVENT_DEF(APP_MN_SMSCB_SIGNAL_START),
    EVENT_DEF(APP_MN_SMSCB_MACRO_MSG_IND),
    EVENT_DEF(APP_MN_SMSCB_REJ_IND),
    #ifdef _ATC_WAIT_L4
    EVENT_DEF(APP_MN_SMSCB_MSG_PAGES_IND),
    #endif
    EVENT_DEF(APP_MN_SMSCB_SIGNAL_END),

    /* be used for ss service */
    EVENT_DEF(APP_MN_SS_SIGNAL_START),
    EVENT_DEF(APP_MN_GET_PASSWORD_IND),
    EVENT_DEF(APP_MN_USSD_IND),
    EVENT_DEF(APP_MN_USSD_NOTIFY_IND),
    EVENT_DEF(APP_MN_RELEASE_SS_IND),
    EVENT_DEF(APP_MN_BEGIN_REGISTER_SS),
    EVENT_DEF(APP_MN_BEGIN_ERASE_SS),
    EVENT_DEF(APP_MN_BEGIN_ACTIVATE_SS),
    EVENT_DEF(APP_MN_BEGIN_DEACTIVATE_SS),
    EVENT_DEF(APP_MN_BEGIN_INTERROGATE_SS),
    EVENT_DEF(APP_MN_BEGIN_REGISTER_PASSWORD),
    EVENT_DEF(APP_MN_BEGIN_USSD_SERVICE),
    EVENT_DEF(APP_MN_REGISTER_SS_CNF),
    EVENT_DEF(APP_MN_ERASE_SS_CNF),
    EVENT_DEF(APP_MN_ACTIVATE_SS_CNF),
    EVENT_DEF(APP_MN_DEACTIVATE_SS_CNF),
    EVENT_DEF(APP_MN_INTERROGATE_CNF),
    EVENT_DEF(APP_MN_REGISTER_PASSWORD_CNF),
    EVENT_DEF(APP_MN_SS_BUSYING_IND),
    EVENT_DEF(APP_MN_USSD_SERVICE_CNF),
    EVENT_DEF(APP_MN_SS_SIGNAL_END),

    /* be used for stk service */
    EVENT_DEF(APP_MN_STK_SIGNAL_START),
    EVENT_DEF(SIMAT_SETUP_EVENT_LIST_REQ),
    EVENT_DEF(APP_MN_SIMAT_REFRESH_IND),
    EVENT_DEF(APP_MN_SIMAT_DISPLAY_TEXT_IND),
    EVENT_DEF(APP_MN_SIMAT_GET_INKEY_IND),
    EVENT_DEF(APP_MN_SIMAT_GET_INPUT_IND),
    EVENT_DEF(APP_MN_SIMAT_PLAY_TONE_IND),
    EVENT_DEF(APP_MN_SIMAT_SETUP_MENU_IND),
    EVENT_DEF(APP_MN_SIMAT_SELECT_ITEM_IND),
    EVENT_DEF(SIMAT_GET_DT_TZ_REQ),
    EVENT_DEF(SIMAT_GET_LANG_REQ),
    EVENT_DEF(APP_MN_SIMAT_SETUP_IDLE_TEXT_IND),
    EVENT_DEF(APP_MN_SIMAT_LANG_NOTIFY_IND),
    EVENT_DEF(APP_MN_SIMAT_SEND_SMS_IND),
    EVENT_DEF(SIMAT_SMS_PP_RSP),
    EVENT_DEF(SIMAT_SMS_CB_RSP),
    EVENT_DEF(APP_MN_SIMAT_SMS_CTL_TO_MMI_IND),
    EVENT_DEF(APP_MN_SIMAT_SEND_SS_IND),
    EVENT_DEF(APP_MN_SIMAT_SEND_USSD_IND),
    EVENT_DEF(SIMAT_MO_SS_CONTROL_RSP),
    EVENT_DEF(SIMAT_MO_USSD_CONTROL_RSP),
    EVENT_DEF(APP_MN_SIMAT_SETUP_CALL_IND),
    EVENT_DEF(APP_MN_SIMAT_SEND_DTMF_IND),
    EVENT_DEF(SIMAT_MO_CC_RSP),
    EVENT_DEF(APP_MN_SIMAT_CC_TO_MMI_IND),
    EVENT_DEF(SIMAT_BC_SMS_RSP),
    EVENT_DEF(SIMAT_PP_SMS_RSP),
    EVENT_DEF(APP_MN_SIMAT_PRO_LOCAL_INFO_IND),
    EVENT_DEF(APP_MN_SIMAT_MORE_TIME_IND),
    EVENT_DEF(APP_MN_SIMAT_REFRESH_CNF),
    EVENT_DEF(SIMAT_SESSION_TERMINATED_IND),
    EVENT_DEF(APP_MN_SIMAT_SEND_DATA_IND),
    EVENT_DEF(APP_MN_SIMAT_RECEIVE_DATA_IND),
    EVENT_DEF(APP_MN_SIMAT_LAUNCH_BROWSER_IND),
    EVENT_DEF(APP_SIMAT_OPEN_CHANNEL_IND),
    EVENT_DEF(APP_SIMAT_CLOSE_CHANNEL_IND),
    EVENT_DEF(APP_SIMAT_GET_CHANNEL_STATUS_IND),
    EVENT_DEF(APP_MN_SIMAT_DISPLAY_MULTIMEDIA_MSG_IND),
    EVENT_DEF(APP_MN_SIMAT_RETRIEVE_MULTIMEDIA_MSG_IND),
    EVENT_DEF(APP_MN_SIMAT_SUBMIT_MULTIMEDIA_MSG_IND),
    EVENT_DEF(APP_MN_STK_SIGNAL_END),

    /* be used for diagnositc service */
    EVENT_DEF(APP_L1_SIGNAL_START),
    EVENT_DEF(APP_L1_GET_AFC_CNF),
    EVENT_DEF(APP_L1_GET_PA_CNF),
    EVENT_DEF(APP_L1_GET_AGC_CNF),
    EVENT_DEF(APP_L1_GET_BLOCK_ERROR_RATIO_CNF),
    EVENT_DEF(APP_MN_READ_DSP_MEM_CNF),
    EVENT_DEF(APP_L1_SIGNAL_END),

    /* be used for mp4 */
    EVENT_DEF(APP_MP4_DISPLAY_IND),

    /* be used for sd card re init */
    EVENT_DEF(APP_SD_REINIT_IND),
};
/******************************************************************************/
// Description : This function to get event name
// Global resource dependence :
// Author : Pizer Fan
// Note :
/******************************************************************************/
const char *ATC_Get_Event_Name( uint16 event );
#endif /*_DEBUG*/
#endif /*ATC_SAT_ENABLE*/

/******************************************************************************/
// Description : This function is used to check call expect event when recv call disconnect
//               called when atc recv call disconect or call error ind
// Global resource dependence : None
// Author : 
// Note : None
/******************************************************************************/
LOCAL void CheckCallEvent(MN_DUAL_SYS_E dual_sys, uint8 call_id);

#ifdef ATC_SAT_ENABLE
/*****************************************************************************/
//  Description : For AT+IPSEND, 
//  when inputing specified length data, appending 0x1a to send out
//  Global resource dependence : none
//  Author: longting.zhao
//  Note: 
/*****************************************************************************/
LOCAL BOOLEAN ATC_CheckIpSendDataByLen(uint8 * nchar, uint8 link_id);
#endif /*ATC_SAT_ENABLE*/
#endif /*_MUX_ENABLE_*/


#ifndef ATC_SAT_ENABLE
static SCI_TIMER_PTR s_atc_sleep_timer = PNULL;   //the timer to handle the atc sleep time-out
#define ATC_SLEEP_TIMER_VALUE       10000       //the value of the timer to wait for the response 
#endif

#define ATC_PS_ATH_TIMER_VALUE      40000       //the value of the timer to wait for the response 
#define ATC_PS_ATH_AVOID_REP_TIMER_VALUE        3000

/* if disconnect modem is received and ath received at almost the same time,
 * then the ath command will be dropped. */
ATC_LINK_INST_T atc_link_inst[ATC_MAX_LINK_ID] = {0};

#ifndef ATC_SAT_ENABLE
/**********************************************************************
//    Description:
//      the funciton to enable sleep flag after timer expire
//    Global resource dependence :
//    Author:bo.chen
//    Note:
***********************************************************************/
static void AtcSleepTimerExpired(
    uint32 lparam   //the function's input parameter
);

/**********************************************************************
//    Description:
//      the funciton to restart atc sleep timer
//    Global resource dependence :
//    Author:bo.chen
//    Note:
***********************************************************************/
static void AtcReStartSleepTimer(
    uint32  timer_value
);
#endif

/**********************************************************************
//    Description:
//      the funciton to protect ATH when ps disconnecting after timer expire
//    Global resource dependence :
//    Author: Pizer.Fan
//    Note:
***********************************************************************/
LOCAL void ATC_Link_ATHWaitTimerTimeout(
    uint32 lparam   //the function's input parameter
);

/**********************************************************************
//    Description:
//      the funciton to protect ATH when ps disconnecting after timer expire
//    Global resource dependence :
//    Author: Pizer.Fan
//    Note:
***********************************************************************/
LOCAL void ATC_Link_ATHAvoidRepTimeout(
    uint32 lparam   //the function's input parameter
);

#ifdef _MODEM_MODE
extern void SCI_ATC_EnableDeepSleep(uint32 mode);

/*************************************************************************/
//    Description: get s32k status
//    Global resource :
//    Author: bo.chen
//    Note:  20051203 in atc_gsm.c
/*************************************************************************/
extern BOOLEAN ATC_SleepIsAllowed(
    void
);
#endif

#ifndef _MODEM_MODE
/*****************************************************************************/
//  Description : This function is the callback of CHR
//  Global resource dependence :
//  Author:       Xueli
//  Note:
/*****************************************************************************/
LOCAL void ATC_ChrCallBack(uint32 taskID, uint32 argc, void *argv);
#endif

/******************************************************************************/
// Description : This function do some initialization work for ATC
// Global resource dependence : g_atc_info_table
//                              g_atc_handle_table
//                              g_atc_result_code_text
// Author : Steven
// Note : This function should be called first on starting ATC task
/******************************************************************************/
LOCAL ATC_STATUS ATC_Initialize(void);  

/******************************************************************************/
// Description : This function reads a new AT command line from sio unit and
//               parses it
// Global resource dependence :
// Author : Steven
// Note :   Modify: Xueli
/******************************************************************************/
LOCAL ATC_STATUS ATC_RecNewLineSig(  // Return S_ATC_SUCCESS if success,
    // otherwise return error code.
    ATC_GLOBAL_INFO_T      *atc_global_info_ptr, // point to the ATC_GLOBAL_INFO_T struct.
    const xSignalHeaderRec *sig_ptr         // point to signal struct
);
/******************************************************************************/
// Description : This function processes the AT commands stored in the AT
//               config struct one by one.
// Global resource dependence : None
// Author : Steven
// Note : None
/******************************************************************************/
LOCAL ATC_STATUS ATC_ProcessCmds( // Return S_ATC_SUCCESS if success,
    // otherwise return error code.
    ATC_GLOBAL_INFO_T     *atc_global_info_ptr // point to the ATC_GLOBAL_INFO_T struct.
);
/******************************************************************************/
// Description : This function despatches AT command to its handle.
// Global resource dependence :
// Author : Steven
// Note : None
/******************************************************************************/
LOCAL ATC_STATUS ATC_DespatchCmd(  // Return S_ATC_STATUS if success,
    // otherwise return error code.
    ATC_GLOBAL_INFO_T   *atc_global_info_ptr, // point to the ATC_GLOBAL_INFO_T struct.
    ATC_CUR_INFO_LIST_T *atc_cur_info_ptr // Point to the info struct of
    // AT command
);
/******************************************************************************/
// Description : This function checks the current received AT command.
// Global resource dependence : None
// Author : Steven
// Note : None
/******************************************************************************/
LOCAL ATC_STATUS ATC_CheckCmd(  // Return S_ATC_SUCCESS if command is valid,
    // otherwise return error code.
    ATC_GLOBAL_INFO_T   *atc_global_info_ptr, // point to the ATC_GLOBAL_INFO_T struct.
    ATC_CUR_INFO_LIST_T *atc_cur_info_ptr  // Point to the info struct of
    // AT command.
);
/******************************************************************************/
// Description : This function builds test command response.
// Global resource dependence : None
// Author : Steven
// Note : None
/******************************************************************************/
LOCAL ATC_STATUS ATC_BuildTestCmdResponse(  // Return S_ATC_SUCCESS if success,
    // otherwise return error code.
    ATC_GLOBAL_INFO_T    *atc_global_info_ptr, // point to the ATC_GLOBAL_INFO_T struct.
    ATC_CUR_INFO_LIST_T *atc_cur_info_ptr  // Point the info struct of cmd
);

/******************************************************************************/
// Description : This function get the error string
// Global resource dependence : None
// Author : Ivan
// Note : None
// History:
/******************************************************************************/
LOCAL void ATC_GetErrVerbose(
    int32 err_idx,
    char *err_verbose
);
/******************************************************************************/
// Description : register the event to MN
// Global resource dependence : None
// Author : Ivan
// Note : None
/******************************************************************************/
LOCAL void ATC_RegisterEvent(void);

/******************************************************************************/
// Description :  This function is used to start PS when power on and ATC_SAT_ENABLE 
//                not defined
// Global resource dependence : None
// Author : yaxun.chen
// Note : None
/******************************************************************************/
LOCAL void ATC_InitAutoStartPSNoSat(void);
// Description : identify if the message is the needed msg on pending state
// Global resource dependence : None
// Author : Ivan
// Note : None
/******************************************************************************/
ATC_STATUS ATC_IdentifyMsg( // the identify result
    int msg_type,
    ATC_GLOBAL_INFO_T *atc_global_info_ptr  // point to the ATC_GLOBAL_INFO_T struct
);

#ifdef _MUX_ENABLE_
LOCAL int ATC_Ring_Space(ATC_BUF_T *pRing);
LOCAL int ATC_Ring_Add(ATC_BUF_T *pRing, int ch);
LOCAL int ATC_Ring_Remove(ATC_BUF_T *pRing);
#endif

/******************************************************************************/
// Description : This function reset dtmf info: include dtmf global variable and timer
// Global resource dependence : None
// Author : minqian.qian
// Note : None
/******************************************************************************/
LOCAL void ATC_ResetDTMFInfo(ATC_CONFIG_T *atc_config_ptr, MN_DUAL_SYS_E   dual_sys);

extern void ATC_Task_Dispatch(xSignalHeaderRec *sig_ptr);

#ifdef ATC_SAT_ENABLE
void ATC_InitAutoStartPS();
#endif
/*******************************************************************************************/
/*****************************************************************************/
//  Description : return this flag for ath in atc_gsm.c
//  Global resource dependence : none
//  Author:       bo.chen
//  Note:
/*****************************************************************************/
BOOLEAN ATC_IsPsActivate(void)
{
    return s_ps_service_is_activate;
}

void ATC_SetPsActivateStatus(BOOLEAN is_activate)
{
    /* if ps is activate increase the arm clk for ps */
    if(s_ps_service_is_activate != is_activate)
    {
        s_ps_service_is_activate = is_activate;
    }
}

/******************************************************************************/
// Description : write the all item which need in ATC
// Global resource dependence : None
// Author : hanjun
// Note :
/******************************************************************************/
BOOLEAN ATC_WriteNVItem(
    ATC_CONFIG_T        *atc_config_ptr,
    uint16 ItemID
)
{
    SCI_ASSERT(NULL != atc_config_ptr); /*assert verified*/
    //write the NV.
    ATC_TRACE_LOW("ATC: ATC_WriteNVItem, write nv itemid: %d", ItemID);

    switch(ItemID)
    {
        case NV_ATC_CONFIG_ID:          //write atc config item

            if(NVERR_NONE != EFS_NvitemWrite(ItemID,  sizeof(ATC_CONFIG_NV_T),
                                             (uint8 *)&atc_config_ptr->atc_config_nv, FALSE))
            {
                ATC_TRACE_LOW("ATC: ATC_WriteNVItem, write NV_ATC_CONFIG_ID failed.");
                return FALSE;
            }

            break;

        default:
            return FALSE;
    }

    return TRUE;
}
/******************************************************************************/
// Description : iread the all item which need in ATC.
// Global resource dependence :
// Author : hanjun
// Note : None
/******************************************************************************/

LOCAL BOOLEAN ATC_ReadNVItem(
    ATC_CONFIG_T        *atc_config_ptr
)
{
    BOOLEAN re_value = TRUE;
    SCI_ASSERT(NULL != atc_config_ptr); /*assert verified*/

    if(NVERR_NONE != EFS_NvitemRead(NV_ATC_CONFIG_ID,
                                    sizeof(ATC_CONFIG_NV_T), (uint8 *)&atc_config_ptr->atc_config_nv))
        //if fail in reading,initiate all the variable
    {
        atc_config_ptr->atc_config_nv.creg              = DEF_CREG;
        atc_config_ptr->atc_config_nv.sidet_flag        = FALSE;
        atc_config_ptr->atc_config_nv.clip_flag         = TRUE;
        atc_config_ptr->atc_config_nv.clir_flag         = FALSE;
        atc_config_ptr->atc_config_nv.colp_flag         = FALSE;
        atc_config_ptr->atc_config_nv.cmgf_flag         = FALSE;
        atc_config_ptr->atc_config_nv.call_ring_index   = 0;
        atc_config_ptr->atc_config_nv.mux_mode          = FALSE;
        atc_config_ptr->atc_config_nv.cmee              = NUMERIC_ERR_VALUE_MODE;
        atc_config_ptr->atc_config_nv.e_value           = FALSE;
        atc_config_ptr->atc_config_nv.battery_indicator = FALSE;
        atc_config_ptr->atc_config_nv.stk_actflag       = TRUE;
        atc_config_ptr->atc_config_nv.stk_time          = ATC_STKTIME_DEFAULT_VAL;
        atc_config_ptr->atc_config_nv.wind_ind_level    = ATC_SIND_DEFAULT_VAL;
        atc_config_ptr->atc_config_nv.spauto_flag        = DEF_SPAUTO;
        atc_config_ptr->atc_config_nv.earphone_vol      = DEF_EARPHONE_VOL;

        ATC_TRACE_LOW("ATC: read atc_config_nv failed");
        re_value = FALSE;

        //write the NV.
        ATC_WriteNVItem(atc_config_ptr, NV_ATC_CONFIG_ID);
    }


    return re_value;
}

/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  *
 **     void ATC_SetEcho()                                                *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     Tell CMUX to echo the char it receives                                 *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     None                                                                  *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **---------------------------------------------------------------------------*/
void ATC_SetEcho(BOOLEAN IsEcho)
{
#ifdef _MUX_ENABLE_
    atc_is_echo = IsEcho;
#else
    s_config_info.atc_config_nv.e_value = IsEcho;

    if(0 == s_config_info.atc_config_nv.e_value)
    {
        SIO_ATC_SetEcho(FALSE);
    }
    else
    {
        s_config_info.atc_config_nv.e_value = TRUE;
        SIO_ATC_SetEcho(TRUE);
    }
#endif
}

void ATC_RegisterExtHandler(uint32 *callback)
{
}

#ifdef _MUX_ENABLE_
/******************************************************************************/
// Description : This function setup link for one ATC entity
// Global resource dependence :
// Author : Wang Hui
// Note :
/******************************************************************************/
void ATC_Mux_Setup_Link(uint8 link_id)
{
    MN_DUAL_SYS_E dual_sys = ATC_GetSimIdFromLinkId(link_id);

    if((0 == link_id) || (ATC_DEBUG_LINK == link_id))
    {
        ATC_TRACE_LOW("ATC: ATC_Mux_Setup_Link, WARNING! link_id:%d", link_id);
        return;
    }

    if(atc_buffer[dual_sys].atc_buf_ptr != PNULL)
    {
        ATC_TRACE_LOW("ATC: ATC_Mux_Setup_Link, WARNING! buffer%d is already allocated", dual_sys);
        return;
    }

    atc_buffer[dual_sys].atc_buf_ptr = (uint8 *)SCI_ALLOC_BASE(ATC_REC_BUF_SIZE);

    if(!atc_buffer[dual_sys].atc_buf_ptr)
    {
        ATC_TRACE_LOW("ATC: ATC_Mux_Setup_Link, ERROR! no mem to setup buffer%d", dual_sys);
        return;
    }

    atc_buffer[dual_sys].size        = ATC_REC_BUF_SIZE;
    atc_buffer[dual_sys].start_point = 0;
    atc_buffer[dual_sys].end_point   = 0;
}

/******************************************************************************/
// Description : This function create link for one ATC entity
// Global resource dependence :
// Author : Wang Hui
// Note :
/******************************************************************************/
void ATC_Mux_Release_Link(uint8 link_id, BOOLEAN free_flg)
{
    MN_DUAL_SYS_E dual_sys = ATC_GetSimIdFromLinkId(link_id);
    uint8 call_index = 0;

    if((0 == link_id) || (ATC_DEBUG_LINK == link_id))
    {
        return;
    }

    for(call_index = 0; call_index < ATC_MAX_ACTIVATE_PDP; call_index++)
    {
        if((s_config_info.atc_psd_call_info[call_index].link_id == link_id) &&
            (s_config_info.atc_psd_call_info[call_index].cid != ATC_INVALID_CID) &&
            (s_config_info.atc_psd_call_info[call_index].state == ATC_PSD_STATE_ACTIVATE_PENDING ||
             s_config_info.atc_psd_call_info[call_index].state == ATC_PSD_STATE_ACTIVE ||
             s_config_info.atc_psd_call_info[call_index].state == ATC_PSD_STATE_MODIFY_PENDING)
        )
        {
            /* directly diconnect PPP as MUX been released */
            ppp_disconnect_for_mux_release(s_config_info.atc_psd_call_info[call_index].data_link_id);
            ATC_DisconnectModem(s_config_info.atc_psd_call_info[call_index].cid);
        }
    }

    /* NOTE: if free_flg is FALSE, nothing to do!!! */
    if(free_flg)
    {
        if (atc_buffer[dual_sys].atc_buf_ptr != PNULL)

        {
            SCI_FREE(atc_buffer[dual_sys].atc_buf_ptr);
            atc_buffer[dual_sys].atc_buf_ptr = PNULL;
        }
        atc_buffer[dual_sys].size        = 0;
        atc_buffer[dual_sys].start_point = 0;
        atc_buffer[dual_sys].end_point   = 0;
    }
}

/******************************************************************************/
// Description : This function set the current link id
// Global resource dependence :
// Author : Wang Hui
// Note :
/******************************************************************************/
void ATC_Set_Current_Link_id(ATC_CONFIG_T *atc_config_ptr, uint8 link_id)
{
    atc_config_ptr->current_link_id = link_id;
}

/******************************************************************************/
// Description : This function clear expected result info
// Global resource dependence :
// Author : Wang Hui
// Note :
/******************************************************************************/
void ATC_Clear_Exp_Result(ATC_EXP_RESULT_INFO_T  *atc_exp_result_info)
{
    atc_exp_result_info->link_id = ATC_INVALID_LINK_ID;
    atc_exp_result_info->exp_event = ATC_NO_EVENT;
    atc_exp_result_info->ext_exp_event = ATC_NO_EVENT;
    atc_exp_result_info->call_id = ATC_INVALID_CALL_ID;
    atc_exp_result_info->cid = ATC_INVALID_CID;
    atc_exp_result_info->dual_sys= MN_DUAL_SYS_1;
}

/******************************************************************************/
// Description : This function handle the received the expected event and set the related link id
// Global resource dependence :
// Author : Wang Hui
// Note :
/******************************************************************************/
BOOLEAN ATC_Receive_Expected_Event(
    ATC_CONFIG_T         *atc_config_ptr,
    uint16               expected_event,
    uint16               id,
    ATC_EVENT_DOMAIN_T   domain_id,
    MN_DUAL_SYS_E    dual_sys
)
{
    BOOLEAN     ret_val = FALSE;
    uint16      start_p = 0;
    uint16      end_p   = 0;
    uint8       link_id = (uint8)ATC_INVALID_LINK_ID;

    start_p = atc_config_ptr->atc_exp_link_info.start_point;
    end_p   = atc_config_ptr->atc_exp_link_info.end_point;

    SCI_ASSERT(end_p < ATC_MAX_EXP_RESULT);/*assert verified: check valid value*/
    SCI_ASSERT(start_p < ATC_MAX_EXP_RESULT);/*assert verified: check valid value*/

    ATC_TRACE_LOW("ATC: Recv_Exp_Evnt, EVENT:%d, ID:%d, SIM ID: %d",
            expected_event, id, dual_sys);

    switch(expected_event)
    {
        case APP_MN_ACTIVATE_PDP_CONTEXT_CNF:
        case APP_MN_DEACTIVATE_PDP_CONTEXT_CNF:

            while(start_p != end_p)
            {
                if((atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].exp_event == expected_event)
                    && (atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].cid == (uint8)id)
                    && (atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].dual_sys == dual_sys))
                {
                    atc_config_ptr->current_link_id = atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].link_id;
                    ATC_Clear_Exp_Result(&atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p]);
                    ret_val = TRUE;
                    break;
                }

                start_p++;

                if(ATC_MAX_EXP_RESULT == start_p)
                {
                    start_p = 0;
                }
            }

            break;

            /* NOTE: no cid field in APP_MN_GPRS_T !!!
             * DONT care cid for APP_MN_SET_QOS_CNF & APP_MN_SET_MIN_QOS_CNF !!! */
        case APP_MN_SET_QOS_CNF:
        case APP_MN_SET_MIN_QOS_CNF:

            while(start_p != end_p)
            {
                if((atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].exp_event == expected_event)
                    && (atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].dual_sys == dual_sys))
                {
                    atc_config_ptr->current_link_id = atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].link_id;
                    ATC_Clear_Exp_Result(&atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p]);
                    ret_val = TRUE;
                    break;
                }

                start_p++;

                if(ATC_MAX_EXP_RESULT == start_p)
                {
                    start_p = 0;
                }
            }

            break;

        case APP_MN_READ_DSP_MEM_CNF:
        case APP_MN_DEACTIVE_PS_CNF:

        case APP_MN_READ_SMS_CNF:
        case APP_MN_SEND_SMS_CNF:
        case APP_MN_WRITE_SMS_CNF:
        case APP_MN_UPDATE_SMS_STATE_CNF:
        case APP_MN_DELETE_ALL_SMS_CNF:

        case APP_MN_SIM_PIN_FUNC_CNF:
        case APP_MN_PLMN_SELECT_CNF:
        case APP_MN_PLMN_LIST_CNF:
        case APP_MN_PLMN_READ_CNF:
        case APP_MN_PLMN_WRITE_CNF:
        case APP_MN_SET_BAND_CNF:
        case APP_MN_PS_POWER_OFF_CNF:
        case APP_MN_PS_POWER_ON_CNF:

        case APP_MN_CRSM_CNF:

            while(start_p != end_p)
            {
                if((atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].exp_event == expected_event)
                    && (atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].dual_sys == dual_sys))
                {
                    atc_config_ptr->current_link_id = atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].link_id;
                    ATC_Clear_Exp_Result(&atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p]);
                    ret_val = TRUE;
                    break;
                }

                start_p++;

                if(ATC_MAX_EXP_RESULT == start_p)
                {
                    start_p = 0;
                }
            }

            break;

        case APP_MN_INTERROGATE_CNF:
        case APP_MN_DEACTIVATE_SS_CNF:
        case APP_MN_ACTIVATE_SS_CNF:
        case APP_MN_ERASE_SS_CNF:
        case APP_MN_REGISTER_SS_CNF:
        case APP_MN_REGISTER_PASSWORD_CNF:
#ifdef _SUPPORT_CCBS_
        case APP_MN_CCBS_ERASE_CNF:
#endif /*_SUPPORT_CCBS_*/ 
            while(start_p != end_p)
            {
                if(atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].dual_sys == dual_sys)
                {
                    expected_event = atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].exp_event;
                    switch (expected_event)
                    {
                        case APP_MN_REGISTER_SS_CNF:
                        case APP_MN_ERASE_SS_CNF:
                        case APP_MN_ACTIVATE_SS_CNF:
                        case APP_MN_DEACTIVATE_SS_CNF:
                        case APP_MN_INTERROGATE_CNF:
                        case APP_MN_REGISTER_PASSWORD_CNF:
                    #ifdef _SUPPORT_CCBS_
                        case APP_MN_CCBS_ERASE_CNF:
                    #endif /*_SUPPORT_CCBS_*/
                            atc_config_ptr->current_link_id = atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].link_id;
                            ATC_Clear_Exp_Result(&atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p]);
                            ret_val = TRUE;
                            break;

                        default :
                            break;
                    }

                    if (TRUE == ret_val)
                    {
                        break;
                    }
                }

                start_p++;

                if(ATC_MAX_EXP_RESULT == start_p)
                {
                    start_p = 0;
                }
            }
            break;

        case APP_MN_RELEASE_SS_IND:

            while(start_p != end_p)
            {
                if(atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].dual_sys == dual_sys)
                {
                    expected_event = atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].exp_event;
                    switch (expected_event)
                    {
                        case APP_MN_REGISTER_SS_CNF:
                        case APP_MN_ERASE_SS_CNF:
                        case APP_MN_ACTIVATE_SS_CNF:
                        case APP_MN_DEACTIVATE_SS_CNF:
                        case APP_MN_INTERROGATE_CNF:
                        case APP_MN_REGISTER_PASSWORD_CNF:
                            atc_config_ptr->current_link_id = atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].link_id;
                            ATC_Clear_Exp_Result(&atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p]);
                            ret_val = TRUE;
                            break;

                        default :
                            break;
                    }

                    if (TRUE == ret_val)
                    {
                        break;
                    }
                }

                start_p++;

                if(ATC_MAX_EXP_RESULT == start_p)
                {
                    start_p = 0;
                }
            }

            break;

        case APP_MN_CALL_START_IND:
            while(start_p != end_p)
            {
                if((atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].exp_event == expected_event)
                    && (atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].dual_sys == dual_sys)) 
                {
                    atc_config_ptr->current_link_id = atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].link_id;
                    ATC_Clear_Exp_Result(&atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p]);
                    ret_val = TRUE;
                    break;
                }

                start_p++;

                if(ATC_MAX_EXP_RESULT == start_p)
                {
                    start_p = 0;
                }
            }

            break;

        case APP_MN_CALL_HELD_CNF:
        case APP_MN_BUILD_MPTY_CNF:
        case APP_MN_SPLIT_MPTY_CNF:
        case APP_MN_ECT_INVOKE_CNF:
        case APP_MN_CALL_RETRIEVED_CNF:
        case APP_MN_SETUP_COMPLETE_IND:

            while(start_p != end_p)
            {
                if((atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].exp_event == expected_event) 
                   && (atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].call_id == id)
                   && (atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].dual_sys == dual_sys))
                {
                    atc_config_ptr->current_link_id = atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].link_id;
                    ATC_Clear_Exp_Result(&atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p]);
                    ret_val = TRUE;
                    break;
                }

                start_p++;

                if(ATC_MAX_EXP_RESULT == start_p)
                {
                    start_p = 0;
                }
            }

            break;

        case APP_MN_CALL_DISCONNECTED_IND:
        case APP_MN_CALL_ERR_IND:
            expected_event = APP_MN_CALL_DISCONNECTED_IND;

            /*In order to find the corresponding and correct expected event, do as follows:
            First, check with exp_event parameter
            Second, check with ext_exp_event parameter*/
            while(start_p != end_p)
            {
                if((atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].exp_event == expected_event)
                    && (atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].call_id == id)
                    && (atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].dual_sys == dual_sys))
                {
                    atc_config_ptr->current_link_id = atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].link_id;
                    ATC_Clear_Exp_Result(&atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p]);
                    ret_val = TRUE;
                    break;
                }

                start_p++;

                if(ATC_MAX_EXP_RESULT == start_p)
                {
                    start_p = 0;
                }
            }

            if(!ret_val)
            {
                while(start_p != end_p)
                {
                    if((atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].ext_exp_event == expected_event)
                        && (atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].call_id == id)
                        && (atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].dual_sys == dual_sys))
                    {
                        atc_config_ptr->current_link_id = atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].link_id;
                        ATC_Clear_Exp_Result(&atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p]);
                        ret_val = TRUE;
                        break;
                    }

                    start_p++;

                    if(ATC_MAX_EXP_RESULT == start_p)
                    {
                        start_p = 0;
                    }
                }
            }

            atc_config_ptr->call_link_id[id] = 0xFF;
            break;

        case APP_MN_START_DTMF_CNF:
        case APP_MN_STOP_DTMF_CNF:

            while(start_p != end_p)
            {
                if((atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].exp_event == expected_event) 
                   && (atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].call_id == id)
                   && (atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].dual_sys == dual_sys))
                {
                    atc_config_ptr->current_link_id = atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].link_id;
                    ATC_Clear_Exp_Result(&atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p]);
                    ret_val = TRUE;
                    break;
                }

                start_p++;

                if(ATC_MAX_EXP_RESULT == start_p)
                {
                    start_p = 0;
                }
            }

            break;

        case APP_MN_FDN_UPDATE_CNF:
        case APP_MN_ADN_UPDATE_CNF:
            expected_event = APP_MN_ADN_UPDATE_CNF;

            while(start_p != end_p)
            {
                if((atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].exp_event == expected_event) 
                   && (atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].call_id == id)
                   && (atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].dual_sys == dual_sys))
                {
                    atc_config_ptr->current_link_id = atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].link_id;
                    ATC_Clear_Exp_Result(&atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p]);
                    ret_val = TRUE;
                    break;
                }

                start_p++;

                if(ATC_MAX_EXP_RESULT == start_p)
                {
                    start_p = 0;
                }
            }

            break;

        default:

            /* Try to match expect event anyway! */
            while(start_p != end_p)
            {
                if((atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].exp_event == expected_event)
                    && (atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].dual_sys == dual_sys))
                {
                    atc_config_ptr->current_link_id = atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].link_id;
                    ATC_Clear_Exp_Result(&atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p]);
                    ret_val = TRUE;
                    break;
                }

                start_p++;

                if(ATC_MAX_EXP_RESULT == start_p)
                {
                    start_p = 0;
                }
            }

            /* NOT FOUND!!! */
            break;
    }

    link_id = ATC_Get_Link_Id(id, domain_id, dual_sys);

    if(ret_val == FALSE)
    {
        if(link_id != (uint8)ATC_INVALID_LINK_ID)
        {
            ATC_TRACE_LOW("ATC: Recv_Exp_Evnt, WARNING! NOT FOUND, set link id: %d", link_id);
            atc_config_ptr->current_link_id = link_id;
        }
        else
        {
            atc_config_ptr->current_link_id = ATC_GetMuxLinkIdFromSimId(dual_sys);
            ATC_TRACE_LOW("ATC: Recv_Exp_Evnt, WARNING! NOT FOUND, set proactive link id: %d", atc_config_ptr->current_link_id);
        }
    }
    else
    {
        if((atc_config_ptr->current_link_id != link_id) && (domain_id == ATC_DOMAIN_PS))
        {
            /* non-data-mode pdp activation, no add PSD info, so */
            if((ATC_INVALID_LINK_ID != link_id))
            {
                ATC_TRACE_LOW("ATC: Recv_Exp_Evnt, FOUND, WARNING! exp_link_id:%d != data_link_id:%d",
                              atc_config_ptr->current_link_id, link_id);
            }
            else
            {
                ATC_TRACE_LOW("ATC: Recv_Exp_Evnt, FOUND, WARNING! exp_link_id:%d, no data_link_id",
                              atc_config_ptr->current_link_id);
            }
        }
        else
        {
            if((ATC_INVALID_LINK_ID != link_id))
            {
                ATC_TRACE_LOW("ATC: Recv_Exp_Evnt, FOUND, exp_link_id:%d, data_link_id:%d",
                              atc_config_ptr->current_link_id, link_id);
            }
            else
            {
                ATC_TRACE_LOW("ATC: Recv_Exp_Evnt, FOUND, exp_link_id:%d",
                              atc_config_ptr->current_link_id);
            }
        }
    }

    if(ret_val)
    {
        ATC_Shrink_Expected_Event_Buffer(atc_config_ptr);
    }

    ATC_List_Expected_Event();
    return ret_val;
}

/******************************************************************************/
// Description : This function add the expected event when requested the expected event
// Global resource dependence :
// Author : Wang Hui
// Note :
/******************************************************************************/
void ATC_Add_Expected_Event(
    ATC_CONFIG_T *atc_config_ptr,
    uint16 expected_event,
    uint16 id,
    ATC_EVENT_DOMAIN_T domain_id,
    MN_DUAL_SYS_E    dual_sys
)
{
    SCI_ASSERT(atc_config_ptr->atc_exp_link_info.end_point < ATC_MAX_EXP_RESULT);/*assert verified: check valid value*/
    SCI_ASSERT(atc_config_ptr->atc_exp_link_info.start_point < ATC_MAX_EXP_RESULT);/*assert verified: check valid value*/

    if(domain_id == ATC_NO_DOMAIN)
    {
        atc_config_ptr->atc_exp_link_info.atc_exp_result_info[atc_config_ptr->atc_exp_link_info.end_point].exp_event = expected_event;
        atc_config_ptr->atc_exp_link_info.atc_exp_result_info[atc_config_ptr->atc_exp_link_info.end_point].link_id = atc_config_ptr->current_link_id;
        atc_config_ptr->atc_exp_link_info.atc_exp_result_info[atc_config_ptr->atc_exp_link_info.end_point].dual_sys= dual_sys;
    }
    else if(domain_id == ATC_DOMAIN_CS ||
            domain_id == ATC_DOMAIN_PHONEBOOK)
    {
        atc_config_ptr->atc_exp_link_info.atc_exp_result_info[atc_config_ptr->atc_exp_link_info.end_point].exp_event = expected_event;
        atc_config_ptr->atc_exp_link_info.atc_exp_result_info[atc_config_ptr->atc_exp_link_info.end_point].call_id = id;
        atc_config_ptr->atc_exp_link_info.atc_exp_result_info[atc_config_ptr->atc_exp_link_info.end_point].link_id = atc_config_ptr->current_link_id;
        atc_config_ptr->atc_exp_link_info.atc_exp_result_info[atc_config_ptr->atc_exp_link_info.end_point].dual_sys= dual_sys;
    }
    else if(domain_id == ATC_DOMAIN_PS)
    {
        atc_config_ptr->atc_exp_link_info.atc_exp_result_info[atc_config_ptr->atc_exp_link_info.end_point].exp_event = expected_event;
        atc_config_ptr->atc_exp_link_info.atc_exp_result_info[atc_config_ptr->atc_exp_link_info.end_point].cid = id;
        atc_config_ptr->atc_exp_link_info.atc_exp_result_info[atc_config_ptr->atc_exp_link_info.end_point].link_id = atc_config_ptr->current_link_id;
        atc_config_ptr->atc_exp_link_info.atc_exp_result_info[atc_config_ptr->atc_exp_link_info.end_point].dual_sys= dual_sys;
    }
    else
    {
        ATC_TRACE_LOW("ATC: ATC_Add_Expected_Event, unknown domain_id: %d", domain_id);
    }

    atc_config_ptr->atc_exp_link_info.end_point++;

    if(atc_config_ptr->atc_exp_link_info.end_point >= ATC_MAX_EXP_RESULT)
    {
        atc_config_ptr->atc_exp_link_info.end_point = 0;
    }

    if(atc_config_ptr->atc_exp_link_info.end_point == atc_config_ptr->atc_exp_link_info.start_point)
    {
        uint16 empty_p;

        empty_p = atc_config_ptr->atc_exp_link_info.end_point;

        //check the buffer is full.
        while(atc_config_ptr->atc_exp_link_info.atc_exp_result_info[empty_p].exp_event != ATC_NO_EVENT)
        {
            empty_p++;

            if(empty_p >= ATC_MAX_EXP_RESULT)
            {
                empty_p = 0;
            }

            if(empty_p == atc_config_ptr->atc_exp_link_info.start_point)
            {
                ATC_TRACE_LOW("ATC: ATC_Add_Expected_Event, Warning :The atc expected event buffer is full!Remove the first one for adding!");
                ATC_Clear_Exp_Result(&atc_config_ptr->atc_exp_link_info.atc_exp_result_info[atc_config_ptr->atc_exp_link_info.start_point]);
                break;
            }
        }

        //reassign the point for add new node
        atc_config_ptr->atc_exp_link_info.end_point = empty_p;
        atc_config_ptr->atc_exp_link_info.start_point = empty_p + 1;

        if(atc_config_ptr->atc_exp_link_info.start_point >= ATC_MAX_EXP_RESULT)
        {
            atc_config_ptr->atc_exp_link_info.start_point = 0;
        }

        ATC_Shrink_Expected_Event_Buffer(atc_config_ptr);

    }

    ATC_List_Expected_Event();
}

/******************************************************************************/
// Description : This function check the expected event is existed or not
// Global resource dependence :
// Author : Pizer Fan
// Note :
/******************************************************************************/
BOOLEAN ATC_Check_Expected_Event(uint8 link_id, uint16 expected_event, uint8 cid, MN_DUAL_SYS_E dual_sys)
{
    ATC_CONFIG_T *atc_config_ptr;
    uint16        start_p, end_p;

    atc_config_ptr = &s_config_info;
    start_p        = atc_config_ptr->atc_exp_link_info.start_point;
    end_p          = atc_config_ptr->atc_exp_link_info.end_point;

    while(start_p != end_p)
    {
        if((link_id == atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].link_id)
              &&(dual_sys == atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].dual_sys)
              &&(((atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].ext_exp_event == expected_event) && (atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].cid == cid))
                    || ((atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].exp_event == expected_event) && (atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].cid == cid)))) //lint !e661
        {
            return TRUE;
        }

        start_p++;

        if(ATC_MAX_EXP_RESULT == start_p)
        {
            start_p = 0;
        }
    }

    return FALSE;
}

/******************************************************************************/
// Description : This function check the expected event is existed or not
// Global resource dependence :
// Author : minqian.qian
// Note :   unrelated to link id
/******************************************************************************/
BOOLEAN ATC_Is_Expected_Event_Present(
    uint16 expected_event,
    uint16 checkid,
    ATC_EVENT_DOMAIN_T domain_id,
    MN_DUAL_SYS_E   dual_sys
)
{
    ATC_CONFIG_T *atc_config_ptr;
    uint16        start_p, end_p;

    atc_config_ptr = &s_config_info;
    start_p        = atc_config_ptr->atc_exp_link_info.start_point;
    end_p          = atc_config_ptr->atc_exp_link_info.end_point;

    while(start_p != end_p)
    {
        if(ATC_DOMAIN_PS == domain_id)
        {
            /*check expected event with cid variable*/
            if((checkid == (uint16)atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].cid)
                    && (atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].exp_event == expected_event)
                    && atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].dual_sys== dual_sys)
            {
                return TRUE;
            }
        }
        else
        {
            /*check expected event with call_id variable*/
            if((checkid == atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].call_id)
                    && (atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].exp_event == expected_event)
                    && atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].dual_sys== dual_sys)
            {
                return TRUE;
            }
        }

        start_p++;

        if(ATC_MAX_EXP_RESULT == start_p)
        {
            start_p = 0;
        }
    }

    return FALSE;
}

/******************************************************************************/
// Description : This function get the link id of the expected event and clean it if existed 
// Global resource dependence :
// Author :
// Note : 
/******************************************************************************/
BOOLEAN ATC_Get_Clean_Expected_Event(
    MN_DUAL_SYS_E   dual_sys,
    uint16 expected_event,
    ATC_EVENT_DOMAIN_T domain_id,
    uint16 *checkid,
    uint8 *link_id
)
{
    ATC_CONFIG_T *atc_config_ptr;
    uint16        start_p, end_p;

    atc_config_ptr = &s_config_info;
    start_p        = atc_config_ptr->atc_exp_link_info.start_point;
    end_p          = atc_config_ptr->atc_exp_link_info.end_point;

    while(start_p != end_p)
    {
        if(ATC_DOMAIN_PS == domain_id)
        {
            /*check expected event with cid variable*/
            if((atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].exp_event == expected_event)
                    && atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].dual_sys== dual_sys)
            {
                *link_id = atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].link_id;
                *checkid = (uint16)atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].cid;
                ATC_Clear_Exp_Result(&atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p]);
                return TRUE;
            }
        }
        else
        {
            /*check expected event with call_id variable*/
            if((atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].exp_event == expected_event)
                    && atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].dual_sys== dual_sys)
            {
                *link_id = atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].link_id;
                *checkid = atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].call_id;
                ATC_Clear_Exp_Result(&atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p]);
                return TRUE;
            }
        }

        start_p++;

        if(ATC_MAX_EXP_RESULT == start_p)
        {
            start_p = 0;
        }
    }

    return FALSE;
}


/******************************************************************************/
// Description : This function shrink the expected event buffer
// Global resource dependence :
// Author : minqian.qian
// Note :
/******************************************************************************/
void ATC_Shrink_Expected_Event_Buffer(ATC_CONFIG_T *atc_config_ptr)
{
    uint16                 end_p;

    //ATC_TRACE_LOW("ATC: ATC_Shrink_Expected_Event_Buffer!");

    while(atc_config_ptr->atc_exp_link_info.start_point != atc_config_ptr->atc_exp_link_info.end_point)
    {
        //shrink from start point
        if(atc_config_ptr->atc_exp_link_info.atc_exp_result_info[atc_config_ptr->atc_exp_link_info.start_point].exp_event == ATC_NO_EVENT)
        {
            atc_config_ptr->atc_exp_link_info.start_point++;

            if(atc_config_ptr->atc_exp_link_info.start_point >= ATC_MAX_EXP_RESULT)
            {
                atc_config_ptr->atc_exp_link_info.start_point = 0;
            }

            continue;
        }

        //shrink from end point
        if(0 == atc_config_ptr->atc_exp_link_info.end_point)
        {
            end_p = ATC_MAX_EXP_RESULT - 1;
        }

        else
        {
            end_p = atc_config_ptr->atc_exp_link_info.end_point - 1;
        }

        if(atc_config_ptr->atc_exp_link_info.atc_exp_result_info[end_p].exp_event == ATC_NO_EVENT)
        {
            atc_config_ptr->atc_exp_link_info.end_point = end_p;
            continue;
        }

        //shrink finished
        break;
    }
}

/* -----------------------------------------------------------------------------------------------

   Ring Buffer Management Routines

   Each ring buffer has:-
   An 'in' pointer, which points to the next location that new data will be inserted at.
   An 'out' pointer, which points to the next character to be removed from the buffer.
   A 'limit' value, which is the size of the data array.
   A 'data' array, of 0..(limit-1) bytes.

   When in == out, the buffer is considered empty.
   When (in + 1 % limit) == out, the buffer is considered full.

   Note that if you have a 'limit' of 10, you will only be able to store 9 characters before
   the buffer full condition is reached.

   ----------------------------------------------------------------------------------------------- */

/*
** Returns the number of characters that can be added to this ring before it becomes full.
*/
LOCAL int ATC_Ring_Space(ATC_BUF_T *pRing)
{
    int ret = pRing->start_point - pRing->end_point - 1;
    return (ret >= 0) ? ret : (ret + (int)pRing->size);
}


/*
** Add 'ch' to this ring.
** Returns -1 if there is not enough room.
** Returns 0 otherwise.
*/
LOCAL int ATC_Ring_Add(ATC_BUF_T *pRing, int ch)
{
    if(ATC_Ring_Space(pRing) == 0)
    {
        return -1;
    }

    pRing->atc_buf_ptr[pRing->end_point++] = ch;

    if(pRing->end_point >= (int)pRing->size)    //lint !e737
    {
        pRing->end_point = 0;
    }

    return 0;
}

/*
** Remove a character from this ring.
** Returns -1 if there are no characters in this ring.
** Returns the character removed otherwise.
*/
LOCAL int ATC_Ring_Remove(ATC_BUF_T *pRing)
{
    int ret;

    if(pRing->end_point == pRing->start_point)
    {
        return -1;
    }

    ret = pRing->atc_buf_ptr[pRing->start_point++];

    if(pRing->start_point >= (int)pRing->size)    //lint !e737
    {
        pRing->start_point = 0;
    }

    return ret;
}

/******************************************************************************/
// Description : This function called by ATC to clear AT buffer
// Global resource dependence :
// Author : Wang Hui
// Note :
/******************************************************************************/
void ATC_ClrCmdLine(MN_DUAL_SYS_E dual_sys)
{
    atc_buffer[dual_sys].start_point = atc_buffer[dual_sys].end_point;
    g_cur_atc_cmd_size[dual_sys] = 0;
}

/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  *
 **     VOID ATC_ReadCmdLine                                              *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     Read AT command from ATC BUFFER                                              *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     None                                                                  *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **---------------------------------------------------------------------------*/
void ATC_ReadCmdLine(
    uint8 link_id,
    uint8 *data_ptr,       // Output argument, point to the buf which is alloced
    // by caller
    uint length,           // The length of cmd line which should be read from
    // sio
    uint *read_length_ptr  // The actual read length
)
{
    MN_DUAL_SYS_E dual_sys = g_current_card_id;//ATC_GetSimIdFromLinkId(link_id);
    uint i = 0;
    int ch;
    BOOLEAN end = FALSE;

    for(i = 0; i < length; i++)
    {
        /* ATC_Ring_Remove returned value: -1, 0-255
         * So ch's type shall be "int" not "uint8" */
        ch = ATC_Ring_Remove(&atc_buffer[dual_sys]);

        if(ch != -1)
        {
            if(ch == atc_end_ch1[link_id] || ch == atc_end_ch2[link_id])
            {
                *data_ptr = (uint8)ch;
                *read_length_ptr += 1;

                i++;
                end = TRUE;
                break;
            }
            else if(ch == atc_bs_ch[link_id])
            {
                if(0 == *read_length_ptr)
                {
                    continue;
                }

                data_ptr--;
                *read_length_ptr -= 1;
            }
            else
            {
                *data_ptr++ = (uint8)ch;
                *read_length_ptr += 1;
            }
        }
        else
        {
            break;
        }
    }

    if(i < length)
    {
        ATC_TRACE_LOW("ATC: ATC_ReadCmdLine Warning: only read %d < %d.", i, length);
    }
    else if(FALSE == end)
    {
        ATC_TRACE_LOW("ATC: ATC_ReadCmdLine Warning: garbage data ???");
    }
}


/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  *
 **     void ATC_SetCmdLineTerminateChar( )                               *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     Set terminate char of of AT command                                   *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     None                                                                  *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **---------------------------------------------------------------------------*/
void ATC_SetCmdLineTerminateChar(
    uint8 link_id,
    uint8 terminate_char1,   // Default terminate char is '\n'
    uint8 terminate_char2    // Default terminate char is '\0'
)
{
    atc_end_ch1[link_id] = terminate_char1;
    atc_end_ch2[link_id] = terminate_char2;
}


/**---------------------------------------------------------------------------*
 ** FUNCTION                                                                  *
 **     void ATC_SetCmdLineBackSpaceChar()                                *
 **                                                                           *
 ** DESCRIPTION                                                               *
 **     Set backspace char of the AT command                                  *
 **                                                                           *
 ** RETURN VALUE                                                              *
 **     None                                                                  *
 **                                                                           *
 ** DEPENDENCIES                                                              *
 **---------------------------------------------------------------------------*/
void ATC_SetCmdLineBackSpaceChar(
    uint8 link_id,
    uint8 back_space_char  // Default backspace char value is 8
)
{
    atc_bs_ch[link_id] = back_space_char;
}

/******************************************************************************/
// Description : This function called by CMUX to ind ATC that a new char received
// Global resource dependence :
// Author : Wang Hui
// Note :
/******************************************************************************/
void ATC_SendNewATInd(uint8 link_id, uint32 data_length, uint8 *data_ptr)
{
    MN_DUAL_SYS_E dual_sys = ATC_GetSimIdFromLinkId(link_id);
    ATC_MUX_RECV_NEW_AT_T   *psig = PNULL;
    unsigned char           nchar = '\0';

    ATC_TRACE_LOW("ATC: N_ATInd,link_id:%d,data_len:%d ,%x, %x, close:%d,cnt:%d,discard:%d,ch1:%d,ch2:%d",
                  link_id, data_length, data_ptr[data_length-2], data_ptr[data_length-1],
                  is_close_atc_queue, s_atc_global_info.buffered_at_cmd_count, s_dicard_count,
                  atc_end_ch1[link_id], atc_end_ch2[link_id]);


    if ((!g_is_mux_single_sys_mod &&(link_id > MN_SYS_NUMBER * 3)&&
       (link_id <= ATC_PDP_NUM_MAX + MN_SYS_NUMBER * 3))
       ||(g_is_mux_single_sys_mod && (link_id >= ATC_DEBUG_LINK - 3)&&
       (link_id <= ATC_DEBUG_LINK - 1)))
    {
        ATC_TRACE_LOW("ATC: N_ATInd, discards packet on data channel!");
        return;
    }

    //@for avoiding task queue full
    if(s_atc_global_info.buffered_at_cmd_count >= 16)  // 16 = atc_task_queue_count - 4
    {
        /*
        * 1.SIO buffer is circle buffer,put in char by adding left edge,and get out
        *  char by minus right edge
        * 2.so call  SIO_ATC_ReadCmdLine() will delete the first at command and not
        *  the newest put in at command.
        * 3.because all the at command buffer in SIO buffer(it is 1200 bytes),we
        *  buffer at command num and should buffer at command length,if it over 1200 bytes
        * we should clear the first at command for very long at command
        */

        is_close_atc_queue = TRUE;
    }

    if(is_close_atc_queue)
    {
        ATC_TRACE_LOW("ATC: N_ATInd, is_close_atc_queue is TRUE, return");
        return;
    }

    if(!atc_buffer[dual_sys].atc_buf_ptr)
    {
        ATC_TRACE_LOW("ATC: N_ATInd, MUX is not setup!!");
        return;
    }

    if(atc_is_echo)
    {
        ATC_TRACE_LOW("ATC: N_ATInd, atc_is_echo is TRUE, send copy back");
        MUX_Write(data_ptr, data_length, link_id);
    }

    while(data_length)
    {
        nchar = *data_ptr++;
        data_length--;

        if(s_atc_global_info.buffered_at_cmd_count >= 16) //16, modify to 50 by auto tesf jungo
        {
            //ATC_TRACE_LOW("ATC:Discard some message!! ");
            s_dicard_count++;
            continue;
        }

        if (g_cur_atc_cmd_size[dual_sys] == 0 && 
            (nchar == atc_end_ch1[link_id] || nchar == atc_end_ch2[link_id])
            || nchar == 0x0a)
        {
            continue;
        }

       #ifdef ATC_SAT_ENABLE
        // only for direct satellite project
        if(g_auto_send_len > 0)
        {
            SCI_TRACE_LOW("atc: test cur_atc_cmd_size =%d, g_auto_send_len=%d", 
                          g_cur_atc_cmd_size[dual_sys],
                          g_auto_send_len);

            if(TRUE == ATC_CheckIpSendDataByLen(&nchar, link_id))
            {
                continue;
            }
            else if(g_auto_send_len == g_cur_atc_cmd_size[dual_sys])
            {
                data_length = 0;    // stop loop
                g_auto_send_len = 0;
            }
        }
       #endif /*ATC_SAT_ENABLE*/

        if(ATC_Ring_Add(&atc_buffer[dual_sys], nchar) == -1)
        {
            ATC_TRACE_LOW("ATC:ERROR AT BUFFER FULL  LINK %d !!!! ", link_id);

            ATC_ClrCmdLine(dual_sys);

            //reset terminate chars when AT buffer is full
            //Fix the bug: the last character(received) of an AT Command is different from terminate chars which set by this command
            if(link_id == ATC_DEBUG_LINK)
            {
                SIO_ATC_SetCmdLineTerminateChar(s_atc_global_info.atc_config_ptr->s3_reg, '\0');
            }
            else
            {
                ATC_SetCmdLineTerminateChar(link_id, s_atc_global_info.atc_config_ptr->s3_reg, '\0');
            }

            ATC_BuildResultCodeRsp(s_atc_global_info.atc_config_ptr, S_ATC_RESULT_CODE_ERROR, ERR_AT_COMMAND_DISCARDED);
            continue;
        }

        g_cur_atc_cmd_size[dual_sys]++;

        if(nchar == atc_end_ch1[link_id] || nchar == atc_end_ch2[link_id])
        {
            //send start up signal to start up the sim task
            SCI_CREATE_SIGNAL((xSignalHeaderRec *)psig, ATC_MUX_RECV_NEW_AT, sizeof(ATC_MUX_RECV_NEW_AT_T), SCI_IdentifyThread());//lint !e63
            psig->len       = g_cur_atc_cmd_size[dual_sys];//lint !e613
            psig->link_id   = link_id;//lint !e613

            s_atc_global_info.buffered_at_cmd_count ++;

            SCI_SEND_SIGNAL((xSignalHeaderRec *)psig, P_ATC);//lint !e63

            // remove all following zeros
            while(data_length)
            {
                if(0 != *data_ptr)
                {
                    break;
                }

                data_ptr++;
                data_length--;
            }

            g_cur_atc_cmd_size[dual_sys] = 0;
        }
    }
}

/******************************************************************************/
// Description : This function called by SIO to ind ATC that a new AT received
// Global resource dependence :
// Author : Wang Hui
// Note : the function is called by SIO Interrupt Service Routine,so it should be
//        optimized
/******************************************************************************/
void ATC_Debug_SendATInd(uint32 len)
{
    MN_DUAL_SYS_E dual_sys = g_current_card_id;//MN_DUAL_SYS_1;
    ATC_MUX_RECV_NEW_AT_T *psig;
    unsigned char  nchar;
    uint8       *cmd_line_ptr = PNULL;
    uint8       *temp_cmd_line_ptr = PNULL;
    uint         temp_line_length    = 0;
    uint         temp_line_length_total    = 0;

    ATC_TRACE_LOW("ATC: Debug_ATInd, len:%d", len);


    if(s_atc_global_info.buffered_at_cmd_count >= 16)  // 16 = atc_task_queue_count - 4
    {
        /*
         * 1.SIO buffer is circle buffer,put in char by adding left edge,and get out
         *  char by minus right edge
         * 2.so call  SIO_ATC_ReadCmdLine() will delete the first at command and not
         *  the newest put in at command.
         * 3.because all the at command buffer in SIO buffer(it is 1200 bytes),we
         *  buffer at command num and should buffer at command length,if it over 1200 byts
         * we should clear the first at command for very long at command
         */
        is_close_atc_queue = TRUE;
    }

    if(is_close_atc_queue)
    {
        SIO_ATC_ClrCmdLine();

        ATC_TRACE_LOW("ATC: Debug_ATInd, is_close_atc_queue is TRUE, return");
        return;
    }

    cmd_line_ptr = (uint8 *)SCI_ALLOC_BASE(ATC_REC_BUF_SIZE);
    if (PNULL == cmd_line_ptr)
    {
        ATC_TRACE_LOW("ATC: Debug_ATInd, no memory to store cmd line");
        return;
    }

    SIO_ATC_ReadCmdLine(cmd_line_ptr, len, &temp_line_length);

    temp_line_length_total = temp_line_length;

    temp_cmd_line_ptr = cmd_line_ptr;
    
    SCI_ASSERT(atc_buffer[dual_sys].atc_buf_ptr != PNULL);/*assert verified: check null pointer*/

    while(temp_line_length)
    {
        nchar = *temp_cmd_line_ptr++;
        temp_line_length--;

        if(ATC_Ring_Add(&atc_buffer[dual_sys], nchar) == -1)
        {
            ATC_TRACE_LOW("ATC:ERROR AT BUFFER FULL  LINK ATC_DEBUG_LINK!!!! ");
            ATC_ClrCmdLine(dual_sys);
            continue;
        }
    }

    SCI_FREE(cmd_line_ptr);

    //send start up signal to start up the sim task
    SCI_CREATE_SIGNAL((xSignalHeaderRec *)psig, ATC_MUX_RECV_NEW_AT, sizeof(ATC_MUX_RECV_NEW_AT_T), SCI_IdentifyThread());//lint !e63
    psig->len = temp_line_length_total;
    psig->link_id = ATC_DEBUG_LINK;
    s_atc_global_info.buffered_at_cmd_count ++;
    SCI_SEND_SIGNAL((xSignalHeaderRec *)psig, P_ATC);

    return;
}

/******************************************************************************/
// Description : This function get link id on which call-id(cid) is
// Global resource dependence :
// Author : Pizer Fan
// Note :
/******************************************************************************/
uint8 ATC_Get_Link_Id(uint8 id, ATC_EVENT_DOMAIN_T domain_id, MN_DUAL_SYS_E dual_sys)
{
    ATC_CONFIG_T *atc_config_ptr;

    atc_config_ptr = &s_config_info;

    if(domain_id == ATC_DOMAIN_PS)
    {
        if((id >= 1 && id <= MN_GPRS_MAX_PDP_CONTEXT_COUNT) &&
                (atc_config_ptr->atc_psd_call_info[id - 1].cid != ATC_INVALID_CID))
        {
            ATC_TRACE_LOW("ATC: ATC_Get_Link_Id, get call link id: %d", atc_config_ptr->atc_psd_call_info[id - 1].link_id);
            ATC_List_PSD_Call_Info(id);
            return atc_config_ptr->atc_psd_call_info[id - 1].link_id;
        }
    }

    return (uint8)ATC_INVALID_LINK_ID;
}

/******************************************************************************/
// Description : This function clear all expected events of call-id(cid)
// Global resource dependence :
// Author : Pizer Fan
// Note :
/******************************************************************************/
void ATC_Clear_All_Exp_Result(uint8 id, ATC_EVENT_DOMAIN_T domain_id, MN_DUAL_SYS_E dual_sys)
{
    ATC_CONFIG_T *atc_config_ptr;
    uint16        start_p, end_p;

    atc_config_ptr = &s_config_info;
    start_p = atc_config_ptr->atc_exp_link_info.start_point;
    end_p  = atc_config_ptr->atc_exp_link_info.end_point;

    ATC_TRACE_LOW("ATC: ATC_Clear_All_Exp_Result id:%d,domain_id:%d, sim_id:%d", id, domain_id, dual_sys);

    if(domain_id == ATC_DOMAIN_PS)
    {
        while(start_p != end_p)
        {
            /* When deactivation, clear all expected events
             * related to call states. */
            if((atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].cid == id) &&
                (atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].dual_sys== dual_sys)&&
                ((atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].exp_event == APP_MN_ACTIVATE_PDP_CONTEXT_CNF) ||
                  (atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].exp_event == APP_MN_DEACTIVATE_PDP_CONTEXT_CNF) ||
                  (atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].exp_event == APP_MN_DEACTIVATE_PDP_CONTEXT_IND)))
            {
                ATC_Clear_Exp_Result(&atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p]);
            }

            start_p++;

            if(ATC_MAX_EXP_RESULT == start_p)
            {
                start_p = 0;
            }
        }
    }
    else if(domain_id == ATC_DOMAIN_CS)
    {
        while(start_p != end_p)
        {
            if((atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].call_id == id) &&
                (atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].dual_sys== dual_sys))
            {
                ATC_TRACE_LOW("ATC: ATC_Clear_All_Exp_Result, call_id:%d, exp event: %d",
                              id, atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].exp_event);
                ATC_Clear_Exp_Result(&atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p]);
            }

            start_p++;

            if(ATC_MAX_EXP_RESULT == start_p)
            {
                start_p = 0;
            }
        }
    }

    ATC_Shrink_Expected_Event_Buffer(atc_config_ptr);
}

#ifndef ATC_SAT_ENABLE
#ifdef _DEBUG
/******************************************************************************/
// Description : This function to get event name
// Global resource dependence :
// Author : Pizer Fan
// Note :
/******************************************************************************/
const char *ATC_Get_Event_Name( uint16 event )
{
    const char *name = NULL;
    uint i;

    for(i = 0; i < sizeof(expect_event_name) / sizeof(expect_event_name[0]); i++)
    {
        if(expect_event_name[i].event == event)
        {
            name = expect_event_name[i].name;
            break;
        }
    }

    return name;
}
#endif
#endif


/******************************************************************************/
// Description : This function list all the expected events
// Global resource dependence :
// Author : Pizer Fan
// Note :
/******************************************************************************/
void ATC_List_Expected_Event(void)
{
#ifdef _DEBUG
    ATC_CONFIG_T   *atc_config_ptr;
    uint16          start_p, end_p;
    int             expected_event_num = 0;

    ATC_TRACE_LOW("ATC: List_Exp_Evnt:");

    atc_config_ptr = &s_config_info;
    start_p = atc_config_ptr->atc_exp_link_info.start_point;
    end_p  = atc_config_ptr->atc_exp_link_info.end_point;

    while(start_p != end_p)
    {
        if(ATC_NO_EVENT != atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].exp_event)
        {
         #ifndef ATC_SAT_ENABLE
            const char *name = NULL;
         #endif

            if(0 == expected_event_num)
            {
                SCI_TRACE_LOW("|Idx|Link-ID| SIM | CID |Call-ID| Event| Name");
                SCI_TRACE_LOW("|---|-------|-----|-----|-------|------|------------------------");
            }


            expected_event_num++;

         #ifndef ATC_SAT_ENABLE
            name = ATC_Get_Event_Name(atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].exp_event);
            if(name)
            {
                SCI_TRACE_LOW("|%-3d|  %d   |   %d  | %-3d |  %-3d | %d   | %s", start_p,
                              atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].link_id,
                              atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].dual_sys,
                              atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].cid,
                              atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].call_id,
                              atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].exp_event,
                              name);
            }
            else
         #endif
            {
                SCI_TRACE_LOW("|%-3d|  %d   |   %d  | %-3d |  %-3d | %d   |", start_p,
                              atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].link_id,
                              atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].dual_sys,
                              atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].cid,
                              atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].call_id,
                              atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].exp_event);
            }
        }

        start_p++;

        if(ATC_MAX_EXP_RESULT == start_p)
        {
            start_p = 0;
        }
    }

    if(0 != expected_event_num)
    {
        ATC_TRACE_LOW("|---|-------|-----|-----|-------|------|------------------------");
    }

#endif
}

/******************************************************************************/
// Description : This function get unsolicited report link id from sim_id
// Global resource dependence : 
// Author : Elvis Xu
// Note :
/******************************************************************************/
uint8 ATC_GetMuxLinkIdFromSimId(MN_DUAL_SYS_E sim_id)
{
    if(RM_GetCalibrationPostMode())  //if cal post mode 
    {
        SCI_TRACE_LOW("ATC: ATC_GetMuxLinkIdFromSimId CAL POST MODE, return ATC_DEBUG_LINK");
        return ATC_DEBUG_LINK;
    }

    if(g_mux_sys_info[MUX_1].is_start==MUX_STATUS_OFF)
    {
        SCI_TRACE_LOW("ATC_GetDualSimMuxLinkId return ctl_link %d", g_mux_sys_info[MUX_1].ctl_link);
        return g_mux_sys_info[MUX_1].ctl_link;
    }

    /*support single sim scheme*/
    if (g_is_mux_single_sys_mod)
    {
        return atc_urc_link_id;
    }

    if (sim_id >= MN_SYS_NUMBER)
    {
        SCI_TRACE_LOW("ATC: ATC_GetMuxLinkIdFromSimId sim id %d", sim_id);
        return ATC_INVALID_LINK_ID;
    }
    else
    {
        return (uint8)(sim_id * ATC_LINK_NUM_PER_SIM + 1);
    }
}

/******************************************************************************/
// Description : This function get sim_id id from link id
// Global resource dependence : 
// Author : Elvis Xu
// Note :
/******************************************************************************/
MN_DUAL_SYS_E ATC_GetSimIdFromLinkId(uint8 link_id)
{
    /*support single sim scheme*/
    if (g_is_mux_single_sys_mod)
    {
        if(g_mux_sys_info[MUX_1].is_start == MUX_STATUS_OFF)
        {
            return g_current_card_id;
        }
        else
        {
            return MN_DUAL_SYS_1;
        }
    }

    if((link_id > MN_DUAL_SYS_1) && (link_id <= MN_SYS_NUMBER * ATC_LINK_NUM_PER_SIM))
    {
        return (MN_DUAL_SYS_E)((link_id - 1) / ATC_LINK_NUM_PER_SIM);
    }
    /*ppp active*/
    else if((link_id > MN_SYS_NUMBER * ATC_LINK_NUM_PER_SIM) && (link_id <  ATC_DEBUG_LINK/* <= (MN_SYS_NUMBER * ATC_LINK_NUM_PER_SIM + ATC_PDP_NUM_MAX))*/))
    {
        return g_atc_modem_dial_sys;
    }
    else if(link_id == ATC_DEBUG_LINK)
    {
        return MN_DUAL_SYS_1;// this link id maybe come from diag at cmd
    }
    /*mux disable or other state*/
    else
    {
        return g_current_card_id;
    }
}

/******************************************************************************/
// Description : This function get cid from data link id
// Global resource dependence : 
// Author : Elvis Xu
// Note :
/******************************************************************************/
uint8 ATC_GetCidFromLinkId(uint8 link_id)
{
    /*support single sim scheme*/
    if (g_is_mux_single_sys_mod)
    {
        if (link_id == ATC_DEBUG_LINK - 3)
        {
            return 1;
        }
        else if (link_id == ATC_DEBUG_LINK - 2)
        {
            return 2;
        }
        else if (link_id == ATC_DEBUG_LINK - 1)
        {
            return 3;
        }
        else
        {
            return ATC_INVALID_CID;
        }
    }

    if((link_id > MN_SYS_NUMBER * ATC_LINK_NUM_PER_SIM) && (link_id < MN_SYS_NUMBER * ATC_LINK_NUM_PER_SIM + ATC_PDP_NUM_MAX))
    {
        return ((link_id - MN_SYS_NUMBER * ATC_LINK_NUM_PER_SIM) <= ATC_PDP_NUM_MAX) ? (link_id - MN_SYS_NUMBER * ATC_LINK_NUM_PER_SIM) : ATC_INVALID_CID;
    }
    else
    {
        return ATC_INVALID_CID;
    }
}

/******************************************************************************/
// Description : This function get data link id id from cid
// Global resource dependence : 
// Author : Elvis Xu
// Note :
/******************************************************************************/
uint8 ATC_GetLinkIdFromCid(uint8 cid)
{
    /*support single sim scheme*/
    if (g_is_mux_single_sys_mod)
    {
        if (cid == 1)
        {
            return ATC_DEBUG_LINK - 3;  //mux 14
        }
        else if (cid == 2)
        {
            return ATC_DEBUG_LINK - 2;  //mux 15
        }
        else if (cid == 3) 
        {
            return ATC_DEBUG_LINK - 1;  //mux 16
        }
        else if (cid > ATC_PDP_NUM_MAX)
        {
            return ATC_INVALID_LINK_ID;
        }
    }

    if((cid < 1) || (cid > ATC_PDP_NUM_MAX))
    {
        return ATC_INVALID_LINK_ID;
    }
    
    return ((cid + MN_SYS_NUMBER * ATC_LINK_NUM_PER_SIM) < ATC_DEBUG_LINK) ? (cid + MN_SYS_NUMBER * ATC_LINK_NUM_PER_SIM) : ATC_INVALID_LINK_ID;
}
#else/*_MUX_ENABLE_*/

/******************************************************************************/
// Description : This function called by SIO to ind ATC that a new AT received
// Global resource dependence :
// Author : Ivan
// Note : the function is called by SIO Interrupt Service Routine,so it should be
//        optimized
/******************************************************************************/
void SendNewATInd(uint32 len)
{
    SIO_RECV_NEW_AT_T* psig = PNULL;
    BOOLEAN is_new_cmd = FALSE;
    int  cmd_len = 0;

    ATC_TRACE_LOW("ATC: SendNewATInd, len:%d", len);

    if(s_atc_global_info.buffered_at_cmd_count >= 16)  // 16 = atc_task_queue_count - 4
    {
        /*
         * 1.SIO buffer is circle buffer,put in char by adding left edge,and get out
         *  char by minus right edge
         * 2.so call  SIO_ATC_ReadCmdLine() will delete the first at command and not
         *  the newest put in at command.
         * 3.because all the at command buffer in SIO buffer(it is 1200 bytes),we
         *  buffer at command num and should buffer at command length,if it over 1200 byts
         * we should clear the first at command for very long at command
         */

        //SIO_ATC_ReadCmdLine(&atc_cmd_line_buf[0],ATC_REC_BUF_SIZE,&atc_read_length);
        //ATC_TRACE_LOW("ATC:NO CREATE SINGNAL!");
        is_close_atc_queue = TRUE;

        //return;
    }

    if(is_close_atc_queue)
    {
        SIO_ATC_ClrCmdLine();

        ATC_TRACE_LOW("ATC: SendNewATInd, is_close_atc_queue is TRUE");
        return;
    }

    cmd_len = SIO_ATC_CheckNewCmd(&is_new_cmd, len);
    if (cmd_len == -1)
    {
        SCI_TRACE_LOW("ATC: SIO Buffer is empty or invalid len %d", len);
        return;
    }

    if (is_new_cmd == TRUE)
    {
        /*lint -e668*/
        //send start up signal to start up the sim task
        SCI_CREATE_SIGNAL(psig,SIO_RECV_NEW_AT,sizeof(SIO_RECV_NEW_AT_T),SCI_IdentifyThread());//lint !e64
        psig->len = cmd_len;
        /*lint +e668*/
    
        s_atc_global_info.buffered_at_cmd_count++;
    
        SCI_SEND_SIGNAL((xSignalHeaderRec*)psig, P_ATC);
        SCI_TRACE_LOW("ATC: recv a new AT  len %d", cmd_len);
    }
#if 0 // only for debug    
    else
    {
        SCI_TRACE_LOW("ATC: recv a cmd fragment len%d", len);
    }
#endif
    return;
}
#endif
/******************************************************************************/
// Description : This function get s_config_info variable pointer
// Global resource dependence :
// Author : Elvis Xu
// Note :
/******************************************************************************/
ATC_CONFIG_T * ATC_GetConfigInfoPtr(void)
{
    return &s_config_info;
}

/******************************************************************************/
// Description : This function create PSD call info
// Global resource dependence :
// Author : Pizer Fan
// Note :
/******************************************************************************/
BOOLEAN ATC_Create_PSD_Call(uint8 cid, uint8 link_id, BOOLEAN data_mode_req)
{
    /* NOTE: when PDP activate request is already send to MN/L4,
     * this function will be invoked */
    if(cid >= 1 && cid <= ATC_MAX_ACTIVATE_PDP)
    {
        ATC_CONFIG_T *atc_config_ptr = (ATC_CONFIG_T *)&s_config_info;
        uint8         data_link_id = ATC_Get_Data_Link_Id(link_id, cid);

        if(ATC_PSD_STATE_INACTIVE == atc_config_ptr->atc_psd_call_info[cid - 1].state)
        {
            /* PDP is not used, so set its data mode directly. */
            atc_config_ptr->atc_psd_call_info[cid - 1].link_id       = link_id;
            atc_config_ptr->atc_psd_call_info[cid - 1].data_link_id  = data_link_id;
            atc_config_ptr->atc_psd_call_info[cid - 1].cid           = cid;
            atc_config_ptr->atc_psd_call_info[cid - 1].data_mode_req = data_mode_req;
            atc_config_ptr->atc_psd_call_info[cid - 1].data_mode     = data_mode_req;
            atc_config_ptr->atc_psd_call_info[cid - 1].state         = ATC_PSD_STATE_ACTIVATE_PENDING;
            atc_config_ptr->atc_psd_call_info[cid - 1].init_mod_state = INIT_BY_ATC;

            ATC_TRACE_LOW("ATC: ATC_Create_PSD_Call,PSD_CALL,cid:%d,link_id:%d,data_link_id:%d,data_mode:%d,state:%d,init_mod_state:%d",
                          cid, link_id, data_link_id, data_mode_req, ATC_PSD_STATE_ACTIVATE_PENDING, INIT_BY_ATC);
        }
        else
        {
            /* in the case of PDP (cid)'s being already active or activate-pending,
             * if link-id is not same, impact appear. */
            if((ATC_INVALID_LINK_ID != atc_config_ptr->atc_psd_call_info[cid - 1].link_id)
                    && (atc_config_ptr->atc_psd_call_info[cid - 1].link_id != link_id))
            {
                ATC_TRACE_LOW("ATC: ATC_Create_PSD_Call,PSD_CALL,ERROR,cid:%d,(!)link_id:%d-\\->%d,data_mode:%d,(!)state:%d",
                              cid, atc_config_ptr->atc_psd_call_info[cid - 1].link_id, link_id,
                              atc_config_ptr->atc_psd_call_info[cid - 1].data_mode,
                              atc_config_ptr->atc_psd_call_info[cid - 1].state);
                return FALSE;
            }

            /* PDP is already data mode */
            if(TRUE == atc_config_ptr->atc_psd_call_info[cid - 1].data_mode)
            {
                ATC_TRACE_LOW("ATC: ATC_Create_PSD_Call,PSD_CALL,ERROR,cid:%d,link_id:%d,(!)data_mode:%d-\\->%d,(!)state:%d",
                              cid, link_id,
                              atc_config_ptr->atc_psd_call_info[cid - 1].data_mode, data_mode_req,
                              atc_config_ptr->atc_psd_call_info[cid - 1].state);
                return FALSE;
            }

            ATC_TRACE_LOW("ATC: ATC_Create_PSD_Call,PSD_CALL,WARNING,cid:%d->%d,link_id:%d,%d->%d,%d,data_mode:%d->%d,(!)state:%d",
                          atc_config_ptr->atc_psd_call_info[cid - 1].cid, cid,
                          atc_config_ptr->atc_psd_call_info[cid - 1].link_id,
                          atc_config_ptr->atc_psd_call_info[cid - 1].data_link_id, link_id, data_link_id,
                          atc_config_ptr->atc_psd_call_info[cid - 1].data_mode, data_mode_req,
                          atc_config_ptr->atc_psd_call_info[cid - 1].state);

            /* anywhere, go on! */
            atc_config_ptr->atc_psd_call_info[cid - 1].link_id       = link_id;
            atc_config_ptr->atc_psd_call_info[cid - 1].data_link_id  = data_link_id;
            atc_config_ptr->atc_psd_call_info[cid - 1].cid           = cid;
            atc_config_ptr->atc_psd_call_info[cid - 1].data_mode_req = data_mode_req;
        }

        return TRUE;
    }
    else
    {
        ATC_TRACE_LOW("ATC: ATC_Create_PSD_Call,PSD_CALL,cid:%d is not in range", cid);
        return FALSE;
    }
}

/******************************************************************************/
// Description : This function set PSD call state
// Global resource dependence :
// Author : Pizer Fan
// Note :
/******************************************************************************/
void ATC_Set_PSD_Call_State(uint8 cid, ATC_PSD_STATE_E state)
{
    if(cid >= 1 && cid <= ATC_MAX_ACTIVATE_PDP)
    {
        ATC_CONFIG_T *atc_config_ptr = &s_config_info;

        ATC_TRACE_LOW("ATC: ATC_Set_PSD_Call_State,PSD_CALL,op-cid:%d,cid:%d,link_id:%d,data_link_id:%d,data_mode:%d,state:%d->%d",
                      cid,
                      atc_config_ptr->atc_psd_call_info[cid-1].cid,
                      atc_config_ptr->atc_psd_call_info[cid-1].link_id,
                      atc_config_ptr->atc_psd_call_info[cid-1].data_link_id,
                      atc_config_ptr->atc_psd_call_info[cid-1].data_mode,
                      atc_config_ptr->atc_psd_call_info[cid-1].state,
                      state);

        atc_config_ptr->atc_psd_call_info[cid-1].state = state;
    }
    else
    {
        ATC_TRACE_LOW("ATC: ATC_Set_PSD_Call_State,PSD_CALL,cid:%d is not in range", cid);
    }
}

/******************************************************************************/
// Description : This function clear PSD call info
// Global resource dependence :
// Author : Pizer Fan
// Note :
/******************************************************************************/
void ATC_Clear_PSD_Call_Info(uint8 cid)
{
    if(cid >= 1 && cid <= ATC_MAX_ACTIVATE_PDP)
    {
        ATC_CONFIG_T *atc_config_ptr;
        atc_config_ptr = &s_config_info;

        ATC_TRACE_LOW("ATC: ATC_Clear_PSD_Call_Info,PSD_CALL,op-cid:%d,cid:%d,link_id:%d,data_link_id:%d,data_mode:%d,state:%d,init_mod_state:%d",
                      cid,
                      atc_config_ptr->atc_psd_call_info[cid-1].cid,
                      atc_config_ptr->atc_psd_call_info[cid-1].link_id,
                      atc_config_ptr->atc_psd_call_info[cid-1].data_link_id,
                      atc_config_ptr->atc_psd_call_info[cid-1].data_mode,
                      atc_config_ptr->atc_psd_call_info[cid-1].state,
                      atc_config_ptr->atc_psd_call_info[cid-1].init_mod_state);

        atc_config_ptr->atc_psd_call_info[cid - 1].link_id       = ATC_INVALID_LINK_ID;
        atc_config_ptr->atc_psd_call_info[cid - 1].data_link_id  = ATC_INVALID_LINK_ID;
        atc_config_ptr->atc_psd_call_info[cid - 1].cid           = ATC_INVALID_CID;
        atc_config_ptr->atc_psd_call_info[cid - 1].nsapi         = cid - 1 + ATC_MIN_NSAPI;
        atc_config_ptr->atc_psd_call_info[cid - 1].data_mode_req = FALSE;
        atc_config_ptr->atc_psd_call_info[cid - 1].data_mode     = FALSE;
        atc_config_ptr->atc_psd_call_info[cid - 1].state         = ATC_PSD_STATE_INACTIVE;
        atc_config_ptr->atc_psd_call_info[cid - 1].init_mod_state = INIT_UNKNOW;
    }
    else
    {
        ATC_TRACE_LOW("ATC: ATC_Clear_PSD_Call_Info,PSD_CALL,cid:%d is not in range", cid);
    }
}

/******************************************************************************/
// Description : This function List PSD call info
// Global resource dependence :
// Author : Pizer Fan
// Note :
/******************************************************************************/
void ATC_List_PSD_Call_Info(uint8 cid)
{
    if(cid >= 1 && cid <= ATC_MAX_ACTIVATE_PDP)
    {
        ATC_CONFIG_T *atc_config_ptr;
        atc_config_ptr = &s_config_info;

        ATC_TRACE_LOW("ATC: ATC_List_PSD_Call_Info,PSD_CALL,op-cid:%d,cid:%d,link_id:%d,%d,data_mode:%d,state:%d,init_mod_state:%d",
                      cid,
                      atc_config_ptr->atc_psd_call_info[cid-1].cid,
                      atc_config_ptr->atc_psd_call_info[cid-1].link_id,
                      atc_config_ptr->atc_psd_call_info[cid-1].data_link_id,
                      atc_config_ptr->atc_psd_call_info[cid-1].data_mode,
                      atc_config_ptr->atc_psd_call_info[cid-1].state,
                      atc_config_ptr->atc_psd_call_info[cid-1].init_mod_state);
    }
}

/******************************************************************************/
// Description : This function judge that psd call info list has datamode item or not
// Global resource dependence :
// Author : minqian.qian
// Note :
/******************************************************************************/
BOOLEAN ATC_IsExist_DataMode_PSD_Call_Info(void)
{
    int i;
    ATC_CONFIG_T *atc_config_ptr = PNULL;

    atc_config_ptr = &s_config_info;

    for(i = 0; i < ATC_MAX_ACTIVATE_PDP; ++i)
    {
        if(TRUE == atc_config_ptr->atc_psd_call_info[i].data_mode &&
                ATC_PSD_STATE_INACTIVE != atc_config_ptr->atc_psd_call_info[i].state &&
                ATC_INVALID_LINK_ID != atc_config_ptr->atc_psd_call_info[i].link_id)
        {
            return TRUE;
        }
    }

    return FALSE;
}

/******************************************************************************/
// Description : This function called by SIO and MMI to know if the state is on dialing or not
// Global resource dependence :
// Author : king.li
// Note :
/******************************************************************************/
ATC_MODEM_STATUS_E ATC_GetModemStatus(void)
{

    uint32 i = 0;

    while(i < MN_GPRS_MAX_PDP_CONTEXT_COUNT)
    {
        if(TRUE == s_atc_global_info.atc_config_ptr->pdp_id_arr[i++])
        {
            return ATC_MODEM_ONDIAL;
        }
    }

    return ATC_MODEM_NOT_ONDIAL;

}

#ifdef MODEM_WAKEUP_SUPPORT
#include "deep_sleep.h"
#include "gpio_prod_cfg.h"

#define SLP_DELAY_TIMEOUT       (2000)


PUBLIC SCI_TIMER_PTR     s_slp_delay_timer_ptr = PNULL;

/*****************************************************************************/
//  Description:    This function is to get the GPIO id of AP wake up Modem
//
//  Author:         Liangwen.zhen
//  Note:           
/*****************************************************************************/
LOCAL uint32 GPIO_GetAPWakeupBBIntGpio(void)
{
    GPIO_CFG_INFO_T_PTR cfg_ptr = GPIO_PROD_GetCfgInfo(GPIO_PROD_AP_WAKEUP_BB_ID);
    
    if (PNULL != cfg_ptr)
    {
        return cfg_ptr->gpio_num;
    }
    else
    {
        return 0xffffffff;
    }
}

/*****************************************************************************/
//  Description:    these action should be done when timeout after INT
//
//  Author:         Liangwen.zhen
//  Note:           
/*****************************************************************************/
LOCAL void Slp_Delay_TimerHandler(uint32 param)
{
    SCI_DisableIRQ();
    
    if (!SCI_IsTimerActive(s_slp_delay_timer_ptr))
    {
       DPSLP_UART_EnableApbSleep(UART0_E, ENABLE_APB_SLEEP);
    }
    SCI_DeactiveTimer(s_slp_delay_timer_ptr);
    SCI_RestoreIRQ();
}

/*****************************************************************************/
//  Description:    start a timer to disable deep sleep each wakeup INT
//
//  Author:         Liangwen.zhen
//  Note:           
/*****************************************************************************/
PUBLIC void Active_Slp_Delay_timer(void)
{
    if (s_slp_delay_timer_ptr)
    {
        SCI_ChangeTimer(s_slp_delay_timer_ptr, Slp_Delay_TimerHandler, SLP_DELAY_TIMEOUT);
        SCI_ActiveTimer(s_slp_delay_timer_ptr);
    }
}

#endif //MODEM_WAKEUP_SUPPORT

#ifdef MUX_UART_FLOW_CTRL
LOCAL BOOLEAN is_atc_task_initialized = SCI_FALSE;
LOCAL void ATC_TaskStatus_Set(BOOLEAN status)
{
    is_atc_task_initialized = status;
}

PUBLIC BOOLEAN ATC_TaskStatus_Get(void)
{
    return is_atc_task_initialized;
}
#endif

void SfsMount()
{
	const uint16 g_file_device_sysname[] = { 'D', 0 };   //将创建D盘
	SFS_ERROR_E	ret = 0;
	SFS_DEVICE_FORMAT_E format = SFS_UNKNOWN_FORMAT;
	ret = SFS_GetDeviceStatus(g_file_device_sysname);  //查看分区是否存在
	wis_debug("SFS_GetDeviceStatus ret 0x%x\r\n",ret);
	if(0 != ret)  //不为0代表没这个分区，需要注册
	{
		ret = SFS_RegisterDevice (g_file_device_sysname, &format); //进行分区注册
		wis_debug("SFS_RegisterDevice ret 0x%x\r\n",ret);
	}
}
/******************************************************************************/
// Description : This function is the main entry point of ATC task.
// Global resource dependence :
// Author : Steven,Ivan
// Note :
// History:
/******************************************************************************/
void ATC_Task(uint32 argc, void *argv)
{
    ATC_STATUS          status   = S_ATC_SUCCESS;
    xSignalHeaderRec    *sig_ptr = PNULL;
    SfsMount();

    status = ATC_Initialize();

#ifdef _MUX_ENABLE_
    MUX_Set_Atc_CallbackFunc(0xff, ATC_SendNewATInd);
    SIO_ATC_SetCallback(ATC_Debug_SendATInd);
#else
    SIO_ATC_SetCallback(SendNewATInd);
#endif

#ifdef MODEM_WAKEUP_SUPPORT
    s_slp_delay_timer_ptr = SCI_CreateTimer ("Slp Delay Timer",\
                                       Slp_Delay_TimerHandler,\
                                       0, 
                                       SLP_DELAY_TIMEOUT,
                                       SCI_NO_ACTIVATE);
    
#endif //MODEM_WAKEUP_SUPPORT

#if defined(_ATC_ONLY)
    if(RM_GetCalibrationPostMode() || (s_config_info.atc_config_nv.spauto_flag & 0x08)) //for CFT calibration mode test
    {
        MNPHONE_PowerOnSimEx(0, TRUE);
        s_config_info.cfun_status[0]=ATC_FUN_ACT_SIM;
    }
    
    if(s_config_info.atc_config_nv.spauto_flag & 0x02)
    {
        MNPHONE_SetGprsAttachModeEx(0, FALSE);//gprs auto attach
    }
#endif

#ifdef MUX_UART_FLOW_CTRL

    ATC_TaskStatus_Set(SCI_TRUE);
#endif

    for(;;)
    {
        // Receive signal from queue
        SCI_RECEIVE_SIGNAL(sig_ptr, P_ATC);

        ATC_Task_Dispatch(sig_ptr);

        if(s_atc_global_info.buffered_at_cmd_count > 16)
        {
            ATC_TRACE_LOW("ATC: CMD COUNT ERROR %d, SigCode=%d",
                          s_atc_global_info.buffered_at_cmd_count, sig_ptr->SignalCode);
#ifndef WIN32
            SCI_ASSERT(0);/*assert verified: check valid value*/
#endif
        }

#ifndef WIN32
#ifdef _MUX_ENABLE_
        if(s_atc_global_info.atc_link_info.atc_info_ptr != PNULL)
        {
            uint32 atc_ptr_value = (uint32)(s_atc_global_info.atc_link_info.atc_info_ptr);

            if(0 != (atc_ptr_value & 0x3))
            {
                ATC_TRACE_LOW("ATC: Error atc_info_ptr = %x", atc_ptr_value);
                SCI_ASSERT(0);/*assert verified: check valid value*/
            }
        }
#endif
#endif

        SCI_FREE_SIGNAL(sig_ptr);
    }
}

void ATC_Task_Dispatch(xSignalHeaderRec *sig_ptr)
{
    ATC_STATUS  status              = S_ATC_SUCCESS;
    BOOLEAN     recv_event_result   = TRUE;
    MN_DUAL_SYS_E    dual_sys  = MN_DUAL_SYS_1;
#ifdef _MUX_ENABLE_
    uint8           link_id = 0;
#endif

    switch(sig_ptr->SignalCode)
    {
#ifdef _MUX_ENABLE_
        case ATC_MUX_LINK_SETUP_REQ:
            ATC_TRACE_LOW("ATC: ATC_MUX_LINK_SETUP_REQ, link_id:%d", ((ATC_MUX_LINK_SETUP_REQ_T *)sig_ptr)->link_id);
            ATC_Mux_Setup_Link(((ATC_MUX_LINK_SETUP_REQ_T *)sig_ptr)->link_id);
            break;

        case ATC_MUX_LINK_RELEASE_REQ:
            ATC_TRACE_LOW("ATC: ATC_MUX_LINK_RELEASE_REQ, link_id:%d", ((ATC_MUX_LINK_SETUP_REQ_T *)sig_ptr)->link_id);
            ATC_Mux_Release_Link(((ATC_MUX_LINK_SETUP_REQ_T *)sig_ptr)->link_id, FALSE);
            break;

        case ATC_MUX_CLOSE_REQ:
            ATC_TRACE_LOW("ATC: ATC_MUX_CLOSE_REQ");
            break;

        case ATC_MUX_RECV_NEW_AT:
            ATC_TRACE_LOW("ATC: ATC_MUX_RECV_NEW_AT, link_id:%d,len:%d,close:%d,count:%d",
                          ((ATC_MUX_RECV_NEW_AT_T *)sig_ptr)->link_id, ((ATC_MUX_RECV_NEW_AT_T *)sig_ptr)->len,
                          is_close_atc_queue, s_atc_global_info.buffered_at_cmd_count);

            ATC_Set_Current_Link_id(S_CONFIG_INFO_PTR, ((ATC_MUX_RECV_NEW_AT_T *)sig_ptr)->link_id);
            ATC_SET_CURRENT_CARD_ID_MUX(ATC_MUX_RECV_NEW_AT_T);

            if(is_close_atc_queue)
            {
                if(0 == s_atc_global_info.buffered_at_cmd_count)
                {
                    ATC_TRACE_LOW("ATC: buffered_at_cmd_count is zero in close");
                }

                if(s_atc_global_info.buffered_at_cmd_count > 0)
                {
                    s_atc_global_info.buffered_at_cmd_count--;
                }

                if(s_atc_global_info.buffered_at_cmd_count <= 10)
                {
                    is_close_atc_queue = FALSE;
                }
            }
            else
            {
                if(0 == s_atc_global_info.buffered_at_cmd_count)
                {
                    ATC_TRACE_LOW("ATC: buffered_at_cmd_count is zero in NOT close ");
                }

                if(s_atc_global_info.buffered_at_cmd_count > 0)
                {
                    s_atc_global_info.buffered_at_cmd_count--;
                }

#ifdef _MODEM_MODE
                SCI_ATC_EnableDeepSleep(0);
#endif
            }

            status = ATC_RecNewLineSig(&s_atc_global_info, sig_ptr);

            if(S_ATC_SUCCESS == status)
            {
                ATC_ProcessCmds(&s_atc_global_info);
            }
            else if(S_ATC_DEFAULT_HANDLE == status)
            {
                break;
            }
            else
            {
                ATCS_TRACE_STATUS(status);
            }

#ifdef _MODEM_MODE

            if(ATC_SleepIsAllowed())
#endif
            {
            #ifndef ATC_SAT_ENABLE
                AtcReStartSleepTimer(ATC_SLEEP_TIMER_VALUE);
            #endif
            }

        #ifdef _MODEM_MODE
            else
            {
                // we can not sleep at all because s32k is set to 0
                // if timer is active, we must disable that timer
                if(SCI_IsTimerActive(s_atc_sleep_timer))
                {
                    SCI_DeactiveTimer(s_atc_sleep_timer);
                }
            }

        #endif
            break;

#else
            // Receive a new command line
        case SIO_RECV_NEW_AT:
            ATC_TRACE_LOW("ATC: SIO_RECV_NEW_AT");

            if(is_close_atc_queue)
            {
                if(s_atc_global_info.buffered_at_cmd_count > 0)
                {
                    s_atc_global_info.buffered_at_cmd_count--;
                }

                if(s_atc_global_info.buffered_at_cmd_count == 0)
                {
                    is_close_atc_queue = FALSE;
                }

                ATC_BuildResultCodeRsp(S_CONFIG_INFO_PTR, S_ATC_RESULT_CODE_ERROR, ERR_AT_COMMAND_DISCARDED);

                break;//just discard the message
            }
            else
            {
                if(s_atc_global_info.buffered_at_cmd_count > 0)
                {
                    s_atc_global_info.buffered_at_cmd_count -= 1;
                }

                // disable atc sleep, we will start of disable that timer after
                // we handle all at command.
            #ifdef _MODEM_MODE
                SCI_ATC_EnableDeepSleep(0);
            #endif
            }

            status = ATC_RecNewLineSig(&s_atc_global_info, sig_ptr);

            if(S_ATC_SUCCESS == status)
            {
                ATC_ProcessCmds(&s_atc_global_info);
            }
            else if(S_ATC_DEFAULT_HANDLE == status)
            {
                break;
            }
            else
            {
                ATCS_TRACE_STATUS(status);
            }

            // bo.chen after we proceed all at command, we start to check the status
            // of atc, if s32k = 0, then we shall not start that timer.
    #ifndef ATC_SAT_ENABLE
        #ifdef _MODEM_MODE
            if(ATC_SleepIsAllowed())
        #endif
            {
                AtcReStartSleepTimer(ATC_SLEEP_TIMER_VALUE);
            }

        #ifdef _MODEM_MODE
            else
            {
                // we can not sleep at all because s32k is set to 0
                // if timer is active, we must disable that timer
                if(SCI_IsTimerActive(s_atc_sleep_timer))
                {
                    SCI_DeactiveTimer(s_atc_sleep_timer);
                }
            }

         #endif
            break;
    #endif /*ATC_SAT_ENABLE*/
#endif

#ifdef _SUPPORT_GPRS_
        case APP_MN_DEACTIVATE_PDP_CONTEXT_IND:
            ATC_TRACE_LOW("ATC: ATC_Task, Signal Code: APP_MN_DEACTIVATE_PDP_CONTEXT_IND");
        #ifdef _MUX_ENABLE_
            dual_sys = ((APP_MN_GPRS_DEACTIVATE_IND_T *)sig_ptr)->dual_sys;
            recv_event_result = ATC_Receive_Expected_Event(S_CONFIG_INFO_PTR, APP_MN_ACTIVATE_PDP_CONTEXT_CNF,
                                                     ((APP_MN_GPRS_DEACTIVATE_IND_T *)sig_ptr)->pdp_id[0], 
                                                     ATC_DOMAIN_PS, dual_sys);
            if(!recv_event_result)
            {
                recv_event_result = ATC_Receive_Expected_Event(S_CONFIG_INFO_PTR, APP_MN_DEACTIVATE_PDP_CONTEXT_CNF,
                                                     ((APP_MN_GPRS_DEACTIVATE_IND_T *)sig_ptr)->pdp_id[0], 
                                                     ATC_DOMAIN_PS, dual_sys);
                if(recv_event_result)
                {
                    ATC_BuildResultCodeRsp(S_CONFIG_INFO_PTR, S_ATC_RESULT_CODE_OK, 0);
                }
                else
                {
                    //set unsolicited report link id
                    ATC_Set_Current_Link_id(S_CONFIG_INFO_PTR, ATC_GetMuxLinkIdFromSimId(dual_sys));
                }
            }
            else
            {
                sprintf((char*)g_rsp_str, "+CME ERROR: %d", MN_GPRS_ERR_TEMPORARILY_BLOCKED);
                ATC_BuildInfoRsp(S_CONFIG_INFO_PTR, (uint8*)g_rsp_str); //this error indicates APP try dial again
            }
            ATC_Clear_All_Exp_Result(((APP_MN_GPRS_DEACTIVATE_IND_T *)sig_ptr)->pdp_id[0], ATC_DOMAIN_PS, dual_sys);
        #endif

            ATC_BuildGprsEvReportRsp(S_CONFIG_INFO_PTR, sig_ptr);
            ATC_UpCENDInfo(S_CONFIG_INFO_PTR, (xSignalHeaderRec *)sig_ptr);
            ATC_ProcessDeactivatePDPContextIND(S_CONFIG_INFO_PTR, ((APP_MN_GPRS_DEACTIVATE_IND_T *)sig_ptr));
            break;

            // the following signals for GPRS services
        case APP_MN_ACTIVATE_PDP_CONTEXT_CNF:
            ATC_TRACE_LOW("ATC: ATC_Task, Signal Code: APP_MN_ACTIVATE_PDP_CONTEXT_CNF");
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     ((APP_MN_GPRS_EXT_T *)sig_ptr)->pdp_id, ATC_DOMAIN_PS,
                                     ((APP_MN_GPRS_EXT_T *)sig_ptr)->dual_sys);
            ATC_ProcessActivatePDPContextCNF(S_CONFIG_INFO_PTR, (APP_MN_GPRS_EXT_T *)sig_ptr, recv_event_result);
            break;

        case APP_MN_DEACTIVATE_PDP_CONTEXT_CNF:
            ATC_TRACE_LOW("ATC: ATC_Task, Signal Code: APP_MN_DEACTIVATE_PDP_CONTEXT_CNF, result:%d,pdp_num:%d,cid:%d, set atc_err_protected_flag",
                          ((APP_MN_GPRS_DEACTIVATE_CNF_T *)sig_ptr)->result,
                          ((APP_MN_GPRS_DEACTIVATE_CNF_T *)sig_ptr)->pdp_num,
                          ((APP_MN_GPRS_DEACTIVATE_CNF_T *)sig_ptr)->pdp_id[0]);

        #ifdef _MUX_ENABLE_
            dual_sys = ((APP_MN_GPRS_DEACTIVATE_CNF_T *)sig_ptr)->dual_sys;
            /*First, check whether deactivate PDP request specified cid or not*/
            recv_event_result = ATC_Receive_Expected_Event(S_CONFIG_INFO_PTR, sig_ptr->SignalCode,
                                                     ATC_CHECKID_FOR_DEACTIVATE_ALL_PDP, ATC_DOMAIN_PS,
                                                     dual_sys);

            if(!recv_event_result)
            {
                /*specified cid procedure.*/
                recv_event_result = ATC_Receive_Expected_Event(S_CONFIG_INFO_PTR, sig_ptr->SignalCode,
                                                     ((APP_MN_GPRS_DEACTIVATE_CNF_T *)sig_ptr)->pdp_id[0],
                                                     ATC_DOMAIN_PS, dual_sys);
            }

            if(MN_GPRS_ERR_SUCCESS == ((APP_MN_GPRS_DEACTIVATE_CNF_T *)sig_ptr)->result)
            {
                ATC_Clear_All_Exp_Result(((APP_MN_GPRS_DEACTIVATE_CNF_T *)sig_ptr)->pdp_id[0],
                                                            ATC_DOMAIN_PS, dual_sys);
            }
        #endif

            ATC_ProcessDeactivatePDPContextCNF(S_CONFIG_INFO_PTR, ((APP_MN_GPRS_DEACTIVATE_CNF_T *)sig_ptr), recv_event_result);
            break;

        case APP_MN_SET_QOS_CNF:
        case APP_MN_SET_MIN_QOS_CNF:
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     ((APP_MN_GPRS_EXT_T *)sig_ptr)->pdp_id, ATC_DOMAIN_PS,
                                     ((APP_MN_GPRS_EXT_T *)sig_ptr)->dual_sys);
        #ifdef _MUX_ENABLE_
            if(recv_event_result)
        #endif
            {
                if(((APP_MN_GPRS_EXT_T *)sig_ptr)->result == MN_GPRS_ERR_SUCCESS)
                {
                    ATC_BuildResultCodeRsp(S_CONFIG_INFO_PTR, S_ATC_RESULT_CODE_OK, 0);
                }
                else
                {
                    ATC_BuildResultCodeRsp(S_CONFIG_INFO_PTR, S_ATC_RESULT_CODE_ERROR, ((APP_MN_GPRS_EXT_T *)sig_ptr)->result);
                }
            }
            break;

        case APP_MN_GPRS_ATTACH_RESULT:
            ATC_ProcessGprsAttachResult(S_CONFIG_INFO_PTR, (APP_MN_GPRS_ATTACH_RESULT_T *)sig_ptr);
            break;

        case APP_MN_GPRS_DETACH_RESULT:
            ATC_ProcessGprsDetachResult(S_CONFIG_INFO_PTR, (APP_MN_GPRS_DETACH_RESULT_T *)sig_ptr);
            break;
#endif

            // the following sigals for call control
        case APP_MN_CALL_START_IND:
            dual_sys = ((APP_MN_CALL_START_IND_T *)sig_ptr)->dual_sys;
            // judge if emergence firstly
            if((((APP_MN_CALL_START_IND_T *)sig_ptr)->call_type == MN_CALL_TYPE_EMERGENCY)
                && ((g_calls_state[dual_sys].call_amount < MN_CALL_MAX_CALL_NUM)))
            {
                g_calls_state[dual_sys].call_context[g_calls_state[dual_sys].call_amount].is_emergency_call = TRUE;
            }
            
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     ((APP_MN_CALL_START_IND_T *)sig_ptr)->call_id, ATC_DOMAIN_CS, dual_sys);
            ATC_ProcessCallStartInd(S_CONFIG_INFO_PTR, (APP_MN_CALL_START_IND_T *)sig_ptr);
            break;

        case APP_MN_ACMMAX_IND:
            ATC_GET_MUX_LINK_ID(link_id, ((APP_MN_ACMMAX_IND_T *)sig_ptr)->dual_sys);
            ATC_SET_CURRENT_LINK_ID_MUX(link_id);
            ATC_ProcessAcmInd(S_CONFIG_INFO_PTR, (APP_MN_ACMMAX_IND_T *)sig_ptr);
            break;

        case APP_MN_SETUP_IND:
            ATC_ProcessSetupInd(S_CONFIG_INFO_PTR, (APP_MN_SETUP_IND_T *)sig_ptr);

            /*add condition for EM, if it set the s0 num*/
            if(s_config_info.atc_config_nv.spauto_flag & 0x1)
            {
                ATC_TRACE_LOW("ATC: ATC_Task, auto answer call, id:%d,type:%d",
                              ((APP_MN_SETUP_IND_T *)sig_ptr)->call_id,
                              ((APP_MN_SETUP_IND_T *)sig_ptr)->call_type);

                MNCALL_IndicateAlertEx(((APP_MN_SETUP_IND_T *)sig_ptr)->dual_sys, ((APP_MN_SETUP_IND_T *)sig_ptr)->call_id, PNULL); //add for ophone spauto_flg
                MNCALL_ConnectCallEx(((APP_MN_SETUP_IND_T *)sig_ptr)->dual_sys, ((APP_MN_SETUP_IND_T *)sig_ptr)->call_id, PNULL);

                g_atc_call_flag = TRUE;
            }

            break;

        case APP_MN_ALERTING_IND:
#ifdef _MUX_ENABLE_
            ATC_GET_MUX_LINK_ID(link_id, ((APP_MN_ALERTING_IND_T *)sig_ptr)->dual_sys);
            /*to record call link id in case of call expire message*/
            s_config_info.call_link_id[((APP_MN_ALERTING_IND_T *)sig_ptr)->call_id] = link_id;
#endif
            ATC_ProcessAlertingInd(S_CONFIG_INFO_PTR, (APP_MN_ALERTING_IND_T *)sig_ptr);
            break;
            
        case APP_MN_PROGRESS_IND:
            ATC_ProcessProgressInd(S_CONFIG_INFO_PTR, (APP_MN_PROGRESS_IND_T *)sig_ptr);
            break;
            
         case APP_MN_DISC_INFO_IND:
            ATC_ProcessDiscInfoInd(S_CONFIG_INFO_PTR, (APP_MN_DISC_INFO_IND_T *)sig_ptr);
            break;
            
        case ATC_CALL_EXP_IND:
            ATC_SET_CURRENT_LINK_ID_MUX(s_config_info.call_link_id[g_atc_expire_call_id]);
            ATC_ProcessCallExpInd(S_CONFIG_INFO_PTR);
            break;

        case ATC_NET_RING_EXP_IND:
            ATC_ProcessNetRingExpInd(S_CONFIG_INFO_PTR);
            break;

        case ATC_POWEROFF_EXP_IND:
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, APP_MN_DEACTIVE_PS_CNF,
                                     0xff, ATC_NO_DOMAIN, ATC_CURRENT_SIM_ID_MUX);
            ATC_BuildResultCodeRsp(S_CONFIG_INFO_PTR, S_ATC_RESULT_CODE_OK, 0);
#ifndef _SUPPORT_IPC_
            POWER_PowerOff();
#endif
            break;

        case APP_MN_SETUP_COMPLETE_IND:
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     ((APP_MN_SETUP_COMPLETE_IND_T *)sig_ptr)->call_id, ATC_DOMAIN_CS,
                                     ((APP_MN_SETUP_COMPLETE_IND_T *)sig_ptr)->dual_sys);
            ATC_ProcessSetupCompleteSignal(S_CONFIG_INFO_PTR, sig_ptr);
            break;

        case APP_MN_SETUP_COMPLETE_CNF:
            ATC_ProcessSetupCompleteSignal(S_CONFIG_INFO_PTR, sig_ptr);
            break;

#ifdef _MUX_ENABLE_
        case APP_MN_CALL_DISCONNECTED_IND:
        case APP_MN_CALL_ERR_IND:
            dual_sys = ((APP_MN_CALL_DISCONNECTED_IND_T *)sig_ptr)->dual_sys;
            SCI_TRACE_LOW("ATC: recv call disc,  ss_operate = %d", s_config_info.ss_operate[dual_sys]);
            
            CheckCallEvent(dual_sys, ((APP_MN_CALL_DISCONNECTED_IND_T *)sig_ptr)->call_id);
            
            ATC_Receive_Expected_Event(S_CONFIG_INFO_PTR, sig_ptr->SignalCode,
                                    ((APP_MN_CALL_DISCONNECTED_IND_T *)sig_ptr)->call_id,
                                    ATC_DOMAIN_CS, dual_sys);

            if(g_calls_state[dual_sys].call_amount != 0 &&
               g_calls_state[dual_sys].call_context[g_calls_state[dual_sys].call_amount - 1].call_mode == ATC_DATA_MODE)
            {
                if (APP_MN_CALL_DISCONNECTED_IND == sig_ptr->SignalCode)
                {
                    ATC_TRACE_LOW("ATC: ATC_Task, APP_MN_CALL_DISCONNECTED_IND, disconnect VT mux link, call_id:%d, cause:%d",
                                  ((APP_MN_CALL_DISCONNECTED_IND_T *)sig_ptr)->call_id,
                                  ((APP_MN_CALL_DISCONNECTED_IND_T *)sig_ptr)->disconnected_cause);
                }
                else
                {
                    ATC_TRACE_LOW("ATC: ATC_Task, APP_MN_CALL_ERR_IND, disconnect VT mux link, call_id:%d, cause:%d",
                                  ((APP_MN_CALL_ERR_IND_T *)sig_ptr)->call_id,
                                  ((APP_MN_CALL_ERR_IND_T *)sig_ptr)->error_cause);
                }
            }

            ATC_ProcessCallDiscInd(S_CONFIG_INFO_PTR, sig_ptr);
            break;

#else
        case APP_MN_CALL_DISCONNECTED_IND:
        case APP_MN_CALL_ERR_IND:
            ATC_ProcessCallDiscInd(S_CONFIG_INFO_PTR, sig_ptr);
            break;
#endif

        case APP_MN_SYNC_IND:       //Can send DTMF tone when the signal received.Tch is already assigned by network.
            ATC_ProcessMnSyncInd(S_CONFIG_INFO_PTR, (APP_MN_SYNC_IND_T *)sig_ptr);
            break;

        case APP_MN_DISCONNECTING_IND:
            ATC_ProcessDisconnectingInd(S_CONFIG_INFO_PTR, (APP_MN_DISCONNECTING_IND_T *)sig_ptr);
            break;

        case APP_MN_CALL_HELD_CNF:

            if(((APP_MN_CALL_HELD_CNF_T *)sig_ptr)->req_is_accepted)
            {
                ATC_ResetDTMFInfo(S_CONFIG_INFO_PTR, ((APP_MN_CALL_HELD_CNF_T *)sig_ptr)->dual_sys);
            }

            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     ((APP_MN_CALL_HELD_CNF_T *)sig_ptr)->call_id, ATC_DOMAIN_CS,
                                     ((APP_MN_CALL_HELD_CNF_T *)sig_ptr)->dual_sys);
            ATC_ProcessHeldCnf(S_CONFIG_INFO_PTR, (APP_MN_CALL_HELD_CNF_T *)sig_ptr);
            break;
            
#ifndef ATC_SAT_ENABLE
        case APP_MN_READ_DSP_MEM_CNF:
            ATC_TRACE_LOW("ATC:call ATC_ProcessReadDSPCnf");
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     0xff, ATC_NO_DOMAIN, ATC_CURRENT_SIM_ID_MUX);
            ATC_ProcessReadDSPCnf(S_CONFIG_INFO_PTR, (APP_MN_READ_DSP_MEM_CNF_T *)sig_ptr);
            break;
#endif

        case APP_MN_DEACTIVE_PS_CNF:
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     0xff, ATC_NO_DOMAIN, ((APP_MN_DEACTIVE_PS_CNF_T *)sig_ptr)->dual_sys);
            ATC_ProcessPowerOffCnf(S_CONFIG_INFO_PTR, (APP_MN_DEACTIVE_PS_CNF_T *)sig_ptr);
            break;

        case APP_MN_NETWORK_STATUS_IND:
            ATC_ProcessNetworkStatusInd(S_CONFIG_INFO_PTR, (APP_MN_NETWORK_STATUS_IND_T *)sig_ptr);
            break;

        case APP_MN_NETWORK_INFO_IND:
            ATC_ProcessNetworkInfoInd(S_CONFIG_INFO_PTR, (APP_MN_NETWORK_INFO_IND_T *)sig_ptr);
            break;

        case APP_MN_CALL_FORWARD_IND:
            ATC_ProcessForwardInd(S_CONFIG_INFO_PTR, (APP_MN_CALL_FORWARD_IND_T *)sig_ptr);
            break;

        case APP_MN_CALL_WAIT_IND:
            ATC_ProcessWaitingInd(S_CONFIG_INFO_PTR, (APP_MN_CALL_WAIT_IND_T *)sig_ptr);
            break;

        case APP_MN_CLIR_SUPP_REJ_IND:
            ATC_ProcessClirSuppRejInd(S_CONFIG_INFO_PTR, (APP_MN_CLIR_SUPP_REJ_IND_T *)sig_ptr);
            break;

        case APP_MN_CALL_DEFLECT_IND:
            ATC_ProcessCallDeflectInd(S_CONFIG_INFO_PTR, (APP_MN_CALL_DEFLECT_IND_T *)sig_ptr);
            break;

        case APP_MN_CALL_BARRED_IND:
            ATC_ProcessBarredInd(S_CONFIG_INFO_PTR, (APP_MN_CALL_BARRED_IND_T *)sig_ptr);
            break;

        case APP_MN_FORWARD_STATUS_IND:
            ATC_ProcessForwardStatusInd(S_CONFIG_INFO_PTR, (APP_MN_FORWARD_STATUS_IND_T *)sig_ptr);
            break;

        case APP_MN_CALL_HELD_IND:
            ATC_ResetDTMFInfo(S_CONFIG_INFO_PTR, ((APP_MN_CALL_HELD_IND_T *)sig_ptr)->dual_sys);
            ATC_ProcessHeldInd(S_CONFIG_INFO_PTR, (APP_MN_CALL_HELD_IND_T *)sig_ptr);
            break;

        case APP_MN_CALL_RETRIEVED_IND:
            ATC_ProcessRetrievedInd(S_CONFIG_INFO_PTR, (APP_MN_CALL_RETRIEVED_IND_T *)sig_ptr);
            break;

        case APP_MN_BUILD_MPTY_IND:
            ATC_ProcessBuildMptyInd(S_CONFIG_INFO_PTR, (APP_MN_BUILD_MPTY_IND_T *)sig_ptr);
            break;

        case APP_MN_BUILD_MPTY_CNF:
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     ((APP_MN_BUILD_MPTY_CNF_T *)sig_ptr)->call_id, ATC_DOMAIN_CS,
                                     ((APP_MN_BUILD_MPTY_CNF_T *)sig_ptr)->dual_sys);
            ATC_ProcessBuildMptyCnf(S_CONFIG_INFO_PTR, (APP_MN_BUILD_MPTY_CNF_T *)sig_ptr);
            break;
            
        case APP_MN_ECT_INVOKE_CNF:
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     ((APP_MN_ECT_INVOKE_CNF_T *)sig_ptr)->call_id, ATC_DOMAIN_CS,
                                     ((APP_MN_ECT_INVOKE_CNF_T *)sig_ptr)->dual_sys);
            ATC_ProcessEctInvokeCnf(S_CONFIG_INFO_PTR, (APP_MN_ECT_INVOKE_CNF_T *)sig_ptr);
            break;

        case APP_MN_SPLIT_MPTY_CNF:
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     ((APP_MN_SPLIT_MPTY_CNF_T *)sig_ptr)->call_id, ATC_DOMAIN_CS, 
                                     ((APP_MN_SPLIT_MPTY_CNF_T *)sig_ptr)->dual_sys);
            ATC_ProcessSplitMptyCnf(S_CONFIG_INFO_PTR, (APP_MN_SPLIT_MPTY_CNF_T *)sig_ptr);
            break;

        case APP_MN_CALL_RETRIEVED_CNF:
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     ((APP_MN_CALL_RETRIEVED_CNF_T *)sig_ptr)->call_id, ATC_DOMAIN_CS,
                                     ((APP_MN_CALL_RETRIEVED_CNF_T *)sig_ptr)->dual_sys);
            ATC_ProcessCallRetrievCnf(S_CONFIG_INFO_PTR, (APP_MN_CALL_RETRIEVED_CNF_T *)sig_ptr);
            break;

            // The following signals for SMS service
        case APP_MN_SMS_READY_IND:
            ATC_ProcessSmsReadyInd(S_CONFIG_INFO_PTR, (APP_MN_SMS_READY_IND_T *)sig_ptr);
            break;

        case APP_MN_READ_SMS_CNF:
#ifdef _MUX_ENABLE_
            dual_sys = ((APP_MN_READ_SMS_CNF_T *)sig_ptr)->dual_sys;
            if (!s_config_info.call_read_flag[dual_sys])
            {
                 if (!ATC_Check_Expected_Event(s_config_info.current_link_id,APP_MN_READ_SMS_CNF,0xff,dual_sys))
                    break;//discard APP_MN_READ_SMS_CNF msg in initial process
            }
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     0xff, ATC_NO_DOMAIN, dual_sys);
            if(!recv_event_result)
            {
                ATC_TRACE_LOW("ATC, WARNING, APP_MN_READ_SMS_CNF is not for the request of ATC");
            }
            else
#endif
            {
                ATC_ProcessReadSmsCnf(S_CONFIG_INFO_PTR, (APP_MN_READ_SMS_CNF_T *)sig_ptr);
            }
            break;

        case APP_MN_SMS_IND:
            ATC_ProcessSmsInd(S_CONFIG_INFO_PTR, (APP_MN_SMS_IND_T *)sig_ptr);
            break;

        case APP_MN_SEND_SMS_CNF:
#ifdef _MUX_ENABLE_
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     0xff, ATC_NO_DOMAIN, ((APP_MN_SEND_SMS_CNF_T *)sig_ptr)->dual_sys);
            if(!recv_event_result)
            {
                ATC_TRACE_LOW("ATC, WARNING, APP_MN_SEND_SMS_CNF is not for the request of ATC");
            }
            else
#endif
            {
            #ifndef ATC_SAT_ENABLE
                if(TRUE == g_is_stk_sendsms[((APP_MN_SEND_SMS_CNF_T *)sig_ptr)->dual_sys])
                {
                    ATC_STKProcessSendSmsCnf(S_CONFIG_INFO_PTR, (APP_MN_SEND_SMS_CNF_T *)sig_ptr);
                }
                else
            #endif
                {
                    ATC_ProcessSendSmsCnf(S_CONFIG_INFO_PTR, (APP_MN_SEND_SMS_CNF_T *)sig_ptr);
                }
            }
            break;

        case APP_MN_SMS_STATUS_REPORT_IND:
            ATC_ProcessSmsStatusReportInd(S_CONFIG_INFO_PTR, (APP_MN_SMS_STATUS_REPORT_IND_T *)sig_ptr);
            break;

        case APP_MN_SMS_MEM_FULL_IND:
            ATC_ProcessSmsFullInd(S_CONFIG_INFO_PTR, (APP_MN_SMS_MEM_FULL_IND_T *)sig_ptr);
            break;

        case APP_MN_WRITE_SMS_CNF:
#ifdef _MUX_ENABLE_
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     0xff, ATC_NO_DOMAIN, ((APP_MN_WRITE_SMS_CNF_T *)sig_ptr)->dual_sys);
            if(!recv_event_result)
            {
                ATC_TRACE_LOW("ATC, WARNING, APP_MN_WRITE_SMS_CNF is not for the request of ATC");
            }
            else
#endif
            {
                ATC_ProcessWriteSmsCnf(S_CONFIG_INFO_PTR, (APP_MN_WRITE_SMS_CNF_T *)sig_ptr);
            }
            break;

        case APP_MN_UPDATE_SMS_STATE_CNF:
#ifdef _MUX_ENABLE_
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     0xff, ATC_NO_DOMAIN, ((APP_MN_UPDATE_SMS_STATUS_CNF_T *)sig_ptr)->dual_sys);
            if(!recv_event_result)
            {
                ATC_TRACE_LOW("ATC, WARNING, APP_MN_UPDATE_SMS_STATE_CNF is not for the request of ATC");
            }
            else
#endif
            {
                ATC_ProcessUpdateSmsCnf(S_CONFIG_INFO_PTR, (APP_MN_UPDATE_SMS_STATUS_CNF_T *)sig_ptr);
            }
            break;

        case APP_MN_DELETE_ALL_SMS_CNF:
#ifdef _MUX_ENABLE_
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     0xff, ATC_NO_DOMAIN, ((APP_MN_DELETE_ALL_SMS_CNF_T *)sig_ptr)->dual_sys);
            if(!recv_event_result)
            {
                ATC_TRACE_LOW("ATC, WARNING, APP_MN_DELETE_ALL_SMS_CNF is not for the request of ATC");
            }
            else
#endif
            {
                ATC_ProcessDeleteAllSmsCnf(S_CONFIG_INFO_PTR, (APP_MN_DELETE_ALL_SMS_CNF_T *)sig_ptr);
            }
            break;

        case APP_MN_AOC_MODIFIED_IND:
            ATC_ProcessIncreaseAcmInd(S_CONFIG_INFO_PTR, (APP_MN_AOC_MODIFIED_IND_T *)sig_ptr);
            break;

            // the following signals for cell broadcast service
    #ifndef ATC_SAT_ENABLE
        case APP_MN_SMSCB_MSG_PAGES_IND:
            ATC_ProcessSmscbMsgPagesInd(S_CONFIG_INFO_PTR, (APP_MN_SMSCB_MSG_PAGES_IND_T *)sig_ptr);
            break;
    #endif

            // The following are signals for SS.
        case APP_MN_INTERROGATE_CNF:
           
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     0xff, ATC_NO_DOMAIN, ((APP_MN_INTERROGATE_CNF_T *)sig_ptr)->dual_sys);
        #ifndef ATC_SAT_ENABLE
            if(g_is_stk_sendss[((APP_MN_INTERROGATE_CNF_T *)sig_ptr)->dual_sys])
            {
                ATC_STKProcessSendSsCnf(S_CONFIG_INFO_PTR,ATC_SS_SERVICE_CNF_INTERROGATE,(void *)sig_ptr);
            }
            else
        #endif
            {
                ATC_ProcessInterCnf(S_CONFIG_INFO_PTR, (APP_MN_INTERROGATE_CNF_T *)sig_ptr);
            }

            break;

        case APP_MN_DEACTIVATE_SS_CNF:
            ATC_TRACE_LOW("ATC:APP_MN_DEACTIVATE_SS_CNF");
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     0xff, ATC_NO_DOMAIN, ((APP_MN_DEACTIVATE_SS_CNF_T *)sig_ptr)->dual_sys);
        #ifndef ATC_SAT_ENABLE
            if(g_is_stk_sendss[((APP_MN_DEACTIVATE_SS_CNF_T *)sig_ptr)->dual_sys])
            {
                ATC_STKProcessSendSsCnf(S_CONFIG_INFO_PTR,ATC_SS_SERVICE_CNF_DEACTIVATE,(void *)sig_ptr);
            }
            else
        #endif
            {
                ATC_ProcessDeactCnf(S_CONFIG_INFO_PTR, (APP_MN_DEACTIVATE_SS_CNF_T *)sig_ptr);
            }

            break;

        case APP_MN_ACTIVATE_SS_CNF:
            ATC_TRACE_LOW("ATC:APP_MN_ACTIVATE_SS_CNF");
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode, 
                                     0xff, ATC_NO_DOMAIN, ((APP_MN_ACTIVATE_SS_CNF_T *)sig_ptr)->dual_sys);
        #ifndef ATC_SAT_ENABLE
            if(g_is_stk_sendss[((APP_MN_ACTIVATE_SS_CNF_T *)sig_ptr)->dual_sys])
            {
                ATC_STKProcessSendSsCnf(S_CONFIG_INFO_PTR,ATC_SS_SERVICE_CNF_ACTIVATE,(void *)sig_ptr);
            }
            else
        #endif
            {
                ATC_ProcessActCnf(S_CONFIG_INFO_PTR, (APP_MN_ACTIVATE_SS_CNF_T *)sig_ptr);
            }

            break;

        case APP_MN_ERASE_SS_CNF:
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     0xff, ATC_NO_DOMAIN, ((APP_MN_ERASE_SS_CNF_T *)sig_ptr)->dual_sys);
        #ifndef ATC_SAT_ENABLE
            if(g_is_stk_sendss[((APP_MN_ERASE_SS_CNF_T *)sig_ptr)->dual_sys])
            {
                ATC_STKProcessSendSsCnf(S_CONFIG_INFO_PTR,ATC_SS_SERVICE_CNF_ERASE,(void *)sig_ptr);
            }
            else
        #endif
            {
                ATC_ProcessEraseCnf(S_CONFIG_INFO_PTR, (APP_MN_ERASE_SS_CNF_T *)sig_ptr);
            }

            break;

        case APP_MN_REGISTER_SS_CNF:
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     0xff, ATC_NO_DOMAIN, ((APP_MN_REGISTER_SS_CNF_T *)sig_ptr)->dual_sys);
        #ifndef ATC_SAT_ENABLE
            if(g_is_stk_sendss[((APP_MN_REGISTER_SS_CNF_T *)sig_ptr)->dual_sys])
            {
                ATC_STKProcessSendSsCnf(S_CONFIG_INFO_PTR,ATC_SS_SERVICE_CNF_REGISTER,(void *)sig_ptr);
            }
            else
        #endif
            {
                ATC_ProcessRegisterCnf(S_CONFIG_INFO_PTR, (APP_MN_REGISTER_SS_CNF_T *)sig_ptr);
            }

            break;

        case APP_MN_PUCT_INFO_CNF:
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result,sig_ptr->SignalCode, 
                                     0xff, ATC_NO_DOMAIN, ((APP_MN_PUCT_INFO_CNF_T *)sig_ptr)->dual_sys);
            ATC_ProcessPuctCnf(S_CONFIG_INFO_PTR, (APP_MN_PUCT_INFO_CNF_T *)sig_ptr);
            break;

            //The following three is about CUSD.
        case APP_MN_USSD_IND:
            ATC_ProcessUssdInd(S_CONFIG_INFO_PTR, (APP_MN_USSD_IND_T *)sig_ptr);
            break;

        case APP_MN_USSD_NOTIFY_IND:
            ATC_ProcessUssdNotifyInd(S_CONFIG_INFO_PTR, (APP_MN_USSD_NOTIFY_IND_T *)sig_ptr);
            break;

        case APP_MN_USSD_SERVICE_CNF:
        #ifndef ATC_SAT_ENABLE
            if(g_is_stk_sendussd[((APP_MN_USSD_SERVICE_CNF_T *)sig_ptr)->dual_sys])
            {
                ATC_STKProcessSendUssdCnf(S_CONFIG_INFO_PTR, (APP_MN_USSD_SERVICE_CNF_T *)sig_ptr);
            }
            else
        #endif
            {
                ATC_ProcessUssdServiceCnf(S_CONFIG_INFO_PTR, (APP_MN_USSD_SERVICE_CNF_T *)sig_ptr);
            }

            break;

        case APP_MN_RELEASE_SS_IND:
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     0xff, ATC_NO_DOMAIN, ((APP_MN_RELEASE_SS_IND_T *)sig_ptr)->dual_sys);
        #ifndef ATC_SAT_ENABLE
            if(g_is_stk_sendss[((APP_MN_RELEASE_SS_IND_T *)sig_ptr)->dual_sys])
            {
                ATC_STKProcessSendSsCnf(S_CONFIG_INFO_PTR, ATC_SS_SERVICE_CNF_RELEASE, (void *)sig_ptr);
            }
            else
        #endif
            {
                ATC_ProcessRelCnf(S_CONFIG_INFO_PTR, (APP_MN_RELEASE_SS_IND_T *)sig_ptr);
            }
            break;

        case APP_MN_GET_PASSWORD_IND:
            SCI_TRACE_MID("ATC:APP_MN_GET_PASSWORD_IND");
            ATC_ProcessGetPswInd(S_CONFIG_INFO_PTR, (APP_MN_GET_PASSWORD_IND_T *)sig_ptr);
            break;

        case APP_MN_REGISTER_PASSWORD_CNF:
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     0xff, ATC_NO_DOMAIN, ((APP_MN_REGISTER_PASSWORD_CNF_T *)sig_ptr)->dual_sys);
            SCI_TRACE_MID("ATC:APP_MN_REGISTER_PASSWORD_CNF");
        #ifndef ATC_SAT_ENABLE
            if(g_is_stk_sendss[((APP_MN_REGISTER_PASSWORD_CNF_T *)sig_ptr)->dual_sys])
            {
                ATC_STKProcessSendSsRegPwdCnf(S_CONFIG_INFO_PTR, (APP_MN_REGISTER_PASSWORD_CNF_T *)sig_ptr);
            }
            else
        #endif
            {
                ATC_ProcessPwdCnf(S_CONFIG_INFO_PTR, (APP_MN_REGISTER_PASSWORD_CNF_T *)sig_ptr);
            }

            break;

            // the following is about the SIM signals
        case APP_MN_SIM_NOT_READY_IND:
            if(s_config_info.cpin_flag[((APP_MN_SIM_NOT_READY_IND_T *)sig_ptr)->dual_sys])
            {
                ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, APP_MN_SIM_READY_IND,
                                        0xff, ATC_NO_DOMAIN, ((APP_MN_SIM_NOT_READY_IND_T *)sig_ptr)->dual_sys);
            }

            ATC_ProcessNotReadyInd(S_CONFIG_INFO_PTR, (APP_MN_SIM_NOT_READY_IND_T *)sig_ptr);
            break;

        case APP_MN_SIM_READY_IND:
            if(s_config_info.cpin_flag[((APP_MN_SIM_READY_IND_T *)sig_ptr)->dual_sys])
            {
                ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     0xff, ATC_NO_DOMAIN, ((APP_MN_SIM_READY_IND_T *)sig_ptr)->dual_sys);
                ATC_BuildResultCodeRsp(S_CONFIG_INFO_PTR, S_ATC_RESULT_CODE_OK, 0);
                s_config_info.cpin_flag[((APP_MN_SIM_READY_IND_T *)sig_ptr)->dual_sys] = FALSE;
            }

            ATC_ProcessReadyInd(S_CONFIG_INFO_PTR, (APP_MN_SIM_READY_IND_T *)sig_ptr);
            break;

        case APP_MN_SIM_PIN_FUNC_CNF:
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     0xff, ATC_NO_DOMAIN, ((APP_MN_SIM_PIN_FUNC_CNF_T *)sig_ptr)->dual_sys);
            ATC_ProcessChvCnf(S_CONFIG_INFO_PTR, (APP_MN_SIM_PIN_FUNC_CNF_T *)sig_ptr);
            ATC_UpECINDInfo(((APP_MN_SIM_PIN_FUNC_CNF_T *)sig_ptr)->dual_sys, S_CONFIG_INFO_PTR,  sig_ptr, 3, 3, 0, 0, 0);
            break;

        case APP_MN_SIM_GET_PIN_IND:
            if(s_config_info.cpin_flag[((APP_MN_SIM_GET_PIN_IND_T *)sig_ptr)->dual_sys])
            {
                ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, APP_MN_SIM_READY_IND,
                                        0xff, ATC_NO_DOMAIN, ((APP_MN_SIM_GET_PIN_IND_T *)sig_ptr)->dual_sys);
            }

            ATC_ProcessPinInd(S_CONFIG_INFO_PTR, (APP_MN_SIM_GET_PIN_IND_T *)sig_ptr);
            break;
        case APP_MN_GET_SIM_TYPE_CNF:
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode, 
                                     0xff, ATC_NO_DOMAIN, ((APP_MN_GET_SIM_TYPE_CNF_T *)sig_ptr)->dual_sys);
            ATC_ProcessSimTypeCnf(S_CONFIG_INFO_PTR, (APP_MN_GET_SIM_TYPE_CNF_T *)sig_ptr);
            break;

        case APP_MN_PLMN_SELECT_CNF:
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode, 
                                     0xff, ATC_NO_DOMAIN, ((APP_MN_PLMN_SELECT_CNF_T *)sig_ptr)->dual_sys);
            ATC_ProcessPlmnSelCnf(S_CONFIG_INFO_PTR, (APP_MN_PLMN_SELECT_CNF_T *)sig_ptr);
            break;

        case APP_MN_SCELL_RSSI_IND:
            ATC_ProcessRssiInd(S_CONFIG_INFO_PTR, (APP_MN_SCELL_RSSI_IND_T *)sig_ptr);
            ATC_ProcessRssiIndEx(S_CONFIG_INFO_PTR, (APP_MN_SCELL_RSSI_IND_T *)sig_ptr);
            break;

        case APP_MN_CALL_READY_IND:
            dual_sys = ((APP_MN_CALL_READY_IND_T *)sig_ptr)->dual_sys;
            s_config_info.call_read_flag[dual_sys] = TRUE;

            s_config_info.is_fdn_enable[dual_sys] = ((APP_MN_CALL_READY_IND_T *)sig_ptr)->fdn_is_enable;

            ATC_TRACE_LOW("ATC%d:APP_MN_CALL_READY_IND is_fdn_enable=%d",dual_sys,s_config_info.is_fdn_enable[dual_sys]);

            ATC_ProcessCallReadyIndByPB(dual_sys, S_CONFIG_INFO_PTR, ((APP_MN_CALL_READY_IND_T *)sig_ptr));
            break;
            
#ifndef ATC_SAT_ENABLE
        case APP_MN_FDN_SERVICE_IND:
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                               0xff, ATC_NO_DOMAIN, ((APP_MN_FDN_SERVICE_IND_T *)sig_ptr)->dual_sys);
            ATC_ProcessFDNServiceInd(S_CONFIG_INFO_PTR, (APP_MN_FDN_SERVICE_IND_T *)sig_ptr);
            break;

        case APP_MN_FDN_CAPABILITY_IND:
            dual_sys = ((APP_MN_FDN_CAPABILITY_IND_T *)sig_ptr)->dual_sys;
            s_config_info.is_fdn_ready[dual_sys] = TRUE;
            s_config_info.is_fdn_enable[dual_sys] = ((APP_MN_FDN_CAPABILITY_IND_T *)sig_ptr)->is_fdn_enable;
            break;
#endif
        case APP_MN_PLMN_LIST_CNF:
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     0xff, ATC_NO_DOMAIN, ((APP_MN_PLMN_LIST_CNF_T *)sig_ptr)->dual_sys);
            ATC_ProcessPlmnListCnf(S_CONFIG_INFO_PTR, (APP_MN_PLMN_LIST_CNF_T *)sig_ptr);
            break;

        case APP_MN_PREFER_PLMN_CHANGE_CNF:
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode, 
                                     0xff, ATC_NO_DOMAIN, ((APP_MN_PREFER_PLMN_CHANGE_CNF_T *)sig_ptr)->dual_sys);
            ATC_ProcessPreferPlmnChangCnf(S_CONFIG_INFO_PTR, (APP_MN_PREFER_PLMN_CHANGE_CNF_T *)sig_ptr);
            break;

        //The following is about STK
    #ifndef ATC_SAT_ENABLE    
        case APP_MN_SIMAT_DISPLAY_TEXT_IND:
        case APP_MN_SIMAT_GET_INKEY_IND:
        case APP_MN_SIMAT_GET_INPUT_IND:
        case APP_MN_SIMAT_SETUP_CALL_IND:
        case APP_MN_SIMAT_SETUP_MENU_IND:
        case APP_MN_SIMAT_SELECT_ITEM_IND:
        case APP_MN_SIMAT_SEND_SMS_IND:
        case SIMAT_SESSION_TERMINATED_IND:

        case APP_MN_SIMAT_SEND_SS_IND:
        case APP_MN_SIMAT_SEND_USSD_IND:
        case APP_MN_SIMAT_PLAY_TONE_IND:
        case APP_MN_SIMAT_PRO_LOCAL_INFO_IND:

        case SIMAT_SETUP_EVENT_LIST_REQ :
        case APP_MN_SIMAT_SETUP_IDLE_TEXT_IND:
        case APP_MN_SIMAT_SEND_DTMF_IND:
        case APP_MN_SIMAT_LANG_NOTIFY_IND:
        case SIMAT_MO_CC_RSP:
        case SIMAT_EVENT_SS_CON_RSP:
        case SIMAT_EVENT_USSD_CON_RSP:
        case EV_MN_APP_SIMAT_SMS_CTL_TO_MMI_IND_F:
        case SIMAT_EVENT_SMS_PP_RSP:
        case SIMAT_EVENT_SMS_CB_RSP:

        case APP_MN_SIMAT_REFRESH_IND:
        case APP_MN_SIMAT_REFRESH_CNF:
        case APP_MN_SIMAT_SEND_DATA_IND:
        case APP_MN_SIMAT_RECEIVE_DATA_IND:
        case APP_MN_SIMAT_LAUNCH_BROWSER_IND:
        case APP_SIMAT_OPEN_CHANNEL_IND:
        case APP_SIMAT_CLOSE_CHANNEL_IND:
        case APP_SIMAT_GET_CHANNEL_STATUS_IND:
            ATC_TRACE_LOW("ATC:call ATC_ProcessCusdInd");
            ATC_ProcessSTKSignal(S_CONFIG_INFO_PTR, sig_ptr);
            break;

        case ATC_STSF_EXP_IND:
        case ATC_STSF_EXP_IND1:
        case ATC_STSF_EXP_IND2:
        case ATC_STSF_EXP_IND3:
            ATC_TRACE_LOW("ATC:ATC_STSF_EXP_IND");
            ATC_ProcessTimerEXp(S_CONFIG_INFO_PTR, sig_ptr);
            break;

            // The following signals for battery charge service
        case ATC_CHR_CAP_IND:
        case ATC_CHR_WARNING_IND:
            ATC_ProcessChrCapInd(S_CONFIG_INFO_PTR, sig_ptr);
            break;

        case ATC_CHR_CHARGE_START_IND:
            ATC_ProcessChrStartInd(S_CONFIG_INFO_PTR, sig_ptr);
            break;

        case ATC_CHR_CHARGE_END_IND:
        case ATC_CHR_FINISH_IND:
            ATC_ProcessChrEndInd(S_CONFIG_INFO_PTR, sig_ptr);
            break;

            //The following is about phonebook.
        case APP_MN_ADN_UPDATE_CNF:
        case APP_MN_FDN_UPDATE_CNF:

            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     ((APP_MN_ADN_UPDATE_CNF_T *)sig_ptr)->entry_id,
                                     ATC_DOMAIN_PHONEBOOK,
                                     ((APP_MN_ADN_UPDATE_CNF_T *)sig_ptr)->dual_sys);

            ATC_ProcessDelItemCnf(S_CONFIG_INFO_PTR, (APP_MN_ADN_UPDATE_CNF_T *)sig_ptr, recv_event_result);
            break;
    #endif

        case APP_MN_SET_BAND_CNF:
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     0xff, ATC_NO_DOMAIN, ((APP_MN_SET_BAND_CNF_T *)sig_ptr)->dual_sys);
            ATC_ProcessSetBandCnf(S_CONFIG_INFO_PTR, (APP_MN_SET_BAND_CNF_T *)sig_ptr);
            break;

        case APP_MN_PS_POWER_OFF_CNF:
            dual_sys = ((APP_MN_PS_POWER_OFF_CNF_T *)sig_ptr)->dual_sys;
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     0xff, ATC_NO_DOMAIN, dual_sys);

            ATC_ProcessPsPowerOffCnf(&s_config_info, dual_sys);
            break;

        case APP_MN_PS_POWER_ON_CNF:
            dual_sys = ((APP_MN_PS_POWER_ON_CNF_T *)sig_ptr)->dual_sys;
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     0xff, ATC_NO_DOMAIN, dual_sys);
            ATC_ProcessPsPowerOnCnf(&s_config_info, dual_sys);
            break;
            
        case APP_MN_EMG_NUM_LIST_IND:
            ATC_ProcessEmgNumListInd(S_CONFIG_INFO_PTR, (APP_MN_EMG_NUM_LIST_IND_T *)sig_ptr);
            break;
            
        case APP_MN_CRSM_CNF:
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     0xff, ATC_NO_DOMAIN, ((APP_MN_CRSM_CNF_T *)sig_ptr)->dual_sys);
            ATC_ProcessCrsmCnf(S_CONFIG_INFO_PTR, (APP_MN_CRSM_CNF_T *)sig_ptr);
            break;

        case APP_MN_SIM_POWER_ON_CNF:
#ifdef _ATC_ONLY
            dual_sys = ((APP_MN_SIM_POWER_ON_CNF_T *)sig_ptr)->dual_sys;
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     0xff, ATC_NO_DOMAIN, dual_sys);
  #ifdef _MUX_ENABLE_
            if(recv_event_result)
            {
                // if user send at+sfun=2 poweron sim, then send OK as response 
                ATC_BuildResultCodeRsp(S_CONFIG_INFO_PTR, S_ATC_RESULT_CODE_OK, 0);
            }
            else
  #endif                
            {
                if (TRUE == s_config_info.cfun_flag[dual_sys] || (RM_GetCalibrationPostMode() && (dual_sys == MN_DUAL_SYS_1)))
                {
                    MNPHONE_PowerOnPsEx(dual_sys);
                    Atc_SetPsActState(dual_sys, TRUE);
                    s_config_info.cfun_status[dual_sys] = ATC_FUN_ACT_PS;
                }
                
             #ifndef _MUX_ENABLE_
                if (s_config_info.cfun_status[dual_sys] == ATC_FUN_ACT_SIM)
                {
                    // if user send at+sfun=2 poweron sim, then send OK as response 
                    ATC_BuildResultCodeRsp(S_CONFIG_INFO_PTR, S_ATC_RESULT_CODE_OK, 0);
                }
             #endif                
            }
            
            s_config_info.sim_flag[dual_sys] = ((APP_MN_SIM_POWER_ON_CNF_T *)sig_ptr)->is_ok ? TRUE : FALSE;
            #ifndef ATC_SAT_ENABLE
            if(g_is_stk_refresh_reset[dual_sys])
            {
                g_is_stk_refresh_reset[dual_sys] = FALSE;
            }
            #endif
#else
            if(g_is_stk_refresh_reset[((APP_MN_SIM_POWER_ON_CNF_T *)sig_ptr)->dual_sys])
            {
                ATC_TRACE_LOW("ATC:MNPHONE_PowerOnPS after refresh reset");
                MNPHONE_PowerOnPsEx(((APP_MN_SIM_POWER_ON_CNF_T *)sig_ptr)->dual_sys);
                g_is_stk_refresh_reset[((APP_MN_SIM_POWER_ON_CNF_T *)sig_ptr)->dual_sys] = FALSE;
            }
#endif      
            break;

        case APP_MN_SIM_POWER_OFF_CNF:
            dual_sys = ((APP_MN_SIM_POWER_OFF_CNF_T *)sig_ptr)->dual_sys;
            if(FALSE == (S_CONFIG_INFO_PTR)->cfun_flag[dual_sys])
            {
                ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     0xff, ATC_NO_DOMAIN, dual_sys);
                ATC_BuildResultCodeRsp(S_CONFIG_INFO_PTR, S_ATC_RESULT_CODE_OK, 0);

                //Send the unsolicited +SIND: 8
                if(ATC_16BIT1 == ((s_config_info.atc_config_nv.wind_ind_level) | ATC_WIND_BIT7))
                {
                    ATC_TRACE_LOW("ATC: cfun_flag:%d,APP_MN_SIM_POWER_OFF_CNF, +SIND: 8", s_config_info.cfun_flag[dual_sys]);
                }

                ATC_UpSINDInfo(dual_sys,
                               S_CONFIG_INFO_PTR,
                               PNULL,
                               ATC_WIND_BIT7,
                               8,
                               (BOOLEAN)(0 != s_config_info.atc_config_nv.creg));
            }

            s_config_info.sim_ready[dual_sys] = FALSE;
            s_config_info.sim_flag[dual_sys] = FALSE;
            g_received_sim_ready[dual_sys] = FALSE;

            s_config_info.cpin_flag[dual_sys] = FALSE;
            s_config_info.cpin[dual_sys]      = ATC_CPIN_CODE_MAX;
            s_config_info.cpin2[dual_sys]      = ATC_CPIN_CODE_MAX;
            s_config_info.sim_err_reason[dual_sys] = MNSIM_ERROR_NULL;
            s_config_info.sim_type[dual_sys] = MN_CARD_TYPE_NUM;
            s_config_info.is_fdn_init[dual_sys] = TRUE;
            s_config_info.is_fdn_enable[dual_sys] = FALSE;
            s_config_info.is_fdn_ready[dual_sys] = FALSE;
            s_config_info.call_read_flag[dual_sys] = FALSE;
    #ifndef ATC_SAT_ENABLE
            ATC_InitUplmnCfgInfo(dual_sys);
    #endif        
            break;

    #ifndef ATC_SAT_ENABLE
        case APP_MN_SIM_MBBMS_ALGO_CNF:
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode, 
                                     0xff, ATC_NO_DOMAIN, ATC_CURRENT_SIM_ID_MUX);
            if(is_xor_mbau_cmd == TRUE)
            {
               ATC_ProcessSIMmbbmsAlgoCnf_Mbau(S_CONFIG_INFO_PTR,(APP_MN_SIM_MBBMS_ALGO_CNF_T*)sig_ptr); 
            }
            else
            {
               ATC_ProcessSIMmbbmsAlgoCnf(S_CONFIG_INFO_PTR,(APP_MN_SIM_MBBMS_ALGO_CNF_T*)sig_ptr);
            }
            break;
    #endif

        case ATC_CALL_DTMF_EXP_IND:
            ATC_ProcessDTMFTimerExpInd(S_CONFIG_INFO_PTR, (ATC_DTMF_TIMER_EXP_IND_T *)sig_ptr);
            break;

        case APP_MN_START_DTMF_CNF:
#ifdef _MUX_ENABLE_
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     ((APP_MN_START_DTMF_CNF_T *)sig_ptr)->call_id, ATC_DOMAIN_CS,
                                     ((APP_MN_START_DTMF_CNF_T *)sig_ptr)->dual_sys);
            if(recv_event_result)
#endif
            {
                ATC_ProcessStartDTMFCnf(S_CONFIG_INFO_PTR, (APP_MN_START_DTMF_CNF_T *)sig_ptr);
            }

            break;

        case APP_MN_STOP_DTMF_CNF:
#ifdef _MUX_ENABLE_
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode,
                                     ((APP_MN_STOP_DTMF_CNF_T *)sig_ptr)->call_id, ATC_DOMAIN_CS,
                                     ((APP_MN_STOP_DTMF_CNF_T *)sig_ptr)->dual_sys);
            if(recv_event_result)
#endif
            {
                ATC_ProcessStopDTMFCnf(S_CONFIG_INFO_PTR, (APP_MN_STOP_DTMF_CNF_T *)sig_ptr);
            }

            break;

        case APP_MN_SET_SMS_RECEIVE_FUNC_CNF:
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode, 
                                     0xff, ATC_NO_DOMAIN, ((APP_MN_SET_SMS_RECEIVE_FUNC_CNF_T *)sig_ptr)->dual_sys);
            ATC_ProcessSPSmsFullInd(S_CONFIG_INFO_PTR, (APP_MN_SET_SMS_RECEIVE_FUNC_CNF_T *)sig_ptr);
            break;
#ifdef _SUPPORT_CCBS_
        case APP_MN_CCBS_ACTIVATE_IND:
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode, 
                                     0xff, ATC_NO_DOMAIN, ((APP_MN_CCBS_ACTIVATE_IND_T *)sig_ptr)->dual_sys);
            ATC_ProcessCCBSActInd(S_CONFIG_INFO_PTR, (APP_MN_CCBS_ACTIVATE_IND_T *)sig_ptr);
            break;

        case APP_MN_CCBS_RECALL_IND:
            ATC_ProcessCCBSRecallInd(S_CONFIG_INFO_PTR, (APP_MN_CCBS_RECALL_IND_T *)sig_ptr);
            break;

        case APP_MN_CCBS_ERASE_CNF:
            ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result, sig_ptr->SignalCode, 
                                     0xff, ATC_NO_DOMAIN, ((APP_MN_CCBS_ERASE_CNF_T *)sig_ptr)->dual_sys);
            ATC_ProcessCCBSEraseCnf(S_CONFIG_INFO_PTR, (APP_MN_CCBS_ERASE_CNF_T *)sig_ptr);
            break;
#endif /*_SUPPORT_CCBS_*/
        case ATC_WRITE_RSP_REQ:
        #ifdef _MUX_ENABLE_
            SCI_TRACE_LOW("ATC: response link id = %d, string = %s",
            ((AT_WRITE_RSP_T*)sig_ptr)->link_id ,
            ((AT_WRITE_RSP_T*)sig_ptr)->str);
            if(((AT_WRITE_RSP_T*)sig_ptr)->link_id == ATC_DEBUG_LINK)
            {
                SIO_ATC_WriteCmdRes((uint8 *)((AT_WRITE_RSP_T*)sig_ptr)->str,((AT_WRITE_RSP_T*)sig_ptr)->len);
            }
            else
            {
                MUX_Write((uint8 *)((AT_WRITE_RSP_T*)sig_ptr)->str,((AT_WRITE_RSP_T*)sig_ptr)->len,((AT_WRITE_RSP_T*)sig_ptr)->link_id);
            }
        #else
            SIO_ATC_WriteCmdRes((uint8 *)((AT_WRITE_RSP_T*)sig_ptr)->str,((AT_WRITE_RSP_T*)sig_ptr)->len);
        #endif
            SCI_FREE(((AT_WRITE_RSP_T*)sig_ptr)->str);
            break;

            //handle asynchronous socket event
            case SOCKET_ASYNC_GETHOSTBYNAME_CNF:
            case SOCKET_CONNECT_EVENT_IND:
            case SOCKET_READ_EVENT_IND:
            case SOCKET_WRITE_EVENT_IND:
            case SOCKET_CONNECTION_CLOSE_EVENT_IND:
            {
#ifndef _GSM_ONLY_
                dual_sys = ATC_CURRENT_SIM_ID_MUX;
                SCI_TRACE_LOW("ATC: dual_sys=%d", dual_sys);
                ATC_RECEIVE_EXPECTED_EVENT_MUX(recv_event_result,sig_ptr->SignalCode,
                                                0xff,ATC_DOMAIN_PS, dual_sys);
                ATC_ProcessSocketEvent(S_CONFIG_INFO_PTR,sig_ptr);
#endif
                break;
            }

#ifdef _SUPPORT_GPRS_
#ifndef ATC_SAT_ENABLE
            case APP_MN_PUSH_GPRS_DATA_CNF:
            {
                ATC_SendDummyDataReqEx(((APP_MN_GPRS_RESUME_IND_SIG*)sig_ptr)->nsapi);
                break;
            }
#endif
#endif            
        default:
            ATC_TRACE_LOW("ATC: ATC_Task: Unknown Signal Code (%d)!", sig_ptr->SignalCode);
            break;
    }
}

#ifdef ATC_SAT_ENABLE
/******************************************************************************/
// Description : This function is used to start PS when power on
// Global resource dependence : None
// Author : longting.zhao
// Note :
/******************************************************************************/
void ATC_InitAutoStartPS()
{
    POWER_RESTART_CONDITION_E      start_condition;
    uint8 i = 0;

    start_condition = POWER_GetRestartCondition();

    if ((RESTART_BY_ALARM != start_condition 
        && RESTART_BY_CHARGE != start_condition)
        || 1 == CLASSMARK_GetModelType() 
       )
    {
#ifdef _ATC_ONLY 
    #ifdef _MUX_ENABLE_
        for (i = MN_DUAL_SYS_1; i < MN_DUAL_SYS_2; i++)
        {
            MNPHONE_PowerOnSimEx((MN_DUAL_SYS_E)i, TRUE);
        }
        //MUX_IsNeedToActivePS return TRUE if MS runs in cal_pos mode.
        if(RM_GetCalibrationPostMode())
        {
            MNPHONE_PowerOnPsEx(MN_DUAL_SYS_1);
            is_init_ps_power_on[MN_DUAL_SYS_1] = TRUE;
        }
        else
        {
            MNPHONE_SelectPlmnWithoutSim(MN_DUAL_SYS_1,460,0,2);//设定卡1在无卡情况下，上460 00（中国移动）。
            MNPHONE_SelectPlmnWithoutSim(MN_DUAL_SYS_1,460,2,2);//设定卡1在无卡情况下，上460 02（中国移动）。
            MNPHONE_SelectPlmnWithoutSim(MN_DUAL_SYS_2,460,1,2);//设定卡2在无卡情况下，上460 01（中国联通）。
            for (i = MN_DUAL_SYS_1; i < MN_DUAL_SYS_2; i++)
            {
                MNPHONE_PowerOnPsEx((MN_DUAL_SYS_E)i);
                is_init_ps_power_on[i] = TRUE;
            }
        }
    #else
            MNPHONE_StartupPs(MN_GMMREG_RAT_GPRS);

             is_init_ps_power_on[MN_DUAL_SYS_1] = TRUE;
   #endif
#endif   
    }
}
#endif

/******************************************************************************/
// Description : This function do some initialization work for ATC
// Global resource dependence : g_atc_info_table
//                              g_atc_handle_table
//                              g_atc_result_code_text
// Author : Steven
// Note : This function should be called first on starting ATC task
/******************************************************************************/
LOCAL ATC_STATUS ATC_Initialize(void  // Retur S_ATC_SUCCESS if success,
                         ) // otherwise return error code.

{
    ATC_STATUS    status = S_ATC_SUCCESS;
    int32    i, j;

    status = ATCM_CreateHeap();

    if(S_ATC_SUCCESS != status)
    {
        ATC_TRACE_LOW("ATC: ATC_Initialize failed in ATCM_CreateHeap.");
        return status;
    }

    // reset the static variable
    SCI_MEMSET(&s_atc_global_info, 0, sizeof(ATC_GLOBAL_INFO_T));

#ifdef _MUX_ENABLE_
    s_atc_global_info.atc_link_info.cur_cmd_line_buf_ptr = PNULL;
    s_atc_global_info.atc_link_info.last_cmd_line_buf_ptr = PNULL;
#else
    s_atc_global_info.cur_cmd_line_buf_ptr  = PNULL;
    s_atc_global_info.last_cmd_line_buf_ptr = PNULL;
#endif

    s_atc_global_info.buffered_at_cmd_count = 0;
    s_atc_global_info.atc_info_table        = g_atc_info_table;
    s_atc_global_info.atc_handle_table      = g_atc_handle_table;
    s_atc_global_info.atc_config_ptr        = &s_config_info;

    for(i = MN_DUAL_SYS_1; i < MN_SYS_NUMBER; i++)
    {
        SCI_MEMSET(&g_calls_state[i], 0, sizeof(g_calls_state[i]));

        for(j = 0; j < MN_CALL_MAX_CALL_NUM; j++)
        {
            g_calls_state[i].call_context[j].call_id = ATC_INVALID_CALLID;
        }

        if(PNULL != g_emg_num_list_ptr[i])
        {
            SCI_FREE(g_emg_num_list_ptr[i]);
        }

        s_sms_cnma[i].cnma_timer = SCI_CreateTimer("ATC CNMA Timer",
                                            AtcCNMATimerExpired, i, ATC_CNMA_TIMER_VALUE, SCI_NO_ACTIVATE);

    }

    ATC_InitConfig(s_atc_global_info.atc_config_ptr);

#ifdef _MUX_ENABLE_
    for(i = MN_DUAL_SYS_1; i < MN_SYS_NUMBER; i++)
    {
        //default atc buffer
        atc_buffer[i].atc_buf_ptr = (uint8 *)SCI_ALLOC_BASE(ATC_REC_BUF_SIZE);
        atc_buffer[i].size        = ATC_REC_BUF_SIZE;
        atc_buffer[i].start_point  = 0;
        atc_buffer[i].end_point   = 0;
    }

    for(i = 0; i < ATC_MAX_LINK_ID; i++)
    {
        atc_bs_ch[i] = ATC_BACKSPACE_CHAR;
    }

#endif

    //Register event
    ATC_RegisterEvent();

#ifndef _MUX_ENABLE_

    if(s_atc_global_info.atc_config_ptr->atc_config_nv.mux_mode)
    {
        SIO_SetUserPortMode(MIX_MODE);
    }
    else
    {
        SIO_SetUserPortMode(ATC_MODE);
    }

#endif

#ifdef ATC_SAT_ENABLE
    ATC_InitAutoStartPS();
#else
    //auto start ps when run in CALIBRATION_POST_MODE
    if (  (CALIBRATION_POST_MODE == POWER_GetResetMode()) || (CALIBRATION_MODE == POWER_GetResetMode()) || (CALIBRATION_POST_NO_LCM_MODE == POWER_GetResetMode()))
    {//modify by longsung  
        ATC_InitAutoStartPSNoSat();
    }
#endif

    //Send the unsolicited +SIND: 3 and 4
    ATC_UpSINDInfo(MN_DUAL_SYS_1,
                   s_atc_global_info.atc_config_ptr,
                   PNULL,
                   ATC_WIND_BIT2,
                   3,
                   (BOOLEAN)(0 != s_atc_global_info.atc_config_ptr->atc_config_nv.creg));
    ATC_UpSINDInfo(MN_DUAL_SYS_1,
                   s_atc_global_info.atc_config_ptr,
                   PNULL,
                   ATC_WIND_BIT3,
                   4,
                   (BOOLEAN)(0 != s_atc_global_info.atc_config_ptr->atc_config_nv.creg));

#ifndef ATC_SAT_ENABLE
    s_atc_sleep_timer = SCI_CreateTimer("ATC SLEEP Timer",
                                        AtcSleepTimerExpired, 0, ATC_SLEEP_TIMER_VALUE, SCI_NO_ACTIVATE);
#endif

    for(i = 0; i < ATC_MAX_LINK_ID; i++)
    {
        atc_link_inst[i].link_id = i;

        /* PSD call fields */
        atc_link_inst[i].cid            = ATC_INVALID_CID;
        atc_link_inst[i].ath_cid        = ATC_INVALID_CID;
        atc_link_inst[i].ath_stat       = ATC_ATH_NULL;
        atc_link_inst[i].ath_wait_timer = SCI_CreateTimer("ATC PS ATH Wait Timer",
                                                          ATC_Link_ATHWaitTimerTimeout, i, ATC_PS_ATH_TIMER_VALUE, SCI_NO_ACTIVATE);
        atc_link_inst[i].ath_avoid_rep_timer = SCI_CreateTimer("ATC PS ATH Avoid Repetition Timer",
                                                               ATC_Link_ATHAvoidRepTimeout, i, ATC_PS_ATH_AVOID_REP_TIMER_VALUE, SCI_NO_ACTIVATE);

    }
#ifdef _MUX_ENABLE_
    for(i=0; i<MN_GPRS_MAX_PDP_CONTEXT_COUNT+1; i++)
    {
    #ifdef _ATC_WAIT_L4
        g_packet_type[i] = TCPIP_PKTTYPE_PPP;
    #endif
    }
#endif

    return status;
}

#ifdef _MUX_ENABLE_
/******************************************************************************/
//  Description : This function reads a new AT command line from cmux unit and
//                      parses it
//  Global resource dependence :
//  Author : Wang Hui
//  Note :
/******************************************************************************/
LOCAL ATC_STATUS ATC_RecNewLineSig(  // Return S_ATC_SUCCESS if success,
    // otherwise return error code.
    ATC_GLOBAL_INFO_T      *atc_global_info_ptr, // point to the ATC_GLOBAL_INFO_T struct.
    const xSignalHeaderRec *sig_ptr              // point to signal struct
)
{
    ATC_STATUS      status             = S_ATC_SUCCESS;
    uint                    atc_line_length    = 0;
    uint                    atc_read_length    = 0;
    uint8                  atc_link_id = 0;
    uint8                   para_str[ATC_SHARP_LEN];
    uint                    para_len;
    ATC_CUR_INFO_LIST_T     *atc_cur_info_ptr  = PNULL;
    ATC_CUR_INFO_LIST_T     *atc_last_info_ptr = PNULL;
    MN_DUAL_SYS_E dual_sys = MN_DUAL_SYS_1;

    SCI_ASSERT(PNULL != atc_global_info_ptr);/*assert verified: check null pointer*/
    SCI_ASSERT(ATC_MUX_RECV_NEW_AT == sig_ptr->SignalCode);/*assert verified: check valid value*/

    /* Get new AT command line length */
    atc_line_length = (uint)(((ATC_MUX_RECV_NEW_AT_T *)sig_ptr)->len);
    atc_link_id = ((ATC_MUX_RECV_NEW_AT_T *)sig_ptr)->link_id;
    atc_global_info_ptr->atc_config_ptr->current_link_id = atc_link_id;
    dual_sys = ATC_GetSimIdFromLinkId(((ATC_MUX_RECV_NEW_AT_T *)sig_ptr)->link_id);

    ATC_TRACE_LOW("ATC: N_AT_L, link_id:%d, len:%d", atc_link_id, atc_line_length);

    if(0 == atc_line_length)
    {
        ATC_BuildResultCodeRsp(atc_global_info_ptr->atc_config_ptr, S_ATC_RESULT_CODE_ERROR, ERR_LEX_LEN_ZERO);
        return S_ATC_FAIL;
    }

    /* Too long atc command, just clear the atc buffer*/
    if(atc_line_length > ATC_MAX_CMD_LINE_LENGTH - 2)
    {
        ATC_ClrCmdLine(dual_sys);
        status = ERR_TEXT_STRING_TOO_LONG;

        /*restore AT command parase mode*/
        if(atc_link_id == ATC_DEBUG_LINK)
        {
            SIO_ATC_SetCmdLineTerminateChar(atc_global_info_ptr->atc_config_ptr->s3_reg, '\0');
        }
        else
        {
            ATC_SetCmdLineTerminateChar(atc_link_id, atc_global_info_ptr->atc_config_ptr->s3_reg, '\0');
        }

        ATC_BuildResultCodeRsp(atc_global_info_ptr->atc_config_ptr, S_ATC_RESULT_CODE_ERROR, status);

        return status;
    }

    /* If the new cmd line two short, it could be an "A/" command. So we can use the short buf to store it.*/
    else if(atc_line_length <= ATC_MIN_CMD_LINE_LENGTH)
    {
        if(atc_global_info_ptr->atc_link_info.last_cmd_line_buf_ptr ==
                atc_global_info_ptr->atc_link_info.cmd_line_buf_short)
        {
            SCI_MEMCPY(atc_global_info_ptr->atc_link_info.cmd_line_buf_long,
                       atc_global_info_ptr->atc_link_info.cmd_line_buf_short,
                       ATC_MIN_CMD_LINE_LENGTH);

            atc_global_info_ptr->atc_link_info.last_cmd_line_buf_ptr =
                atc_global_info_ptr->atc_link_info.cmd_line_buf_long;
        }

        atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr =
            atc_global_info_ptr->atc_link_info.cmd_line_buf_short;
    }
    else
    {
        atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr =
            atc_global_info_ptr->atc_link_info.cmd_line_buf_long;
    }


    /*Read AT command line from ATC buffer*/
    ATC_ReadCmdLine(atc_link_id, atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr,
                    atc_line_length, &atc_read_length);

    if(atc_read_length != atc_line_length)
    {
        ATC_TRACE_LOW("ATC: backspace? len: %d->%d", atc_line_length, atc_read_length);
        atc_line_length = atc_read_length;
    }

    /* Add two PNULL terminater for atcparse */
    atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr[atc_line_length] = '\0';
    atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr[atc_line_length + 1] = '\0';


    /* trace out the at command line */
    if(atc_line_length < 200)//please make sure all the context length is less than 250 in the quotation mark including prefixed characters
    {
        uint8 tchar[42] = {0};
        ConvertBinToHex(atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr, (atc_line_length > 20) ? 20 : atc_line_length, tchar);
        ATC_TRACE_LOW("ATC: N_AT_L, len:%d, hex:%s, line:%s", atc_line_length, tchar, atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr);
    }
    else
    {
        uint8   line_str[201] = {0};
        uint8   temp_str_len = 0;
        uint8  *temp_str = PNULL;
        uint8 i;
        
        temp_str = atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr;
        temp_str_len = atc_line_length/200 + ((atc_line_length % 200) ? 1 : 0);
        ATC_TRACE_LOW("ATC: N_AT_L, len:%d, hex: too long, separated into %d segments ", atc_line_length, temp_str_len);
        for (i = 0; i < temp_str_len; i++)
        {
            strncpy((char *)line_str, (char *)(temp_str + i * 200), 200);
            line_str[200] = '\0';
            ATC_TRACE_LOW("ATC: Rec Seg [%d]:%s", i, line_str);
        }
    }

    if(atc_global_info_ptr->atc_config_ptr->s3_reg  == *atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr ||
        atc_global_info_ptr->atc_config_ptr->s4_reg == *atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr)
    {
        atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr ++;
        atc_line_length --;
    }

    /*remove ; from atc line for atcparase*/
    if((atc_line_length > 2) &&
        (atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr[atc_line_length - 1] == atc_global_info_ptr->atc_config_ptr->s3_reg) &&
        (atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr[atc_line_length - 2] == ';'))
    {
        atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr[atc_line_length - 2] = atc_global_info_ptr->atc_config_ptr->s3_reg;
        atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr[atc_line_length - 1] = '\0'; //add null terminater for atcparse
        atc_line_length --;
    }

    if(atc_global_info_ptr->atc_config_ptr->atc_link_config[atc_link_id].is_ctrlz_end)
    {
        atc_global_info_ptr->atc_config_ptr->atc_link_config[atc_link_id].is_ctrlz_end = FALSE;

        switch(atc_global_info_ptr->atc_config_ptr->atc_link_config[atc_link_id].ctrlz_end_status)
        {
            case ATC_CTRLZ_SMS:
                status = ATC_ProcessSmsPdu(atc_global_info_ptr->atc_config_ptr,
                                           atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr);
                break;

            case ATC_CTRLZ_SMS_STATUS:
                status = ATC_ProcessSmsStatusPdu(atc_global_info_ptr->atc_config_ptr,
                                                 atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr);
                break;
                
#ifndef ATC_SAT_ENABLE
            case ATC_CTRLZ_STK:
    #ifndef _ULTRA_LOW_CODE_
                status = ATC_ProcessSioStkData(atc_global_info_ptr->atc_config_ptr,
                                               atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr);
    #endif
                break;

            case ATC_CTRLZ_SRWD:
    #ifndef _ULTRA_LOW_CODE_
                status = ATC_ProcessSioSRWDData(atc_global_info_ptr->atc_config_ptr,
                                                atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr);
    #endif
                break;
#endif

            case ATC_CTRLZ_SSEA:
                status = ATC_ProcessSioSSEAData(atc_global_info_ptr->atc_config_ptr,
                                                atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr, atc_line_length);
                break;

            case ATC_CTRLZ_SADM:
                status = ATC_ProcessSioSADMData(atc_global_info_ptr->atc_config_ptr,
                                                atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr, atc_line_length);
                break;

            case ATC_CTRLZ_SADMNV:
                status = ATC_ProcessSioSADMNVData(atc_global_info_ptr->atc_config_ptr,
                                                  atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr, atc_line_length);
                break;

            case ATC_CTRLZ_GPRS:
        #ifdef _SUPPORT_GPRS_
                status = ATC_ProcessSioData(atc_global_info_ptr->atc_config_ptr, 
                                            atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr, atc_line_length);
        #endif
                break;

            case ATC_CTRLZ_SNVM:
        #ifdef _ATC_ONLY
                status = ATC_ProcessSioSNVMData(atc_global_info_ptr->atc_config_ptr,
                                                atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr);
        #endif
                break;

               case ATC_CTRLZ_SADM_ARM:
        #ifdef AUDIO_PARAM_ARM
                status = ATC_ProcessSioARMSADMData(atc_global_info_ptr->atc_config_ptr,
                                                atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr,atc_line_length);
        #endif
                break;

            case ATC_CTRLZ_SADMNV_ARM:
        #ifdef AUDIO_PARAM_ARM
                status = ATC_ProcessSioARMSADMNVData(atc_global_info_ptr->atc_config_ptr,
                                                atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr,atc_line_length);
        #endif
            break;

            case ATC_CTRLZ_SADMDSP:
            {
#ifdef AUDIO_PARAM_ARM
                status = ATC_ProcessSioDSPSADMData(atc_global_info_ptr->atc_config_ptr,
                                                atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr, atc_line_length);
#endif
                break;
            }

            case ATC_CTRLZ_SADMNV_DSP:
            {
#ifdef AUDIO_PARAM_ARM
                status = ATC_ProcessSioDSPSADMNVData(atc_global_info_ptr->atc_config_ptr,
                                                atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr, atc_line_length);
#endif
                break;
            }

            case ATC_CTRLZ_LVVE:
            {
                status = ATC_ProcessSioSADLVVEData(atc_global_info_ptr->atc_config_ptr,
                                                atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr,atc_line_length);
                break;
            }
            
            case ATC_CTRLZ_LVVENV:
            {
                status = ATC_ProcessSioSADLVVENVData(atc_global_info_ptr->atc_config_ptr,
                                               atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr,atc_line_length);
                break;
            }
        
        #ifndef ATC_SAT_ENABLE
            case ATC_CTRLZ_ENHA:
                status = ATC_ProcessSioENHA_Data(atc_global_info_ptr->atc_config_ptr,
                                                atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr,atc_line_length);
                break;
            case ATC_CTRLZ_ENHA_TUN:
                status = ATC_ProcessSioENHA_Tun_Data(atc_global_info_ptr->atc_config_ptr,
                                                atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr,atc_line_length);
                break;

            case ATC_CTRLZ_ENHANV:
                status = ATC_ProcessSioENHA_NVData(atc_global_info_ptr->atc_config_ptr,
                                                atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr,atc_line_length);
                break;

            case ATC_CTRLZ_ENHANV_TUN:
                status = ATC_ProcessSioENHA_Tun_NVData(atc_global_info_ptr->atc_config_ptr,
                                                atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr,atc_line_length);
                break;
        #endif /*ATC_SAT_ENABLE*/

            default:
               ATC_TRACE_LOW("ATC: N_AT_L, ctrlz_end_status is not allowed");
               ATC_BuildResultCodeRsp(atc_global_info_ptr->atc_config_ptr, S_ATC_RESULT_CODE_ERROR, ERR_LEX_INVALID_CTRL_Z_COMMAND);
               return S_ATC_DEFAULT_HANDLE;
        }

        if(atc_link_id == ATC_DEBUG_LINK)
        {
            SIO_ATC_SetCmdLineTerminateChar(atc_global_info_ptr->atc_config_ptr->s3_reg, '\0');
        }
        else
        {
            ATC_SetCmdLineTerminateChar(atc_link_id, atc_global_info_ptr->atc_config_ptr->s3_reg, '\0');
        }

    }
    else
    {
        /* Parse the AT command line */
        zerolex(); //add for reset lex state

        if(atc_err_protected_flag && atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr)
        {
            const uint8 *buf_ptr;
            uint         buf_len;

            buf_ptr = atcparse_at(atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr, atc_line_length);
            buf_len = atc_line_length - ((uint32)buf_ptr - (uint32)atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr);

            if (buf_len < atc_line_length)
                ATC_TRACE_LOW("ATC: N_AT_L, ignore ip data, len:%d->%d, newatcmd:%s", atc_line_length, buf_len, (const char *)buf_ptr);
            atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr = (uint8 *)buf_ptr;
            atc_line_length = buf_len;
        }

        atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr[atc_line_length-1] = 0xD;
        status = atcparse(atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr, &atc_cur_info_ptr);

        if(((0x41 == *atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr) ||         //'A'
                (0x61 == *atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr))   &&      //'a'
                ((0x54 == *(atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr + 1))  || //'T'
                 (0x74 == *(atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr + 1))))    //'t'
        {
            if((0x23 == *(atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr + 2)) &&     //'#'
                    (0x23 == *(atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr + 3)))
            {
                SCI_MEMSET(para_str, 0, ATC_SHARP_LEN);
                para_len = atc_line_length - 4;

                if(para_len > ATC_SHARP_LEN)
                {
                    ATCM_FreeAll();
                    ATC_TRACE_LOW("ATC: N_AT_L, Assert Failure sharp len too long.");
                    return ERR_OPERATION_NOT_SUPPORTED;
                }

                SCI_MEMCPY(para_str, (atc_global_info_ptr->atc_link_info.cur_cmd_line_buf_ptr + 4), para_len);
                para_len = para_len - 1;
                /*call the extern function*/
                ATC_TRACE_LOW("ATC: N_AT_L, Call the external function");
            #ifndef _ATC_ONLY
                ATC_ExtCmd(para_str, para_len);
            #endif
                ATCM_FreeAll();

                /*need do nothing while received AT## command*/
                return S_ATC_DEFAULT_HANDLE;
            }
        }
        else
        {
            /*if cmd ptr do not start with "AT" string, it will be discarded*/
            ATC_BuildResultCodeRsp(atc_global_info_ptr->atc_config_ptr, S_ATC_RESULT_CODE_ERROR, ERR_LEX_NO_AT_PREX);
            
            ATCM_FreeAll();
            return S_ATC_DEFAULT_HANDLE;
        }
    }

    switch(status)
    {
        case S_ATC_SUCCESS:
            SCI_ASSERT(PNULL != atc_cur_info_ptr);/*assert verified: check null pointer*/
            ATC_TRACE_LOW("ATC: N_AT_L, Parse success.");

            /*we link the infos of AT cmds to the end of the list which store in atc_config */
            if(PNULL == atc_global_info_ptr->atc_link_info.atc_info_ptr)
            {
                atc_global_info_ptr->atc_link_info.atc_info_ptr = atc_cur_info_ptr;
            }
            else
            {
                atc_last_info_ptr = atc_global_info_ptr->atc_link_info.atc_info_ptr;

                while(PNULL != atc_last_info_ptr->next)
                {
                    atc_last_info_ptr = atc_last_info_ptr->next;
                }

                atc_last_info_ptr->next = atc_cur_info_ptr;
            }

            break;

        case S_ATC_DEFAULT_HANDLE:
            ATC_TRACE_LOW("ATC: N_AT_L, Parse SMS PDU successful");
            break;

        default:
            ATC_TRACE_LOW("ATC: N_AT_L, Parse failed.");
            /*response the result code*/
            ATC_BuildResultCodeRsp(atc_global_info_ptr->atc_config_ptr, S_ATC_RESULT_CODE_ERROR, status);
            break;
    }

    return status;
}

/******************************************************************************/
// Description : This function processes the AT commands stored in the AT
//               config struct one by one.
// Global resource dependence : None
// Author : Steven
// Note : None
/******************************************************************************/
LOCAL ATC_STATUS ATC_ProcessCmds( // Return S_ATC_SUCCESS if success,
    // otherwise return error code.
    ATC_GLOBAL_INFO_T      *atc_global_info_ptr // point to the ATC_GLOBAL_INFO_T struct.
)
{
    ATC_STATUS      status = S_ATC_SUCCESS;
    uint8    atc_link_id = 0;
    uint8    i;

    SCI_ASSERT(PNULL != atc_global_info_ptr);/*assert verified: check null pointer*/
    atc_link_id = atc_global_info_ptr->atc_config_ptr->current_link_id;

    while(PNULL != atc_global_info_ptr->atc_link_info.atc_info_ptr)
    {
        status = ATC_DespatchCmd(atc_global_info_ptr, atc_global_info_ptr->atc_link_info.atc_info_ptr);

        if(S_ATC_SUCCESS != status && S_ATC_DEFAULT_HANDLE != status)
        {
            atc_global_info_ptr->atc_link_info.atc_info_ptr = PNULL;
            break;
        }

        atc_global_info_ptr->atc_link_info.atc_info_ptr = atc_global_info_ptr->atc_link_info.atc_info_ptr->next;
    }

    switch(status)
    {
        case S_ATC_SUCCESS:
            SCI_ASSERT(PNULL == atc_global_info_ptr->atc_link_info.atc_info_ptr);/*assert verified: check valid value*/

            if(is_cmux_recv)
            {
                MUX_Startup(MUX_SIDE_UE, cmux_sys_para);
                is_cmux_recv = FALSE;
                if (FALSE == g_is_mux_single_sys_mod)
                {
                    for (i = MN_DUAL_SYS_1; i < MN_SYS_NUMBER * 3; i++)
                    {
                        g_mux_dlc_info[i + 1].dlci       = i + 1;
                        g_mux_dlc_info[i + 1].from       = MUX_1;
                        g_mux_dlc_info[i + 1].used_state = MUX_DLC_STATE_CONNECT;
                    }
                    
                    for (i = 1; i <= ATC_PDP_NUM_MAX; i++)
                    {    
                        g_mux_dlc_info[i + MN_SYS_NUMBER * 3].dlci       = i + MN_SYS_NUMBER * 3;
                        g_mux_dlc_info[i + MN_SYS_NUMBER * 3].from       = MUX_1;
                        g_mux_dlc_info[i + MN_SYS_NUMBER * 3].used_state = MUX_DLC_STATE_CONNECT;
                    }

                    //initialization for voice record channel
                    g_mux_dlc_info[13].dlci = 13;
                    g_mux_dlc_info[13].from = MUX_1;
                    g_mux_dlc_info[13].used_state = MUX_DLC_STATE_CONNECT;
                }
                else
                {
                    SCI_TRACE_LOW("ATC: sigle sim MUX mode");
                    for (i = 1; i < ATC_DEBUG_LINK - 1; i++)
                    {
                        g_mux_dlc_info[i].dlci       = i;
                        g_mux_dlc_info[i].from       = MUX_1;
                        g_mux_dlc_info[i].used_state = MUX_DLC_STATE_CONNECT;
                    }
                }
            }

            ATC_BuildResultCodeRsp(atc_global_info_ptr->atc_config_ptr,
                                   S_ATC_RESULT_CODE_OK , 0);
            break;

        case S_ATC_LAST_ERROR:
            ATC_BuildResultCodeRsp(atc_global_info_ptr->atc_config_ptr,
                                   S_ATC_RESULT_CODE_OK , 0);

            ATC_BuildResultCodeRsp(atc_global_info_ptr->atc_config_ptr,
                                   S_ATC_RESULT_CODE_ERROR , status);
            status = S_ATC_SUCCESS;
            break;

        case S_ATC_DEFAULT_HANDLE:
            break;

        default:
            ATC_BuildResultCodeRsp(atc_global_info_ptr->atc_config_ptr,
                                   S_ATC_RESULT_CODE_ERROR , status);
            break;
    }

    // If the AT commands are all handled, we free our private heap mem
    ATCM_FreeAll();
    return status;
}

#else/*_MUX_ENABLE_*/
/******************************************************************************/
// Description : This function reads a new AT command line from sio unit and
//               parses it
// Global resource dependence :
// Author : Steven
// Note :   Modify: Xueli
/******************************************************************************/

LOCAL ATC_STATUS ATC_RecNewLineSig(  // Return S_ATC_SUCCESS if success,
    // otherwise return error code.
    ATC_GLOBAL_INFO_T      *atc_global_info_ptr, // point to the ATC_GLOBAL_INFO_T struct.
    const xSignalHeaderRec *sig_ptr              // point to signal struct
)
{
    ATC_STATUS              status             = S_ATC_SUCCESS;
    uint                    atc_line_length    = 0;
    uint                    atc_read_length    = 0;
    uint8                   para_str[ATC_SHARP_LEN];
    uint                    para_len;
    ATC_CUR_INFO_LIST_T     *atc_cur_info_ptr  = PNULL;
    ATC_CUR_INFO_LIST_T     *atc_last_info_ptr = PNULL;


    SCI_ASSERT(PNULL != atc_global_info_ptr);/*assert verified: check null pointer*/
    SCI_ASSERT(PNULL != sig_ptr);/*assert verified: check null pointer*/
    SCI_ASSERT(SIO_RECV_NEW_AT == sig_ptr->SignalCode);/*assert verified: check valid value*/

    /* Get new AT command line length */
    atc_line_length = (uint)(((SIO_RECV_NEW_AT_T *)sig_ptr)->len);

    if(0 == atc_line_length)
    {
        return S_ATC_FAIL;
    }

    /* Too long atc command,just clear the sio buffer and
     * note the sio buffer size is different from atc buffer size
     */
    if(atc_line_length > ATC_MAX_CMD_LINE_LENGTH - 2)
    {
        SIO_ATC_ClrCmdLine();
        status = ERR_TEXT_STRING_TOO_LONG;

        //restore AT command parase mode
        SIO_ATC_SetCmdLineTerminateChar(atc_global_info_ptr->atc_config_ptr->s3_reg, '\0');

        ATC_BuildResultCodeRsp(atc_global_info_ptr->atc_config_ptr,
                               S_ATC_RESULT_CODE_ERROR, status);

        return status;
    }
    /*
    ** If the new cmd line two short, it could be an "A/" command.
    ** So we can use the short buf to store it.
    */
    else if(atc_line_length <= ATC_MIN_CMD_LINE_LENGTH)
    {
        if(atc_global_info_ptr->last_cmd_line_buf_ptr ==
                atc_global_info_ptr->cmd_line_buf_short)
        {
            SCI_MEMCPY(atc_global_info_ptr->cmd_line_buf_long, atc_global_info_ptr->cmd_line_buf_short,
                       ATC_MIN_CMD_LINE_LENGTH);

            atc_global_info_ptr->last_cmd_line_buf_ptr =
                atc_global_info_ptr->cmd_line_buf_long;
        }

        atc_global_info_ptr->cur_cmd_line_buf_ptr =
            atc_global_info_ptr->cmd_line_buf_short;
    }
    //if(atc_line_length <= ATC_MAX_CMD_LINE_LENGTH - 2)
    else
    {
        atc_global_info_ptr->cur_cmd_line_buf_ptr =
            atc_global_info_ptr->cmd_line_buf_long;
    }

#ifndef ATC_SAT_ENABLE
    if (g_is_diag_cmd)
    {
        ATC_ReadDiagCmdLine(atc_global_info_ptr->cur_cmd_line_buf_ptr,
                            atc_line_length, &atc_read_length);
    }
    else
#endif
    {
        /*
         ** Read AT command line from SIO
         */
        SIO_ATC_ReadCmdLine(atc_global_info_ptr->cur_cmd_line_buf_ptr,
                            atc_line_length, &atc_read_length);
    }

    if(atc_read_length != atc_line_length)
    {
        atc_line_length = atc_read_length;
    }

    /* Add two PNULL terminater for atcparse */
    atc_global_info_ptr->cur_cmd_line_buf_ptr[atc_line_length] = '\0';
    atc_global_info_ptr->cur_cmd_line_buf_ptr[atc_line_length + 1] = '\0';


    /* trace out the at command line */
    if(atc_line_length < 200)//please make sure all the context length is less than 250 in the quotation mark including prefixed characters
    {
        uint8 tchar[42] = {0};
        ConvertBinToHex(atc_global_info_ptr->cur_cmd_line_buf_ptr, (atc_line_length > 20) ? 20 : atc_line_length, tchar);
        ATC_TRACE_LOW("ATC: N_AT_L, len:%d,hex:%s,line:%s", atc_line_length, tchar, atc_global_info_ptr->cur_cmd_line_buf_ptr);
    }
    else
    {
        uint8   line_str[201] = {0};
        uint8   temp_str_len = 0;
        uint8  *temp_str = PNULL;
        uint8 i;
        
        temp_str = atc_global_info_ptr->cur_cmd_line_buf_ptr;
        temp_str_len = atc_line_length/200 + ((atc_line_length % 200) ? 1 : 0);
        ATC_TRACE_LOW("ATC: N_AT_L, len:%d, hex: too long, separated into %d segments ", atc_line_length, temp_str_len);
        for (i = 0; i < temp_str_len; i++)
        {
            strncpy((char *)line_str, (char *)(temp_str + i * 200), 200);
            line_str[200] = '\0';
            ATC_TRACE_LOW("ATC: Rec Seg [%d]:%s", i, line_str);
        }
    }

    if(atc_global_info_ptr->atc_config_ptr->s3_reg  == *atc_global_info_ptr->cur_cmd_line_buf_ptr ||
            atc_global_info_ptr->atc_config_ptr->s4_reg == *atc_global_info_ptr->cur_cmd_line_buf_ptr)
    {
        //ATC_TRACE_LOW("ATC:desert the unuseful char.");
        atc_global_info_ptr->cur_cmd_line_buf_ptr ++;
        atc_line_length --;
    }

    // remove ; from atc line for atcparase
    if((atc_line_length > 2) &&
            (atc_global_info_ptr->cur_cmd_line_buf_ptr[atc_line_length - 1] == atc_global_info_ptr->atc_config_ptr->s3_reg) &&
            (atc_global_info_ptr->cur_cmd_line_buf_ptr[atc_line_length - 2] == ';'))
    {
        atc_global_info_ptr->cur_cmd_line_buf_ptr[atc_line_length - 2] = atc_global_info_ptr->atc_config_ptr->s3_reg;
        atc_global_info_ptr->cur_cmd_line_buf_ptr[atc_line_length - 1] = '\0'; //add null terminater for atcparse
        atc_line_length --;
    }

    if(atc_global_info_ptr->atc_config_ptr->is_ctrlz_end)
    {
        atc_global_info_ptr->atc_config_ptr->is_ctrlz_end = FALSE;

        switch(atc_global_info_ptr->atc_config_ptr->ctrlz_end_status)
        {
            case ATC_CTRLZ_SMS:
                status = ATC_ProcessSmsPdu(atc_global_info_ptr->atc_config_ptr,
                                           atc_global_info_ptr->cur_cmd_line_buf_ptr);
                break;

            case ATC_CTRLZ_SMS_STATUS:
                status = ATC_ProcessSmsStatusPdu(atc_global_info_ptr->atc_config_ptr,
                                                 atc_global_info_ptr->cur_cmd_line_buf_ptr);
                break;
#ifndef ATC_SAT_ENABLE  
            case ATC_CTRLZ_STK:
    #ifndef _ULTRA_LOW_CODE_
                status = ATC_ProcessSioStkData(atc_global_info_ptr->atc_config_ptr,
                                               atc_global_info_ptr->cur_cmd_line_buf_ptr);
    #endif
                break;

            case ATC_CTRLZ_SRWD:
    #ifndef _ULTRA_LOW_CODE_
                status = ATC_ProcessSioSRWDData(atc_global_info_ptr->atc_config_ptr,
                                                atc_global_info_ptr->cur_cmd_line_buf_ptr);
    #endif
                break;
#endif

            case ATC_CTRLZ_SSEA:
                status = ATC_ProcessSioSSEAData(atc_global_info_ptr->atc_config_ptr,
                                                atc_global_info_ptr->cur_cmd_line_buf_ptr, atc_line_length);
                break;

            case ATC_CTRLZ_SADM:
                status = ATC_ProcessSioSADMData(atc_global_info_ptr->atc_config_ptr,
                                                atc_global_info_ptr->cur_cmd_line_buf_ptr, atc_line_length);
                break;

            case ATC_CTRLZ_SADMNV:
                status = ATC_ProcessSioSADMNVData(atc_global_info_ptr->atc_config_ptr,
                                                  atc_global_info_ptr->cur_cmd_line_buf_ptr, atc_line_length);
                break;

            case ATC_CTRLZ_GPRS:
        #ifdef _SUPPORT_GPRS_
                status = ATC_ProcessSioData(atc_global_info_ptr->atc_config_ptr, 
                                            atc_global_info_ptr->cur_cmd_line_buf_ptr, atc_line_length);
                SCI_TRACE_LOW("ATC: ATC_CTRLZ_GPRS, status=%d",status);
        #endif
                break;

            case ATC_CTRLZ_SNVM:
#ifdef _ATC_ONLY
                status = ATC_ProcessSioSNVMData(atc_global_info_ptr->atc_config_ptr,
                                                atc_global_info_ptr->cur_cmd_line_buf_ptr);
#endif
                break;
             case ATC_CTRLZ_SADM_ARM:
#ifdef AUDIO_PARAM_ARM
                status = ATC_ProcessSioARMSADMData(atc_global_info_ptr->atc_config_ptr,
                                                   atc_global_info_ptr->cur_cmd_line_buf_ptr,atc_line_length);
#endif
                break;

            case ATC_CTRLZ_SADMNV_ARM:
#ifdef AUDIO_PARAM_ARM
                status = ATC_ProcessSioARMSADMNVData(atc_global_info_ptr->atc_config_ptr,
                                                     atc_global_info_ptr->cur_cmd_line_buf_ptr,atc_line_length);
#endif
                break;
            case ATC_CTRLZ_SADMDSP:
            {
#ifdef AUDIO_PARAM_ARM
                status = ATC_ProcessSioDSPSADMData(atc_global_info_ptr->atc_config_ptr,
                                                   atc_global_info_ptr->cur_cmd_line_buf_ptr, atc_line_length);
#endif
                break;
            }

            case ATC_CTRLZ_SADMNV_DSP:
            {
#ifdef AUDIO_PARAM_ARM
                status = ATC_ProcessSioDSPSADMNVData(atc_global_info_ptr->atc_config_ptr,
                                                     atc_global_info_ptr->cur_cmd_line_buf_ptr, atc_line_length);
#endif
                break;
            }

            case ATC_CTRLZ_LVVE:
            {
                status = ATC_ProcessSioSADLVVEData(atc_global_info_ptr->atc_config_ptr,
                                                   atc_global_info_ptr->cur_cmd_line_buf_ptr,atc_line_length);
                break;
            }
            
            case ATC_CTRLZ_LVVENV:
            {
                status = ATC_ProcessSioSADLVVENVData(atc_global_info_ptr->atc_config_ptr,
                                                     atc_global_info_ptr->cur_cmd_line_buf_ptr,atc_line_length);
                break;
            }

#ifndef ATC_SAT_ENABLE
            case ATC_CTRLZ_ENHA:
                status = ATC_ProcessSioENHA_Data(atc_global_info_ptr->atc_config_ptr,
                                                atc_global_info_ptr->cur_cmd_line_buf_ptr,atc_line_length);
                break;
            case ATC_CTRLZ_ENHA_TUN:
                status = ATC_ProcessSioENHA_Tun_Data(atc_global_info_ptr->atc_config_ptr,
                                                atc_global_info_ptr->cur_cmd_line_buf_ptr,atc_line_length);
                break;

            case ATC_CTRLZ_ENHANV:
                status = ATC_ProcessSioENHA_NVData(atc_global_info_ptr->atc_config_ptr,
                                                atc_global_info_ptr->cur_cmd_line_buf_ptr,atc_line_length);
                break;

            case ATC_CTRLZ_ENHANV_TUN:
                status = ATC_ProcessSioENHA_Tun_NVData(atc_global_info_ptr->atc_config_ptr,
                                                atc_global_info_ptr->cur_cmd_line_buf_ptr,atc_line_length);
                break;
    #endif /*ATC_SAT_ENABLE*/
            default:
               ATC_TRACE_LOW("ATC: N_AT_L, ctrlz_end_status is not allowed");
               return S_ATC_DEFAULT_HANDLE;
        }

        SIO_ATC_SetCmdLineTerminateChar(atc_global_info_ptr->atc_config_ptr->s3_reg, '\0');
    }
    else
    {
        /* Parse the AT command line */
        zerolex(); //add for reset lex state

        if(atc_err_protected_flag && atc_global_info_ptr->cur_cmd_line_buf_ptr)
        {
            const uint8 *buf_ptr;
            uint         buf_len;

            buf_ptr = atcparse_at(atc_global_info_ptr->cur_cmd_line_buf_ptr, atc_line_length);
            buf_len = atc_line_length - ((uint32)buf_ptr - (uint32)atc_global_info_ptr->cur_cmd_line_buf_ptr);
            if (buf_len < atc_line_length)
                ATC_TRACE_LOW("ATC: N_AT_L, ignore ip data, len:%d->%d, newatcmd:%s", atc_line_length, buf_len, (const char *)buf_ptr);
            atc_global_info_ptr->cur_cmd_line_buf_ptr = (uint8 *)buf_ptr;
            atc_line_length = buf_len;

        }

        atc_global_info_ptr->cur_cmd_line_buf_ptr[atc_line_length-1] = 0xD;
        status = atcparse(atc_global_info_ptr->cur_cmd_line_buf_ptr, &atc_cur_info_ptr);

        if(((0x41 == *atc_global_info_ptr->cur_cmd_line_buf_ptr) ||            //'A'
                (0x61 == *atc_global_info_ptr->cur_cmd_line_buf_ptr))  &&      //'a'
                ((0x54 == *(atc_global_info_ptr->cur_cmd_line_buf_ptr + 1)) || //'T'
                 (0x74 == *(atc_global_info_ptr->cur_cmd_line_buf_ptr + 1))))   //'t'
        {
            //if cmd ptr starts with "AT" string, it will be considered as AT command
            if((0x23 == *(atc_global_info_ptr->cur_cmd_line_buf_ptr + 2)) &&    //'#'
                    (0x23 == *(atc_global_info_ptr->cur_cmd_line_buf_ptr + 3)))
            {
                SCI_MEMSET(para_str, 0, ATC_SHARP_LEN);
                para_len = atc_line_length - 4;

                if(para_len > ATC_SHARP_LEN)
                {
                    ATCM_FreeAll();
                    ATC_TRACE_LOW("ATC: N_AT_L, Assert Failure sharp len too long.");
                    return ERR_OPERATION_NOT_SUPPORTED;
                }

                SCI_MEMCPY(para_str, (atc_global_info_ptr->cur_cmd_line_buf_ptr + 4), para_len);
                para_len = para_len - 1;

                //call the extern function
                ATC_TRACE_LOW("ATC: N_AT_L, Call the external function");
            #ifndef _ATC_ONLY
                ATC_ExtCmd(para_str, para_len);
            #endif
                ATCM_FreeAll();

                //need do nothing while received AT## command
                return S_ATC_DEFAULT_HANDLE;
            }
        }
        else
        {
            //if cmd ptr do not start with "AT" string, it will be discarded
            ATCM_FreeAll();
            return S_ATC_DEFAULT_HANDLE;
        }
    }

    switch(status)
    {
        case S_ATC_SUCCESS:
            SCI_ASSERT(PNULL != atc_cur_info_ptr);/*assert verified: check null pointer*/
            ATC_TRACE_LOW("ATC: N_AT_L, Parse success.");

            /*
             ** we link the infos of AT cmds to the end of the list which
             ** store in atc_config
             */
            if(PNULL == atc_global_info_ptr->atc_info_ptr)
            {
                atc_global_info_ptr->atc_info_ptr = atc_cur_info_ptr;
            }
            else
            {
                atc_last_info_ptr = atc_global_info_ptr->atc_info_ptr;

                while(PNULL != atc_last_info_ptr->next)
                {
                    atc_last_info_ptr = atc_last_info_ptr->next;
                }

                atc_last_info_ptr->next = atc_cur_info_ptr;
            }

            break;

        case S_ATC_DEFAULT_HANDLE:
            ATC_TRACE_LOW("ATC: N_AT_L, Parse SMS PDU successful");
            break;

        default:
            ATC_TRACE_LOW("ATC: N_AT_L, Parse failed.");
            //ATCS_TRACE_STATUS( status );
            //response the result code
            ATC_BuildResultCodeRsp(atc_global_info_ptr->atc_config_ptr, S_ATC_RESULT_CODE_ERROR, status);
            break;
    }

    return status;
}

/******************************************************************************/
// Description : This function processes the AT commands stored in the AT
//               config struct one by one.
// Global resource dependence : None
// Author : Steven
// Note : None
/******************************************************************************/
LOCAL ATC_STATUS ATC_ProcessCmds( // Return S_ATC_SUCCESS if success,
    // otherwise return error code.
    ATC_GLOBAL_INFO_T      *atc_global_info_ptr // point to the ATC_GLOBAL_INFO_T struct.
)
{
    ATC_STATUS      status = S_ATC_SUCCESS;


    SCI_ASSERT(PNULL != atc_global_info_ptr);/*assert verified: check null pointer*/

    while(PNULL != atc_global_info_ptr->atc_info_ptr)
    {
        status = ATC_DespatchCmd(atc_global_info_ptr, atc_global_info_ptr->atc_info_ptr);

        if(S_ATC_SUCCESS != status && S_ATC_DEFAULT_HANDLE != status)
        {
            atc_global_info_ptr->atc_info_ptr = PNULL;
            break;
        }

        atc_global_info_ptr->atc_info_ptr = atc_global_info_ptr->atc_info_ptr->next;
    }

    switch(status)
    {
        case S_ATC_SUCCESS:
            SCI_ASSERT(PNULL == atc_global_info_ptr->atc_info_ptr);/*assert verified: check valid value*/

            ATC_BuildResultCodeRsp(atc_global_info_ptr->atc_config_ptr,
                                   S_ATC_RESULT_CODE_OK , 0);
            break;

        case S_ATC_LAST_ERROR:
            ATC_BuildResultCodeRsp(atc_global_info_ptr->atc_config_ptr,
                                   S_ATC_RESULT_CODE_OK , 0);
            ATC_BuildResultCodeRsp(atc_global_info_ptr->atc_config_ptr,
                                   S_ATC_RESULT_CODE_ERROR , status);
            status = S_ATC_SUCCESS;
            break;

        case S_ATC_DEFAULT_HANDLE:
            break;

        default:
            ATC_BuildResultCodeRsp(atc_global_info_ptr->atc_config_ptr,
                                   S_ATC_RESULT_CODE_ERROR , status);
            break;
    }

    // If the AT commands are all handled, we free our private heap mem
    ATCM_FreeAll();
    return status;
}
#endif


/******************************************************************************/
// Description : This function despatches AT command to its handle.
// Global resource dependence :
// Author : Steven
// Note : None
/******************************************************************************/
LOCAL ATC_STATUS ATC_DespatchCmd(                                               // Return S_ATC_STATUS if success, otherwise return error code.
    ATC_GLOBAL_INFO_T      *atc_global_info_ptr,   // point to the ATC_GLOBAL_INFO_T struct.
    ATC_CUR_INFO_LIST_T    *atc_cur_info_ptr       // Point to the info struct of AT command
)
{
    ATC_STATUS  status = S_ATC_SUCCESS;
    MN_DUAL_SYS_E dual_sys = ATC_CURRENT_SIM_ID_MUX;

    SCI_ASSERT(PNULL != atc_global_info_ptr);/*assert verified: check null pointer*/
    SCI_ASSERT(PNULL != atc_cur_info_ptr);/*assert verified: check null pointer*/

    status = ATC_CheckCmd(atc_global_info_ptr, atc_cur_info_ptr);

    if(S_ATC_SUCCESS == status)
    {
        ATC_TRACE_LOW("ATC: ATC_DespatchCmd, cmd_type:%d(1-exe,2-set,4-read,8-test)", (atc_cur_info_ptr->cmd_type & 0x000F));

        //check the SIM card
        if(((S_CONFIG_INFO_PTR)->sim_flag[dual_sys] != TRUE) ||
                ((S_CONFIG_INFO_PTR)->cpin[dual_sys] != ATC_CPIN_CODE_READY))
        {
            ATC_TRACE_LOW("ATC: ATC_DespatchCmd, sim_flag:%d, cpin:%d",
                          (S_CONFIG_INFO_PTR)->sim_flag[dual_sys], (S_CONFIG_INFO_PTR)->cpin[dual_sys]);
        }

        if(ATC_CMD_TYPE_TEST == (atc_cur_info_ptr->cmd_type & 0x000F))
        {
            // add for handle the specific test command
            switch(atc_cur_info_ptr->cmd_index)
            {
                case AT_CMD_CPBR:
                case AT_CMD_CPBS:
                case AT_CMD_CPBF:
                case AT_CMD_CPBW:
                case AT_CMD_CPMS:
                case AT_CMD_CPBP:
                case AT_CMD_CPBN:
                case AT_CMD_CNUM:
                case AT_CMD_CSVM:
                case AT_CMD_CCWA:
                case AT_CMD_COPS:
                case AT_CMD_SAC:
                case AT_CMD_CMGD:
                case AT_CMD_CMGR:
                case AT_CMD_CMGW:
                case AT_CMD_CMGL:
                case AT_CMD_CGACT:
                case AT_CMD_CGDCONT:
                case AT_CMD_CGPCO:
                case AT_CMD_SMSC:
                case AT_CMD_CGQREQ:
                case AT_CMD_CPOL:
                case AT_CMD_CAMM:
                case AT_CMD_CLCC:
                case AT_CMD_CSCA:
                case AT_CMD_CIMI:
                case AT_CMD_CGSN:
                case AT_CMD_CUSD:
                case AT_CMD_CMGS:
                case AT_CMD_CCFC:
                case AT_CMD_CGMI:
                case AT_CMD_CGMM:
                case AT_CMD_CGMR:
                case AT_CMD_CEER:
                case AT_CMD_CACM:
                case AT_CMD_CPUC:
                case AT_CMD_VTD:
                case AT_CMD_CMSS:
                case AT_CMD_CSMP:
            #ifndef ATC_SAT_ENABLE
                case AT_CMD_CGANS:
                case AT_CMD_CSCB:           
                case AT_CMD_CHUP:
                case AT_CMD_CGDSCONT:
                case AT_CMD_CGTFT:
                case AT_CMD_CGEQNEG: 
                case AT_CMD_CNMA:                
                case AT_CMD_SHARP_CLS:
                case AT_CMD_STSF:
                case AT_CMD_FCLASS:
                case AT_CMD_GCI:
                case AT_CMD_ERGA:
                case AT_CMD_ERTCA:
                case AT_CMD_SIPCONFIG:
                case AT_CMD_xor_MBAU:
                case AT_CMD_xor_SCPBR:
                case AT_CMD_xor_SCPBW:
                case AT_CMD_xor_SPN:  
                case AT_CMD_xor_DSCI:                    
            #endif
                    status = atc_global_info_ptr->atc_handle_table[atc_cur_info_ptr->cmd_index]
                             (atc_global_info_ptr->atc_config_ptr, atc_cur_info_ptr);
                    break;
                default:
                    ATC_TRACE_LOW("ATC: ATC_DespatchCmd, %s Test Command: Default Response",
                                  g_atc_info_table[atc_cur_info_ptr->cmd_index].cmd_name->str_ptr);
                    status = ATC_BuildTestCmdResponse(atc_global_info_ptr, atc_cur_info_ptr);
                    break;
            }
        }
        else
        {
            status = atc_global_info_ptr->atc_handle_table[atc_cur_info_ptr->cmd_index]
                     (atc_global_info_ptr->atc_config_ptr, atc_cur_info_ptr);
        }
    }
    else if(S_ATC_LAST_ERROR == status)
    {
        status = S_ATC_LAST_ERROR;
    }
    else
    {
        status = S_ATC_RESULT_CODE_ERROR;
    }

    return status;
}


/******************************************************************************/
// Description : This function checks the current received AT command.
// Global resource dependence : None
// Author : Steven
// Note : None
/******************************************************************************/
LOCAL ATC_STATUS ATC_CheckCmd(  // Return S_ATC_SUCCESS if command is valid,
    // otherwise return error code.
    ATC_GLOBAL_INFO_T   *atc_global_info_ptr, // point to the ATC_GLOBAL_INFO_T struct.
    ATC_CUR_INFO_LIST_T *atc_cur_info_ptr  // Point to the info struct of
    // AT command.
)
{
    ATC_STATUS                   status              = S_ATC_SUCCESS;
    const ATC_INFO_T             *atc_info_ptr       = PNULL;
    ATC_CUR_PARAM_INFO_LIST_T    *cur_param_info_ptr = PNULL;
    const ATC_PARAM_INFO_T       *param_info_ptr     = PNULL;
    const ATC_PARAM_NUMERIC_T    *num_param_ptr      = PNULL;
    const ATC_PARAM_STRING_T     *str_param_ptr      = PNULL;
    uint16                       i                   = 0;


    SCI_ASSERT(PNULL != atc_global_info_ptr);/*assert verified: check null pointer*/
    SCI_ASSERT(PNULL != atc_cur_info_ptr);/*assert verified: check null pointer*/

    atc_info_ptr = &atc_global_info_ptr->atc_info_table[atc_cur_info_ptr->cmd_index];
    SCI_ASSERT(atc_info_ptr->cmd_index == atc_cur_info_ptr->cmd_index);/*assert verified: check valid value*/

    if(atc_cur_info_ptr->cmd_type & ATC_CMD_TYPE_ERROR)
    {
        status = S_ATC_LAST_ERROR;
        ATC_TRACE_LOW("===ATC_CMD_TYPE_ERROR");
        goto CheckCmdDone;
    }

    /* Check AT Command Type */
    if(!(atc_cur_info_ptr->cmd_type & atc_info_ptr->cmd_type & 0x000F))
    {
        status = ERR_OPERATION_NOT_SUPPORTED;
        ATC_TRACE_LOW("===ERR_OPERATION_NOT_SUPPORTED cmd type[0x%x] than expected[0x%x]", atc_cur_info_ptr->cmd_type, atc_info_ptr->cmd_type);
        goto CheckCmdDone;
    }

    /* If Command action type is not SET, then we need not to check param */
    if(ATC_CMD_TYPE_SET != (atc_cur_info_ptr->cmd_type & 0x000F))
    {
        status = S_ATC_SUCCESS;
        ATC_TRACE_LOW("===Command action is not set");
        goto CheckCmdDone;
    }

    /* Check AT Command Parameter Number */
    //the num of params should equal or less than the max param numbers
    if(atc_cur_info_ptr->cur_param_num > atc_info_ptr->param_num)
    {
        status = ERR_INVALID_INDEX;
        ATC_TRACE_LOW("===ERR_INVALID_INDEX %d than preset %d", atc_cur_info_ptr->cur_param_num, atc_info_ptr->param_num);
        goto CheckCmdDone;
    }

    if(0 == atc_info_ptr->param_num)
    {
        status = S_ATC_SUCCESS;
        ATC_TRACE_LOW("===zero param");
        goto CheckCmdDone;
    }

    param_info_ptr = atc_info_ptr->param_ptr;
    cur_param_info_ptr = atc_cur_info_ptr->cur_param_list_ptr;

    while(PNULL != cur_param_info_ptr)
    {
        if((ATC_CMD_PARAM_TYPE_DEFAULT != param_info_ptr->param_type) &&
                (ATC_CMD_PARAM_TYPE_DEFAULT != cur_param_info_ptr->param_type))
        {
            if(param_info_ptr->param_type != cur_param_info_ptr->param_type)
            {
                status = ERR_OPERATION_NOT_ALLOWED;
                ATC_TRACE_LOW("===ERR_OPERATION_NOT_ALLOWED");
                goto CheckCmdDone;
            }
        }

        switch(param_info_ptr->param_type)
        {
            case ATC_CMD_PARAM_TYPE_NUMERIC:
                num_param_ptr = param_info_ptr->param_info.num_array_ptr;

                for(i = 0; i < param_info_ptr->param_range_num; i++)
                {
                    if(cur_param_info_ptr->param_info.num < num_param_ptr->val_min)
                    {
                        status = ERR_OPERATION_NOT_ALLOWED;
                        ATC_TRACE_LOW("===ERR_OPERATION_NOT_ALLOWED in ATC_CMD_PARAM_TYPE_NUMERIC case 1");
                        goto CheckCmdDone;
                    }

                    if((cur_param_info_ptr->param_info.num > num_param_ptr->val_max) &&
                            (i == (param_info_ptr->param_range_num - 1)))
                    {
                        status = ERR_OPERATION_NOT_ALLOWED;
                        ATC_TRACE_LOW("===ERR_OPERATION_NOT_ALLOWED in ATC_CMD_PARAM_TYPE_NUMERIC case 2");
                        goto CheckCmdDone;
                    }

                    if((cur_param_info_ptr->param_info.num >= num_param_ptr->val_min) &&
                            (cur_param_info_ptr->param_info.num <= num_param_ptr->val_max))
                    {
                        break;
                    }

                    num_param_ptr++;
                } /* End of loop for(i) */

                break;

            case ATC_CMD_PARAM_TYPE_STRING:
                str_param_ptr = param_info_ptr->param_info.str_array_ptr;

                for(i = 0; i < param_info_ptr->param_range_num; i++)
                {
                    //add check valid the pointer of string type param
                    if(cur_param_info_ptr->param_info.str_ptr != PNULL &&
                            0 == strcmp((char *)cur_param_info_ptr->param_info.str_ptr->str_ptr, (char *)str_param_ptr->str_ptr))
                    {
                        break;
                    }
                    else
                    {
                        if((param_info_ptr->param_range_num - 1) == i)
                        {
                            status = ERR_OPERATION_NOT_ALLOWED;
                            ATC_TRACE_LOW("===ERR_OPERATION_NOT_ALLOWED in ATC_CMD_PARAM_TYPE_STRING case 1");
                            goto CheckCmdDone;
                        }
                    }

                    str_param_ptr++;
                } /* End of loop for(i) */

                break;

            default:
                break;
        } /* End of switch ( cur_param_info_ptr->param_type ) */

        param_info_ptr++;
        cur_param_info_ptr = cur_param_info_ptr->next;
    } /* End of loop while ( PNULL != cur_param_info_ptr ) */

CheckCmdDone:

    if(S_ATC_SUCCESS == status)
    {
        ATC_TRACE_LOW("ATC: Check AT Cmd valid.");
    }
    else
    {
        ATC_TRACE_LOW("ATC: Check AT Cmd invalid.");
        ATCS_TRACE_STATUS(status);
    }

    return status;
}

/******************************************************************************/
// Description : This function contructs the result code response
// Global resource dependence :
// Author : Ivan
// Note :
// History:
/******************************************************************************/
void ATC_BuildResultCodeRsp(
    ATC_CONFIG_T       *atc_config_ptr,  // point to the ATC_CONFIG_T struct.
    int32              result_code_idx,  // result code index
    int32              err_code_idx      //error code index
)
{
    uint8       *rsp_buf_ptr  = g_rsp_str;
    uint8       cr_val;
    uint8       lf_val;

    SCI_ASSERT(PNULL != atc_config_ptr);/*assert verified: check null pointer*/

    cr_val = atc_config_ptr->s3_reg;
    lf_val = atc_config_ptr->s4_reg;

    if(err_code_idx == ERR_TEXT_STRING_TOO_LONG)
    {
        ATC_TRACE_LOW("ATC: RsltCodeRsp, err text string too long");

        g_rsp_str[0] = cr_val;
        g_rsp_str[1] = lf_val;
        rsp_buf_ptr = &g_rsp_str[2];

        sprintf((char *)rsp_buf_ptr, "%s%ld%c%c",
                "+CME ERROR: ", err_code_idx, cr_val, lf_val);
#ifdef _MUX_ENABLE_
        ATC_TRACE_LOW("ATC: RsltCodeRsp link_id: %d, str: %s", 
            atc_config_ptr->current_link_id, (char *)rsp_buf_ptr);
#else
        ATC_TRACE_LOW("ATC: RsltCodeRsp str: %s", (char *)rsp_buf_ptr);
#endif
        ATC_SendReslutRsp(atc_config_ptr, g_rsp_str, strlen((char*)g_rsp_str));
        return;
    }

    if(atc_config_ptr->q_value)
    {
        ATC_TRACE_LOW("ATC: RsltCodeRsp, q_value is TRUE");
        return ;
    }

    if(0 == atc_config_ptr->v_value)
    {
        sprintf((char *)rsp_buf_ptr, "%ld%c",
                (result_code_idx - S_ATC_RESULT_CODE_BASE), cr_val);
    }
    else
    {
        g_rsp_str[0] = cr_val;
        g_rsp_str[1] = lf_val;
        rsp_buf_ptr = &g_rsp_str[2];

        if(S_ATC_RESULT_CODE_ERROR == result_code_idx)
        {
            switch(atc_config_ptr->atc_config_nv.cmee)
            {
                case NO_RESULT_CODE_MODE:
                    sprintf((char *)rsp_buf_ptr, "%s%c%c",
                            (char *)g_atc_result_code_text[result_code_idx - S_ATC_RESULT_CODE_BASE], cr_val, lf_val);
                    break;

                case NUMERIC_ERR_VALUE_MODE:
                    sprintf((char *)rsp_buf_ptr, "%s%ld%c%c",
                            "+CME ERROR: ", err_code_idx, cr_val, lf_val);
                    break;

                case VERBOSE_ERR_VALUE_MODE:
                    sprintf((char *)rsp_buf_ptr, "+CME ERROR: "); //, err_verbose, cr_val, lf_val);
                    rsp_buf_ptr += strlen("+CME ERROR: ");
                    ATC_GetErrVerbose(err_code_idx, (char *)rsp_buf_ptr);
                    rsp_buf_ptr += strlen((char *)rsp_buf_ptr);
                    sprintf((char *)rsp_buf_ptr, "%c%c", cr_val, lf_val);
                    break;

                default:
                    SCI_ASSERT(FALSE);/*assert verified: check valid value*/
                    break;
            }
        }
        else
        {
            sprintf((char *)rsp_buf_ptr, "%s%c%c",
                    (char *)g_atc_result_code_text[result_code_idx - S_ATC_RESULT_CODE_BASE], cr_val, lf_val);
        }
    }

#ifdef _MUX_ENABLE_
    ATC_TRACE_LOW("ATC: RsltCodeRsp link_id: %d, str: %s", 
        atc_config_ptr->current_link_id, (char *)g_rsp_str);
#else
    ATC_TRACE_LOW("ATC: RsltCodeRsp str: %s",(char *)g_rsp_str);
#endif
    ATC_SendReslutRsp(atc_config_ptr, g_rsp_str, strlen((char*)g_rsp_str));
}
/******************************************************************************/
// Description : This function contructs the result code response
// Global resource dependence :
// Author : wanghui
// Note :
// History:
/******************************************************************************/
void ATC_BuildResultErrCodeRsp(
    ATC_CONFIG_T       *atc_config_ptr,  // point to the ATC_CONFIG_T struct.
    int32              result_code_idx,  // result code index
    int32              err_code_idx      //error code index
)
{
    uint8       *rsp_buf_ptr  = g_rsp_str;
    uint8       cr_val;
    uint8       lf_val;

    SCI_ASSERT(PNULL != atc_config_ptr);/*assert verified: check null pointer*/

    cr_val = atc_config_ptr->s3_reg;
    lf_val = atc_config_ptr->s4_reg;

    if(atc_config_ptr->q_value)
    {
        ATC_TRACE_LOW("ATC: RsltErrCodeRsp, q_value is TRUE");
        return ;
    }

    if(0 == atc_config_ptr->v_value)
    {
        sprintf((char *)rsp_buf_ptr, "%ld%c",
                (result_code_idx - S_ATC_RESULT_CODE_BASE), cr_val);
    }
    else
    {
        g_rsp_str[0] = cr_val;
        g_rsp_str[1] = lf_val;
        rsp_buf_ptr = &g_rsp_str[2];

        sprintf((char *)rsp_buf_ptr, "%s%c%c",
                (char *)g_atc_result_code_text[result_code_idx - S_ATC_RESULT_CODE_BASE], cr_val, lf_val);
    }

#ifdef _MUX_ENABLE_
    ATC_TRACE_LOW("ATC: RsltErrCodeRsp link_id: %d, str: \n%s", 
        atc_config_ptr->current_link_id, (char *)g_rsp_str);
#else
    ATC_TRACE_LOW("ATC: RsltErrCodeRsp str: \n%s", (char *)g_rsp_str);
#endif
    ATC_SendReslutRsp(atc_config_ptr, g_rsp_str, strlen((char*)g_rsp_str));
}

/******************************************************************************/
// Description : This function contructs the result code response for SMS commands
// Global resource dependence :
// Author : Ivan
// Note :
/******************************************************************************/
void ATC_BuildSmsCodeRsp(
    ATC_CONFIG_T       *atc_config_ptr,  // point to the ATC_CONFIG_T struct.
    int32              err_code_idx      //error code index
)
{
    uint8       *rsp_buf_ptr  = g_rsp_str;
    uint8       cr_val;
    uint8       lf_val;

    SCI_ASSERT(PNULL != atc_config_ptr);/*assert verified: check null pointer*/

    cr_val = atc_config_ptr->s3_reg;
    lf_val = atc_config_ptr->s4_reg;

    g_rsp_str[0] = cr_val;
    g_rsp_str[1] = lf_val;
    rsp_buf_ptr = &g_rsp_str[2];

    if(err_code_idx <= ATC_SMS_ERR_512)
    {
        switch(atc_config_ptr->atc_config_nv.cmee)
        {
            case NO_RESULT_CODE_MODE:
                sprintf((char *)rsp_buf_ptr, "%s%c%c",
                        (char *)g_atc_result_code_text[S_ATC_RESULT_CODE_ERROR - S_ATC_RESULT_CODE_BASE], cr_val, lf_val);
                break;

            case NUMERIC_ERR_VALUE_MODE:
                sprintf((char *)rsp_buf_ptr, "%s%d%c%c", "+CMS ERROR: ",
                        s_sms_verbos_info[err_code_idx].error_code, cr_val, lf_val);
                break;

            case VERBOSE_ERR_VALUE_MODE:
                sprintf((char *)rsp_buf_ptr, "%s%s%c%c", "+CMS ERROR: ",
                        s_sms_verbos_info[err_code_idx].error_str, cr_val, lf_val);
                break;

            default:
                SCI_ASSERT(FALSE);/*assert verified: check valid value*/
                break;
        }
    }

#ifdef _MUX_ENABLE_
    ATC_TRACE_LOW("ATC: SmsRsltCodeRsp link_id: %d, str: %s", 
        atc_config_ptr->current_link_id, (char *)g_rsp_str);
#else
    ATC_TRACE_LOW("ATC: SmsRsltCodeRsp str: %s", (char *)g_rsp_str);
#endif
    ATC_SendReslutRsp(atc_config_ptr, g_rsp_str, strlen((char*)g_rsp_str));
}
/******************************************************************************/
// Description : This function constructs the AT command response.
// Global resource dependence : None
// Author : Ivan
// Note :
/******************************************************************************/
void ATC_BuildInfoRsp(
    ATC_CONFIG_T   *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    uint8          *rsp_str_ptr     // Point the response string
)
{
    uint8      cr_val;                 // holds the atc_config_ptr->S3_reg value
    uint8      lf_val;                 // holds the atc_config_ptr->S4_reg value
    uint8      v_val;                  // holds the atc_config_ptr->V_option value
    uint8      rsp_buf[10];

    SCI_ASSERT(PNULL != atc_config_ptr);/*assert verified: check null pointer*/
    SCI_ASSERT(PNULL != rsp_str_ptr);/*assert verified: check null pointer*/

    // Set the config values
    cr_val = atc_config_ptr->s3_reg;
    lf_val = atc_config_ptr->s4_reg;
    v_val  = atc_config_ptr->v_value;

    if(atc_config_ptr->q_value)
    {
        ATC_TRACE_LOW("ATC: InfoRsp, q_value is TRUE");
        return;
    }

    if(atc_config_ptr->v_value)
    {
        sprintf((char *)rsp_buf, "%c%c", cr_val, lf_val);
        ATC_SendReslutRsp(atc_config_ptr, rsp_buf, strlen((char*)rsp_buf));
    }

#ifdef _MUX_ENABLE_
    ATC_TRACE_LOW("ATC: InfoRsp link_id: %d, str: \n%s", 
        atc_config_ptr->current_link_id, (char *)rsp_str_ptr);
#else
    ATC_TRACE_LOW("ATC: InfoRsp str: \n%s", (char *)rsp_str_ptr);
#endif
    ATC_SendReslutRsp(atc_config_ptr, rsp_str_ptr, strlen((char*)rsp_str_ptr));

    sprintf((char *)rsp_buf, "%c%c", cr_val, lf_val);
    ATC_SendReslutRsp(atc_config_ptr, rsp_buf, strlen((char*)rsp_buf));
}

/******************************************************************************/
// Description : This function builds test command response.
// Global resource dependence : None
// Author : Steven
// Note : None
/******************************************************************************/
LOCAL ATC_STATUS ATC_BuildTestCmdResponse(  // Return S_ATC_SUCCESS if success,
    // otherwise return error code.
    ATC_GLOBAL_INFO_T   *atc_global_info_ptr, // point to the ATC_GLOBAL_INFO_T struct.
    ATC_CUR_INFO_LIST_T *atc_cur_info_ptr // Point the info struct of cmd
)
{
    ATC_STATUS                    status          = S_ATC_SUCCESS;
    const ATC_INFO_T              *atc_info_ptr   = PNULL;
    const ATC_PARAM_INFO_T        *param_info_ptr = PNULL;
    const ATC_PARAM_STRING_T      *str_param_ptr  = PNULL;
    const ATC_PARAM_NUMERIC_T     *num_param_ptr  = PNULL;

    uint8                         *res_buf_ptr    = PNULL;
    uint16                        i               = 0;
    uint16                        j               = 0;

    SCI_ASSERT(PNULL != atc_global_info_ptr);/*assert verified: check null pointer*/
    SCI_ASSERT(ATC_CMD_TYPE_TEST == (atc_cur_info_ptr->cmd_type & 0x0F));/*assert verified: check valid value*/

    atc_info_ptr = &atc_global_info_ptr->atc_info_table[atc_cur_info_ptr->cmd_index];
    SCI_ASSERT(atc_info_ptr->cmd_index == atc_cur_info_ptr->cmd_index);/*assert verified: check valid value*/

    res_buf_ptr = g_rsp_str;

    if(atc_global_info_ptr->atc_config_ptr->q_value)
    {
        ATC_TRACE_LOW("ATC: TestCmdRsp, q_value is TRUE");
        return status;
    }

    if(atc_global_info_ptr->atc_config_ptr->v_value)
    {
        *res_buf_ptr++ = atc_global_info_ptr->atc_config_ptr->s3_reg;
        *res_buf_ptr++ = atc_global_info_ptr->atc_config_ptr->s4_reg;
    }

    /* Put AT command name in response buf */
    SCI_ASSERT(atc_info_ptr->cmd_name->str_len <= MAX_ATC_RSP_LEN);/*assert verified: check valid value*/
    SCI_MEMCPY(res_buf_ptr, atc_info_ptr->cmd_name->str_ptr, atc_info_ptr->cmd_name->str_len);

    res_buf_ptr += atc_info_ptr->cmd_name->str_len;
    *res_buf_ptr++ = ':';

    param_info_ptr = atc_info_ptr->param_ptr;

    for(i = 0; i < atc_info_ptr->param_num; i++)
    {
        switch(param_info_ptr->param_type)
        {
            case ATC_CMD_PARAM_TYPE_NUMERIC:
                *res_buf_ptr++ = '(';
                num_param_ptr = param_info_ptr->param_info.num_array_ptr;

                for(j = 0; j < param_info_ptr->param_range_num; j++)
                {
                    sprintf((char *)res_buf_ptr, "%ld", num_param_ptr->val_min);
                    res_buf_ptr += strlen((const char *)res_buf_ptr);

                    if(num_param_ptr->val_min != num_param_ptr->val_max)
                    {
                        *res_buf_ptr++ = '-';
                        sprintf((char *)res_buf_ptr, "%ld", num_param_ptr->val_max);
                        res_buf_ptr += strlen((const char *)res_buf_ptr);
                    }

                    if((param_info_ptr->param_range_num - 1) != j)
                    {
                        *res_buf_ptr++ = ',';
                    }

                    num_param_ptr++;
                } /* End of loop for(i) */

                *res_buf_ptr++ = ')';
                break;

            case ATC_CMD_PARAM_TYPE_STRING:
                *res_buf_ptr++ = '(';
                str_param_ptr = param_info_ptr->param_info.str_array_ptr;

                for(j = 0; j < param_info_ptr->param_range_num; j++)
                {
                    *res_buf_ptr++ = '"';
                    memcpy(res_buf_ptr, str_param_ptr->str_ptr, str_param_ptr->str_len);
                    res_buf_ptr += str_param_ptr->str_len;
                    *res_buf_ptr++ = '"';

                    if((param_info_ptr->param_range_num - 1) != j)
                    {
                        *res_buf_ptr++ = ',';
                    }

                    str_param_ptr++;
                }

                *res_buf_ptr++ = ')';
                break;

            default:
                break;
        } /* End of switch ( param_info_ptr->param_type ) */

        if((atc_info_ptr->param_num - 1) != i)
        {
            *res_buf_ptr++ = ',';
        }

        param_info_ptr++;
    } /* End of loop for(i) */

    *res_buf_ptr++ = atc_global_info_ptr->atc_config_ptr->s3_reg;
    *res_buf_ptr++ = atc_global_info_ptr->atc_config_ptr->s4_reg;
    *res_buf_ptr   = '\0';

#ifdef _MUX_ENABLE_
    ATC_TRACE_LOW("ATC: TestCmdRsp link_id: %d, str: \n%s", 
        atc_global_info_ptr->atc_config_ptr->current_link_id, (char *)g_rsp_str);
#else
    ATC_TRACE_LOW("ATC: TestCmdRsp str: \n%s", (char *)g_rsp_str);
#endif
    ATC_SendReslutRsp(atc_global_info_ptr->atc_config_ptr, g_rsp_str, strlen((char*)g_rsp_str));

    return status;
}

/******************************************************************************/
// Description : This function get the error string of +CEER
// Global resource dependence : None
// Author : Pizer.Fan
// Note : None
// History:
/******************************************************************************/
BOOLEAN ATC_GetCeerErrVerbose(
    uint16 err_code,
    char  *err_verbose
)
{
    uint i;

    for(i = 0; i < sizeof(atc_ceer_error_verbose) / sizeof(atc_ceer_error_verbose[0]); i++)
    {
        if(atc_ceer_error_verbose[i].err_code == err_code)
        {
            strcpy(err_verbose, atc_ceer_error_verbose[i].err_verbose);
            return TRUE;
        }
    }

    strcpy(err_verbose, "unknown");
    return FALSE;
}

/******************************************************************************/
// Description : This function get the error string
// Global resource dependence : None
// Author : Ivan
// Note : None
// History:
/******************************************************************************/
LOCAL void ATC_GetErrVerbose(
    int32 err_idx,
    char *err_verbose
)
{

    switch(err_idx)
    {
        case ERR_PHONE_FAILURE:
        default:
            strcpy(err_verbose, "phone failure"); //0
            break;
        case ERR_NO_CONNECT_TO_PHONE:
            strcpy(err_verbose, "no connection to phone"); //1
            break;
        case ERR_PHONE_ADAPTOR_LINK_RESERVED:
            strcpy(err_verbose, "phone-adaptor link reserved"); //2
            break;
        case ERR_OPERATION_NOT_ALLOWED:
            strcpy(err_verbose, "operation not allowed"); //3
            break;
        case ERR_OPERATION_NOT_SUPPORTED:
            strcpy(err_verbose, "operation not supported"); //4
            break;
        case ERR_PH_SIM_PIN_REQUIRED:
            strcpy(err_verbose, "PH-SIM PIN required"); //5
            break;
        case ERR_PH_FSIM_PIN_REQUIRED:
            strcpy(err_verbose, "PH-FSIM PIN required"); //6
            break;
        case ERR_PH_FSIM_PUK_REQUIRED:
            strcpy(err_verbose, "PH-FSIM PUK required"); //7
            break;
        case ERR_SIM_NOT_INSERTED:
            strcpy(err_verbose, "SIM not inserted"); //10
            break;
        case ERR_SIM_PIN_REQUIRED:
            strcpy(err_verbose, "SIM PIN required"); //11
            break;
        case ERR_SIM_PUK_REQUIRED:
            strcpy(err_verbose, "SIM PUK required"); //12
            break;
        case ERR_SIM_FAILURE:
            strcpy(err_verbose, "SIM failure"); //13
            break;
        case ERR_SIM_BUSY:
            strcpy(err_verbose, "SIM busy"); //14
            break;
        case ERR_SIM_WRONG:
            strcpy(err_verbose, "SIM wrong"); //15
            break;
        case ERR_INCORRECT_PASSWORD:
            strcpy(err_verbose, "incorrect password"); //16
            break;
        case ERR_SIM_PIN2_REQUIRED:
            strcpy(err_verbose, "SIM PIN2 required"); //17
            break;
        case ERR_SIM_PUK2_REQUIRED:
            strcpy(err_verbose, "SIM PUK2 required"); //18
            break;
        case ERR_MEMORY_FULL:
            strcpy(err_verbose, "memory full"); //20
            break;
        case ERR_INVALID_INDEX:
            strcpy(err_verbose, "invalid index"); //21
            break;
        case ERR_NOT_FOUND:
            strcpy(err_verbose, "not found"); //22
            break;
        case ERR_MEMORY_FAILURE:
            strcpy(err_verbose, "memory failure"); //23
            break;
        case ERR_TEXT_STRING_TOO_LONG:
            strcpy(err_verbose, "text string too long"); //24
            break;
        case ERR_INVALID_CHARACTERS_IN_TEXT_STRING:
            strcpy(err_verbose, "invalid characters in text string"); //25
            break;
        case ERR_DIAL_STRING_TOO_LONG:
            strcpy(err_verbose, "dial string too long"); //26
            break;
        case ERR_INVALID_CHARACTERS_IN_DIAL_STRING:
            strcpy(err_verbose, "invalid characters in dial string"); //27
            break;
        case ERR_GPRS_OPERATION_FAILURE:
            strcpy(err_verbose, "GPRS operation failure"); //28
            break;
        case ERR_GPRS_SEND_DATA_FAILURE:
            strcpy(err_verbose, "GPRS send data failure"); //29
            break;
        case ERR_NO_NETWORK_SERVICE:
            strcpy(err_verbose, "no network service"); //30
            break;
        case ERR_NETWORK_TIMEOUT:
            strcpy(err_verbose, "network timeout"); //31
            break;
        case ERR_NETWORK_NOT_ALLOWED_EMERGENCY_CALLS_ONLY:
            strcpy(err_verbose, "network not allowed - emergency calls only"); //32
            break;
        case ERR_NETWORK_PERSONALISATION_PIN_REQUIRED:
            strcpy(err_verbose, "network personalisation PIN required"); //40
            break;
        case ERR_NETWORK_PERSONALISATION_PUK_REQUIRED:
            strcpy(err_verbose, "network personalisation PUK required"); //41
            break;
        case ERR_NETWORK_SUBSET_PERSONALISATION_PIN_REQUIRED:
            strcpy(err_verbose, "network subset personalisation PIN required"); //42
            break;
        case ERR_NETWORK_SUBSET_PERSONALISATION_PUK_REQUIRED:
            strcpy(err_verbose, "network subset personalisation PUK required"); //43
            break;
        case ERR_SERVICE_PROVIDER_PERSONALISATION_PIN_REQUIRED:
            strcpy(err_verbose, "service provider personalisation PIN required"); //44
            break;
        case ERR_SERVICE_PROVIDER_PERSONALISATION_PUK_REQUIRED:
            strcpy(err_verbose, "service provider personalisation PUK required"); //45
            break;
        case ERR_CORPORATE_PERSONALISATION_PIN_REQUIRED:
            strcpy(err_verbose, "corporate personalisation PIN required"); //46
            break;
        case ERR_CORPORATE_PERSONALISATION_PUK_REQUIRED:
            strcpy(err_verbose, "corporate personalisation PUK required"); //47
            break;
        case ERR_HIDDEN_KEY_REQUIRED:
            strcpy(err_verbose, "hidden key required"); //48
            break;
        case ERR_AT_COMMAND_DISCARDED:
            strcpy(err_verbose, "AT command discarded");//60
            break;
        case ERR_SIM_CARD_REJ_BY_NET:
            strcpy(err_verbose, "SIM card reject by network");//62
            break;
        case ERR_SIM_CARD_SERVICE_NOT_AVAILABLE:
            strcpy(err_verbose, "SIM card service not available");//63
            break;
        case ERR_SIM_CARD_PIN_UNINITIALIZED:
            strcpy(err_verbose, "SIM card PIN uninitialized");//64
            break;
        case ERR_SIM_CARD_PIN_BLOCKED:
            strcpy(err_verbose, "SIM card PIN blocked");//65
            break;
        case ERR_SIM_CARD_PUK_BLOCKED:
            strcpy(err_verbose, "SIM card PUK blocked");//66
            break;
        case ERR_SIM_FDN_FAILED:
            strcpy(err_verbose, "service is barred by fdn"); //70
            break;
        case ERR_UNKNOWN:
            strcpy(err_verbose, "unknown"); //numeric 100
            break;
        case ERR_ILLEGAL_MS:
            strcpy(err_verbose, "Illegal MS (#3)"); //numeric 103
            break;
        case ERR_ILLEGAL_ME:
            strcpy(err_verbose, "Illegal ME (#6)"); //numeric 106
            break;
        case ERR_GPRS_SERVICES_NOT_ALLOWED:
            strcpy(err_verbose, "GPRS services not allowed (#7)"); //numeric 107
            break;
        case ERR_PLMN_NOT_ALLOWED:
            strcpy(err_verbose, "PLMN not allowed (#11)"); //numeric 111
            break;
        case ERR_LOCATION_AREA_NOT_ALLOWED:
            strcpy(err_verbose, "Location area not allowed (#12)"); //numeric 112
            break;
        case ERR_ROAMING_NOT_ALLOWED_IN_THIS_LOCATION_AREA:
            strcpy(err_verbose, "Roaming not allowed in this location area (#13)"); //numeric 113
            break;
        case ERR_SERVICE_OPTION_NOT_SUPPORTED  :
            strcpy(err_verbose, "Service option not supported (#32)"); //132
            break;
        case ERR_REQUEST_SERVICE_OPTION_NOT_SUBSCRIBED :
            strcpy(err_verbose, "Request service option not subscribed (#33)"); //133
            break;
        case ERR_SERVICE_OPTION_OUTOF_ORDER:
            strcpy(err_verbose, "Service option temprarily out of order(#34)"); //134
            break;
        case ERR_UNSPECIFIED_GPRS_ERROR:
            strcpy(err_verbose, "unspecified GPRS error"); //148
            break;
        case ERR_PDP_AUTHENTICATION_FAILED:
            strcpy(err_verbose, "PDP authentication failure"); //149
            break;
        case ERR_INVALID_MOBILE_CLASS:
            strcpy(err_verbose, "invalid mobile class"); //150
            break;
        case ERR_LEX_LEN_ZERO:
            strcpy(err_verbose, "AT command line length is zero"); //200
            break;
        case ERR_LEX_NO_AT_PREX:
            strcpy(err_verbose, "no or wrong command prefix"); //201
            break;
        case ERR_LEX_INVALID_CTRL_Z_COMMAND:
            strcpy(err_verbose, "wrong ctrl_z command"); //202
            break;
        case ERR_MISSING_OR_UNKOWN_APN:
            strcpy(err_verbose, "Missing or unkown apn"); //533
            break;
    }
}

/******************************************************************************/
// Description : This function get the error string
// Global resource dependence : None
// Author : Ivan
// Note : None
/******************************************************************************/
void ATC_InitConfig(
    ATC_CONFIG_T    *atc_config_ptr
)
{
    uint32  i   = 0;
    FIXED_NVITEM_T  classmark = {0};
    
    SCI_ASSERT(PNULL != atc_config_ptr);/*assert verified: check null pointer*/

    SCI_MEMSET(atc_config_ptr, 0, sizeof(ATC_CONFIG_T));

    atc_config_ptr->s0_reg  = DEF_S0;
    atc_config_ptr->s2_reg  = DEF_S2;
    atc_config_ptr->s3_reg  = DEF_S3;
    atc_config_ptr->s4_reg  = DEF_S4;
    atc_config_ptr->s5_reg  = DEF_S5;
    atc_config_ptr->s6_reg  = DEF_S6;
    atc_config_ptr->s7_reg  = DEF_S7;
    atc_config_ptr->s8_reg  = DEF_S8;
    atc_config_ptr->s10_reg = DEF_S10;

    atc_config_ptr->cr      = DEF_CR;
    atc_config_ptr->crc     = DEF_CRC;

    atc_config_ptr->audio_flag  = FALSE;
    atc_config_ptr->cgreg       = DEF_CGREG;

    atc_config_ptr->l_value     = DEF_L;
    atc_config_ptr->m_value     = DEF_M;
    atc_config_ptr->p_value     = DEF_P;
    atc_config_ptr->q_value     = DEF_Q;
    atc_config_ptr->v_value     = DEF_V;
    atc_config_ptr->x_value     = DEF_X;
    atc_config_ptr->ath_flag    = FALSE;
    atc_config_ptr->cmut_flag   = FALSE;
    atc_config_ptr->sdmut_flag  = FALSE;
    atc_config_ptr->crsl_value  = DEF_CRSL_VALUE;
    atc_config_ptr->vtd_value   = 0;
    atc_config_ptr->cicb_val    = CICB_SPEECH_CALL;
    atc_config_ptr->cmod        = DEF_CMOD;
    atc_config_ptr->ceer_val    = 0;

    atc_config_ptr->cbst.speed  = DEF_CBST_SPEED;
    atc_config_ptr->cbst.name   = DEF_CBST_NAME;
    atc_config_ptr->cbst.ce     = DEF_CBST_CE;


    atc_config_ptr->dsci_flag   = FALSE;


    atc_config_ptr->cpas        = ATC_CPAS_STATUS_READY;

    atc_config_ptr->tz_report_flag = FALSE;
    atc_config_ptr->urc_switch = TRUE;
    atc_config_ptr->usb_flag = FALSE;

    for(i = 0; i < MN_SYS_NUMBER; i++)
    {
        atc_config_ptr->cpin[i] = ATC_CPIN_CODE_MAX; //ATC_CPIN_CODE_READY;
        atc_config_ptr->cpin2[i] = ATC_CPIN_CODE_MAX; //ATC_CPIN_CODE_READY;
        atc_config_ptr->cpin_flag[i] = FALSE;
        atc_config_ptr->pin1_enable_flag[i] = FALSE;
        atc_config_ptr->smscb_flag[i] = FALSE;
        atc_config_ptr->smscb_mode[i] = 0;
        atc_config_ptr->cfun_flag[i]  = FALSE;
        atc_config_ptr->cfun_status[i]  = ATC_FUN_DEACT_SIM_PS;
        atc_config_ptr->sim_flag[i] = FALSE;
        atc_config_ptr->sim_ready[i] = FALSE;
        atc_config_ptr->sim_err_reason[i] = MNSIM_ERROR_NULL;
        atc_config_ptr->sim_type[i] = MN_CARD_TYPE_NUM;
        atc_config_ptr->is_fdn_init[i] = TRUE;
        atc_config_ptr->is_fdn_enable[i] = FALSE;
        atc_config_ptr->is_fdn_ready[i] = FALSE;
        atc_config_ptr->call_read_flag[i] = FALSE;

        atc_config_ptr->ccwa_flag[i] = TRUE;
        atc_config_ptr->csq_flag[i] = FALSE;
        atc_config_ptr->cced_type[i] = ATC_CCED_NCELL;
        atc_config_ptr->cscs[i] = ATC_CHSET_IRA;

        atc_config_ptr->cops[i].mode   = (PLMN_SELECTION_MODE_E)DEF_COPS_MODE;
        atc_config_ptr->cops[i].format = (ATC_PLMN_FORMAT_E)DEF_COPS_FORMAT;
        atc_config_ptr->ss_operate[i]     = ATC_INVALID_OPERATE;
        atc_config_ptr->cmer_flag[i]      = FALSE;
        atc_config_ptr->rel_former[i]     = 127;
        atc_config_ptr->audio_rec = ATC_AUDIO_REC_STOP;

#ifdef _SUPPORT_CCBS_
        //ccbs function initialize
        atc_config_ptr->ccbs_info[i].cur_call_id = 0xff;
        atc_config_ptr->ccbs_info[i].cur_ccbs_index= 0xff;
#endif // #ifdef _SUPPORT_CCBS_
    }


    atc_config_ptr->aoc_mode            = 0;
    atc_config_ptr->ccm_val             = 0;

    atc_config_ptr->audio_mode          = AUDIO_DEVICE_MODE_HANDHOLD;
    atc_config_ptr->speaker_volume      = AT_AUD_VOL_DEFAULT_VAL;
    atc_config_ptr->ssea_vol_up         = AT_AUD_VOL_DEFAULT_VAL;
    atc_config_ptr->ssea_vol_down       = AT_AUD_VOL_DEFAULT_VAL;

#ifdef _MUX_ENABLE_
    for (i = 0; i < MN_CALL_MAX_CALL_NUM; i++)
    {
        atc_config_ptr->call_link_id[i]            = 0xFF;
    }
    atc_config_ptr->c_activate_pdp                  = 0;
    atc_config_ptr->current_link_id                 = ATC_INVALID_LINK_ID;
    atc_config_ptr->atc_exp_link_info.start_point   = 0;
    atc_config_ptr->atc_exp_link_info.end_point     = 0;

    for(i = 0; i < ATC_MAX_EXP_RESULT; i++)
    {
        ATC_Clear_Exp_Result(&atc_config_ptr->atc_exp_link_info.atc_exp_result_info[i]);
    }

    for(i = 0; i < ATC_MAX_LINK_ID; i++)
    {
        atc_config_ptr->atc_link_config[i].is_ctrlz_end = FALSE;
    }
#else
    atc_config_ptr->is_ctrlz_end  = FALSE;
#endif

    atc_config_ptr->trace_value = DEF_TRACE;

    for(i = 0; i < ATC_MAX_ACTIVATE_PDP; i++)
    {
        ATC_Clear_PSD_Call_Info(i + 1);
    }

    atc_config_ptr->atc_sync_srv.status = ATC_SYNCHRONIZESERVICE_STAT_FINISHED;

    atc_config_ptr->atc_sync_srv.cfu    = AT_STATUS_NOT_ACTIVE;
    atc_config_ptr->atc_sync_srv.cfu    = AT_STATUS_NOT_ACTIVE;
    atc_config_ptr->atc_sync_srv.cfb    = AT_STATUS_NOT_ACTIVE;
    atc_config_ptr->atc_sync_srv.cfnry  = AT_STATUS_NOT_ACTIVE;
    atc_config_ptr->atc_sync_srv.cfnrc  = AT_STATUS_NOT_ACTIVE;
    atc_config_ptr->atc_sync_srv.cfcond = AT_STATUS_NOT_ACTIVE;
    atc_config_ptr->atc_sync_srv.cfall  = AT_STATUS_NOT_ACTIVE;

    atc_config_ptr->cgerep_value.mode   = MN_GPRS_EVENT_REPORT_MODE_2;
    atc_config_ptr->cgerep_value.bfr    = MN_GPRS_EVENT_REPORT_BUFFER_1;

    atc_config_ptr->esqopt_param.option         = ATC_ESQOPT_STOP_INDICATION;
    atc_config_ptr->esqopt_param.timer_value    = ATC_ESQOPT_TIMER_VALUE;
    atc_config_ptr->esqopt_param.thres_value    = 0;
    atc_config_ptr->esqopt_timer  = SCI_CreateTimer("ATC:create +ESQOPT Timer", ATC_ProcessEsqoptTimeout, 0,
                                                  atc_config_ptr->esqopt_param.timer_value * ATC_ESQOPT_TIMER_UNIT,
                                                  SCI_NO_ACTIVATE);

    atc_config_ptr->ccwe_flag           = FALSE;
    atc_config_ptr->async_mode_flag     = FALSE;
#ifndef ATC_SAT_ENABLE
    atc_config_ptr->psntrg_flag         = FALSE;
    atc_config_ptr->spreadyflag         = 0;
#endif    

    for(i = 0; i < CEER_MAX_NUM; i++)
    {
        atc_config_ptr->ceer_code[i]    = 0xFFFF;
    }

    atc_config_ptr->ceer_last_type      = 0xFF;

    T_ATC_CALL_EXP_IND                  = PNULL;
    T_ATC_POWEROFF_EXP_IND              = PNULL;


    //Read all the item which store in the NV.
    ATC_ReadNVItem(atc_config_ptr);

    //edge support setting for phone
    if (NVERR_NONE == EFS_NvitemRead(NV_CLASSMARK, sizeof(FIXED_NVITEM_T), (uint8 *)(&classmark)))
    {
        atc_config_ptr->edge_support = classmark.egprs_supported;
    }
    else
    {
        atc_config_ptr->edge_support = FALSE;
    }

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

    for(i=0; i<MN_SYS_NUMBER; i++)
    {
        g_atc_ps_not_save_sms[i] = 0;
#ifndef ATC_SAT_ENABLE
        ATC_InitUplmnCfgInfo(i);
#endif
    }
    
#ifndef ATC_SAT_ENABLE
    //Initialise the variable of STK
    ATC_InitStkVar(atc_config_ptr);
#endif

    //Initialise the variable of SMS
    ATC_InitSmsConfig(atc_config_ptr);

    //Initialise the variable of Phonebook
    ATC_InitPhonebookStaticVar();

    //Initialise the variable of Basic AT command
    ATC_InitBasicVar();

    //Initialise the variable of SS
    ATC_InitSsVar();
    
#ifndef ATC_SAT_ENABLE    
    ATC_InitEngVar();


    //Initialise the parameters of cell broadcast
    ATC_InitSmsCellBroadcastPara(atc_config_ptr, TRUE, MN_SYS_NUMBER);
#endif
}

/******************************************************************************/
// Description : register the event to MN
// Global resource dependence : None
// Author : Ivan
// Note : None
/******************************************************************************/
LOCAL void ATC_RegisterEvent(void)
{
    POWER_RESTART_CONDITION_E      start_condition = POWER_GetRestartCondition();

    if ((RESTART_BY_ALARM != start_condition &&RESTART_BY_CHARGE != start_condition)
        || 1 == CLASSMARK_GetModelType() )
    {
        //Register phone event
        SCI_RegisterMsg( MN_APP_PHONE_SERVICE, 
                                (uint8)EV_MN_APP_NETWORK_STATUS_IND_F, 
                                (uint8)(MAX_MN_APP_PHONE_EVENTS_NUM -1), 
                                SCI_NULL );

        //Register STK evet
#ifdef  _ATC_ONLY
        SCI_RegisterMsg(MN_APP_SIMAT_SERVICE, 
                                (uint8)EV_MN_APP_SIMAT_DISPLAY_TEXT_IND_F, 
                                (uint8)(SIMAT_EVENT_NUM - 1), 
                                SIMAT_CallBack );
                                SCI_TRACE_LOW("ATC:RegisterMsg STK");
#endif

#ifdef _SUPPORT_GPRS_
        SCI_RegisterMsg(MN_APP_GPRS_SERVICE,
                                (uint8)EV_MN_APP_SET_PDP_CONTEXT_CNF_F,
                                (uint8)(MAX_MN_APP_GPRS_EVENTS_NUM - 1),
                                SCI_NULL); 
#endif

#ifndef _ULTRA_LOW_CODE_
        SCI_RegisterMsg( MN_APP_SMS_SERVICE, 
                                (uint8)EV_MN_APP_SMS_READY_IND_F, 
                                (uint8)(MAX_MN_APP_SMS_EVENTS_NUM - 1), 
                                SCI_NULL ); 
#endif
        SCI_RegisterMsg(MN_APP_CALL_SERVICE, 
        (uint8)EV_MN_APP_CALL_START_IND_F, 
        (uint8)(MAX_MN_APP_CALL_EVENTS_NUM - 1), 
        SCI_NULL);

#ifndef _ULTRA_LOW_CODE_
        SCI_RegisterMsg( MN_APP_SS_SERVICE, 
                                (uint8)EV_MN_APP_BEGIN_REGISTER_SS_F, 
                                (uint8)(MAX_MN_APP_SS_EVENTS_NUM - 1), 
                                SCI_NULL); 

        SCI_RegisterMsg( MN_APP_SMSCB_SERVICE, 
                                (uint8)EV_MN_APP_SMSCB_MSG_IND_F, 
                                (uint8)(MAX_MN_APP_SMSCB_EVENTS_NUM - 1), 
                                SCI_NULL );
        
        //register SIM server.
        SCI_RegisterMsg( MN_APP_SIM_SERVICE, 
                                (uint8)EV_MN_APP_MSISDN_UPDATE_CNF_F, 
                                (uint8)(MAX_MN_APP_SIM_EVENTS_NUM - 1), 
                                SCI_NULL );
#endif
    }
#ifndef _ULTRA_LOW_CODE_
#ifndef _ATC_ONLY
#ifndef ATC_SAT_ENABLE
    SCI_RegisterMsg(CHR_SERVICE, 
                            (uint8)CHR_CAP_IND, 
                            (uint8)(CHR_MSG_MAX_NUM - 1), 
                            ATC_ChrCallBack );
#endif /*ATC_SAT_ENABLE*/
#endif
#endif
}

/******************************************************************************/
// Description : identify if the message is the needed msg on pending state
// Global resource dependence : None
// Author : Ivan.Yin
// Note : None
/******************************************************************************/
ATC_STATUS ATC_IdentifyMsg( // the identify result
    int msg_type, // received the msg type
    ATC_GLOBAL_INFO_T *atc_global_info_ptr  // global info
)
{
    switch(msg_type)
    {
        case APP_MN_ACTIVATE_PDP_CONTEXT_CNF:
            return(strcmp((const char *)atc_global_info_ptr->atc_info_table->cmd_name,
                          "+CGACT") ? S_ATC_SUCCESS : S_ATC_FAIL);
        default:
            SCI_ASSERT(FALSE);/*assert verified: check valid value*/
            break;
    }

    return S_ATC_SUCCESS; //lint !e527
}


#ifdef _MUX_ENABLE_
/******************************************************************************/
// Description : clear atc state after ppp modem disconnect
// Global resource dependence : None
// Author : hyman
// Note : None
/******************************************************************************/
void ATC_DisconnectModem(uint8 cid)
{
#ifdef _SUPPORT_GPRS_
    ATC_CONFIG_T *atc_config_ptr = &s_config_info;
    uint32 pdp_id[MN_GPRS_MAX_PDP_CONTEXT_COUNT] = {0};
    MN_DUAL_SYS_E dual_sys = ATC_CURRENT_SIM_ID_MUX;
    uint8 link_id;

    ATC_TRACE_LOW("ATC: ATC_DisconnectModem ");
    pdp_id[0] = (uint32)cid;

    if((cid < 1) || (cid > MN_GPRS_MAX_PDP_CONTEXT_COUNT))
    {
        ATC_TRACE_LOW("ATC: ATC_DisconnectModem, illegal cid:%d, not in range", cid);
        return;
    }

    if ((ATC_PSD_STATE_INACTIVE == atc_config_ptr->atc_psd_call_info[cid-1].state)
        ||(ATC_PSD_STATE_DEACTIVATE_PENDING == atc_config_ptr->atc_psd_call_info[cid-1].state))
    {
        ATC_TRACE_LOW("ATC: ATC_DisconnectModem, cid %d is inactive or deactivating!", cid);
        return;
    }
    else
    {
        ATC_Set_PSD_Call_State(cid, ATC_PSD_STATE_DEACTIVATE_PENDING);
    }


    link_id = ATC_Get_Link_Id(cid, ATC_DOMAIN_PS, dual_sys);

    if(ERR_MNGPRS_NO_ERR == MNGPRS_DeactivatePdpContextEx(g_atc_modem_dial_sys, FALSE, pdp_id))
    {
        if(link_id != (uint8)ATC_INVALID_LINK_ID)
        {
            /* Protect incoming ATH from this link id
             * This link-id enters PROTECTED mode. */
            ATC_Link_ATHEventHandle(link_id, ATC_ATH_EV_PDP_DEACT, cid);

            /*This Gprs deactive process is triggered by TCP/IP,not a command, 
            so unsolicited result code is not necessary.*/
            
            /*ATC_Set_Current_Link_id(&s_config_info, link_id);
            ATC_Add_Expected_Event(&s_config_info, APP_MN_DEACTIVATE_PDP_CONTEXT_CNF,
                    pdp_id[0], ATC_DOMAIN_PS, dual_sys);
            */
        }
        else
        {
            ATC_TRACE_LOW("ATC: ATC_DisconnectModem, no call(cid:%d) link id! PSD is not existed!", cid);
            ATC_List_PSD_Call_Info(cid);
        }
    }
    else
    {
        ATC_TRACE_LOW("ATC: ATC_DisconnectModem, failure in MNGPRS_DeactivatePdpContext");
    }
#ifndef ATC_SAT_ENABLE
    AtcReStartSleepTimer(ATC_SLEEP_TIMER_VALUE);
#endif

#endif /* _SUPPORT_GPRS_ */
}
#else
/******************************************************************************/
// Description : clear atc state after ppp modem disconnect
// Global resource dependence : None
// Author : hyman
// Note : None
/******************************************************************************/
void ATC_DisconnectModem(void)
{
#ifdef _SUPPORT_GPRS_
    uint8 i;
    uint32 pdp_id[MN_GPRS_MAX_PDP_CONTEXT_COUNT] = {0};

    ATC_TRACE_LOW("ATC: ATC_DisconnectModem ");

    for(i = 0; i < MN_GPRS_MAX_PDP_CONTEXT_COUNT; i++)
    {
        if(ATC_PSD_STATE_INACTIVE != s_atc_global_info.atc_config_ptr->atc_psd_call_info[i].state)
        {
            ATC_Set_PSD_Call_State(i + 1, ATC_PSD_STATE_DEACTIVATE_PENDING);
        }
    }

    //deactive pdp context
    MNGPRS_DeactivatePdpContextEx(ATC_CURRENT_SIM_ID_MUX, TRUE, pdp_id);
    //SIO return to AT mode
    SIO_ATC_SetDataMode(FALSE);
    
#ifndef ATC_SAT_ENABLE
    AtcReStartSleepTimer(ATC_SLEEP_TIMER_VALUE);
#endif
    //ATC_TRACE_LOW("ATC:ATC_DisconnectModem ");

    /* Protect incoming ATH later,
     * link_id use 0 for non-mux ATC version. */
    ATC_Link_ATHEventHandle(0, ATC_ATH_EV_PDP_DEACT, ATC_INVALID_CID);

#endif /* _SUPPORT_GPRS_ */
}
#endif

#ifndef _MODEM_MODE //@brand.cheng
/*****************************************************************************/
//  Description : This function is the callback of CHR
//  Global resource dependence :
//  Author:       Xueli
//  Note:
/*****************************************************************************/
LOCAL void ATC_ChrCallBack(uint32 taskID, uint32 argc, void *argv)
{
#ifndef ATC_SAT_ENABLE
    uint16                signal_size;
    uint16                signal_code;
    xSignalHeaderRec      *signal_ptr = PNULL;

    switch(argc)
    {
        case CHR_CAP_IND:
            signal_code = ATC_CHR_CAP_IND;
            signal_size = sizeof(xSignalHeaderRec);
            break;
        case CHR_CHARGE_START_IND:
            signal_code = ATC_CHR_CHARGE_START_IND;
            signal_size = sizeof(xSignalHeaderRec);
            break;
        case CHR_CHARGE_END_IND:
            signal_code = ATC_CHR_CHARGE_END_IND;
            signal_size = sizeof(xSignalHeaderRec);
            break;
        case CHR_WARNING_IND:
            signal_code = ATC_CHR_WARNING_IND;
            signal_size = sizeof(xSignalHeaderRec);
            break;
        case CHR_SHUTDOWN_IND:
            signal_code = ATC_CHR_SHUTDOWN_IND;
            signal_size = sizeof(xSignalHeaderRec);
            break;
        case CHR_CHARGE_FINISH:
            signal_code = ATC_CHR_FINISH_IND;
            signal_size = sizeof(xSignalHeaderRec);
            break;
        case CHR_CHARGE_DISCONNECT:
            signal_code = ATC_CHR_DISC_IND;
            signal_size = sizeof(xSignalHeaderRec);
            break;
        case CHR_CHARGE_FAULT:
            signal_code = ATC_CHR_FAULT_IND;
            signal_size = sizeof(xSignalHeaderRec);
            break;

        default:
            ATC_TRACE_LOW("ATC: Assert Failure unknow chr signal.");
            return;
    }

    // create a signal
    SCI_CREATE_SIGNAL(
        signal_ptr,
        signal_code,
        signal_size,
        SCI_IDENTIFY_THREAD())
    // Send the signal to the dedicated task's queue
    SCI_SEND_SIGNAL((xSignalHeaderRec *)signal_ptr, taskID);
#endif /*ATC_SAT_ENABLE*/    
}
#endif //@brand.cheng

#ifndef ATC_SAT_ENABLE
/**********************************************************************
//    Description:
//      the funciton to enable sleep flag after timer expire
//    Global resource dependence :
//    Author:bo.chen
//    Note:
***********************************************************************/
static void AtcSleepTimerExpired(
    uint32 lparam   //the function's input parameter
)
{
#ifdef _MODEM_MODE

    if(ATC_IsPsActivate())
    {
        SCI_ATC_EnableDeepSleep(0);
    }
    else
    {
        SCI_ATC_EnableDeepSleep(1);
    }

#endif
}

/**********************************************************************
//    Description:
//      the funciton to restart atc sleep timer
//    Global resource dependence :
//    Author:bo.chen
//    Note:
***********************************************************************/
static void AtcReStartSleepTimer(
    uint32  timer_value
)
{
    SCI_ASSERT(0 < timer_value);/*assert verified: check valid value*/

    if(SCI_IsTimerActive(s_atc_sleep_timer))
    {
        SCI_DeactiveTimer(s_atc_sleep_timer);
    }

    SCI_ChangeTimer(s_atc_sleep_timer, AtcSleepTimerExpired, timer_value);
    SCI_ActiveTimer(s_atc_sleep_timer);
}
#endif

/**********************************************************************
//    Description:
//      the function to protect ATH when ps disconnecting after timer expire
//    Global resource dependence :
//    Author: Pizer.Fan
//    Note:
***********************************************************************/
LOCAL void ATC_Link_ATHWaitTimerTimeout(
    uint32 lparam   //the function's input parameter
)
{
    uint8 link_id;
    /* Get link id of timer expired. */
    link_id = (uint8)lparam;
    ATC_Link_ATHEventHandle(link_id, ATC_ATH_EV_WAIT_TIMEOUT, 0);
}

/**********************************************************************
//    Description:
//      the funciton to protect ATH when ps disconnecting after timer expire
//    Global resource dependence :
//    Author: Pizer.Fan
//    Note:
***********************************************************************/
LOCAL void ATC_Link_ATHAvoidRepTimeout(
    uint32 lparam   //the function's input parameter
)
{
    uint8 link_id;

    /* Get link id of timer expired. */
    link_id = (uint8)lparam;
    ATC_Link_ATHEventHandle(link_id, ATC_ATH_EV_AVOID_REP_TIMEOUT, 0);
}

/**********************************************************************
//    Description:
//      the funciton to handle PS PDP ATH related event.
//    Global resource dependence :
//    Author: Pizer.Fan
//    Note:
***********************************************************************/
BOOLEAN ATC_Link_ATHEventHandle(uint8 link_id, ATC_ATH_EVENT_E ath_ev, uint8 cid)
{
#ifdef _MUX_ENABLE_
    ATC_CONFIG_T *atc_config_ptr = &s_config_info;
#endif

    /* Check parameters */
    if(link_id >= ATC_MAX_LINK_ID)
    {
        ATC_TRACE_LOW("ATC: ATHEventHandle,ev:%d,ERROR!!! link_id:%d not in range!", ath_ev, link_id);
        return FALSE;
    }

    /* Reset IP filter */
    if(atc_link_inst[link_id].ath_stat != ATC_ATH_WAITING)
    {
        atc_err_protected_flag = FALSE;
    }

    switch(ath_ev)
    {
        case ATC_ATH_EV_NEW_PDP_SRV:
        case ATC_ATH_EV_NEW_PPP_SRV:
            link_id = ATC_Get_Data_Link_Id(link_id, cid);

            if(atc_link_inst[link_id].ath_stat == ATC_ATH_LISTENING)
            {
                /* ERROR: PDP is activating or active */
                ATC_TRACE_LOW("ATC: ATHEventHandle,NEW_PDP(cid:%d),ERROR:link_id:%d,cid:%d,ath_stat:(!)LISTENING",
                              cid, link_id, atc_link_inst[link_id].ath_cid);
            }
            else if(atc_link_inst[link_id].ath_stat == ATC_ATH_WAITING)
            {
                /* Reset orignal link status */
                ATC_TRACE_LOW("ATC: ATHEventHandle,NEW_PDP(cid:%d),Reset,link_id:%d,cid:%d->INVALID,ath_stat:WAITING->PNULL",
                              cid, link_id, atc_link_inst[link_id].ath_cid);

                /* Stop timer */
                SCI_DeactiveTimer(atc_link_inst[link_id].ath_wait_timer);
                atc_link_inst[link_id].ath_stat    = ATC_ATH_NULL;
                atc_link_inst[link_id].ath_cid     = ATC_INVALID_CID;
            }
            else if(atc_link_inst[link_id].ath_stat == ATC_ATH_AVOID_REPETITION)
            {
                /* Reset orignal link status */
                ATC_TRACE_LOW("ATC: ATHEventHandle,NEW_PDP(cid:%d),Reset,link_id:%d,cid:%d->INVALID,ath_stat:AVOID_REPETITION->PNULL",
                              cid, link_id, atc_link_inst[link_id].ath_cid);

                /* Stop timer */
                SCI_DeactiveTimer(atc_link_inst[link_id].ath_avoid_rep_timer);
                atc_link_inst[link_id].ath_stat    = ATC_ATH_NULL;
                atc_link_inst[link_id].ath_cid     = ATC_INVALID_CID;
            }

            if((atc_link_inst[link_id].ath_stat == ATC_ATH_NULL)
                    && (ATC_ATH_EV_NEW_PPP_SRV == ath_ev))
            {
                /* Reset orignal link status */
                ATC_TRACE_LOW("ATC: ATHEventHandle,NEW_PPP(cid:%d),link_id:%d,cid:%d->%d,ath_stat:PNULL->LISTENING",
                              cid, link_id, atc_link_inst[link_id].ath_cid, cid);

                /* Change state to ATC_ATH_LISTENING */
                atc_link_inst[link_id].ath_stat    = ATC_ATH_LISTENING;
                atc_link_inst[link_id].ath_cid     = cid;
                return TRUE;
            }

            break;

        case ATC_ATH_EV_NEW_CS_SRV:
#ifndef ATC_SAT_ENABLE
            if(atc_link_inst[link_id].ath_stat == ATC_ATH_LISTENING)
            {
                /* ERROR: PDP is activating or active */
                ATC_TRACE_LOW("ATC: ATHEventHandle,NEW_CS,ERROR:link_id:%d,cid:%d,ath_stat:%d==LISTENING",
                              cid, link_id, atc_link_inst[link_id].ath_cid, atc_link_inst[link_id].ath_stat);
            }
            else if(atc_link_inst[link_id].ath_stat == ATC_ATH_WAITING)
            {
                /* Reset orignal link status */
                ATC_TRACE_LOW("ATC: ATHEventHandle,NEW_CS,link_id:%d,cid:%d,ath_stat:%d",
                              link_id, atc_link_inst[link_id].ath_cid, atc_link_inst[link_id].ath_stat);

                /* Stop timer */
                SCI_DeactiveTimer(atc_link_inst[link_id].ath_wait_timer);
                atc_link_inst[link_id].ath_stat    = ATC_ATH_NULL;
                atc_link_inst[link_id].ath_cid     = ATC_INVALID_CID;

                return TRUE;
            }
            else if(atc_link_inst[link_id].ath_stat == ATC_ATH_AVOID_REPETITION)
            {
                /* Reset orignal link status */
                ATC_TRACE_LOW("ATC: ATHEventHandle,NEW_CS,link_id:%d,cid:%d,ath_stat:%d",
                              link_id, atc_link_inst[link_id].ath_cid, atc_link_inst[link_id].ath_stat);

                /* Stop timer */
                SCI_DeactiveTimer(atc_link_inst[link_id].ath_avoid_rep_timer);
                atc_link_inst[link_id].ath_stat    = ATC_ATH_NULL;
                atc_link_inst[link_id].ath_cid     = ATC_INVALID_CID;

                return TRUE;
            }
#endif
            break;

        case ATC_ATH_EV_RECV_ATH:

            if(atc_link_inst[link_id].ath_stat == ATC_ATH_LISTENING)
            {
                /* NOTE: PDP is activating or active */
                ATC_TRACE_LOW("ATC: ATHEventHandle,RECV_ATH,link_id:%d,cid:%d,ath_stat:LISTENING->AVOID_REPETITION",
                              link_id, atc_link_inst[link_id].ath_cid);

                /* Skip INPUT cid parameter!!!
                 * WE need get cid from link instance!!! */
                cid = atc_link_inst[link_id].ath_cid;

                if(((cid >= 1) && (cid <= MN_GPRS_MAX_PDP_CONTEXT_COUNT))
                        && (ATC_PSD_STATE_INACTIVE != s_atc_global_info.atc_config_ptr->atc_psd_call_info[cid-1].state)
                        && (ATC_PSD_STATE_DEACTIVATE_PENDING != s_atc_global_info.atc_config_ptr->atc_psd_call_info[cid-1].state))
                {
#ifdef _MUX_ENABLE_
                    uint16 start_p, end_p;
                    BOOLEAN  is_exp_event_present = FALSE;
#endif
                    MN_DUAL_SYS_E dual_sys = ATC_CURRENT_SIM_ID_MUX;
                    uint32 cid_list[MN_GPRS_MAX_PDP_CONTEXT_COUNT] = {0};

                    /* Deactivate PDP context */
                    cid_list[0] = atc_link_inst[link_id].ath_cid;
                    cid_list[1] = MN_GPRS_PDP_ID_INVALID;
                    MNGPRS_DeactivatePdpContextEx(dual_sys, FALSE, cid_list);
                    ATC_Set_PSD_Call_State(cid, ATC_PSD_STATE_DEACTIVATE_PENDING);

#ifdef _MUX_ENABLE_
                    /* Change Expected Event:  APP_MN_ACTIVATE_PDP_CONTEXT_CNF->APP_MN_DEACTIVATE_PDP_CONTEXT_CNF */
                    start_p = atc_config_ptr->atc_exp_link_info.start_point;
                    end_p  = atc_config_ptr->atc_exp_link_info.end_point;

                    while(start_p != end_p)
                    {
                        if((link_id == atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].link_id)
                                && (APP_MN_ACTIVATE_PDP_CONTEXT_CNF == atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].exp_event)
                                && (atc_link_inst[link_id].ath_cid == atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].cid))
                        {
                            /* Change Expected Event */
                            ATC_TRACE_LOW("ATC: ATHEventHandle,RECV_ATH,link_id:%d,change [%d] expected event: ACT->DEACT", start_p);

                            atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].exp_event = APP_MN_DEACTIVATE_PDP_CONTEXT_CNF;
                            ATC_List_Expected_Event();
                            is_exp_event_present = TRUE;
                            break;
                        }

                        start_p++;

                        if(ATC_MAX_EXP_RESULT == start_p)
                        {
                            start_p = 0;
                        }
                    }

                    if(!is_exp_event_present)
                    {
                        ATC_TRACE_LOW("ATC: ATHEventHandle,RECV_ATH,add deact expected event. cid:%d", atc_link_inst[link_id].ath_cid);
                        ATC_Add_Expected_Event(atc_config_ptr, APP_MN_DEACTIVATE_PDP_CONTEXT_CNF,
                                atc_link_inst[link_id].ath_cid, ATC_DOMAIN_PS, dual_sys);
                    }

#endif
                }

                /* Change state to ATC_ATH_AVOID_REPETITION */
                atc_link_inst[link_id].ath_stat = ATC_ATH_AVOID_REPETITION;
                SCI_ChangeTimer(atc_link_inst[link_id].ath_avoid_rep_timer,
                                ATC_Link_ATHAvoidRepTimeout, ATC_PS_ATH_AVOID_REP_TIMER_VALUE);
                SCI_ActiveTimer(atc_link_inst[link_id].ath_avoid_rep_timer);

                return TRUE;
            }
            else if(atc_link_inst[link_id].ath_stat == ATC_ATH_WAITING)
            {
                ATC_TRACE_LOW("ATC: ATHEventHandle,RECV_ATH,link_id:%d,cid:%d,ath_stat:WAITING->AVOID_REPETITION",
                              link_id, atc_link_inst[link_id].ath_cid);

                /* Stop timer */
                SCI_DeactiveTimer(atc_link_inst[link_id].ath_wait_timer);

                /* Change state to ATC_ATH_AVOID_REPETITION */
                atc_link_inst[link_id].ath_stat = ATC_ATH_AVOID_REPETITION;
                SCI_ChangeTimer(atc_link_inst[link_id].ath_avoid_rep_timer,
                                ATC_Link_ATHAvoidRepTimeout, ATC_PS_ATH_AVOID_REP_TIMER_VALUE);
                SCI_ActiveTimer(atc_link_inst[link_id].ath_avoid_rep_timer);

                return TRUE;
            }
            else if(atc_link_inst[link_id].ath_stat == ATC_ATH_AVOID_REPETITION)
            {
                ATC_TRACE_LOW("ATC: ATHEventHandle,RECV_ATH,link_id:%d,cid:%d,ath_stat:AVOID_REPETITION",
                              link_id, atc_link_inst[link_id].ath_cid);
                return TRUE;
            }

            break;

        case ATC_ATH_EV_PDP_ACT_REJ:
            link_id = ATC_Get_Data_Link_Id(link_id, cid);

            if(ATC_ATH_NULL != atc_link_inst[link_id].ath_stat)
            {
                if((atc_link_inst[link_id].ath_cid == cid)
                        && (atc_link_inst[link_id].ath_stat == ATC_ATH_LISTENING))
                {
                    ATC_TRACE_LOW("ATC: ATHEventHandle,ACT_REJ,link_id:%d,cid:%d,ath_stat:LISTENING->WAITING",
                                  link_id, atc_link_inst[link_id].ath_cid);

                    /* Change state to ATC_ATH_WAITING */
                    atc_link_inst[link_id].ath_stat = ATC_ATH_WAITING;
                    SCI_ChangeTimer(atc_link_inst[link_id].ath_wait_timer,
                                    ATC_Link_ATHWaitTimerTimeout, ATC_PS_ATH_TIMER_VALUE);
                    SCI_ActiveTimer(atc_link_inst[link_id].ath_wait_timer);

                    /* Set IP filter */
                    atc_err_protected_flag = TRUE;
                }
                else
                {
                    /* Error */
                    ATC_TRACE_LOW("ATC: ATHEventHandle,ACT_REJ,ERROR:link_id:%d,cid:%d,%d,ath_stat:%d",
                                  link_id, atc_link_inst[link_id].ath_cid, cid, atc_link_inst[link_id].ath_stat);
                }

                return TRUE;
            }

            break;

            /* 1. ATC_DisconnectModem
             * 2. NW deactivation-ind
             * 3. PDP deactivation confirm (other link) */
        case ATC_ATH_EV_PDP_DEACT:
            link_id = ATC_Get_Data_Link_Id(link_id, cid);

            if(ATC_ATH_NULL != atc_link_inst[link_id].ath_stat)
            {
                if((atc_link_inst[link_id].ath_cid == cid)
                        && (atc_link_inst[link_id].ath_stat == ATC_ATH_LISTENING))
                {
                    ATC_TRACE_LOW("ATC: ATHEventHandle,DEACT,link_id:%d,cid:%d,ath_stat:LISTENING->WAITING",
                                  link_id, atc_link_inst[link_id].ath_cid);

                    /* Change state to ATC_ATH_WAITING */
                    atc_link_inst[link_id].ath_stat = ATC_ATH_WAITING;
                    SCI_ChangeTimer(atc_link_inst[link_id].ath_wait_timer,
                                    ATC_Link_ATHWaitTimerTimeout, ATC_PS_ATH_TIMER_VALUE);
                    SCI_ActiveTimer(atc_link_inst[link_id].ath_wait_timer);

                    /* Set IP filter */
                    atc_err_protected_flag = TRUE;
                }
                else if((atc_link_inst[link_id].ath_cid == cid)
                        && (atc_link_inst[link_id].ath_stat == ATC_ATH_WAITING))
                {
                    /*ATC_TRACE_LOW("ATC: ATHEventHandle,DEACT,link_id:%d,cid:%d,ath_stat:WAITING->WAITING",
                        link_id, atc_link_inst[link_id].ath_cid);*/
                }
                else if((atc_link_inst[link_id].ath_cid == cid)
                        && (atc_link_inst[link_id].ath_stat == ATC_ATH_AVOID_REPETITION))
                {
                    /*ATC_TRACE_LOW("ATC: ATHEventHandle,DEACT,link_id:%d,cid:%d,ath_stat:AVOID_REPETITION->AVOID_REPETITION",
                        link_id, atc_link_inst[link_id].ath_cid);*/
                }
                else
                {
                    /* Error */
                    ATC_TRACE_LOW("ATC: ATHEventHandle,DEACT,ERROR:link_id:%d,cid:%d,%d,ath_stat:%d",
                                  link_id, atc_link_inst[link_id].ath_cid, cid, atc_link_inst[link_id].ath_stat);
                }

                return TRUE;
            }

            break;

        case ATC_ATH_EV_WAIT_TIMEOUT:

            if(atc_link_inst[link_id].ath_stat == ATC_ATH_WAITING)
            {
                ATC_TRACE_LOW("ATC: ATHEventHandle,WAIT_TIMEOUT,link_id:%d,cid:%d,ath_stat:WAITING->PNULL",
                              link_id, atc_link_inst[link_id].ath_cid);
                atc_link_inst[link_id].ath_stat    = ATC_ATH_NULL;
                atc_link_inst[link_id].ath_cid     = ATC_INVALID_CID;
                return TRUE;
            }

            break;

        case ATC_ATH_EV_AVOID_REP_TIMEOUT:

            if(atc_link_inst[link_id].ath_stat == ATC_ATH_AVOID_REPETITION)
            {
                ATC_TRACE_LOW("ATC: ATHEventHandle,AVOID_REP_TIMEOUT,link_id:%d,cid:%d,ath_stat:AVOID_REPETITION->PNULL",
                              link_id, atc_link_inst[link_id].ath_cid);
                atc_link_inst[link_id].ath_stat    = ATC_ATH_NULL;
                atc_link_inst[link_id].ath_cid     = ATC_INVALID_CID;
                return TRUE;
            }

            break;

        default:
            ATC_TRACE_LOW("ATC: ATHEventHandle,unknown event:%d,link_id:%d", ath_ev, link_id);
            break;
    }

    return FALSE;
}
/**********************************************************************
//    Description:
//      the funciton to get LINK property
//    Global resource dependence :
//    Author: Pizer.Fan
//    Note:
***********************************************************************/
const ATC_LINK_INST_T *ATC_Link_Property(uint8 link_id)
{
    /* Check parameters */
    if(link_id >= ATC_MAX_LINK_ID)
    {
        ATC_TRACE_LOW("ATC: ATC_Link_Property invalid link id: %d.", link_id);
        return PNULL;
    }

    return (const ATC_LINK_INST_T *)&atc_link_inst[link_id];
}

#ifndef ATC_SAT_ENABLE
/**********************************************************************
//    Description:
//      the funciton to judge if there is data PSD on the specific link or not
//    Global resource dependence :
//    Author: Pizer.Fan
//    Note:
***********************************************************************/
BOOLEAN ATC_Link_Is_Existed_Data_PSD(uint8 link_id)
{
    ATC_CONFIG_T *atc_config_ptr;
    uint8         cid;

    /* Check parameters */
    if(link_id >= ATC_MAX_LINK_ID)
    {
        ATC_TRACE_LOW("ATC: ATC_Link_Is_Existed_PSD invalid link id: %d.", link_id);
        return FALSE;
    }

    atc_config_ptr = &s_config_info;

    for(cid = 1; cid <= MN_GPRS_MAX_PDP_CONTEXT_COUNT; cid++)
    {
        if((atc_config_ptr->atc_psd_call_info[cid - 1].state != ATC_PSD_STATE_INACTIVE) &&
                (atc_config_ptr->atc_psd_call_info[cid - 1].cid != ATC_INVALID_CID) &&
                (atc_config_ptr->atc_psd_call_info[cid - 1].data_mode != FALSE) &&
                (atc_config_ptr->atc_psd_call_info[cid - 1].link_id == link_id))
        {
            return TRUE;
        }
    }

    return FALSE;
}
#endif

/*****************************************************************************/
//  Description : This function get call state use in CLCC.
//  Global resource dependence : none
//  Author:       William
//  Note:
/*****************************************************************************/
uint8 ATC_GetCallState(                 // return call state
    MN_DUAL_SYS_E dual_sys,
    uint8   call_id     // in call_id
)
{
    uint8   call_state = 0xFF;

    switch(g_calls_state[dual_sys].call_context[call_id].call_entity_state)
    {
        case ATC_CALL_ACTIVE_STATE:

            if(ATC_HOLD_IDLE == g_calls_state[dual_sys].call_context[call_id].hold_state)
            {
                call_state = ATC_CALL_IN_ACTIVE;
            }
            else
            {
                call_state = ATC_CALL_IN_HELD;
            }

            break;

        case ATC_CALL_OUTGOING_STATE:
            //call_state = ATC_CALL_IN_ALERTNG;
            call_state = ATC_CALL_IN_DIALING;
            break;

        case ATC_CALL_ALERTING_STATE:
            call_state = ATC_CALL_IN_ALERTNG;
            break;

        case ATC_CALL_INCOMING_STATE:

            if(1 == g_calls_state[dual_sys].call_amount)
            {
                call_state = ATC_CALL_IN_INCOMING;
            }
            else
            {
                call_state = ATC_CALL_IN_WAITING;
            }

            break;

        default:
            ATC_TRACE_LOW("ATC: get call state error");
            break;
    }

    return call_state;
}

/*****************************************************************************/
//  Description : This function synchronize services
//  Global resource dependence : none
//  Author:       Pizer.Fan
//  Note:
/*****************************************************************************/
BOOLEAN ATC_SynchronizeService(MN_DUAL_SYS_E  dual_sys)
{
    ATC_SYNCHRONIZESERVICE_CNTXT_T *atc_sync_srv = &s_config_info.atc_sync_srv;

    switch(atc_sync_srv->status)
    {
        case ATC_SYNCHRONIZESERVICE_STAT_NULL:
            {
#ifdef _ATC_ONLY

                if(FALSE == ATC_InterrogateCFService(MN_SS_CFU, dual_sys))
                {
                    ATC_TRACE_LOW("ATC: SynchronizeService, ERROR!!! status: %d, MN_SS_CFU", atc_sync_srv->status);
                    return FALSE;
                }

#endif
            }
            break;

        case ATC_SYNCHRONIZESERVICE_STAT_CFU:
            {
#ifdef _ATC_ONLY

                if(FALSE == ATC_InterrogateCFService(MN_SS_CFB, dual_sys))
                {
                    ATC_TRACE_LOW("ATC: SynchronizeService, ERROR!!! status: %d, MN_SS_CFB", atc_sync_srv->status);
                    return FALSE;
                }

#endif
            }
            break;

        case ATC_SYNCHRONIZESERVICE_STAT_CFB:
            {
#ifdef _ATC_ONLY

                if(FALSE == ATC_InterrogateCFService(MN_SS_CFNRY, dual_sys))
                {
                    ATC_TRACE_LOW("ATC: SynchronizeService, ERROR!!! status: %d, MN_SS_CFNRY", atc_sync_srv->status);
                    return FALSE;
                }

#endif
            }
            break;

        case ATC_SYNCHRONIZESERVICE_STAT_CFNRY:
            {
#ifdef _ATC_ONLY

                if(FALSE == ATC_InterrogateCFService(MN_SS_CFNRC, dual_sys))
                {
                    ATC_TRACE_LOW("ATC: SynchronizeService, ERROR!!! status: %d, MN_SS_CFNRC", atc_sync_srv->status);
                    return FALSE;
                }

#endif
            }
            break;

        case ATC_SYNCHRONIZESERVICE_STAT_CFNRC:
            /* nothing to do, anyway change status to next */
            break;

        case ATC_SYNCHRONIZESERVICE_STAT_FINISHED:
        default:
            /* return directly, no status change */
            return FALSE;
    }

    /* status change */
    ATC_TRACE_LOW("ATC: SynchronizeService, status: %d->%d", atc_sync_srv->status, atc_sync_srv->status + 1);
    atc_sync_srv->status++;

    /* if finished, LOGING */
    if(ATC_SYNCHRONIZESERVICE_STAT_FINISHED == atc_sync_srv->status)
    {
        ATC_TRACE_LOW("ATC: SynchronizeService, finished! cfall:%d,cfu:%d,cfcond:%d,cfb:%d,cfnrc:%d,cfnry:%d",
                      atc_sync_srv->cfall,
                      atc_sync_srv->cfu,
                      atc_sync_srv->cfcond,
                      atc_sync_srv->cfb,
                      atc_sync_srv->cfnrc,
                      atc_sync_srv->cfnry);
    }

    return TRUE;
}

/*****************************************************************************/
//  Description : This function handles +Esqopt Time out
//  Global resource dependence : g_rsp_str
//  Author:      sunny.shen
//  Note:
/*****************************************************************************/
void ATC_ProcessEsqoptTimeout(
    uint32 lparam   //the function's input parameter
)
{
    ATC_CONFIG_T *atc_config_ptr = &s_config_info;
    MN_PHONE_SCELL_MEAS_T rssi;
    uint8         ecsq_str[20] = {0};
    MN_DUAL_SYS_E dual_sys = ATC_CURRENT_SIM_ID_MUX;

    rssi = MNPHONE_GetRSSIEx(dual_sys);

    ATC_TRACE_LOW("ATC: ATC_ProcessEsqoptTimeout,+ECSQ rxlev=%d,rxqual=%d", \
                  rssi.rxlev, rssi.rxqual_full);

    sprintf((char *)ecsq_str, "+ECSQ: %d,%d",
            ATC_GetConvertedRSSIVlaueForECSQ(rssi.rxlev), rssi.rxqual_full);
    ATC_BuildUnsolicitedInfo(dual_sys, atc_config_ptr, ecsq_str);

    if(SCI_IsTimerActive(atc_config_ptr->esqopt_timer))
    {
        SCI_DeactiveTimer(atc_config_ptr->esqopt_timer);
        ATC_TRACE_LOW("ATC: +ECSQ deactivate timer first while time out");
    }

    if(atc_config_ptr->esqopt_param.option == ATC_ESQOPT_TIMER_INTERVAL)
    {
        SCI_ChangeTimer(atc_config_ptr->esqopt_timer, ATC_ProcessEsqoptTimeout, \
                        atc_config_ptr->esqopt_param.timer_value * ATC_ESQOPT_TIMER_UNIT);
        SCI_ActiveTimer(atc_config_ptr->esqopt_timer);
    }

    return;
}

uint8 ATC_Get_Data_Link_Id(uint8 link_id, uint8 cid)
{
    uint8 data_link_id = ATC_INVALID_LINK_ID;

#ifdef _MUX_ENABLE_
    if(g_mux_sys_info[MUX_1].is_start == MUX_STATUS_OFF)
    {
        data_link_id = 0;
    }
    else if((cid >= 1) &&
       ( cid < MIN(MN_GPRS_MAX_PDP_CONTEXT_COUNT, ATC_DEBUG_LINK - MN_SYS_NUMBER * ATC_LINK_NUM_PER_SIM) )) //lint !e506
    {
        data_link_id = ATC_GetLinkIdFromCid(cid);
    }
    else
#endif
    {
        data_link_id = 0;
    }

    return data_link_id;
}

/******************************************************************************/
// Description : This function do some reset work for ATC
// Global resource dependence : g_atc_info_table
//                              g_atc_handle_table
//                              g_atc_result_code_text
// Author : Yijin
// Note : This function should be called when ps reset confirm received
/******************************************************************************/
int ATC_Reset(MN_DUAL_SYS_E dual_sys)
{
    ATC_CONFIG_T *atc_config_ptr = &s_config_info;
    uint8 i;
#ifdef _MUX_ENABLE_
    uint8 link_id = 0xff;
    /* save current link id */
    uint8 current_link_id = atc_config_ptr->current_link_id;
#endif

    ATC_TRACE_LOW("ATC: ATC_Reset");

    SCI_MEMSET((void *)&g_cur_plmn_info[dual_sys], 0, sizeof(ATC_PLMN_INFO_T));

    if(g_calls_state[dual_sys].call_amount != 0)
    {
        APP_MN_CALL_DISCONNECTED_IND_T disc_ind;
        uint8                          call_index;

        /* Makeup disconnect-ind message*/
        SCI_MEMSET(((void *)&disc_ind), 0x00, sizeof(disc_ind));
        disc_ind.SignalCode         = APP_MN_CALL_DISCONNECTED_IND;
        disc_ind.disconnected_cause = MN_CAUSE_LOW_FAILURE;

        for(call_index = 0; call_index < g_calls_state[dual_sys].call_amount; call_index++)
        {
            if(ATC_CALL_IDLE_STATE != g_calls_state[dual_sys].call_context[call_index].call_entity_state)
            {
                disc_ind.call_id = g_calls_state[dual_sys].call_context[call_index].call_id;

#ifdef _MUX_ENABLE_
                ATC_Receive_Expected_Event(S_CONFIG_INFO_PTR, disc_ind.SignalCode,
                                        disc_ind.call_id, ATC_DOMAIN_CS, dual_sys);
#endif
                ATC_ProcessCallDiscInd(S_CONFIG_INFO_PTR, (xSignalHeaderRec *)&disc_ind);
            }
        }
    }

    SCI_MEMSET(((void *)&g_calls_state[dual_sys]), 0, sizeof(ATC_CALLS_STATE_T));

    for(i = 0; i < MN_CALL_MAX_CALL_NUM; i++)
    {
        g_calls_state[dual_sys].call_context[i].call_id = ATC_INVALID_CALLID;
    }

    for(i = 0; i < ATC_MAX_ACTIVATE_PDP; i++)
    {
        if(atc_config_ptr->atc_psd_call_info[i].state != ATC_PSD_STATE_INACTIVE)
        {
            APP_MN_GPRS_DEACTIVATE_IND_T deact_ind;
#ifdef _MUX_ENABLE_
            if (g_atc_modem_dial_sys != dual_sys)
            {
                continue;
            }
#endif            
            /* close data panel */
            {
#ifdef  _MUX_ENABLE_
#ifdef _ATC_WAIT_L4
                TCPIPPS_ResetModemService(i + 1);
#endif
#else
                //SIO_ATC_SetDataMode(FALSE);
                TCPIP_ResetPPPInterface();

#endif /* _MUX_ENABLE_ */
            }

            /* Makeup pseudo deactivate-ind message */
            SCI_MEMSET(((void *)&deact_ind), 0x00, sizeof(deact_ind));
            deact_ind.SignalCode = APP_MN_DEACTIVATE_PDP_CONTEXT_IND;
            deact_ind.result     = MN_GPRS_ERR_SUCCESS;
            deact_ind.pdp_num    = 1;
            deact_ind.pdp_id[0]  = i + 1; /* cid */

            ATC_GET_MUX_LINK_ID(link_id, dual_sys);
            ATC_SET_CURRENT_LINK_ID_MUX(link_id);

            if(atc_config_ptr->atc_psd_call_info[i].state == ATC_PSD_STATE_ACTIVE)
            {
                ATC_BuildGprsEvReportRsp(S_CONFIG_INFO_PTR, (xSignalHeaderRec *)&deact_ind);
            }
            else
            {
                SCI_TRACE_LOW("ATC: pdp state error.");
            }

#ifdef _MUX_ENABLE_
            /*remove all excepted events of pdp */
            ATC_Clear_All_Exp_Result(i + 1, ATC_DOMAIN_PS, dual_sys);
#endif

            ATC_Clear_PSD_Call_Info(i + 1);
        }
    }

    for(i = 0; i < ATC_MAX_LINK_ID; i++)
    {
        /* PSD call fields */
        atc_link_inst[i].cid            = ATC_INVALID_CID;
        atc_link_inst[i].ath_cid        = ATC_INVALID_CID;
        atc_link_inst[i].ath_stat       = ATC_ATH_NULL;

        if(SCI_IsTimerActive(atc_link_inst[i].ath_wait_timer))
        {
            SCI_DeactiveTimer(atc_link_inst[i].ath_wait_timer);
        }

        if(SCI_IsTimerActive(atc_link_inst[i].ath_avoid_rep_timer))
        {
            SCI_DeactiveTimer(atc_link_inst[i].ath_avoid_rep_timer);
        }
    }

#ifdef _MUX_ENABLE_

    //to check expect event,  if exit, then report an error to AP
    for(i = 0; i < ATC_MAX_EXP_RESULT; i++)
    {
        if (atc_config_ptr->atc_exp_link_info.atc_exp_result_info[i].dual_sys != dual_sys)
        {
            continue;
        }
    
        if((ATC_INVALID_LINK_ID != atc_config_ptr->atc_exp_link_info.atc_exp_result_info[i].link_id)
                && (ATC_NO_EVENT != atc_config_ptr->atc_exp_link_info.atc_exp_result_info[i].exp_event)
                && (APP_MN_ACTIVATE_PDP_CONTEXT_CNF != atc_config_ptr->atc_exp_link_info.atc_exp_result_info[i].exp_event)
                && (APP_MN_DEACTIVATE_PDP_CONTEXT_CNF != atc_config_ptr->atc_exp_link_info.atc_exp_result_info[i].exp_event)
                && (APP_MN_CALL_START_IND != atc_config_ptr->atc_exp_link_info.atc_exp_result_info[i].exp_event)
                && (APP_MN_ALERTING_IND != atc_config_ptr->atc_exp_link_info.atc_exp_result_info[i].exp_event)
                && (APP_MN_SETUP_COMPLETE_IND != atc_config_ptr->atc_exp_link_info.atc_exp_result_info[i].exp_event)
                && (APP_MN_CALL_DISCONNECTED_IND != atc_config_ptr->atc_exp_link_info.atc_exp_result_info[i].exp_event)
                && (APP_MN_CALL_ERR_IND != atc_config_ptr->atc_exp_link_info.atc_exp_result_info[i].exp_event)
          )
        {
            if(
                (APP_MN_PS_POWER_OFF_CNF != atc_config_ptr->atc_exp_link_info.atc_exp_result_info[i].exp_event)
                && (APP_MN_SIM_POWER_OFF_CNF != atc_config_ptr->atc_exp_link_info.atc_exp_result_info[i].exp_event)
            )
            {
                if (current_link_id != atc_config_ptr->atc_exp_link_info.atc_exp_result_info[i].link_id)
                {
                    ATC_Set_Current_Link_id(S_CONFIG_INFO_PTR, atc_config_ptr->atc_exp_link_info.atc_exp_result_info[i].link_id);
                    ATC_BuildResultErrCodeRsp(S_CONFIG_INFO_PTR, S_ATC_RESULT_CODE_ERROR, 0);
                }
                ATC_Clear_Exp_Result(&atc_config_ptr->atc_exp_link_info.atc_exp_result_info[i]);
            }
        }

    }

    ATC_Shrink_Expected_Event_Buffer(atc_config_ptr);
#endif

#ifdef _MUX_ENABLE_
    /* restore current link id */
    atc_config_ptr->current_link_id = current_link_id;
#endif

    if(PNULL != g_emg_num_list_ptr[dual_sys])
    {
        SCI_FREE(g_emg_num_list_ptr[dual_sys]);
    }

    #ifdef ATC_SAT_ENABLE
    SCI_MEMSET((void *)g_rxlev_arr,0,sizeof(g_rxlev_arr));
    #endif

    return TRUE;
}

#ifdef _MUX_ENABLE_
/******************************************************************************/
// Description : This function find pending event from event list
// Global resource dependence :
// Author : Martin Jiang
// Note :
/******************************************************************************/
BOOLEAN ATC_GetPending_Event(
    ATC_CONFIG_T *atc_config_ptr,
    uint16 expected_event,
    uint8 link_id,
    MN_DUAL_SYS_E dual_sys
)
{
    return (0 != ATC_Find_Expected_Event(atc_config_ptr, expected_event, link_id, dual_sys));
}

/******************************************************************************/
// Description : This function find number of expected events from event list
// Global resource dependence :
// Author : minqian Qian
// Note :
/******************************************************************************/
uint8 ATC_Find_Expected_Event(
    ATC_CONFIG_T *atc_config_ptr,
    uint16 expected_event,
    uint8 link_id,
    MN_DUAL_SYS_E dual_sys
)
{
    uint8   count = 0;
    uint16 start_p, end_p;

    start_p = atc_config_ptr->atc_exp_link_info.start_point;
    end_p  = atc_config_ptr->atc_exp_link_info.end_point;

    if ((end_p >= ATC_MAX_EXP_RESULT) || (start_p >= ATC_MAX_EXP_RESULT))
    {
        ATC_TRACE_LOW("ATC: ATC_Find_Expected_Event  end_p %d or start_p %d exceeds %d",
                end_p, start_p, ATC_MAX_EXP_RESULT);
        return 0;
    }

    while(start_p != end_p)
    {
        if((atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].dual_sys == dual_sys) &&
            ((atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].exp_event == expected_event) ||
            (atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].ext_exp_event == expected_event))
          )
        {
            if(ATC_INVALID_LINK_ID == link_id)
            {
                count++;
            }
            else if(atc_config_ptr->atc_exp_link_info.atc_exp_result_info[start_p].link_id == link_id)
            {
                count++;
            }
        }

        start_p++;

        if(ATC_MAX_EXP_RESULT == start_p)
        {
            start_p = 0;
        }
    }

    return count;
}

/******************************************************************************/
// Description : This function is used to check call expect event when recv call disconnect
//               called when atc recv call disconect or call error ind
// Global resource dependence : None
// Author : 
// Note : None
/******************************************************************************/
LOCAL void CheckCallEvent(MN_DUAL_SYS_E dual_sys, uint8 call_id)
{
    BOOLEAN recv_event_result = FALSE;
    
    recv_event_result = ATC_Receive_Expected_Event(S_CONFIG_INFO_PTR, APP_MN_SETUP_COMPLETE_IND,
                                                     call_id,
                                                     ATC_DOMAIN_CS, dual_sys);
    if(recv_event_result && s_config_info.ss_operate[dual_sys] != ATC_INVALID_OPERATE)
    {
        SCI_TRACE_LOW("ATC: check call event, exist APP_MN_SETUP_COMPLETE_IND");
        ATC_BuildResultCodeRsp(S_CONFIG_INFO_PTR, S_ATC_RESULT_CODE_OK, 0);
        return ;
    }
    
    recv_event_result = ATC_Receive_Expected_Event(S_CONFIG_INFO_PTR, APP_MN_CALL_HELD_CNF,
                                             call_id,
                                             ATC_DOMAIN_CS, dual_sys);
    if(recv_event_result && s_config_info.ss_operate[dual_sys] != ATC_INVALID_OPERATE)
    {
        SCI_TRACE_LOW("ATC: check call event, exist APP_MN_CALL_HELD_CNF");
        ATC_BuildResultCodeRsp(S_CONFIG_INFO_PTR, S_ATC_RESULT_CODE_OK, 0);
        return ;
    }
    
    recv_event_result = ATC_Receive_Expected_Event(S_CONFIG_INFO_PTR, APP_MN_CALL_RETRIEVED_CNF,
                                             call_id,
                                             ATC_DOMAIN_CS, dual_sys);
    if(recv_event_result && s_config_info.ss_operate[dual_sys] != ATC_INVALID_OPERATE)
    {
        SCI_TRACE_LOW("ATC: check call event, exist APP_MN_CALL_RETRIEVED_CNF");
        ATC_BuildResultCodeRsp(S_CONFIG_INFO_PTR, S_ATC_RESULT_CODE_OK, 0);
        return ;
    }
    
    recv_event_result = ATC_Receive_Expected_Event(S_CONFIG_INFO_PTR, APP_MN_BUILD_MPTY_CNF,
                                             call_id,
                                             ATC_DOMAIN_CS, dual_sys);
    if(recv_event_result && s_config_info.ss_operate[dual_sys] != ATC_INVALID_OPERATE)
    {
        SCI_TRACE_LOW("ATC: check call event, exist APP_MN_BUILD_MPTY_CNF");
        ATC_BuildResultCodeRsp(S_CONFIG_INFO_PTR, S_ATC_RESULT_CODE_OK, 0);
        return ;
    }
}
#endif

/******************************************************************************/
// Description : This function up DSCI info to TE.
// Global resource dependence : None
// Author : minqian.qian
// Note : None
/******************************************************************************/
void ATC_UpDSCIInfo(
    MN_DUAL_SYS_E dual_sys,
    ATC_CONFIG_T    *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    xSignalHeaderRec        *sig_ptr,
    uint8       call_index,         //the index of call state context
    uint8       callstat               //the state of cal     0 ~ 6
)
{
    uint8   bs_type_info;

    SCI_ASSERT(PNULL != atc_config_ptr && PNULL != sig_ptr);/*assert verified: check null pointer*/
    
    if(!atc_config_ptr->urc_switch)
    {
        return;
    }

    /*^DSCI: <id>,<idr>,<stat>,<type>,<mpty>,<number>,<num_type>,[<bs_type>][,cause]*/
    if(atc_config_ptr->dsci_flag == TRUE)
    {
        sprintf((char *)g_rsp_str, "^DSCI: %d,%d,%d,%d,%d,%s,%d", g_calls_state[dual_sys].call_context[call_index].call_id + 1, \
                g_calls_state[dual_sys].call_context[call_index].direction,  \
                callstat,  \
                g_calls_state[dual_sys].call_context[call_index].call_mode,  \
                g_calls_state[dual_sys].call_context[call_index].mpty_state,  \
                g_calls_state[dual_sys].call_context[call_index].party_number,  \
                g_calls_state[dual_sys].call_context[call_index].number_type
               );

        //append <bs_type> or [cause] info according to call_mode and expected_event
        if(g_calls_state[dual_sys].call_context[call_index].call_mode == ATC_DATA_MODE)
        {
            bs_type_info = 1;

            if(APP_MN_CALL_DISCONNECTED_IND == sig_ptr->SignalCode)
            {
                sprintf((char *)(&g_rsp_str[strlen((char *)g_rsp_str)]), ",%d,%d", \
                        bs_type_info, atc_config_ptr->ceer_val);
            }
            else
            {
                sprintf((char *)(&g_rsp_str[strlen((char *)g_rsp_str)]), ",%d", bs_type_info);    /* only support VT now  */
            }
        }
        else
        {
            if(APP_MN_CALL_DISCONNECTED_IND == sig_ptr->SignalCode)
            {
                sprintf((char *)(&g_rsp_str[strlen((char *)g_rsp_str)]), ",,%d", \
                        atc_config_ptr->ceer_val);
            }
        }

        ATC_BuildUnsolicitedInfo(dual_sys, atc_config_ptr, g_rsp_str);
    }
}

/******************************************************************************/
// Description : This function up +SIND info
// Global resource dependence : None
// Author : minqian.qian
// Note : None
/******************************************************************************/
void ATC_UpSINDInfo(MN_DUAL_SYS_E  dual_sys,
                    ATC_CONFIG_T *atc_config_ptr,
                    xSignalHeaderRec        *sig_ptr,
                    uint16 atc_wind_bit_flag,       //which the wind bit to check with
                    uint16  upValue,            //+sind up value
                    BOOLEAN  autoUpFlag         //is auto up or not
                   )
{
    uint16  rpValue;
    BOOLEAN isSet;

    SCI_ASSERT(PNULL != atc_config_ptr);/*assert verified: check null pointer*/
    ATC_TRACE_LOW("ATC: ATC_UpSINDInfo, IndLevel: %d, atc_wind_bit_flag: %d, upValue: %d, UpFlag:%d",
                  atc_config_ptr->atc_config_nv.wind_ind_level, atc_wind_bit_flag, upValue, autoUpFlag);

    if(!autoUpFlag || !atc_config_ptr->urc_switch)
    {
        return;
    }

    isSet = FALSE;
    rpValue = upValue;

    if(ATC_16BIT1 == ((atc_config_ptr->atc_config_nv.wind_ind_level) | atc_wind_bit_flag))
    {
        if(ATC_WIND_BIT4 == atc_wind_bit_flag)
        {
            //sind call status
            uint8     callid;

            SCI_ASSERT(PNULL != sig_ptr);/*assert verified: check null pointer*/

            switch(sig_ptr->SignalCode)
            {
                case APP_MN_CALL_START_IND:
                    callid = ((APP_MN_CALL_START_IND_T *)sig_ptr)->call_id + 1;
                    break;
                case APP_MN_SETUP_IND:
                    callid = ((APP_MN_SETUP_IND_T *)sig_ptr)->call_id + 1;
                    break;
                default:
                    return;
            }

            sprintf((char *)g_rsp_str, "%s: %d,%d",
                    g_atc_info_table[AT_CMD_SIND].cmd_name->str_ptr, rpValue, callid);
            isSet = TRUE;
        }
        else if(ATC_WIND_BIT5 == atc_wind_bit_flag)
        {
            //sind call is realesed
            SCI_ASSERT(PNULL != sig_ptr);/*assert verified: check null pointer*/
            sprintf((char *)g_rsp_str, "%s: %d,%d,%d",
                    g_atc_info_table[AT_CMD_SIND].cmd_name->str_ptr,
                    rpValue,
                    ((APP_MN_CALL_DISCONNECTED_IND_T *)sig_ptr)->call_id + 1,
                    ((APP_MN_CALL_DISCONNECTED_IND_T *)sig_ptr)->disconnected_cause);
            isSet = TRUE;
        }
        else if(ATC_WIND_BIT9 == atc_wind_bit_flag)
        {
            //sind phonebook status
            switch(sig_ptr->SignalCode)
            {
                case APP_MN_SIM_NOT_READY_IND:
                    sprintf((char *)g_rsp_str, "%s: %d,\"SM\",0,\"FD\",0,\"LD\",0,\"MC\",0,\"RC\",0,\"ME\",0",
                            g_atc_info_table[AT_CMD_SIND].cmd_name->str_ptr, rpValue);
                    break;
                case APP_MN_SIM_READY_IND:
                    sprintf((char *)g_rsp_str, "%s: %d,\"SM\",1,\"FD\",1,\"LD\",1,\"MC\",1,\"RC\",1,\"ME\",1",
                            g_atc_info_table[AT_CMD_SIND].cmd_name->str_ptr, rpValue);
                    break;
                default:
                    return;
            }

            isSet = TRUE;
        }

        if(!isSet)
        {
            sprintf((char *)g_rsp_str, "%s: %d", g_atc_info_table[AT_CMD_SIND].cmd_name->str_ptr, rpValue);
        }

        ATC_BuildUnsolicitedInfo(dual_sys, atc_config_ptr, g_rsp_str);
    }
}

/******************************************************************************/
// Description : This function up ^SYSINFO info
// Global resource dependence : None
// Author : minqian.qian
// Note : None
/******************************************************************************/
void ATC_UpSYSINFOInfo(MN_DUAL_SYS_E dual_sys,
                       ATC_CONFIG_T *atc_config_ptr,
                       BOOLEAN       autoUpFlag
                       )
{
    ATC_PHONE_SERVICE_STATUS_E              atc_srv_status = ATC_PHONE_NO_SERVICE;
    MN_PHONE_USER_SERVICE_SELECT_E     atc_srv_domain = MN_PHONE_USER_SELECT_NONE;
    BOOLEAN                                                  atc_roam_status = FALSE;
    ATC_PHONE_RAT_E                                   atc_sys_mode = ATC_PHONE_RAT_NONE;
    ATC_PHONE_SIM_STATE_E                        atc_sim_state = ATC_PHONE_SIM_INVALID;
    CELL_State_E                                           atc_cell_state = CELL_STATE_NULL;
    MN_PHONE_CURRENT_PLMN_INFO_T       register_status = {0};
    MNSIM_ERROR_E        error_code = MNSIM_ERROR_NULL;
    ATC_SERVICE_STATE_E  srv_state      = ATC_SEVICE_NONE;
    BOOLEAN              is_emc_allowed = MNPHONE_GetScellEmcSupportCapEx(dual_sys);
  
    SCI_TRACE_LOW("ATC: ATC_UpSYSINFOInfo");
    SCI_ASSERT(PNULL != atc_config_ptr);/* assert verified: null pointer */
    atc_srv_domain  = MNPHONE_GetServiceTypeEx(dual_sys);

    if (MNSIM_GetSimStatusEx(dual_sys, &error_code))
    {
        atc_sim_state = ATC_PHONE_SIM_VALID;
    }
    else if ((error_code <= MNSIM_ERROR_INVALID_STATUS_RSP) ||
            ((error_code >= MNSIM_ERROR_PIN1_UNINITIALIZED) && 
            (error_code <= MNSIM_ERROR_NO_SECRETCODE_INITIALIZED)) )
    {
        atc_sim_state = ATC_PHONE_SIM_UNKNOW;
    }
    else
    {
        atc_sim_state = ATC_PHONE_SIM_INVALID;
    }
    
    register_status = MNPHONE_GetCurrentPLMNInfoEx(dual_sys);
    atc_roam_status = register_status.plmn_is_roaming;
    switch(register_status.plmn_status)
    {
        case PLMN_NORMAL_GSM_ONLY:
            srv_state = (TRUE == is_emc_allowed) ? ATC_SERVICE_CS_EMC : ATC_SERVICE_CS;
            atc_srv_status = ATC_PHONE_FULL_SERVICE;
            break;
        case PLMN_NORMAL_GPRS_ONLY:
            srv_state = (TRUE == is_emc_allowed) ? ATC_SERVICE_PS_EMC : ATC_SERVICE_PS;
            atc_srv_status = ATC_PHONE_FULL_SERVICE;
            break;
        case PLMN_NORMAL_GSM_GPRS_BOTH:
            srv_state = (TRUE == is_emc_allowed) ? ATC_SERVICE_CS_PS_EMC : ATC_SERVICE_CS_PS;
            atc_srv_status = ATC_PHONE_FULL_SERVICE;
            break;

        case PLMN_EMERGENCY_ONLY:
        case PLMN_EMERGENCY_SIM_INVALID:
            srv_state = (TRUE == is_emc_allowed) ? ATC_SERVICE_EMC : ATC_SEVICE_NONE;
            atc_srv_status = ATC_PHONE_LIMITED_SERVICE;
            break;
        case PLMN_EMERGENCY_GPRS_ONLY:
        case PLMN_EMERGENCY_SIM_INVALID_GPRS_ONLY:
            srv_state = (TRUE == is_emc_allowed) ? ATC_SERVICE_PS_EMC : ATC_SERVICE_PS;
            atc_srv_status = ATC_PHONE_LIMITED_SERVICE;
            break;

        case PLMN_NO_SERVICE:
        default :
            atc_srv_status = ATC_PHONE_NO_SERVICE;
            // srv_state      = ATC_SEVICE_NONE;
            break;
    }
    
    if (PLMN_NORMAL_GSM_GPRS_BOTH == register_status.plmn_status ||
        PLMN_NORMAL_GSM_ONLY ==  register_status.plmn_status)
    {
        atc_sys_mode = ATC_PHONE_RAT_GSM;
        if (register_status.rat == MN_GMMREG_RAT_GPRS)
        {
            if (register_status.cell_capability.edge_support)
            {
               atc_cell_state = CELL_STATE_EDGE;
            }
            else
            {
                atc_cell_state = CELL_STATE_GSM;
            }
        }
        /* OTHERS */
        else
        {
            atc_cell_state = CELL_STATE_GSM;
        }
    }
    else
    {
       atc_sys_mode = ATC_PHONE_RAT_NONE;
       atc_cell_state = CELL_STATE_NULL;
    }

    sprintf((char*)g_rsp_str, "^SYSINFO: %d,%d,%d,%d,%d,0,%d,%d",
                atc_srv_status,atc_srv_domain,atc_roam_status,atc_sys_mode,atc_sim_state,atc_cell_state,srv_state);

    if (autoUpFlag)
    {
        ATC_BuildUnsolicitedInfo(dual_sys, atc_config_ptr, g_rsp_str);
    }
    else
    {
        ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
    }
}

/******************************************************************************/
// Description : This function up +ECIND info to TE.
// Global resource dependence : None
// Author : minqian.qian
// Note : None
/******************************************************************************/
void ATC_UpECINDInfo(
    MN_DUAL_SYS_E     dual_sys,
    ATC_CONFIG_T     *atc_config_ptr,
    xSignalHeaderRec *sig_ptr,
    uint16            type, 
    uint16            value1,
    uint16            value2, 
    uint16            value3, 
    uint16            value4)
{
    SCI_ASSERT(PNULL != atc_config_ptr);/*assert verified: check null pointer*/
    
    if(!atc_config_ptr->urc_switch)
    {
        return;
    }

    switch(type)
    {
        case 0:     //up <Call status>
            sprintf((char *)g_rsp_str, "+ECIND: %d,%d,%d", type, value1, value2);
            break;

        case 1:     //up <Signal strength>
            return;

        case 2:     //up <SMS service related>
        {
            MN_SMS_MEM_FULL_STATUS_E    mem_status;
            SCI_ASSERT(PNULL != sig_ptr);/*assert verified: check null pointer*/
            mem_status = ((APP_MN_SMS_MEM_FULL_IND_T *)sig_ptr)->mem_status;

            if(MN_SMS_ALL_FULL == mem_status || MN_SMS_SIM_FULL == mem_status)
            {
                sprintf((char *)g_rsp_str, "+ECIND: %d,1", type);
            }
            else if(MN_SMS_MEM_AVAIL == mem_status)
            {
                sprintf((char *)g_rsp_str, "+ECIND: %d,0", type);
            }
            else
            {
                return;
            }
            break;
        }

        case 3:     //up <SIM service related>
        {
            switch(value1)
            {
                case 0:     //Indicate that it is rejected SIM card
                    sprintf((char *)g_rsp_str, "+ECIND: %d,%d", type, value1);
                    break;

                case 1:     //Indicate that the remove event of SIM occurs
                    sprintf((char *)g_rsp_str, "+ECIND: %d,%d", type, value1);
                    break;

                case 2:     //Indicate that the insert event of SIM occurs
                    return;

                case 3:     //sim status
                {
                    uint8 sim_status = 0;
                    uint8 pin_status = 0;
                    MN_CARD_TYPE_E card_type = MN_CARD_TYPE_NUM;

                    ATC_GetUiccInfo(dual_sys, atc_config_ptr, &sim_status, &pin_status, &card_type);

                    SCI_ASSERT(PNULL != sig_ptr);/*assert verified: check null pointer*/

                    if(sig_ptr->SignalCode == APP_MN_SIM_READY_IND)
                    {
                        sim_status = 0;
                        pin_status = 0;
                    }
                    else if(sig_ptr->SignalCode == APP_MN_NETWORK_STATUS_IND && 1 == sim_status)
                    {
                        return;     //if MN_SIM_TYPE_NO_SIM status, return
                    }

                    sprintf((char *)g_rsp_str, "+ECIND: %d,%d,%d,%d,%d",
                            type, value1, sim_status, pin_status, card_type);
                    break;
                }

                default:
                    return;
            }
            break;
        }
        case 4:     //up <PS service related>
        case 5:     //up <CP related>
        default:
            return;
    }

    ATC_BuildUnsolicitedInfo(dual_sys, atc_config_ptr, g_rsp_str);
}

/******************************************************************************/
// Description : This function up +CREG info to TE.
// Global resource dependence : None
// Author : minqian.qian
// Note : None
/******************************************************************************/
void ATC_UpCREGInfo(MN_DUAL_SYS_E                   dual_sys,
                    ATC_CONFIG_T                   *atc_config_ptr,
                    MN_PHONE_CURRENT_PLMN_INFO_T   *plmn_info_ptr,  //plmn info
                    OPER_STATUS_E                   oper_status,   //oper status
                    BOOLEAN                         autoUpFlag    //unsolicited or not
                   )
{
    uint8                           cregValue;
    ATC_REG_NETWORK_ACT_E           nw_act = NW_ACT_0_GSM;
    MN_PHONE_EHPLMN_LIST_T          ehplmn_list;
    uint32                          i;
    MN_PHONE_CURRENT_PLMN_INFO_T    cur_plmn = {0};
    MN_PLMN_T                       hplmn = {0};

    SCI_ASSERT(PNULL != atc_config_ptr && PNULL != plmn_info_ptr);/*assert verified: check null pointer*/

    cregValue = atc_config_ptr->atc_config_nv.creg;
    ATC_TRACE_LOW("ATC: UP +CREG, mode:%d,unsolicited flag:%d,oper_status:%d",
                  cregValue, autoUpFlag, oper_status);

    if(oper_status == REGISTERED_ROAMING)
    {
        //get current register plmn
        cur_plmn = MNPHONE_GetCurrentPLMNInfoEx(dual_sys);
        
        //atc check ehplmn list for creg
        if(MNSIM_GetEHPLMNEx(dual_sys, &ehplmn_list))
        {
            for(i=0; i<ehplmn_list.plmn_num; i++)
            {
                SCI_TRACE_LOW("ATC: eplmn num = %d,mnc = %d",ehplmn_list.plmn_num,ehplmn_list.plmn_arr[i].mnc);
                if(ehplmn_list.plmn_arr[i].mnc == cur_plmn.mnc && ehplmn_list.plmn_arr[i].mcc == cur_plmn.mcc)
                {
                    oper_status = REGISTERED_HOME_PLMN;
                    break;
                }
            }
        }
        
        if(oper_status == REGISTERED_ROAMING)
        {
            //get hplmn
            hplmn = MNSIM_GetHplmnEx(dual_sys);
            SCI_TRACE_LOW("ATC: hplmn mnc %d",hplmn.mnc);
            if(cur_plmn.mnc == hplmn.mnc && cur_plmn.mcc == hplmn.mcc)
            {
                oper_status = REGISTERED_HOME_PLMN;
            }
        }
    }
    
    if(autoUpFlag)
    {
        /*unsolicited flow*/
        switch(cregValue)
        {
            case ATC_CREG_ENABLE:
                sprintf((char *)g_rsp_str, "%s: %d", g_atc_info_table[AT_CMD_CREG].cmd_name->str_ptr, oper_status);
                break;
            case ATC_CREG_AND_INFO:

                if((REGISTERED_ROAMING == oper_status) ||
                        (REGISTERED_HOME_PLMN == oper_status))
                {
                    nw_act = ATC_GetRegNwAct(atc_config_ptr, plmn_info_ptr);
                    sprintf((char *)g_rsp_str, "%s: %d,\"%04X\",\"%08X\",%d",
                            g_atc_info_table[AT_CMD_CREG].cmd_name->str_ptr,
                            oper_status, plmn_info_ptr->lac, plmn_info_ptr->cell_id, nw_act);
                }
                else
                {
                    sprintf((char *)g_rsp_str, "%s: %d", g_atc_info_table[AT_CMD_CREG].cmd_name->str_ptr, oper_status);
                }

                break;
            case ATC_CREG_DISABLE:  //do not send unsolicited +CREG
                return;
            default:
                ATC_TRACE_LOW("ATC: ATC_UpCREGInfo, unknow creg enum:%d.", cregValue);
                return;
        }

        ATC_BuildUnsolicitedInfo(dual_sys, atc_config_ptr, g_rsp_str);
    }
    else
    {
        /*AT+CREG process flow*/
        switch(cregValue)
        {
            case ATC_CREG_DISABLE:
                sprintf((char *)g_rsp_str, "%s: 0,%d", g_atc_info_table[AT_CMD_CREG].cmd_name->str_ptr, oper_status);
                break;
            case ATC_CREG_ENABLE:
                sprintf((char *)g_rsp_str, "%s: 1,%d", g_atc_info_table[AT_CMD_CREG].cmd_name->str_ptr, oper_status);
                break;
            case ATC_CREG_AND_INFO:

                if((REGISTERED_ROAMING == oper_status) ||
                        (REGISTERED_HOME_PLMN == oper_status))
                {
                    nw_act = ATC_GetRegNwAct(atc_config_ptr, plmn_info_ptr);
                    sprintf((char *)g_rsp_str, "%s: %d,%d,\"%04X\",\"%08X\",%d",
                            g_atc_info_table[AT_CMD_CREG].cmd_name->str_ptr,
                            cregValue, oper_status, plmn_info_ptr->lac, plmn_info_ptr->cell_id, nw_act);
                }
                else
                {
                    sprintf((char *)g_rsp_str, "%s: %d,%d",
                            g_atc_info_table[AT_CMD_CREG].cmd_name->str_ptr, cregValue, oper_status);
                }

                break;
            default:
                ATC_TRACE_LOW("ATC: ATC_UpCREGInfo, unknow creg enum:%d.", cregValue);
                return;
        }

        ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
    }
}

/******************************************************************************/
// Description : This function up +CGREG info to TE.
// Global resource dependence : None
// Author : minqian.qian
// Note : None
/******************************************************************************/
void ATC_UpCGREGInfo(MN_DUAL_SYS_E                   dual_sys,
                     ATC_CONFIG_T                   *atc_config_ptr,
                     MN_PHONE_CURRENT_PLMN_INFO_T   *plmn_info_ptr,  //plmn info
                     OPER_STATUS_E                   gprs_oper_status,  //GPRS oper status
                     BOOLEAN                         autoUpFlag     //unsolicited or not
                     )
{
    uint8                           cgregValue;
    ATC_REG_NETWORK_ACT_E           nw_act = NW_ACT_0_GSM;
    MN_PHONE_EHPLMN_LIST_T          ehplmn_list;
    uint32                          i;
    MN_PHONE_CURRENT_PLMN_INFO_T    cur_plmn = {0};
    MN_PLMN_T                       hplmn = {0};

    SCI_ASSERT(PNULL != atc_config_ptr && PNULL != plmn_info_ptr);/*assert verified: check null pointer*/

    cgregValue = atc_config_ptr->cgreg;
    ATC_TRACE_LOW("ATC: UP +CGREG, mode:%d,unsolicited flag:%d,gprs_oper_status:%d",
                  cgregValue, autoUpFlag, gprs_oper_status);

    if(gprs_oper_status == REGISTERED_ROAMING)
    {
        //get current register plmn
        cur_plmn = MNPHONE_GetCurrentPLMNInfoEx(dual_sys);
        
        //atc check ehplmn list for creg
        if(MNSIM_GetEHPLMNEx(dual_sys, &ehplmn_list))
        {
            for(i=0; i<ehplmn_list.plmn_num; i++)
            {
                if(ehplmn_list.plmn_arr[i].mnc == cur_plmn.mnc && ehplmn_list.plmn_arr[i].mcc == cur_plmn.mcc)
                {
                    gprs_oper_status = REGISTERED_HOME_PLMN;
                    break;
                }
            }
        }
        
        if(gprs_oper_status == REGISTERED_ROAMING)
        {
            //get hplmn
            hplmn = MNSIM_GetHplmnEx(dual_sys);
            
            if(cur_plmn.mnc == hplmn.mnc && cur_plmn.mcc == hplmn.mcc)
            {
                gprs_oper_status = REGISTERED_HOME_PLMN;
            }
        }
    }
    
    if((atc_config_ptr->atc_config_nv.spauto_flag & 0x04) && (dual_sys == 0))
    {
        SCI_TRACE_LOW("ATC: set auto test flag for IQ catcher.");
        gprs_oper_status = REGISTERED_HOME_PLMN;
    }
    
    if(autoUpFlag)
    {
        /*unsolicited flow*/
        switch(cgregValue)
        {
            case ATC_CREG_ENABLE:
                sprintf((char *)g_rsp_str, "%s: %d",
                        g_atc_info_table[AT_CMD_CGREG].cmd_name->str_ptr, gprs_oper_status);
                break;
            case ATC_CREG_AND_INFO:

                if((REGISTERED_ROAMING == gprs_oper_status) ||
                        (REGISTERED_HOME_PLMN == gprs_oper_status))
                {
                    nw_act = ATC_GetRegNwAct(atc_config_ptr, plmn_info_ptr);

                    sprintf((char *)g_rsp_str, "%s: %d,\"%04X\",\"%08X\",%d",
                            g_atc_info_table[AT_CMD_CGREG].cmd_name->str_ptr,
                            gprs_oper_status, plmn_info_ptr->lac, plmn_info_ptr->cell_id, nw_act);
                }
                else
                {
                    sprintf((char *)g_rsp_str, "%s: %d",
                            g_atc_info_table[AT_CMD_CGREG].cmd_name->str_ptr, gprs_oper_status);
                }
                break;

            case ATC_CREG_DISABLE:  //do not send unsolicited +CREG
                return;

            default:
                ATC_TRACE_LOW("ATC: ATC_UpCGREGInfo, unknow cgreg enum:%d.", cgregValue);
                return;
        }

        ATC_BuildUnsolicitedInfo(dual_sys, atc_config_ptr, g_rsp_str);
    }
    else
    {
        /*AT+CGREG process flow*/
        switch(cgregValue)
        {
            case ATC_CREG_DISABLE:
                sprintf((char *)g_rsp_str, "%s: 0,%d", g_atc_info_table[AT_CMD_CGREG].cmd_name->str_ptr, gprs_oper_status);
                break;

            case ATC_CREG_ENABLE:
                sprintf((char *)g_rsp_str, "%s: 1,%d",
                        g_atc_info_table[AT_CMD_CGREG].cmd_name->str_ptr, gprs_oper_status);
                break;

            case ATC_CREG_AND_INFO:

                if((REGISTERED_ROAMING == gprs_oper_status) ||
                        (REGISTERED_HOME_PLMN == gprs_oper_status))
                {
                    nw_act = ATC_GetRegNwAct(atc_config_ptr, plmn_info_ptr);
                    sprintf((char *)g_rsp_str, "%s: %d,%d,\"%04X\",\"%08X\",%d",
                            g_atc_info_table[AT_CMD_CGREG].cmd_name->str_ptr,
                            cgregValue, gprs_oper_status, plmn_info_ptr->lac, plmn_info_ptr->cell_id, nw_act);
                }
                else
                {
                    sprintf((char *)g_rsp_str, "%s: %d,%d",
                            g_atc_info_table[AT_CMD_CGREG].cmd_name->str_ptr, cgregValue, gprs_oper_status);
                }

                break;

            default:
                ATC_TRACE_LOW("ATC: ATC_UpCGREGInfo, unknow cgreg enum:%d.", cgregValue);
                return;
        }

        ATC_BuildInfoRsp(atc_config_ptr, g_rsp_str);
    }
}

/******************************************************************************/
// Description : This function up ^CEND info to TE.
// Global resource dependence : None
// Author : minqian.qian
// Note : None
/******************************************************************************/
void ATC_UpCENDInfo(ATC_CONFIG_T      *atc_config_ptr,
                    xSignalHeaderRec *sig_ptr
                   )
{
    BOOLEAN upFlag          = TRUE;
    MN_DUAL_SYS_E    dual_sys = MN_DUAL_SYS_1;

    SCI_ASSERT(PNULL != atc_config_ptr);/*assert verified: check null pointer*/
    SCI_ASSERT(PNULL != sig_ptr);/*assert verified: check null pointer*/

    switch(sig_ptr->SignalCode)
    {
        case APP_MN_DEACTIVATE_PDP_CONTEXT_IND:
            sprintf((char *)g_rsp_str, "^CEND: %d,,%d,%d,%d", ((APP_MN_GPRS_DEACTIVATE_IND_T *)sig_ptr)->pdp_id[0], \
                    ATC_CM_CALL_END_NETWORK_END, MN_CAUSE_NORMAL_CLEARING, ATC_CMCC_CALLIND_CALL_TYPE_PACKET_DATA);
            dual_sys = ((APP_MN_GPRS_DEACTIVATE_IND_T *)sig_ptr)->dual_sys;
            break;
        case APP_MN_DEACTIVATE_PDP_CONTEXT_CNF:
            sprintf((char *)g_rsp_str, "^CEND: %d,,%d,%d,%d", ((APP_MN_GPRS_DEACTIVATE_CNF_T *)sig_ptr)->pdp_id[0], \
                    ATC_CM_CALL_END_CLIENT_END, MN_CAUSE_NORMAL_CLEARING, ATC_CMCC_CALLIND_CALL_TYPE_PACKET_DATA);
            dual_sys = ((APP_MN_GPRS_DEACTIVATE_CNF_T *)sig_ptr)->dual_sys;
            break;
        default:
            ATC_TRACE_LOW("ATC: ATC_UpCENDInfo, invalid signal code:%d.", sig_ptr->SignalCode);
            upFlag = FALSE;
            break;
    }

    if(upFlag)
    {
        ATC_BuildUnsolicitedInfo(dual_sys, atc_config_ptr, g_rsp_str);
    }

}

/******************************************************************************/
// Description : This function is used to unsolicited report +CUSD info to TE.
// Global resource dependence : None
// Author : minqian.qian
// Note : None
/******************************************************************************/
void ATC_UpCUSDInfo(
    MN_DUAL_SYS_E   dual_sys,
    ATC_CONFIG_T       *atc_config_ptr,
    MN_SS_USSD_DATA_T *ussd_ptr,   //ussd data info
    uint8             ussd_action  //corresponding to <m> parameter in unsolicited +CUSD
)
{
    uint16  ussd_str_len    = 0;
    uint8   *ussd_str_ptr   = PNULL;
    uint8     *default_buffer_ptr = NULL;
    uint16    default_str_len = 0;

    SCI_ASSERT(PNULL != atc_config_ptr);/*assert verified: check null pointer*/

    /*+CUSD: <m>[,<str>,<dcs>]*/
    /*<m> parameter:
        0:   //no further user action required
        1:   //further user action required
        2:   //USSD terminated by network
        3:   //other local client has responded
        4:   //operation not supported
        5:   //network time out
    */

    if(PNULL == ussd_ptr || 0 == ussd_ptr->str_len)
    {
        /*no <str> parameter*/
        sprintf((char *)g_rsp_str, "%s: %d", g_atc_info_table[AT_CMD_CUSD].cmd_name->str_ptr, ussd_action);
    }
    else
    {
        SCI_ASSERT(ussd_ptr->str_len <= MN_SS_MAX_USSD_STRING_LENGTH_BEFORE_ENCODE);/*assert verified: check valid value*/

        ussd_str_ptr = (uint8 *)SCI_ALLOC_BASEZ(ATC_USSD_MAX_STRING_LENGTH * sizeof(uint8));
        if (ussd_str_ptr == PNULL)
        {
            ATC_TRACE_LOW("ATC: ATC_UpCUSDInfo  no memory available");
            return;
        }

        switch(ussd_ptr->data_code_scheme)
        {
            case MN_SS_DEFAULT_DCS_VALUE:     //default alphabet

                default_str_len = ( ussd_ptr->str_len * 8) / 7;
                default_str_len += 1;    //the last character stores '\0'
                default_buffer_ptr = (uint8 *)SCI_ALLOC_BASE(default_str_len);
                SCI_ASSERT(NULL != default_buffer_ptr);/*assert verified: check null pointer*/
                SCI_MEMSET(default_buffer_ptr, 0, default_str_len);

                /*transform 7bit string to 8bit string*/
                if(SCI_SUCCESS != SCI_Unpack7bitsTo8bits(ussd_ptr->ussd_str, (int16)ussd_ptr->str_len, 0, default_buffer_ptr, (int16 *)&default_str_len))
                {
                    ATC_TRACE_LOW("ATC: ATC_UpCUSDInfo convert 7 bits to 8 bits wrong!");
                    break;
                }

                if(ATC_TransformIRAToOtherChset(default_buffer_ptr,
                                                default_str_len,
                                                (ATC_CHARACTER_SET_TYPE_E)atc_config_ptr->cscs[dual_sys],
                                                ATC_USSD_MAX_STRING_LENGTH,
                                                ussd_str_ptr,
                                                &ussd_str_len))
                {
                    sprintf((char *)g_rsp_str, "%s: %d,%s,15",
                            g_atc_info_table[AT_CMD_CUSD].cmd_name->str_ptr,
                            ussd_action, ussd_str_ptr);
                    ATC_TRACE_LOW((char *)g_rsp_str);
                }

                SCI_FREE(default_buffer_ptr);
                break;
                //USC2 aphabet
            case MN_SS_USC2_NO_CLASS_DCS:
                //Return USC2 string
                ATC_TranUint8IntoChar(ussd_ptr->ussd_str,
                                      ussd_ptr->str_len, ussd_str_ptr);

                sprintf((char *)g_rsp_str, "%s:%d,%s,%d",
                        g_atc_info_table[AT_CMD_CUSD].cmd_name->str_ptr, ussd_action, ussd_str_ptr, MN_SS_USC2_NO_CLASS_DCS);
                break;

            case MN_SS_8BIT_NO_CLASS_DCS:
                if(ATC_TransformIRAToOtherChset(ussd_ptr->ussd_str,
                                                ussd_ptr->str_len,
                                                (ATC_CHARACTER_SET_TYPE_E)atc_config_ptr->cscs[dual_sys],
                                                ATC_USSD_MAX_STRING_LENGTH,
                                                ussd_str_ptr,
                                                &ussd_str_len))
                {
                    sprintf((char *)g_rsp_str, "%s: %d,%s,15",
                            g_atc_info_table[AT_CMD_CUSD].cmd_name->str_ptr,
                            ussd_action, ussd_str_ptr);
                }
                break;

            default:
                break;
        }
        SCI_FREE(ussd_str_ptr);
    }

    ATC_BuildUnsolicitedInfo(dual_sys, atc_config_ptr, g_rsp_str);
}

/******************************************************************************/
// Description : This function is used to unsolicited report +SPERROR info to TE.
// Global resource dependence : None
// Author : 
// Note : None
/******************************************************************************/
void ATC_UpSPERRORInfo(
    MN_DUAL_SYS_E dual_sys,
    ATC_CONFIG_T    *atc_config_ptr,
    ATC_ERROR_ITEM_CLASS item_class, 
    uint32 cause
)
{
    if((MM_AUTH_FAIL_CLASS==item_class) || (PS_SECURITY_FAIL_CLASS==item_class)||
       (NW_CS_AUTH_REJECT_CLASS==item_class) || (NW_PS_AUTH_REJECT_CLASS==item_class))
    {
        sprintf((char *)g_rsp_str, "+SPERROR: %d",  item_class);
    }
    else
    {
        sprintf((char *)g_rsp_str, "+SPERROR: %d,%x",  item_class, cause);
    }

    ATC_BuildUnsolicitedInfo(dual_sys, atc_config_ptr, g_rsp_str);

    return;
}


/******************************************************************************/
// Description : This function reset dtmf info: include dtmf global variable and timer
// Global resource dependence : None
// Author : minqian.qian
// Note : None
/******************************************************************************/
LOCAL void ATC_ResetDTMFInfo(ATC_CONFIG_T *atc_config_ptr, MN_DUAL_SYS_E   dual_sys)
{
    ATC_ResetDTMFTimer();
    ATC_ResetDTMFVar(dual_sys);
}


/******************************************************************************/
// Description : This function constructs the AT command response.
// Global resource dependence : None
// Author : Ivan
// Note :
/******************************************************************************/
void ATC_BuildUnsolicitedInfo(
        MN_DUAL_SYS_E dual_sys,
        ATC_CONFIG_T   *atc_config_ptr,  // point to the ATC_CONFIG_T struct.
        uint8          *rsp_str_ptr      // Point the response string
        )
{
    SCI_ASSERT( NULL != atc_config_ptr );/*assert verified*/
    SCI_ASSERT( NULL != rsp_str_ptr );/*assert verified*/

    if(atc_config_ptr->urc_switch)
    {
         // Set the config values
        uint8      cr_val = atc_config_ptr->s3_reg;  // holds the atc_config_ptr->S3_reg value
        uint8      lf_val = atc_config_ptr->s4_reg;  // holds the atc_config_ptr->S4_reg value
        uint8      rsp_buf[10];
        
    #ifdef _MUX_ENABLE_
        uint8      tmp_link_id = atc_config_ptr->current_link_id;
        atc_config_ptr->current_link_id = ATC_GetMuxLinkIdFromSimId(dual_sys);
    #endif
   
        if (atc_config_ptr->q_value)
        {
            SCI_TRACE_LOW("ATC: UnslInfoRsp, q_value is TRUE");
            return;
        }

        if(0 == strlen( (char *)rsp_str_ptr ))
        {
            SCI_TRACE_LOW("ATC: UnslInfoRsp, rsp_str_ptr's length is zero");
            return;
        }

        if (atc_config_ptr->v_value)
        {
            sprintf((char*)rsp_buf, "%c%c",cr_val,lf_val);
            ATC_SendReslutRsp(atc_config_ptr, rsp_buf, strlen((char*)rsp_buf));
        }
#ifdef _MUX_ENABLE_
        ATC_TRACE_LOW("ATC: UnslInfoRsp link_id: %d, str: \n%s", 
            atc_config_ptr->current_link_id, (char *)rsp_str_ptr);
#else
        ATC_TRACE_LOW("ATC: UnslInfoRsp str: \n%s", (char *)rsp_str_ptr);
#endif
        ATC_SendReslutRsp(atc_config_ptr, rsp_str_ptr, strlen((char*)rsp_str_ptr));
        
        sprintf((char*)rsp_buf, "%c%c", cr_val, lf_val);
        ATC_SendReslutRsp(atc_config_ptr, rsp_buf, strlen((char*)rsp_buf));
        
    #ifdef _MUX_ENABLE_
        atc_config_ptr->current_link_id = tmp_link_id;
    #endif
    }
}

typedef struct ATCResponse
{
    uint8 *data;
    uint32 dataLen;
} ATCResponse;

typedef void (*ATCRESULTCB)(void);
static ATCResponse g_atcResponse[ATC_MAX_LINK_ID] = {0};
static ATCRESULTCB g_cb = NULL;

void ATC_RegisterCallback(ATCRESULTCB cb)
{
    g_cb = cb;
}

void ATC_SaveResultRsp(uint8 link_id, uint8 *data_ptr, uint32 data_length)
{
    if (g_cb == NULL) return;

    if (link_id < ATC_MAX_LINK_ID)
    {
        if (g_atcResponse[link_id].data == NULL)
        {
            g_atcResponse[link_id].data = malloc(data_length + 1);
            memset(g_atcResponse[link_id].data, 0x0, data_length + 1);
            memcpy(g_atcResponse[link_id].data, data_ptr, data_length);
        }
        else
        {
            //TODO:
        }

        g_cb();
    }
}

void ATC_GetResultRsp(uint8 link_id, uint8 **data_ptr, uint32 *data_length)
{
    if (link_id < ATC_MAX_LINK_ID)
    {
        if (g_atcResponse[link_id].data != NULL)
        {
            *data_ptr = g_atcResponse[link_id].data;
            *data_length = g_atcResponse[link_id].dataLen;
            g_atcResponse[link_id].data = NULL;
            g_atcResponse[link_id].dataLen = 0;
        }
        else
        {
            *data_ptr = NULL;
            *data_length = 0;
        }
    }
}

/******************************************************************************/
// Description : This function send the AT command response.
// Global resource dependence : None
// Author : Elvis.xu
// Note :
/******************************************************************************/
void ATC_SendReslutRsp(
                 ATC_CONFIG_T   *atc_config_ptr,  // point to the ATC_CONFIG_T struct.
                 uint8  *data_ptr,                // point to the response string
                 uint32 data_length               // length of response string
                 )
{
    ATC_SaveResultRsp(atc_config_ptr->current_link_id, data_ptr, data_length);
#ifdef _MUX_ENABLE_
    if(ATC_DEBUG_LINK == atc_config_ptr->current_link_id)
    {
        SIO_ATC_WriteCmdRes(data_ptr, data_length);
    }
    else
    {
        MUX_Write(data_ptr, data_length, atc_config_ptr->current_link_id);
    }
#else
    SIO_ATC_WriteCmdRes(data_ptr, data_length);
#endif
}

/******************************************************************************/
// Description : This function send the asynchronous information.
// Global resource dependence : None
// Author : Elvis.xu
// Note :
/******************************************************************************/
void ATC_SendAsyncInfo(
                 ATC_CONFIG_T   *atc_config_ptr,  // point to the ATC_CONFIG_T struct.
                 uint8 ctlz_end_status
                 )
{
    uint8      data_ptr[20]={0};
    uint8      data_length = 0;
#ifdef _MUX_ENABLE_
    uint8  atc_link_id = atc_config_ptr->current_link_id;

    atc_config_ptr->atc_link_config[atc_link_id].is_ctrlz_end = TRUE;
    atc_config_ptr->atc_link_config[atc_link_id].ctrlz_end_status = ctlz_end_status;
    sprintf((char*)data_ptr, "%c%c> ", atc_config_ptr->s3_reg, atc_config_ptr->s4_reg);

    data_length = strlen((char*)data_ptr);
    if(atc_link_id == ATC_DEBUG_LINK)
    {
        SCI_TRACE_LOW("ATC: send > Async Info to SIO");
        SIO_ATC_SetCmdLineTerminateChar(CTRL_Z, ESC);      
        SIO_ATC_WriteCmdRes(data_ptr, data_length);
    }
    else
    {
        SCI_TRACE_LOW("ATC: send Async Info with current_link_id is %d", atc_link_id);
        // set the terminate char to receive PDU from the SIO
        ATC_SetCmdLineTerminateChar(atc_link_id, CTRL_Z, ESC);
        MUX_Write(data_ptr, data_length, atc_link_id);
    }
    
#else
    SCI_TRACE_LOW("ATC: send > Async Info to SIO");
    sprintf((char*)data_ptr, "%c%c> ", atc_config_ptr->s3_reg, atc_config_ptr->s4_reg);

    data_length = strlen((char*)data_ptr);
    // set the terminate char to receive PDU from the SIO
    SIO_ATC_SetCmdLineTerminateChar(CTRL_Z, ESC);
    atc_config_ptr->is_ctrlz_end = TRUE;
    atc_config_ptr->ctrlz_end_status = ctlz_end_status;
    SIO_ATC_WriteCmdRes(data_ptr, data_length);
#endif
}


/******************************************************************************/
// Description :  This function is used to start PS when power on and ATC_SAT_ENABLE 
//                not defined
// Global resource dependence : None
// Author : yaxun.chen
// Note : None
/******************************************************************************/
LOCAL void ATC_InitAutoStartPSNoSat(void)
{
    POWER_RESTART_CONDITION_E      start_condition;
#ifdef _MUX_ENABLE_
    uint8 i = 0;
#endif
    start_condition = POWER_GetRestartCondition();
    if ((RESTART_BY_ALARM != start_condition 
        && RESTART_BY_CHARGE != start_condition)
        || 1 == CLASSMARK_GetModelType() 
       )
    {
#ifdef _ATC_ONLY 
    #ifdef _MUX_ENABLE_
        for (i = MN_DUAL_SYS_1; i < MN_SYS_NUMBER; i++)
        {
            MNPHONE_PowerOnSimEx((MN_DUAL_SYS_E)i, TRUE);
        }
        //MUX_IsNeedToActivePS return TRUE if MS runs in cal_pos mode.
        if(RM_GetCalibrationPostMode())
        {
            MNPHONE_PowerOnPsEx(MN_DUAL_SYS_1);
        }
        else
        {
            for (i = MN_DUAL_SYS_1; i < MN_SYS_NUMBER; i++)
            {
                MNPHONE_PowerOnPsEx((MN_DUAL_SYS_E)i);
            }
        }
    #else
            MNPHONE_StartupPs(MN_GMMREG_RAT_GPRS);
   #endif
#endif   

    }
}



#ifdef ATC_SAT_ENABLE
#ifdef _MUX_ENABLE_
/*****************************************************************************/
//  Description : For AT+IPSEND, 
//  when inputing given length data, appending 0x1a to send out
//  Global resource dependence : none
//  Author: longting.zhao
//  Note: 
/*****************************************************************************/
BOOLEAN ATC_CheckIpSendDataByLen(uint8 * nchar, uint8 link_id)
{
    MN_DUAL_SYS_E dual_sys = ATC_GetSimIdFromLinkId(link_id);
    uint8 status = FALSE;

    SCI_TRACE_LOW("ATC: ATC_ProcessSendDataByLen");

    // if back space
    if (*nchar == atc_bs_ch[link_id])
    {
        if (atc_buffer[dual_sys].end_point != atc_buffer[dual_sys].start_point)
        {
            if (0 == atc_buffer[dual_sys].end_point)
            {
                atc_buffer[dual_sys].end_point = atc_buffer[dual_sys].size - 1;
            }
            else
            {
                atc_buffer[dual_sys].end_point--;
            }
            
            g_cur_atc_cmd_size[dual_sys]--;
        }

        status = TRUE; // escape current course

    }
    else
    {
        if(g_cur_atc_cmd_size[dual_sys] + 1 == g_auto_send_len)
        {
            if ( -1 == ATC_Ring_Add(&atc_buffer[dual_sys], *nchar)) 
            {
                ATC_ClrCmdLine(dual_sys);
                status = TRUE; //escape current course
            }
            else
            {
                g_cur_atc_cmd_size[dual_sys]++;
                *nchar = CTRL_Z; // set end character ctrl-z
            }
        }
    }
    
    return status;
}
#endif /*_MUX_ENABLE_*/
#endif

