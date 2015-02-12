/******************************************************************************
 ** File Name:      atc_basic_cmd.h                                           *
 ** Author:         Xueli.lv                                                  *
 ** DATE:           20/08/2002                                                *
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
 ** ------------------------------------------------------------------------- *
 ** DATE           NAME             DESCRIPTION                               *
 ** 20/08/2002     Xueli.lv         Create.                                   *
 ** 15/07/2003     Liuhan           Modify.                                   *
 ******************************************************************************/

#ifndef _ATC_BASIC_CMD_H
#define _ATC_BASIC_CMD_H

#include "atc.h"
#include "stdio.h"
/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
}
#endif
/******************************* macro variables*******************************/
#define AT_SIDE_TONE0                    0
#define AT_SIDE_TONE1                    1
#define AT_SIDE_TONE2                    2
#define AT_SIDE_TONE3                    3
#define AT_SIDE_TONE4                    4
#define AT_SIDE_TONE5                    5
#define AT_SIDE_TONE6                    6
#define AT_SIDE_TONE7                    7
#define AT_SIDE_TONE8                    8
#define AT_SIDE_TONE9                    9
#define AT_SIDE_TONEA                   10
#define AT_SIDE_TONEF                   15

#define AT_CCLK_STR_LEN                 17

// The following macros define the supported baudrate
#define AT_BAUD_1200                  1200
#define AT_BAUD_2400                  2400
#define AT_BAUD_4800                  4800
#define AT_BAUD_9600                  9600
#define AT_BAUD_19200                19200
#define AT_BAUD_38400                38400
#define AT_BAUD_57600                57600
#define AT_BAUD_115200              115200
#define AT_BAUD_230400              230400
#define AT_BAUD_460800              460800

// define the max value for time yy/mm/dd,hh:mm:ss
#define AT_MAX_MONTH                    12
#define AT_MAX_DAY                      31
#define AT_MAX_HOUR                     23
#define AT_MAX_MINUTE                   59
#define AT_MAX_SECOND                   59

// for AT+CBC command
#define AT_CBC_NO_CHARGE                 0
#define AT_CBC_CHARGE                    1
#define AT_CBC_NO                        2

// The uint duration 100ms
#define AT_SPEAKER_UNIT_DURATION       100
#define AT_SPEAKER_MAX_GAIN             15
#define AT_DEFAULT_WDTMF_GAIN            9

#define AT_MAX_SIDET_LEV                 5
#define AT_INVALID_RXQUAL               99

#define AT_CLCK_ALL_BAR_SVR      0X00
#define AT_CLCK_BAR_OUTGOING     0X01
#define AT_CLCK_BAOC             0X02
#define AT_CLCK_BOIC             0X03
#define AT_CLCK_BOIC_EXHC        0X04
#define AT_CLCK_BAR_INCOMING     0X05
#define AT_CLCK_BAIC             0X06
#define AT_CLCK_BICROAM          0X07

#define AT_MIX_MODE              1   // The current mode is mix mode
#define AT_AT_MODE               0   // The current mode is AT mode

// for the battery charge
#define AT_STOP_CHR        0
#define AT_START_CHR       1
#define AT_GET_BATT_VOL    2
#define AT_SET_BATT_PARM   3

#define AT_IPR_MAX         10
#define ATC_POWEROFF_INTERVAL    5*1000   // 5 seconds

#define AT_SPEAKER_GAIN_LEVEL    6

#define ATC_SNVM_NUM            0
#define ATC_SNVM_STRING         1

#define ATC_REF_PARAMETER       1
#define ATC_IMEI                2
//#define ATC_LOG_SWITCH          3
//#define ATC_DEEP_SLEEP          4

//for GPIO define
#define AT_GPIO_SET_DIRECTION    0
#define AT_GPIO_GET_DIRECTION    1
#define AT_GPIO_SET_VALUE        0
#define AT_GPIO_GET_VALUE        1

#define ATC_SPEAKER              1
#define ATC_PIN1                 0
#define ATC_PIN2                 1
#define ATC_PUK1                 2
#define ATC_PUK2                 3

#define AT_STOP_CHR        0
#define AT_START_CHR       1
#define AT_GET_BATT_VOL    2
#define AT_SET_BATT_PARM   3


#define AT_AUD_VOL_DEFAULT_VAL                        4 //3
#define AT_AUD_VOL_SPEAKER_MAX                        9 //7

#define AT_AUD_VOL_SIDET_TONE_MAX                    32768
#define AT_AUD_VOL_DIGITAL_HANDSET_UP_MAX            65535
#define AT_AUD_VOL_DIGITAL_HANDSET_DOWN_MAX          65535
#define AT_AUD_VOL_DIGITAL_HANDFREE_UP_MAX           65535
#define AT_AUD_VOL_DIGITAL_HANDFREE_DOWN_MAX         65535
#define AT_AUD_VOL_PGA_HANDSET_UP_MAX                11
#define AT_AUD_VOL_PGA_HANDSET_DOWN_MAX              11
#define AT_AUD_VOL_PGA_HANDFREE_UP_MAX               11
#define AT_AUD_VOL_PGA_HANDFREE_DOWN_MAX             11

#define AT_AUD_VOL_HANDSET_UL_EQ_INDEX               32
#define AT_AUD_VOL_HANDSET_UL_EQ_MAX                 65535
#define AT_AUD_VOL_HANDFREE_UL_EQ_INDEX              32
#define AT_AUD_VOL_HANDFREE_UL_EQ_MAX                65535
#define AT_AUD_VOL_HANDSET_DL_EQ_INDEX               32
#define AT_AUD_VOL_HANDSET_DL_EQ_MAX                 65535
#define AT_AUD_VOL_HANDFREE_DL_EQ_INDEX              32
#define AT_AUD_VOL_HANDFREE_DL_EQ_MAX                65535
#define AT_AUD_ECHO_TEST_MAX                         1
#define AT_AUD_ECHO_TEST_FREQ_MIN                    250
#define AT_AUD_ECHO_TEST_FREQ_MAX                    3400
#define AT_AUD_VOL_HANDSET_DIGITAL_SCALE_UP_MAX      65535
#define AT_AUD_VOL_HANDSET_DIGITAL_SCALE_DOWN_MAX    65535
#define AT_AUD_VOL_HANDFREE_DIGITAL_SCALE_UP_MAX     65535
#define AT_AUD_VOL_HANDFREE_DIGITAL_SCALE_DOWN_MAX   65535

