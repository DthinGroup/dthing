/******************************************************************************
 ** File Name:      sc6530_spi_cfg.h                                                 *
 ** Author:         liuhao                                                   *
 ** DATE:           12/20/2010                                                  *
 ** Copyright:      2010 Spreadtrum, Incoporated. All Rights Reserved.         *
 ** Description:    This file define the basic hw interfaces of spi device.      *
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                       *
 ** ------------------------------------------------------------------------- *
 ** DATE           NAME             DESCRIPTION                               *
 ** 12/20/2010     liuhao     Create.                                   *
 ** 05/30/2010     wenjun.shi     Add platform specific information, include:
 **                                threshold ratio info
 **                                interrupt enable bit info
 **                                cs bit info.                                   *
 ******************************************************************************/

#ifndef _SC6530_SPI_CFG_H_
#define _SC6530_SPI_CFG_H_
/*----------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **------------------------------------------------------------------------- */
#include "..\..\chip_module\spi\spi_phy.h"
#include "spi_drvapi.h"

/**---------------------------------------------------------------------------*
 **                             Compiler Flag                                 *
 **--------------------------------------------------------------------------*/
#ifdef   __cplusplus
extern   "C"
{
#endif

/**---------------------------------------------------------------------------*
 **                         Debugging Flag                                    *
 **---------------------------------------------------------------------------*/
//#define DEBUG_SPI
#ifdef DEBUG_SPI
#define SPI_PRINT   SCI_TRACE_LOW
#else
#define SPI_PRINT(_x_)
#endif

/**---------------------------------------------------------------------------*
**                               Macro Define for spi                        **
**---------------------------------------------------------------------------*/
#define SPI_PORT                    0x0
#define MAX_SPI_PORT_NUM            (2UL)

#define SPI_TX_WATERMARK            0x208
#define SPI_RX_WATERMARK            0x208

#define SPI0_TX_REQ_ID              4
#define SPI0_RX_REQ_ID              5
#define SPI1_TX_REQ_ID              6
#define SPI1_RX_REQ_ID              7

#define SPI_DMA_INT_TIME_OUT        300000
#define SPI_INT_TIME_OUT        10000
#define SPI_DMA_TIME_OUT            0x80000
#define SPI_PHY_POLL_TIMEOUT 0xff00000
#define SPI_BURST_SIZE                  16
#define SPI_BURST_SIZE_MARK             0xF
#define LENGTH_4_DIVIDE             4

#define SPI_RX_FIFO_FULL            BIT_0
#define SPI_RX_FIFO_EMPTY           BIT_1
#define SPI_TX_FIFO_FULL            BIT_2
#define SPI_TX_FIFO_EMPTY           BIT_3
#define SPI_RX_FIFO_REALLY_FULL     BIT_4
#define SPI_RX_FIFO_REALLY_EMPTY    BIT_5
#define SPI_TX_FIFO_REALLY_FULL     BIT_6
#define SPI_TX_FIFO_REALLY_EMPTY    BIT_7

/**---------------------------------------------------------------------------*
**                               Micro Define                                **
**---------------------------------------------------------------------------*/

#ifdef GPIO_SIMULATE_SPI_SUPPORT
#define SPI_BUS_MAX 4
#define SPI_ID_MAX 16

#define VIR_PORT_SPI0 0
#define VIR_PORT_SPI1 1
#define MAX_VIR_SPI_NUM 2
#else
#define SPI_BUS_MAX 2
#define SPI_ID_MAX 20
#endif

/*spi fifo threshold value offset*/
#define SPI_RXFULL_THRHLD_OFFSET  0
#define SPI_RXEMPTY_THRHLD_OFFSET 8
#define SPI_TXFULL_THRHLD_OFFSET  0
#define SPI_TXEMPTY_THRHLD_OFFSET 8

#define SPI_FIFO_ADDR_MASK 0x1F
#define SPI_FIFO_RX_W_ADDR_OFFSET 8

#define SPI_BITLEN_MASK 0x1F
#define SPI_BITLEN_OFFSET 2


#define SPI_CS_MASK 0xF
#define SPI_CS_OFFSET 8

#define SPI_RXF_FULL_INT_EN  BIT_6
#define SPI_TXF_EMPTY_INT_EN BIT_7
#define SPI_RXF_FULL_INT_CLR BIT_0
#define SPI_TXF_EMPTY_INT_CLR BIT_3


#define SPI_PAD_FUNC_MASK 0x30
#define SPI_FIFO_SIZE 32
#define SPI_THRHOLD_RATIO 1/2   //spi threshold ratio

/*number of words for spi threshold*/
#define SPI_THRHOLD_CNT (SPI_FIFO_SIZE*SPI_THRHOLD_RATIO)   
#define SPI_THRHOLD_CNT_CMPT (SPI_FIFO_SIZE - SPI_THRHOLD_CNT)   
#define SPI_MAX_RECEIVE_WORD 256
#define SPI_MAX_RECEIVE_BYTE (SPI_MAX_RECEIVE_WORD << 2)

#define SPI_RX_INTERVAL_MS 1 //time interval in spi rx irq mode


typedef struct
{
    uint32 phy_id;
    uint32 base_addr;
    uint16 tx_channel;
    uint16 rx_channel;
    uint16 tx_req_id;
    uint16 rx_req_id;
} SPI_BASE_INFO;

typedef struct
{
    uint32 phy_id;
    uint32 cs_id;
    uint32 pin_reg;
    uint32 spi_fun;
    uint32 gpio_fun;
} SPI_CS_INFO;

typedef struct
{
    uint32 phy_id;
    uint32 spi_clk_pin;
    uint32 spi_cs_pin;
    uint32 spi_di_pin;
    uint32 spi_do_pin;
} SPI_GPIO_INFO;

typedef struct
{
    uint32 logic_id;
    uint32 phy_id;
    uint32 cs_id;
    uint32 pin_gid;
    SPI_PHY_FUN *phy_fun;
} SPI_PHY_CFG;

typedef struct
{
    SPI_PAD_NAME_E spi_pad_name;
    uint32 addr;
    uint32 value_spi;
    uint32 value_gpio;
    uint32 gpio_id;
} SPI_PAD_INFO;

typedef struct
{
    uint32 logic_id;
    _spi_callback spi_cs_callback;
    _spi_callback spi_tx_callback;
    _spi_callback spi_rx_callback;
    _spi_callback spi_txdma_callback;
    _spi_callback spi_rxdma_callback;

} SPI_CALLBACK_INFO;


/**----------------------------------------------------------------------------*
**                         Local Function Prototype                           **
**----------------------------------------------------------------------------*/


/**----------------------------------------------------------------------------*
**                           Function Prototype                               **
**----------------------------------------------------------------------------*/
PUBLIC  uint32  __SPI_SetFifoThreshold (uint32 phy_id, SPI_THRESHOLD_PTR spi_threshold_ptr);
/**----------------------------------------------------------------------------*
**                         Compiler Flag                                      **
**----------------------------------------------------------------------------*/
#ifdef   __cplusplus
}
#endif
/**---------------------------------------------------------------------------*/
#endif
// End
