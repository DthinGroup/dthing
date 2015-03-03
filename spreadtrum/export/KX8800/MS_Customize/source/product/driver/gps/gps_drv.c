/******************************************************************************
 ** File Name:      gps_api.c                                                 *
 ** Author:         Liangwen.Zhen                                             *
 ** DATE:           07/26/2007                                                *
 ** Copyright:      2007 Spreadtrum, Incoporated. All Rights Reserved.        *
 ** Description:    This file defines the basic operation interfaces of GPS   *
 **                                                                           *
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                       *
 ** ------------------------------------------------------------------------- *
 ** DATE           NAME             DESCRIPTION                               *
 ** 07/26/2007     Liangwen.Zhen/David.Jia    Create.                         *
 ******************************************************************************/

/**---------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **---------------------------------------------------------------------------*/ 


/**---------------------------------------------------------------------------*
 **                         Debugging Flag                                    *
 **---------------------------------------------------------------------------*/

#include "ms_customize_trc.h"
#include "gps_drv.h"
#include "gps_cfg.h"
#include "os_api.h"
#include "nv_item_id.h"
#include "layer1_engineering.h"
#include "gps_nmea.h"

#define DEBUG_GPS_DRV
#ifdef  DEBUG_GPS_DRV
	#define GPS_TRACE   SCI_TRACE_LOW 
#else
	#define GPS_TRACE( _format_string )  
#endif

#define GPS_COM 1 //UART_COM1

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    extern   "C" 
    {
#endif
/**---------------------------------------------------------------------------*
 **                         Macro Definition                                   *
 **---------------------------------------------------------------------------*/
#define GPS_SLEEP_TIME_MAX	(2*60*60*1000)	//ms, 在GPS模块处于sleep状态超过这个时间不唤醒，GPS就会丢失定位信息，进入LOST状态
#define	GPS_IDLE_TIME_MAX	(20 * 1000)		//ms, 在GPS模块处于idle状态超过这个时间不使用，就让GPS进入SLEEP状态
/**---------------------------------------------------------------------------*
 **                         Global Variables                                  *
 **---------------------------------------------------------------------------*/
#ifndef _SP7560_P1_
	#define _GPS_CIPHER
#endif
/**---------------------------------------------------------------------------*
 **                         Local Variables                                   *
 **---------------------------------------------------------------------------*/
LOCAL GPS_STATUS_E 		s_gps_status 			= GPS_STATUS_NOT_INIT;
LOCAL GPS_OPERATIONS_T*	s_gps_operations_ptr	= PNULL;
LOCAL uint32			s_gps_idle_time_count	= 0;
LOCAL uint32			s_gps_sleep_time_count  = 0;
LOCAL GPS_NV_INFO_T		s_gps_nv_info			= {0};
#ifdef _GPS_CIPHER
LOCAL GPS_CIPHER_REPORT_T s_gps_cipher			= {0};
#endif
LOCAL BOOLEAN 			s_is_get_gps_cipher		= SCI_FALSE;
LOCAL GPS_MODE_E		s_gps_mode				= GPS_MODE_MAX;
//LOCAL GPS_INFO_T		s_gps_info				= {0};

/**---------------------------------------------------------------------------*
 **                         Constant Variables                                *
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 **                     Local Function Prototypes                             *
 **---------------------------------------------------------------------------*/
/*****************************************************************************/
//  Description:    This function is used to set GPS module idle time    
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
LOCAL void _GPS_SetIdleTime(void)
{
	s_gps_idle_time_count = SCI_GetTickCount();
}

/*****************************************************************************/
//  Description:    This function is used to get GPS module idle time   
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
LOCAL uint32 _GPS_GetIdleTime(void)
{
	return s_gps_idle_time_count;
}

/*****************************************************************************/
//  Description:    This function is used to set GPS module sleep time    
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
LOCAL void _GPS_SetSleepTime(void)
{
	s_gps_sleep_time_count = SCI_GetTickCount();
}

/*****************************************************************************/
//  Description:    This function is used to get GPS module sleep time    
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
LOCAL uint32 _GPS_GetSleepTime(void)
{
	return s_gps_sleep_time_count;
}

/*****************************************************************************/
//  Description:    This function is used to set GPS module status    
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
LOCAL void _GPS_SetStatus(GPS_STATUS_E status)
{	
	s_gps_status = status;	
	if(GPS_STATUS_OPEN == status)
	{
		_GPS_SetIdleTime();
	}
	else if(GPS_STATUS_SLEEP == status)
	{
		_GPS_SetSleepTime();
	}	
	
}

/*****************************************************************************/
//  Description:    This function is used to manage GPS module power    
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
LOCAL void _GPS_PowerManager(void)
{
	uint32 time = 0;
	uint32 cur_time = SCI_GetTickCount();
	
	if(GPS_STATUS_OPEN == GPS_GetStatus())
	{
		time =  _GPS_GetIdleTime();
		if(cur_time > time)
		{
			time = cur_time - time;
		}
		else
		{
			time = 0xFFFFFFFF - time + cur_time;	
		}

		if(time >= GPS_IDLE_TIME_MAX)
		{
			GPS_EnterSleep(SCI_TRUE);	
		}
	}
	else if(GPS_STATUS_SLEEP == GPS_GetStatus())
	{
		time = _GPS_GetSleepTime();
		if(cur_time > time)
		{
			time = cur_time - time;
		}
		else
		{
			time = 0xFFFFFFFF - time + cur_time;	
		}

		if(time >= GPS_SLEEP_TIME_MAX)
		{
			_GPS_SetStatus(GPS_STATUS_LOST);
		}
	}
}

#ifdef _GPS_CIPHER
/*****************************************************************************/
//  Description:    This function is used to get GPS status    
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
LOCAL void _GPS_ValidationCallback(GPS_CIPHER_REPORT_T GPS_cipher)
{
	//GPS_TRACE:"_GPS_ValidationCallback: high_cipher 0x%08x, low_cipher 0x%08x"
	SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_DRV_171_112_2_18_0_33_7_1554,(uint8*)"dd", GPS_cipher.high_level, GPS_cipher.low_level);
	SCI_MEMCPY(&s_gps_cipher, &GPS_cipher, sizeof(GPS_CIPHER_REPORT_T));

	s_is_get_gps_cipher = SCI_TRUE;
}
#endif

