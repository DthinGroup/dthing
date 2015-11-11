/******************************************************************************
 ** File Name:      mux_driver_8800s.c                                      *
 ** Author:         yayan.xu                                             *
 ** DATE:           07/10/2008                                              *
 ** Copyright:      2002 Spreatrum, Incoporated. All Rights Reserved.         *
 ** Description:      The file defines MUX RECV TASK behavior*
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                       *
 ** ------------------------------------------------------------------------- *
 ** DATE           NAME             DESCRIPTION                               *
 ** 07/10/2008     yayan.xu     Create.                                   *
 ******************************************************************************/
#include "ms_ref_mux_trc.h"
#include "mux_all.h"
#include "mux_driver.h"
#include "com_drv.h"
#include "uartcom_drv.h"  //extern declaration of uart 
#include "ucom_api.h"     //extern declaration of usb virtual com
#include "sio.h"
#include "sc_reg.h"
#include "ucom_common.h"     //extern declaration of usb virtual com
#include "run_mode.h"
#include "char_dev.h"
#ifdef MUX_UART_FLOW_CTRL
#include "gpio_prod_cfg.h"
#include "doidleonpage.h"
#endif

#define MAX_FIFO_SIZE  0x1000  /*should be in header file*/
#define GET_DATABUF_SIZE(data_size, _buf)  \
do  \
{   \
    if (_buf->end_point >= _buf->start_point) \
    {\
        data_size =  (_buf->end_point - _buf->start_point);\
    }\
    else\
    {\
        data_size =  ((int)_buf->size + _buf->end_point - _buf->start_point);\
    }\
} while(0);

//extern uint32   mux_recv_task_id;
extern PUBLIC void      REFPARAM_SetDataPortBaudRate(uint32  baud_rate);
extern PUBLIC uint32    REFPARAM_GetDataPortBaudRate(void);
#ifdef MUX_UART_FLOW_CTRL
extern uint32 SCI_GetTickCount(void);
#endif
LOCAL uint32 MUX_PortHdl(uint32 event,uint8 mux_index);
LOCAL void MUX_DefaultFlush(uint32 portnum);
LOCAL uint32 MUX_PortHdl_Data1(uint32 event);
LOCAL uint32 MUX_PortHdl_Data2(uint32 event);
#ifdef MUX_UART_FLOW_CTRL
LOCAL int  MUX_Ring_Space( SIO_BUF_S *pRing );
#endif

extern UCOM_DEVICE_T * UCOM_GetDevfromPort(uint8 port_num);
/*lint -esym(752, USB_get_max_packet_size) */
/*------------------------------------------------------------------------------
Defination :MUX var
------------------------------------------------------------------------------*/
MUX_MUTEX_PTR   mux_driver_mutex[MAX_MUX_NUM];    // drive communication mutex
MUX_LLI_INFO_T  mux_port[MAX_MUX_NUM];
//LOCAL uint8     mux_data_send_buf[DATA_PORT_SEND_BUF_SIZE];  // User port tx buffer
//LOCAL uint8     mux_data_rec_buf[DATA_PORT_REC_BUF_SIZE];    // User port rx buffer
LOCAL uint8     mux_data_send_buf[MAX_MUX_NUM][MUX_SEND_DATA_MAX];  // User port tx buffer
LOCAL uint8     mux_data_rec_buf[MAX_MUX_NUM][MUX_REC_DATA_MAX];    // User port rx buffer


LOCAL  uint8    s_mux_uart_data_txbuf[MAX_MUX_NUM][MAX_FIFO_SIZE];
LOCAL  uint8    s_mux_uart_data_rxbuf[MAX_MUX_NUM][MAX_FIFO_SIZE];

LOCAL  BOOLEAN  is_mux_usb_data_enable  = FALSE;  //judge if usb data is enable@ellyzhou

#ifdef MUX_UART_FLOW_CTRL
LOCAL uint32 s_mux_task_id = 0;//this task used to collect garbage generated in write operation.
#define  MUX_STACK_SIZE     		0x800
#define  MUX_QUEUE_NUM        	    0x20
#define  MUX_TASK_PRI               0//SCI_PRIORITY_LOWEST//only lower then layer1 and timer
#define  MUX_UART_FIFO_SIZE         128

typedef enum
{
	MUX_RX_DATA =0,
	MUX_TX_DATA
}MUX_TASK_COMMAND_E;
typedef enum
{
    MUX_REV_SUCCESS = 0,
	MUX_END_FLAG_ERR = 1<<0,
	MUX_FRAME_LEN_ERR = 1<<1,
	MUX_FIFO_NO_EMPTY_ERR = 1<<2,
	MUX_REV_ERR_MAX = 0xff
}MUX_REV_ERR_E;

typedef struct UMEM_HANDLER_IRP_TAG
{
    MUX_TASK_COMMAND_E cmd;
    uint32  cmdwparam;
    void   *cmdlparam;
}MUX_IRP_T;

typedef struct  MUX_HANDLER_IRP_SIG_TAG
{
   xSignalHeaderRec     sig;
   MUX_IRP_T            irp;
} MUX_IRP_SIG_T;

#define HDLC_FLAG               		0x7E
#define HDLC_ESCAPE             		0x7D
#define HDLC_ESCAPE_MASK        	    0x20


static int32 g_rec_frame_num = 0;
static int32 g_rec_frame_num_modem = 0;
static BOOLEAN g_last_rec_escape = SCI_FALSE;

static unsigned int s_step = 0;
static BOOLEAN g_revert = SCI_FALSE;
static char rev[MUX_UART_FIFO_SIZE];

static uint32 mux_rx_flow_ctrl_status = 0;

static uint32  tx_len = 0x0;
static uint32  rx_len = 0x0;

static MUX_REV_ERR_E mux_rev_err_info = MUX_REV_SUCCESS;


extern BOOLEAN ATC_TaskStatus_Get(void);
extern BOOLEAN Uart_Fifo_Overflow_Get(void);
#endif
#ifdef MUX_ATCIPSEND

LOCAL MUX_ATCIPSEND_T g_mux_atcipsend = {0};

/*---------------------------------------------------------------------------*/  
// FUNCTION                                                                    
//     MUX_ATC_RtnGblCtl                                                
// DESCRIPTION                                                                 
//     Return the global control pointer when AT+IPSEND application needes.                                            
// OUTPUT                                                                      
//     None                                                                    
// RETURN VALUE                                                                
//     Pointer                                                                    
// DEPENDENCIES                                                                
//     Called by ATC application                                                         
/*---------------------------------------------------------------------------*/
PUBLIC MUX_ATCIPSEND_T * MUX_ATC_RtnGblCtl(void)
{
    return &g_mux_atcipsend;
}

/*---------------------------------------------------------------------------*/  
// FUNCTION                                                                    
//     SIO_ATC_SetCmdLineMaxLen                                                
// DESCRIPTION                                                                 
//     Set the global g_sio_atc_is_max_mode & g_sio_atc_cmdline_maxlen,        
// for max mode and max ATC length.                                            
// OUTPUT                                                                      
//     None                                                                    
// RETURN VALUE                                                                
//     None                                                                    
// DEPENDENCIES                                                                
//     Called by ATC                                                           
/*---------------------------------------------------------------------------*/
PUBLIC void MUX_ATC_SetCmdLineMaxLen(BOOLEAN is_max, uint32 max_len)
{
	g_mux_atcipsend.atc_is_max_mode		= is_max;
	g_mux_atcipsend.atc_cmdline_maxlen	= max_len;
}

#endif  //#ifdef MUX_ATCIPSEND

