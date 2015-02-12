/******************************************************************************
 ** File Name:      Gps_SiRF.c                                                *
 ** Author:         David.Jia                                                 *
 ** DATE:           7/31/2007                                                 *
 ** Copyright:      2005 Spreatrum, Incoporated. All Rights Reserved.         *
 ** Description:    SiRF GPS interface.                                       *
 *****************************************************************************/
/******************************************************************************
 **                   Edit    History                                         *
 **---------------------------------------------------------------------------* 
 ** DATE            NAME            DESCRIPTION                               * 
 ** 07/31/2007      David.Jia       SiRF GPS interface base on Gps_drv.       *
 ** 08/02/2007      David.Jia       Change names of COM_Init/COM_Close/Map_Read/Map_Write 
 **     to GPS_ComInit/GPS_ComClose/GPS_ComRead/GPS_ComWrite.
 ** 08/14/2007      David.Jia       Add gpio control.                         *
 ** 09/04/2007      David.Jia       Modify Srf_Open for download firmware.    *
 *****************************************************************************/
/*----------------------------------------------------------------------------*
**                        Dependencies                                        *
**---------------------------------------------------------------------------*/
#include "ms_customize_trc.h"
#include "gps_drv.h"
#include "Srf_func.h"
#include "nv_productionparam_type.h"
#include "sci_types.h"
#include "GPS_COM.h"
#include "gpio_prod_api.h"           //@David.Jia 2007.8.14
#include "gps_nmea.h"

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    extern   "C" 
    {
#endif


#define SIRF_DEBUG

#ifdef  SIRF_DEBUG
#define GPS_LOG     SCI_TraceLow
#endif

#ifdef GPS_SUPPORT

#define     GPIO_GPS_ON                 17      //GPS On/Off
#define     GPIO_GPS_BOOT_INTER         16      //GPS boot from Internal(Reflash)/External
#define     GPIO_GPS_RESET              48      //GPS Reset
#define     GPIO_GPS_POWER              13      //GPS Power On/Off

#ifdef _SP7560_P1_
 #define     GPIO_GPS_WAKEUP             4       //@David.Jia 2007.9.25 GPS Wakeup/Sleep
#else
 #define     GPIO_GPS_WAKEUP                   // no wakeup pin @P2 7560
#endif

/*****************************************************************************/
//  Description:    Control GPS's power
//                  is_open = SCI_TRUE, 	power on
//                  is_open = SCI_FALSE, 	power off
//  Author:         David.Jia
//  Note:           
/*****************************************************************************/
LOCAL void GPIO_GPS_PowerOn(BOOLEAN is_open)
{
    //SCI_TraceLow:"GPIO_GPS_PowerOn: %d"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_SIRF_60_112_2_18_0_33_21_1687,(uint8*)"d", is_open);

    GPIO_SetValue( GPIO_GPS_POWER, is_open);

}

/*****************************************************************************/
//  Description:    Control GPS On/Off
//                  is_open = SCI_TRUE, 	on
//                  is_open = SCI_FALSE, 	off
//  Author:         David.Jia
//  Note:           
/*****************************************************************************/
LOCAL void GPIO_GPS_On(BOOLEAN is_open)
{
    //SCI_TraceLow:"GPIO_GPS_On: %d"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_SIRF_73_112_2_18_0_33_21_1688,(uint8*)"d", is_open);

    GPIO_SetValue( GPIO_GPS_ON, is_open);

}

/*****************************************************************************/
//  Description:    Control GPS boot from internal(reflash)/external
//                  is_open = SCI_TRUE, 	internal (reflash)
//                  is_open = SCI_FALSE, 	external (normal)
//  Author:         David.Jia
//  Note:           
/*****************************************************************************/
LOCAL void GPIO_GPS_BootInter(BOOLEAN is_open)
{
    //SCI_TraceLow:"GPIO_GPS_BootInter: %d"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_SIRF_86_112_2_18_0_33_21_1689,(uint8*)"d", is_open);

    GPIO_SetValue( GPIO_GPS_BOOT_INTER, is_open);

}

/*****************************************************************************/
//  Description:    Control GPS reset
//                  is_open = SCI_TRUE, 	reset high
//                  is_open = SCI_FALSE, 	reset low
//  Author:         David.Jia
//  Note:           
/*****************************************************************************/
LOCAL void GPIO_GPS_Reset(BOOLEAN is_open)
{
    //SCI_TraceLow:"GPIO_GPS_ResetLow: %d"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_SIRF_99_112_2_18_0_33_21_1690,(uint8*)"d", is_open);

    GPIO_SetValue( GPIO_GPS_RESET, !is_open);       //low to reset

}

/*****************************************************************************/
//  Description:    Control GPS wakeup/sleep
//                  is_open = SCI_TRUE, 	sleep
//                  is_open = SCI_FALSE, 	wakeup
//  Author:         David.Jia
//  Note:           
/*****************************************************************************/
LOCAL void GPIO_GPS_Wakeup(BOOLEAN is_open)
{
    //SCI_TraceLow:"GPIO_GPS_Wakeup: %d"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_SIRF_112_112_2_18_0_33_21_1691,(uint8*)"d", is_open);
   
      #ifdef _SP7560_P1_
       GPIO_SetValue( GPIO_GPS_WAKEUP, !is_open);      //low to wakeup
      #endif 
}

/*****************************************************************************/
//  Description:    config uart1 to GPIO or GPIO to uart1
//                  is_gpio = SCI_TRUE, 	uart1 to GPIO
//                  is_gpio = SCI_FALSE, 	GPIO to uart1
//  Author:         Sea.Hou
//  Note:           
/*****************************************************************************/

LOCAL void GPIO_GPS_UART2GPIO(BOOLEAN is_gpio)
{
	//SCI_TraceLow:"GPIO_GPS_UART2GPIO: %d"
	SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_SIRF_127_112_2_18_0_33_22_1692,(uint8*)"d", is_gpio);
	if(is_gpio)
	 {
	*(volatile uint32 *)(0x8C000010)|=(BIT_3|BIT_2|BIT_1|BIT_0);  //GPIO
    *(volatile uint32 *)(0x8A000088)&=~(BIT_4|BIT_5);   //IN
    *(volatile uint32 *)(0x8C0000D0)&=~(BIT_8|BIT_9);
    *(volatile uint32 *)(0x8C00010C)|=(BIT_8|BIT_9);   //pull down

	 }
	else
	 {
	*(volatile uint32 *)(0x8C000010) &=~(BIT_3|BIT_2|BIT_1|BIT_0);
	*(volatile uint32 *)(0x8C0000D0)|=BIT_9;	
	 	
	 } 
}


