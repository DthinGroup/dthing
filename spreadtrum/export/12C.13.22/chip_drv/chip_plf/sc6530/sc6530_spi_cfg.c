/******************************************************************************
 ** File Name:      sc6530_spi_cfg.c                                                *
 ** Author:         liuhao                                                   *
 ** DATE:           12/20/2010                                                 *
 ** Copyright:      2010 Spreadtrum, Incoporated. All Rights Reserved.         *
 ** Description:    This file define the hal layer of spi device.      *
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                       *
 ** ------------------------------------------------------------------------- *
 ** DATE           NAME             DESCRIPTION                               *
 ** 06/28/2010     liuhao     Create.                                   *
 ******************************************************************************/

/**---------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **---------------------------------------------------------------------------*/
#include "sci_types.h"
#include "sc6530_reg_base.h"
#include "..\..\chip_module\spi\spi_phy.h"
#include "..\..\chip_module\pin\v5\pin_reg_v5.h"
#include "sc6530_spi_cfg.h"
#include "..\..\chip_module\spi\v5\spi_reg_v5.h"
#include "..\..\chip_plf\common\inc\chip_drv_common_io.h"

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
extern   "C"
{
#endif

/**---------------------------------------------------------------------------*
 **                            Macro Define
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 **                            Local Variables
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 **                            Global Variables
 **---------------------------------------------------------------------------*/

extern SPI_PHY_FUN spi_phy_fun_v5;

#ifdef GPIO_SIMULATE_SPI_SUPPORT
extern SPI_PHY_FUN spi_phy_fun_simu;
#endif

const SPI_PHY_CFG __spi_phy_cfg[SPI_ID_MAX] =
{
    /*logic id, controller id, cs id, method*/
    {SPI0_0_CS0, SPI_BUS_ID_0, SPI_CS_ID_0, SPI_PIN_GID_0, &spi_phy_fun_v5}, /*hw spi controller1, cs 0*/
    {SPI0_0_CS1, SPI_BUS_ID_0, SPI_CS_ID_1, SPI_PIN_GID_0, &spi_phy_fun_v5}, /*hw spi controller1, cs 1*/
#ifdef GPIO_SIMULATE_SPI_SUPPORT
    {SPI2_0_CS0, SPI_BUS_ID_2, SPI_CS_ID_0, SPI_PIN_GID_0, &spi_phy_fun_simu}, /*sw simulation spi controller1, port 0*/
    {SPI3_0_CS0, SPI_BUS_ID_3, SPI_CS_ID_0, SPI_PIN_GID_0, &spi_phy_fun_simu}, /*sw simulation spi controller2, port 0*/
    {SPI0_1_CS0, SPI_BUS_ID_0, SPI_CS_ID_0, SPI_PIN_GID_1, &spi_phy_fun_v5}, /*hw spi controller1, cs 1*/
    {SPI0_1_CS1, SPI_BUS_ID_0, SPI_CS_ID_1, SPI_PIN_GID_1, &spi_phy_fun_v5}, /*hw spi controller1, cs 1*/
    {SPI0_2_CS0, SPI_BUS_ID_0, SPI_CS_ID_0, SPI_PIN_GID_2, &spi_phy_fun_v5}, /*hw spi controller1, cs 1*/
    {SPI0_2_CS1, SPI_BUS_ID_0, SPI_CS_ID_1, SPI_PIN_GID_2, &spi_phy_fun_v5}, /*hw spi controller1, cs 1*/
    {SPI1_0_CS0, SPI_BUS_ID_1, SPI_CS_ID_0, SPI_PIN_GID_0, &spi_phy_fun_v5}, /*hw spi controller1, cs 1*/
    {SPI1_0_CS1, SPI_BUS_ID_1, SPI_CS_ID_1, SPI_PIN_GID_0, &spi_phy_fun_v5}, /*hw spi controller1, cs 1*/
    {SPI1_1_CS0, SPI_BUS_ID_1, SPI_CS_ID_0, SPI_PIN_GID_1, &spi_phy_fun_v5}, /*hw spi controller1, cs 1*/
    {SPI1_1_CS1, SPI_BUS_ID_1, SPI_CS_ID_1, SPI_PIN_GID_1, &spi_phy_fun_v5}, /*hw spi controller1, cs 1*/
    {SPI1_2_CS0, SPI_BUS_ID_1, SPI_CS_ID_0, SPI_PIN_GID_2, &spi_phy_fun_v5}, /*hw spi controller1, cs 1*/
    {SPI1_2_CS1, SPI_BUS_ID_1, SPI_CS_ID_1, SPI_PIN_GID_2, &spi_phy_fun_v5}, /*hw spi controller1, cs 1*/
    {SPI1_3_CS0, SPI_BUS_ID_1, SPI_CS_ID_0, SPI_PIN_GID_3, &spi_phy_fun_v5}, /*hw spi controller1, cs 1*/
    {SPI1_3_CS1, SPI_BUS_ID_1, SPI_CS_ID_1, SPI_PIN_GID_3, &spi_phy_fun_v5}, /*hw spi controller1, cs 1*/
#else
    {SPI0_1_CS0, SPI_BUS_ID_0, SPI_CS_ID_0, SPI_PIN_GID_1, &spi_phy_fun_v5}, /*hw spi controller1, cs 1*/
    {SPI0_1_CS1, SPI_BUS_ID_0, SPI_CS_ID_1, SPI_PIN_GID_1, &spi_phy_fun_v5}, /*hw spi controller1, cs 1*/
    {SPI0_2_CS0, SPI_BUS_ID_0, SPI_CS_ID_0, SPI_PIN_GID_2, &spi_phy_fun_v5}, /*hw spi controller1, cs 1*/
    {SPI0_2_CS1, SPI_BUS_ID_0, SPI_CS_ID_1, SPI_PIN_GID_2, &spi_phy_fun_v5}, /*hw spi controller1, cs 1*/
#ifndef M2M_EVB_SUPPORT

    //{SPI1_0_CS0, SPI_BUS_ID_1, SPI_CS_ID_0, SPI_PIN_GID_0, &spi_phy_fun_v5}, /*hw spi controller1, cs 1*/
    {SPI1_0_CS1, SPI_BUS_ID_1, SPI_CS_ID_1, SPI_PIN_GID_0, &spi_phy_fun_v5}, /*hw spi controller1, cs 1*/
    {SPI1_1_CS0, SPI_BUS_ID_1, SPI_CS_ID_0, SPI_PIN_GID_1, &spi_phy_fun_v5}, /*hw spi controller1, cs 1*/
    {SPI1_1_CS1, SPI_BUS_ID_1, SPI_CS_ID_1, SPI_PIN_GID_1, &spi_phy_fun_v5}, /*hw spi controller1, cs 1*/
#endif
    {SPI1_2_CS0, SPI_BUS_ID_1, SPI_CS_ID_0, SPI_PIN_GID_2, &spi_phy_fun_v5}, /*hw spi controller1, cs 1*/
    {SPI1_2_CS1, SPI_BUS_ID_1, SPI_CS_ID_1, SPI_PIN_GID_2, &spi_phy_fun_v5}, /*hw spi controller1, cs 1*/
    {SPI1_3_CS0, SPI_BUS_ID_1, SPI_CS_ID_0, SPI_PIN_GID_3, &spi_phy_fun_v5}, /*hw spi controller1, cs 1*/
    {SPI1_3_CS1, SPI_BUS_ID_1, SPI_CS_ID_1, SPI_PIN_GID_3, &spi_phy_fun_v5}, /*hw spi controller1, cs 1*/
    {SPI0_0_CD,  SPI_BUS_ID_0, SPI_CS_ID_0, SPI_PIN_GID_1, &spi_phy_fun_v5},
    {SPI0_1_CD,  SPI_BUS_ID_0, SPI_CS_ID_0, SPI_PIN_GID_1, &spi_phy_fun_v5},
    {SPI0_2_CD,  SPI_BUS_ID_0, SPI_CS_ID_0, SPI_PIN_GID_1, &spi_phy_fun_v5},
    {SPI1_0_CD,  SPI_BUS_ID_1, SPI_CS_ID_0, SPI_PIN_GID_1, &spi_phy_fun_v5},
    {SPI1_1_CD,  SPI_BUS_ID_1, SPI_CS_ID_0, SPI_PIN_GID_1, &spi_phy_fun_v5},
    {SPI1_2_CD,  SPI_BUS_ID_1, SPI_CS_ID_0, SPI_PIN_GID_1, &spi_phy_fun_v5},
#endif
};

PUBLIC SPI_BASE_INFO __spi_base_info[SPI_BUS_MAX] =
{
    /*hw controller id, base address, tx chanel, rx chanel*/
    {SPI_BUS_ID_0, SPI0_BASE, INVALID_U16, INVALID_U16, SPI0_TX_REQ_ID, SPI0_RX_REQ_ID},/*hw spi controller1, register base*/
    {SPI_BUS_ID_1, SPI1_BASE, INVALID_U16, INVALID_U16, SPI1_TX_REQ_ID, SPI1_RX_REQ_ID},/*hw spi controller1, register base*/
#ifdef GPIO_SIMULATE_SPI_SUPPORT
    {SPI_BUS_ID_2, INVALID_U32, INVALID_U16, INVALID_U16, INVALID_U16, INVALID_U16}, /*sw spi controller1, no register base*/
    {SPI_BUS_ID_3, INVALID_U32, INVALID_U16, INVALID_U16, INVALID_U16, INVALID_U16} /*sw spi controller2, no register base*/
#endif
};

const SPI_PAD_INFO __spi_pad_func_info[]=
{
    /*SPI_PAD_NAME_E, pin_addr, value_spi, value_gpio, gpio_id*/
    {PAD_SPI0_0_CLK,  PIN_SD_CLK_REG, PIN_FUNC_2, PIN_FUNC_3, 12 },
    {PAD_SPI0_0_DI,  PIN_SD_D0_REG,  PIN_FUNC_2, PIN_FUNC_3, 11 },
    {PAD_SPI0_0_DO,  PIN_SD_CMD_REG,  PIN_FUNC_2, PIN_FUNC_3, 10 },
    {PAD_SPI0_0_CS0,  PIN_SD_D3_REG, PIN_FUNC_2, PIN_FUNC_3, 13 },
    {PAD_SPI0_0_CS1,  PIN_SPI0_0_CS1_REG,  PIN_FUNC_DEF,   PIN_FUNC_3, 14 },

    {PAD_SPI0_1_CLK,  PIN_LCMD2_REG, PIN_FUNC_1, PIN_FUNC_3, 55 },
    {PAD_SPI0_1_DI,  PIN_LCMD0_REG,  PIN_FUNC_1, PIN_FUNC_3, 53 },
    {PAD_SPI0_1_DO,  PIN_LCMD1_REG,  PIN_FUNC_1, PIN_FUNC_3, 54 },
    {PAD_SPI0_1_CS0,  PIN_LCMD3_REG, PIN_FUNC_1, PIN_FUNC_3, 56 },
    {PAD_SPI0_1_CS1,  PIN_LCMD4_REG,  PIN_FUNC_1,   PIN_FUNC_3, 57 },

    {PAD_SPI0_2_CLK,  PIN_LCMCD_REG, PIN_FUNC_1, PIN_FUNC_3, 41 },
    {PAD_SPI0_2_DI,  PIN_LCMRD_REG,  PIN_FUNC_1, PIN_FUNC_3, 39 },
    {PAD_SPI0_2_DO,  PIN_LCMWR_REG,  PIN_FUNC_1, PIN_FUNC_3, 40 },
    {PAD_SPI0_2_CS0,  PIN_LCMCS0_REG, PIN_FUNC_1, PIN_FUNC_3, 42 },
    {PAD_SPI0_2_CS1,  INVALID_U32,  INVALID_U32,   INVALID_U32, INVALID_U32 },

    {PAD_SPI1_0_CLK,  PIN_SIMCLK2_REG, PIN_FUNC_1, PIN_FUNC_3, 17 },
    {PAD_SPI1_0_DI,  PIN_SIMRST2_REG,  PIN_FUNC_1, PIN_FUNC_3, 16 },
    {PAD_SPI1_0_DO,  PIN_SIMDA2_REG,  PIN_FUNC_1, PIN_FUNC_3, 15 },
    {PAD_SPI1_0_CS0,  PIN_SPI1_0_CS0_REG, PIN_FUNC_DEF, PIN_FUNC_3, 18 },
    {PAD_SPI1_0_CS1,  INVALID_U32,  INVALID_U32,   INVALID_U32, INVALID_U32 },

    {PAD_SPI1_1_CLK,  PIN_LCMD2_REG, PIN_FUNC_2, PIN_FUNC_3, 55 },
    {PAD_SPI1_1_DI,  PIN_LCMD0_REG,  PIN_FUNC_2, PIN_FUNC_3, 53 },
    {PAD_SPI1_1_DO,  PIN_LCMD1_REG,  PIN_FUNC_2, PIN_FUNC_3, 54 },
    {PAD_SPI1_1_CS0,  PIN_LCMD3_REG, PIN_FUNC_2, PIN_FUNC_3, 56 },
    {PAD_SPI1_1_CS1,  PIN_LCMD4_REG,  PIN_FUNC_2,   PIN_FUNC_3, 57 },

    {PAD_SPI1_2_CLK,  PIN_LCMCD_REG, PIN_FUNC_2, PIN_FUNC_3, 41 },
    {PAD_SPI1_2_DI,  PIN_LCMRD_REG,  PIN_FUNC_2, PIN_FUNC_3, 39 },
    {PAD_SPI1_2_DO,  PIN_LCMWR_REG,  PIN_FUNC_2, PIN_FUNC_3, 40 },
    {PAD_SPI1_2_CS0,  PIN_LCMCS0_REG, PIN_FUNC_2, PIN_FUNC_3, 42 },
    {PAD_SPI1_2_CS1,  PIN_LCMCS1_REG,  PIN_FUNC_2,   PIN_FUNC_3, 49 },

    {PAD_SPI1_3_CLK, PIN_SPI1_0_CS0_REG, PIN_FUNC_2, PIN_FUNC_3, 18 },
    {PAD_SPI1_3_CLK_ALTERNATE, PIN_LCMD14_REG, PIN_FUNC_2, PIN_FUNC_3, 75 },
    {PAD_SPI1_3_DI,  PIN_LCMD16_REG,  PIN_FUNC_2, PIN_FUNC_3, 77 },
    {PAD_SPI1_3_DO,  PIN_LCMD17_REG,  PIN_FUNC_2, PIN_FUNC_3, 78 },
    {PAD_SPI1_3_CS0,  PIN_SPI0_0_CS1_REG, PIN_FUNC_2, PIN_FUNC_3, 14 },
    {PAD_SPI1_3_CS0_ALTERNATE, PIN_LCMD15_REG, PIN_FUNC_2, PIN_FUNC_3, 76 },
    {PAD_SPI1_3_CS1, INVALID_U32,  INVALID_U32,   INVALID_U32, INVALID_U32 },

    {INVALID_U32, INVALID_U32,      INVALID_U32,   INVALID_U32, INVALID_U32}
};


/**---------------------------------------------------------------------------*
 **                      Function  Definitions
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
}
#endif

/*  End Of File */