//for managing the file
#define ATC_SNVM_READ            0
#define ATC_SNVM_ADD             1
#define ATC_SNVM_REPLACE         2
#define ATC_SNVM_GETSIZE         3
#define ATC_SNVM_DELETE          4

#define MAX_SIM_DATALEN 255*2

#define MN_MAX_CRSM_PATHID_LEN 8


/*******************************************************************************
 **                              Type declaration                              *
 ******************************************************************************/
typedef enum
{
    ATC_CREG_DISABLE,        // disable network registration unsolicited result code
    ATC_CREG_ENABLE,         // enable network registration unsolicited result code +CREG: <stat>
    ATC_CREG_AND_INFO        // enable network registration and location information unsolicited result code +CREG:<stat>,[<lac>,<ci>]
} ATC_CREG_TYPE_E;

typedef enum
{
    ATC_NO_OPERATE,
    ATC_RESET_ACM_UNIT,
    ATC_SET_ACM_MAX
} ATC_AOC_OPERATE_E;
typedef enum
{
    ATC_NO_SERVICE = 0,
    RESERVED_1 = 1,
    RESERVED_2 = 2,
    ATC_GSM_GPRS = 3,
    ATC_TD = 4
} ATC_SYS_MODE_E;

typedef enum
{
    ATC_NV_OS_PARAMETER = 0x15,
    ATC_NV_REF_PARAMETER = 0x16,
    ATC_NVM_PARAM_TYPE_PREV_UMTS_RA_CAPABILITY = 0x3b,
    ATC_NVM_PARAM_TYPE_EM_CFG = 0x49
} ATC_NVM_PARAM_E;

// audio mode operation type for AT+SADM command
typedef enum
{
    ATC_GET_AUD_MODE_NUM = 0,        //  0  :get audio mode total number
    ATC_GET_AUD_MODE_NAME,           //  1  :get audio mode name
    ATC_GET_AUD_MODE_DATA,           //  2  :get audio data from memory
    ATC_SET_AUD_MODE_DATA,           //  3  :set audio data into memory
    ATC_SET_AUD_DEV_MODE,            //  4  :set audio mode
    ATC_SET_AUD_MODE_NV_DATA,        //  5  :set audio mode parameters into flash
    ATC_GET_AUD_MODE_NV_DATA,        //  6  :get audio mode parameters from flash
    ATC_GET_AUD_ARM_MODE_NUM,        //  7  :get audio mode total number
    ATC_GET_AUD_ARM_MODE_NAME,       //  8  :get audio mode name
    ATC_GET_AUD_ARM_MODE_DATA,       //  9  :get audio data from memory
    ATC_SET_AUD_ARM_MODE_DATA,       //  10  :set audio data into memory
    ATC_SET_AUD_ARM_MODE_NV_DATA,    //  11  :set audio mode parameters into flash
    ATC_GET_AUD_ARM_MODE_NV_DATA     //  12  :get audio mode parameters from flash
} ATC_AUD_MODE_OPT_TYPE_E;


typedef enum
{
    ATC_POWER_OFF_SIM_PS = 0,
    ATC_POWER_ON_SIM_ONLY,
    ATC_POWER_ON_SIM_NORMAL,
    ATC_POWER_OFF_SIM,
    ATC_POWER_ON_PS,
    ATC_POWER_OFF_PS
} ATC_SFUN_TYPE_E;

typedef enum
{
    ATC_FUN_DEACT_SIM_PS = 0,
    ATC_FUN_ACT_SIM_PS,
    ATC_FUN_ACT_SIM,
    ATC_FUN_DEACT_SIM,
    ATC_FUN_ACT_PS,
    ATC_FUN_DEACT_PS
} ATC_FUN_STATUS_E;



#ifndef ATC_SAT_ENABLE
typedef enum
{
    ATC_GET_ENHA_MODE_NUM = 0,      //  0  :get audio enha mode total number
    ATC_GET_ENHA_MODE_DATA,         //  1  :get audio enha data from memory
    ATC_SET_ENHA_MODE_DATA,         //  2  :set audio enha data into memory
    ATC_GET_ENHA_MODE_NV_DATA,      //  3  :get audio enha mode parameters from flash
    ATC_SET_ENHA_MODE_NV_DATA       //  4  :set audio enha mode parameters into flash
}ATC_ENHA_MODE_OPT_TYPE_E;
#endif

typedef enum
{
    ATC_GET_LEVE_MODE_NUM = 0,       //  0  :get audio mode total number
    ATC_GET_LEVE_MODE_NAME,          //  1  :get audio mode name
    ATC_GET_LEVE_MODE_DATA,          //  2  :get audio data from memory
    ATC_SET_LEVE_MODE_DATA,           //  3  :set audio mode
    ATC_GET_LEVE_MODE_NV_DATA,       //  4  :get audio mode parameters from flash
    ATC_SET_LEVE_MODE_NV_DATA,       //  5  :set audio mode parameters into flash
} ATC_LEVE_MODE_OPT_TYPE_E;

typedef enum
{
    ATC_IDLE ,
    ATC_TRAFFIC,
    ATC_BOTUM
} ATC_RRCELL_STATUS;

