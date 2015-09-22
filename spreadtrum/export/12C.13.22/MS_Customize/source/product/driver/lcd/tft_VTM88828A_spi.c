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

typedef int DEV_HANDLE;
typedef unsigned long S32;
typedef char                S8;
typedef short               S16;

typedef  long s32;
typedef char                s8;
typedef short               s16;
typedef  unsigned char bool;
typedef unsigned int size_t;

typedef unsigned long       U32;
typedef unsigned char       U8;
typedef unsigned short      U16;
typedef unsigned long       u32;
typedef unsigned char       u8;
typedef unsigned short      u16;
typedef int                 BOOL_T;



#define MONO_X_PIXELS 128
#define MONO_Y_PIXELS 64
#define MONO_Y_PAGE_SIZE 8 //bits
#define MONO_Y_PAGES (MONO_Y_PIXELS+MONO_Y_PAGE_SIZE-1)/MONO_Y_PAGE_SIZE
#define DISPLAY_BUF_SIZE (MONO_X_PIXELS*MONO_Y_PIXELS)/MONO_Y_PAGE_SIZE

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
  U8 x_image;
  U8 y_image;
}fillImage_t;


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

typedef struct  _mdp_pwm_cfg_s
{
    U32  freq;
    U16  duty_cycle;
}MDP_PWM_CFG_S;
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

typedef enum
{
    MDP_GPIO_TYPE_INPUT = 0,
    MDP_GPIO_TYPE_OUTPUT
}MDP_GPIO_TYPE;

typedef enum
{
    MDP_GPIO_LEVEL_LOW = 0, //µÍµçÆ½
    MDP_GPIO_LEVEL_HIGH,        //¸ßµçÆ½
    MDP_GPIO_LEVEL_UNVALID  //ÎÞÐ§×´Ì¬
}MDP_GPIO_LEVEL;

typedef struct
{
    U8 int_enable;
    U8 b_shake;
    U16 n_shake_time;  //ms
}MDP_GPIO_INT_CTRL_S;

typedef struct  _mdp_gpio_cfg_s
{
    //U8 gpio_id;
    MDP_GPIO_TYPE  gpio_type;
    MDP_GPIO_LEVEL gpio_lvl;
    MDP_GPIO_INT_CTRL_S gpio_int_ctl;
}MDP_GPIO_CFG_S;

typedef struct Gpio_Body_Tag
{
    DEV_HANDLE      handle;
    MDP_GPIO_CFG_S  cfg;
    BOOL_T          cfgFlag;
}Gpio_Body_T;

typedef enum
{
    GPIO_PIN_INPUT,
    GPIO_PIN_OUTPUT,
    GPIO_Mode_IN_FLOATING,
    GPIO_Mode_IPD,
    GPIO_Mode_IPU,
    GPIO_Mode_Out_OD,
    GPIO_Mode_Out_PP,
    GPIO_Mode_AF_OD,
    GPIO_Mode_AF_PP
}gpio_direction_e;

typedef enum
{
    PIN_LOW = 0,
    PIN_HIGH,
    PIN_STATE_INVALID
}gpio_level_e;

typedef struct
{
  U8 x;
  U8 y;
  U8 xlen;
  U8 ylen;
}holeType_t;

typedef enum{
  FLUSH_LEFT = 0,
  FLUSH_MID = 1,
  FLUSH_RIGHT
}flushType_e;

typedef enum//PWM ID
{
    PWM_ID_0 = 0,
    PWM_ID_1,
    PWM_ID_2,

    PWM_ID_MAX
}pwm_id_E;

typedef struct Pwm_Body_Tag
{
    DEV_HANDLE      handle;
    MDP_PWM_CFG_S   cfg;
}Pwm_Body_T;

#define LCD_SPI (SPI_NO_1)
#define LCD_RST_PIN  59 //GPIO_59
#define LCD_CMD_DATA_PIN 49 //GPIO_49

#define GPIO_INPUT_DIRECTION  0
#define GPIO_OUTPUT_DIRECTION 1
#define PIN_HIGH 1
#define PIN_LOW 0
#define GPIO_ID_MAX 150
#define PWM_BACKLIGHT_FREQ_DEF                  200//LCD±³¹âÄ¬ÈÏµÄPWMÆµÂÊ
#define PWM_BACKLIGHT_DUTYCYCLE_ON              100//LCD±³¹â³£ÁÁµÄÕ¼¿Õ±È
#define PWM_BACKLIGHT_DUTYCYCLE_OFF         0//LCD±³¹â³£ÃðµÄÕ¼¿Õ±È
#define MDP_TRUE    1
#define MDP_FALSE  0

#define  false 0
#define  true  1
#define  RET_TRUE  1
#define  RET_FALSE 0

#define  RET_SUCCESS 0
#define  OS_SUCCESS  0
#define RET_FAILED   -1
#define  OS_FAIL     -1
#define  RET_ERR_NOT_OPENED   -2
#define  RET_ERR_NOT_CLOSED   -3

#define  RET_ERR_INVALID_PARAM   -4
#define  RET_ERR_DISCONNECT    -5
#define RET_ERR_INVALID_HANDLE      -6
#define  RET_ERR_TIME_OUT     -7
#define RET_ERR_OPEN_TIMES          -7

#define  BINARY_MODE   1
#define  TEXT_MODE   0

#define MDP_SUCC 0
#define MDP_FAIL ((U32)-1)

#define PWM_CTL_S_ONOFF 0x30    /*Set PWM ON/OFF*/
#define PWM_CTL_S_CLKSRC 0x31   /*Set CLK sorce*/
#define PWM_CTL_G_CLKSRC 0x32   /*Get CLK sorce*/
#define PWM_CTL_S_POLARITY 0x33 /*Set PWM Output Polarity*/