/*****************************************************************************/
//  Description:    This function is used to get GPS mode    
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
LOCAL GPS_MODE_E _GPS_GetMode(void)
{
	return s_gps_mode;
}

/*****************************************************************************/
//  Description:    This function is used to get GPS mode    
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
LOCAL void _GPS_SetMode(GPS_MODE_E mode)
{
	s_gps_mode = mode;
	//GPS_TRACE:"_GPS_SetMode: s_gps_mode %d, mode %d"
	SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_DRV_192_112_2_18_0_33_7_1555,(uint8*)"dd", s_gps_mode, mode);
}
/**---------------------------------------------------------------------------*
 **                     Global Function Prototypes                            *
 **---------------------------------------------------------------------------*/
/*****************************************************************************/
//  Description:    This function is used to get GPS status    
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
PUBLIC GPS_STATUS_E GPS_GetStatus(void)
{
	return s_gps_status;
}
/*****************************************************************************/
//  Description:    This function is used to init GPS mode and global variable    
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
PUBLIC GPS_ERR_E GPS_Init(void)
{
	GPS_ERR_E err_val = GPS_ERR_NO_MODULE;
	
	if(PNULL != s_gps_operations_ptr)
	{		
		// must Identify 
		if(PNULL != s_gps_operations_ptr->gps_identify)
		{
			if(s_gps_operations_ptr->gps_identify())
			{
				err_val = GPS_ERR_NONE;						
			}
		}
		else
		{
			SCI_PASSERT(0, ("NO GPS identify!"));   /*assert verified*/
		}		
		
	}
	else
	{	
		// Second: to find the right operation in the table
		s_gps_operations_ptr = *(GPS_OPERATIONS_T**)GPS_GetOperationTab();
		
		while(PNULL != s_gps_operations_ptr)
		{		
			// must Identify 
			if(PNULL != s_gps_operations_ptr->gps_identify)
			{
				if(s_gps_operations_ptr->gps_identify())
				{					
					err_val = GPS_ERR_NONE;
					break;			
				}
			}
			else
			{
				SCI_PASSERT(0, ("NO GPS identify!"));   /*assert verified*/
			}		
			
			// Find next operation
			s_gps_operations_ptr++;
		}	
	}
	
	
	if(GPS_ERR_NONE != err_val)
	{
	    //GPS_TRACE:"GPS_Init: Identify fail !! status %d"
	    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_DRV_255_112_2_18_0_33_7_1556,(uint8*)"d", err_val);
		
	}
	else
	{
		_GPS_SetStatus(GPS_STATUS_INIT);		
		//GPS_TRACE:"GPS_Init: Identify successful !!"
		SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_DRV_261_112_2_18_0_33_7_1557,(uint8*)"");
		
		//call init
		if(PNULL != s_gps_operations_ptr->gps_init)
		{
			s_gps_operations_ptr->gps_init();		
		}
	}	
	
	GPS_DIAG_RegDiagCmdRoutine();
	
	return err_val;
}

