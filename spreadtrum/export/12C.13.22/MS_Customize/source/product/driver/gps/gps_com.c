/******************************************************************************
 ** File Name:    Gps_SiRF.c                                                     *
 ** Author:       David.Jia                                                 *
 ** DATE:         7/11/2007                                                   *
 ** Copyright:    2005 Spreatrum, Incoporated. All Rights Reserved.           *
 ** Description:                                                              *
 *****************************************************************************/
/******************************************************************************
 **                   Edit    History                                         *
 **---------------------------------------------------------------------------* 
 ** DATE          NAME            DESCRIPTION                                 * 
 ** 06/19/2007      David.Jia       Test uartcom_drv interface.
 ** 06/23/2007      David.Jia       add data to cycle queue, in the uartcom_drv's callback.
 ** 06/26/2007      David.Jia       implement map application interface--Map_Read/Write,
 **     Calibration interface--Cal_Read/Write.
 ** 07/11/2007      David.Jia       move uartcom_drv and Map_Read/Write to Gps_SiRF.h/c.
 ** 08/02/2007      David.Jia       Change names of COM_Init/COM_Close/Map_Read/Map_Write 
 **     to GPS_ComInit/GPS_ComClose/GPS_ComRead/GPS_ComWrite.
 *****************************************************************************/
/*----------------------------------------------------------------------------*
**                        Dependencies                                        *
**---------------------------------------------------------------------------*/
#include "ms_customize_trc.h"
#include "os_api.h"
#include"com_drv.h"
/**---------------------------------------------------------------------------*
**                        Compiler Flag                                       *
**---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*
**                            Mcaro Definitions                               *
**---------------------------------------------------------------------------*/
#define COM_BUFFER_SIZE 1048        //default com send/receive buffer size
#define GPS_TX_WATER_MARK	8
#define GPS_RX_WATER_MARK	48
#define GPS_GET_DELAY       50      //@David.Jia 2007.7.12 get command fequency 20Hz
#define GPS_COM   2               //uart port

typedef struct {
    uint8 *buf;
    uint32 size;        //queue volume
    uint32 head;
    uint32 tail;
    unsigned empty: 1;
    unsigned full:  1;
    unsigned overflow:  1;  //queue input after full, overwrite occur
}   CycleQueue;

typedef enum {
    COM_SEND,       //send complete
    COM_REC,        //receive complete, no data in uart rx fifo
    COM_REMAINDER,  //receive complete, remainder data in uart rx fifo
    COM_TIMEOUT     //receive timeout, received data number less than expected
} COM_EVENT; 

typedef int (*COM_CallBack)(COM_EVENT event, uint32 num);   


typedef struct {
    uint32		 port;                    //uart port
    uint8*		 send_buf;                //save send buffer address and size
    uint32 		 send_len;    
    uint32 		 send_len_done;           //already sent
    COM_CallBack send_callback;
    uint8* 		 rec_buf;                 //save receive buffer address and size
    uint32 	  	 rec_len;
    uint32 		 rec_len_done;            //already received
    COM_CallBack rec_callback;
    int 		 error;                      //lastest error code
} COM_OBJ;


/*----------------------------------------------------------------------------*
**                             Data Structures                                *
**---------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
**                             Local Vars                                     *
**---------------------------------------------------------------------------*/
static uint8 s_send_buf[COM_BUFFER_SIZE];       //default send buffer
static uint8 s_rec_buf[COM_BUFFER_SIZE];        //

//com receive queue
CycleQueue Input_Q = {
    s_send_buf,
    COM_BUFFER_SIZE,
    0,
    0,
    1,              //empty
    0,         
    0,
};

//com send queue
CycleQueue Output_Q = {
    s_rec_buf,
    COM_BUFFER_SIZE,
    0,
    0,
    1,              //empty
    0,         
    0,
};


COM_OBJ gps_com_ins = {         //static instance of com 1
    GPS_COM,              //port 0
    s_send_buf,     //default send buffer
    0,              //none to send
    0,              //none has send
    NULL,           //no callback
    s_rec_buf,      //default receive buffer
    0,
    0,
    NULL,
    0               //no error
};

