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
#include "spi_drvapi_old.h"
#include "gpio_drvapi.h"
#include "pwm_drvapi.h"

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
        SCI_Sleep( ticks );

#define VTM88828A_SUPPORT_WIDTH   128
#define VTM88828A_SUPPORT_HEIGHT  64
#define VTM88828A_Y_PAGE_SIZE 8 //bits
#define VTM88828A_Y_PAGES ((VTM88828A_SUPPORT_HEIGHT + VTM88828A_Y_PAGE_SIZE - 1) / VTM88828A_Y_PAGE_SIZE)
#define DISPLAY_BUF_SIZE ((VTM88828A_SUPPORT_WIDTH * VTM88828A_SUPPORT_HEIGHT) / VTM88828A_Y_PAGE_SIZE)

#define MONO_X_PIXELS VTM88828A_SUPPORT_WIDTH
#define MONO_Y_PIXELS VTM88828A_SUPPORT_HEIGHT
#define MONO_Y_PAGE_SIZE VTM88828A_Y_PAGE_SIZE //bits
#define MONO_Y_PAGES VTM88828A_Y_PAGES

#define Clear_X_PIXELS 132
#define CLEAR_BUF_SIZE ((Clear_X_PIXELS)*MONO_Y_PIXELS)/MONO_Y_PAGE_SIZE//yezq 2012.1.29

#define LCD_X_PIXEX_OFFSET 0  // 3 
#define GB_PIXEL_H 12 //×Ö¿âµÄµãÕó´óÐ¡12*12/6*12
#define GB_MAX_LEN ((GB_PIXEL_H+7)/8)*GB_PIXEL_H //Ò»¸öºº×ÖËùÕ¼µÄ×î´ó×Ö½ÚÊý 
typedef enum {
 HOLE_IMAGE = 0,
 HOLE_GB_ASII
}fillType_e;

typedef struct{
  uint8 x_image;
  uint8 y_image; 
}fillImage_t;

typedef struct
{
  uint8 x;
  uint8 y;
  uint8 xlen;
  uint8 ylen;
}holeType_t;

typedef enum{
  FLUSH_LEFT = 0,
  FLUSH_MID = 1,
  FLUSH_RIGHT 
}flushType_e;

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


static spi_handle_f spiHandlef = NULL;//SPI»Øµ÷
static Spi_Body_T spiBody[SPI_NO_MAX] = {0};//SPI½á¹¹±í

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
//ÉèÖÃÏÔÊ¾ÆðÊ¼ÐÐ
void set_lcd_line(uint8 line);
//ÉèÖÃÏÔÊ¾Ò³µØÖ·
void  set_lcd_page(uint8 pageNum);

#define PWM_CTL_S_ONOFF 0x30    /*Set PWM ON/OFF*/
#define PWM_CTL_S_CLKSRC 0x31   /*Set CLK sorce*/
#define PWM_CTL_G_CLKSRC 0x32   /*Get CLK sorce*/
#define PWM_CTL_S_POLARITY 0x33 /*Set PWM Output Polarity*/

#define GBK_ASCII_MAX 0xA1
#define GBK_16_16_CHAR_LEN 32
#define GBK_12_12_CHAR_LEN 24
#define GB16_16_PIXEL_H 16
#define GB16_16_MAX_LEN ((GB16_16_PIXEL_H+7)>>3)*GB16_16_PIXEL_H 

uint8 pixel_merge_bits[MONO_Y_PAGE_SIZE+1]={
   0x00,
   0x80,
   0xC0,
   0xE0,
   0xF0,
   0xF8,
   0xFC,
   0xFE,  
   0xFF
};  