/*****************************************************************************/
//  Description:    This function is used to open GPS module by the special mode    
//  Author:         Liangwen.Zhen
//  Note:           
//	input:			GPS_MODE_E
//	Output:			GPS_ERR_E
/*****************************************************************************/
PUBLIC GPS_ERR_E GPS_Open(GPS_MODE_E mode)
{
	GPS_ERR_E err_val = GPS_ERR_NONE;	
	
	if(PNULL != s_gps_operations_ptr)
	{
		SCI_ASSERT(PNULL != s_gps_operations_ptr->gps_open);/*assert verified*/	
		err_val = s_gps_operations_ptr->gps_open(mode);
		if(GPS_ERR_NONE == err_val)
		{
			_GPS_SetStatus(GPS_STATUS_OPEN);	
			//DoIdle_RegisterCallback(_GPS_PowerManager);
			if( GPS_MODE_TEST_NMEA == mode )
			{				
				NMEA_Init();
			}

			_GPS_SetMode(mode);
		}
	}
	else
	{
		err_val = GPS_ERR_NO_MODULE;
	}
	UART_Rx_Int_Enable(GPS_COM,TRUE);
	
	return err_val;
}

/*****************************************************************************/
//  Description:    This function is used to Close GPS module    
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
PUBLIC GPS_ERR_E GPS_Close(void)
{
	GPS_ERR_E err_val = GPS_ERR_NONE;
	
	if(PNULL != s_gps_operations_ptr)
	{
		SCI_ASSERT(PNULL != s_gps_operations_ptr->gps_close);/*assert verified*/
		err_val = s_gps_operations_ptr->gps_close();
		if(GPS_ERR_NONE == err_val)
		{
			_GPS_SetStatus(GPS_STATUS_CLOSE);
		}

		if( GPS_MODE_TEST_NMEA == _GPS_GetMode() )
		{				
			NMEA_Close();
		}

		_GPS_SetMode( GPS_MODE_MAX );
	}
	else
	{
		err_val = GPS_ERR_NO_MODULE;
	}
	
	return err_val;
}