/*----------------------------------------------------------------------------*
**                         Local Function Prototype                           *
**---------------------------------------------------------------------------*/
static void QueueClean(CycleQueue *Q_ptr);
static int QueueInsert(CycleQueue *Q_ptr, uint8 *data, uint32 len);
static int QueueDelete(CycleQueue *Q_ptr, uint8 *data, uint32 len);

/*----------------------------------------------------------------------------*
**                         Function Definitions                               *
**---------------------------------------------------------------------------*/
/*****************************************************************************/
//  FUNCTION:     QueueClean
//  Description:    clean cycle queue
//  INPUT:      pointer to a CycleQueue
//  OUTPUT:     void
//  Author:         David.Jia
//  date:           2007.6.23
//	Note:       
/*****************************************************************************/
static void QueueClean(CycleQueue *Q_ptr)
{
    Q_ptr->head = Q_ptr->tail = 0;
    Q_ptr->empty = 1;
    Q_ptr->full = 0;
    Q_ptr->overflow = 0;    
}

/*****************************************************************************/
//  FUNCTION:     QueueInsert(CycleQueue *Q_ptr, uint8 *data, uint32 len)
//  Description:    insert a string in cycle queue
//  INPUT:          Q_ptr--pointer to a CycleQueue, uint8 *data--string pointer, uint32 len--string length
//  OUTPUT:         >=0 actual insert number, <0 error
//  Author:         David.Jia
//  date:           2007.6.23
//	Note:        when queue is full, new data wil overwrite olds.
/*****************************************************************************/
static int QueueInsert(CycleQueue *Q_ptr, uint8 *data, uint32 len)
{
    uint32 ret = 0;
#if 1       //no overwrite    //KST DEBUG
    if (!Q_ptr->full)
    {
        while(ret < len)
        {
            *(Q_ptr->buf + Q_ptr->head) = *(data + ret);
            Q_ptr->head = ++Q_ptr->head % Q_ptr->size;
            ret++;
            
            if (Q_ptr->head == Q_ptr->tail)
            {
                Q_ptr->full = 1;
                break;
            }
        }
   }
   else
   {
        /*needing more buffer,for test stage*/
		SCI_ASSERT(0); /*assert verified*/
   }
    if ((1==Q_ptr->empty) && (ret > 0)) 
    {
        Q_ptr->empty = 0;
    }
#else       //overwrite
    for (; ret < len; ret++)
    {
        *(Q_ptr->buf + Q_ptr->head) = *(data + ret);
        
        if ((1==Q_ptr->full) && (Q_ptr->head==Q_ptr->tail))
        {
            Q_ptr->overflow = 1;               
        }
        
        Q_ptr->head = ++Q_ptr->head % Q_ptr->size;
        
        if (Q_ptr->head == Q_ptr->tail)
        {
            Q_ptr->full = 1;
        }
        
        if (1 == Q_ptr->empty)
        {
            Q_ptr->empty = 0;
        }
    }
    
    if (Q_ptr->overflow)
    {
        Q_ptr->tail = Q_ptr->head;
    }
#endif
   
    return ret;       
}


/*****************************************************************************/
//  FUNCTION:     QueueDelete(CycleQueue *Q_ptr, uint8 *data, uint32 len)
//  Description:    delete a string from cycle queue
//  INPUT:          Q_ptr--pointer to a CycleQueue, uint8 *data--string pointer, uint32 len--string length
//  OUTPUT:         >=0 actual delete number, <0 error
//  Author:         David.Jia
//  date:           2007.6.23
//	Note:       
/*****************************************************************************/
static int QueueDelete(CycleQueue *Q_ptr, uint8 *data, uint32 len)
{
    uint32 ret = 0;
    
    if (!Q_ptr->empty)
    {
        while(ret < len) 
        {
            *(data + ret) = *(Q_ptr->buf + Q_ptr->tail);
            Q_ptr->tail = ++Q_ptr->tail % Q_ptr->size;
            ret++;
            
            if (Q_ptr->tail == Q_ptr->head)
            {
                Q_ptr->empty = 1;
                break;
            }
        }
    }
   
    if ((ret>0) && (1==Q_ptr->full))   
    {
        Q_ptr->full = 0;
        Q_ptr->overflow = 0;
    }
    
    return ret;    
}

