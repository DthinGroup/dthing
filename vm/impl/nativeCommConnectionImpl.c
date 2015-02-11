#include <vm_common.h>
#include "nativeCommConnectionImpl.h"

//STATIC_iot_oem_comm_CommConnectionImpl_DEVICE_NORMAL
#define DEVICE_NORMAL 0
//STATIC_iot_oem_comm_CommConnectionImpl_DEVICE_GPS
#define DEVICE_GPS    1

static int s_deviceType = DEVICE_NORMAL;

#if defined(ARCH_ARM_SPD)
#include "sci_types.h"
#include "os_api.h"
#include "com_drvapi.h"
#include "AsyncIO.h"
//#include "gps_drv.h"
#endif

#if defined(ARCH_ARM_SPD)
static int cpl_com_default_init(int port, uint32 rate);
static int cpl_com_default_open(int port);
static int cpl_com_default_close(int port);
static int cpl_com_read(int port, uint8 *buf, uint32 len);
static int cpl_com_write(int port, uint8 *buf, uint32 len);
static void cpl_com_start_reading(int port);
static void cpl_com_stop_reading(int port);
static void cpl_com_start_writing(int port);
static void cpl_com_stop_writing(int port);
#endif

#define GPS_MODE_NORMAL 0
#define GPS_ERR_NONE = 0

int GPS_GetStatus()
{
    //TODO:
    return -1;
}

int GPS_Init()
{
    //TODO:
    return -1;
}

int GPS_Open(int mode)
{
    //TODO:
    return -1;
}

int GPS_Close()
{
    //TODO:
    return -1;
}

int GPS_ReadData(char * read_buf_ptr, int byte_to_read)
{
    //TODO:
    return -1;
}

int GPS_WriteData(char * write_buf_ptr, int byte_to_write)
{
    //TODO:
    return -1;
}

int GPS_SetBaudRate(int bps)
{
    //TODO:
    return -1;
}

/**
 * Class:     iot_oem_comm_CommConnectionImpl
 * Method:    open0
 * Signature: (III)I
 */
void Java_iot_oem_comm_CommConnectionImpl_open0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint port = (jint) args[1];
    jint bps = (jint)args[2];
    jint device = (jint)args[3];
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    if (s_deviceType == DEVICE_GPS)
    {
        int i = 0;
        for (i = 0; i < 10; i++)
        {
            SCI_Sleep(10000);
            ret = GPS_Init();

            if(ret == 0)
                break;
        }
        DthingTraceD("[GPS] do init with result %d and gps status %d\n", ret, GPS_GetStatus());
        ret = GPS_Open(GPS_MODE_NORMAL);
        DthingTraceD("[GPS] do open with result %d and gps status %d\n", ret, GPS_GetStatus());
    }
    else
    {
        s_deviceType = device;

        ret = cpl_com_default_init(port, bps);
        SCI_Sleep(1000);
        cpl_com_default_open(port);
        DthingTraceD("[COM] open COM%d with baudrate %d\n", port, bps);
    }
#endif

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_comm_CommConnectionImpl
 * Method:    getBaudRate0
 * Signature: (I)I
 */
void Java_iot_oem_comm_CommConnectionImpl_getBaudRate0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint port = (jint) args[1];
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    jint speed = UART_GetBaudSpeed(port);

    switch(speed)
    {
    case BAUD_9600:
      ret = 9600;
      break;
    case BAUD_38400:
      ret = 38400;
      break;
    case BAUD_57600:
      ret = 57600;
      break;
    case BAUD_115200:
      ret = 115200;
      break;
    case BAUD_460800:
      ret = 460800;
      break;
    default:
      break;
    }
    DthingTraceD("[COM] COM%d baudrate is %d\n", port, speed);
 #endif
    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_comm_CommConnectionImpl
 * Method:    setBaudRate0
 * Signature: (II)I
 */
void Java_iot_oem_comm_CommConnectionImpl_setBaudRate0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint port = (jint) args[1];
    jint bps = (jint) args[2];
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    if (s_deviceType == DEVICE_GPS)
    {
        GPS_SetBaudRate(bps);
    }
    else
    {
        UART_SetBaudSpeed(port, bps);
    }
    DthingTraceD("[COM] set COM%d baudrate to %d\n", port, bps);
 #endif

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_comm_CommConnectionImpl
 * Method:    close0
 * Signature: (I)I
 */