/*****************************************************************************/
//  Description:    This function is used to Close GPS module    
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
PUBLIC GPS_ERR_E GPS_EnterSleep(BOOLEAN is_sleep)
{
	GPS_ERR_E err_val = GPS_ERR_NONE;
	
	if(PNULL != s_gps_operations_ptr)
	{
		if((!is_sleep && (GPS_STATUS_SLEEP == GPS_GetStatus()))
		 || (!is_sleep && (GPS_STATUS_LOST == GPS_GetStatus()))
		 || (is_sleep && (GPS_STATUS_OPEN == GPS_GetStatus()))
		 )
		{
			SCI_ASSERT(PNULL != s_gps_operations_ptr->gps_sleep);/*assert verified*/
			err_val = s_gps_operations_ptr->gps_sleep(is_sleep);
			if(GPS_ERR_NONE == err_val)
			{
				if(is_sleep)
				{					
					_GPS_SetStatus(GPS_STATUS_SLEEP);					
				}
				else
				{
					if(GPS_STATUS_LOST == GPS_GetStatus())
					{
						SCI_ASSERT(PNULL != s_gps_operations_ptr->gps_reflash);/*assert verified*/
						s_gps_operations_ptr->gps_reflash();
					}					
					_GPS_SetStatus(GPS_STATUS_OPEN);
				}
			}
		}
	}
	
	return err_val;
	
}

/*****************************************************************************/
//  Description:    This function is used to read data from GPS module    
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
PUBLIC uint32 GPS_ReadData(uint8 * read_buf_ptr, uint32 byte_to_read)
{
	uint32 data_size = 0;
	
	if((GPS_STATUS_SLEEP == GPS_GetStatus()) || (GPS_STATUS_LOST == GPS_GetStatus()))
	{
		GPS_EnterSleep(SCI_FALSE);
	}
	
	if(GPS_STATUS_OPEN == GPS_GetStatus())
	{
		SCI_ASSERT(PNULL != s_gps_operations_ptr);/*assert verified*/
		SCI_ASSERT(PNULL != s_gps_operations_ptr->gps_readdata);/*assert verified*/
		_GPS_SetIdleTime();

		data_size = s_gps_operations_ptr->gps_readdata(read_buf_ptr, byte_to_read);
		if(GPS_MODE_TEST_NMEA == _GPS_GetMode())
		{
			// Snatch NMEA frame from data stream
			NMEA_SnatchFramesFromStream((uint8*)read_buf_ptr, data_size);
			// Flush the frames
			while(NMEA_ERR_NONE == NMEA_GetFrame( PNULL, NULL, NULL));
		}
		
		return  data_size;
	}
	
}

/*****************************************************************************/
//  Description:    This function is used to write data to GPS module    
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
PUBLIC uint32 GPS_WriteData(uint8 * write_buf_ptr, uint32 byte_to_write)
{
	if((GPS_STATUS_SLEEP == GPS_GetStatus()) || (GPS_STATUS_LOST == GPS_GetStatus()))
	{
		GPS_EnterSleep(SCI_FALSE);
	}
	
	if(GPS_STATUS_OPEN == GPS_GetStatus())
	{
		SCI_ASSERT(PNULL != s_gps_operations_ptr);/*assert verified*/
		SCI_ASSERT(PNULL != s_gps_operations_ptr->gps_writedata);/*assert verified*/
		_GPS_SetIdleTime();
		return  s_gps_operations_ptr->gps_writedata(write_buf_ptr, byte_to_write);
	}
}

/*****************************************************************************/
//  Description:    This function is used to test GPS module
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
PUBLIC uint32 GPS_Test(void* ptr, uint32 param)
{
	if((GPS_STATUS_SLEEP == GPS_GetStatus()) || (GPS_STATUS_LOST == GPS_GetStatus()))
	{
		GPS_EnterSleep(SCI_FALSE);
	}
	
	if(GPS_STATUS_OPEN == GPS_GetStatus())
	{
		SCI_ASSERT(PNULL != s_gps_operations_ptr);/*assert verified*/
		SCI_ASSERT(PNULL != s_gps_operations_ptr->gps_test);/*assert verified*/
		_GPS_SetIdleTime();
		return  s_gps_operations_ptr->gps_test(ptr, param);
	}
}