/*****************************************************************************/
//  FUNCTION:     uart1_callback(uint32 event)
//  Description:    callback function for uartcom_drv.
//  Author:         David.Jia
//  date:           2007.6.19
//	Note:           assume there is still data in when call UART_ReadData.
//      return value will large than called UART_GetRxFifoCnt.
/*****************************************************************************/

static void gps_uart_callback(uint32 event)
{
	uint8 tmp_buf[128+1];
    uint32 cnt_old, cnt;
    COM_OBJ *pcom = &gps_com_ins;
    
    //SCI_ASSERT(event < COM_MAX_EVENT);
    //SCI_TraceLow("event=%x",event);
    switch (event)
    {
        case EVENT_DATA_TO_READ:    
            {      
                       
            cnt_old = UART_GetRxFifoCnt(pcom->port);            
            cnt = UART_ReadData(pcom->port, tmp_buf, 128);            
            pcom->rec_len_done += cnt;
            
            QueueInsert(&Input_Q, tmp_buf, cnt);
            
            //SCI_TraceLow("\r\nuart1_callback:cnt=0x%x 0x%x,0x%x,0x%x,0x%x", cnt,tmp_buf[0], tmp_buf[1],tmp_buf[2],tmp_buf[3]);
                       
            break;
        }
        
        case EVENT_WRITE_COMPLETE:  
        {
            cnt = UART_GetTxFifoCnt(pcom->port);

            /*SCI_TraceLow("event event cnt=%x",cnt);*/
            
            cnt = QueueDelete(&Output_Q, tmp_buf, cnt);
            UART_WriteData(pcom->port, tmp_buf, cnt);

            if (0 == cnt) 
            UART_Tx_Int_Enable(pcom->port, FALSE);

            break;    
        }
        
        case EVENT_INIT_COMPLETE:
            break;
        
        case EVENT_SHUTDOWN_COMPLETE:
            break;
        
        default:
            break;            
    }
}


/**---------------------------------------------------------------------------*/
//                            Global Function Definitions
/**---------------------------------------------------------------------------*/



/*****************************************************************************/
//  FUNCTION:     GPS_ComRead
//  Description:    read string from uart (initialized by COM_Init)
//  INPUT:          uint8* buf--data buffer alocated by caller, uint32 len--bytes number wish to read.
//  OUTPUT:         read byte number in fact.
//  Author:         David.Jia
//  date:           2007.6.26
//	Note:          
/*****************************************************************************/
int GPS_ComRead(uint8 *buf, uint32 len)
{
    int rec;
    COM_OBJ *pCom = &gps_com_ins;
    
    UART_Rx_Int_Enable(pCom->port, FALSE);
    rec = QueueDelete(&Input_Q, buf, len);
    UART_Rx_Int_Enable(pCom->port, TRUE);
    //SCI_TRACE_LOW("[GPS_ComRead]:%d,%d,%d,%d",*buf,*(buf+1),*(buf+2),*(buf+3));
    return rec;
}

/*****************************************************************************/
//  FUNCTION:     GPS_ComWrite
//  Description:    write string to uart (initialized by COM_Init)
//  INPUT:          uint8* buf--data buffer alocated by caller, uint32 len--bytes number wish to read.
//  OUTPUT:         write byte number in fact.
//  Author:         David.Jia
//  date:           2007.6.26
//	Note:          if len > Output_Q.size, data will overwrite.
/*****************************************************************************/
int GPS_ComWrite(uint8 *buf, uint32 len)
{
    int ret;
    COM_OBJ *pCom = &gps_com_ins;
    UART_Tx_Int_Enable(pCom->port, FALSE);
    ret = QueueInsert(&Output_Q, buf, len);
    UART_Tx_Int_Enable(pCom->port, TRUE);
    
    return ret;
}