void Java_iot_oem_comm_CommConnectionImpl_close0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint port = (jint) args[1];
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    if (s_deviceType == DEVICE_GPS)
    {
        if (GPS_ERR_NONE != GPS_Close())
        {
            ret = -1;
        }
    }
    else
    {
        cpl_com_default_close(port);
    }
    DthingTraceD("[COM] close COM%d\n", port);
 #endif

    RETURN_INT(ret);
}

#if defined(ARCH_ARM_SPD)
/**
 * APIs and definitions for uart data access
 */
#define COM_BUFFER_SIZE 2048
#define MAX_UART_BUFFER_SIZE 128

static uint8 s_send_buf[COM_BUFFER_SIZE];       //default send buffer
static uint8 s_rec_buf[COM_BUFFER_SIZE];        //default receive buffer

typedef enum
{
  NOT_OVERWIRTE_QUEUE = 0,
  OVERWIRTE_QUEUE = 1
} QueueOverwriteStatus;

typedef enum {
    COM_SEND,       //send complete
    COM_REC,        //receive complete, no data in uart rx fifo
    COM_REMAINDER,  //receive complete, remainder data in uart rx fifo
    COM_TIMEOUT     //receive timeout, received data number less than expected
} COM_EVENT;

typedef int (*COM_CallBack)(COM_EVENT event, uint32 num);

typedef struct
{
  uint32 port;          //uart port
  uint8* send_buf;        //save send buffer address and size
  uint32 send_len;
  uint32 send_len_done;       //already sent
  COM_CallBack  send_callback;
  uint8* rec_buf;         //save receive buffer address and size
  uint32 rec_len;
  uint32 rec_len_done;      //already received
  COM_CallBack  rec_callback;
  int error;            //lastest error code
} COM_OBJ;

static COM_OBJ uart_object = {
  UART_COM1,           //port 0
  s_send_buf,  //default send buffer
  0,           //none to send
  0,           //none has send
  NULL,        //no callback
  s_rec_buf,   //default receive buffer
  0,
  0,
  NULL,
  0            //no error
};

//Cycle Queue struct
typedef struct
{
  uint8 *buf;
  uint32 size;      //queue volume
  uint32 head;
  uint32 tail;
  unsigned empty: 1;
  unsigned full:  1;
  unsigned overflow:  1;  //queue input after full, overwrite occur
} CycleQueue;

//com receive queue
CycleQueue COM_Input_Q =
{
  s_send_buf,
  COM_BUFFER_SIZE,
  0,
  0,
  1,        //empty
  0,
  0,
};

//com send queue
CycleQueue COM_Output_Q =
{
  s_rec_buf,
  COM_BUFFER_SIZE,
  0,
  0,
  1,        //empty
  0,
  0,
};

static ASYNC_Notifier *dataReadyNotifier = NULL;
static ASYNC_Notifier *writeDoneNotifier = NULL;

static void QueueClean(CycleQueue *Q_ptr);
static int QueueInsert(CycleQueue *Q_ptr, uint8 *data, uint32 len, QueueOverwriteStatus status);
static int QueueDelete(CycleQueue *Q_ptr, uint8 *data, uint32 len);
void default_uart_callback(uint32 event);

/*****************************************************************************/
//  FUNCTION:   QueueClean
//  Description:  clean cycle queue
//  INPUT:    pointer to a CycleQueue
//  OUTPUT:     void
//  Note:
/*****************************************************************************/
static void QueueClean(CycleQueue *Q_ptr)
{
  Q_ptr->head = Q_ptr->tail = 0;
  Q_ptr->empty = 1;
  Q_ptr->full = 0;
  Q_ptr->overflow = 0;
}

/*****************************************************************************/
//  FUNCTION:   QueueInsert(CycleQueue *Q_ptr, uint8 *data, uint32 len)
//  Description:  insert a string in cycle queue
//  INPUT:    Q_ptr--pointer to a CycleQueue, uint8 *data--string pointer, uint32 len--string length
//  OUTPUT:     >=0 actual insert number, <0 error
//  Note:     when queue is full, new data wil overwrite olds.
/*****************************************************************************/
static int QueueInsert(CycleQueue *Q_ptr, uint8 *data, uint32 len, QueueOverwriteStatus status)
{
  int ret = 0;

  if (status == NOT_OVERWIRTE_QUEUE)
  {
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
      //TODO: need more memory
      DthingTraceD("jpl_comm.c: out of queue error\n");
    }

    if ((1 == Q_ptr->empty) && (ret > 0))
    {
      Q_ptr->empty = 0;
    }
  }
  else
  {
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
  }

  return ret;
}