PUBLIC uint8 *MUX_GetRxBuffer(uint8 index)
{
	if(index < MAX_MUX_NUM)
	{
		return s_mux_uart_data_rxbuf[index];
	}
	return NULL;
}
PUBLIC BOOLEAN MUX_TxBufferIsEmpty(void)
{

     uint8 mux_index = MUX_1;

       //For SPI, default COM_DATA
	MUX_TRACE_LOW(("start: %d, end: %d", mux_port[mux_index].tx_buf.start_point, mux_port[mux_index].tx_buf.end_point));
	
	if(mux_port[mux_index].tx_buf.end_point != mux_port[mux_index].tx_buf.start_point)
	{
		return SCI_FALSE;
	}
	else
	{
		return SCI_TRUE;
	}

}
//for COM_DATA2
PUBLIC BOOLEAN MUX_TxBufferIsEmpty_2(void)
{

     uint8 mux_index = MUX_2;

	SCI_TRACE_LOW("MUX2:start: %d, end: %d", mux_port[mux_index].tx_buf.start_point, mux_port[mux_index].tx_buf.end_point);
	
	if(mux_port[mux_index].tx_buf.end_point != mux_port[mux_index].tx_buf.start_point)
	{
		return SCI_FALSE;
	}
	else
	{
		return SCI_TRUE;
	}

}

#ifdef MUX_UART_FLOW_CTRL

LOCAL uint32  Mux_DoIdleCallBack (uint32 param)
{
    SCI_TRACE_LOW("Mux_DoIdleCallBack: tx_len=0x%x, rx_len=0x%x", tx_len, rx_len);
    return 0;
}

LOCAL void Mux_SendCommand (MUX_IRP_T *irp_ptr)                                
{
	MUX_IRP_SIG_T * sig_ptr = NULL;
	BLOCK_ID            sender = SCI_IdentifyThread();
	
	sig_ptr = (MUX_IRP_SIG_T *)SCI_ALLOC(sizeof(MUX_IRP_SIG_T));
	SCI_ASSERT(SCI_NULL != sig_ptr);/*assert verified*/
	sig_ptr->sig.SignalSize = sizeof(MUX_IRP_SIG_T);
	sig_ptr->sig.Sender     = sender;
	sig_ptr->sig.Pre  = SCI_NULL;
	sig_ptr->sig.Suc = SCI_NULL;
	sig_ptr->sig.SignalCode = 0;
	sig_ptr->irp = *irp_ptr;   
	if (SCI_SUCCESS != SCI_SendSignal((xSignalHeader)sig_ptr, s_mux_task_id))
	{
	        SCI_ASSERT(0);/*assert verified*/
	}
}


LOCAL uint32 Mux_Tx_Data_Process (uint8 *src_ptr, uint8* des_ptr, uint32 num)
{
    uint32 i=0x0, send_len=0x0;
	uint8  curval;
	
	/*start flag*/
	*(des_ptr++) = HDLC_FLAG;
	 send_len++;
	 *(des_ptr++) = num;//send num¡¤¡é?¨ª2¨¤¨¨¡¤¡À¡ê?TD¨¨¡Áa¨°?¡ê??¨°?¡¥?¨®¨º?2¨¤2¨´¡Á¡Â
	 send_len++;
	 
	 /*content */
	 for (i = 0; i < num; i++)
	 {
			curval = *(src_ptr + i);
			if ((HDLC_FLAG == curval) || (HDLC_ESCAPE == curval)) {
				*(des_ptr++) = HDLC_ESCAPE;
				*(des_ptr++) = ~HDLC_ESCAPE_MASK & curval;
				send_len++;
			} else {
				*(des_ptr++) = curval;
			}
			send_len++;
	 }
	 
	/*end flag*/
	  *(des_ptr++) = HDLC_FLAG;
	  send_len++;
	  SCI_ASSERT(send_len<= MUX_UART_FIFO_SIZE);/*assert verified*/

	  return send_len;
}


LOCAL uint32 Mux_Rx_Data_Process (uint32 phy_port, uint32 mux_index)
{
		uint32 st=0, count=0, port=0;
		int32  ch=0;

		//SCI_TRACE_LOW("Mux_ThreadEntry Mux_Rx_Data_Process enter");
		port = SIO_PhyPortfromIndex(phy_port);
		st = (mux_port[mux_index].mux_op.get_rx_fifocnt)(port);

		while (st & 0xff)
		{
			ch = (mux_port[mux_index].mux_op.get_char)(port);
			//SCI_TRACE_LOW("Mux_ThreadEntry read a char=%x s_step=%d STS1=%x \n",ch, s_step, st);
			switch(s_step)
			{
				case 0:
					if (HDLC_FLAG == ch)
					s_step = 1;
					else
					{
					    mux_rev_err_info |= MUX_END_FLAG_ERR;
						//SCI_TRACE_LOW("Mux_ThreadEntry HDLC_FLAG: begin flag error=%x \n",ch);
					}
					break;

				case 1:						
					g_rec_frame_num_modem = ch;
					//SCI_TRACE_LOW("Mux_ThreadEntry frame_len = %d \n", g_rec_frame_num_modem);
					s_step=2;
					break;

				case 2:
					if (HDLC_FLAG == ch)
					{	
						if (g_rec_frame_num != g_rec_frame_num_modem)
						{
						     mux_rev_err_info |= MUX_FRAME_LEN_ERR;
							//SCI_TRACE_LOW("Mux_ThreadEntry : end flag error \n");
						}
						else
						{					
							st = (mux_port[mux_index].mux_op.get_rx_fifocnt)(port);
							if(st != 0x0)
							{
							       mux_rev_err_info |= MUX_FIFO_NO_EMPTY_ERR;
								//SCI_TRACE_LOW("Mux_ThreadEntry FIFO NOT EMPTY!, STS1=0x%04x\n",st);
							}

							{
								s_step =0;
								g_rec_frame_num	= 0;
								g_rec_frame_num_modem = 0;
								g_last_rec_escape = SCI_FALSE;
								g_revert = SCI_TRUE;
							}
							//SCI_TRACE_LOW("Mux_ThreadEntry receive complete frame");
						}
					}
					else
					{						    
						if(HDLC_ESCAPE == ch)  //0x7d
						{
							//SCI_TRACE_LOW("Mux_ThreadEntry recv 0x7d \n");
							g_last_rec_escape = SCI_TRUE;
						}
						else
						{
							if (g_last_rec_escape)
							{
								rev[count++] = ch^HDLC_ESCAPE_MASK;
								g_last_rec_escape = SCI_FALSE;
							}
							else
							{
								rev[count++] = ch;
							}
							g_rec_frame_num++;
						}
					}
					break;
			}
			st = (mux_port[mux_index].mux_op.get_rx_fifocnt)(port);
	   }

	   return count;
}


LOCAL void MUX_Io_Init(void)
{
    mux_sw_flow_ctrl *mux_pin_info = GPIO_MuxFlowCtrlPinInfo();
	GPIO_Enable (mux_pin_info->mux_tx_pin);
	GPIO_Enable (mux_pin_info->mux_rx_pin);
	GPIO_SetDirection (mux_pin_info->mux_tx_pin, SCI_FALSE);
	GPIO_SetDirection (mux_pin_info->mux_rx_pin, SCI_TRUE);
}