uint8 pixel_bits[MONO_Y_PAGE_SIZE]={
   0x01,
   0x02,
   0x04,
   0x08,
   0x10,
   0x20,
   0x40,
   0x80
}; 



 const unsigned char ascii_6_12[]={ 
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//" ",0
  0x00,0x00,0x00,0x03,0x02,0x02,0x00,0x00,0x00,0xF0,0x00,0x00,//"",1
  0x02,0x02,0x02,0x03,0x00,0x00,0x00,0x00,0x00,0xF0,0x00,0x00,//"",2
  0x00,0x00,0x00,0xFE,0x02,0x02,0x00,0x00,0x00,0x00,0x00,0x00,//"",3
  0x02,0x02,0x02,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//"",4
  0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0xF0,0x00,0x00,//"",5
  0x02,0x02,0x02,0x02,0x02,0x02,0x00,0x00,0x00,0x00,0x00,0x00,//"",6
  0x00,0x0E,0x1F,0x1F,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//"",7
  0xFF,0xF1,0xE0,0xE0,0xF1,0xFF,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,//"",8
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//"	",9
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//" ",10
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//" ",11
  0x00,0x01,0x02,0x4E,0x71,0x78,0x00,0x80,0x40,0x40,0x80,0x00,//"",12
  0x00,0x32,0x4A,0x4F,0x4A,0x32,0x00,0x00,0x00,0xC0,0x00,0x00,//"",13
  0x00,0x00,0x3F,0x28,0x51,0x7F,0x00,0xC0,0xC0,0x00,0x80,0x80,//"",14
  0x00,0x64,0x1F,0x60,0x1F,0x64,0x00,0xC0,0x00,0xC0,0x00,0xC0,//"",15
  0x02,0x02,0x02,0xFF,0x02,0x02,0x00,0x00,0x00,0xF0,0x00,0x00,//"",16
  0x00,0x04,0x0E,0x1F,0x3F,0x7F,0x00,0x00,0x00,0x00,0x80,0xC0,//"",17
  0x00,0x00,0x40,0xFF,0x40,0x00,0x00,0x00,0x80,0xC0,0x80,0x00,//"",18
  0x00,0x00,0xFE,0x00,0xFE,0x00,0x00,0x00,0xC0,0x00,0xC0,0x00,//"",19
  0x00,0x70,0x88,0xFF,0x80,0xFF,0x00,0x00,0x00,0xC0,0x00,0xC0,//"",20
  0x02,0x02,0x02,0xFE,0x02,0x02,0x00,0x00,0x00,0x00,0x00,0x00,//"",21
  0x02,0x02,0x02,0x03,0x02,0x02,0x00,0x00,0x00,0xF0,0x00,0x00,//"",22
  0x02,0x02,0x02,0xFF,0x00,0x00,0x00,0x00,0x00,0xF0,0x00,0x00,//"",23
  0x00,0x00,0x40,0xFF,0x40,0x00,0x00,0x00,0x00,0xC0,0x00,0x00,//"",24
  0x00,0x00,0x00,0xFF,0x02,0x02,0x00,0x00,0x00,0xF0,0x00,0x00,//"",25
  0x00,0x04,0x04,0x04,0x0E,0x04,0x00,0x00,0x00,0x00,0x00,0x00,//"",26
  0x00,0x04,0x0E,0x04,0x04,0x04,0x00,0x00,0x00,0x00,0x00,0x00,//"",27
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//"",28
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//"",29
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//"",30
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//"",31
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//" ",32
  0x00,0x00,0x3F,0x00,0x00,0x00,0x00,0x00,0x40,0x00,0x00,0x00,//"!",33
  0x00,0x30,0x40,0x30,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//""",34
  0x09,0x0B,0x3D,0x0B,0x3D,0x09,0x00,0xC0,0x00,0xC0,0x00,0x00,//"#",35
  0x18,0x24,0x7F,0x22,0x31,0x00,0xC0,0x40,0xE0,0x40,0x80,0x00,//"$",36
  0x18,0x24,0x1B,0x0D,0x32,0x01,0x00,0xC0,0x00,0x80,0x40,0x80,//"%",37
  0x03,0x1C,0x27,0x1C,0x07,0x00,0x80,0x40,0x40,0x80,0x40,0x40,//"&",38
  0x10,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//"'",39
  0x00,0x00,0x00,0x1F,0x20,0x40,0x00,0x00,0x00,0x80,0x40,0x20,//"(",40
  0x00,0x40,0x20,0x1F,0x00,0x00,0x00,0x20,0x40,0x80,0x00,0x00,//")",41
  0x09,0x06,0x1F,0x06,0x09,0x00,0x00,0x00,0x80,0x00,0x00,0x00,//"*",42
  0x04,0x04,0x3F,0x04,0x04,0x00,0x00,0x00,0x80,0x00,0x00,0x00,//"+",43
  0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x60,0x00,0x00,0x00,0x00,//",",44
  0x04,0x04,0x04,0x04,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//"-",45
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x00,0x00,0x00,0x00,//".",46
  0x00,0x01,0x06,0x38,0x40,0x00,0x20,0xC0,0x00,0x00,0x00,0x00,//"/",47
  0x1F,0x20,0x20,0x20,0x1F,0x00,0x80,0x40,0x40,0x40,0x80,0x00,//"0",48
  0x00,0x10,0x3F,0x00,0x00,0x00,0x00,0x40,0xC0,0x40,0x00,0x00,//"1",49
  0x18,0x21,0x22,0x24,0x18,0x00,0xC0,0x40,0x40,0x40,0x40,0x00,//"2",50
  0x10,0x20,0x24,0x24,0x1B,0x00,0x80,0x40,0x40,0x40,0x80,0x00,//"3",51
  0x02,0x0D,0x11,0x3F,0x01,0x00,0x00,0x00,0x00,0xC0,0x40,0x00,//"4",52
  0x3C,0x24,0x24,0x24,0x23,0x00,0x80,0x40,0x40,0x40,0x80,0x00,//"5",53
  0x1F,0x24,0x24,0x34,0x03,0x00,0x80,0x40,0x40,0x40,0x80,0x00,//"6",54
  0x30,0x20,0x27,0x38,0x20,0x00,0x00,0x00,0xC0,0x00,0x00,0x00,//"7",55
  0x1B,0x24,0x24,0x24,0x1B,0x00,0x80,0x40,0x40,0x40,0x80,0x00,//"8",56
  0x1C,0x22,0x22,0x22,0x1F,0x00,0x00,0xC0,0x40,0x40,0x80,0x00,//"9",57
  0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x40,0x00,0x00,0x00,//":",58
  0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x60,0x00,0x00,0x00,//";",59
  0x00,0x04,0x0A,0x11,0x20,0x40,0x00,0x00,0x00,0x00,0x80,0x40,//"<",60
  0x09,0x09,0x09,0x09,0x09,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//"=",61
  0x00,0x40,0x20,0x11,0x0A,0x04,0x00,0x40,0x80,0x00,0x00,0x00,//">",62
  0x18,0x20,0x23,0x24,0x18,0x00,0x00,0x00,0x40,0x00,0x00,0x00,//"?",63
  0x1F,0x20,0x27,0x29,0x1F,0x00,0x80,0x40,0x40,0x40,0x40,0x00,//"@",64
  0x00,0x07,0x39,0x0F,0x01,0x00,0x40,0xC0,0x00,0x00,0xC0,0x40,//"A",65
  0x20,0x3F,0x24,0x24,0x1B,0x00,0x40,0xC0,0x40,0x40,0x80,0x00,//"B",66
  0x1F,0x20,0x20,0x20,0x30,0x00,0x80,0x40,0x40,0x40,0x80,0x00,//"C",67
  0x20,0x3F,0x20,0x20,0x1F,0x00,0x40,0xC0,0x40,0x40,0x80,0x00,//"D",68
  0x20,0x3F,0x24,0x2E,0x30,0x00,0x40,0xC0,0x40,0x40,0xC0,0x00,//"E",69
  0x20,0x3F,0x24,0x2E,0x30,0x00,0x40,0xC0,0x40,0x00,0x00,0x00,//"F",70
  0x0F,0x10,0x20,0x22,0x33,0x02,0x00,0x80,0x40,0x40,0x80,0x00,//"G",71
  0x20,0x3F,0x04,0x04,0x3F,0x20,0x40,0xC0,0x00,0x00,0xC0,0x40,//"H",72
  0x20,0x20,0x3F,0x20,0x20,0x00,0x40,0x40,0xC0,0x40,0x40,0x00,//"I",73
  0x00,0x20,0x20,0x3F,0x20,0x20,0x60,0x20,0x20,0xC0,0x00,0x00,//"J",74
  0x20,0x3F,0x24,0x0B,0x30,0x20,0x40,0xC0,0x40,0x00,0xC0,0x40,//"K",75
  0x20,0x3F,0x20,0x00,0x00,0x00,0x40,0xC0,0x40,0x40,0x40,0xC0,//"L",76
  0x3F,0x3C,0x03,0x3C,0x3F,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x00,//"M",77
  0x20,0x3F,0x0C,0x23,0x3F,0x20,0x40,0xC0,0x40,0x00,0xC0,0x00,//"N",78
  0x1F,0x20,0x20,0x20,0x1F,0x00,0x80,0x40,0x40,0x40,0x80,0x00,//"O",79
  0x20,0x3F,0x24,0x24,0x18,0x00,0x40,0xC0,0x40,0x00,0x00,0x00,//"P",80
  0x1F,0x21,0x21,0x20,0x1F,0x00,0x80,0x40,0x40,0xE0,0xA0,0x00,//"Q",81
  0x20,0x3F,0x24,0x26,0x19,0x00,0x40,0xC0,0x40,0x00,0xC0,0x40,//"R",82
  0x18,0x24,0x24,0x22,0x31,0x00,0xC0,0x40,0x40,0x40,0x80,0x00,//"S",83
  0x30,0x20,0x3F,0x20,0x30,0x00,0x00,0x40,0xC0,0x40,0x00,0x00,//"T",84
  0x20,0x3F,0x00,0x00,0x3F,0x20,0x00,0x80,0x40,0x40,0x80,0x00,//"U",85
  0x20,0x3E,0x01,0x07,0x38,0x20,0x00,0x00,0xC0,0x00,0x00,0x00,//"V",86
  0x38,0x07,0x3C,0x07,0x38,0x00,0x00,0xC0,0x00,0xC0,0x00,0x00,//"W",87
  0x20,0x39,0x06,0x39,0x20,0x00,0x40,0xC0,0x00,0xC0,0x40,0x00,//"X",88
  0x20,0x38,0x07,0x38,0x20,0x00,0x00,0x40,0xC0,0x40,0x00,0x00,//"Y",89
  0x30,0x21,0x26,0x38,0x20,0x00,0x40,0xC0,0x40,0x40,0xC0,0x00,//"Z",90
  0x00,0x00,0x7F,0x40,0x40,0x00,0x00,0x00,0xE0,0x20,0x20,0x00,//"[",91
  0x00,0x70,0x0C,0x03,0x00,0x00,0x00,0x00,0x00,0x80,0x40,0x00,//"\",92
  0x00,0x40,0x40,0x7F,0x00,0x00,0x00,0x20,0x20,0xE0,0x00,0x00,//"]",93
  0x00,0x20,0x40,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//"^",94
  0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x10,0x10,0x10,0x10,0x10,//"_",95
  0x00,0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//"`",96
  0x00,0x02,0x05,0x05,0x03,0x00,0x00,0x80,0x40,0x40,0xC0,0x40,//"a",97
  0x20,0x3F,0x04,0x04,0x03,0x00,0x00,0xC0,0x40,0x40,0x80,0x00,//"b",98
  0x00,0x03,0x04,0x04,0x06,0x00,0x00,0x80,0x40,0x40,0x40,0x00,//"c",99
  0x00,0x03,0x04,0x24,0x3F,0x00,0x00,0x80,0x40,0x40,0xC0,0x40,//"d",100
  0x00,0x03,0x05,0x05,0x03,0x00,0x00,0x80,0x40,0x40,0x40,0x00,//"e",101
  0x00,0x04,0x1F,0x24,0x24,0x20,0x00,0x40,0xC0,0x40,0x40,0x00,//"f",102
  0x00,0x02,0x05,0x05,0x06,0x04,0x00,0xE0,0x50,0x50,0x50,0x20,//"g",103
  0x20,0x3F,0x04,0x04,0x03,0x00,0x40,0xC0,0x40,0x00,0xC0,0x40,//"h",104
  0x00,0x04,0x27,0x00,0x00,0x00,0x00,0x40,0xC0,0x40,0x00,0x00,//"i",105
  0x00,0x00,0x04,0x27,0x00,0x00,0x10,0x10,0x10,0xE0,0x00,0x00,//"j",106
  0x20,0x3F,0x01,0x07,0x04,0x04,0x40,0xC0,0x40,0x00,0xC0,0x40,//"k",107
  0x20,0x20,0x3F,0x00,0x00,0x00,0x40,0x40,0xC0,0x40,0x40,0x00,//"l",108
  0x07,0x04,0x07,0x04,0x03,0x00,0xC0,0x00,0xC0,0x00,0xC0,0x00,//"m",109
  0x04,0x07,0x04,0x04,0x03,0x00,0x40,0xC0,0x40,0x00,0xC0,0x40,//"n",110
  0x00,0x03,0x04,0x04,0x03,0x00,0x00,0x80,0x40,0x40,0x80,0x00,//"o",111
  0x04,0x07,0x04,0x04,0x03,0x00,0x10,0xF0,0x50,0x40,0x80,0x00,//"p",112
  0x00,0x03,0x04,0x04,0x07,0x00,0x00,0x80,0x40,0x50,0xF0,0x10,//"q",113
  0x04,0x07,0x02,0x04,0x04,0x00,0x40,0xC0,0x40,0x00,0x00,0x00,//"r",114
  0x00,0x06,0x05,0x05,0x04,0x00,0x00,0x40,0x40,0x40,0xC0,0x00,//"s",115
  0x00,0x04,0x1F,0x04,0x00,0x00,0x00,0x00,0x80,0x40,0x40,0x00,//"t",116
  0x04,0x07,0x00,0x04,0x07,0x00,0x00,0x80,0x40,0x40,0xC0,0x40,//"u",117
  0x04,0x07,0x04,0x01,0x06,0x04,0x00,0x00,0xC0,0x80,0x00,0x00,//"v",118
  0x06,0x01,0x07,0x01,0x06,0x00,0x00,0xC0,0x00,0xC0,0x00,0x00,//"w",119
  0x04,0x06,0x01,0x06,0x04,0x00,0x40,0xC0,0x00,0xC0,0x40,0x00,//"x",120
  0x04,0x07,0x04,0x01,0x06,0x04,0x10,0x10,0xE0,0x80,0x00,0x00,//"y",121
  0x00,0x04,0x05,0x06,0x04,0x00,0x00,0x40,0xC0,0x40,0x40,0x00,//"z",122
  0x00,0x00,0x04,0x7B,0x40,0x00,0x00,0x00,0x00,0xE0,0x20,0x00,//"",123
  0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0xF0,0x00,0x00,//"|",124
  0x00,0x40,0x7B,0x04,0x00,0x00,0x00,0x20,0xE0,0x00,0x00,0x00,//"",125
  0x40,0x80,0x40,0x20,0x20,0x40,0x00,0x00,0x00,0x00,0x00,0x00,//"~",126
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//"",127
};