/*****************************************************************************/
//  FUNCTION:   QueueDelete(CycleQueue *Q_ptr, uint8 *data, uint32 len)
//  Description:  delete a string from cycle queue
//  INPUT:    Q_ptr--pointer to a CycleQueue, uint8 *data--string pointer, uint32 len--string length
//  OUTPUT:     >=0 actual delete number, <0 error
//  Note:
/*****************************************************************************/
static int QueueDelete(CycleQueue *Q_ptr, uint8 *data, uint32 len)
{
  int ret = 0;

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

  if ((ret>0) && (1 == Q_ptr->full))
  {
    Q_ptr->full = 0;
    Q_ptr->overflow = 0;
  }

  return ret;
}

static int cpl_com_default_init(int port, uint32 rate)
{
  int ret = 0;
  UART_INIT_PARA_T uart_info = {0};

  uart_info.tx_watermark  = 8; // 0~127B
  uart_info.rx_watermark  = 48; // 0~127B

  switch(rate)
  {
  case 38400:
    uart_info.baud_rate   = BAUD_38400;
    break;
  case 57600:
    uart_info.baud_rate   = BAUD_57600;
    break;
  case 115200:
    uart_info.baud_rate   = BAUD_115200;
    break;
  case 9600:
    uart_info.baud_rate   = BAUD_9600;
    break;
  default:
    uart_info.baud_rate   = BAUD_115200;
    break;
  }

  uart_info.parity    = FALSE;
  uart_info.parity_enable = PARITY_DISABLE;
  uart_info.byte_size   = EIGHT_BITS;
  uart_info.stop_bits   = ONE_STOP_BIT;
  uart_info.flow_control  = NO_FLOW_CONTROL;
  uart_info.ds_wakeup_en  = DS_WAKEUP_DISABLE;

  uart_object.port = port;


  //assume u1rxd/u1txd have selected in pinmap
  ret = UART_Initilize(port, &uart_info, default_uart_callback);
  cpl_com_stop_reading(port);
  cpl_com_stop_writing(port);
  QueueClean(&COM_Input_Q);
  QueueClean(&COM_Output_Q);
  return ret;
}

static int cpl_com_default_open(int port)
{
  cpl_com_start_reading(port);
  return 0;
}

static int cpl_com_default_close(int port)
{
  UART_INIT_PARA_T  uart_info = {0};

  uart_info.tx_watermark  = 96; // 0~127B
  uart_info.rx_watermark  = 8; // 0~127B
  uart_info.baud_rate   = BAUD_115200;
  uart_info.parity    = FALSE;
  uart_info.parity_enable = PARITY_DISABLE;
  uart_info.byte_size   = EIGHT_BITS;
  uart_info.stop_bits   = ONE_STOP_BIT;
  uart_info.flow_control  = NO_FLOW_CONTROL;
  uart_info.ds_wakeup_en  = DS_WAKEUP_DISABLE;

  uart_object.port = port;
  uart_object.rec_len = 0;

  //assume u1rxd/u1txd have selected in pinmap
  return UART_Close(port);
}

static int cpl_com_read(int port, uint8 *buf, uint32 len)
{
  int rec = 0;
  COM_OBJ *pCom = &uart_object;

  if (port != pCom->port)
  {
    rec = -1;
    goto end;
  }

  cpl_com_stop_reading(pCom->port);
  rec = QueueDelete(&COM_Input_Q, buf, len);
  pCom->rec_len = (rec > 0)? (pCom->rec_len - rec) : 0;
  cpl_com_start_reading(pCom->port);

end:
  return rec;
}

static int cpl_com_write(int port, uint8 *buf, uint32 len)
{
  int ret = 0;
  COM_OBJ *pCom = &uart_object;

  if (port != pCom->port)
  {
    ret = -1;
    goto end;
  }

  cpl_com_stop_writing(pCom->port);
  ret = QueueInsert(&COM_Output_Q, buf, len, NOT_OVERWIRTE_QUEUE);
  cpl_com_start_writing(pCom->port);

end:
  return ret;
}

static void cpl_com_start_reading(int port)
{
  UART_Rx_Int_Enable(port, TRUE);
}

static void cpl_com_stop_reading(int port)
{
  UART_Rx_Int_Enable(port, FALSE);
}

static void cpl_com_start_writing(int port)
{
  UART_Tx_Int_Enable(port, TRUE);
}

static void cpl_com_stop_writing(int port)
{
  UART_Tx_Int_Enable(port, FALSE);
}

