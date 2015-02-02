/******************************************************************************
 ** File Name:      uart_phy.h                                             *
 ** Author:         Zhengjiang.Lei                                            *
 ** DATE:           11/03/2005                                                *
 ** Copyright:      2005 Spreadtrum, Incoporated. All Rights Reserved.         *
 ** Description:    This file defines the basic operation interfaces of       *
 **                      serrial device. It manages create, send, receive     *
 **                      interface of serrial port.                           *
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                       *
 ** --------------------------------------------------------------------------*
 ** DATE           NAME             DESCRIPTION                               *
 ** 11/03/2005     Zhengjiang.Lei     Create.                                 *
 ** 07/04/2007     David.Jia       CR 82866, add UART_GetRxFifoCnt.           *
 ** 02/09/2008     Tao.Zhou        driver arrangement                         *
 ******************************************************************************/
#ifndef _UART_PHY_H_
#define _UART_PHY_H_
#include "uart_drvapi.h"
/**---------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **---------------------------------------------------------------------------*/
/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef __cplusplus
extern   "C"
{
#endif

typedef struct UART_INTERRUPT
{
    uint32 int_num;
    TB_ISR func;
} UART_INTERRUPT_T;

typedef struct UART_HW_CONFIG
{
    uint32 baseAddr[4];
    uint32 fifoSize[4];
    UART_INTERRUPT_T inter[4];
    uint32 max_num;
    uint32 rx_watermark;

} UART_HW_CONFIG_T;

typedef struct serial_config_funs serial_config_funs;
struct serial_config_funs
{
    void (*SetBaudSpeed) (uint32 port, uint32 baudrate);
    void (*SetParity) (uint32 port, UART_PARITY_E parity);
    void (*SetParityEnable) (uint32 port, UART_PARITY_SET_E is_enable);
    void (*SetWordSize) (uint32 port, UART_BYTE_SIZE_E byte_size);
    void (*SetStopBits) (uint32 port, UART_STOP_BIT_E stop_bits);
    void (*SetTxWaterMark) (uint32 port, uint16 tx_watermark);
    void (*SetRxWaterMark) (uint32 port, uint16 rx_watermark);
    void (*SetFlowCtl) (uint32 port, UART_FLOW_CTR_E is_enable);
    void (*SetDmaMode) (uint32 port, BOOLEAN is_enable);
    void (*SetSelfLoop) (uint32 port, BOOLEAN is_enable);
};

typedef struct serial_get_funs serial_get_funs;
struct serial_get_funs
{
    UART_PARITY_SET_E (*GetParityEnable) (uint32 port);
    uint16 (*GetTxWaterMark) (uint32 port);
    uint16 (*GetRxWaterMark) (uint32 port);
    uint32 (*GetTxFifoCnt) (uint32 port);
    BOOLEAN (*GetIsTxFifoBusy) (uint32 port);
    uint32 (*GetRxFifoCnt) (uint32 port);
    uint32 (*GetFifoSize) (uint32 port);
    uint32 (*GetBaudSpeed) (uint32 port);
};

typedef struct
{
    UART_INFO_S *p_info;
    UART_HW_CONFIG_T *p_hw;
    serial_config_funs *p_cfg_funcs;
    serial_get_funs *p_get_funcs;

} UART_CB_T;



/*
    Uart settings
*/
/* Here FIFO_SIZE = 127 is to make sure the FLAG_BYTE 0x7E to be sent at the frame end! */

#define UART_TRANSFER_TIME_OUT_CHECK(check_condition) {\
        next_tick = SCI_GetTickCount();\
        if ((next_tick - cur_tick) > UART_TRANSFER_TIME_OUT)\
        {\
            if(check_condition)\
            {\
                SCI_PASSERT(0, ("[UART]:Wait UART transfering data TIME OUT!"));/*assert verified*/\
            }\
            break;\
        }\
    }

/**---------------------------------------------------------------------------*
 **                         Macro definition                                  *
 **---------------------------------------------------------------------------*/


/**---------------------------------------------------------------------------*
 **                         Function Prototypes                               *
 **---------------------------------------------------------------------------*/

PUBLIC uint32 UART_PHY_Initilize (uint32 port_num,UART_INIT_PARA_T *para);
PUBLIC uint32 UART_PHY_Close (uint32 port_num);

PUBLIC void UART_PHY_TxInEnable (uint32 phy_port, BOOLEAN is_enable);
PUBLIC void UART_PHY_RxIntEnable (uint32 phy_port, BOOLEAN is_enable);

PUBLIC uint32 UART_PHY_GetDmaTXChannel (uint32 port_num);