void custGetAscii12_12_data(uint8 *charCode, uint8 *p_result, uint16 *p_result_len);
void mergeOneLineCharToHole(uint8 hole_xlen, uint8 x_now, uint8 y_now, flushType_e dispType, uint8*p_dest, uint8* p_source);
int update_display(uint8 x, uint8 y, uint8 xlen, uint8 ylen, uint8* p_data);
void fillAscStr2Hole(holeType_t holeInfo, flushType_e type, uint8 *p_STR, uint8 agChar_len,uint8 reserve);
int display_Screan(uint8 *p_data, uint16 dataLen);
static void Mmi_Idle_Display(void);

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
                  ÒÔÏÂ¶¨Òå±¾ÎÄ¼þÖÐÊ¹ÓÃµÄË½ÓÐº¯Êý
***************************************************************/
/*
 * ¹¦ÄÜ : »ñÈ¡Ö¸¶¨¾ä±ú¶ÔÓ¦µÄNOºÅ
 * Handle : Éè±¸¾ä±ú
 * no_p : ±àºÅ
 * ·µ»Ø : ¾ä±ú·Ç·¨·µ»ØRET_ERR_INVALID_HANDLE,
 *               ¾ä±úÃ»ÓÐÆôÓÃ·µ»ØRET_ERR_NOT_OPENED
 *               ·ñÔò·µ»ØSCI_SUCCESS
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
                  ÒÔÏÂ¶¨Òå±¾ÎÄ¼þÖÐÊ¹ÓÃµÄ¹«¹²º¯Êý
***************************************************************/
/*
 * ¹¦ÄÜ : SPIÊ¼»¯
 * ËµÃ÷ : ¹©Æ½Ì¨µ÷ÓÃ
 * ·µ»Ø : ³É¹¦»òÕßÊ§°Ü
 */