//@David.Jia 2007.8.13  end



/*****************************************************************************/
//  FUNCTION:       Srf_Init
//  Description:    hardware initialization, called on cell power on.
//  return:         
//  INPUT:          
//  Author:         David.Jia
//  date:           2007.7.31
//	Note:           u1rxd/u1txd, power_control/power down/reset/boot_from pin select.
/*****************************************************************************/
LOCAL GPS_ERR_E Srf_Init(void)
{
    //GPS_LOG:"Srf_Init"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_SIRF_158_112_2_18_0_33_22_1693,(uint8*)"");
   
    //GPS_LOG:"config Uart1 to GPIO and pull them down for saving power"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_SIRF_160_112_2_18_0_33_22_1694,(uint8*)"");
    GPIO_GPS_UART2GPIO(SCI_TRUE);
   
    return GPS_ERR_NONE;
}

/*****************************************************************************/
//  FUNCTION:       Srf_Open
//  Description:    called before GPS work.
//  return:         
//  INPUT:          
//  Author:         David.Jia
//  date:           2007.7.31
//	Note:           com initialize, whether download firmware, power_control on, 
//      sleep off, reset.
/*****************************************************************************/
LOCAL GPS_ERR_E Srf_Open(GPS_MODE_E mode)
{
    uint16 IsSrfDownload;   //BIT_0 == 1: has downloaded
    
    //GPS_LOG:"Srf_Open: mode=%d"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_SIRF_178_112_2_18_0_33_22_1695,(uint8*)"d", mode);

  	GPIO_GPS_UART2GPIO(SCI_FALSE); // GPIO-->uart1

    //serial com init    
    GPS_ComInit();
     
    //download firmware
	IsSrfDownload = PROD_GetPeripheralNVParam( PROD_NV_ID_GPS );
    if (!(IsSrfDownload && 0x1))   //download firmware
    {
        extern int GpsDownloadSrf(uint32);
        
        //GPS_LOG:"Srf_Open: reflash!"
        SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_SIRF_191_112_2_18_0_33_22_1696,(uint8*)"");
        
        GPIO_GPS_BootInter(SCI_TRUE);       //boot from internal, reflash
        
        //@David.Jia 2007.9.4   begin
        GPIO_GPS_PowerOn(SCI_TRUE);             //power on
        GPIO_GPS_On(SCI_TRUE);                  //gps on
        OS_TickDelay(10);
        
        //reset gps
        GPIO_GPS_Reset(SCI_FALSE);
        OS_TickDelay(10);
        GPIO_GPS_Reset(SCI_TRUE);
        OS_TickDelay(10);
        GPIO_GPS_Reset(SCI_FALSE);
        //@David.Jia 2007.9.4   end
        
        GPS_ComClose();     //GpsDownloadSrf will call GPS_ComInit and GPS_ComClose.
        if (0 == GpsDownloadSrf(921600))      //download ok    
        {
            PROD_SetPeripheralNVParam( PROD_NV_ID_GPS , IsSrfDownload | 0x1);
        }
        GPS_ComInit();
        
        //after reflash, must power off and boot from external again
        GPIO_GPS_PowerOn(SCI_FALSE);
        OS_TickDelay(10);
        GPIO_GPS_BootInter(SCI_FALSE);      //boot from external, normal
    }
    
    
    GPIO_GPS_PowerOn(SCI_TRUE);             //power on
    GPIO_GPS_On(SCI_TRUE);                  //gps on
    OS_TickDelay(10);
    
    //reset gps
    GPIO_GPS_Reset(SCI_FALSE);
    OS_TickDelay(10);
    GPIO_GPS_Reset(SCI_TRUE);
    OS_TickDelay(10);
    GPIO_GPS_Reset(SCI_FALSE);
    
    return GPS_ERR_NONE;
}