LOCAL void Mux_ThreadEntry(uint32 argc, void *argv)
{

	uint8    mux_index;
	BOOLEAN  tx_low_or_high, rx_low_or_high;
	BOOLEAN handshake= SCI_FALSE;
	uint32  num, phy_port, fifo_cnt, loop_value;
	MUX_IRP_SIG_T  *sig_ptr;
	MUX_TASK_COMMAND_E cmd;
	mux_sw_flow_ctrl *mux_pin_info = GPIO_MuxFlowCtrlPinInfo();

	MUX_Io_Init();
	//#ifdef MUX_UART_FLOW_CTRL_DEBUG
	DoIdle_RegisterCallback ((DOIDLE_CALLBACK_FUNC) Mux_DoIdleCallBack);
	//#endif

	while(TRUE)
	{
		sig_ptr = (MUX_IRP_SIG_T *)SCI_GetSignal(s_mux_task_id);
		cmd = sig_ptr->irp.cmd;
		mux_index = sig_ptr->irp.cmdwparam;
		phy_port = mux_port[mux_index].phy_port;
		
		(mux_port[mux_index].mux_op.get_reg_status) (SIO_PhyPortfromIndex(phy_port));
		//SCI_TRACE_LOW("MUX_ThreadEntry get cmd =0x%x, mux_index=0x%x",sig_ptr->irp.cmd, mux_index);
		//SCI_TRACE_LOW("MUX_ThreadEntry rx fifo overflow =0x%x, cur_tick=0x%x",Uart_Fifo_Overflow_Get(), SCI_GetTickCount());
		switch(cmd)
		{      	
		 	  case MUX_TX_DATA:
					{
						static  uint32 mux_tx_flow_ctrl_status = 0;
						static  BOOLEAN mux_tx_start = SCI_FALSE;
						uint32 tick_init  = SCI_GetTickCount();
						uint32 tick_sleep = SCI_GetTickCount();

						//SCI_TRACE_LOW("MUX_ThreadEntry  mux_tx_flow_ctrl_status =0x%x, mux_tx_start=0x%x",mux_tx_flow_ctrl_status, mux_tx_start);


						fifo_cnt = MUX_GetTxFifoCnt(mux_index);

						if(fifo_cnt> (MUX_UART_FIFO_SIZE/2-2))
						{
							fifo_cnt = MUX_UART_FIFO_SIZE/2-2;
						}	
						if (fifo_cnt)
						{
							//SCI_TRACE_LOW("MUX_ThreadEntry111 handshake=0x%x, tx_low_or_high=0x%x, mux_tx_flow_ctrl_status=0x%x", handshake, tx_low_or_high, mux_tx_flow_ctrl_status);
							num = MUX_TxForPort(s_mux_uart_data_txbuf[mux_index], fifo_cnt, mux_index);

							if (num > 0)
							{
								uint8  *src_ptr, *des_ptr;
								uint8  upacket[MUX_UART_FIFO_SIZE];
								uint32 send_len = 0;

								src_ptr  = s_mux_uart_data_txbuf[mux_index];
								des_ptr = (uint8*)&upacket;
								handshake = SCI_FALSE;
								loop_value = mux_pin_info->mux_loop_cnt;



								send_len = Mux_Tx_Data_Process(src_ptr, des_ptr, num);

								while((SCI_GetTickCount()-tick_init)<loop_value)
								{
									tx_low_or_high = GPIO_GetValue(mux_pin_info->mux_tx_pin);
									if(!mux_tx_start)
									{
										mux_tx_start = SCI_TRUE;
										mux_tx_flow_ctrl_status = tx_low_or_high;
										//SCI_TRACE_LOW("MUX_ThreadEntry : init_enter");
									}

									if(tx_low_or_high == mux_tx_flow_ctrl_status)
									{
										mux_tx_flow_ctrl_status = !mux_tx_flow_ctrl_status;
										handshake = SCI_TRUE;
										//SCI_TRACE_LOW("MUX_ThreadEntry : handshake");
										break;
									}

									if((SCI_GetTickCount()-tick_sleep)> 5)
									{
										tick_sleep = SCI_GetTickCount();
										SCI_SLEEP(5);
										SCI_TRACE_LOW("MUX_ThreadEntry : busy, sleep");
									}

									if((SCI_GetTickCount()-tick_init)> mux_pin_info->mux_tx_timeout)
									{
										tick_init = SCI_GetTickCount();
										mux_tx_start = SCI_FALSE;
										SCI_TRACE_LOW("MUX_ThreadEntry : timeout");
									}
								}
								//SCI_TRACE_LOW("MUX_ThreadEntry num=0x%x", num);
								if(handshake)
								{
								    tx_len += num;
									(mux_port[mux_index].mux_op.write) (SIO_PhyPortfromIndex(phy_port), (uint8*)&upacket , send_len);
								} 
								else
								{
									//before enter MUX_PortHdl function has already disable tx fifo empty irq, so enable it again.
									//SCI_TRACE_LOW("MUX_ThreadEntry : handshake failed");
									MUX_Tx_Int_Enable(TRUE, mux_index);
								}
							}
							else
							{
									//when using usb virtual com, then should disable it(not closing intbits in usb isr handle)
									//it will affect nothing to disable double times for uart, 
									MUX_Tx_Int_Enable(FALSE, mux_index);
									//mux_tx_start = SCI_FALSE;
								    //SCI_TRACE_LOW("MUX_ThreadEntry : tx finished");
							}
						}
						else
						{
							//here only useful for uart,when current fifocnt
							//is more than max FIFO_SIZE,then only enable uart TX
							//if current is usb, fifo_cnt will not be zero
							MUX_Tx_Int_Enable(TRUE, mux_index);
						}
					}
					break;

			  case MUX_RX_DATA:
				    //receive data from uart or vcom
					{
						uint32 st=0, count=0;
						int32  ch=0;
						int    space = 0;
						
						count = Mux_Rx_Data_Process(phy_port, mux_index);
						if(count>0)
						{
						      SCI_ASSERT(count <= MUX_UART_FIFO_SIZE);/*assert verified*/
							  SCI_MEMCPY(s_mux_uart_data_rxbuf[mux_index], rev, count);
							  space = MUX_Ring_Space(&mux_port[mux_index].rx_buf);
							  // SCI_TRACE_LOW("Mux_ThreadEntry g_revert = 0x%x, mux_rx_flow_ctrl_status=0x%x", g_revert, mux_rx_flow_ctrl_status);
							  while(space < (int)count)
							  {
									SCI_SLEEP(5);
									space = MUX_Ring_Space(&mux_port[mux_index].rx_buf);
									SCI_TRACE_LOW("Mux_ThreadEntry rx ring buf full space = 0x%x", space);
							  }

							  MUX_RxForPort(s_mux_uart_data_rxbuf[mux_index], count, mux_index);
							  
							  if(g_revert)
							  {
								  	mux_rx_flow_ctrl_status = !mux_rx_flow_ctrl_status;
									GPIO_SetValue(mux_pin_info->mux_rx_pin, mux_rx_flow_ctrl_status);
									g_revert = SCI_FALSE;
							  }
						}
					}
				    break;

			    default:
					break;
		}
		
		SCI_FREE(sig_ptr);
	}
}


LOCAL void Mux_TaskInit(void)
{
	    /*create mux data tx and receive  task*/
	    if(s_mux_task_id == 0x0)
	    {
		    s_mux_task_id  = SCI_CreateThread("T_MUX_TASK_ID",
									"Q_MUX",
									Mux_ThreadEntry,
									0,
									0,
									MUX_STACK_SIZE,
									MUX_QUEUE_NUM,
									MUX_TASK_PRI,
									SCI_PREEMPT,
									SCI_AUTO_START);
	    }
}
#endif


/*
** Returns the number of characters that can be added to this ring before it becomes full.
*/
static int MUX_Ring_Space( SIO_BUF_S *pRing )
{
  int ret = pRing->start_point - pRing->end_point - 1;
  return (ret >= 0) ? ret : (ret + (int)pRing->size);
}

/*
** Add 'ch' to this ring.
** Returns -1 if there is not enough room.
** Returns 0 otherwise.
*/
int MUX_Ring_Add( SIO_BUF_S *pRing, unsigned char ch )
{
    if(MUX_Ring_Space( pRing ) == 0 ){
        return -1;
    }

    pRing->sio_buf_ptr[pRing->end_point++] = ch;

    if ( pRing->end_point >= (int)pRing->size )
        pRing->end_point = 0;

    return 0;
}

/*****************************************************************************/
//  Description:  Clear MUX Ring buffer
//  Global resource dependence: 
//  Author:        from EllyZhou
//  Note:          TRUE :enable
//                     FALSE:disable
/*****************************************************************************/
PUBLIC void  MUX_Ring_Clear( SIO_BUF_S *pRing )
{
      /* Set receive and transmit buffer. */
    pRing->start_point = 0;
    pRing->end_point = 0;
    pRing->status = 0;
    pRing->lost_num = 0;
    return ;
}

/*For SPI Tx*/
PUBLIC uint32 MUX_Get_TxDataNeedSend(uint8 mux_index)
{
    uint32   data_size = 0;
    UCOM_DEVICE_T  * dev_ptr    = NULL;
    MUX_ASSERT(mux_index < MAX_MUX_NUM);/*assert verified*/
    if(mux_port[mux_index].open_flag == SIO_ALREADY_OPEN)
    {

        GET_DATABUF_SIZE(data_size, (&mux_port[mux_index].tx_buf));
        dev_ptr = UCOM_GetDevfromPort(USB_COM1);
        if(dev_ptr->state == UCOM_DEV_READY)
        {
		    //SCI_TRACE_LOW:"mux has %d data need send"
		    SCI_TRACE_ID(TRACE_TOOL_CONVERT,MUX_DRIVER_148_112_2_18_1_49_23_102,(uint8*)"d" ,data_size);
        }
    }
	
	return data_size;
	
}
/*****************************************************************************/
// Description :    open a UART or USB device
// Global resource dependence : none
// Author :    yayan.xu  
// DATE :      10th July.08
// Note :           
/*****************************************************************************/
PUBLIC HSIO MUX_Create(uint32      port, 
                       uint32        phy_port, 
                       MUX_CONTROL_T *dcb
                       )
                       