#ifdef ATC_SAT_ENABLE
typedef struct firboot_seed_nv_tag
{
    uint8 seed[20];
}FIRBOOT_SEED_NV_T;
#endif

typedef enum _ATC_SERVICE_STATE_E
{
    ATC_SEVICE_NONE,  // not support all service
    ATC_SERVICE_EMC,     // only support emergency call
    ATC_SERVICE_CS_EMC,  // support: normal cs service && emergency call
    ATC_SERVICE_CS,      // support: normal cs service, not support emergency call
    ATC_SERVICE_PS_EMC,  // support: normal ps service && emergency call
    ATC_SERVICE_PS,      // support: normal ps service
    ATC_SERVICE_CS_PS_EMC, // support all service. cs+ps+emergency call
    ATC_SERVICE_CS_PS    // support: cs+ps, not support emergency call
}ATC_SERVICE_STATE_E;

/*****************************************************************************/
//  Description : This function initializes the static variables
//  Global resource dependence :
//  Author:       hanjun.liu
//  Note:
/*****************************************************************************/
extern void ATC_InitBasicVar(void);

/*****************************************************************************/
//  Description : This function handle the ATAMS command
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessAMS(
    ATC_CONFIG_T         *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the ATAMSOTA command
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessAMSOTA(
    ATC_CONFIG_T         *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the AMSINSTALL command
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessAMSINSTALL(
    ATC_CONFIG_T         *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the ATAMSDL command
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessAMSDL(
    ATC_CONFIG_T         *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the ATAMSDELETE command
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessAMSDELETE(
    ATC_CONFIG_T         *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the ATAMSDELETEALL command
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessAMSDELETEALL(
    ATC_CONFIG_T         *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the ATAMSRUN command
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessAMSRUN(
    ATC_CONFIG_T         *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the ATAMSLIST command
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessAMSLIST(
    ATC_CONFIG_T         *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the ATAMSDESTROY command
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessAMSDESTROY(
    ATC_CONFIG_T         *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the ATAMSSTATUS command
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessAMSSTATUS(
    ATC_CONFIG_T         *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the ATAMSRESET command
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessAMSRESET(
    ATC_CONFIG_T         *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the ATAMSCFGURL command
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessAMSCFGURL(
    ATC_CONFIG_T         *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the ATAMSCFGACCOUNT command
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessAMSCFGACCOUNT(
    ATC_CONFIG_T         *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the ATAMSCFGINIT command
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessAMSCFGINIT(
    ATC_CONFIG_T         *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the ATAMSCFGCANCEL command
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessAMSCFGCANCEL(
    ATC_CONFIG_T         *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the AMSCFGCANCELALL command
//  Global resource dependence : none
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessAMSCFGCANCELALL(
    ATC_CONFIG_T         *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the ATQ command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessQ(
    ATC_CONFIG_T         *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the ATV command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessV(
    ATC_CONFIG_T         *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the ATV command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessCMEE(
    ATC_CONFIG_T         *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);



/*****************************************************************************/
//  Description : This function handle the AT+CREG command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessCREG(
    ATC_CONFIG_T        *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the AT+CR command
//                set if its GPRS type before show connect
//  Global resource dependence : none
//  Author:       xu.xiaogang
//  Note:
//  History:
//         xu.xiaogang 2003-9-26 create
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessCR(
    ATC_CONFIG_T        *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the AT+CRC command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessCRC(
    ATC_CONFIG_T        *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);



/*****************************************************************************/
//  Description : This function handle the ATS0 command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessS0(
    ATC_CONFIG_T        *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);
/*****************************************************************************/
//  Description : This function handle the ATS2 command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessS2(
    ATC_CONFIG_T        *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the ATS3 command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessS3(
    ATC_CONFIG_T        *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the ATS4 command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessS4(
    ATC_CONFIG_T        *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the ATS5 command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessS5(
    ATC_CONFIG_T        *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the ATS6 command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessS6(
    ATC_CONFIG_T        *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the ATS7 command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessS7(
    ATC_CONFIG_T        *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the ATS8 command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessS8(
    ATC_CONFIG_T        *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the ATS10 command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessS10(
    ATC_CONFIG_T        *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the AT+CSQ command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessCSQ(
    ATC_CONFIG_T        *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the AT+ESQOPT command
//  Global resource dependence : g_rsp_str
//  Author:      sunny.shen
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessESQOPT(
    ATC_CONFIG_T        *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the AT+CGMI command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessCGMI(
    ATC_CONFIG_T        *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the AT+CGSN command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessCGSN(
    ATC_CONFIG_T        *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the AT+CGMM command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessCGMM(
    ATC_CONFIG_T        *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the AT+CGMR command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessCGMR(
    ATC_CONFIG_T        *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/*****************************************************************************/
//  Description : This function handle the APP_MN_NETWORK_STATUS_IND signal and
//                send the unsolicited result code
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessNetworkStatusInd(
    ATC_CONFIG_T                  *atc_config_ptr,
    APP_MN_NETWORK_STATUS_IND_T   *sig_ptr
);

/*****************************************************************************/
//  Description : This function handle the APP_MN_NETWORK_INFO_IND signal and
//                send the unsolicited result code
//  Global resource dependence : none
//  Author:       Tianming Wu
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessNetworkInfoInd(
    ATC_CONFIG_T                  *atc_config_ptr,
    APP_MN_NETWORK_INFO_IND_T     *sig_ptr
);

/*****************************************************************************/
//  Description : This function handle the AT+CSCS command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCSCS);

/*****************************************************************************/
//  Description : This function handle the AT+CPAS command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCPAS);

/*****************************************************************************/
//  Description : This function handle the AT+CPIN command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCPIN);

#ifndef ATC_SAT_ENABLE
/*****************************************************************************/
//  Description : This function handle the AT+ECPIN2 command
//  Global resource dependence : none
//  Author:       Bin Li
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessECPIN2);

AT_CMD_FUNC(ATC_ProcessX);
#endif

/*****************************************************************************/
//  Description : This function handles the APP_MN_AOC_INCREASE_ACM_IND signal
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessIncreaseAcmInd(
    ATC_CONFIG_T                   *atc_config_ptr,
    APP_MN_AOC_MODIFIED_IND_T      *sig_ptr
);
/*****************************************************************************/
//  Description : This function handle the AT command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessAT);

/*****************************************************************************/
//  Description : This function handle the ATE command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessE);

/*****************************************************************************/
//  Description : This function handle the ATE command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessZ);

/*****************************************************************************/
//  Description : This function handle the AT&F command
//  Global resource dependence : none
//  Author:       Ivan.yin
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_Processand_F);

/*****************************************************************************/
//  Description : This function handle the AT&D command
//  Global resource dependence : none
//  Author:       Ivan.yin
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_Processand_D);

/*****************************************************************************/
//  Description : This function handle the AT&C command
//  Global resource dependence : none
//  Author:       Ivan.yin
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_Processand_C);

/*****************************************************************************/
//  Description : This function handle the ATW command
//  Global resource dependence : none
//  Author:       Ivan.yin
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessW);

/*****************************************************************************/
//  Description : This function handle the AT+XX command
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessXX);

/*****************************************************************************/
//  Description : This function handle the AT+CCID command
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCCID);
/*****************************************************************************/
//  Description : This function handles the AT+ECIND command
//  Global resource dependence : g_rsp_str
//  Author:       binli
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessECIND);
/*****************************************************************************/
//  Description : This function handle the AT+CCLK command
//  Global resource dependence : none
//  Author:       xueli.lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCCLK);
/*****************************************************************************/
//  Description : This function handle the AT+VGT command
//  Global resource dependence : none
//  Author:       Xueli.lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessVGT);
/*****************************************************************************/
//  Description : This function handle the AT+VGR command
//  Global resource dependence : none
//  Author:       Xueli.lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessVGR);
/*****************************************************************************/
//  Description : This function handle the AT+SPEAKER command
//                 AT+SVGT=<value>,<value>
//                        <value>:0 -- 1
//  Global resource dependence : none
//  Author:       Shijun.cui
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSPEAKER);
/*****************************************************************************/
//  Description : This function handle the AT+SEQT command
//                 AT+SEQT=<value>
//                         <value>:0 - 3
//  Global resource dependence : g_rsp_str
//  Author:       Shijun.cui
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSEQT);
/*****************************************************************************/
//  Description : This function handle the AT+SSEA command
//                 AT+SVGT=<value>
//                        <value>:0 -- 8
//  Global resource dependence : none
//  Author:       Xueli.lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSSEA);

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
);

/*****************************************************************************/
//  Description : This function handle the AT+CFUN command
//  Global resource dependence : none
//  Author:       Ivan.Yin
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCFUN);

/*****************************************************************************/
//  Description : This function handle the AT+IPR command
//  Global resource dependence : none
//  Author:       Ivan.Yin
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessIPR);
/*****************************************************************************/
//  Description : This function handle the AT+CIND command
//  Global resource dependence : none
//  Author:       Ivan.Yin
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCIND);

/*****************************************************************************/
//  Description : This function handle the AT+CCED command
//  Global resource dependence : none
//  Author:       Ivan.Yin
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCCED);

/*****************************************************************************/
//  Description : This function handles the APP_MN_SIM_NOT_READY_IND signal
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessNotReadyInd(
    ATC_CONFIG_T                    *atc_config_ptr,
    APP_MN_SIM_NOT_READY_IND_T      *sig_ptr
);

/*****************************************************************************/
//  Description : This function handle the AT## command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSHARP_SHARP);

/*****************************************************************************/
//  Description : This function handle the AT+WTONE command
//  Global resource dependence : none
//  Author:       Xueli.lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSTONE);

/*****************************************************************************/
//  Description : This function handle the AT+ASSERT command
//  Global resource dependence : none
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessASSERT);

/*****************************************************************************/
//  Description : This function handles the AT+WIND command
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:         Modify by Liuhan
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSIND);

/*****************************************************************************/
//  Description : This function handle the ATS95 command
//  Global resource dependence : none
//  Author:       Ivan.Yin
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessTHREE_PERCENT);

/*****************************************************************************/
//  Description : This function handleS the AT&W command
//              AT&W is used to save the current configuration to the nv
//  Global resource dependence : none
//  Author:       Xueli.lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_Processand_W);

/*****************************************************************************/
//  Description : This function handles the AT+WDTMF command
//              AT+WDTMF is used to play the DTMF tone on the current speaker
//               AT+WDTMF=<mode>,<dtmf>,<gain>,<duartion>
//  Global resource dependence : none
//  Author:       Xueli.lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSDTMF);

/*****************************************************************************/
//  Description : This function handle the AT+SPEAKER command
//  Global resource dependence : none
//  Author:  Xueli.lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSSAM);

/*****************************************************************************/
//  Description : This function handle the AT+SSAP command
//  Global resource dependence : none
//  Author:  Shijun.cui
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSSAP);

/*****************************************************************************/
//  Description : This function handle the AT+SADM command
//  Global resource dependence : none
//  Author:  Shijun.cui
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSADM);

/*****************************************************************************/
//  Description : This function handle the AT+CICB command
//  this command is used to set the type of incoming calls
//               AT+CICB=<mode>
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCICB);

/*****************************************************************************/
//  Description : This function handle the AT+ECHO command
//  Global resource dependence : none
//  Author:     Xueli.lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessECHO);

/*****************************************************************************/
//  Description : This function handles the AT+CRSL command.This command is used
//                to set or get the sound level of the ringer on incoming call.
//  AT+CRSL=<sound level>
//  Global resource dependence : none
//  Author:       Xueli.lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCRSL);

/*****************************************************************************/
//  Description : This function handles the AT+CMUT command.This command is used
//                to mute the micrphone input.
//  AT+CRSL=<sound level>
//  Global resource dependence : none
//  Author:       Xueli.lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCMUT);

/*****************************************************************************/
//  Description : This function handles the AT+CIMI command. This command is used
//                to get the IMSI of the SIM card.
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCIMI);

/*****************************************************************************/
//  Description : This function handleS the AT+CPOF command. This command is used
//  to power off the module
//  AT+CPOF
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCPOF);

#ifndef ATC_SAT_ENABLE
/*****************************************************************************/
//  Description : This function handle the AT&K command
//  Global resource dependence : none
//  Author:       Ivan.yin
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_Processand_K);

/*****************************************************************************/
//  Description : This function handle the ATS95 command
//  Global resource dependence : none
//  Author:       Ivan.yin
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessS95);

/*****************************************************************************/
//  Description : This function handleS the AT+SMUX command. This command is used
//  to active or deactive the multiplex mode(data/commands)
//  AT+SMUX=<mode>  0: deactive 1: active
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSMUX);

/*****************************************************************************/
//  Description : This function handleS the AT+SBCM command. This command is used
//  to manage battery charge.
//  AT+SBCM=<mode>,<chargeind>,<battLevMax>,<battLevMin>,<TPulseInCharge>,<TPulseOutCharge>,
//  <BattIntRes>
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSBCM);

/*****************************************************************************/
//  Description : This function handle the AT+CBC command
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCBC);

/*****************************************************************************/
//  Description : This function handles the AT^DSCI command
//  Global resource dependence : g_rsp_str
//  Author:       yi.jin
/*****************************************************************************/
AT_CMD_FUNC(ATC_Processxor_DSCI);

/*****************************************************************************/
//  Description : This function handles the AT_CLS command
//  Global resource dependence : 
//  Author:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSHARP_CLS);

/*****************************************************************************/
//  Description : This function handles the AT+GMM command
//  Global resource dependence : 
//  Author:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessGMM);

/*****************************************************************************/
//  Description : This function handles the AT+CGI command
//  Global resource dependence : 
//  Author:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessGCI);

/*****************************************************************************/
//  Description : This function handles the AT+I1 command
//  Global resource dependence : 
//  Author:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessI1);

/*****************************************************************************/
//  Description : This function handles the AT+I2 command
//  Global resource dependence : 
//  Author:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessI2);

/*****************************************************************************/
//  Description : This function handles the AT+I3 command
//  Global resource dependence : 
//  Author:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessI3);

/*****************************************************************************/
//  Description : This function handles the AT+I4 command
//  Global resource dependence : 
//  Author:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessI4);

/*****************************************************************************/
//  Description : This function handles the AT+I5 command
//  Global resource dependence : 
//  Author:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessI5);

/*****************************************************************************/
//  Description : This function handles the AT+I6 command
//  Global resource dependence : 
//  Author:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessI6);

/*****************************************************************************/
//  Description : This function handles the AT+I7 command
//  Global resource dependence : 
//  Author:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessI7);

/*****************************************************************************/
//  Description : check whether a call present or not
//  Global resource dependence : g_calls_state
//  Author:  Elvis.xu
//  Note:
/*****************************************************************************/
BOOLEAN ATC_IsVoiceConnectPresent(MN_DUAL_SYS_E dual_sys);

/*****************************************************************************/
//  Description : check whether a data call present or not
//  Global resource dependence : 
//  Author:  Elvis.xu
//  Note:
/*****************************************************************************/
BOOLEAN ATC_IsDataConnectPresent(MN_DUAL_SYS_E dual_sys);

/*****************************************************************************/
//  Description : This function handle the ATO command to recover data mode
//  Global resource dependence : g_rsp_str
//  Author:  Elvis.xu
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessO);

/*****************************************************************************/
//  Description : This function handle the AT+FCLASS command
//  Global resource dependence : g_rsp_str
//  Author:  Elvis.xu
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessFCLASS);

/*****************************************************************************/
//  Description : This function handle the ATM command
//  Global resource dependence : none
//  Author:       minqian.qian
//  Note:      Monitor speaker mode(ITU V.25 (subclause: 6.3.14))
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessM);