uint8 *p_lcd_pixels_buf = NULL;


static Spi_Body_T spiBody[SPI_NO_MAX] = {0};//SPI½á¹¹±í
static Gpio_Body_T gpioBody[GPIO_ID_MAX] = {0};//GPIO½á¹¹±í
static Pwm_Body_T pwmBody[PWM_ID_MAX] = {0};//PWM½á¹¹±í
static const MDP_PWM_CFG_S BackLightOnCfg = {PWM_BACKLIGHT_FREQ_DEF,PWM_BACKLIGHT_DUTYCYCLE_ON};//±³¹â¿ªÆô
static MDP_PWM_CFG_S pwmBackLightCfg = {PWM_BACKLIGHT_FREQ_DEF,PWM_BACKLIGHT_DUTYCYCLE_ON};//LCD±³¹âPWMÊä³öÅäÖÃ
static const MDP_PWM_CFG_S BackLightOffCfg = {PWM_BACKLIGHT_FREQ_DEF,PWM_BACKLIGHT_DUTYCYCLE_OFF};//±³¹â¹Ø±Õ


static DEV_HANDLE spiRdHandle = 0;
static DEV_HANDLE rstRdHandle = 0;
static DEV_HANDLE dcRdHandle = 0;
static DEV_HANDLE pwmBackLightHandle = 0;


#define GBK_ASCII_MAX 0xA1
#define GBK_16_16_CHAR_LEN 32
#define GBK_12_12_CHAR_LEN 24
#define GB16_16_PIXEL_H 16
#define GB16_16_MAX_LEN ((GB16_16_PIXEL_H+7)>>3)*GB16_16_PIXEL_H