BOOLEAN m2mHalSpiInit(VOID)
{
  //²ÎÊý³õÊ¼»¯
  spiHandlef = NULL;
  memset(spiBody, 0x00, sizeof(spiBody));

  return SCI_TRUE;
}

/*
 * ¹¦ÄÜ : ´ò¿ªSPI
 * num : SPIÍ¨µÀ±êÊ¶
 * ·µ»Ø : ³É¹¦Ê±·µ»Ø¶ÔÓ¦µÄ¾ä±ú(num + 1),Ê§°ÜÊ±·µ»ØÔ­Òò
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
 * ¹¦ÄÜ : ¹Ø±ÕSPI
 * Handle : Éè±¸¾ä±ú
 * ·µ»Ø : ³É¹¦Ê±·µ»ØRET_SUCCES, Ê§°ÜÊ±·µ»ØÔ­Òò
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
 * ¹¦ÄÜ : SPIÅäÖÃ
 * DevHandle : Éè±¸¾ä±ú
 * pConfig : SPIÅäÖÃ
 * ·µ»Ø : ³É¹¦Ê±·µ»ØRET_SUCCES,Ê§°ÜÊ±·µ»ØÔ­Òò
 */
int spiConfig(int DevHandle, spi_config_t* pConfig)
{
  int ret = 0;
  uint8 spiNo = 0;
  MDP_SPI_CFG_S devCfg = {0};
  SPI_DEV dev = {0};
  SPI_CONFIG_T spiCfg = {0};

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
  dev.mode = CPOL0_CPHA0;
  spiCfg.cs_id = SPI_CS_ID0;
  spiCfg.dev_type = SPI_SDCARD;
  spiCfg.tx_shift_edge = SCI_TRUE;

  //²ÎÊý×ª»»
  if((pConfig->ioctrlMode == SPI_IOCTL_DMA) || (pConfig->modeSel == SPI_SLAVER))
  {
    return SCI_ERROR;
  }

  if(pConfig->divider >= 2)
  {
    dev.freq = 6500000 / (2 * (pConfig->divider - 1));
    devCfg.freq = 6500000 / (2 * (pConfig->divider - 1));
    spiCfg.freq = 6500000 / (2 * (pConfig->divider - 1));
  }
  else
  {
    dev.freq = 6500000;
    devCfg.freq = 6500000;
    spiCfg.freq = 6500000;
  }

  if(pConfig->bitPri == SPI_LSB_FIRST)
  {
    devCfg.lsb_first = SCI_TRUE;
    spiCfg.lsb_first = SCI_TRUE;
  }
  else
  {
    devCfg.lsb_first = SCI_FALSE;
    spiCfg.lsb_first = SCI_FALSE;
  }

  if(pConfig->bitLen == SPI_BL_16BIT)
  {
    dev.tx_bit_length = 16;
    devCfg.tx_bit_length = 16;
    spiCfg.tx_bit_length = 16;
  }
  else
  {
    dev.tx_bit_length = 8;
    devCfg.tx_bit_length = 8;
    spiCfg.tx_bit_length = 8;
  }

  if(pConfig->datatrans == SPI_DATA_HH)
  {
    devCfg.sck_reverse = SCI_TRUE;
    spiCfg.sck_reverse = SCI_TRUE;
    devCfg.rx_shift_edge = SCI_TRUE;
    spiCfg.rx_shift_edge = SCI_TRUE;
  }
  else if(pConfig->datatrans == SPI_DATA_HL)
  {
    devCfg.sck_reverse = SCI_TRUE;
    spiCfg.sck_reverse = SCI_TRUE;
    devCfg.rx_shift_edge = SCI_FALSE;
    spiCfg.rx_shift_edge = SCI_FALSE;
  }
  else if(pConfig->datatrans == SPI_DATA_LH)
  {
    devCfg.sck_reverse = SCI_FALSE;
    spiCfg.sck_reverse = SCI_FALSE;
    devCfg.rx_shift_edge = SCI_TRUE;
    spiCfg.rx_shift_edge = SCI_TRUE;
  }
  else
  {
    devCfg.sck_reverse = SCI_FALSE;
    spiCfg.sck_reverse = SCI_FALSE;
    devCfg.rx_shift_edge = SCI_FALSE;
    spiCfg.rx_shift_edge = SCI_FALSE;
  }

  //Ö´ÐÐÅäÖÃ
  //ret = SPI_HAL_Open(&dev);
  ret = SPI_Open(spiNo, &spiCfg);

  if (0 == ret)
  {
    if (SCI_FALSE == SPI_Init(spiNo))
    {
      return SCI_ERROR;
    }
    spiBody[spiNo].cfg = devCfg;
    return SCI_SUCCESS;
  }
  else
  {
    return SCI_ERROR;
  }
}