/*****************************************************************************/
//  Description : This function handle the ATM command
//  Global resource dependence : none
//  Author:
//  Note:     modify the related nvram
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessNVOPERATE);

/******************************************************************************/
//    DESCRIPTION:
//           get wifi/bluetooth mac address 
//    Global: atc_config_ptr
//    AUTHOR: Winnie.zhu
//    Note :
//    History:
/******************************************************************************/
ATC_STATUS ATC_ProcessSPHAR( // return the handle result
    ATC_CONFIG_T    *atc_config_ptr, // point to the ATC_CONFIG_T struct.
    ATC_CUR_INFO_LIST_T *cur_cmd_info // point to the atc cmd info
);

/******************************************************************************/
//  Description : This function handles the AT+CTZR command.This command is used
//                to set or get time and time zone report config information .
//  Global resource dependence : g_rsp_str
//  Author:
/******************************************************************************/
AT_CMD_FUNC( ATC_ProcessCTZR);

/*****************************************************************************/
//  Description : This function handle the AT+SPENHA command 
//  Global resource dependence : g_rsp_str
//  Author:
//  Note:
/*****************************************************************************/
AT_CMD_FUNC( ATC_ProcessSPENHA );

/*****************************************************************************/
//Description : This function handles ^CURC
//Global resource dependence : g_rsp_str
//Author:
//Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_Processxor_CURC);

