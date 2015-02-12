/******************************************************************************
 ** File Name:      pinmap_cfg.c                                              *
 ** Author:         Sunny.Fei                                                 *
 ** DATE:           2011.01.12                                                *
 ** Copyright:      2011 Spreadtrum, Incoporated. All Rights Reserved.         *
 ** Description:                                                              *
 **                                                                           *
 **                                                                           *
 ******************************************************************************
 **---------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **---------------------------------------------------------------------------*/
#include "arm_reg.h"/*lint -esym(766, chip_drv\export\inc\outdated\arm_reg.h)*/
#include "pinmap.h"
#include "sc_reg.h"/*lint -esym(766, chip_drv\export\inc\outdated\sc_reg.h)*/

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    extern   "C" 
    {
#endif

/**---------------------------------------------------------------------------*
 **                         Data                                              *
 **---------------------------------------------------------------------------*/
const PM_PINFUNC_T pm_func[]=
{
//  | Pin Register      Sleep OE/IE |  Slp PU/PD  |  Func Select | Func PU/PD | DS              // GPIO NUM: Used as: NOTICE: ...
#ifdef UART1_PIN_SEL_UART1_1 // sdio pin
    { APB_PIN_CTL0,        ( 0x00000040) },	// uart1 use UART1_1
#endif    
    { PIN_CTRL_REG,        ( 0x1FFFF) },
    { PIN_CAM_PD_REG,    ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_3   | PIN_FPX_EN | PIN_DS_1) },    // GPIO_37: NC
    { PIN_CAM_RSTN_REG,  ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_3   | PIN_FPX_EN | PIN_DS_1) },    // GPIO_38:NC
    { PIN_CCIRCK_REG,    ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_CCIRD0_REG,    ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_CCIRD1_REG,    ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_CCIRD2_REG,    ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_CCIRD3_REG,    ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_CCIRD4_REG,    ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_CCIRD5_REG,    ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_CCIRD6_REG,    ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_CCIRD7_REG,    ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_CCIRHS_REG,    ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_CCIRMCK_REG,   ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_CCIRVS_REG,    ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_CLK_AUX_REG,   ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_3   | PIN_FPX_EN | PIN_DS_1) },    // GPIO_24:NC
    { PIN_EMADVN_REG,    ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_EMCEN1_REG,    ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_2) },
    { PIN_EMCLK_REG,     ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_2) },
    { PIN_EMCRE_REG,     ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_EMD0_REG,      ( PIN_I_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_EMD1_REG,      ( PIN_I_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_EMD2_REG,      ( PIN_I_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_EMD3_REG,      ( PIN_I_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_EMD4_REG,      ( PIN_I_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_EMD5_REG,      ( PIN_I_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_EMD6_REG,      ( PIN_I_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_EMD7_REG,      ( PIN_I_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_EMD8_REG,      ( PIN_I_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_EMD9_REG,      ( PIN_I_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_EMD10_REG,     ( PIN_I_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_EMD11_REG,     ( PIN_I_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_EMD12_REG,     ( PIN_I_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_EMD13_REG,     ( PIN_I_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_EMD14_REG,     ( PIN_I_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_EMD15_REG,     ( PIN_I_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_EMA16_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_EMA17_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_EMA18_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_EMA19_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_EMA20_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },  
    { PIN_EMA21_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) }, 
    { PIN_EMA22_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) }, 
    { PIN_EMA23_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) }, 
    { PIN_EMA24_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) }, 
    { PIN_EMLBN_REG,     ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_EMOEN_REG,     ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_EMUBN_REG,     ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_EMWAIT_REG,    ( PIN_I_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_EMWEN_REG,     ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_ESMCLK_REG,    ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_ESMCSN0_REG,   ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_ESMCSN1_REG,   ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_ESMD0_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_ESMD1_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_ESMD2_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_ESMD3_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },

    { PIN_EXTINT0_REG,   ( PIN_I_EN | PIN_SPU_EN  | PIN_FUNC_DEF | PIN_FPU_EN | PIN_DS_1) },
    { PIN_EXTINT1_REG,   ( PIN_I_EN | PIN_SPU_EN  | PIN_FUNC_DEF | PIN_FPU_EN | PIN_DS_1) },

    { PIN_GPIO_0_REG,    ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPD_EN | PIN_DS_1) },    // GPIO_0, NC
    { PIN_GPIO_1_REG,    ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPD_EN | PIN_DS_1) },    // GPIO_1: NC
    { PIN_GPIO_2_REG,    ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPD_EN | PIN_DS_1) },    // GPIO_2: NC
    { PIN_GPIO_3_REG,    ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPD_EN | PIN_DS_1) },    // GPIO_3: NC
    { PIN_GPIO_63_REG,   ( PIN_I_EN | PIN_SPD_EN  | PIN_FUNC_1   | PIN_FPD_EN | PIN_DS_1) },    // GPIO_63: BT_XTL_EN, NOTICE: must be SPD and FPD
    //{ PIN_IISCLK_REG,    ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPD_EN | PIN_DS_1) },
    { PIN_IISCLK_REG,       ( PIN_O_EN | PIN_SPU_EN  | PIN_FUNC_3 | PIN_FPU_EN | PIN_DS_1) },
    { PIN_IISDI_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPD_EN | PIN_DS_1) },
    { PIN_IISDO_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_IISLRCK_REG,   ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPD_EN | PIN_DS_1) },
    { PIN_IISMCK_REG,    ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_1   | PIN_FPX_EN | PIN_DS_1) },      // NOTICE: 32K used for FM/BT
    { PIN_KEYIN0_REG,    ( PIN_I_EN | PIN_SPU_EN  | PIN_FUNC_DEF | PIN_FPU_EN | PIN_DS_1) },    
    { PIN_KEYIN1_REG,    ( PIN_I_EN | PIN_SPU_EN  | PIN_FUNC_DEF | PIN_FPU_EN | PIN_DS_1) },    
    { PIN_KEYIN2_REG  ,  ( PIN_I_EN | PIN_SPU_EN  | PIN_FUNC_DEF | PIN_FPU_EN | PIN_DS_1) },
    { PIN_KEYIN3_REG,    ( PIN_I_EN | PIN_SPU_EN  | PIN_FUNC_DEF | PIN_FPU_EN | PIN_DS_1) },    
    { PIN_KEYIN4_REG,    ( PIN_I_EN | PIN_SPU_EN  | PIN_FUNC_DEF | PIN_FPU_EN | PIN_DS_1) },    
    { PIN_KEYIN5_REG,    ( PIN_I_EN | PIN_SPU_EN  | PIN_FUNC_DEF | PIN_FPU_EN | PIN_DS_1) },    
    { PIN_KEYIN6_REG,    ( PIN_Z_EN | PIN_SPU_EN  | PIN_FUNC_1   | PIN_FPU_EN | PIN_DS_1) }, // SDA1
    { PIN_KEYIN7_REG,    ( PIN_Z_EN | PIN_SPU_EN  | PIN_FUNC_1   | PIN_FPU_EN | PIN_DS_1) }, // SCL1
    { PIN_KEYOUT0_REG ,  ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_KEYOUT1_REG ,  ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_KEYOUT2_REG ,  ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_3 | PIN_FPX_EN | PIN_DS_1) },    // GPIO_88:
    { PIN_KEYOUT3_REG ,  ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_KEYOUT4_REG ,  ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_KEYOUT5_REG ,  ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },
    { PIN_KEYOUT6_REG ,  ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },

#ifdef MAINLCM_INTERFACE_SPI
    { PIN_LCMCD_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPD_EN | PIN_DS_1) },    // GPIO_41 : 
    { PIN_LCMCS0_REG,    ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPD_EN | PIN_DS_1) },    // GPIO_42 : 
    { PIN_LCMCS1_REG,    ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPD_EN | PIN_DS_1) },    // GPIO_49 : 
    { PIN_LCMD0_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_2   | PIN_FPX_EN | PIN_DS_2) },    // GPIO_53 : 
    { PIN_LCMD1_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_2   | PIN_FPX_EN | PIN_DS_2) },    // GPIO_54 : 
    { PIN_LCMD2_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_2   | PIN_FPX_EN | PIN_DS_2) },    // GPIO_55 : 
    { PIN_LCMD3_REG,     ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_2   | PIN_FPX_EN | PIN_DS_2) },    // GPIO_56 : 
    { PIN_LCMD4_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_1   | PIN_FPX_EN | PIN_DS_2) },    // GPIO_57 : 
    { PIN_LCMD5_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPD_EN | PIN_DS_1) },    // GPIO_58 : 
    { PIN_LCMD6_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPD_EN | PIN_DS_1) },    // GPIO_59 : 
    { PIN_LCMD7_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPD_EN | PIN_DS_1) },    // GPIO_60 : 
    { PIN_LCMD8_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPD_EN | PIN_DS_1) },    // GPIO_61 : 
    { PIN_LCMRD_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPD_EN | PIN_DS_1) },    // GPIO_39 : 
    { PIN_LCMRSTN_REG,   ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // GPIO_62 : 
    { PIN_LCMWR_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPD_EN | PIN_DS_1) },    // GPIO_40 : 