/*****************************************************************************/
//  FUNCTION:       Srf_Close
//  Description:    called after GPS work.
//  return:         
//  INPUT:          
//  Author:         David.Jia
//  date:           2007.7.31
//	Note:           serial com close, power_control 
/*****************************************************************************/
LOCAL GPS_ERR_E Srf_Close(void)
{
    //GPS_LOG:"Srf_Close"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_SIRF_245_112_2_18_0_33_22_1697,(uint8*)"");
    //serial com close
    GPS_ComClose();
  

    //reset gps
    GPIO_GPS_Reset(SCI_FALSE);
    SCI_Sleep(10);
    GPIO_GPS_Reset(SCI_TRUE);
    SCI_Sleep(10);
    GPIO_GPS_Reset(SCI_FALSE);
    
    //power off   
    GPIO_GPS_On(SCI_FALSE);       //GPS_ON_OFF 0
    GPIO_GPS_BootInter(SCI_FALSE);    //GPS_BOOT_INTER 0
    GPIO_GPS_Reset(SCI_FALSE);   // GPS_RESET 1
        
    GPIO_GPS_UART2GPIO(SCI_TRUE);  //uart1-->GPIO and PULL all Down.

    GPIO_GPS_PowerOn(SCI_FALSE);  //
    //SCI_TRACE_LOW:"[GPIO_GPS_PowerOn] OFF"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_SIRF_265_112_2_18_0_33_22_1698,(uint8*)"");

    return GPS_ERR_NONE;
}