/*
 * ¹¦ÄÜ : ´ÓSPI×ÜÏß¶ÁÈ¡Ö¸¶¨´óÐ¡µÄBufferÊý¾Ý
 * DevHandle : Éè±¸¾ä±ú
 * pchTxBuf : ÊäÈëÖ¸ÁîÊý¾Ý
 * pchUserBuf : Ö¸Ïò¶ÁÈ¡Êý¾ÝBufferµÄÍ·Ö¸Õë
 * dwCount : Ö¸¶¨¶ÁÈ¡Êý¾ÝµÄÊýÁ¿
 * ·µ»Ø : ³É¹¦Ê±·µ»ØRET_SUCCES,Ê§°ÜÊ±·µ»ØÔ­Òò
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

  //ÔÝ²»´¦Àí¶Á

  return SCI_SUCCESS;
}

/*
 * ¹¦ÄÜ : ÏòSPI×ÜÏßÐ´ÈëÖ¸¶¨´óÐ¡µÄBufferÊý¾Ý
 * DevHandle : Éè±¸¾ä±ú
 * pchUserBuf : Ö¸Ïò½«ÒªÐ´ÈëÊý¾ÝbufferµÄÍ·Ö¸Õë
 * dwCount : Ö¸¶¨Ð´ÈëBufferµÄÊýÁ¿
 * ·µ»Ø : Ð´ÈëµÄÊý¾Ý³¤¶È
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
  SPI_SetCSSignal(spiNo, SPI_CS_LOW);
  ret = SPI_Write(spiNo, pchUserBuf, length);
  SPI_SetCSSignal(spiNo, SPI_CS_HIGH);
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

  ret = SPI_Write(spiNo, pchUserBuf, length);
  return ret;
}

/*
 * ¹¦ÄÜ : ×¢²áSPI´¦ÀíµÄ»Øµ÷º¯Êý
 * SpiHandle : SPI»Øµ÷º¯ÊýÖ¸Õë
 * ·µ»Ø : ³É¹¦Ê±·µ»ØRET_SUCCES,Ê§°ÜÊ±·µ»ØÔ­Òò
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
 * ¹¦ÄÜ : ×¢ÏúSPI´¦ÀíµÄ»Øµ÷º¯Êý
 * ·µ»Ø : ³É¹¦Ê±·µ»ØRET_SUCCES,Ê§°ÜÊ±·µ»ØRET_FAILED
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

  //SPI³õÊ¼»¯
  spiCfg.ioctrlMode = SPI_IOCTL_POLL;
  spiCfg.modeSel = SPI_MASTER;
  spiCfg.divider = 2;
  spiCfg.bitPri = SPI_MSB_FIRST;
  spiCfg.bitLen = SPI_BL_8BIT;
  spiCfg.datatrans = SPI_DATA_LL;

  g_spiRdHandle = spiOpen(LCD_SPI);
  spiConfig(g_spiRdHandle, &spiCfg);

  //RST³õÊ¼»¯
  g_rstRdHandle = LCD_RST_PIN;
  GPIO_Enable(g_rstRdHandle);
  GPIO_SetDirection(g_rstRdHandle, GPIO_OUTPUT_DIRECTION);
  GPIO_SetValue(g_rstRdHandle, PIN_HIGH);

  //DC³õÊ¼»¯
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
  GPIO_SetDirection(g_dcRdHandle, GPIO_OUTPUT_DIRECTION);
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

//  ¿ªÆô¹Ø±ÕÏÔÊ¾
void open_lcd(BOOLEAN on_off)
{
  uint8 temp = (on_off | LCD_DISPLAY_OFF);
  cust_lcd_drv_write(SDK_LCD_CMD, 1, &temp);
  //simu_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

// ÉèÖÃÏÔÊ¾·½ÏòA0,A1
void set_lcd_adc(uint8 direction)
{
  uint8 temp = (direction|0xA0);
  cust_lcd_drv_write(SDK_LCD_CMD, 1, &temp);
}

//ÉèÖÃÏÔÊ¾µÄÆðÊ¼ÁÐ
void set_lcd_col(uint8 colum)
{
  uint8 temp = ((colum>>4)|0x10);
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
  temp = colum&0x0F;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//ÉèÖÃÏÔÊ¾ÆðÊ¼ÐÐ
void set_lcd_line(uint8 line)
{
  uint8 temp = (line|0x40);
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}


//ÉèÖÃÏÔÊ¾Ò³µØÖ·
void  set_lcd_page(uint8 pageNum)
{
  uint8 temp = (pageNum&0x0F)|0xB0;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//ÉèÖÃÏÔÊ¾Õý·´ÏÔÊ¾A6,A7
void  set_lcd_reverse(uint8 reverse)
{
  uint8 temp = reverse|0xA6;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//ÉèÖÃÈ«ÆÁÏÔÊ¾(Îª1Ê±ÀàËÆÇåÆÁÏÔÊ¾Ð§¹û)  A4£¬A5
void  set_lcd_all(uint8 display)
{
  uint8 temp = display|0xA4;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//ÉèÖÃÆ«Ñ¹  0£º1/9     1: 1/7   A2,A3
void set_lcd_bias(uint8 bias79)
{
  uint8 temp = bias79|0xA2;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//ÉèÖÃ¶Á-¸Ä-Ð´ Ä£Ê½£¬Ã¿´ÎÐ´²Ù×÷,cloum×Ô¼Ó
void set_lcd_R_M_W(void)
{
  uint8 temp = 0xE0;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}


//ÉèÖÃ½áÊø¶Á-¸Ä-Ð´Ä£Ê½
void  set_lcd_R_M_W_End(void)
{
  uint8 temp = 0xEE;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//ÉèÖÃÈí¼þ¸´Î»£º½«³õÊ¼»¯ÏÔÊ¾ÆðÊ¼ÐÐ¡¢ÆðÊ¼ÁÐµØÖ·¡¢ÆðÊ¼Ò³µØÖ·¡¢Õý³£Êä³öÄ£Ê½
void  set_lcd_soft_reset(void)
{
  uint8 temp = 0xE2;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//ÉèÖÃÐÐÉ¨Ãè·½Ïò0:COM0-COM63  1:COM63-COM0
void  set_lcd_com(uint8 comNum)
{
   uint8 temp = ((comNum<<3)&0x0F)|0xC0;
   cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//ÉèÖÃµçÔ´¿ØÖÆµÄCircuit operation mode  28-2F
void set_lcd_power_circuit(uint8 mode)
{
  uint8 temp = (mode&0x07)|0x28;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//ÉèÖÃÄÚ²¿µç×èµ÷ÕûÉèÖÃ20-27
void  set_lcd_resistor(uint8 resistor)
{
  uint8 temp = (resistor&0x07)|0x20;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//ÉèÖÃ¶Ô±È¶È£¬Í¬ÁÐÏÔÊ¾£¬Ë«×Ö²Ù×÷0-63
void  set_lcd_contrast(uint8 contrast)
{
  uint8 temp = 0x81;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
  temp = contrast&0x3F;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}


//ÉèÖÃ¾²Ì¬ÏÔÊ¾£¬Í¬ÉÏÎªË«×Ö²Ù×÷0£ºoff  1£ºon
void  set_lcd_static(uint8 on_off, uint8 flashMode)
{
  uint8 temp = (on_off&0x01)|0xAC;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
  temp = flashMode&0x03;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//´«ÊäLCD Êý¾Ý
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
  uint32 arg = 1;
  //GPIO_SetLcdBackLight( SCI_FALSE );
  PWM_Ioctl (0, PWM_CTL_S_CLKSRC, &arg);//select RTC clock source
  PWM_Ioctl (0, PWM_CTL_S_ONOFF, &arg);//pwm on
  PWM_Config (0, 5, 0);
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
    uint32 arg = 1;
    VTM88828A_Init();

    PWM_Ioctl (0, PWM_CTL_S_CLKSRC, &arg);//select RTC clock source
    PWM_Ioctl (0, PWM_CTL_S_ONOFF, &arg);//pwm on
    PWM_Config (0, 5, 100);
    LCD_Delay(3000);
    Mmi_Idle_Display();
    LCD_Delay(500);
    while(1) 
    { 
      LCD_Delay(1000);
    } 
}

void SPILCD_Test(void)
{
    SCI_CreateThread(
        "_SPILCD_Test_Task",
        "_SPILCD_Test_Queue",
        _SPILCD_Test_Task,
        0,
        NULL,
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


void custGetAscii12_12_data(uint8 *charCode, uint8 *p_result, uint16 *p_result_len)
{  
  uint8 qw=0; //ÇøºÅ£¬Î»ºÅ
  uint32 ulOffset;  //Æ«ÒÆ
  uint8 low=(uint8)(*charCode &0xFF);
  {
		qw =low;
		ulOffset = (uint32)(qw)*(GBK_12_12_CHAR_LEN>>1);		//¼ÆËã³öÆ«ÒÆÁ¿
		//Ó¢ÎÄ12 ×Ö½Ú£¬ºó6×Ö½ÚÖ»È¡¸ß4Î»
		memcpy(p_result,&ascii_6_12[ulOffset],GBK_12_12_CHAR_LEN>>1);
    	*p_result_len = GBK_12_12_CHAR_LEN>>1;
  } 		     
}


void mergeOneLineCharToHole(uint8 hole_xlen, uint8 x_now, uint8 y_now, flushType_e dispType, uint8*p_dest, uint8* p_source)
{
  uint8 x_offset =0,itemp=0;
  uint16 dest_offset = 0;
  uint16 dest_next_offset = 0;
  uint16 source_offset_next = 0;
  switch(dispType)
  {
    case FLUSH_LEFT:
      x_offset = 0;
      break;
    case FLUSH_MID:
      x_offset = (hole_xlen-x_now)/2;
      break;
    case FLUSH_RIGHT:
      x_offset = (hole_xlen-x_now);
      break;
    default:
      break;
  }
  dest_offset = (y_now/8)*hole_xlen+x_offset;
  dest_next_offset = dest_offset+hole_xlen;
  source_offset_next = hole_xlen;

  if(y_now&0x07)//ÐèÒª4 bit merge ²Ù×÷3´Î
  {
    for(itemp =0;itemp < x_now;itemp++)
    {
      p_dest[dest_offset+itemp] = (p_dest[dest_offset+itemp]&0xF0)|(( p_source[itemp]&0xF0)>>4);
      p_dest[dest_next_offset+itemp] = ((p_source[itemp]&0x0F)<<4)|((p_source[source_offset_next+itemp]&0xF0)>>4);
    }
  }
  else
  {//merge Ö®ºóµÄ4bit£¬ ¿ÉÒÔÖ±½Ó¿½±´£¬²»ÓÃmerge
     for(itemp =0;itemp < x_now;itemp++)
     {
       p_dest[dest_offset+itemp] = p_source[itemp];
       p_dest[dest_next_offset+itemp] = p_source[source_offset_next+itemp];
     }
  }

}



int update_display(uint8 x, uint8 y, uint8 xlen, uint8 ylen, uint8* p_data)
{
  uint8 y_page_start=(y)>>3, y_offset=(y)&0x07;/*A_bits*/
  uint8 y_offset_reserve = 8-y_offset;/*A'*/
  uint8 y_refresh_pages=(ylen+y_offset+7)>>3;
  uint8 B_bits = (0==((y+ylen)&0x07))?8:((y+ylen)&0x07);/*B_bits*/
  uint8 merge_page = (ylen+7)>>3;
  uint8 itemp=0,jtemp=0;
  uint8 *p_data_buf = p_data;
  uint16 updata_data_offset= 0;
  //start merge display buf
  if((x+xlen > MONO_X_PIXELS)||(y+ylen> MONO_Y_PIXELS)||(0==xlen)||(0==ylen))
    return -1;

  //merge source to display buf
  for(itemp =0; itemp < merge_page;itemp++)
  {
    if(itemp == (merge_page-1))//Ô´Êý¾ÝµÄ×îºóÒ»²¿·Ö£¬Õû8bit»ò²¿·Ö
    {
      uint8 C_bits= y_offset+((0==(ylen&0x07))?8:(ylen&0x07));
      if(C_bits > 8)//Ô´Êý¾ÝÒªmergeµ½lcd µÄ2¸öpageÖÐ
      {// ÌØ¶¨µÄbit Î»ÐèÒªÍùlcd ÉÏÏÂ2¸öpage½øÐÐmerge
        uint16 lcd_offset = ((y+itemp*8)/8)*MONO_X_PIXELS+x;
        uint16 lcd_offset_next = lcd_offset+MONO_X_PIXELS;
        uint16 source_offset = itemp*xlen;
        uint8 page_H_bits = 8-y_offset;
        uint8 page_L_bits = ((0==(ylen&0x07))?8:(ylen&0x07))-page_H_bits;        

        for(jtemp=0;jtemp<xlen;jtemp++)
        {
          uint8 s_data = *(p_data_buf+source_offset+jtemp);
          uint8 lcd_buf = g_lcd_pixels_buf[lcd_offset+jtemp];
          uint8 lcd_buf_next = g_lcd_pixels_buf[lcd_offset_next+jtemp];
          g_lcd_pixels_buf[lcd_offset+jtemp]=((s_data&pixel_merge_bits[page_H_bits])>>y_offset)|
                                              (lcd_buf&pixel_merge_bits[y_offset]); 
          g_lcd_pixels_buf[lcd_offset_next+jtemp]=((s_data<<page_H_bits)&pixel_merge_bits[page_L_bits])|
                                                  (lcd_buf_next&(~pixel_merge_bits[page_L_bits])); 
        }        
      }
      else//Ô´Êý¾ÝÖ»ÒªÔÚlcd µÄÒ»¸öpage½øÐÐmerge
      {
        uint8 distence = B_bits-y_offset;
        uint8 merge_mask = (~pixel_merge_bits[distence])>>y_offset;
        uint16 lcd_offset = (y_page_start+itemp)*MONO_X_PIXELS+x;
        uint16 source_offset = itemp*xlen;
        //SDK_FOTA_DEBUG_STR_2_SPY("last 1 page itemp%d lcd_h%d,source%d,dis%d,offset%d,mask0x%02x",
        //                   itemp,lcd_offset,source_offset,distence,y_offset,merge_mask);
        for(jtemp = 0;jtemp <xlen;jtemp++)
        {
          uint8 s_data = *(p_data_buf+source_offset+jtemp);
          uint8 lcd_buf =  g_lcd_pixels_buf[lcd_offset+jtemp];
          g_lcd_pixels_buf[lcd_offset+jtemp] = (lcd_buf&merge_mask)|((s_data&pixel_merge_bits[distence])>>y_offset);
        }
      }     
    }
    else
    {//ÆäËûÒ³Ãæ, Ã¿8bit¶¼¿ÉÄÜÐèÒªÍùlcdµÄÉÏÏÂpage ½øÐÐmerge
      uint16 lcd_offset = ((y+itemp*8)/8)*MONO_X_PIXELS+x;
      uint16 lcd_offset_next = lcd_offset+MONO_X_PIXELS;
      uint16 source_offset = itemp*xlen;

      for(jtemp=0;jtemp<xlen;jtemp++)
      {
        uint8 s_data= *(p_data_buf+source_offset+jtemp);
        uint8 lcd_buf = g_lcd_pixels_buf[lcd_offset+jtemp];
        uint8 lcd_buf_next = g_lcd_pixels_buf[lcd_offset_next+jtemp];
        g_lcd_pixels_buf[lcd_offset+jtemp]=((s_data&pixel_merge_bits[y_offset_reserve])>>y_offset)|
                                              (lcd_buf&pixel_merge_bits[y_offset]);
        g_lcd_pixels_buf[lcd_offset_next+jtemp]=((s_data&(~pixel_merge_bits[y_offset_reserve]))<<y_offset_reserve)|
                                                  (lcd_buf_next&(~pixel_merge_bits[y_offset]));        
      }      
    }
  }  
  // merge data end ,so call lcd function, updata data to lcd
  updata_data_offset =  y_page_start*MONO_X_PIXELS+x;
  for(itemp = 0;itemp < y_refresh_pages;itemp++)
  {    
    uint8 page_num = 7-y_page_start-itemp;
    set_lcd_col(x+LCD_X_PIXEX_OFFSET);//set colum
    #if 0//NT7534_LCD_ENABLE
    if(page_num >=4)
    {
      set_lcd_page(page_num-4);
    }
    else
    {
      set_lcd_page(page_num+4);
    }
    #else
    set_lcd_page(page_num);//set pages lcd ÖÐµÄpage ±àºÅ´ÓÉÏÍùÏÂÊÇ±äÐ¡µÄ±àºÅ
    #endif
    set_lcd_data(xlen,&g_lcd_pixels_buf[updata_data_offset+itemp*MONO_X_PIXELS]);   
  }

  return(1);
}