/*****************************************************************************/
//Description : This function handles AT+SSRVT, set ms to service type
//Global resource dependence : g_rsp_str
//Author:
//Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSSRVT);

/*****************************************************************************/
//  Description : This function handles AT+SSCR for atc audio record
//Global resource dependence : atc_record_codec
//Author:
//Note:
/*****************************************************************************/
AT_CMD_FUNC( ATC_ProcessSSCR );

/*****************************************************************************/
//  Description : This function handles the data send from the SIO when AT+SRWD
//  Global resource dependence : g_rsp_str
//  Author:       hanjun.liu
//  Note:
/*****************************************************************************/
PUBLIC uint32 ATC_ProcessSioSRWDData(
    ATC_CONFIG_T    *atc_config_ptr,
    uint8           *data_ptr  // pointer to the data
);


/*****************************************************************************/
//  Description : This function handles the AT+SRWD command, This command is used
//  to manage the NV items.
//  Global resource dependence : g_rsp_str
//  Author:       hanjun.liu
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSRWD);

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
);


/*****************************************************************************/
//  Description : This function handles the AT+SSGF command.This command is used
//  to set or get GPIO function.
//  Global resource dependence : g_rsp_str
//  Author:       Shijun.cui
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSSGF);

/*****************************************************************************/
//  Description : This function handles the AT+SSGS command.This command is used
//  to set or get GPIO value.
//  Global resource dependence : g_rsp_str
//  Author:       Shijun.cui
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSSGS);


