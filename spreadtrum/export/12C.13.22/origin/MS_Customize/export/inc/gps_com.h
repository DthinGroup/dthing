/******************************************************************************
 ** File Name:    Gps_SiRF.h                                                     *
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
 ** 08/02/2007      David.Jia       Change names of COM_Init/COM_Close/Map_Read/Map_Write 
 **     to GPS_ComInit/GPS_ComClose/GPS_ComRead/GPS_ComWrite.
 *****************************************************************************/
#include "com_drv.h"
#ifndef GPS_COM_H
#define GPS_COM_H

#define COM_0   0
#define COM_1   1               //uart port

/*----------------------------------------------------------------------------*
**                             Data Structures                                *
**---------------------------------------------------------------------------*/
typedef enum {
    COM_SEND,       //send complete
    COM_REC,        //receive complete, no data in uart rx fifo
    COM_REMAINDER,  //receive complete, remainder data in uart rx fifo
    COM_TIMEOUT     //receive timeout, received data number less than expected
} COM_EVENT; 

/*****************************************************************************/
//  FUNCTION:     COM_CallBack
//  Description:    callback function for com communication. called after
//      sent all bytes ,after received all bytes, receive timeout and error.
//      called in ISR, so only send a event to COM TASK and return.
//  INPUT:          event--indetify which event occur, num--bytes done.
//  return:         0--ok, others--error.
//  Author:         David.Jia
//  date:           2007.6.19
//	Note:           
/*****************************************************************************/
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

extern COM_OBJ com1_ins;

//@David.Jia 2007.6.23  begin
//Cycle Queue struct
typedef struct {
    uint8 *buf;
    uint32 size;        //queue volume
    uint32 head;
    uint32 tail;
    unsigned empty: 1;
    unsigned full:  1;
    unsigned overflow:  1;  //queue input after full, overwrite occur
}   CycleQueue;


/*****************************************************************************/
//  FUNCTION:     GPS_ComRead
//  Description:    read string from uart (initialized by COM_Init)
//  INPUT:          uint8* buf--data buffer alocated by caller, uint32 len--bytes number wish to read.
//  OUTPUT:         read byte number in fact.
//  Author:         David.Jia
//  date:           2007.6.26
//	Note:          
/*****************************************************************************/
int GPS_ComRead(uint8 *buf, uint32 len);

/*****************************************************************************/
//  FUNCTION:     GPS_ComWrite
//  Description:    write string to uart (initialized by COM_Init)
//  INPUT:          uint8* buf--data buffer alocated by caller, uint32 len--bytes number wish to read.
//  OUTPUT:         write byte number in fact.
//  Author:         David.Jia
//  date:           2007.6.26
//	Note:          if len > Output_Q.size, data will overwrite.
/*****************************************************************************/
int GPS_ComWrite(uint8 *buf, uint32 len);

/*****************************************************************************/
//  FUNCTION:     GPS_ComInit
//  Description:    pin select of uart 1, and initialize uart 1.
//  return:         0--ok, others--error.
//  Author:         David.Jia
//  date:           2007.6.19
//	Note:           call before using uart 1.
/*****************************************************************************/
int GPS_ComInit(uint32 BaudRate);

/*****************************************************************************/
//  FUNCTION:     GPS_ComClose
//  Description:    pin deselect of uart 1, and close uart 1.
//  return:         0--ok, others--error.
//  Author:         David.Jia
//  date:           2007.6.19
//	Note:           call after using uart 1.
/*****************************************************************************/
int GPS_ComClose(void);




#endif  //GPS_COM_H