{
    UART_INIT_PARA_T    uart_st;
    uint8  index;

    MUX_ASSERT((phy_port == 0xff) || (phy_port < MAX_SIO_PORT_NUM));/*assert verified*/
    
    switch (port)
    {
        case COM_DATA:
            index = MUX_1;
            break;
        case COM_DATA2:
            index = MUX_2;
            break;
        default:
            //MUX_TRACE_WARNING:"MUX_Create: Fail due to port(%d) parameter error"
            SCI_TRACE_ID(TRACE_TOOL_CONVERT,MUX_DRIVER_177_112_2_18_1_49_23_103,(uint8*)"d", port);
            return PNULL;
    }

    if (0xff == phy_port)
    {
        //indicated current port can't be used,return directly
        mux_port[index].open_flag = 0;
        mux_port[index].phy_port = phy_port;

        if (NULL == mux_driver_mutex[index])
        {
            mux_driver_mutex[index] = MUX_CREATE_MUTEX("MUX_DRIVER_MUTEX", MUX_INHERIT);
        }
         //MUX_TRACE_WARNING:"MUX_Create: Fail due to phy_port parameter error in port %d"
         SCI_TRACE_ID(TRACE_TOOL_CONVERT,MUX_DRIVER_191_112_2_18_1_49_23_104,(uint8*)"d", port);
        return PNULL;
    }

    /* Check if the port is opened before. */
    if (SIO_ALREADY_OPEN == mux_port[index].open_flag)
        return ((HSIO) & mux_port[index]);

    mux_port[index].open_flag = SIO_ALREADY_OPEN;
    mux_port[index].phy_port = phy_port;

    mux_port[index].dcb.baud_rate = dcb->baud_rate;
    mux_port[index].dcb.parity = 0;//ULCRNoParity;
    mux_port[index].dcb.stop_bits = 0;//ULCRS1StopBit;
    mux_port[index].dcb.byte_size = 0;//ULCR8bits;
    mux_port[index].dcb.flow_control = dcb->flow_control;



    uart_st.tx_watermark = TX_WATER_MARK; // 0~127B
    uart_st.rx_watermark = UART_RxWaterMark(); // 0~127B; 
    uart_st.baud_rate = dcb->baud_rate;
    uart_st.parity = FALSE; 
    uart_st.parity_enable = PARITY_DISABLE;
    uart_st.byte_size = EIGHT_BITS;
#ifdef MUX_UART_FLOW_CTRL
    uart_st.stop_bits = TWO_STOP_BIT;//modified by tao
#else
	uart_st.stop_bits = ONE_STOP_BIT;
#endif
    uart_st.flow_control = dcb->flow_control; /*lint !e64*/ 
    uart_st.ds_wakeup_en = DS_WAKEUP_DISABLE;
    uart_st.ds_wakeup_type = DS_WAKEUP_BY_RXD;


#ifdef _U0_FLOW_CONTROL
    uart_st.flow_control = HW_FLOW_CONTROL;
#endif

#ifdef MUX_ATCIPSEND
	g_mux_atcipsend.atc_is_max_mode = FALSE;
	g_mux_atcipsend.atc_cmdline_maxlen = 0;
	g_mux_atcipsend.cur_atc_frame_size = 0;
#endif	//	#ifdef MUX_ATCIPSEND

    /* Set receive and transmit buffer. */
    mux_port[index].tx_buf.sio_buf_ptr = mux_data_send_buf[index];
    mux_port[index].tx_buf.size = MUX_SEND_DATA_MAX;
    mux_port[index].tx_buf.start_point = 0;
    mux_port[index].tx_buf.end_point = 0;
    mux_port[index].tx_buf.status = 0;
    mux_port[index].tx_buf.lost_num = 0;

    mux_port[index].rx_buf.sio_buf_ptr = mux_data_rec_buf[index];
    //mux_port.rx_buf.size = DATA_PORT_REC_BUF_SIZE;
    mux_port[index].rx_buf.size = MUX_REC_DATA_MAX;
    mux_port[index].rx_buf.start_point = 0;
    mux_port[index].rx_buf.end_point = 0;
    mux_port[index].rx_buf.status = 0;
    mux_port[index].rx_buf.lost_num = 0;

    if(MUX_1 == index )					/*lint !e774*/
    {
        mux_port[index].func = MUX_PortHdl_Data1; /*lint !e64*/
    }
    else
    {
        mux_port[index].func = MUX_PortHdl_Data2; /*lint !e64*/
    }
    
    //register call back func

	if( CharDev_Install_Operation(&(mux_port[index].mux_op),phy_port) != TRUE)
	{
	    return PNULL;
	}
    
    switch (phy_port)
    {
        case COM0:
        case COM1:
        case COM2:
        case COM3:
            {
                mux_port[index].mux_op.flush = MUX_DefaultFlush;
            }
            break;          
        default:
            break;
    }


    if (NULL == mux_driver_mutex[index])
    {
        mux_driver_mutex[index] = MUX_CREATE_MUTEX("MUX_DRIVE_MUTEX", MUX_INHERIT);
    }


    MUX_ASSERT(mux_port[index].mux_op.init != NULL);/*assert verified*/

    //low init handle

    (mux_port[index].mux_op.init) (SIO_PhyPortfromIndex(phy_port), &uart_st, mux_port[index].func);/*lint !e64*/
    
    //MUX_TRACE_WARNING:"MUX_Create: Open UART/USB success"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,MUX_DRIVER_284_112_2_18_1_49_23_105,(uint8*)"");
#ifdef MUX_UART_FLOW_CTRL
    Mux_TaskInit();
#endif
    /* Return handle of the sio contol block. */
    return ((HSIO) & mux_port[index]);
}
/*****************************************************************************/
// Description :    Software Reset a UART or USB device 
// Global resource dependence : none
// Author :    yayan.xu  
// DATE :      20th May.09
// Note :           
/*****************************************************************************/
PUBLIC void MUX_Reset(uint32  port ,                 
                       uint32        phy_port, 
                       MUX_CONTROL_T *dcb
                       )
                       
{
   uint8  index;

    MUX_ASSERT((phy_port == 0xff) || (phy_port < MAX_SIO_PORT_NUM));/*assert verified*/
    
    switch (port)
    {
        case COM_DATA:
            index = MUX_1;
            break;
        case COM_DATA2:
            index = MUX_2;
            break;
        default:
            //MUX_TRACE_WARNING:"MUX_Reset: Fail due to port(%d) parameter error"
            SCI_TRACE_ID(TRACE_TOOL_CONVERT,MUX_DRIVER_310_112_2_18_1_49_24_106,(uint8*)"d", port);
            return ;
    }

   
    /* Check if the port is opened before. */
    if (SIO_ALREADY_OPEN != mux_port[index].open_flag)
        return ;

 
    /* Set receive and transmit buffer. */
    mux_port[index].tx_buf.sio_buf_ptr = mux_data_send_buf[index];
    mux_port[index].tx_buf.size = MUX_SEND_DATA_MAX;
    mux_port[index].tx_buf.start_point = 0;
    mux_port[index].tx_buf.end_point = 0;
    mux_port[index].tx_buf.status = 0;
    mux_port[index].tx_buf.lost_num = 0;

    mux_port[index].rx_buf.sio_buf_ptr = mux_data_rec_buf[index];
    //mux_port.rx_buf.size = DATA_PORT_REC_BUF_SIZE;
    mux_port[index].rx_buf.size = MUX_REC_DATA_MAX;
    mux_port[index].rx_buf.start_point = 0;
    mux_port[index].rx_buf.end_point = 0;
    mux_port[index].rx_buf.status = 0;
    mux_port[index].rx_buf.lost_num = 0;

    //MUX_TRACE_WARNING:"MUX_Reset: Open UART/USB success"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,MUX_DRIVER_336_112_2_18_1_49_24_107,(uint8*)"");
    
    //notify MUX recv task
    MUX_Upon_Close(&g_mux_sys_info[index]);




    /* Return handle of the sio contol block. */
    return ;
}