/*****************************************************************************/
//  Description : This function handles the AT+SBRA command, This command is used
//  to enable the ATC
//  Global resource dependence : g_rsp_str
//  Author:       Liuhan
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSBRA);
#endif

/*****************************************************************************/
//  Description : This function handles the AT+SBAND command, This command is used
//  to select the band in EGSM900/DCS1800/PCS1900.
//  Global resource dependence : g_rsp_str
//  Author:       Liuhan
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSBAND);

/*****************************************************************************/
//  Description : This function handles the AT+SNVM command, This command is used
//  to manage the NV items.
//  Global resource dependence : g_rsp_str
//  Author:       Liuhan
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSNVM);

/*****************************************************************************/
//  Description : This function handles the data send from the SIO when AT+SNVM
//  Global resource dependence :
//  Author:       SHIJUN
//  Note:
/*****************************************************************************/
PUBLIC uint32 ATC_ProcessSioSNVMData(
    ATC_CONFIG_T    *atc_config_ptr,
    uint8           *data_ptr  // pointer to the data
);

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
);

/*****************************************************************************/
//  Description : This function handles the data send from the SIO when AT+SADM
//  Global resource dependence : g_rsp_str
//  Author:
//  Note:
/*****************************************************************************/
PUBLIC uint32 ATC_ProcessSioSADMNVData(
    ATC_CONFIG_T    *atc_config_ptr,
    uint8           *data_ptr,  // pointer to the data
    uint16          param_len
);

/*****************************************************************************/
//  Description : This function handles the AT+SBOOT command.This command is used
//  to set the spreadtrum module to BOOT mode.
//  Global resource dependence : g_rsp_str
//  Author:       Xueli.lv
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSBOOT);

/*****************************************************************************/
//  Description : This function handles the AT+SLOG command.This command is used
//  to enable the log output.
//  Global resource dependence : g_rsp_str
//  Author:       Shijun.cui
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSLOG);

/*****************************************************************************/
//  Description : This function handles the AT+SUPS command.This command is used
//  to enable the log output.
//  Global resource dependence : g_rsp_str
//  Author:       Shijun.cui
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSUPS);

/*****************************************************************************/
//  Description : This function handles the AT+EUICC command.This command is used
//  to set or get GPIO value.
//  Global resource dependence : g_rsp_str
//  Author:       bin.li
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessEUICC);

/*****************************************************************************/
//  Description : This function handles the APP_MN_SET_BAND_CNF
//  Global resource dependence : none
//  Author:       Liuhan
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessSetBandCnf(
    ATC_CONFIG_T                *atc_config_ptr,
    APP_MN_SET_BAND_CNF_T       *sig_ptr
);

/*****************************************************************************/
//  Description : This function handle the APP_MN_SYNC_IND signal
//  Global resource dependence : none
//  Author:       hanjun.liu
//  Note:
/*****************************************************************************/
extern ATC_STATUS  ATC_ProcessMnSyncInd(
    ATC_CONFIG_T            *atc_config_ptr,
    APP_MN_SYNC_IND_T  *sig_ptr
);


/*****************************************************************************/
//  Description : This function handles the APP_MN_SIM_READY_IND signal
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessReadyInd(
    ATC_CONFIG_T                  *atc_config_ptr,
    APP_MN_SIM_READY_IND_T        *sig_ptr);

/*****************************************************************************/
//  Description : This function handles the APP_MN_SCELL_RSSI_IND signal
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessRssiInd(
    ATC_CONFIG_T                  *atc_config_ptr,
    APP_MN_SCELL_RSSI_IND_T       *sig_ptr);

/*****************************************************************************/
//  Description : This function handles the APP_MN_SCELL_RSSI_IND signal
//  Global resource dependence : g_rsp_str
//  Author:      sunny.shen
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessRssiIndEx(
    ATC_CONFIG_T                  *atc_config_ptr,
    APP_MN_SCELL_RSSI_IND_T       *sig_ptr);

/*****************************************************************************/
//  Description : This function handles the APP_MN_DETACH_GSM_CNF signal
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessPowerOffCnf(
    ATC_CONFIG_T                  *atc_config_ptr,
    APP_MN_DEACTIVE_PS_CNF_T     *sig_ptr
);

/*****************************************************************************/
//  Description : This function handles the ATC_CHR_CAP_IND signal.And inform the
//  DTE the current battery level
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessChrCapInd(
    ATC_CONFIG_T                  *atc_config_ptr,
    xSignalHeaderRec              *sig_ptr
);

/*****************************************************************************/
//  Description : This function handles the ATC_CHR_CHARGE_START_IND signal.Inform
//  that the battery starts charge.
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessChrStartInd(
    ATC_CONFIG_T                  *atc_config_ptr,
    xSignalHeaderRec              *sig_ptr
);