//@David.Jia 2007.6.26  end

/*****************************************************************************/
//  FUNCTION:     GPS_ComInit
//  Description:    pin select of uart 1, and initialize uart 1.
//  return:         0--ok, others--error.
//  Author:         David.Jia
//  date:           2007.6.19
//	Note:           call before using uart 1.
/*****************************************************************************/
int GPS_ComInit(uint32 BaudRate)
{
    int ret; 
    
    UART_INIT_PARA_T    uart_st;
    
    //SCI_TRACE_LOW:"\r\nGPS_ComInit.BaudRate=%d"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_COM_351_112_2_18_0_33_6_1549,(uint8*)"d",BaudRate);

    uart_st.tx_watermark  = GPS_TX_WATER_MARK; // 0~127B
    uart_st.rx_watermark  = GPS_RX_WATER_MARK; // 0~127B
    switch(BaudRate)
    {
        case 38400:
        	uart_st.baud_rate     = BAUD_38400;
        break;
        case 57600:
        	uart_st.baud_rate     = BAUD_57600;
        break;	
        case 115200:
        	uart_st.baud_rate     = BAUD_115200;
        break;
        default:
        uart_st.baud_rate     = BAUD_115200;
        break;
    }
	
    uart_st.parity        = FALSE; 
    uart_st.parity_enable = PARITY_DISABLE;
    uart_st.byte_size     = EIGHT_BITS;
    uart_st.stop_bits     = ONE_STOP_BIT;
    uart_st.flow_control  = NO_FLOW_CONTROL;
    uart_st.ds_wakeup_en  = DS_WAKEUP_DISABLE;
    
    gps_com_ins.port = GPS_COM;
    
    ret = UART_Initilize(GPS_COM, &uart_st, gps_uart_callback);
    QueueClean(&Input_Q);
    QueueClean(&Output_Q);
    UART_Tx_Int_Enable(GPS_COM, FALSE);
    
    return ret;
}

int GPS_SetBaudRate(uint32 baudrate_bps)
{
	uint32 baudrate;
		
	switch(baudrate_bps)
	{
		case 38400:
			baudrate     = BAUD_38400;
			break;
		case 57600:
			baudrate     = BAUD_57600;
			break;	
		case 115200:
			baudrate     = BAUD_115200;
			break;
		default:
			baudrate     = BAUD_115200;
			break;
	}
	
	UART_SetBaudSpeed(GPS_COM,baudrate);
	return 0;
}


/*****************************************************************************/
//  FUNCTION:     GPS_ComClose
//  Description:    pin deselect of uart 1, and close uart 1.
//  return:         0--ok, others--error.
//  Author:         David.Jia
//  date:           2007.6.19
//	Note:           call after using uart 1.
/*****************************************************************************/
int GPS_ComClose(void)
{
    UART_INIT_PARA_T    uart1_st;
    
    //SCI_TRACE_LOW:"\r\nGPS_ComClose."
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_COM_423_112_2_18_0_33_6_1550,(uint8*)"");
    
    uart1_st.tx_watermark  = GPS_TX_WATER_MARK; // 0~127B
    uart1_st.rx_watermark  = GPS_RX_WATER_MARK; // 0~127B
    uart1_st.baud_rate     = BAUD_115200;
    //uart1_st.baud_rate     = BAUD_57600;
    uart1_st.parity        = FALSE; 
    uart1_st.parity_enable = PARITY_DISABLE;
    uart1_st.byte_size     = EIGHT_BITS;
    uart1_st.stop_bits     = ONE_STOP_BIT;
    uart1_st.flow_control  = NO_FLOW_CONTROL;
    uart1_st.ds_wakeup_en  = DS_WAKEUP_DISABLE;
    
    gps_com_ins.port = GPS_COM;
    
    //assume u1rxd/u1txd have selected in pinmap    
    return UART_Close(GPS_COM);
}