/*****************************************************************************/
// Description :    Close a UART or USB device
// Global resource dependence : none
// Author :    yayan.xu  
// DATE :      10th July.08
// Note :           
/*****************************************************************************/
PUBLIC void MUX_Close(uint32 port)
{
    uint32  phy_port;
    uint8  index;
    
    switch (port)
    {
        case COM_DATA:
            index = MUX_1;
            break;
        case COM_DATA2:
            index = MUX_2;
            break;
        default:
            //MUX_TRACE_WARNING:"MUX_Close: Fail due to port(%d) parameter error"
            SCI_TRACE_ID(TRACE_TOOL_CONVERT,MUX_DRIVER_364_112_2_18_1_49_24_108,(uint8*)"d", port);
            return ;
    }


    phy_port = mux_port[index].phy_port;
    
    if (SIO_ALREADY_OPEN != mux_port[index].open_flag)
    {
        //return directly
        mux_port[index].open_flag = 0;
        return ;
    }

    MUX_ASSERT(phy_port < MAX_SIO_PORT_NUM);/*assert verified*/

    mux_port[index].open_flag = 0;

    /* Below is Hardware close. */    
    (mux_port[index].mux_op.close)(SIO_PhyPortfromIndex(phy_port));

    if (NULL != mux_driver_mutex[index])
    {
        MUX_DELETE_MUTEX(mux_driver_mutex[index]);
        mux_driver_mutex[index] = NULL;
    }
    //MUX_TRACE_WARNING:"MUX_Close: Close UART/USB success"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,MUX_DRIVER_390_112_2_18_1_49_24_109,(uint8*)"");
    //notify MUX recv task. MS00145432 
    MUX_Upon_Close(&g_mux_sys_info[index]);
}

/*****************************************************************************/
// Description :    MUX callback function for Drive.
// Global resource dependence : none
// Author :    yayan.xu  
// DATE :      10th July.08
// Note :           
/*****************************************************************************/
LOCAL uint32 MUX_PortHdl(uint32 event, uint8 mux_index)
{
    uint32  num=0;    
    uint32  phy_port;
    uint32  fifo_cnt;
#ifdef MUX_UART_FLOW_CTRL
	uint32  count;
    MUX_IRP_T mux_irp;
#endif

    MUX_ASSERT(event < COM_MAX_EVENT);/*assert verified*/

    MUX_ASSERT(mux_index < MAX_MUX_NUM);/*assert verified*/

    phy_port = mux_port[mux_index].phy_port;

    switch (event)
    {
        case EVENT_DATA_TO_READ:
 #ifdef MUX_UART_FLOW_CTRL
		   {
				int    space = 0;
				mux_sw_flow_ctrl *mux_pin_info = GPIO_MuxFlowCtrlPinInfo();
				count = Mux_Rx_Data_Process(phy_port, mux_index);
				if(count>0)
				{
				      SCI_ASSERT(count <= MUX_UART_FIFO_SIZE);/*assert verified*/
					  SCI_MEMCPY(s_mux_uart_data_rxbuf[mux_index], rev, count);
					  space = MUX_Ring_Space(&mux_port[mux_index].rx_buf);
					  if(space < (int)count)
					  {
							SCI_ASSERT(0);/*assert verified*/
					  }
					  rx_len += count;
					  MUX_RxForPort(s_mux_uart_data_rxbuf[mux_index], count, mux_index);
					  if(g_revert)
					  {
						  	mux_rx_flow_ctrl_status = !mux_rx_flow_ctrl_status;
							GPIO_SetValue(mux_pin_info->mux_rx_pin, mux_rx_flow_ctrl_status);
							g_revert = SCI_FALSE;
					  }
				} 
		   }
 #else
            //receive data from uart or vcom
            {
                fifo_cnt = MUX_GetRxFifoCnt(mux_index);
                num = (mux_port[mux_index].mux_op.read) (SIO_PhyPortfromIndex(phy_port), s_mux_uart_data_rxbuf[mux_index], fifo_cnt);
                if (num > 0)
                {
                   //here only useful for uart,when current fifocnt
                   //is more than max FIFO_SIZE,then only enable uart TX
                   //if current is usb, fifo_cnt will not be zero
                   num = MUX_RxForPort(s_mux_uart_data_rxbuf[mux_index], num, mux_index);

                }
            }
 #endif
            break;
        case EVENT_WRITE_COMPLETE:
            //send data to uart or vcom 
        #ifdef MUX_UART_FLOW_CTRL
            if(ATC_TaskStatus_Get())
            {
                mux_irp.cmd = MUX_TX_DATA;
                mux_irp.cmdwparam = mux_index;
                Mux_SendCommand((MUX_IRP_T*)&mux_irp);
            }
            else
        #endif
            {
                {
                    fifo_cnt = MUX_GetTxFifoCnt(mux_index);
                }
            #ifdef MUX_UART_FLOW_CTRL
			   	if(fifo_cnt>= (MUX_UART_FIFO_SIZE/2-2))
				{
					fifo_cnt = MUX_UART_FIFO_SIZE/2-2;
				}
            #endif
                
                if (fifo_cnt)
                {
                    num = MUX_TxForPort(s_mux_uart_data_txbuf[mux_index], fifo_cnt, mux_index);
                    if (num > 0)
                    {
                    #ifdef MUX_UART_FLOW_CTRL
						uint8  *src_ptr, *des_ptr;
						uint8  upacket[MUX_UART_FIFO_SIZE];
						uint32 send_len = 0;

						src_ptr  = s_mux_uart_data_txbuf[mux_index];
						des_ptr = (uint8*)&upacket;


						send_len = Mux_Tx_Data_Process(src_ptr, des_ptr, num);
						//write function will enable tx fifo empty irq again.
						tx_len += num;
						(mux_port[mux_index].mux_op.write) (SIO_PhyPortfromIndex(phy_port), (uint8*)&upacket , send_len);
                    #else
						//write function will enable tx fifo empty irq again.
						(mux_port[mux_index].mux_op.write) (SIO_PhyPortfromIndex(phy_port), s_mux_uart_data_txbuf[mux_index], num);
                    #endif
                    }
                    else
                    {
                        //when using usb virtual com, then should disable it(not closing intbits in usb isr handle)
                        //it will affect nothing to disable double times for uart, 
                        MUX_Tx_Int_Enable(FALSE, mux_index);
                    }
                }
                else
                {
                    //here only useful for uart,when current fifocnt
                    //is more than max FIFO_SIZE,then only enable uart TX
                    //if current is usb, fifo_cnt will not be zero
                    MUX_Tx_Int_Enable(TRUE, mux_index);
                }
            }
            break;
        case EVENT_INIT_COMPLETE:
            if ((VIR_COM0 == phy_port) || (VIR_COM1 == phy_port))
            {
                is_mux_usb_data_enable = TRUE;
            }
            break;
        case EVENT_SHUTDOWN_COMPLETE:
            is_mux_usb_data_enable = FALSE;
            break;
        default:
            break;
    }


    return num;
}
/*****************************************************************************/
// Description :    MUX callback function for COM_DATA1
// Global resource dependence : none
// Author :    yayan.xu  
// DATE :      10th July.08
// Note :           
/*****************************************************************************/
LOCAL uint32 MUX_PortHdl_Data1(uint32 event)
{


    MUX_ASSERT(event < COM_MAX_EVENT);/*assert verified*/

    return MUX_PortHdl(event, MUX_1);


}


/*****************************************************************************/
// Description :    MUX callback function for COM_DATA2
// Global resource dependence : none
// Author :    yayan.xu  
// DATE :      10th July.08
// Note :           
/*****************************************************************************/
LOCAL uint32  MUX_PortHdl_Data2(uint32 event)
{

    MUX_ASSERT(event < COM_MAX_EVENT);/*assert verified*/

    return MUX_PortHdl(event, MUX_2);

}