/*****************************************************************************/
//  FUNCTION:       Srf_Sleep
//  Description:    switch power saving mode
//  return:         
//  INPUT:          
//  Author:         David.Jia
//  date:           2007.7.31
//	Note:           
/*****************************************************************************/
LOCAL GPS_ERR_E Srf_Sleep(BOOLEAN is_sleep)
{
    //GPS_LOG:"Srf_Sleep: is_sleep=%d"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_SIRF_279_112_2_18_0_33_22_1699,(uint8*)"d", is_sleep);
    
    GPIO_GPS_Wakeup(!is_sleep);
    
    return GPS_ERR_NONE;
}

/*****************************************************************************/
//  FUNCTION:       Srf_Reflash
//  Description:    clear GPS infomation in the memory, restart location process.
//  return:         
//  INPUT:          
//  Author:         David.Jia
//  date:           2007.7.31
//	Note:           ColdStart
/*****************************************************************************/
GPS_ERR_E	Srf_Reflash(void)
{
    //GPS_LOG:"Srf_Reflash"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_SIRF_295_112_2_18_0_33_22_1700,(uint8*)"");
    
    //call GpsGetTTFFSrf
    GpsGetTTFFSrf(100*1000);    
    
    return GPS_ERR_NONE;
}

/*****************************************************************************/
//  FUNCTION:       Srf_Read
//  Description:    call Map_Read
//  return:         
//  INPUT:          
//  Author:         David.Jia
//  date:           2007.7.31
//	Note:           
/*****************************************************************************/
uint32 Srf_ReadData(uint8* read_buf_ptr, uint32 byte_to_read)
{
	uint32 ret_val = 0;
    //GPS_LOG:"Srf_ReadData: buf=%x, len=%d"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_SIRF_313_112_2_18_0_33_22_1701,(uint8*)"dd", (uint32)read_buf_ptr, byte_to_read);
    ret_val = (uint32)GPS_ComRead(read_buf_ptr, byte_to_read);

	return ret_val;
}

/*****************************************************************************/
//  FUNCTION:       Srf_WriteData
//  Description:    call Map_Write
//  return:         
//  INPUT:          
//  Author:         David.Jia
//  date:           2007.7.31
//	Note:           
/*****************************************************************************/
uint32 Srf_WriteData(uint8* read_buf_ptr, uint32 byte_to_read)
{
    //GPS_LOG:"Srf_WriteData: buf=%x, len=%d"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_SIRF_328_112_2_18_0_33_22_1702,(uint8*)"dd", (uint32)read_buf_ptr, byte_to_read);
    return (uint32)GPS_ComWrite(read_buf_ptr, byte_to_read);
}


/*****************************************************************************/
//  FUNCTION:       Srf_Identify(void)
//  Description:    true for SiRF GPS
//  return:         
//  INPUT:          
//  Author:         David.Jia
//  date:           2007.7.31
//	Note:           
/*****************************************************************************/
BOOLEAN Srf_Identify(void)
{
    //GPS_LOG:"Srf_Identify"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_SIRF_342_112_2_18_0_33_22_1703,(uint8*)"");
    return SCI_TRUE;
}

/*****************************************************************************/
//  FUNCTION:       Srf_Test(void)
//  Description:    TTFF test for SiRF GPS
//  return:         
//  INPUT:          
//  Author:         David.Jia
//  date:           2007.7.31
//	Note:           
/*****************************************************************************/
uint32 Srf_Test(void* ptr, uint32 param)
{
    //GPS_LOG:"Srf_Test"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_SIRF_355_112_2_18_0_33_22_1704,(uint8*)"");
    return GpsGetTTFFSrf(param);
}

/**************************************************************************************/
// SiRF GPS interface
/**************************************************************************************/
PUBLIC GPS_OPERATIONS_T g_sirf_gps_operation = 
{
    Srf_Init,
    Srf_Open,
    Srf_Close,
    Srf_Sleep,
    Srf_Reflash,
    Srf_ReadData,           
    Srf_WriteData,
    Srf_Identify,
    Srf_Test    
};

#endif  //_GPS
/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    }
#endif

// End of Gps_SIRF.c
