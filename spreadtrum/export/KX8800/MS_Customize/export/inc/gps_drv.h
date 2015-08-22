/******************************************************************************
 ** File Name:      gps_drv.h                                                 *
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
#ifndef _GPS_DRV_H_
#define _GPS_DRV_H_
/**---------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **---------------------------------------------------------------------------*/ 


/**---------------------------------------------------------------------------*
 **                         Debugging Flag                                    *
 **---------------------------------------------------------------------------*/

#include "sci_types.h"

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
#define GPS_CHN_CNT_MAX				32
/**---------------------------------------------------------------------------*
 **                         Enum Definition                                   *
 **---------------------------------------------------------------------------*/
typedef enum
{
	GPS_ERR_NONE = 0x00,
	GPS_ERR_PARAM,
	GPS_ERR_NO_MODULE,
		
	GPS_ERR_MAX	
}GPS_ERR_E;

typedef enum
{
	GPS_MODE_NORMAL = 0x00,
	GPS_MODE_AGPS,
	GPS_MODE_TEST_NMEA,
	
	GPS_MODE_MAX
}GPS_MODE_E;

typedef enum
{
	GPS_STATUS_NOT_INIT = 0x00,
	GPS_STATUS_INIT,
	GPS_STATUS_OPEN,
	GPS_STATUS_SLEEP,
	GPS_STATUS_LOST,
	GPS_STATUS_CLOSE,
	
	GPS_STATUS_MAX

}GPS_STATUS_E;



/**---------------------------------------------------------------------------*
 **                         Data Structure Definition                         *
 **---------------------------------------------------------------------------*/
 typedef struct gps_nv_info_tag
{
	uint8 engine_sn[24];
	uint8 map_version[4];
	uint8 activate_code[16];
}GPS_NV_INFO_T;

typedef struct gps_validation_cipher_tag
{
	uint32 high_cipher;
	uint32 low_cipher;
}GPS_VALIDATION_CIPHER_T;

typedef struct gps_channel_info_tag
{
	uint16 	sat_id;
	uint16  snr;
	uint16  elevation;
	uint16  azimuth;
}GPS_CHN_INFO_T;

typedef struct gps_info_tag
{
	BOOLEAN			is_hw_work;
	BOOLEAN			is_position_fix;
	uint16			cnt_of_sat_in_view;
	uint16			cnt_of_sat_fix;
	
	uint16			north_or_south;		// 'N': North, 'S': South; Others: invalid
	uint16			latitude_high;		// ddmm
	uint16			latitude_low;		// .mmmm

	uint16			east_or_west;		// 'E': East; 'W': West: Others: invalid
	uint16			longitude_high;		// ddmm
	uint16			longitude_low;		// .mmmm
	
	uint16			cnt_of_chn_valid;
	GPS_CHN_INFO_T	gps_chn[GPS_CHN_CNT_MAX];
}GPS_INFO_T, *GPS_INFO_T_PTR;

typedef struct gps_operations_tag
{
  GPS_ERR_E   (*gps_init)   (uint32 port, uint32 baudrate);
  GPS_ERR_E   (*gps_open)   (GPS_MODE_E mode);
  GPS_ERR_E   (*gps_close)  (void);
  GPS_ERR_E (*gps_sleep)  (BOOLEAN is_sleep);
  GPS_ERR_E (*gps_reflash)  (void);
  uint32    (*gps_readdata) (uint8* read_buf_ptr, uint32 byte_to_read);
  uint32    (*gps_writedata)(uint8* write_buf_ptr, uint32 byte_to_write);
  BOOLEAN   (*gps_identify) (void);
  uint32    (*gps_test)   (void* ptr, uint32 param);

}GPS_OPERATIONS_T; 



/**---------------------------------------------------------------------------*
 **                     Local Function Prototypes                             *
 **---------------------------------------------------------------------------*/
/*****************************************************************************/
//  Description:    This function is used to get GPS status    
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
PUBLIC GPS_STATUS_E GPS_GetStatus(void);

/*****************************************************************************/
//  Description:    This function is used to init GPS mode and global variable    
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
PUBLIC GPS_ERR_E GPS_Init(uint32 port, uint32 baudrate);

/*****************************************************************************/
//  Description:    This function is used to open GPS module by the special mode    
//  Author:         Liangwen.Zhen
//  Note:           
//	input:			GPS_MODE_E
//	Output:			GPS_ERR_E
/*****************************************************************************/
PUBLIC GPS_ERR_E GPS_Open(GPS_MODE_E mode);

/*****************************************************************************/
//  Description:    This function is used to Close GPS module    
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
PUBLIC GPS_ERR_E GPS_Close(void);

/*****************************************************************************/
//  Description:    This function is used to Close GPS module    
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
PUBLIC GPS_ERR_E GPS_EnterSleep(BOOLEAN is_sleep);


/*****************************************************************************/
//  Description:    This function is used to read data from GPS module    
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
PUBLIC uint32 GPS_ReadData(uint8 * read_buf_ptr, uint32 byte_to_read);

/*****************************************************************************/
//  Description:    This function is used to write data to GPS module    
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
PUBLIC uint32 GPS_WriteData(uint8 * write_buf_ptr, uint32 byte_to_write);

/*****************************************************************************/
//  Description:    This function is used to test GPS module
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
PUBLIC GPS_INFO_T_PTR GPS_GetGpsInfo (void);

/*****************************************************************************/
//  Description:    This function is used to test GPS mode   
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
PUBLIC uint32 GPS_Test(void* ptr, uint32 param);

/*****************************************************************************/
//  Description:    This function is used to get information about GPS engine and MAP    
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
PUBLIC GPS_NV_INFO_T* GPS_GetNVInfo(void);

/*****************************************************************************/
//  Description:    This function is used to write information about GPS engine and MAP to nv
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
PUBLIC uint32 GPS_WriteNVInfo(GPS_NV_INFO_T* nv_info_ptr);


/*****************************************************************************/
//  Description:    This function is used to get validation cipher from DSP
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
PUBLIC uint32 GPS_GetValidationCipher(
		uint32						input_serial, 
		GPS_VALIDATION_CIPHER_T * 	output_validation_cipher_ptr);
		
/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    }
    
#endif  // End of gsp_drv.h

#endif // end _GPS_DRV_H_