/*****************************************************************************/
// Description :    Get avaliable Tx FIFO bytes
// Global resource dependence : none
// Author :    yayan.xu  
// DATE :      10th July.08
// Note :           
/*****************************************************************************/
PUBLIC uint32 MUX_GetTxFifoCnt(uint8 mux_index)
{
    uint32  fifocnt = 64;
    uint32  phy_port;

    MUX_ASSERT(mux_index < MAX_MUX_NUM);/*assert verified*/

    phy_port = mux_port[mux_index].phy_port;

    //phy port is valid
    MUX_ASSERT(phy_port < MAX_SIO_PORT_NUM);/*assert verified*/

    if(mux_port[mux_index].mux_op.get_txfifocnt)
    {
         fifocnt = (mux_port[mux_index].mux_op.get_txfifocnt) (SIO_PhyPortfromIndex(phy_port));
    }
    return fifocnt;
}
/*****************************************************************************/
// Description :    Get avaliable Tx FIFO bytes
// Global resource dependence : none
// Author :    yayan.xu  
// DATE :      10th July.08
// Note :           
/*****************************************************************************/
PUBLIC uint32 MUX_GetRxFifoCnt(uint8 mux_index)
{
    uint32  fifocnt = UART_HAL_FifoSize(UART_COM0);
    uint32  phy_port;

    MUX_ASSERT(mux_index < MAX_MUX_NUM);/*assert verified*/

    phy_port = mux_port[mux_index].phy_port;

    //phy port is valid
    MUX_ASSERT(phy_port < MAX_SIO_PORT_NUM);/*assert verified*/

    //get remain avaliable fifo size 
    if(mux_port[mux_index].mux_op.get_rxfifocnt)
    {
         fifocnt = (mux_port[mux_index].mux_op.get_rxfifocnt) (SIO_PhyPortfromIndex(phy_port));
    }

    return fifocnt;
}
/*****************************************************************************/
// Description :   Operate Tx Interrupt enable or disable
// Global resource dependence : none
// Author :    yayan.xu  
// DATE :      10th July.08
// Note :           
/*****************************************************************************/
PUBLIC void MUX_Tx_Int_Enable(BOOLEAN enable_flag, uint8 mux_index)
{
    uint32  phy_port;

    MUX_ASSERT(mux_index < MAX_MUX_NUM);/*assert verified*/

    phy_port = mux_port[mux_index].phy_port;

    MUX_ASSERT(phy_port < (MAX_SIO_PORT_NUM));/*assert verified*/

    (mux_port[mux_index].mux_op.tx_enable) (SIO_PhyPortfromIndex(phy_port), enable_flag);

    return ;
}

/*****************************************************************************/
// Description :   Operate Rx Interrupt enable or disable
// Global resource dependence : none
// Author :    yayan.xu  
// DATE :      10th July.08
// Note :           
/*****************************************************************************/
PUBLIC void MUX_Rx_Int_Enable(BOOLEAN enable_flag, uint8 mux_index)
{
    uint32  phy_port;

    MUX_ASSERT(mux_index < MAX_MUX_NUM);/*assert verified*/

    phy_port = mux_port[mux_index].phy_port;

    MUX_ASSERT(phy_port < (MAX_SIO_PORT_NUM));/*assert verified*/

    (mux_port[mux_index].mux_op.rx_enable) (SIO_PhyPortfromIndex(phy_port), enable_flag);

    return ;
}
/*****************************************************************************/
// Description :   Operate uart port to flush
// Global resource dependence : none
// Author :    yayan.xu  
// DATE :      10th July.08
// Note :           
/*****************************************************************************/
LOCAL void MUX_DefaultFlush(uint32 portnum)
{
    //dummy for uart
}

/*****************************************************************************/
// Description :   Deal with  data from Drive Rx FIFO, 
//                      and Send MUX_RECEIVE_DATA_FROM_DRIVE to MUX RECV TASK
//
// Global resource dependence : none
// Author :    yayan.xu  
// DATE :      10th July.08
// Note :           
/*****************************************************************************/
PUBLIC uint32 MUX_RxForPort(uint8 *data_buf, uint32 rec_num, uint8 mux_index)
{

 
    unsigned char  nchar;
    uint32 org_rec_num = rec_num;
    uint32 save_len=0;

    
    MUX_ASSERT(mux_index < MAX_MUX_NUM);/*assert verified*/

#ifdef _U0_FLOW_CONTROL
        if ( ring_space(&(mux_port[mux_index].rx_buf)) <= EMPTY_SPACE_WATER_MARK )
        {
            MUX_Rx_Int_Enable(0, mux_index); // Disable Out_endp interrupt of usb
            //MUX_TRACE_WARNING:"MUX_RxForPort (%d): Disable Out_endp interrupt of usb"
            SCI_TRACE_ID(TRACE_TOOL_CONVERT,MUX_DRIVER_631_112_2_18_1_49_24_110,(uint8*)"d", mux_index);
        }
#endif


#ifdef _U0_SOFTWARE_FLOW_CONTROL
        if ( (1 == gXon_on) && (ring_space(&(mux_port[mux_index].rx_buf)) <= DATA_PORT_REC_BUF_SIZE*3/4) )
        {
            //MUX_TRACE_WARNING:"MUX_RxForPort (%d): Software flow control:send XOFF"
            SCI_TRACE_ID(TRACE_TOOL_CONVERT,MUX_DRIVER_639_112_2_18_1_49_24_111,(uint8*)"d", mux_index);
            gXon_on = 0;
        }
#endif


        while(rec_num)
        {
            nchar = *data_buf++;
            rec_num--;
#ifdef _U0_SOFTWARE_FLOW_CONTROL
            if ( nchar == XON ) 
            gRemoteXon_on = 1;
            else if ( nchar == XOFF )
            gRemoteXon_on = 0;
#endif


            if (MUX_Ring_Add(&mux_port[mux_index].rx_buf, nchar ) == -1 ) 
            {
                //too long length,it is should not valid at commands!
                //so just clear buffer,else will lead to overrun
                mux_port[mux_index].rx_buf.end_point = mux_port[mux_index].rx_buf.start_point;
                //MUX_TRACE_WARNING:"MUX_RxForPort (%d): Rx buffer over writtn!"
                SCI_TRACE_ID(TRACE_TOOL_CONVERT,MUX_DRIVER_662_112_2_18_1_49_24_112,(uint8*)"d", mux_index);
                continue;
            }
	     save_len++;


        }
        
#ifdef _MUX_DEBUG_
        MUX_TRACE_LOW(("MUX_RxForPort: Rx buffer recv length = %d in MUX %d!", org_rec_num, mux_index));
#endif//ifdef _MUX_DEBUG_

       if(org_rec_num > 0)
       {
           /*data write to mux_port.rx_buf. set event*/
           if(g_mux_sys_info[mux_index].g_MuxRecv_Eptr)
           {
               MUX_SET_EVENT(g_mux_sys_info[mux_index].g_MuxRecv_Eptr, g_mux_sys_info[mux_index].mux_recvtask_event, MUX_OR);
           }
       }

        return save_len;
}




    