/*****************************************************************************/
//  Description:    This function is used to test GPS module
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
PUBLIC GPS_INFO_T_PTR GPS_GetGpsInfo (void)
{
	if(GPS_STATUS_OPEN == GPS_GetStatus())
	{
		return (GPS_INFO_T_PTR)NMEA_GetGpsInfo();
	}
	else
	{
		return PNULL;
	}
}

/*****************************************************************************/
//  Description:    This function is used to get information about GPS engine and MAP    
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
PUBLIC GPS_NV_INFO_T* GPS_GetNVInfo(void)
{
	 uint32  status = 0;
    
    // Read REF parameter initial values from NVItem.
    status = EFS_NvitemRead(NV_GPS_PARAM, sizeof(s_gps_nv_info), (uint8 *)(&s_gps_nv_info));    
    // If the initial values don't exist.
    if (/*NVERR_NOT_EXIST*/4 == status)
    {
#ifdef WIN32
        EFS_NvitemWrite(NV_GPS_PARAM, sizeof(s_gps_nv_info), (uint8 *)(&s_gps_nv_info), 1);
#else
        if (!NVITEM_IsSimulate())
        {
    	    SCI_PASSERT(0, ("Please download new NVItem !"));   /*assert verified*/
        }
#endif  // WIN32        
    }
    
    return &s_gps_nv_info;

}

/*****************************************************************************/
//  Description:    This function is used to write information about GPS engine and MAP to nv
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
PUBLIC uint32 GPS_WriteNVInfo(GPS_NV_INFO_T* nv_info_ptr)
{
	SCI_MEMCPY(&s_gps_nv_info, nv_info_ptr, sizeof(GPS_NV_INFO_T));
	EFS_NvitemWrite(NV_GPS_PARAM, sizeof(GPS_NV_INFO_T), (uint8 *)(&s_gps_nv_info), 1);
	 
	 return SCI_SUCCESS;
}

/*****************************************************************************/
//  Description:    This function is used to get validation cipher from DSP
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
PUBLIC uint32 GPS_GetValidationCipher(
		uint32						input_serial, 
		GPS_VALIDATION_CIPHER_T * 	output_validation_cipher_ptr)
{
	uint32 count = 0;
#ifdef _GPS_CIPHER	
	//GPS_TRACE:"GPS_GetValidationCipher: input_serial 0x%08x, start time %d"
	SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_DRV_509_112_2_18_0_33_8_1558,(uint8*)"dd", input_serial, SCI_GetTickCount());
	
	s_is_get_gps_cipher = SCI_FALSE;
	
	L1API_StartGPSValidation(input_serial, _GPS_ValidationCallback);
	
	while(!(s_is_get_gps_cipher || (count > 200)))
	{
		SCI_Sleep(10);
		count++;
	}
	
	if(count < 200)
	{
		output_validation_cipher_ptr->high_cipher = s_gps_cipher.high_level;
		output_validation_cipher_ptr->low_cipher  = s_gps_cipher.low_level;
		//GPS_TRACE:"GPS_GetValidationCipher OK  end time %d!"
		SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_DRV_525_112_2_18_0_33_8_1559,(uint8*)"d", SCI_GetTickCount());
		return SCI_SUCCESS;
	}
	else
	{
		output_validation_cipher_ptr->high_cipher = 0;
		output_validation_cipher_ptr->low_cipher  = 0;
		//GPS_TRACE:"GPS_GetValidationCipher timer out, end time %d!"
		SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_DRV_532_112_2_18_0_33_8_1560,(uint8*)"d", SCI_GetTickCount());
		
		return SCI_ERROR;
	}
#else
	//extern uint16 g_cipherBuffer[];
	
	output_validation_cipher_ptr->high_cipher = 0x1122;
	output_validation_cipher_ptr->low_cipher  = 0x33445566;
	//g_cipherBuffer[0] = 0;
	//g_cipherBuffer[1] = 0x5566;
	//g_cipherBuffer[2] = 0x3344;
	//g_cipherBuffer[3] = 0x1122;
	
	return SCI_SUCCESS;
#endif		
	
}

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    }
    
#endif  // End of gsp_drv.c