U8 pixel_merge_bits[MONO_Y_PAGE_SIZE+1]={
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


U8 pixel_bits[MONO_Y_PAGE_SIZE]={
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
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//"   ",9
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


static int getSpiNo(int Handle, uint8* no_p);
BOOLEAN m2mHalSpiInit(void);
DEV_HANDLE spiOpen(uint32 num);
int spiClose(int Handle);
int spiConfig(int DevHandle, spi_config_t* pConfig);
int spiRead(int DevHandle,uint8* pchTxBuf,uint8* pchUserBuf,uint32 dwCount);
int spiWrite(int DevHandle, uint8* pchUserBuf, uint32 length);
void cust_cfg_lcd_spi(void);
void custLcdRstPinSet(uint8 val);
void custLcdCmdDataSet(uint8 val);
void custSpiWriteData(uint8 *pData, uint16 wLen);
void cust_lcd_drv_write(uint8 cmd_data, uint16 writeLen, uint8 *data);
void open_lcd(BOOL_T on_off);
void set_lcd_adc(uint8 direction);
void set_lcd_col(uint8 colum);
void set_lcd_line(uint8 line);
void  set_lcd_page(uint8 pageNum);
void  set_lcd_reverse(uint8 reverse);
void  set_lcd_all(uint8 display);
void set_lcd_bias(uint8 bias79);
void set_lcd_R_M_W(void);
void  set_lcd_R_M_W_End(void);
void  set_lcd_soft_reset(void);
void  set_lcd_com(uint8 comNum);
void set_lcd_power_circuit(uint8 mode);
void  set_lcd_resistor(uint8 resistor);
void  set_lcd_contrast(uint8 contrast);
void  set_lcd_static(uint8 on_off, uint8 flashMode);
void set_lcd_data(uint16 writeLen,uint8 *data);
void VTM88828A_driver(void);
void VTM88828A_Init(void);
void _SPILCD_Test_Task(uint32 argc, void* argv);
DEV_HANDLE gpioOpen(U32 gpionum);
s32 getGpioNum(DEV_HANDLE Handle,U32* Num_p);
void  tft_mdp_gpio_default_cb(U32 gpio_id, U32 status);
U32 tft_sys_gpio_cfg_set(U32 gpionum, MDP_GPIO_CFG_S* param, U32 ulParamSize);
int tft_sys_spi_cfg_set(int spino, MDP_SPI_CFG_S* param, int ulParamSize);

s32 gpioConfig(DEV_HANDLE Handle,gpio_direction_e GPIODirect);
U32 tft_sys_gpio_cfg_get(U32 gpionum, MDP_GPIO_CFG_S* param, U32 ulParamSize);
S32 tft_sys_gpio_write(U32 gpionum, U8 data, U32 ulDataLen);
s32 gpioWrite(DEV_HANDLE Handle,gpio_level_e GPIOLevel);
void custGetAscii12_12_data(U8 *charCode, U8 *p_result, U16 *p_result_len);
void mergeOneLineCharToHole(U8 hole_xlen, U8 x_now, U8 y_now, flushType_e dispType, U8*p_dest, U8* p_source);
int update_display(U8 x, U8 y, U8 xlen, U8 ylen, U8* p_data);
void fillAscStr2Hole(holeType_t holeInfo, flushType_e type, U8 *p_STR, U8 agChar_len,U8 reserve);
int display_Screan(U8 *p_data, U16 dataLen);
static void Mmi_Idle_Display(void);


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
BOOLEAN m2mHalSpiInit(void)
{
  //²ÎÊý³õÊ¼»¯
  memset(spiBody, 0x00, sizeof(spiBody));
  memset(gpioBody, 0x00, sizeof(gpioBody));
  memset(pwmBody,0x00,sizeof(pwmBody));
  return SCI_TRUE;
}

/*
 * ¹¦ÄÜ : ´ò¿ªSPI
 * num : SPIÍ¨µÀ±êÊ¶
 * ·µ»Ø : ³É¹¦Ê±·µ»Ø¶ÔÓ¦µÄ¾ä±ú(num + 1),Ê§°ÜÊ±·µ»ØÔ­Òò
 */
DEV_HANDLE spiOpen(uint32 num)
{

  DEV_HANDLE ret;
  spiBody[num].handle = (int)(num + 1);
  ret = spiBody[num].handle;

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


int tft_sys_spi_cfg_set(int spino, MDP_SPI_CFG_S* param, int ulParamSize)
{
    MDP_SPI_CFG_S *SPIConfig = (MDP_SPI_CFG_S*)param;
    SPI_CONFIG_T spi_cfg = {0};


    spi_cfg.freq = SPIConfig->freq;
    spi_cfg.cs_id = SPI_CS_ID0;
    spi_cfg.lsb_first = SPIConfig->lsb_first;
    spi_cfg.rx_shift_edge = SPIConfig->rx_shift_edge;
    spi_cfg.sck_reverse = SPIConfig->sck_reverse;
    spi_cfg.tx_bit_length = SPIConfig->tx_bit_length;
    spi_cfg.tx_shift_edge = TRUE;


    SPI_Open (spino, &spi_cfg);

    if (SCI_FALSE == SPI_Init (spino))
    {
        return FALSE;
    }

    return TRUE;
}


/*
 * ¹¦ÄÜ : SPIÅäÖÃ
 * DevHandle : Éè±¸¾ä±ú
 * pConfig : SPIÅäÖÃ
 * ·µ»Ø : ³É¹¦Ê±·µ»ØRET_SUCCES,Ê§°ÜÊ±·µ»ØÔ­Òò
 */
int spiConfig(int DevHandle, spi_config_t* pConfig)
{
    s32 ret;
    U8 spiNo;
    MDP_SPI_CFG_S devCfg;

    ret = getSpiNo(DevHandle,&spiNo);
#ifndef DAM_TEST
  wis_debug("---ret=%d,pConfig=0x%x, spiNo=%d--- \r\n ",ret, pConfig, spiNo);
#endif
    if(ret != RET_SUCCESS)
    {
        return(ret);
    }
    else if(pConfig == PNULL)
    {
        return(RET_ERR_INVALID_PARAM);
    }

    memset(&devCfg,0x00,sizeof(devCfg));

    //²ÎÊý×ª»»
    if((pConfig->ioctrlMode == SPI_IOCTL_DMA) || (pConfig->modeSel == SPI_SLAVER))
    {
        return(RET_ERR_INVALID_PARAM);
    }

    if(pConfig->divider >= 2)
    {
        devCfg.freq = 6500000 / (2 * (pConfig->divider - 1));
    }
    else
    {
        devCfg.freq = 6500000;
    }

    if(pConfig->bitPri == SPI_LSB_FIRST)
    {
        devCfg.lsb_first = MDP_TRUE;
    }
    else
    {
        devCfg.lsb_first = MDP_FALSE;
    }

    if(pConfig->bitLen == SPI_BL_16BIT)
    {
        devCfg.tx_bit_length = 16;
    }
    else
    {
        devCfg.tx_bit_length = 8;
    }

    if(pConfig->datatrans == SPI_DATA_HH)
    {
        devCfg.sck_reverse = MDP_TRUE;
        devCfg.rx_shift_edge = MDP_TRUE;
    }
    else if(pConfig->datatrans == SPI_DATA_HL)
    {
        devCfg.sck_reverse = MDP_TRUE;
        devCfg.rx_shift_edge = MDP_FALSE;
    }
    else if(pConfig->datatrans == SPI_DATA_LH)
    {
        devCfg.sck_reverse = MDP_FALSE;
        devCfg.rx_shift_edge = MDP_TRUE;
    }
    else
    {
        devCfg.sck_reverse = MDP_FALSE;
        devCfg.rx_shift_edge = MDP_FALSE;
    }

    //Ö´ÐÐÅäÖÃ
    if(tft_sys_spi_cfg_set(spiNo,&devCfg,sizeof(MDP_SPI_CFG_S)) == MDP_SUCC)
    {
        spiBody[spiNo].cfg = devCfg;
        return(RET_SUCCESS);
    }
    else
    {
        return(RET_FAILED);
    }
}


    S32 tft_sys_spi_write(uint8 spiNo, uint8* pchUserBuf, U32 ulDataLen)
    {


        SPI_SetCSSignal(spiNo, SPI_CS_LOW);
        SPI_Write(spiNo, pchUserBuf, ulDataLen);
        SPI_SetCSSignal(spiNo, SPI_CS_HIGH);
        return MDP_SUCC;
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


  ret = tft_sys_spi_write(spiNo, pchUserBuf, length);
  return ret;
}




void cust_cfg_lcd_spi(void)
{
    spi_config_t spiCfg;

    //SPI³õÊ¼»¯
    spiCfg.ioctrlMode = SPI_IOCTL_POLL;
    spiCfg.modeSel = SPI_MASTER;
    spiCfg.divider = 2;
    spiCfg.bitPri = SPI_MSB_FIRST;
    spiCfg.bitLen = SPI_BL_8BIT;
    spiCfg.datatrans = SPI_DATA_LL;
    spiRdHandle = spiOpen(LCD_SPI);
    spiConfig(spiRdHandle,&spiCfg);

    //RST³õÊ¼»¯
    rstRdHandle = gpioOpen(LCD_RST_PIN);
    gpioConfig(rstRdHandle,GPIO_PIN_OUTPUT);
    gpioWrite(rstRdHandle,PIN_HIGH);

    //DC³õÊ¼»¯
    dcRdHandle = gpioOpen(LCD_CMD_DATA_PIN);
    gpioConfig(dcRdHandle,GPIO_PIN_OUTPUT);
    gpioWrite(dcRdHandle,PIN_HIGH);
}

void VTM88828A_driver(void)
{

    U8 itemp = 0;
    U8 tempBuf[132] = {0x00};
    cust_cfg_lcd_spi();

    p_lcd_pixels_buf =  malloc(DISPLAY_BUF_SIZE);
    memset(p_lcd_pixels_buf,0x00,DISPLAY_BUF_SIZE);
    custLcdRstPinSet(1);
    SCI_Sleep(10);
    custLcdRstPinSet(0);
    SCI_Sleep(20);
    custLcdRstPinSet(1);

    set_lcd_soft_reset();

    SCI_Sleep(10);
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
    SCI_Sleep(10);
    open_lcd(1);//open display
    for(itemp = 0;itemp< 8;itemp++)
    {
        memset(tempBuf,0x00,sizeof(tempBuf));
        set_lcd_page(itemp);
        set_lcd_col(0);
        set_lcd_data(131,tempBuf);
    }
    set_lcd_page(8);
    set_lcd_col(0);
    set_lcd_data(131,tempBuf);
}



U32 tft_sys_pwm_cfg_set(U8 pwm_num, void* param, U32 ulParamSize)
{
    MDP_PWM_CFG_S *cfg = (MDP_PWM_CFG_S*)param;
    uint32 arg = 1;


    PWM_Ioctl (pwm_num, PWM_CTL_S_CLKSRC, &arg);//select RTC clock source
    PWM_Ioctl (pwm_num, PWM_CTL_S_ONOFF, &arg);//pwm on
    PWM_Config (pwm_num,cfg->freq,cfg->duty_cycle);

    return MDP_SUCC;
}


s32 getPwmId(DEV_HANDLE Handle,U8* id_p)
{
    U8 iTemp;

    *id_p = 0;
    if(Handle <= 0)
    {
        return(RET_ERR_INVALID_HANDLE);
    }

    for(iTemp = PWM_ID_0;iTemp < PWM_ID_MAX;iTemp++)
    {
        if(pwmBody[iTemp].handle == Handle)
        {
            *id_p = iTemp;
            return(RET_SUCCESS);
        }
    }

    return(RET_ERR_NOT_OPENED);
}



/*
 * ¹¦ÄÜ : PWMÆôÓÃ
 * Handle : Éè±¸¾ä±ú
 * halfPeriod : Êä³öÖÜÆÚ(ms)
 * pwmLevel : ¸ßµÍµçÆ½Õ¼¿Õ±È(0~100)
 * ·µ»Ø : ³É¹¦Ê±·µ»ØRET_SUCCES,Ê§°ÜÊ±·µ»ØÔ­Òò
 */
s32 pwmEnable(DEV_HANDLE Handle,u8 halfPeriod,u8 pwmLevel)
{
    s32 ret;
    U8 pwmId;
    MDP_PWM_CFG_S pwmCfg;

    ret = getPwmId(Handle,&pwmId);
    if(ret != RET_SUCCESS)
    {
        return(ret);
    }

    memset(&pwmCfg,0x00,sizeof(pwmCfg));


    //²ÎÊý¼ì²é
    if((halfPeriod == 0) || (pwmLevel > 100))
    {
        return(RET_ERR_INVALID_PARAM);
    }
    else
    {
        pwmCfg.freq = 1000 / halfPeriod;
        pwmCfg.duty_cycle = pwmLevel;
    }

    //Ö´ÐÐÅäÖÃ
    if(tft_sys_pwm_cfg_set(pwmId,(void*)&pwmCfg,sizeof(MDP_PWM_CFG_S)) == MDP_SUCC)
    {
        pwmBody[pwmId].cfg = pwmCfg;
        return(RET_SUCCESS);
    }
    else
    {
        return(RET_FAILED);
    }
}


DEV_HANDLE pwmOpen(u32 Opt)
{
    DEV_HANDLE ret;
    U8 pwmId;

    //Ä¬ÈÏ¹Ì¶¨Ê¹ÓÃPWM_ID_0
    pwmId = PWM_ID_0;
    if(pwmBody[pwmId].handle > 0)
    {
        ret = RET_ERR_OPEN_TIMES;
    }
    else
    {
        pwmBody[pwmId].handle = (DEV_HANDLE)(pwmId + 1);
        ret = pwmBody[pwmId].handle;
    }

    return(ret);
}

BOOL_T Cust_Pwm_BackLight_Cfg_Set(MDP_PWM_CFG_S pwmCfg)
{
    if((pwmCfg.freq == 0) || (pwmCfg.duty_cycle > 100))
    {
        return(MDP_FALSE);
    }
    else if(pwmEnable(pwmBackLightHandle,1000 / pwmCfg.freq,pwmCfg.duty_cycle) == RET_SUCCESS)
    {
        pwmBackLightCfg = pwmCfg;
        return(MDP_TRUE);
    }
    else
    {
        return(MDP_FALSE);
    }
}

/***************************************************************
                  ÒÔÏÂ¶¨Òå±¾ÎÄ¼þÖÐÊ¹ÓÃµÄ¹«¹²º¯Êý
***************************************************************/
/*
 * ¹¦ÄÜ : PWM³õÊ¼»¯
 * ·µ»Ø : ÎÞ
 */
void Cust_Pwm_BackLight_Init(void)
{
    pwmBackLightHandle = pwmOpen(0);
}

/**************************************************************************************/
// Description: initialize all LCD with LCDC MCU MODE and LCDC mcu mode
// Global resource dependence:
// Author:
// Note:
/**************************************************************************************/
void VTM88828A_Init(void)
{

  m2mHalSpiInit();
  Cust_Pwm_BackLight_Init();
  VTM88828A_driver();
  Mmi_Idle_Display();
  Cust_Pwm_BackLight_Cfg_Set(BackLightOnCfg);
  SCI_Sleep(3000);
}

static BOOLEAN allowRunning = SCI_FALSE;

void _SPILCD_Test_Task(uint32 argc, void* argv)
{
    wis_debug("---VTM88828A_Init ---\r\n");
    VTM88828A_Init();
    SCI_Sleep(500);

    while(allowRunning)
    {
        SCI_Sleep(1000);
    }

    wis_debug("---VTM88828A exit ---\r\n");
    Cust_Pwm_BackLight_Cfg_Set(BackLightOffCfg);
    open_lcd(0);
    SCI_Sleep(10);
    //DC»¹Ô­
    gpioConfig(dcRdHandle,GPIO_PIN_INPUT);
    gpioWrite(dcRdHandle,PIN_LOW);
    //RST»¹Ô­
    gpioConfig(rstRdHandle,GPIO_PIN_INPUT);
    gpioWrite(rstRdHandle,PIN_LOW);

    spiClose(LCD_SPI);
}

void SPILCD_Test(void)
{
    wis_debug("---SPILCD_Test----\r\n");
    SCI_CreateThread(
        "_SPILCD_Test_Task",
        "_SPILCD_Test_Queue",
        _SPILCD_Test_Task,
        0,
        PNULL,
        0x1000,
        10,
        1,//SCI_APP_PRI(31),
        SCI_PREEMPT,
        SCI_AUTO_START);
}

PUBLIC void SPILCD_Open(void) {
    if (allowRunning != SCI_TRUE) {
        allowRunning = SCI_TRUE;
        SPILCD_Test();
    }
}

PUBLIC void SPILCD_Close(void) {
    allowRunning = SCI_FALSE;
}

DEV_HANDLE gpioOpen(U32 gpionum)
{
    DEV_HANDLE ret;

    gpioBody[gpionum].handle = (DEV_HANDLE)(gpionum + 1);
    ret = gpioBody[gpionum].handle;

    return(ret);
}


s32 getGpioNum(DEV_HANDLE Handle,U32* Num_p)
{
    U32 iTemp;

    *Num_p = 0;
    if(Handle <= 0)
    {
        return(SCI_ERROR);
    }
    for(iTemp = 0;iTemp < GPIO_ID_MAX;iTemp++)
    {
        if(gpioBody[iTemp].handle == Handle)
        {
            *Num_p = iTemp;

        return(SCI_SUCCESS);
        }
    }
    return(SCI_ERROR);
}


void  tft_mdp_gpio_default_cb(U32 gpio_id, U32 status)
{
    // low pulse interrupt
    // disable GPIO INT each low pulse, so modem will not be interruptted when high pulse.
    // enable GPIO INT when sleep delay timer arrived.
    if (status)
    {
        GPIO_SetInterruptSense(gpio_id, GPIO_INT_LEVEL_LOW);
    }
    else
    {
        GPIO_SetInterruptSense(gpio_id, GPIO_INT_LEVEL_HIGH);
    }
}

U32 tft_sys_gpio_cfg_set(U32 gpionum, MDP_GPIO_CFG_S* param, U32 ulParamSize)
{
    MDP_GPIO_CFG_S *cfg = (MDP_GPIO_CFG_S*)param;
    U8 gpio_id = gpionum;
    U8 gpio_type = cfg->gpio_type;
    U8 gpio_lvl = cfg->gpio_lvl;

    GPIO_DisableIntCtl(gpio_id);
    GPIO_DeleteFromIntTable(gpio_id);

    if(gpio_type  == MDP_GPIO_TYPE_OUTPUT)
    {
        if(cfg->gpio_int_ctl.int_enable)
        {
            return SCI_ERROR;
        }
        // Config it can be READ and WRITE.
        GPIO_SetDataMask(gpio_id,SCI_TRUE);
        // Config it to be OUTPUT.
        GPIO_SetDirection(gpio_id,SCI_TRUE);
        // Disable interrupt.
        GPIO_DisableIntCtl(gpio_id);
        // Set init value
        GPIO_SetValue(gpio_id,gpio_lvl);

    }
    else
    {
        // Config it can be READ and WRITE.
        GPIO_SetDataMask(gpio_id,SCI_TRUE);
        // Config it to be INPUT.
        GPIO_SetDirection(gpio_id,SCI_FALSE);

        if(cfg->gpio_int_ctl.int_enable)
        {
            if (GPIO_GetValue (gpio_id))
            {
                GPIO_SetInterruptSense (gpio_id, GPIO_INT_LEVEL_LOW);

            }
            else
            {
                GPIO_SetInterruptSense (gpio_id, GPIO_INT_LEVEL_HIGH);

            }

            GPIO_AddCallbackToIntTable (gpio_id, cfg->gpio_int_ctl.b_shake,
                cfg->gpio_int_ctl.n_shake_time,
                (GPIO_CALLBACK) tft_mdp_gpio_default_cb);

            GPIO_EnableIntCtl(gpio_id);

        }
        else
        {
            GPIO_DisableIntCtl(gpio_id);
        }
    }

    return SCI_SUCCESS;

}




s32 gpioConfig(DEV_HANDLE Handle,gpio_direction_e GPIODirect)
{
    s32 ret;
    U32 num;
    MDP_GPIO_CFG_S devCfg;

    ret = getGpioNum(Handle,&num);

    memset(&devCfg, 0x00, sizeof(MDP_GPIO_CFG_S));
    //GPIOÊôÐÔÅäÖÃÔÝ²»Ö§³Ö
    switch(GPIODirect)
    {
        case GPIO_PIN_INPUT:
            {
                devCfg.gpio_type = MDP_GPIO_TYPE_INPUT;
                devCfg.gpio_lvl = MDP_GPIO_LEVEL_HIGH;
            }
            break;

        case GPIO_PIN_OUTPUT:
            {
                devCfg.gpio_type = MDP_GPIO_TYPE_OUTPUT;
                devCfg.gpio_lvl = MDP_GPIO_LEVEL_HIGH;
            }
            break;

        default:
            {
                ret = SCI_ERROR;
            }
            break;
    }



    //Ö´ÐÐÅäÖÃ
    tft_sys_gpio_cfg_set(num,&devCfg,sizeof(MDP_GPIO_CFG_S));

    return 0;
}


U32 tft_sys_gpio_cfg_get(U32 gpionum, MDP_GPIO_CFG_S* param, U32 ulParamSize)
{
    MDP_GPIO_CFG_S *cfg = (MDP_GPIO_CFG_S*)param;
    U8 gpio_id = gpionum;


    cfg->gpio_type = GPIO_GetDirection(gpio_id);

    //if(MDP_GPIO_TYPE_INPUT == cfg->gpio_type)
    {
        cfg->gpio_lvl = GPIO_GetValue(gpio_id);
    }

    cfg->gpio_int_ctl.int_enable = GPIO_GetIntState(gpio_id);


    return SCI_SUCCESS;
}



S32 tft_sys_gpio_write(U32 gpionum, U8 data, U32 ulDataLen)
{
    U8 gpio_id = gpionum;
    U8 lvl = data;
    MDP_GPIO_CFG_S cfg;

    memset(&cfg, 0x00, sizeof(cfg));
    tft_sys_gpio_cfg_get(gpio_id, &cfg, sizeof(MDP_GPIO_CFG_S));

    if(cfg.gpio_type != MDP_GPIO_TYPE_OUTPUT)
    {
        return SCI_ERROR;
    }

    GPIO_SetValue(gpio_id, lvl);


    return SCI_SUCCESS;

}



s32 gpioWrite(DEV_HANDLE Handle,gpio_level_e GPIOLevel)
{
    s32 ret;
    U32 num;
    U8 value;

    getGpioNum(Handle,&num);

    value = (U8)GPIOLevel;

    tft_sys_gpio_write(num,value,1);

    return 0;

}


void custGetAscii12_12_data(U8 *charCode, U8 *p_result, U16 *p_result_len)
{
  U8 qw=0; //ÇøºÅ£¬Î»ºÅ
  U32 ulOffset;  //Æ«ÒÆ
  U8 low=(U8)(*charCode &0xFF);
  {
        qw =low;
        ulOffset = (U32)(qw)*(GBK_12_12_CHAR_LEN>>1);       //¼ÆËã³öÆ«ÒÆÁ¿
        //Ó¢ÎÄ12 ×Ö½Ú£¬ºó6×Ö½ÚÖ»È¡¸ß4Î»
        memcpy(p_result,&ascii_6_12[ulOffset],GBK_12_12_CHAR_LEN>>1);
        *p_result_len = GBK_12_12_CHAR_LEN>>1;
  }
}


void mergeOneLineCharToHole(U8 hole_xlen, U8 x_now, U8 y_now, flushType_e dispType, U8*p_dest, U8* p_source)
{
  U8 x_offset =0,itemp=0;
  U16 dest_offset = 0;
  U16 dest_next_offset = 0;
  U16 source_offset_next = 0;
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



int update_display(U8 x, U8 y, U8 xlen, U8 ylen, U8* p_data)
{
  U8 y_page_start=(y)>>3, y_offset=(y)&0x07;/*A_bits*/
  U8 y_offset_reserve = 8-y_offset;/*A'*/
  U8 y_refresh_pages=(ylen+y_offset+7)>>3;
  U8 B_bits = (0==((y+ylen)&0x07))?8:((y+ylen)&0x07);/*B_bits*/
  U8 merge_page = (ylen+7)>>3;
  U8 itemp=0,jtemp=0;
  U8 *p_data_buf = p_data;
  U16 updata_data_offset= 0;
  //start merge display buf
  if((x+xlen > MONO_X_PIXELS)||(y+ylen> MONO_Y_PIXELS)||(0==xlen)||(0==ylen))
    return -1;

  //merge source to display buf
  for(itemp =0; itemp < merge_page;itemp++)
  {
    if(itemp == (merge_page-1))//Ô´Êý¾ÝµÄ×îºóÒ»²¿·Ö£¬Õû8bit»ò²¿·Ö
    {
      U8 C_bits= y_offset+((0==(ylen&0x07))?8:(ylen&0x07));
      if(C_bits > 8)//Ô´Êý¾ÝÒªmergeµ½lcd µÄ2¸öpageÖÐ
      {// ÌØ¶¨µÄbit Î»ÐèÒªÍùlcd ÉÏÏÂ2¸öpage½øÐÐmerge
        U16 lcd_offset = ((y+itemp*8)/8)*MONO_X_PIXELS+x;
        U16 lcd_offset_next = lcd_offset+MONO_X_PIXELS;
        U16 source_offset = itemp*xlen;
        U8 page_H_bits = 8-y_offset;
        U8 page_L_bits = ((0==(ylen&0x07))?8:(ylen&0x07))-page_H_bits;

        for(jtemp=0;jtemp<xlen;jtemp++)
        {
          U8 s_data = *(p_data_buf+source_offset+jtemp);
          U8 lcd_buf = p_lcd_pixels_buf[lcd_offset+jtemp];
          U8 lcd_buf_next = p_lcd_pixels_buf[lcd_offset_next+jtemp];
          p_lcd_pixels_buf[lcd_offset+jtemp]=((s_data&pixel_merge_bits[page_H_bits])>>y_offset)|
                                              (lcd_buf&pixel_merge_bits[y_offset]);
          p_lcd_pixels_buf[lcd_offset_next+jtemp]=((s_data<<page_H_bits)&pixel_merge_bits[page_L_bits])|
                                                  (lcd_buf_next&(~pixel_merge_bits[page_L_bits]));
        }
      }
      else//Ô´Êý¾ÝÖ»ÒªÔÚlcd µÄÒ»¸öpage½øÐÐmerge
      {
        U8 distence = B_bits-y_offset;
        U8 merge_mask = (~pixel_merge_bits[distence])>>y_offset;
        U16 lcd_offset = (y_page_start+itemp)*MONO_X_PIXELS+x;
        U16 source_offset = itemp*xlen;
        //SDK_FOTA_DEBUG_STR_2_SPY("last 1 page itemp%d lcd_h%d,source%d,dis%d,offset%d,mask0x%02x",
        //                   itemp,lcd_offset,source_offset,distence,y_offset,merge_mask);
        for(jtemp = 0;jtemp <xlen;jtemp++)
        {
          U8 s_data = *(p_data_buf+source_offset+jtemp);
          U8 lcd_buf =  p_lcd_pixels_buf[lcd_offset+jtemp];
          p_lcd_pixels_buf[lcd_offset+jtemp] = (lcd_buf&merge_mask)|((s_data&pixel_merge_bits[distence])>>y_offset);
        }
      }
    }
    else
    {//ÆäËûÒ³Ãæ, Ã¿8bit¶¼¿ÉÄÜÐèÒªÍùlcdµÄÉÏÏÂpage ½øÐÐmerge
      U16 lcd_offset = ((y+itemp*8)/8)*MONO_X_PIXELS+x;
      U16 lcd_offset_next = lcd_offset+MONO_X_PIXELS;
      U16 source_offset = itemp*xlen;

      for(jtemp=0;jtemp<xlen;jtemp++)
      {
        U8 s_data= *(p_data_buf+source_offset+jtemp);
        U8 lcd_buf = p_lcd_pixels_buf[lcd_offset+jtemp];
        U8 lcd_buf_next = p_lcd_pixels_buf[lcd_offset_next+jtemp];
        p_lcd_pixels_buf[lcd_offset+jtemp]=((s_data&pixel_merge_bits[y_offset_reserve])>>y_offset)|
                                              (lcd_buf&pixel_merge_bits[y_offset]);
        p_lcd_pixels_buf[lcd_offset_next+jtemp]=((s_data&(~pixel_merge_bits[y_offset_reserve]))<<y_offset_reserve)|
                                                  (lcd_buf_next&(~pixel_merge_bits[y_offset]));
      }
    }
  }
  // merge data end ,so call lcd function, updata data to lcd
  updata_data_offset =  y_page_start*MONO_X_PIXELS+x;
  for(itemp = 0;itemp < y_refresh_pages;itemp++)
  {
    U8 page_num = 7-y_page_start-itemp;
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
    set_lcd_data(xlen,&p_lcd_pixels_buf[updata_data_offset+itemp*MONO_X_PIXELS]);
  }

  return(1);
}



void fillAscStr2Hole(holeType_t holeInfo, flushType_e type, U8 *p_STR, U8 agChar_len,U8 reserve)
{
  U8 y_remay = holeInfo.ylen, /*agCharRemay = agChar_len,*/x_now=0,y_now =0,itemp=0,jtemp=0;
  U16 holeSize = ((holeInfo.ylen+7)/8)*holeInfo.xlen;
  U8 *p_hole_buf = NULL;
  U8 *p_one_line_buf = NULL;
  U16 one_line_buf_size = 2*holeInfo.xlen;
  U8 gbPixelBuf[GB_MAX_LEN]={0};
  U16 gbPixelLen = 0;
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
        U8 charPixel = (gbPixelLen/2);
        U16 offset_1 = x_now;
        U16 offset_2 = x_now+holeInfo.xlen;
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
      U8 charPixel = (gbPixelLen/2);
      U16 offset_1 = x_now;
      U16 offset_2 = x_now+holeInfo.xlen;
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
  if(p_hole_buf != PNULL)
  {
    free(p_hole_buf);
    p_hole_buf =NULL;
  }

  if(p_one_line_buf != PNULL)
  {
    free(p_one_line_buf);
    p_one_line_buf = NULL;
  }
}




int display_Screan(U8 *p_data, U16 dataLen)
{
  U8 itemp = 0;
  if(dataLen > DISPLAY_BUF_SIZE)
    return -1;
  memcpy(p_lcd_pixels_buf,p_data,dataLen);
  for(itemp = 0;itemp < MONO_Y_PAGES;itemp++)
  {
    set_lcd_page(itemp);
    set_lcd_col(0);
    set_lcd_data(MONO_X_PIXELS,&p_lcd_pixels_buf[(MONO_Y_PAGES-itemp-1)*MONO_X_PIXELS]);
  }

  return(1);
}



static void Mmi_Idle_Display(void)
{
    holeType_t nullHole = {1,10,126,12};
    holeType_t menuHole = {22,50,24,12};
    holeType_t returnHole = {76,50,36,12};
    U8 nullHint[] = "display test";
    U8 menuHint[] = "MENU";
    U8 returnHint[] = "RETURN";

    //´¦Àí´ý»úÏÔÊ¾
    fillAscStr2Hole(nullHole,FLUSH_MID,nullHint,strlen((S8*)nullHint),0);
    fillAscStr2Hole(menuHole,FLUSH_MID,menuHint,strlen((S8*)menuHint),0);
    fillAscStr2Hole(returnHole,FLUSH_MID,returnHint,strlen((S8*)returnHint),0);
}


void custSpiWriteData(U8 *pData,U16 wLen)
{
    spiWrite(spiRdHandle,pData,wLen);
}

void custLcdRstPinSet(U8 val)
{
    gpioWrite(rstRdHandle,(gpio_level_e)val);
}

void custLcdCmdDataSet(U8 val)
{
    gpioWrite(dcRdHandle,(gpio_level_e)val);
}

void cust_lcd_drv_write(U8 cmd_data,U16 writeLen,U8 *data)
{
    if(SDK_LCD_CMD == cmd_data)
        custLcdCmdDataSet(0);
    else
        custLcdCmdDataSet(1);
    custSpiWriteData(data,writeLen);
}

//  ¿ªÆô¹Ø±ÕÏÔÊ¾
void open_lcd(BOOL_T on_off)
{
  U8 temp = (on_off |LCD_DISPLAY_OFF);
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
  //simu_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

// ÉèÖÃÏÔÊ¾·½ÏòA0,A1
void  set_lcd_adc(U8 direction)
{
  U8 temp = (direction|0xA0);
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//ÉèÖÃÏÔÊ¾µÄÆðÊ¼ÁÐ
void set_lcd_col(U8 colum)
{
  U8 temp = ((colum>>4)|0x10);
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
  temp = colum&0x0F;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//ÉèÖÃÏÔÊ¾ÆðÊ¼ÐÐ
void set_lcd_line(U8 line)
{
  U8 temp = (line|0x40);
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}


//ÉèÖÃÏÔÊ¾Ò³µØÖ·
void  set_lcd_page(U8 pageNum)
{
  U8 temp = (pageNum&0x0F)|0xB0;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//ÉèÖÃÏÔÊ¾Õý·´ÏÔÊ¾A6,A7
void  set_lcd_reverse(U8 reverse)
{
  U8 temp = reverse|0xA6;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//ÉèÖÃÈ«ÆÁÏÔÊ¾(Îª1Ê±ÀàËÆÇåÆÁÏÔÊ¾Ð§¹û)  A4£¬A5
void  set_lcd_all(U8  display)
{
  U8 temp = display|0xA4;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//ÉèÖÃÆ«Ñ¹  0£º1/9     1: 1/7   A2,A3
void set_lcd_bias(U8 bias79)
{
  U8 temp = bias79|0xA2;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//ÉèÖÃ¶Á-¸Ä-Ð´ Ä£Ê½£¬Ã¿´ÎÐ´²Ù×÷,cloum×Ô¼Ó
void set_lcd_R_M_W(void)
{
  U8 temp = 0xE0;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}


//ÉèÖÃ½áÊø¶Á-¸Ä-Ð´Ä£Ê½
void  set_lcd_R_M_W_End(void)
{
  U8 temp = 0xEE;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//ÉèÖÃÈí¼þ¸´Î»£º½«³õÊ¼»¯ÏÔÊ¾ÆðÊ¼ÐÐ¡¢ÆðÊ¼ÁÐµØÖ·¡¢ÆðÊ¼Ò³µØÖ·¡¢Õý³£Êä³öÄ£Ê½
void  set_lcd_soft_reset(void)
{
  U8 temp = 0xE2;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//ÉèÖÃÐÐÉ¨Ãè·½Ïò0:COM0-COM63    1:COM63-COM0
void  set_lcd_com(U8 comNum)
{
   U8 temp = ((comNum<<3)&0x0F)|0xC0;
   cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//ÉèÖÃµçÔ´¿ØÖÆµÄCircuit operation mode  28-2F
void set_lcd_power_circuit(U8 mode)
{
  U8 temp = (mode&0x07)|0x28;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//ÉèÖÃÄÚ²¿µç×èµ÷ÕûÉèÖÃ20-27
void  set_lcd_resistor(U8 resistor)
{
  U8 temp = (resistor&0x07)|0x20;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//ÉèÖÃ¶Ô±È¶È£¬Í¬ÁÐÏÔÊ¾£¬Ë«×Ö²Ù×÷0-63
void  set_lcd_contrast(U8 contrast)
{
  U8 temp = 0x81;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
  temp = contrast&0x3F;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}


//ÉèÖÃ¾²Ì¬ÏÔÊ¾£¬Í¬ÉÏÎªË«×Ö²Ù×÷0£ºoff  1£ºon
void  set_lcd_static(U8 on_off, U8 flashMode)
{
  U8 temp = (on_off&0x01)|0xAC;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
  temp = flashMode&0x03;
  cust_lcd_drv_write(SDK_LCD_CMD,1,&temp);
}

//´«ÊäLCD Êý¾Ý
void set_lcd_data(U16 writeLen,U8 *data)
{
   cust_lcd_drv_write(SDK_LCD_DATA,writeLen,data);
}

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    }
#endif