/*****************************************************************************/
// Description :   Write data to data_buf, wirte data length is fifo_size. 
//
// Global resource dependence : none
// Author :    yayan.xu  
// DATE :      10th July.08
// Note :           
/*****************************************************************************/ 
PUBLIC uint32 MUX_TxForPort(uint8 *data_buf, uint32 fifo_size, uint8 mux_index)
{
    uint32  snd_size        = 0;
    uint32  s_start_point  ;
    uint32  s_end_point    ;
    uint32  s_size          ;
    uint8  *s_buf           ;
    unsigned char  nchar;
    uint32  data_size=0;
    uint32  Port_MPS = 64;
     
    MUX_ASSERT(mux_index < MAX_MUX_NUM);/*assert verified*/

    s_start_point   = (&mux_port[mux_index].tx_buf)->start_point;
    s_end_point     = (&mux_port[mux_index].tx_buf)->end_point;
    s_size          = (&mux_port[mux_index].tx_buf)->size;
    s_buf           = (&mux_port[mux_index].tx_buf)->sio_buf_ptr;
    #ifdef USB_PROTOCOL_SUPPORT_USB20 
        Port_MPS = USB_get_max_packet_size();
	#endif
     if(is_mux_usb_data_enable)   //usb
    {
	 GET_DATABUF_SIZE(data_size, (&mux_port[mux_index].tx_buf));//wMaxPacketSize
	 if(data_size == 0)
	 {
	 	return 0;
	 }	 
	 if((data_size <= (uint32)USB_FIFO_MAX_BYTE)&&((data_size%Port_MPS) == 0))
	 {
	 	fifo_size = data_size - 4;
	 }
    }

    snd_size = 0;

#ifdef _U0_SOFTWARE_FLOW_CONTROL       
    if (gRemoteXon_on) // Remote device can receive data.
    {
#endif
        while (fifo_size)
        {
            if (s_end_point == s_start_point)
            {
                /* no char in buffer, so disable tx irq  */

                break;
            }

            nchar = s_buf[s_start_point++];

            if (s_start_point >= s_size)
                s_start_point = 0;

            *data_buf++ = nchar;
            snd_size++;

            fifo_size--;
        }/* End while */

        (&mux_port[mux_index].tx_buf)->start_point = s_start_point;
#ifdef _U0_SOFTWARE_FLOW_CONTROL
    }
#endif

    return snd_size;
}

/*****************************************************************************/
// Description :    this function is used to send data to driver 
// Global resource dependence :
// Author :         yayan.xu
// DATE :         2008.7.26
// Note :   return value is actually sent byte numbers.   
/*****************************************************************************/
PUBLIC uint32 MUX_Data_Tx(uint8 *buf, uint32 len, uint8 mux_index)
{
    uint32  free_size       = 0;


#ifdef USB_PROTOCOL_SUPPORT_USB20     
    uint32 index ;
    uint32 num;
    uint32 fifo_cnt;

    fifo_cnt = MUX_GetTxFifoCnt(mux_index);
#endif

    MUX_ASSERT(mux_index < MAX_MUX_NUM);/*assert verified*/

    if ((0 == len) || (PNULL == buf))
    {
        //MUX_TRACE_WARNING:"MUX_Data_Tx fails  len is zero!!"
        SCI_TRACE_ID(TRACE_TOOL_CONVERT,MUX_DRIVER_793_112_2_18_1_49_25_113,(uint8*)"");
        return len;
    }

    if (mux_port[mux_index].open_flag != SIO_ALREADY_OPEN) /*lint !e661*/ /*lint !e662*/
    {
        //MUX_TRACE_WARNING:"MUX_Data_Tx fails  PORT %d is closed!"
        SCI_TRACE_ID(TRACE_TOOL_CONVERT,MUX_DRIVER_799_112_2_18_1_49_25_114,(uint8*)"d", mux_index);
        return len;
    }
    if (mux_driver_mutex[mux_index])
    {
        if(MUX_GET_MUTEX(mux_driver_mutex[mux_index], MUX_INVALID_BLOCK_ID != MUX_IdentifyThread() ? MUX_WAIT_FOREVER : 0)!= SCI_SUCCESS)
        {
            //MUX_TRACE_WARNING:"MUX_Data_Tx fails in MUX_%d due to get MUTEX fails"
            SCI_TRACE_ID(TRACE_TOOL_CONVERT,MUX_DRIVER_806_112_2_18_1_49_25_115,(uint8*)"d", mux_index);
            return len;
        }
    }
      //MUX_TRACE_LOW(("MUX_Data_Tx: length = %d  in PORT %d!",len,  mux_index));

      // Get free size which can be hold 
      MUX_GET_FREEBUF_SIZE(free_size, (&mux_port[mux_index].tx_buf));
      if(free_size < MUX_SEND_DATA_MAX >> 3)
      {
          //SCI_TRACE_LOW:"  mux free_size low 0x203000d0[0x%x],0x203000cc[0x%x],0x203000c0[0x%x],0x203000d8[0x%x]"
          SCI_TRACE_ID(TRACE_TOOL_CONVERT,MUX_DRIVER_822_112_2_18_1_49_25_116,(uint8*)"dddd",*(volatile uint32 *)0x203000d0,*(volatile uint32 *)0x203000cc,*(volatile uint32 *)0x203000c0,*(volatile uint32 *)0x203000d8);
      }
        // Not enough size to hold, because there is some masked words we have to 
        // consider before
        if (free_size<=  len)
        {
            if (mux_driver_mutex[mux_index])
            {
                MUX_PUT_MUTEX(mux_driver_mutex[mux_index]);
            }
            //SCI_RestoreIRQ(); //MS00144475
            //SCI_TRACE_LOW:"MUX_Data_Tx full skip date %d in %d PORT"
            SCI_TRACE_ID(TRACE_TOOL_CONVERT,MUX_DRIVER_833_112_2_18_1_49_25_117,(uint8*)"dd",len , mux_index);
            //SCI_TRACE_LOW:"0x203000d0[0x%x],0x203000cc[0x%x],0x203000c0[0x%x],0x203000d8[0x%x]"
            SCI_TRACE_ID(TRACE_TOOL_CONVERT,MUX_DRIVER_838_112_2_18_1_49_25_118,(uint8*)"dddd",*(volatile uint32 *)0x203000d0,*(volatile uint32 *)0x203000cc,*(volatile uint32 *)0x203000c0,*(volatile uint32 *)0x203000d8);
            return len;
        }
        else
        {

            uint32  tmp_end_point    ;
            uint32  tmp_size          ;
            uint8   *tmp_buf           ;
            uint32  todo = len;
            uint32  split; 

             
            tmp_end_point     = (&mux_port[mux_index].tx_buf)->end_point;
            tmp_size          = (&mux_port[mux_index].tx_buf)->size;
            tmp_buf           = (&mux_port[mux_index].tx_buf)->sio_buf_ptr;

            split =  ((tmp_end_point + len) > tmp_size) ? (tmp_size - tmp_end_point) : 0;

            if (split > 0) /*First copy data from end_point to max size*/
            {
                SCI_MEMCPY(tmp_buf + tmp_end_point, buf, split);
                buf += split;
                todo -= split;
                tmp_end_point= 0;
                (&mux_port[mux_index].tx_buf)->end_point = 0;
            }

            
            SCI_MEMCPY(tmp_buf + tmp_end_point, buf, todo);
            (&mux_port[mux_index].tx_buf)->end_point = (tmp_end_point + todo) % tmp_size;


        }

#ifdef USB_PROTOCOL_SUPPORT_USB20
	  index =  SIO_PhyPortfromIndex(mux_port[mux_index].phy_port);
	  if(UCOM_IsInPortIdle(index))
	  {
	         num = MUX_TxForPort(s_mux_uart_data_txbuf[mux_index], fifo_cnt, mux_index);
	         if (num > 0)
	         {
	         	
	             (mux_port[mux_index].mux_op.write) (index, s_mux_uart_data_txbuf[mux_index], num);
	         }
	  }
#else
       SCI_DisableIRQ();  //MS00144475 avoid time of close intrrept is too more.
       MUX_Tx_Int_Enable(TRUE, mux_index);
       SCI_RestoreIRQ(); 
#endif	  

    if (mux_driver_mutex[mux_index])
    {
        MUX_PUT_MUTEX(mux_driver_mutex[mux_index]);
    }


    return len;
}