void default_uart_callback(uint32 event)
{
  uint8 tmp_buf[MAX_UART_BUFFER_SIZE] = {0};
  uint32 cnt_old = 0;
  uint32 cnt = 0;
  COM_OBJ *pcom = &uart_object;
  COM_EVENT status = {0};

  switch (event)
  {
  case EVENT_DATA_TO_READ:
    cnt_old = UART_GetRxFifoCnt(pcom->port);
    cnt = UART_ReadData(pcom->port, tmp_buf, MAX_UART_BUFFER_SIZE);
    pcom->rec_len_done += cnt;
    pcom->rec_len += cnt;
    DthingTraceD("[COM%d] EVENT_DATA_TO_READ data[%d]:%s\n", pcom->port, cnt, tmp_buf);
    QueueInsert(&COM_Input_Q, tmp_buf, cnt, NOT_OVERWIRTE_QUEUE);
    break;

  case EVENT_WRITE_COMPLETE:
    cnt = UART_GetTxFifoCnt(pcom->port);
    cnt = QueueDelete(&COM_Output_Q, tmp_buf, cnt);

    DthingTraceD("[COM%d] EVENT_WRITE_COMPLETE data[%d]:%s\n", pcom->port, cnt, tmp_buf);

    if (cnt > 0)
    {
      UART_WriteData(pcom->port, tmp_buf, cnt);
    }
    else
    {
      cpl_com_stop_writing(pcom->port);
    }
    break;

  case EVENT_INIT_COMPLETE:
    DthingTraceD("[COM%d] EVENT_INIT_COMPLETE\n", pcom->port);
    break;

  case EVENT_SHUTDOWN_COMPLETE:
    DthingTraceD("[COM%d] EVENT_SHUTDOWN_COMPLETE\n", pcom->port);
    break;

  default:
    DthingTraceD("jpl_comm.c: UNKNOWN UART EVENT\n");
    break;
  }
}
#endif //ARCH_ARM_SPD

/**
 * Class:     iot_oem_comm_CommConnectionImpl
 * Method:    writeBytes0
 * Signature: (I[BII)I
 */
void Java_iot_oem_comm_CommConnectionImpl_writeBytes0(const u4*args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint port = (jint) args[1];
    ArrayObject * dataArr = (ArrayObject *)args[2];
    jbyte * dataArrPtr = (jbyte *)(KNI_GET_ARRAY_BUF(args[2]));
    int dataArrLen = KNI_GET_ARRAY_LEN(args[2]);
    jint offset = (jint)args[3];
    jint len = (jint)args[4];
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    uint8 *data = (uint8 *)(dataArrPtr + offset);

    if (AsyncIO_firstCall())
    {
        writeDoneNotifier = Async_getCurNotifier();
        if (s_deviceType == DEVICE_GPS)
        {
            ret = GPS_WriteData(data, len);
            DthingTraceD("[GPS] write gps data with result[%d]\n", ret);
        }
        else
        {
            ret = cpl_com_write(port, data, len);
        }
        DthingTraceD("[COM%d] write data with result[%d]\n", port, ret);
        //FIXME: Implement async write function
        writeDoneNotifier = NULL;
    }
#endif

    RETURN_INT(ret);

}

/**
 * Class:     iot_oem_comm_CommConnectionImpl
 * Method:    readBytes0
 * Signature: (I[BII)I
 */
void Java_iot_oem_comm_CommConnectionImpl_readBytes0(const u4*args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint port = (jint) args[1];
    ArrayObject * bufArr = (ArrayObject *)args[2];
    jbyte * bufArrPtr = (jbyte *)(KNI_GET_ARRAY_BUF(args[2]));
    int bufArrLen = KNI_GET_ARRAY_LEN(args[2]);
    jint offset = (jint)args[3];
    jint len = (jint)args[4];
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    uint8 *buf = (uint8 *)(bufArrPtr + offset);

    if (AsyncIO_firstCall())
    {
        if (s_deviceType == DEVICE_GPS)
        {
            ret = GPS_ReadData(buf, len);
            DthingTraceD("[GPS] read gps data with result[%d]\n", ret);
        }
        else
        {
            if (uart_object.rec_len > 0)
            {
                ret = cpl_com_read(port, buf, len);
                DthingTraceD("[COM%d] read data with result[%d]\n", port, ret);
            }
        }
        //FIXME: Implement async read function
        dataReadyNotifier = NULL;
    }
#endif
    RETURN_INT(ret);
}
