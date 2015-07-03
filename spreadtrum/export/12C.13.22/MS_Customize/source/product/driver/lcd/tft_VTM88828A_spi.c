/******************************************************************************
 ** File Name:     tft_VTM88828A_spi.c                                        *
 ** Description:                                                              *
 **    This file contains driver for color LCD.(CSTN)                         *
 ** Author:                                                                   *
 ** DATE:                                                                     *
 ** Copyright:      2015 Yalung Soft, Incoporated. All Rights Reserved.       *
 ** Description:                                                              *
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                       *
 ** ------------------------------------------------------------------------- *
 ** DATE                NAME             DESCRIPTION                               *
 **                                      Create.
 ******************************************************************************/

#include "ms_customize_trc.h"
#include "os_api.h"
#include "dal_lcd.h"
#include "lcd_cfg.h"
#include "spi_drvapi.h"
#include "gpio_drvapi.h"

/**---------------------------------------------------------------------------*
 **                            Macro Define
 **---------------------------------------------------------------------------*/

#ifdef   __cplusplus
    extern   "C"
    {
#endif

#define SCI_TraceLow_Test SCI_TraceLow

/*lint -save -e767 -e718*/
#define LCD_CtrlWrite_VTM88828A( _cmd )\
        LCD_SendCmd( (_cmd ), 0 );

#define LCD_DataWrite_VTM88828A( _data )\
        LCD_SendData( (_data), 0 );

#define LCD_DataRead_VTM88828A( _cmd )\
        LCD_ReadRegVal( 0, (_cmd))

#define LCD_CtrlData_VTM88828A(cmd, data)  LCD_CtrlWrite_VTM88828A( cmd );  \
                                          LCD_DataWrite_VTM88828A( data );

/*lint -restore*/
#define LCD_Delay( ticks)\
        OS_TickDelay( ticks );

#define VTM88828A_SUPPORT_WIDTH   128
#define VTM88828A_SUPPORT_HEIGHT  64
#define VTM88828A_Y_PAGE_SIZE 8 //bits
#define VTM88828A_Y_PAGES ((VTM88828A_SUPPORT_HEIGHT + VTM88828A_Y_PAGE_SIZE - 1) / VTM88828A_Y_PAGE_SIZE)
#define DISPLAY_BUF_SIZE ((VTM88828A_SUPPORT_WIDTH * VTM88828A_SUPPORT_HEIGHT) / VTM88828A_Y_PAGE_SIZE)

LOCAL LCD_DIRECT_E s_lcd_direct = LCD_DIRECT_NORMAL;

typedef enum
{
  SDK_LCD_CMD = 0,
  SDK_LCD_DATA =1  
}sdk_lcd_opmode;

#define LCD_DISPLAY_ON 0xAF  //Open lcd display
#define LCD_DISPLAY_OFF 0xAE //Close lcd display

/***************************************************************
                  SPI Define
***************************************************************/
typedef int (*spi_handle_f)(void* para);

typedef struct
{
  uint8   rx_shift_edge; // "1" Enable Rx data shift in at clock neg-edge
  uint8   sck_reverse;   // "1" reverse the sck
  uint8   lsb_first;     // "1" enable data transmit/receive from LSB
  uint8   tx_bit_length; // Transmit data bit number
  uint32  freq;          // SPI bus clock
} MDP_SPI_CFG_S;

typedef struct Spi_Body_Tag
{
  int    handle;
  MDP_SPI_CFG_S cfg;
}Spi_Body_T;


typedef enum//SPI NO
{
  SPI_NO_0 = 0,
  SPI_NO_1,

  SPI_NO_MAX
}spi_no_E;

typedef enum
{
  SPI_IOCTL_DMA = 1,/*DMA MODE*/
  SPI_IOCTL_POLL/*POLL MODE*/
}spi_ioctl_mode_e;

typedef enum
{
  SPI_MASTER = 1,
  SPI_SLAVER
}spi_ms_mode_e;

typedef enum
{
  SPI_MSB_FIRST = 1,
  SPI_LSB_FIRST
}spi_bit_pri_e;

typedef enum
{
  SPI_BL_8BIT = 1,
  SPI_BL_16BIT
}spi_bitlen_e;

typedef enum
{
  SPI_DATA_HH = 1,
  SPI_DATA_HL,
  SPI_DATA_LH,
  SPI_DATA_LL
}spi_data_trans_e;

typedef struct
{
  spi_ioctl_mode_e  ioctrlMode;
  spi_ms_mode_e modeSel;
  uint16       divider;
  spi_bit_pri_e   bitPri;
  spi_bitlen_e    bitLen;
  spi_data_trans_e  datatrans;
}spi_config_t;

#define LCD_SPI (SPI_NO_1)
#define LCD_RST_PIN  59 //GPIO_59
#define LCD_CMD_DATA_PIN 49 //GPIO_49

#define GPIO_INPUT_DIRECTION  0
#define GPIO_OUTPUT_DIRECTION 1
#define PIN_HIGH 1
#define PIN_LOW 0

static int g_spiRdHandle = 0;
static int g_rstRdHandle = 0;
static int g_dcRdHandle = 0;
static uint8 *g_lcd_pixels_buf = NULL;


static spi_handle_f spiHandlef = NULL;//SPI回调
static Spi_Body_T spiBody[SPI_NO_MAX] = {0};//SPI结构表

int spiOpen(uint32 num);
int spiClose(int Handle);
int spiConfig(int DevHandle,spi_config_t* pConfig);
int spiRead(int DevHandle,uint8* pchTxBuf,uint8* pchUserBuf,uint32 dwCount);
int spiWrite(int DevHandle,uint8* pchUserBuf,uint32 length);
int spiWrite0(int DevHandle, uint8* pchUserBuf, uint32 length);
int spiSubscribe(spi_handle_f SpiHandle);
int spiUnSubscribe(void);

void custLcdRstPinSet(uint8 val);
void custLcdCmdDataSet(uint8 val);
static int getSpiNo(int Handle, uint8* no_p);

void set_lcd_col(uint8 colum);
//设置显示起始行
void set_lcd_line(uint8 line);
//设置显示页地址
void  set_lcd_page(uint8 pageNum);

  /******************************************************************************/
//  Description:   Close the lcd.(include sub lcd.)
//  Global resource dependence:
//  Author:
//  Note:
/******************************************************************************/
LOCAL void VTM88828A_Close(void);

  /******************************************************************************/
//  Description:   Enter/Exit sleep mode .
//  Global resource dependence:
//  Author:
//  Note:
/******************************************************************************/
LOCAL ERR_LCD_E  VTM88828A_EnterSleep(
  BOOLEAN is_sleep  //SCI_TRUE: exter sleep mode;SCI_FALSE:exit sleep mode.
  );

/*********************************************************************/
//  Description:   Initialize color LCD : HX8347
//  Input:
//      None.
//  Return:
//      None.
//  Note:
/*********************************************************************/
LOCAL ERR_LCD_E VTM88828A_Init(void);

/******************************************************************************/
//  Description:   invalidate a rectang of in LCD
//  Global resource dependence:
//  Author:
//  Note:
/******************************************************************************/
LOCAL ERR_LCD_E VTM88828A_Invalidate(void);

/******************************************************************************/
//  Description:   Copy a retangle data from clcd_buffer to display RAM.
//                     then the rectangle display is to be refreshed
//  Global resource dependence:
//  Author:
//  Note:
//     To improve speed, lcd is operate in HIGH SPEED RAM WRITE MODE(4
//     uint16 are write continuously always.) So, some dummy uint16
//     should be inserted to satisfy this mode.   Please refer to spec.
/******************************************************************************/
LOCAL ERR_LCD_E VTM88828A_InvalidateRect(
  uint16 left,  //the left value of the rectangel
  uint16 top,   //top of the rectangle
  uint16 right,   //right of the rectangle
  uint16 bottom //bottom of the rectangle
  );
/******************************************************************************/
//  Description:   Set the windows address to display, in this windows
//                 color is  refreshed.
//  Global resource dependence:
//  Author:
//  Note:
/******************************************************************************/
LOCAL void VTM88828A_set_display_window(
  uint16 left,  // start Horizon address
  uint16 right,   // end Horizon address
  uint16 top,     // start Vertical address
  uint16 bottom // end Vertical address
  );


/**---------------------------------------------------------------------------*
 **                         Utils for SPI                                     *
 **---------------------------------------------------------------------------*/
/***************************************************************
                  以下定义本文件中使用的私有函数
***************************************************************/
/*
 * 功能 : 获取指定句柄对应的NO号
 * Handle : 设备句柄
 * no_p : 编号
 * 返回 : 句柄非法返回RET_ERR_INVALID_HANDLE,
 *               句柄没有启用返回RET_ERR_NOT_OPENED
 *               否则返回SCI_SUCCESS
 */
static int getSpiNo(int Handle, uint8* no_p)
{
  uint8 iTemp = 0;

  *no_p = 0;

  if(Handle <= 0)
  {
    return SCI_ERROR;
  }

  for(iTemp = SPI_NO_0; iTemp < SPI_NO_MAX; iTemp++)
  {
    if(spiBody[iTemp].handle == Handle)
    {
      *no_p = iTemp;
      return SCI_SUCCESS;
    }
  }

  return SCI_ERROR;
}



/***************************************************************
                  以下定义本文件中使用的公共函数
***************************************************************/
/*
 * 功能 : SPI始化
 * 说明 : 供平台调用
 * 返回 : 成功或者失败
 */
BOOLEAN m2mHalSpiInit(VOID)
{
  //参数初始化
  spiHandlef = NULL;
  memset(spiBody, 0x00, sizeof(spiBody));

  return SCI_TRUE;
}

/*
 * 功能 : 打开SPI
 * num : SPI通道标识
 * 返回 : 成功时返回对应的句柄(num + 1),失败时返回原因
 */
int spiOpen(uint32 num)
{
  int ret = 0;

  switch(num)
  {
    case SPI_NO_1:
      {
        if(spiBody[num].handle > 0)
        {
          ret = SCI_ERROR;
        }
        else
        {
          spiBody[num].handle = (int)(num + 1);
          ret = spiBody[num].handle;
        }
      }
      break;

    default:
      {
        ret = SCI_ERROR;
      }
      break;
  }

  return ret;
}

/*
 * 功能 : 关闭SPI
 * Handle : 设备句柄
 * 返回 : 成功时返回RET_SUCCES, 失败时返回原因
 */
int spiClose(int Handle)
{
  int ret;
  uint8 spiNo;

  ret = getSpiNo(Handle,&spiNo);
  if(ret == SCI_SUCCESS)
  {
    memset(&spiBody[spiNo], 0x00, sizeof(Spi_Body_T));
  }

  return(ret);
}

/*
 * 功能 : SPI配置
 * DevHandle : 设备句柄
 * pConfig : SPI配置
 * 返回 : 成功时返回RET_SUCCES,失败时返回原因
 */
int spiConfig(int DevHandle, spi_config_t* pConfig)
{
  int ret = 0;
  uint8 spiNo = 0;
  MDP_SPI_CFG_S devCfg = {0};
  SPI_DEV dev = {0};

  ret = getSpiNo(DevHandle, &spiNo);

  if(ret != SCI_SUCCESS)
  {
    return ret;
  }
  else if(pConfig == NULL)
  {
    return SCI_ERROR;
  }

  dev.id = spiNo;
  dev.mode = CPOL1_CPHA1;

  //参数转换
  if((pConfig->ioctrlMode == SPI_IOCTL_DMA) || (pConfig->modeSel == SPI_SLAVER))
  {
    return SCI_ERROR;
  }

  if(pConfig->divider >= 2)
  {
    dev.freq = 6500000 / (2 * (pConfig->divider - 1));
    devCfg.freq = 6500000 / (2 * (pConfig->divider - 1));
  }
  else
  {
    dev.freq = 6500000;
    devCfg.freq = 6500000;
  }

  if(pConfig->bitPri == SPI_LSB_FIRST)
  {
    devCfg.lsb_first = SCI_TRUE;
  }
  else
  {
    devCfg.lsb_first = SCI_FALSE;
  }

  if(pConfig->bitLen == SPI_BL_16BIT)
  {
    dev.tx_bit_length = 16;
    devCfg.tx_bit_length = 16;
  }
  else
  {
    dev.tx_bit_length = 8;
    devCfg.tx_bit_length = 8;
  }

  if(pConfig->datatrans == SPI_DATA_HH)
  {
    devCfg.sck_reverse = SCI_TRUE;
    devCfg.rx_shift_edge = SCI_TRUE;
  }
  else if(pConfig->datatrans == SPI_DATA_HL)
  {
    devCfg.sck_reverse = SCI_TRUE;
    devCfg.rx_shift_edge = SCI_FALSE;
  }
  else if(pConfig->datatrans == SPI_DATA_LH)
  {
    devCfg.sck_reverse = SCI_FALSE;
    devCfg.rx_shift_edge = SCI_TRUE;
  }
  else
  {
    devCfg.sck_reverse = SCI_FALSE;
    devCfg.rx_shift_edge = SCI_FALSE;
  }

  //执行配置
  ret = SPI_HAL_Open(&dev);

  if(0 == ret)
  {
    spiBody[spiNo].cfg = devCfg;
    return SCI_SUCCESS;
  }
  else
  {
    return SCI_ERROR;
  }
}

/*
 * 功能 : 从SPI总线读取指定大小的Buffer数据
 * DevHandle : 设备句柄
 * pchTxBuf : 输入指令数据
 * pchUserBuf : 指向读取数据Buffer的头指针
 * dwCount : 指定读取数据的数量
 * 返回 : 成功时返回RET_SUCCES,失败时返回原因
 */
int spiRead(int DevHandle,uint8* pchTxBuf,uint8* pchUserBuf,uint32 dwCount)
{
  int ret;
  uint8 spiNo;

  ret = getSpiNo(DevHandle,&spiNo);
  if(ret != SCI_SUCCESS)
  {
    return(ret);
  }

  //暂不处理读

  return SCI_SUCCESS;
}

/*
 * 功能 : 向SPI总线写入指定大小的Buffer数据
 * DevHandle : 设备句柄
 * pchUserBuf : 指向将要写入数据buffer的头指针
 * dwCount : 指定写入Buffer的数量
 * 返回 : 写入的数据长度
 */
int spiWrite(int DevHandle, uint8* pchUserBuf, uint32 length)
{
  int ret = 0;
  uint8 spiNo = 0;

  ret = getSpiNo(DevHandle, &spiNo);

  if(ret != SCI_SUCCESS)
  {
    return ret;
  }

  ret = SPI_HAL_LCDWrite(spiNo, pchUserBuf, length, 0);
  return ret;
}

int spiWrite0(int DevHandle, uint8* pchUserBuf, uint32 length)
{
  int ret = 0;
  uint8 spiNo = 0;

  ret = getSpiNo(DevHandle, &spiNo);

  if(ret != SCI_SUCCESS)
  {
    return ret;
  }

  ret = SPI_HAL_Write(spiNo, pchUserBuf, length);
  return ret;
}

/*
 * 功能 : 注册SPI处理的回调函数
 * SpiHandle : SPI回调函数指针
 * 返回 : 成功时返回RET_SUCCES,失败时返回原因
 */
int spiSubscribe(spi_handle_f SpiHandle)
{
  if(SpiHandle == NULL)
  {
    return SCI_ERROR;
  }
  else
  {
    spiHandlef = SpiHandle;
    return SCI_SUCCESS;
  }
}

/*
 * 功能 : 注销SPI处理的回调函数
 * 返回 : 成功时返回RET_SUCCES,失败时返回RET_FAILED
 */
int spiUnSubscribe(void)
{
  if(spiHandlef == NULL)
  {
    return SCI_ERROR;
  }
  else
  {
    spiHandlef = NULL;
    return SCI_SUCCESS;
  }
}


/******************************************************************************/
//  Description:  Set LCD invalidate direction
//  Input:
//      is_invert: 0, horizontal; 1, vertical
//  Return:
//      None.
//  Note: Application should correct invalidate direction right after current
//      image displayed
/******************************************************************************/
LOCAL ERR_LCD_E VTM88828A_SetDirection(
  LCD_DIRECT_E direct_type
  )
{
  s_lcd_direct = direct_type;
  return ERR_LCD_NONE;
}

/******************************************************************************/
//  Description:   Set the windows address to display, in this windows
//                 color is  refreshed.
//  Global resource dependence:
//  Author:
//  Note:
/******************************************************************************/
LOCAL void VTM88828A_set_display_window(
  uint16 left,  // start Horizon address
  uint16 top,   // end Horizon address
  uint16 right,     // start Vertical address
  uint16 bottom // end Vertical address
  )
{
  uint16 newleft   = left;
  uint16 newtop    = top;
  uint16 newright  = right;
  uint16 newbottom = bottom;
  SCI_TraceLow_Test("[DRV_LCD]VTM88828A_set_display_window L = %d, top = %d, R = %d, bottom = %d",left,top,right,bottom);
  SCI_TraceLow_Test("[DRV_LCD]VTM88828A_set_display_window s_lcd_direct = %d",s_lcd_direct);

  //TODO:
  //Set Column Address
  set_lcd_col(0);

  //Set Page Address
  set_lcd_page(0);

}

void cust_cfg_lcd_spi(void)
{
  spi_config_t spiCfg = {0};

  //SPI初始化
  spiCfg.ioctrlMode = SPI_IOCTL_POLL;
  spiCfg.modeSel = SPI_MASTER;
  spiCfg.divider = 2;
  spiCfg.bitPri = SPI_MSB_FIRST;
  spiCfg.bitLen = SPI_BL_8BIT;
  spiCfg.datatrans = SPI_DATA_LL;

  g_spiRdHandle = spiOpen(LCD_SPI);
  spiConfig(g_spiRdHandle, &spiCfg);

  //RST初始化
  g_rstRdHandle = LCD_RST_PIN;
  GPIO_Enable(g_rstRdHandle);
  GPIO_SetDirection(g_rstRdHandle, GPIO_OUTPUT_DIRECTION);
  GPIO_SetValue(g_rstRdHandle, PIN_HIGH);

  //DC初始化
  g_dcRdHandle = LCD_CMD_DATA_PIN;
  GPIO_Enable(g_dcRdHandle);
  GPIO_SetDirection(g_rstRdHandle, GPIO_OUTPUT_DIRECTION);
  GPIO_SetValue(g_dcRdHandle, PIN_HIGH);
}

void custLcdRstPinSet(uint8 val)
{
  GPIO_SetDirection(g_rstRdHandle, GPIO_OUTPUT_DIRECTION);
  GPIO_SetValue(g_rstRdHandle, val);
}

void custLcdCmdDataSet(uint8 val)
{
  GPIO_SetDirection(g_rstRdHandle, GPIO_OUTPUT_DIRECTION);
  GPIO_SetValue(g_dcRdHandle, val);
}

void custSpiWriteData(uint8 *pData, uint16 wLen)
{
  spiWrite0(g_spiRdHandle, pData, wLen);
}

void cust_lcd_drv_write(uint8 cmd_data, uint16 writeLen, uint8 *data)
{
  if(SDK_LCD_CMD == cmd_data)
  {
    custLcdCmdDataSet(0);
  }
  else
  {
    custLcdCmdDataSet(1);
    custSpiWriteData(data, writeLen);
  }
}

//  开启关闭显示
void open_lcd(BOOLEAN on_off)
{
  uint8 temp = (on_off | LCD_DISPLAY_OFF);
  cust_lcd_drv_write(SDK_LCD_CMD, 1, &temp);
  //simu_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

// 设置显示方向A0,A1
void set_lcd_adc(uint8 direction)
{
  uint8 temp = (direction|0xA0);
  cust_lcd_drv_write(SDK_LCD_CMD, 1, &temp);
}

//设置显示的起始列
void set_lcd_col(uint8 colum)
{
  uint8 temp = ((colum>>4)|0x10);
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
  temp = colum&0x0F;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//设置显示起始行
void set_lcd_line(uint8 line)
{
  uint8 temp = (line|0x40);
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}


//设置显示页地址
void  set_lcd_page(uint8 pageNum)
{
  uint8 temp = (pageNum&0x0F)|0xB0;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//设置显示正反显示A6,A7
void  set_lcd_reverse(uint8 reverse)
{
  uint8 temp = reverse|0xA6;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//设置全屏显示(为1时类似清屏显示效果)  A4，A5
void  set_lcd_all(uint8 display)
{
  uint8 temp = display|0xA4;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//设置偏压  0：1/9     1: 1/7   A2,A3
void set_lcd_bias(uint8 bias79)
{
  uint8 temp = bias79|0xA2;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//设置读-改-写 模式，每次写操作,cloum自加
void set_lcd_R_M_W(void)
{
  uint8 temp = 0xE0;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}


//设置结束读-改-写模式
void  set_lcd_R_M_W_End(void)
{
  uint8 temp = 0xEE;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//设置软件复位：将初始化显示起始行、起始列地址、起始页地址、正常输出模式
void  set_lcd_soft_reset(void)
{
  uint8 temp = 0xE2;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//设置行扫描方向0:COM0-COM63  1:COM63-COM0
void  set_lcd_com(uint8 comNum)
{
   uint8 temp = ((comNum<<3)&0x0F)|0xC0;
   cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//设置电源控制的Circuit operation mode  28-2F
void set_lcd_power_circuit(uint8 mode)
{
  uint8 temp = (mode&0x07)|0x28;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//设置内部电阻调整设置20-27
void  set_lcd_resistor(uint8 resistor)
{
  uint8 temp = (resistor&0x07)|0x20;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//设置对比度，同列显示，双字操作0-63
void  set_lcd_contrast(uint8 contrast)
{
  uint8 temp = 0x81;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
  temp = contrast&0x3F;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}


//设置静态显示，同上为双字操作0：off  1：on
void  set_lcd_static(uint8 on_off, uint8 flashMode)
{
  uint8 temp = (on_off&0x01)|0xAC;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
  temp = flashMode&0x03;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//传输LCD 数据
void set_lcd_data(uint16 writeLen,uint8 *data)
{
   cust_lcd_drv_write(SDK_LCD_DATA,writeLen,data);
}

LOCAL void VTM88828A_driver(void)
{
  uint8 itemp = 0;
  uint8 tempBuf[132] = {0};

  SCI_TraceLow_Test("[REF_LCD]VTM88828A_driver");
  cust_cfg_lcd_spi();

  g_lcd_pixels_buf = malloc(DISPLAY_BUF_SIZE);
  memset(g_lcd_pixels_buf, 0x0, DISPLAY_BUF_SIZE);

  custLcdRstPinSet(1);
  LCD_Delay(10);
  custLcdRstPinSet(0);
  LCD_Delay(20);
  custLcdRstPinSet(1);

  set_lcd_soft_reset();
  LCD_Delay(10);
  set_lcd_bias(0);
  set_lcd_adc(0);//colum 0-130
  set_lcd_com(0);//row 0-63
  set_lcd_resistor(0x24);//(1+Rb/Ra) 100:5.0 ; 111: 6.4
  set_lcd_power_circuit(7);//power control set //2F
  set_lcd_contrast(40);
  set_lcd_line(0);
  set_lcd_page(0);
  set_lcd_col(0);
  set_lcd_reverse(0);//
  LCD_Delay(10);
  open_lcd(1);//open display
  for(itemp = 0;itemp< 8;itemp++)
  {
    SCI_Memset(tempBuf,0x00,sizeof(tempBuf));
    set_lcd_page(itemp);
    set_lcd_col(0);
    set_lcd_data(131,tempBuf);
  }
  set_lcd_page(8);
  set_lcd_col(0);
  set_lcd_data(131,tempBuf);
}

/**************************************************************************************/
// Description: initialize all LCD with LCDC MCU MODE and LCDC mcu mode
// Global resource dependence:
// Author:
// Note:
/**************************************************************************************/
LOCAL ERR_LCD_E VTM88828A_Init(void)
{
  uint32 id = 0;

  SCI_TraceLow_Test(" VTM88828A_Init");
  VTM88828A_driver();
  VTM88828A_SetDirection(LCD_DIRECT_NORMAL);

  SCI_TraceLow_Test(" VTM88828A_Init ID = 0x%x\n", id);
  return 0;
}

/******************************************************************************/
//  Description:   Enter/Exit sleep mode .
//  Global resource dependence:
//  Author:
//  Note:
/******************************************************************************/
LOCAL ERR_LCD_E  VTM88828A_EnterSleep(
  BOOLEAN is_sleep  //SCI_TRUE: enter sleep mode;SCI_FALSE:exit sleep mode.
  )
{
  SCI_TraceLow_Test(" VTM88828A_EnterSleep");
  return ERR_LCD_NONE;
}

/******************************************************************************/
//  Description:   Close the lcd.(include sub lcd.)
//  Global resource dependence:
//  Author:
//  Note:
/******************************************************************************/
LOCAL void VTM88828A_Close(void)
{
  //GPIO_SetLcdBackLight( SCI_FALSE );
  SCI_TraceLow_Test(" VTM88828A_Close");
  VTM88828A_EnterSleep(SCI_TRUE);
  free(g_lcd_pixels_buf);
  g_lcd_pixels_buf = NULL;
}

/******************************************************************************/
//  Description:   invalidate a rectang of in LCD
//  Global resource dependence:
//  Author:
//  Note:
/******************************************************************************/
LOCAL ERR_LCD_E VTM88828A_Invalidate(void)
{
  SCI_TraceLow_Test(" VTM88828A_Invalidate");

  VTM88828A_set_display_window(0x0, 0x0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
  return ERR_LCD_NONE;
}

/******************************************************************************/
//  Description:   Copy a retangle data from clcd_buffer to display RAM.
//                     then the rectangle display is to be refreshed
//  Global resource dependence:
//  Author:
//  Note:
//     To improve speed, lcd is operate in HIGH SPEED RAM WRITE MODE(4
//     uint16 are write continuously always.) So, some dummy uint16
//     should be inserted to satisfy this mode.   Please refer to spec.
/******************************************************************************/
PUBLIC ERR_LCD_E VTM88828A_InvalidateRect(
  uint16 left,  //the left value of the rectangel
  uint16 top,   //top of the rectangle
  uint16 right,   //right of the rectangle
  uint16 bottom //bottom of the rectangle
  )
{
  SCI_TraceLow_Test(" VTM88828A_InvalidateRect");
  left  = (left >= LCD_WIDTH)    ? LCD_WIDTH-1 : left;
  right   = (right >= LCD_WIDTH)   ? LCD_WIDTH-1 : right;
  top   = (top >= LCD_HEIGHT)    ? LCD_HEIGHT-1 : top;
  bottom  = (bottom >= LCD_HEIGHT) ? LCD_HEIGHT-1 : bottom;

  if ( ( right < left ) || ( bottom < top ) )
  {
    return ERR_LCD_OPERATE_FAIL;
  }

  VTM88828A_set_display_window(left, top, right, bottom);
  return ERR_LCD_NONE;

}


/**************************************************************************************/
// Description: refresh a rectangle of lcd
// Global resource dependence:
// Author:
// Note:
//    left - the left value of the rectangel
//    top - the top value of the rectangel
//    right - the right value of the rectangel
//    bottom - the bottom value of the rectangel
/**************************************************************************************/
LOCAL ERR_LCD_E VTM88828A_RotationInvalidateRect(uint16 left,uint16 top,uint16 right,uint16 bottom,LCD_ANGLE_E angle)
{
  //int32 error;
  SCI_TraceLow_Test(" VTM88828A_RotationInvalidateRect");

  switch(angle)
  {
    case LCD_ANGLE_0:
      VTM88828A_set_display_window(left, top, right, bottom);
      break;
    case LCD_ANGLE_90:
      VTM88828A_set_display_window(left, top, bottom,right);
      break;
    case LCD_ANGLE_180:
      VTM88828A_set_display_window(left, top, right, bottom);
      break;
    case LCD_ANGLE_270:
      VTM88828A_set_display_window(left, top, bottom,right);
      break;
    default:
      VTM88828A_set_display_window(left, top, right, bottom);
      break;
  }

  return ERR_LCD_NONE;
}//en of S6D0139_VerticalInvalidateRect


/******************************************************************************/
//  Description:  set the contrast value
//  Global resource dependence:
//  Author:
//  Note:
/******************************************************************************/
LOCAL ERR_LCD_E   VTM88828A_SetContrast(
  uint16  contrast  //contrast value to set
  )
{
  uint8 value = 0;

  SCI_TraceLow_Test(" VTM88828A_SetContrast");
  value = (uint8)contrast;
  set_lcd_contrast(value);
  return ERR_LCD_NONE;
}


/*****************************************************************************/
//  Description:    Set the brightness of LCD.
//  Global resource dependence:
//  Author:
//  Note:
/*****************************************************************************/
LOCAL ERR_LCD_E   VTM88828A_SetBrightness(
  uint16 brightness //birghtness to set
  )
{
  SCI_TraceLow_Test(" VTM88828A_SetBrightness");
  return ERR_LCD_FUNC_NOT_SUPPORT;
}

/*****************************************************************************/
//  Description:    Enable lcd to partial display mode, so can save power.
//  Global resource dependence:
//  Author:
//  Return:         SCI_TRUE:SUCCESS ,SCI_FALSE:failed.
//  Note:           If all input parameters are 0, exit partial display mode.
/*****************************************************************************/
LOCAL ERR_LCD_E VTM88828A_SetDisplayWindow(
  uint16 left,    //left of the window
  uint16 top,     //top of the window
  uint16 right,   //right of the window
  uint16 bottom   //bottom of the window
  )
{
  SCI_TraceLow_Test(" VTM88828A_SetDisplayWindow");

  VTM88828A_set_display_window(left, top, right, bottom);

  return ERR_LCD_NONE;
}

/******************************************************************************/
// Description: Invalidate Pixel
// Global resource dependence:
// Author:
// Note:
/******************************************************************************/
LOCAL void VTM88828A_InvalidatePixel(uint16 x, uint16 y, uint32 data)
{
  SCI_TraceLow_Test(" VTM88828A_InvalidatePixel");
  VTM88828A_InvalidateRect(x,y,x,y);
}

void _SPILCD_Test_Task(uint32 argc, void* argv)
{
    VTM88828A_Init();
}

void SPILCD_Test(void)
{
    SCI_CreateThread(
        "_SPILCD_Test_Task",
        "_SPILCD_Test_Queue",
        _SPILCD_Test_Task,
        0,
        PNULL,
        0x8000,
        10,
        31,
        SCI_PREEMPT,
        SCI_AUTO_START);
}

/******************************************************************************/
//  Description:   Close the lcd.(include sub lcd.)
//  Global resource dependence:
//  Author:
//  Note:
/******************************************************************************/
const LCD_OPERATIONS_T VTM88828A_spi_operations =
{
  VTM88828A_Init,
  VTM88828A_EnterSleep,
  VTM88828A_SetContrast,
  VTM88828A_SetBrightness,
  VTM88828A_SetDisplayWindow,
  VTM88828A_InvalidateRect,
  VTM88828A_Invalidate,
  VTM88828A_Close,
  VTM88828A_RotationInvalidateRect,
  VTM88828A_SetDirection,
  NULL,
  NULL
};

LOCAL const LCD_TIMING_U s_VTM88828A_spitiming =
{
  6500000,0,1,
/*SPI_CLK_52MHZ,        // clk frequency support (unit:MHz)
  SPI_CLK_IDLE_LOW,     // CPOL: 0--SPI_CLK_IDLE_LOW, 1--SPI_CLK_IDLE_HIGH
  SPI_SAMPLING_RISING,  // CPHA: 0--SPI_SAMPLING_RISING,  1--SPI_SAMPLING_FALLING
*/
  8,                    // tx bit length: 8/16bits refer to lcm driver ic
  0,
  0
};

const LCD_SPEC_T g_lcd_VTM88828A_spi =
{
  VTM88828A_SUPPORT_WIDTH,
  VTM88828A_SUPPORT_HEIGHT,
  LCD_MCU,
  BUS_MODE_SPI,
  WIDTH_8,
  (LCD_TIMING_U*)&s_VTM88828A_spitiming,
  (LCD_OPERATIONS_T*)&VTM88828A_spi_operations,
  0,
  0
};

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    }
#endif