/*****************************************************************************/
// Description :    this function is used to save  data in mux_port tx_buf
// Global resource dependence :
// Author :         yayan.xu
// DATE :         2008.7.26
// Note :           
/*****************************************************************************/
PUBLIC MUX_RETURN_E MUX_PutChar_Tx(uint8 ch, uint8 mux_index)
{
    uint32          free_size   = 0;
    MUX_LLI_BUF_T  *pBuf  ;
    
    MUX_ASSERT(mux_index < MAX_MUX_NUM);/*assert verified*/

    pBuf        = &mux_port[mux_index].tx_buf;
    //maybe not need here
    if (mux_port[mux_index].open_flag != SIO_ALREADY_OPEN)
    {
        return MUX_RET_FAILURE;
    }
    MUX_GET_FREEBUF_SIZE(free_size, pBuf );

    /* check buffer not full */
    while (free_size != 0)
    {
        MUX_GET_FREEBUF_SIZE(free_size,pBuf );
    }

    /*Get free memory to save.*/    
    pBuf->sio_buf_ptr[pBuf->end_point++] = ch;
    if (pBuf->end_point >= (int)pBuf->size)
    {
        pBuf->end_point = 0;
    }

    return MUX_RET_SUCCESS;
}
/*****************************************************************************/
// Description :    external module call this interface to set current speed rate.
// Global resource dependence : none
// Author :    yayan.xu  
// DATE :      21th July.08
// Note :           Just only for ATC
/*****************************************************************************/
PUBLIC uint32 MUX_Setbaud_rate(uint32 baud_rate)
{
    uint32          ret = MUX_RET_SUCCESS;
    MUX_CONTROL_T   m_dcb;
    uint8 mux_index;
    uint32 port;
#if 0    
    if ((baud_rate > BAUD_1200) || (baud_rate < BAUD_921600))
        return MUX_RET_FAILURE;
#endif/*if 0*/

    //Only support set COM_DATA baudrate,  COM_DATA2 not allow be set.

    mux_index = MUX_1;
    port = COM_DATA;


    // Set baud rate
    m_dcb.baud_rate = baud_rate;
    m_dcb.flow_control = 0;

    switch (mux_port[mux_index].phy_port)
    {
        case COM0:
        case COM1:
        case COM2:
        case COM3:
            {
                /*Close Before Open again */
                MUX_Close(port);
                MUX_Create(port, mux_port[mux_index].phy_port, &m_dcb);
            }
            break;
        case VIR_COM0:
        case VIR_COM1:
            {
                /*
                  *though baudrate is useless for usb virtual com
                  *update this info in sio struct   
                  */  
                mux_port[mux_index].dcb.baud_rate = baud_rate;
            }
            break;
        default:
            ret = MUX_RET_FAILURE;
            break;
    }

    REFPARAM_SetDataPortBaudRate(baud_rate);

    return ret;
}

/*****************************************************************************/
// Description :    external module call this interface to current speed rate.
// Global resource dependence : none
// Author :    yayan.xu  
// DATE :      21th July.08
// Note :           Just only for PPP.
/*****************************************************************************/
long MUX_Getbaud_rate(void)
{
    long    baud_rate;
    /*MUX only control COM DATA port*/
    baud_rate = REFPARAM_GetDataPortBaudRate();

#if 0 //Add SPI clock, range enlarged
    if ((baud_rate > BAUD_1200) || (baud_rate < BAUD_921600))
    {
        baud_rate = BAUD_115200;
        REFPARAM_SetDataPortBaudRate(baud_rate);
    }
#endif
    return baud_rate;
}

/*****************************************************************************/
//  Description:  get the status of if usb data out enable 
//  Global resource dependence: 
//  Author:        from EllyZhou
//  Note:          TRUE :enable
//                     FALSE:disable
/*****************************************************************************/
PUBLIC BOOLEAN MUX_IsUsbDataEnable(void)
{
    return is_mux_usb_data_enable;
}

/*****************************************************************************/
// Description :   Deal with  data from other application eg.SME,The only test interface for SME 
//                       MUX_Create shall not called by SME, so the funcation need to allocate Rx buff when 
//                       first call
//
//
// Global resource dependence : none
// Author :    yayan.xu  
// DATE :      10th July.08
// Note :           
/*****************************************************************************/
PUBLIC void    MUX_Fill_Rxbuff(uint8 *data_buf, uint32 rec_num) 
{
	unsigned char  nchar;
	uint32 org_rec_num = rec_num;
       uint8 mux_index = MUX_1;//default use COM_DATA port.

       /* MUX_Create shall not called by SME, 
            so the funcation need to allocate Rx buff when first call*/
            
	if(SIO_ALREADY_OPEN != mux_port[mux_index].open_flag)
	{

		mux_port[mux_index].open_flag =SIO_ALREADY_OPEN;
		mux_port[mux_index].rx_buf.sio_buf_ptr = mux_data_rec_buf[mux_index];
		mux_port[mux_index].rx_buf.size = MUX_REC_DATA_MAX;
		mux_port[mux_index].rx_buf.start_point = 0;
		mux_port[mux_index].rx_buf.end_point = 0;
		mux_port[mux_index].rx_buf.status = 0;
		mux_port[mux_index].rx_buf.lost_num = 0;

	}


#ifdef _U0_FLOW_CONTROL
        if ( ring_space(&(mux_port[mux_index].rx_buf)) <= EMPTY_SPACE_WATER_MARK )
        {
	        MUX_Rx_Int_Enable(0, mux_index); // Disable Out_endp interrupt of usb
	        //MUX_TRACE_WARNING:"MUX_Fill_Rxbuff: Disable Out_endp interrupt of usb"
	        SCI_TRACE_ID(TRACE_TOOL_CONVERT,MUX_DRIVER_1039_112_2_18_1_49_25_119,(uint8*)"");
        }
#endif
	
#ifdef _U0_SOFTWARE_FLOW_CONTROL
        if ( (1 == gXon_on) && (ring_space(&(mux_port[mux_index].rx_buf)) <= DATA_PORT_REC_BUF_SIZE*3/4) )
        {
	        //MUX_TRACE_WARNING:"MUX_Fill_Rxbuff: Software flow control:send XOFF"
	        SCI_TRACE_ID(TRACE_TOOL_CONVERT,MUX_DRIVER_1046_112_2_18_1_49_25_120,(uint8*)"");
	        gXon_on = 0;
        }
#endif
        //MUX_TRACE_LOW(("MUX_RxForPort: Rx buffer recv length = %d!", rec_num));

        while(rec_num)
        {
            nchar = *data_buf++;
            rec_num--;
#ifdef _U0_SOFTWARE_FLOW_CONTROL
            if ( nchar == XON ) 
            gRemoteXon_on = 1;
            else if ( nchar == XOFF )
            gRemoteXon_on = 0;
#endif


            if (MUX_Ring_Add(&mux_port[mux_index].rx_buf, nchar ) == -1 ) 
            {
                //too long length,it is should not valid at commands!
                //so just clear buffer,else will lead to overrun
                mux_port[mux_index].rx_buf.end_point = mux_port[mux_index].rx_buf.start_point;
                //MUX_TRACE_WARNING:"MUX_Fill_Rxbuff: Rx buffer over writtn!"
                SCI_TRACE_ID(TRACE_TOOL_CONVERT,MUX_DRIVER_1069_112_2_18_1_49_25_121,(uint8*)"");
                continue;
            }


        }
		
       /*Because other application eg.SMS task is lower priroty than MUX Recv task*/
	/*So we need first fill data in RX buff, then set event*/
	if(org_rec_num > 0)
	{
		/*data write to mux_port.rx_buf. set event*/
		if(g_mux_sys_info[mux_index].g_MuxRecv_Eptr)
		{
			MUX_SET_EVENT(g_mux_sys_info[mux_index].g_MuxRecv_Eptr,g_mux_sys_info[mux_index].mux_recvtask_event, MUX_OR);
		}
	}

}

PUBLIC void MUX_EnableFlowCtl(void)
{
	uint32 val = 0;

	if(SIO_ALREADY_OPEN != mux_port[MUX_1].open_flag)
	{
		return;
	}

    SCI_TRACE_LOW("MUX_EnableFlowCtl: Enable Uart0 Flow Control\r\n");

	//Config Uart0 CTS
	val = *(volatile uint32*)PIN_U0CTS_REG;
	val &= ~(BIT_6 | BIT_7);
	*(volatile uint32*)PIN_U0CTS_REG = val;

	//Config Uart0 RTS
	val = *(volatile uint32*)PIN_U0RTS_REG;
	val &= ~(BIT_6 | BIT_7);
	*(volatile uint32*)PIN_U0RTS_REG = val;

    UART_FlowCtlEnable(SIO_PhyPortfromIndex(mux_port[MUX_1].phy_port), HW_FLOW_CONTROL);
}

PUBLIC BOOLEAN  MUX_IsNeedToActivePS(void)
{
    return RM_GetCalibrationPostMode();
}