/*****************************************************************************/
//  Description : This function handles the ATC_CHR_CHARGE_END_IND signal.Inform
//  that the battery starts charge.
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessChrEndInd(
    ATC_CONFIG_T                  *atc_config_ptr,
    xSignalHeaderRec              *sig_ptr
);

/*****************************************************************************/
//  Description : This function handles the APP_MN_SIM_NOT_READY_IND signal
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessNotReadyInd(
    ATC_CONFIG_T                    *atc_config_ptr,
    APP_MN_SIM_NOT_READY_IND_T      *sig_ptr
);

/*****************************************************************************/
//  Description : This function handles the APP_MN_SIM_GET_PIN_IND signal
//  Global resource dependence : g_rsp_str
//  Author:       Xueli Lv
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessPinInd(
    ATC_CONFIG_T                   *atc_config_ptr,
    APP_MN_SIM_GET_PIN_IND_T       *sig_ptr);

/*****************************************************************************/
//  Description : This function handles the AT+CLVL command
//  Global resource dependence : g_rsp_str
//  Author:       peng.chen
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCLVL);

/*****************************************************************************/
//  Description : This function handles the AT+CMER command
//  Global resource dependence : 
//  Author:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCMER);

/******************************************************************************/
//  Description : This function convert APP_MN_NETWORK_STATUS_IND_T infos to
//                MN_PHONE_CURRENT_PLMN_INFO_T
//  Global resource dependence : None
//  Author : minqian.qian
//  Note : None
/******************************************************************************/
MN_PHONE_CURRENT_PLMN_INFO_T ConvertNWStatusToPLMNInfo(
    APP_MN_NETWORK_STATUS_IND_T *nw_plmn_status
);

/*****************************************************************************/
//  Description : This function handles the AT+SSYS command.This command 
//                    is used to set or get the operated sim id .
//  Global resource dependence : g_rsp_str
//  Author:       Shijun.cui
/*****************************************************************************/
AT_CMD_FUNC( ATC_ProcessSSYS );

/*****************************************************************************/
//  Description : This function handle the AT+SPCM command
//  Global resource dependence : g_rsp_str
//  Author:  Shijun.cui
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSPCM);

/*****************************************************************************/
//  Description : This function handle the AT+SFUN command 
//  Global resource dependence : none
//  Author:       
//  Note:
/*****************************************************************************/
AT_CMD_FUNC( ATC_ProcessSFUN );

/*****************************************************************************/
//  Description : This function handle the AT+SPAUDVOLTYPE command 
//  Global resource dependence : g_rsp_str
//  Author:
//  Note:
/*****************************************************************************/
AT_CMD_FUNC( ATC_ProcessSPAUDVOLTYPE );

#ifndef ATC_SAT_ENABLE
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
				);
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
				);
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
				);
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
                );
#endif

/*****************************************************************************/
//  Description : This function set the static variables g_is_ps_act
//  Global resource dependence :
//  Author:
//  Note:
/*****************************************************************************/
extern void Atc_SetPsActState(
        MN_DUAL_SYS_E dual_sys, 
        BOOLEAN is_act
        );
/***********************************audio func********************************/

/*****************************************************************************/
//  Description : open/close downlink mute
//  Global resource dependence : 
//  Author: xiaonian.li
//  Note: 
/*****************************************************************************/
BOOLEAN ATC_EnableRemoteMute ( BOOLEAN is_enable );

/*****************************************************************************/
//  Description : open/close Uplink mute
//  Global resource dependence : 
//  Author: xiaonian.li
//  Note: 
/*****************************************************************************/
BOOLEAN ATC_EnableUplinkMute ( BOOLEAN is_enable );

/*****************************************************************************/
//  Description : ATC_HandleMoMuteAndLocalRingState
//  Global resource dependence : 
//  Author:xiaonian.li
//  Note: open downlink mute if MS has received APP_MN_SYNC_IND,but not received 
//        APP_MN_SETUP_COMPLETE_CNF(APP_MN_PROGRESS_IND);
//        close downlink mute after receiving APP_MN_PROGRESS_IND or
//        APP_MN_SETUP_COMPLETE_CNF
/*****************************************************************************/
BOOLEAN  ATC_HandleMoMuteAndLocalRingState(MN_DUAL_SYS_E dual_sys, uint16 msg_id, uint8 call_index);


/*****************************************************************************/
//  Description : This function handles the ATC_NET_RING_EXP_IND signal
//  Global resource dependence : none
//  Author:       xiaonian.li
//  Note:
/*****************************************************************************/
void ATC_ProcessNetRingExpInd(ATC_CONFIG_T  *atc_config_ptr);

/*****************************************************************************/
//  Description : handle APP_MN_GET_SIM_TYPE_CNF signal
//  Global resource dependence : 
//  Author: 
//  Note: 
/*****************************************************************************/
void ATC_ProcessSimTypeCnf(
    ATC_CONFIG_T *atc_config_ptr,
    APP_MN_GET_SIM_TYPE_CNF_T *sig_ptr
    );
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
                );

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
               );

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
                );

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
                );
#endif /*AUDIO_PARAM_ARM*/

/*****************************************************************************/
//  Description : This function handle the AT+SLOOPTEST command, to do loop back test
//  Global resource dependence : g_rsp_str
//  Author:       longting.zhao
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSLOOPTEST);

/*****************************************************************************/
//  Description : This function handles the AT+CRSM command
//  Global resource dependence : g_rsp_str
//  Author:       Wanghui
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCRSM);

/*****************************************************************************/
//  Description : This function handles the APP_MN_CRSM_CNF signal in
//  deleting only one item.
//  Global resource dependence :
//  Author:       WangHui
//  Note:
/*****************************************************************************/
extern ATC_STATUS ATC_ProcessCrsmCnf(
    ATC_CONFIG_T             *atc_config_ptr,
    APP_MN_CRSM_CNF_T    *sig_ptr
);

