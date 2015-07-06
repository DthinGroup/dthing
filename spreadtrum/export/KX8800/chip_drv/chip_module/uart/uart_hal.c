/******************************************************************************
 ** File Name:      uartcom_drv.c                                             *
 ** Author:         Zhengjiang.Lei                                            *
 ** DATE:           11/03/2005                                                *
 ** Copyright:      2005 Spreadtrum, Incoporated. All Rights Reserved.         *
 ** Description:    This file defines the basic operation interfaces of       *
 **                      serrial device. It manages create, send, receive     *
 **                      interface of serrial port.                           *
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                                                                           *
 ** --------------------------------------------------------------------------*
 ** DATE           NAME             DESCRIPTION                               *
 ** 11/03/2005      Zhengjiang.Lei     Create.                                *
 ** 07/04/2007      David.Jia       CR 82866, add UART_GetRxFifoCnt.          *
 ** 02/09/2008      Tao.Zhou        driver arrangement                        *
 ******************************************************************************/

/**---------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **---------------------------------------------------------------------------*/
#include "sci_types.h"
#include "sci_api.h"
#include "com_drvapi.h"
#include "chip_plf_export.h"
#include "tb_drv.h"
#include "iram_mgr.h"
#include "mmu_drvapi.h"
#include "uart_phy.h"