#else                                                                                           //LCM_INTERFACE_LCM
    { PIN_LCMCD_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // GPIO_41 :
    { PIN_LCMCS0_REG,    ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // GPIO_42 :
    { PIN_LCMCS1_REG,    ( PIN_I_EN | PIN_SPU_EN  | PIN_FUNC_3   | PIN_FPU_EN | PIN_DS_1) },    // GPIO_49 : GPIO_PROD_SIM_PLUG_IN_ID
#if 0
    { PIN_LCMD0_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // GPIO_53 :
#else
	{ PIN_LCMD0_REG,     ( PIN_O_EN | PIN_SPU_EN  | PIN_FUNC_3 | PIN_FPU_EN | PIN_DS_1) },    // GPIO_53 :
	//{ PIN_IISCLK_REG,    ( PIN_O_EN | PIN_SPU_EN  | PIN_FUNC_3 | PIN_FPU_EN | PIN_DS_1) },
#endif    
    { PIN_LCMD1_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // GPIO_54 :
    { PIN_LCMD2_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // GPIO_55 :
    { PIN_LCMD3_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // GPIO_56 :
    { PIN_LCMD4_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // GPIO_57 :
    { PIN_LCMD5_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // GPIO_58 :
    { PIN_LCMD6_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // GPIO_59 :
    { PIN_LCMD7_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // GPIO_60 :
    { PIN_LCMD8_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // GPIO_61 :
    { PIN_LCMRD_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // GPIO_39 :
    { PIN_LCMRSTN_REG,   ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // GPIO_62 :
    { PIN_LCMWR_REG,     ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // GPIO_40 :
#endif

    { PIN_LCMD14_REG,    ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPD_EN | PIN_DS_1) },
    { PIN_LCMD15_REG,    ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPD_EN | PIN_DS_1) },
    { PIN_LCMD16_REG,    ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPD_EN | PIN_DS_1) },
    { PIN_LCMD17_REG,    ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPD_EN | PIN_DS_1) },

#ifdef 1
    { PIN_MTDO_REG,      ( PIN_Z_EN | PIN_SPU_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // GPIO_22 :
    { PIN_MTDI_REG,      ( PIN_Z_EN | PIN_SPU_EN  | PIN_FUNC_DEF | PIN_FPU_EN | PIN_DS_1) },    // GPIO_20 : 
    { PIN_MTCK_REG,      ( PIN_Z_EN | PIN_SPU_EN  | PIN_FUNC_DEF | PIN_FPU_EN | PIN_DS_1) },    // GPIO_21 :
    { PIN_MTMS_REG,      ( PIN_Z_EN | PIN_SPU_EN  | PIN_FUNC_DEF | PIN_FPU_EN | PIN_DS_1) },    // GPIO_19 :
    { PIN_MTRSTN_REG,    ( PIN_Z_EN | PIN_SPU_EN  | PIN_FUNC_DEF | PIN_FPU_EN | PIN_DS_1) },    // GPIO_23 :
#else
    { PIN_MTDO_REG,      ( PIN_Z_EN | PIN_SPU_EN  | PIN_FUNC_1   | PIN_FPU_EN | PIN_DS_1) },    // GPIO_22 : SDA2: gsensor
    { PIN_MTDI_REG,      ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPD_EN | PIN_DS_1) },    // GPIO_20 :
    { PIN_MTCK_REG,      ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPD_EN | PIN_DS_1) },    // GPIO_21 : 
    { PIN_MTMS_REG,      ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPD_EN | PIN_DS_1) },    // GPIO_19 :
    { PIN_MTRSTN_REG,    ( PIN_Z_EN | PIN_SPU_EN  | PIN_FUNC_1   | PIN_FPU_EN | PIN_DS_1) },    // GPIO_23 : SCL2: gsensor
#endif      
    { PIN_PTEST_REG,     ( PIN_Z_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPD_EN | PIN_DS_1) },    // PULL DOWN TO GROUND EXTERNALLY
#ifdef 0
    { PIN_PWM_REG,       ( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_2   | PIN_FPX_EN | PIN_DS_1) },    // GPIO_25 : ATV_PCLK
#else
    { PIN_PWM_REG,       ( PIN_O_EN | PIN_SPU_EN  | PIN_FUNC_DEF   | PIN_FPU_EN | PIN_DS_1) },    // GPIO_25 : EXT_GPIO7/PWM
#endif
    // following RF pins, if not used as DSP RF, they could be used as GPIO, e.g. PIN_RF_PAMODE_REG
    { PIN_RF_PAMODE_REG,   ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1)},    // GPIO_47 : DSP_RF
    { PIN_RFSCK_REG,     ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // DSP only
    { PIN_RFSDA_REG,     ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // DSP only
    { PIN_RFSEN_REG,     ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // DSP only
    { PIN_RF_VBS1_REG,   ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // GPIO_44 : DSP_RF
    { PIN_RF_VBS2_REG,   ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // GPIO_45 : DSP_RF
    { PIN_RF_VBS3_REG,   ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // GPIO_46 : DSP_RF
    { PIN_RF_PABAND_REG,  ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1)},    // GPIO_43 : DSP_RF
    { PIN_RF_PAEN_REG,   ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // GPIO_48 : DSP_RF
 
    { PIN_SCL_REG,       ( PIN_Z_EN | PIN_SPU_EN  | PIN_FUNC_DEF | PIN_FPU_EN | PIN_DS_1) },    // GPIO_32 : Camera SCL
    { PIN_SDA_REG,       ( PIN_Z_EN | PIN_SPU_EN  | PIN_FUNC_DEF | PIN_FPU_EN | PIN_DS_1) },    // GPIO_33 : Camera SDA
    { PIN_SD_CLK_REG,    ( PIN_Z_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_2) },    // SD/SDIO     
    { PIN_SD_CLK1_REG,   ( PIN_Z_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_2) },    // NC
#ifdef UART1_PIN_SEL_UART1_1 // sdio pin
    { PIN_SD_CMD_REG,    ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_1   | PIN_FPX_EN | PIN_DS_1) },    // SD/SDIO: UART1_TXD
    { PIN_SD_D0_REG,     ( PIN_I_EN | PIN_SPU_EN  | PIN_FUNC_1   | PIN_FPU_EN | PIN_DS_1) },    // SD/SDIO: UART1_RXD
#else
    { PIN_SD_CMD_REG,    ( PIN_Z_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPU_EN | PIN_DS_2) },    // SD/SDIO
    { PIN_SD_D0_REG,     ( PIN_Z_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPU_EN | PIN_DS_2) },    // SD/SDIO
#endif    
    { PIN_SD_D1_REG,     ( PIN_Z_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPU_EN | PIN_DS_2) },    // SD/SDIO
    { PIN_SD_D2_REG,     ( PIN_Z_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPU_EN | PIN_DS_2) },    // SD/SDIO
    { PIN_SD_D3_REG,     ( PIN_Z_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPU_EN | PIN_DS_2) },    // SD/SDIO    
    { PIN_SIMCLK0_REG,   ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // GPIO_81 : SIM0
    { PIN_SIMDA0_REG,    ( PIN_Z_EN | PIN_SPU_EN  | PIN_FUNC_DEF | PIN_FPU_EN | PIN_DS_1) },    // GPIO_82 : 
    { PIN_SIMRST0_REG,   ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // GPIO_80 : 
    { PIN_SIMCLK1_REG,   ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // GPIO_85 : SIM1
    { PIN_SIMDA1_REG,    ( PIN_Z_EN | PIN_SPU_EN  | PIN_FUNC_DEF | PIN_FPU_EN | PIN_DS_1) },    // GPIO_83 : 
    { PIN_SIMRST1_REG,   ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // GPIO_84 : 
    { PIN_SIMCLK2_REG,   ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // GPIO_17 : SIM2
    { PIN_SIMDA2_REG,    ( PIN_Z_EN | PIN_SPU_EN  | PIN_FUNC_DEF | PIN_FPU_EN | PIN_DS_1) },    // GPIO_15 :  
    { PIN_SIMRST2_REG,   ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // GPIO_16 : 
    { PIN_SPI0_0_CS1_REG,( PIN_Z_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // GPIO_14 :
    { PIN_SPI1_0_CS0_REG,( PIN_Z_EN | PIN_SPD_EN  | PIN_FUNC_3   | PIN_FPD_EN | PIN_DS_1) },    // GPIO_18 : 

#ifdef UART0_HWFLOW_SUPPORT
    { PIN_U0CTS_REG,     ( PIN_I_EN | PIN_SPD_EN  | PIN_FUNC_DEF | PIN_FPD_EN | PIN_DS_1) },    // GPIO_4  : UART0_CTSn, default: PD, active to send data to peer device
    { PIN_U0RTS_REG,     ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // GPIO_5  : UART0_RTSn
#else
    { PIN_U0CTS_REG,     ( PIN_I_EN | PIN_SPU_EN  | PIN_FUNC_1   | PIN_FPU_EN | PIN_DS_1) },    // GPIO_4  : UART1_RXD
    { PIN_U0RTS_REG,     ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_1   | PIN_FPX_EN | PIN_DS_1) },    // GPIO_5  : UART1_TXD
#endif
    { PIN_U0RXD_REG,     ( PIN_I_EN | PIN_SPU_EN  | PIN_FUNC_DEF | PIN_FPU_EN | PIN_DS_1) },    // GPIO_95 : UART0_BT
    { PIN_U0TXD_REG,     ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1) },    // GPIO_94 : UART1_BT

    { ANA_PIN_TESTRSTN_REG,( PIN_I_EN | PIN_SPU_EN| PIN_FUNC_DEF | PIN_FPU_EN | PIN_DS_1 )},    // TEST POINT
    { ANA_PIN_PBINT_REG, ( PIN_I_EN | PIN_SPU_EN  | PIN_FUNC_DEF | PIN_FPU_EN | PIN_DS_1 )},
    { ANA_PIN_TP_XL_REG, ( PIN_I_EN | PIN_SPU_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1 )},    // GPIO_128: 
    { ANA_PIN_TP_XR_REG, ( PIN_Z_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1 )},    // GPIO_129: 
    { ANA_PIN_TP_YU_REG, ( PIN_Z_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1 )},    // GPIO_130: 
    { ANA_PIN_TP_YD_REG, ( PIN_O_EN | PIN_SPX_EN  | PIN_FUNC_DEF | PIN_FPX_EN | PIN_DS_1 )},    // GPIO_131: 
    
    {0xffffffff, 0xffffffff}           
};

                          
const PM_GPIO_CTL_T  pm_gpio_default_map[]=
{                         
// The description below   only for SC6600L Chip
// GPIOs --> need to set   directory(input/output), 
//             if input, n      eed to set interrupt sense,
//             if output,       need to set default value
// GPOs  --> need to set  afault value, others would ingore
// GPIs  --> only need t  o regist, and would ingore all parameters expect NO.
                          
// GPIO Type          General NO.            SC6600L valible NO.
// GPI                00 - 15                00 - 05
// GPO                16 - 47                16 - 47
// GPIO               48 - 79                48 - 55 / 64 - 71
                          
//  |  NO. | Def Valule   |     Direct           | INT Sense      |
//   {0,         0,              PM_OUTPUT,          PM_NO_INT       },  // GPIO_PROD_SPEAKER_PA_EN_ID
//   {47,        0,              PM_OUTPUT,          PM_NO_INT       },  // GPIO_PROD_BT_RESET_ID
//   {49,        0,              PM_INPUT,           PM_LEVEL        },  // GPIO_PROD_SIM_PLUG_IN_ID 
    {8,        0,              PM_INPUT,          PM_LEVEL       },
    {53,        0,              PM_INPUT,          PM_NO_INT       },
    {0xffff,    0,              PM_INVALID_DIR,     PM_INVALID_INT  }
};

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    }
#endif

// End of pinmap_cfg.c