void fillAscStr2Hole(holeType_t holeInfo, flushType_e type, uint8 *p_STR, uint8 agChar_len,uint8 reserve)
{
  uint8 y_remay = holeInfo.ylen, /*agCharRemay = agChar_len,*/x_now=0,y_now =0,itemp=0,jtemp=0;
  uint16 holeSize = ((holeInfo.ylen+7)/8)*holeInfo.xlen;
  uint8 *p_hole_buf = NULL;
  uint8 *p_one_line_buf = NULL;  
  uint16 one_line_buf_size = 2*holeInfo.xlen;
  uint8 gbPixelBuf[GB_MAX_LEN]={0};  
  uint16 gbPixelLen = 0;
  if((holeInfo.ylen < GB_PIXEL_H) || (0 == agChar_len)||(type > FLUSH_RIGHT))//hole µÄÊúÖ±·½Ïò²»¹»Ò»¸ö×Ö·ûÏÔÊ¾£¬ÍË³ö
    return;
  p_hole_buf = malloc(holeSize);//¿ª±ÙÕû¸öhole µÄ¿Õ¼ä
  
  p_one_line_buf = malloc(one_line_buf_size);//¿ª±ÙÒ»ÐÐºº×ÖµÄ¿Õ¼ä ,Îª×óÓÒ¶ÔÆëÏÈ»º³åÊý¾Ý
  
  memset(p_hole_buf,0x00,holeSize);//Çå³ý»º³åÇø
  memset(p_one_line_buf,0x00,one_line_buf_size);
  for(itemp = 0;itemp < agChar_len;itemp++)
  {
    if(y_remay < GB_PIXEL_H)
      break;//Ê£Óà¿Õ¼ä²»¹»·ÅÏÂÒ»¸ö×Ö·û£¬ÍË³öÑ­»·
    memset(gbPixelBuf,0x00,sizeof(gbPixelBuf));
    custGetAscii12_12_data(&p_STR[itemp],gbPixelBuf,&gbPixelLen);//»ñÈ¡Ò»¸öºº×ÖµãÕóµ½»º³å
    if((x_now+(gbPixelLen/2))>holeInfo.xlen)
    {//¸Ãºº×ÖÐÐ²»¹»¿Õ¼ä·ÅÈëÒ»¸öºº×Ö,ÐèÒªÁíÆðÒ»ÐÐ,ÕûÀí¸Ãºº×ÖÐÐµ½hole ÖÐÈ¥
      if(y_remay < (GB_PIXEL_H*2))
        break;//yÖáÊ£Óà¿Õ¼ä²»¹»ÁíÆðÒ»ÐÐºº×Ö£¬ÍË³öÑ­»·£¬ºóÃæµÄ´úÂë½øÐÐÌî×Ö
      mergeOneLineCharToHole(holeInfo.xlen,x_now,y_now,type,p_hole_buf,p_one_line_buf);
      x_now = 0;
      y_now += GB_PIXEL_H;
      y_remay -= GB_PIXEL_H;
      memset(p_one_line_buf,0x00,one_line_buf_size);
      {
        uint8 charPixel = (gbPixelLen/2);
        uint16 offset_1 = x_now;
        uint16 offset_2 = x_now+holeInfo.xlen;
        for(jtemp=0;jtemp < charPixel;jtemp++)
        {
          *(p_one_line_buf+offset_1+jtemp) = gbPixelBuf[jtemp];
          *(p_one_line_buf+offset_2+jtemp)= gbPixelBuf[jtemp+charPixel];   
        } 
        x_now += charPixel;
      }
    }
    else
    {//¸ÃÐÐ»¹ÓÐ¿Õ¼ä£¬Ìî³ä×Ö·ûµãÕóµ½¸ÃÐÐ»º³å
      uint8 charPixel = (gbPixelLen/2);
      uint16 offset_1 = x_now;
      uint16 offset_2 = x_now+holeInfo.xlen;
      for(jtemp=0;jtemp < charPixel;jtemp++)
      {
        *(p_one_line_buf+offset_1+jtemp) = gbPixelBuf[jtemp];
        *(p_one_line_buf+offset_2+jtemp)= gbPixelBuf[jtemp+charPixel];   
      } 
      x_now += charPixel;//Æ«ÒÆx_now £¬×¼´¢´æÏÂÒ»¸öºº×ÖµãÕó
    }    	
  }
  mergeOneLineCharToHole(holeInfo.xlen,x_now,y_now,type,p_hole_buf,p_one_line_buf);//Ìî³ä×îºóÒ»ÐÐµ½holeÖÐ
  if(reserve)
  {
    for(itemp = 0;itemp < holeSize;itemp++)
    {
      p_hole_buf[itemp] = ~p_hole_buf[itemp];
    }
  }
  update_display(holeInfo.x,holeInfo.y,holeInfo.xlen,holeInfo.ylen,p_hole_buf);//¸üÐÂholeµ½lcd
  if(p_hole_buf != NULL)
  {
    free(p_hole_buf);
    p_hole_buf =NULL;  
  }

  if(p_one_line_buf != NULL)
  {
    free(p_one_line_buf);
    p_one_line_buf = NULL;  
  }
}