#ifdef __cplusplus
extern "C"
{
#endif
/**---------------------------------------------------------------------------*
 **                         variable section                                  *
 **---------------------------------------------------------------------------*/
extern UART_CB_T uart_cb;
PUBLIC  BOOLEAN     g_sio_enable_deep_sleep = SCI_TRUE;  //Must be inited as SCI_TRUE;
/**---------------------------------------------------------------------------*
 **                      Function  Definitions
 **---------------------------------------------------------------------------*/
PUBLIC void UART_DMA_CONNECT(uint32 port, UART_MODE_E uart_mode, uint32 chn)
{
    uint32 req_addr = 0;

    if (UART_DMA_TX_MODE == uart_mode)
    {
        req_addr = UART_PHY_GetTxReqAddr( port );
         CHIP_REG_SET(req_addr, chn);
    }
    else
    {
        req_addr = UART_PHY_GetRxReqAddr( port );
         CHIP_REG_SET(req_addr, chn);
    }
}

PUBLIC void PurgeSio (uint32 port, uint32 clear_flag)
{
    SCI_ASSERT (port < uart_cb.p_hw->max_num);/*assert verified*/

    if (clear_flag & PURGE_TX_CLEAR)
    {
        UART_PHY_WaiTxFifoEmpty (port, 0);
    }

    if (clear_flag & PURGE_RX_CLEAR)
    {
        UART_PHY_WaitRxFifoEmpty (port, 0, 0);
    }
}

PUBLIC uint32 UART_Initilize (uint32 port,
                              UART_INIT_PARA_T *para,
                              UartCallback func)
{
    SCI_ASSERT (para != NULL);/*assert verified*/
    SCI_ASSERT (port < uart_cb.p_hw->max_num);/*assert verified*/
    SCI_ASSERT (para != NULL);/*assert verified*/
    //SCI_TRACE_LOW("para->tx_watermark:%d,para->rx_watermark:%d \n",para->tx_watermark,para->rx_watermark);
    //SCI_TRACE_LOW("UART_PHY_GetFifoSize:%d \n",UART_PHY_GetFifoSize(port));
    SCI_ASSERT ( (para->tx_watermark <= UART_PHY_GetFifoSize (port)) && (para->rx_watermark <= UART_PHY_GetFifoSize (port)));/*assert verified*/
    SCI_ASSERT (para->baud_rate <= BAUDRATE_MAX);/*assert verified*/

    //SCI_TRACE_LOW("===>>UART_Initilize port:%d,baudrate:%d \n",port,para->baud_rate);

    if (UART_ALREADY_OPEN == uart_cb.p_info[port].open_flag)
    {
        //SCI_TRACE_LOW("===>>Reopen uart0,close pre");
#if 1  //nix
	//return UART_SUCCESS;
 //       return UART_PORT_REOPEN_ERR;
#else
    UART_Close(port);
#endif
    }

    ISR_RegHandler_Ex (uart_cb.p_hw->inter[port-UART_COM0].int_num, uart_cb.p_hw->inter[port-UART_COM0].func,
                       Uart_CallbackHisr, CHIPDRV_HISR_PRIO_1, NULL);

    UART_PHY_Initilize (port,para);

    PurgeSio (port, PURGE_TX_CLEAR | PURGE_RX_CLEAR);

    uart_cb.p_info[port].open_flag = UART_ALREADY_OPEN;

    //SCI_TRACE_LOW("===>>ready to reg callback:0x%x",(void*)func);
    if (NULL != func)
    {
        //SCI_TRACE_LOW("===>>reg callback:0x%x",(void*)func);
        UART_RegCallBack (port, func);
    }

    CHIPDRV_EnableIRQINT (uart_cb.p_hw->inter[port-UART_COM0].int_num);

    return UART_SUCCESS;
}

PUBLIC uint32 UART_Close (uint32 port)
{
    SCI_ASSERT (port < uart_cb.p_hw->max_num);/*assert verified*/

    if (UART_ALREADY_OPEN != uart_cb.p_info[port].open_flag)
    {
        return UART_PORT_RECLOSE_ERR; /*lint !e527 comfirmed by xuepeng*/
    }

    CHIPDRV_DisableIRQINT (uart_cb.p_hw->inter[port-UART_COM0].int_num);
    PurgeSio (port, PURGE_TX_CLEAR | PURGE_RX_CLEAR); // Purge data in SIO FIFO

    UART_PHY_Close (port);

#ifndef PLATFORM_SC6530
{
    uint32 channel = 0;
    //disable dma mode after set open_flag zero, otherwise UART_DmaWriteData may enable dma again
    channel = UART_PHY_GetDmaTXChannel (port);
    SCI_ASSERT (0xFFFFFFFF != channel);/*assert verified*/
    DMA_HAL_ChnEnable (channel, SCI_FALSE);
}
#endif

    UART_DmaModeEnable (port, SCI_FALSE);

    uart_cb.p_info[port].open_flag = 0;

    if (NULL != uart_cb.p_info[port].callback)
    {
        UART_UnRegCallBack (port);
    }

    return UART_SUCCESS;
}

/*****************************************************************************/
//  Description:   This function read data from receiving buffer.It is called*
//                 when up layfer receives the event of "EVENT_DATA_TO_READ".*
//  Author:        weihua.wang                                               *
//  Param :        uint32 port : uart com number index                   *
//                 uint8* buffer   : des buffer header address               *
//                 uint32 length   : size to be read                         *
//  Return:        if success,return size read in effect                     *
//  Note:          NONE                                                      *
/*****************************************************************************/
PUBLIC uint32 UART_ReadData (uint32 port,
                             uint8 *buffer,
                             uint32 length)
{
    uint32 count = 0;

    SCI_ASSERT (port < uart_cb.p_hw->max_num);/*assert verified*/
    SCI_ASSERT (buffer != NULL);/*assert verified*/

    if (UART_ALREADY_OPEN != uart_cb.p_info[port].open_flag)
    {
        SCI_PASSERT (0, ("[UART]:UART port %d is not initialized", port));/*assert to do*/
        return UART_FAILURE; /*lint !e527 comfirmed by xuepeng*/
    }

    count = UART_PHY_ReadData (port, buffer, length);

    return count;
}

/*****************************************************************************/
//  Description:   This function read data in sync mode.                     *
//  Author:        Tao.Zhou                                                  *
//  Return value:  Returns a char from uart receive register.                *
//  Note:          return -1 indicates that no char received from fifo       *
/*****************************************************************************/
PUBLIC uint32 UART_ReadDataSync (uint32 port, uint8 *buffer, uint32 length)
{
    int32 result;
    uint32 count = 0;

    SCI_ASSERT (port < uart_cb.p_hw->max_num);/*assert verified*/
    SCI_ASSERT (buffer != NULL);/*assert verified*/

    while (length--)
    {
        result = UART_PHY_GetChar (port);

        if (result >= 0)
        {
            buffer[count++] = (uint8) result;
        }
        else
        {
            return count;
        }
    }

    return count;
}


/*****************************************************************************/
//  Description:   This function record data which will be written to        *
//                  tx fifo and enable tx fifo empty interrupt. The length   *
//                  of data should be not larger than FIFO_SIZE     .       *
//  Author:        weihua.wang                                               *
//  Param :        uint32 port : uart com number index                   *
//                 uint8* buffer   : src buffer header address               *
//                 uint32 length   : size to be writed                       *
//  Return:        if success,return size writed in effect                   *
//  Note:          NONE                                                      *
/*****************************************************************************/
PUBLIC uint32 UART_WriteData (uint32 port,
                              const uint8 *buffer,
                              uint32 length)
{
    return UART_WriteDataEx (port,buffer,length,SCI_TRUE);
}

/*****************************************************************************/
//  Description:   This function record data which will be written to        *
//                  tx fifo and enable tx fifo empty interrupt. The length   *
//                  of data should be not larger than FIFO_SIZE     .       *
//  Author:        weihua.wang                                               *
//  Param :        uint32 port : uart com number index                   *
//                 uint8* buffer   : src buffer header address               *
//                 uint32 length   : size to be writed                       *
//  Return:        if success,return size writed in effect                   *
//  Note:          NONE                                                      *
/*****************************************************************************/
PUBLIC uint32 UART_WriteDataEx (uint32 port,
                                const uint8 *buffer,
                                uint32 length,
                                BOOLEAN irq_en)
{
    uint32 count = 0;

    SCI_ASSERT (port < uart_cb.p_hw->max_num);/*assert verified*/
    SCI_ASSERT (buffer != NULL);/*assert verified*/

    if (UART_ALREADY_OPEN != uart_cb.p_info[port].open_flag)
    {
        SCI_PASSERT (0, ("[UART]:UART port %d is not initialized", port));/*assert to do*/
        return UART_FAILURE; /*lint !e527 comfirmed by xuepeng*/
    }

    count = UART_PHY_WriteData (port, buffer, length, irq_en);

    return count;
}

/*****************************************************************************/
//  Description:   This function write data in sync mode.                    *
//  Author:        Tao.Zhou                                                  *
//  Return value:  put a char from uart receive register.                    *
//  Note:          NONE                                                      *
/*****************************************************************************/
PUBLIC void UART_WriteDataSync (uint32 port, const uint8 *buffer, uint32 length)
{
    uint32 count = 0;

    SCI_ASSERT (port < uart_cb.p_hw->max_num);/*assert verified*/
    SCI_ASSERT (buffer != NULL);/*assert verified*/

    while (length--)
    {
        UART_PHY_DumpPutChar (port, buffer[count++]);
    }
}

/*****************************************************************************/
//  Description:   uart dma mode writing                                     *
//  Global resource dependence : NONE                                        *
//  Author:        Tao.Zhou                                                  *
//  Note:          NONE                                                      *
/*****************************************************************************/
PUBLIC void UART_DmaWriteData (uint32 port, const uint8 *buffer, uint32 length, uint32 channel)
{
    DMA_CHNCFG_T tx_dma_cfg = {0};
    uint32 uart_base_addr = 0;

    SCI_ASSERT (port < uart_cb.p_hw->max_num);/*assert verified*/
    SCI_ASSERT (buffer != NULL);/*assert verified*/
    SCI_ASSERT (0 != length);/*assert verified*/

    // make sure the uart has already open
    if (UART_ALREADY_OPEN != uart_cb.p_info[port].open_flag)
    {
        return;
    }


    uart_base_addr = (uart_cb.p_hw->baseAddr[port-UART_COM0]);
    SCI_ASSERT (0xFFFFFFFF != uart_base_addr);/*assert verified*/

    UART_DmaModeEnable (port, TRUE);
    DMA_HAL_ChnEnable (channel, SCI_FALSE);
    //DMA_CLR_ALLTRNSF_INT (channel);

    tx_dma_cfg.type           = DMA_CHN_TYPE_HARD;
    tx_dma_cfg.priority       = DMA_CHN_PRIORITY_0;
    tx_dma_cfg.workmode       = DMA_CHN_WORKMODE_NORMAL;
    tx_dma_cfg.src_datawidth  = DMA_DATAWIDTH_BYTE;
    tx_dma_cfg.dest_datawidth = DMA_DATAWIDTH_BYTE;
    tx_dma_cfg.src_address    = (uint32) buffer;
    tx_dma_cfg.requestmode    = DMA_CHN_REQMODE_NORMAL;
    tx_dma_cfg.block_length   = MIN(UART_PHY_GetFifoSize (port) - uart_cb.p_get_funcs->GetTxWaterMark (port), length);//lint !e666 confirmed by windy.wang
    tx_dma_cfg.total_length   = length;
    tx_dma_cfg.src_step       = 1;  // src increament
    tx_dma_cfg.dest_step      = 0;  // des nochange

#ifdef CHIP_ENDIAN_LITTLE
    tx_dma_cfg.dest_address   = (uint32) (uart_base_addr + ARM_UART_TXD);
    tx_dma_cfg.endian_type       = DMA_ENDIANTYPE_LITTLE;

    tx_dma_cfg.endian_switchmode = DMA_ENDIANSWITCHMODE_UN;//sc8800g2

#else
    tx_dma_cfg.dest_address   = (uint32) (uart_base_addr + ARM_UART_TXD + 0x03);
    tx_dma_cfg.endian_type    = DMA_ENDIANTYPE_BIG;
#endif

    DMA_HAL_ChnConfig (channel, &tx_dma_cfg);

    {
        DMA_CAHCECONFIG_T cacheConfig[1];
        cacheConfig[0].bufferAddr = (uint32) buffer;
        cacheConfig[0].bufferSize = length;
        cacheConfig[0].dir = DMABUFFER_TO_DEVICE;

        DMA_HAL_ChnEnable_Ex (channel, SCI_TRUE, cacheConfig, 1);

    }

}


/*****************************************************************************/
//  Description:   This function get char from uart receive register.        *
//  Author:        xingyun.he                                                *
//  Return value:  Returns a char from uart receive register.                *
//  Note:          return -1 indicates that no char received from fifo       *
/*****************************************************************************/
PUBLIC int32 UART_DumpGetChar (uint32 port)
{
    int32 result;

    SCI_ASSERT (port < uart_cb.p_hw->max_num);/*assert verified*/

    result = UART_PHY_GetChar (port);

    if (result >= 0)
    {
        return (uint8) (result);
    }
    else
    {
        return -1;
    }
}

/*****************************************************************************/
//  Description:   This function put a char to the uart send register.       *
//  Author:        xingyun.he                                                *
//  Return value:  put a char from uart receive register.                    *
//  Note:          NONE                                                      *
/*****************************************************************************/
PUBLIC void UART_DumpPutChar (uint32 port, uint8 nchar)
{
    SCI_ASSERT (port < uart_cb.p_hw->max_num);/*assert verified*/

    UART_PHY_DumpPutChar (port, nchar);
}

/*****************************************************************************/
//  Description:   Get the FIFO size of UART
//  Global resource dependence : NONE
//  Author:        Tao.Zhou
//  Note:          NONE
/*****************************************************************************/
PUBLIC  uint32 UART_HAL_FifoSize (uint32 port)
{
    return UART_PHY_GetFifoSize (port);
}
/*****************************************************************************/
//  Description:   This function set uart speed                              *
//  Author:        xingyun.he                                                *
//  Note:          NONE                                                      *
/*****************************************************************************/
PUBLIC void UART_SetBaudSpeed (uint32 port, uint32 baudrate)
{
    SCI_ASSERT (port < uart_cb.p_hw->max_num);/*assert verified*/

    SCI_DisableIRQ();
    uart_cb.p_cfg_funcs->SetBaudSpeed (port, baudrate);
    SCI_RestoreIRQ();
}

PUBLIC uint32 UART_GetBaudSpeed (uint32 port)
{
    uint32 speed = 0;
    SCI_ASSERT (port < uart_cb.p_hw->max_num);/*assert verified*/
    speed = uart_cb.p_get_funcs->GetBaudSpeed(port);
    return speed;
}

/*****************************************************************************/
//  Description:   This function set uart parity parameter                   *
//  Global resource dependence : NONE                                        *
//  Author:        Tao.Zhou                                                  *
//  Note:          NONE                                                      *
/*****************************************************************************/
PUBLIC void UART_SetParity (uint32 port, UART_PARITY_E parity)
{
    SCI_ASSERT (port < uart_cb.p_hw->max_num);/*assert verified*/

    SCI_DisableIRQ();
    uart_cb.p_cfg_funcs->SetParity (port, parity);
    SCI_RestoreIRQ();
}

/*****************************************************************************/
//  Description:   uart parity enable/disable                                *
//  Global resource dependence : NONE                                        *
//  Author:        Tao.Zhou                                                  *
//  Note:          NONE                                                      *
/*****************************************************************************/
PUBLIC void UART_ParityEnable (uint32 port, UART_PARITY_SET_E is_enable)
{
    SCI_ASSERT (port < uart_cb.p_hw->max_num);/*assert verified*/

    SCI_DisableIRQ();
    uart_cb.p_cfg_funcs->SetParityEnable (port, is_enable);
    SCI_RestoreIRQ();
}

/*****************************************************************************/
//  Description:   This function set uart byte size                          *
//  Global resource dependence : NONE                                        *
//  Author:        Tao.Zhou                                                  *
//  Note:          NONE                                                      *
/*****************************************************************************/
PUBLIC void UART_SetByteSize (uint32 port, UART_BYTE_SIZE_E byte_size)
{
    SCI_ASSERT (port < uart_cb.p_hw->max_num);/*assert verified*/

    SCI_DisableIRQ();
    uart_cb.p_cfg_funcs->SetWordSize (port, byte_size);
    SCI_RestoreIRQ();
}

/*****************************************************************************/
//  Description:   This function set uart stop bits                          *
//  Global resource dependence : NONE                                        *
//  Author:        Tao.Zhou                                                  *
//  Note:          NONE                                                      *
/*****************************************************************************/
PUBLIC void UART_SetStopBits (uint32 port, UART_STOP_BIT_E stop_bits)
{
    SCI_ASSERT (port < uart_cb.p_hw->max_num);/*assert verified*/

    SCI_DisableIRQ();
    uart_cb.p_cfg_funcs->SetStopBits (port, stop_bits);
    SCI_RestoreIRQ();
}

/*****************************************************************************/
//  Description:   This function set uart tx water mark                      *
//  Global resource dependence : NONE                                        *
//  Author:        Tao.Zhou                                                  *
//  Note:          NONE                                                      *
/*****************************************************************************/
PUBLIC void UART_SetTxWaterMark (uint32 port, uint16 tx_watermark)
{
    SCI_ASSERT (port < uart_cb.p_hw->max_num);/*assert verified*/
    SCI_ASSERT (tx_watermark <= UART_PHY_GetFifoSize (port));/*assert verified*/

    SCI_DisableIRQ();
    uart_cb.p_cfg_funcs->SetTxWaterMark (port, tx_watermark);
    SCI_RestoreIRQ();
}


/*****************************************************************************/
//  Description:   This function set uart rx water mark                      *
//  Global resource dependence : NONE                                        *
//  Author:        Tao.Zhou                                                  *
//  Note:          NONE                                                      *
/*****************************************************************************/
PUBLIC void UART_SetRxWaterMark (uint32 port, uint16 rx_watermark)
{
    SCI_ASSERT (port < uart_cb.p_hw->max_num);/*assert verified*/
    SCI_ASSERT (rx_watermark <= UART_PHY_GetFifoSize (port));/*assert verified*/

    SCI_DisableIRQ();
    uart_cb.p_cfg_funcs->SetRxWaterMark (port, rx_watermark);
    SCI_RestoreIRQ();
}

/*****************************************************************************/
//  Description:   uart hardware flow control enable/disable                 *
//  Global resource dependence : NONE                                        *
//  Author:        Tao.Zhou                                                  *
//  Note:          NONE                                                      *
/*****************************************************************************/
PUBLIC void UART_FlowCtlEnable (uint32 port, UART_FLOW_CTR_E is_enable)
{
    SCI_ASSERT (port < uart_cb.p_hw->max_num);/*assert verified*/

    SCI_DisableIRQ();
    uart_cb.p_cfg_funcs->SetFlowCtl (port, is_enable);
    SCI_RestoreIRQ();
}

/*****************************************************************************/
//  Description:   uart fifo control by DAM enable/disable                   *
//  Global resource dependence : NONE                                        *
//  Author:        Tao.Zhou                                                  *
//  Note:          NONE                                                      *
/*****************************************************************************/
PUBLIC void UART_DmaModeEnable (uint32 port, BOOLEAN is_enable)
{
    SCI_ASSERT (port < uart_cb.p_hw->max_num);/*assert verified*/

    SCI_DisableIRQ();
    uart_cb.p_cfg_funcs->SetDmaMode (port, is_enable);
    SCI_RestoreIRQ();
}

/*****************************************************************************/
//  Description:    uart tx empty enable/disable
//  Global resource dependence:  NONE
//  Author:         weihua.wang
//  Param   :       uint32 port: usb virtual com number
//                  BOOLEAN is_enable: TRUE/FALSE
//  Note:           NONE
/*****************************************************************************/
PUBLIC void UART_Tx_Int_Enable (uint32 port, BOOLEAN is_enable)
{
    SCI_ASSERT (port < uart_cb.p_hw->max_num);/*assert verified*/
        //SCI_TRACE_LOW("===>>UART_Tx_Int_Enable port(%d),enable:%d \n",port,is_enable);

    SCI_DisableIRQ();
    UART_PHY_TxInEnable (port, is_enable);
    SCI_RestoreIRQ();
}

/*****************************************************************************/
//  Description:    uart rx full enable/disable
//  Global resource dependence:  NONE
//  Author:         weihua.wang
//  Param   :       uint32 port: usb virtual com number
//                  BOOLEAN is_enable: TRUE/FALSE
//  Note:           NONE
/*****************************************************************************/
PUBLIC void UART_Rx_Int_Enable (uint32 port, BOOLEAN is_enable)
{
    SCI_ASSERT (port < uart_cb.p_hw->max_num);/*assert verified*/
    //SCI_TRACE_LOW("===>>UART_Rx_Int_Enable port(%d),enable:%d \n",port,is_enable);

    SCI_DisableIRQ();
    UART_PHY_RxIntEnable (port, is_enable);
    SCI_RestoreIRQ();
}

/*****************************************************************************/
//  Description:   enable/disable self loop mode                             *
//  Global resource dependence : NONE                                        *
//  Author:        Tao.Zhou                                                  *
//  Note:          NONE                                                      *
/*****************************************************************************/
PUBLIC void UART_SelfLoopEnable (uint32 port, BOOLEAN is_enable)
{
    SCI_ASSERT (port < uart_cb.p_hw->max_num);/*assert verified*/

    SCI_DisableIRQ();
    uart_cb.p_cfg_funcs->SetSelfLoop (port, is_enable);
    SCI_RestoreIRQ();
}

/*****************************************************************************/
//  Description:    return the available size in uart fifo
//  Global resource dependence:  NONE
//  Author:         weihua.wang
//  Param   :       uint32 port: usb virtual com number
//  Note:           NONE
/*****************************************************************************/
PUBLIC uint32 UART_GetTxFifoCnt (uint32 port)
{
    uint32 avail_size;
    uint32 fifo_cnt;

    SCI_ASSERT (port < uart_cb.p_hw->max_num);/*assert verified*/

    fifo_cnt = uart_cb.p_get_funcs->GetTxFifoCnt (port);

    if (fifo_cnt >= UART_PHY_GetFifoSize (port))
    {
        avail_size = 0;
    }
    else
    {
        avail_size = UART_PHY_GetFifoSize (port) - fifo_cnt;
    }

    return avail_size;
}

/*****************************************************************************/
//  Description:    return the received bytes in uart fifo
//  Global resource dependence:  NONE
//  Author:         David.Jia
//  Param   :       uint32 port: com number
//  Note:           NONE
/*****************************************************************************/
PUBLIC uint32 UART_GetRxFifoCnt (uint32 port)
{
    SCI_ASSERT (port < uart_cb.p_hw->max_num);/*assert verified*/

    return uart_cb.p_get_funcs->GetRxFifoCnt (port);
}

/*****************************************************************************/
//  Description:    return the Tx fifo is busy or idle
//  Global resource dependence:  NONE
//  Author  :       Jeff.Li
//  Param   :       uint32 port: usb virtual com number
//  Return  :       1 - busy, 0 - idle
//  Note:           This interface is just for 8801H platform currently
/*****************************************************************************/
PUBLIC BOOLEAN UART_IsTxFifoBusy (uint32 port)
{
    uint32 is_busy = 0;

    SCI_ASSERT (port < uart_cb.p_hw->max_num);/*assert verified*/

    is_busy = uart_cb.p_get_funcs->GetIsTxFifoBusy (port);

    return is_busy;
}

/*****************************************************************************/
//  Description:   get the max number of UART
//  Global resource dependence : NONE
//  Author:        hanjun.liu
//  Note:          NONE
/*****************************************************************************/
PUBLIC uint32 UART_MaxPortNum (void)
{
    return uart_cb.p_hw->max_num;
}
/*****************************************************************************/
//  Description:   get the  of UART
//  Global resource dependence : NONE
//  Author:        hanjun.liu
//  Note:          NONE
/*****************************************************************************/
PUBLIC uint32 UART_RxWaterMark (void)
{
    return uart_cb.p_hw->rx_watermark;
}

/*****************************************************************************/
//  Description:   uart callback function register                           *
//  Global resource dependence : NONE                                        *
//  Author:        Tao.Zhou                                                  *
//  Note:          NONE                                                      *
/*****************************************************************************/
extern     void gps_uart_callback(uint32 event);
PUBLIC void UART_RegCallBack (uint32 port, UartCallback func)
{
    SCI_ASSERT (port < uart_cb.p_hw->max_num);/*assert verified*/
    SCI_ASSERT (func != NULL);/*assert verified*/

    uart_cb.p_info[port].callback = func;
    //SCI_TRACE_LOW("===>>send EVENT_INIT_COMPLETE");

    //gps_uart_callback(2/*EVENT_INIT_COMPLETE*/);
    (uart_cb.p_info[port].callback) (EVENT_INIT_COMPLETE);
}

/*****************************************************************************/
//  Description:   uart callback function unregister                         *
//  Global resource dependence : NONE                                        *
//  Author:        Tao.Zhou                                                  *
//  Note:          NONE                                                      *
/*****************************************************************************/
PUBLIC void UART_UnRegCallBack (uint32 port)
{
    SCI_ASSERT (port < uart_cb.p_hw->max_num);/*assert verified*/
    SCI_ASSERT ( (uart_cb.p_info[port].callback) != NULL);/*assert verified*/

    (uart_cb.p_info[port].callback) (EVENT_SHUTDOWN_COMPLETE);
    uart_cb.p_info[port].callback = NULL;
}

PUBLIC void UART_SendBreak (uint32 port, BOOLEAN is_enable)
{
    UART_PHY_SendBreak (port,  is_enable);
}
void UartDSlp_Init (void)
{
    #ifndef PLATFORM_SC6530
    /* For SC6530, Interrupt channel 0 ISR is registered in EIC module */
    ISR_RegHandler (TB_UINT, UartDSlp_Handler);
    CHIPDRV_EnableIRQINT (TB_UINT);
    #endif
}

/*****************************************************************************/
//  Description:  This function is used to set uart port deepsleep-awaken
//                function status: enable or disable
//  Author:       zhengjiang.lei
//  Param :
//  Note:         NONE
/*****************************************************************************/
PUBLIC void UARTCOM_DslpFuncSts_Set (uint32 port, BOOLEAN is_func_enabled)
{
    SCI_ASSERT (port < uart_cb.p_hw->max_num);/*assert verified*/
    uart_cb.p_info[port].dslp_func_sts = is_func_enabled;
}

/*****************************************************************************/
//  Description:  This function is used to get uart port deepsleep-awaken
//                function status: enable or disable
//  Author:       zhengjiang.lei
//  Param :
//  Note:         NONE
/*****************************************************************************/
PUBLIC BOOLEAN UARTCOM_DslpFuncSts_Get (uint32 port)
{
    SCI_ASSERT (port < uart_cb.p_hw->max_num);/*assert verified*/
    return (uart_cb.p_info[port].dslp_func_sts);
}

/*****************************************************************************/
// Description :   check the DSR and set the DTR to enable/disable deep sleep
// Global resource dependence:  NONE
// Author :        Jim.zhang
// Note :          If DSR = 1: Disable deep sleep, DTR = 1
//                 If DSR = 0, Enable deep sleep,  DTR = 0
/*****************************************************************************/
PUBLIC void UART_CheckDeepSleepFlag (void)
{
    if (g_sio_enable_deep_sleep)
    {
        // if DSR turn to 1, then disable deep sleep and DTR = 1
        if ( (REG32 (ARM_UART0_BASE + 0x08)) & BIT_8)
        {
            g_sio_enable_deep_sleep = SCI_FALSE;                    //Disable deep sleep
            REG32 (ARM_UART0_BASE + 0x18) |= BIT_8;   //DTR = 1 to Send response to terminal
        }
        else
        {
            REG32 (ARM_UART0_BASE + 0x18) &= ~BIT_8;  //DTR = 0
        }
    }
    else
    {
        // if DSR turn to 0, then enable deep sleep and DTR = 0
        if (! ( (REG32 (ARM_UART0_BASE + 0x08)) & BIT_8))
        {
            g_sio_enable_deep_sleep = SCI_TRUE;                     //Enable deep sleep
            REG32 (ARM_UART0_BASE + 0x18) &= ~BIT_8;  //DTR = 0
        }
        else
        {
            REG32 (ARM_UART0_BASE + 0x18) |= BIT_8;   //DTR = 1
        }
    }
}

PUBLIC void DSPLogUARTSwitchOn (void)
{
    UART_PHY_Arbitrate (1, 0);
}

PUBLIC void DSPLogUARTSwitchOff (void)
{
    UART_PHY_Arbitrate (1, 1);
}

LOCAL const  sio_operation_st    uart_operation = {
    UART_Initilize,
       UART_Close,
       UART_GetTxFifoCnt,
       UART_ReadData,
       UART_WriteData,
       UART_DumpGetChar,
       UART_DumpPutChar,
       UART_Tx_Int_Enable,
       UART_Rx_Int_Enable,
       NULL,  /*flush*/
       NULL,  /*purchars*/
       UART_HAL_FifoSize,
};

PUBLIC const  sio_operation_st *uart_get_operation(void)
{
    return &uart_operation;
}
/**--------------------------------------------------------------------------*
**                         Compiler Flag                                     *
**---------------------------------------------------------------------------*/
#ifdef   __cplusplus
}
#endif

/*  End Of File */