/*****************************************************************************/
//  Description : This function handle the AT+SPSETSAVESMS command 
//  Global resource dependence : none
//  Author:   bin.li
//  Note:
/*****************************************************************************/
AT_CMD_FUNC( ATC_ProcessSPSETSAVESMS);

#ifdef ATC_SAT_ENABLE
/*****************************************************************************/
//  Description : This function handle the AT+SIME command 
//  Global resource dependence : g_rsp_str
//  Author:       Feng.shi
//  Note: 
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSIME);

/*****************************************************************************/
//  Description : This function handle the AT+SPLO command, Preferred operator
//  list 
//  Global resource dependence : g_rsp_str
//  Author:  Shijun.cui
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSPLO);

/*****************************************************************************/
// Description : This function handle the AT+POSI command 
//                 Automatic downloader
// Global resource dependence : none
// Author:winnie.zhu
// Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessPOSI);

/*****************************************************************************/
//  Description : This function handles the AT+SCPFRE command.This command is used
//  to set or get frequency value.
//  Global resource dependence : g_rsp_str
//  Author:       king.li
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSCPFRE);

/*****************************************************************************/
//  Description : This function handles the AT+SPEQPARA command  
//                      Get(Set) eq para from(to) RAM or FALSH
//  Global resource dependence : 
//  Author:     elvis.xu
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSPEQPARA);

/*****************************************************************************/
//  Description : This function handle the AT+SPREF command 
//                 Automatic downloader
//  Global resource dependence : none
//  Author:
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSPREF);

/*****************************************************************************/
//  Description : This function handles the AT+PSORGMOD command.This command is used
//  to set audio mode. This is used for AP+BB scenario. If AP connect only with BB's 
//  receiver, and AP want to use this physical channel as handon/handfree/BT earphone,
//  it will use this function to set audio param to DSP without change physical channel.
//  handon/handfree/BT should have the same device name in the NV item.
//  Global resource dependence : g_rsp_str
//  Author:       
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessPSORGMOD);

/*****************************************************************************/
//  Description : This function handle the AT+SCREG command 
//  Global resource dependence : g_rsp_str
//  Author:       Feng.shi
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSCREG);

/*****************************************************************************/
// Description : This function handle the AT+GSMSTATUS command 
//                 Automatic downloader
// Global resource dependence : none
// Author:winnie.zhu
// Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessGSMSTATUS);

/*****************************************************************************/
//  Description : This function handle the AT+CIEV command 
//  Global resource dependence : none
//  Author:      
//  Note: this command set the rssi report level
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessCIEV);

/*****************************************************************************/
//  Description : This function handle the AT+FIRBOOT command 
//                 Automatic downloader
//  Global resource dependence : none
//  Author:winnie.zhu
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessFIRBOOT);

/*****************************************************************************/
// Description : This function handle the AT+VGSMSW command 
//                 Automatic downloader
// Global resource dependence : none
// Author:winnie.zhu
// Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessVGSMSW);

/*****************************************************************************/
//  Description : This function handles the AT+SCINF command.This command is used
//                      to get some information about the serving and neighbour cells .
//  Global resource dependence : g_rsp_str
//  Author:       elvis.xu
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSCINF);

/*****************************************************************************/
//  Description : is sim OK?
//  Global resource dependence : none
//  Author:       fancier.fan
//  Note:
/*****************************************************************************/
extern BOOLEAN  DMSIM_SimIsReady( // TRUE: sim is ready
              MN_DUAL_SYS_E dual_sys
              );
extern BOOLEAN  GPIO_IsAllowedReportRssiToAP(void);

#endif

/*****************************************************************************/
//  Description : This function handles the ATC_ProcessPowerOnCnf signal
//  that the battery starts charge.
//  Global resource dependence : g_rsp_str
//  Author:       Feng.shi
//  Note:
/*****************************************************************************/
ATC_STATUS ATC_ProcessPsPowerOnCnf(
            ATC_CONFIG_T *atc_config_ptr, 
            MN_DUAL_SYS_E    dual_sys
            );

/*****************************************************************************/
//  Description : This function handles the ATC_ProcessPsPowerOffCnf signal
//  that the battery starts charge.
//  Global resource dependence : g_rsp_str
//  Author:       Feng.shi
//  Note:
/*****************************************************************************/
ATC_STATUS ATC_ProcessPsPowerOffCnf(
            ATC_CONFIG_T *atc_config_ptr, 
            MN_DUAL_SYS_E    dual_sys
            );

/*****************************************************************************/
//  Description : This function handle the AT+SADMDSP command 
//  Global resource dependence : g_rsp_str
//  Author:  Shijun.cui
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSADMDSP);

/*****************************************************************************/
//Description : This function handle the AT+SPADLVVE command
//Global resource dependence : g_rsp_str
// Author:  chong.liu
//Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSPADLVVE);

/*****************************************************************************/
//   Description : This function handle the AT+SPAUDVOLTYPE command
//  Global resource dependence : g_rsp_str
//  Author:  chong.liu
//  Note:
/*****************************************************************************/
AT_CMD_FUNC(ATC_ProcessSPAUDVOLTYPE);

/*****************************************************************************/
// Description : This function handles the data send from the SIO when AT+SPADLVVE
// Global resource dependence : g_rsp_str
//  Author: chong.liu
//Note:
/*****************************************************************************/
PUBLIC uint32 ATC_ProcessSioSADLVVEData(
                ATC_CONFIG_T    *atc_config_ptr,
                uint8           *data_ptr,  // pointer to the data
                uint16          param_len
                );

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
                );
#ifndef DAM_TEST
AT_CMD_FUNC(ATC_ProcessSFSTEST);
#endif
/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
}
#endif
#endif