int display_Screan(uint8 *p_data, uint16 dataLen)
{
  uint8 itemp = 0;
  if(dataLen > DISPLAY_BUF_SIZE)
    return -1;
  memcpy(g_lcd_pixels_buf,p_data,dataLen);
  for(itemp = 0;itemp < MONO_Y_PAGES;itemp++)
  {
    set_lcd_page(itemp);
    set_lcd_col(0);
    set_lcd_data(MONO_X_PIXELS,&g_lcd_pixels_buf[(MONO_Y_PAGES-itemp-1)*MONO_X_PIXELS]);
  }

  return(1);
}

static void Mmi_Idle_Display(void)
{
	holeType_t nullHole = {1,10,126,12};
	holeType_t menuHole = {22,50,24,12};
	holeType_t returnHole = {76,50,36,12};
	uint8 nullHint[] = "display test";
	uint8 menuHint[] = "MENU";
	uint8 returnHint[] = "RETURN";

	//´¦Àí´ý»úÏÔÊ¾
	fillAscStr2Hole(nullHole,FLUSH_MID,nullHint,strlen((int8*)nullHint),0);
	fillAscStr2Hole(menuHole,FLUSH_MID,menuHint,strlen((int8*)menuHint),0);
	fillAscStr2Hole(returnHole,FLUSH_MID,returnHint,strlen((int8*)returnHint),0);
}

#ifdef   __cplusplus
    }
#endif