PUBLIC uint32 UART_PHY_GetTxReqAddr(uint32 port);
PUBLIC uint32 UART_PHY_GetRxReqAddr(uint32 port);

/*****************************************************************************/
//  Description:   This function read data from receiving buffer directly.   *
//  Author:        Tao.Zhou                                                  *
//  Param :        UART_REG_S *p   : uart register base address pointer      *
//                 uint8* buffer   : des buffer header address               *
//                 uint32 length   : size to be read                         *
//  Return:        if success,return size read in effect                     *
//  Note  :        NONE                                                      *
/*****************************************************************************/
PUBLIC uint32 UART_PHY_ReadData (uint32 port, uint8 *buffer, uint32 length);

/******************************************************************************/
//  Description:   This function record data which will be written to         *
//                  tx fifo and enable tx fifo empty interrupt. The length    *
//                  of data should be not larger than FIFO_SIZE(127B).        *
//  Author:        Tao.Zhou                                                   *
//  Param :        UART_REG_S *p   : uart register base address pointer       *
//                 uint8* buffer   : src buffer header address                *
//                 uint32 length   : size to be writed                        *
//  Return:        if success,return size writed in effect                    *
//  Note  :        NONE                                                       *
/******************************************************************************/
PUBLIC uint32 UART_PHY_WriteData (uint32 port, const uint8 *buffer, uint32 length, BOOLEAN irq_en);

/*****************************************************************************/
//  Description:   This function get char from uart receive register         *
//                   directly.                                               *
//  Author:        Tao.Zhou                                                  *
//  Return value:  Returns a char from uart receive register.                *
//  Note:          return -1 indicates that no char received from fifo       *
/*****************************************************************************/
PUBLIC int32 UART_PHY_GetChar (uint32 port);

/*****************************************************************************/
//  Description:   This function put a char to the uart send register        *
//                   directly                                                *
//  Author:        Tao.Zhou                                                  *
//  Return value:  NONE                                                      *
//  Note:          NONE                                                      *
/*****************************************************************************/
PUBLIC void UART_PHY_DumpPutChar (uint32 port, uint8 nchar);


PUBLIC void UART_PHY_SendBreak (uint32 port, BOOLEAN is_enable);

/*****************************************************************************/
//  Description:    Clear send buffer
//  Global resource dependence: NONE
//  Author:         Tao.Zhou
//  Param :         p: uart register pointer, UART_REG_S struct;
//                  fifo_size: the number of data in the fifo
//                  time_out: time out limit
//  Note:           NONE
/*****************************************************************************/
PUBLIC void  UART_PHY_WaiTxFifoEmpty (uint32 port, uint32 fifo_size);
/*****************************************************************************/
//  Description:    Clear receive buffer
//  Global resource dependence: NONE
//  Author:         Tao.Zhou
//  Param :         p: uart register pointer, UART_REG_S struct;
//                      fifo_size: the number of data in the fifo
//                  time_out: time out limit
//                  nchar: temp value used reserving the data read out
//  Note:           NONE
/*****************************************************************************/
PUBLIC void UART_PHY_WaitRxFifoEmpty (uint32 port, uint32 fifo_size, int nchar);


/*****************************************************************************/
//  Description:   The timer handler for the uart break detect                            *
//  Global resource dependence : NONE                                                             *
//  Author:        Tao.Zhou                                                                               *
//  Note:          NONE                                                                                   *
/*****************************************************************************/
PUBLIC void UART_BreakDetectTimeoutHandler (void);

/*****************************************************************************/
//  Description:  Active the uart break timer                                         *
//  Global resource dependence : NONE                                                             *
//  Author:        Tao.Zhou                                                                               *
//  Note:          NONE                                                                                   *
/*****************************************************************************/
PUBLIC void UART_ActiveBreakDetectTimer (void);

PUBLIC ISR_EXE_T UartDSlp_Handler (uint32 param);
PUBLIC void UARTCOM_DslpFuncSts_Set (uint32 port_num, BOOLEAN is_func_enabled);
PUBLIC void UART_PHY_Arbitrate (uint32 pord_id, uint is_ARM);
PUBLIC uint32 UART_PHY_GetFifoSize (uint32 port);
PUBLIC ISR_EXE_T UART0_RXD_DSlp_Handler (uint32 param);
PUBLIC ISR_EXE_T UART0_CTS_DSlp_Handler (uint32 param);
PUBLIC ISR_EXE_T UART1_RXD_DSlp_Handler (uint32 param);



#define PURGE_TX_CLEAR     (1)
#define PURGE_RX_CLEAR     (2)


PUBLIC void  Uart_CallbackHisr (uint32 data, void *pData);



//
//
/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif

#endif //_UART_PHY_H_
