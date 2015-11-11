/******************************************************************************
 ** File Name:      nmea_drv.c                                                *
 ** Author:         Liangwen.Zhen                                             *
 ** DATE:           11/15/2007                                                *
 ** Copyright:      2007 Spreadtrum, Incoporated. All Rights Reserved.        *
 ** Description:    This file defines the basic operation interfaces of NMEA  *
 **                                                                           *
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                       *
 ** ------------------------------------------------------------------------- *
 ** DATE           NAME             DESCRIPTION                               *
 ** 11/15/2007     Liangwen.Zhen    Create.                         		  *
 ******************************************************************************/

/**---------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **---------------------------------------------------------------------------*/ 
#include "ms_customize_trc.h"
#include "os_api.h"
#include "gps_nmea.h"
/**---------------------------------------------------------------------------*
 **                         Debugging Flag                                    *
 **---------------------------------------------------------------------------*/


#define DEBUG_NMEA_DRV
#ifdef  DEBUG_NMEA_DRV
	#define NMEA_TRACE   SCI_TRACE_LOW 

	//#define NMEA_LOG_ENABLE
	#ifdef NMEA_LOG_ENABLE
		#define NMEA_LOG( _format_string )	 SCI_TRACE_LOW _format_string
	#else
		#define NMEA_LOG( _format_string )
	#endif
#else
	#define NMEA_TRACE( _format_string ) 
	#define NMEA_LOG( _format_string )
#endif


/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    extern   "C" 
    {
#endif
/**---------------------------------------------------------------------------*
 **                         Macro Definition                                  *
 **---------------------------------------------------------------------------*/
#define NMEA_FRAME_LEN_MAX			76		// Bytes
#define NMEA_FRAME_CNT_MAX			16		// frame

#define NMEA_FRAME_FLAG_SIZE		6



#define NMEA_FIELD_FLAG				','

// GPGSV MSG COMMA POSITION
#define GPGSV_MID					0		// Message ID
#define GPGSV_NOM					1		// Number of Message
#define GPGSV_MN					2		// Message number
#define GPGSV_SIV					3		// Satellites in View
#define GPGSV_SID1					4		// Satellites ID
#define GPGSV_ELE1					5		// Elevation
#define GPGSV_AZI1					6		// Azimuth
#define GPGSV_SNR1					7		// SNR(C/No)
#define GPGSV_SID2					8
#define GPGSV_ELE2					9
#define GPGSV_AZI2					10
#define GPGSV_SNR2					11
#define GPGSV_SID3					12
#define GPGSV_ELE3					13
#define GPGSV_AZI3					14
#define GPGSV_SNR3					15
#define GPGSV_SID4					16
#define GPGSV_ELE4					17
#define GPGSV_AZI4					18
#define GPGSV_SNR4					19
#define GPGSV_CHKSM					20		// Checksum

// GPGGA MSG COMMA POSITION
#define GPGGA_MID					0		// Message ID
#define GPGGA_UTCT					1		// UCT Time
#define GPGGA_LAT					2		// Latitude
#define GPGGA_NSI					3		// N/S Indicator
#define GPGGA_LON					4		// Longitude
#define GPGGA_EWI					5		// E/W Indicator
#define GPGGA_PFI					6		// Position Fix Indicator
#define GPGGA_STU					7		// Satellites Used
#define GPGGA_HDOP					8		// Horizontal Dilution of precision
#define GPGGA_MSLA					9		// MSL Altitude
#define GPGGA_MUNT					10		// Units
#define GPGGA_GES					11		// Geoid Separation
#define GPGGA_GUNT					12		// Units
#define GPGGA_ADC					13		// Age of Diff. Corr.
#define GPGGA_DRSID					14		// Diff. Ref. Station ID
#define GPGGA_CHKSM					15		// Checksum

/**---------------------------------------------------------------------------*
 **                         Data Structure Definition                         *
 **---------------------------------------------------------------------------*/
 

// enum: queue status
typedef enum
{
	QUEUE_STATUS_NORMAL = 0x00,
	QUEUE_STATUS_EMPTY,
	QUEUE_STATUS_FULL,
	QUEUE_STATUS_OVERFLOW,
	
	QUEUE_STATUS_MAX	
}QUEUE_STATUS_E;

// enum: frame type
typedef enum
{
	FRAME_TYPE_GPGGA = 0x00,
	FRAME_TYPE_GPGSA,
	FRAME_TYPE_GPGSV,
	FRAME_TYPE_GPRMC,
	
	FRAME_TYPE_MAX,
	
	FRAME_TYPE_UNKNOWN = 0xFFFF0000,
	FRAME_TYPE_INGORE = 0xFFFFFFFF		
}FRAME_TYPE_E;

// struct: frame buffer information
typedef struct
{
	FRAME_TYPE_E    frame_type;
	uint32 			frame_len;
	uint8  			frame_buf[NMEA_FRAME_LEN_MAX];
	
}FRAME_INFO_T, *FRAME_INFO_T_PTR;

// struct: frame queue information
typedef struct
{
	uint16 			 in_index;
	uint16 			 out_index;
	FRAME_INFO_T_PTR queue[NMEA_FRAME_CNT_MAX];
	QUEUE_STATUS_E 	 status;
		
}FRAME_QUEUE_T, *FRAME_QUEUE_T_PTR;

 typedef BOOLEAN(* NMEA_DECODE_FUN_PTR)(FRAME_INFO_T_PTR frame_info_ptr );
/**---------------------------------------------------------------------------*
 **                         Local Variables                                   *
 **---------------------------------------------------------------------------*/
LOCAL FRAME_QUEUE_T 		s_frame_queue 	= {0};
LOCAL FRAME_INFO_T_PTR 		s_f_info_ptr	= {0};
LOCAL BOOLEAN 				s_nmea_is_init  = SCI_FALSE;
LOCAL NMEA_GPS_INFO_T		s_gps_info      = {0};

LOCAL const uint8 s_nmea_frame_flag[FRAME_TYPE_MAX][NMEA_FRAME_FLAG_SIZE] = 
{
	{"GPGGA"},
	{"GPGSA"},
	{"GPGSV"},
	{"GPRMC"}	
};


LOCAL BOOLEAN _NMEA_DecodeGPGGA(	FRAME_INFO_T_PTR frame_info_ptr );
LOCAL BOOLEAN _NMEA_DecodeGPGSA(	FRAME_INFO_T_PTR frame_info_ptr );
LOCAL BOOLEAN _NMEA_DecodeGPGSV(	FRAME_INFO_T_PTR frame_info_ptr );
LOCAL BOOLEAN _NMEA_DecodeGPRMC(	FRAME_INFO_T_PTR frame_info_ptr );

LOCAL const NMEA_DECODE_FUN_PTR s_nmea_decode_fun_tab[FRAME_TYPE_MAX] = {
	_NMEA_DecodeGPGGA,
	_NMEA_DecodeGPGSA,
	_NMEA_DecodeGPGSV,
	_NMEA_DecodeGPRMC,
	
	};

/**---------------------------------------------------------------------------*
 **                     Local Function Prototypes                             *
 **---------------------------------------------------------------------------*/
/*****************************************************************************/
//  Description:    This function is used to check if nmea is initial    
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
LOCAL BOOLEAN _NMEA_IsInit(void)
{
	return s_nmea_is_init;
}

/*****************************************************************************/
//  Description:    This function is used to set frame queue status    
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
LOCAL void _NMEA_SetQueueStatus(QUEUE_STATUS_E status)
{
	s_frame_queue.status = status;
	NMEA_LOG(("NMEA: _NMEA_SetQueueStatus() status %d", status));
}

/*****************************************************************************/
//  Description:    This function is used to get frame queue status    
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
LOCAL QUEUE_STATUS_E _NMEA_GetQueueStatus(void)
{
	return s_frame_queue.status;
}

/*****************************************************************************/
//  Description:    This function is used to reset frame queue    
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
LOCAL void _NMEA_ResetFrameQueue(void)
{
	uint32 i = 0;
	
	s_frame_queue.in_index  = 0;
	s_frame_queue.out_index = 0;
	s_frame_queue.status    = QUEUE_STATUS_EMPTY;
	
	// flush the queue
	for(i = 0; i < NMEA_FRAME_CNT_MAX; i++)
	{
		if(PNULL != s_frame_queue.queue[i])
		{
			SCI_FREE(s_frame_queue.queue[i]);
		}		
		
	}
	
}

/*****************************************************************************/
//  Description:    This function is used to decode NMEA frame 	
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
LOCAL FRAME_TYPE_E _NMEA_GetFrameType( NMEA_FRAME_PTR  frame_ptr )
{
	FRAME_TYPE_E frame_type = FRAME_TYPE_UNKNOWN;
	uint32		 cnt        = 0;
	uint32		 i          = 0;

	while(cnt < FRAME_TYPE_MAX)
	{
		for(i = 0; i < NMEA_FRAME_FLAG_SIZE; i++)
		{			
			if(s_nmea_frame_flag[cnt][i] == '\0')
			{
				frame_type = cnt;
			}
			else if(s_nmea_frame_flag[cnt][i] != frame_ptr[i])
			{
				break;
			}
		}

		if (FRAME_TYPE_UNKNOWN != frame_type)
		{
			break;
		}
		cnt++;
	} 

	if(FRAME_TYPE_MAX == cnt)
	{
		//NMEA_TRACE:"Unknown NMEA FRAME 0x%x"
		SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_NMEA_258_112_2_18_0_33_8_1561,(uint8*)"d", frame_ptr);
	}
	
	return frame_type;
}

/*****************************************************************************/
//  Description:    This function is used to add one frame to the queue
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
LOCAL QUEUE_STATUS_E _NMEA_AddFrame2Queue(FRAME_INFO_T_PTR frame_info_ptr)
{
	uint16 cur_in_index  = s_frame_queue.in_index;
	uint16 cur_out_index = s_frame_queue.out_index;

	SCI_ASSERT(PNULL != frame_info_ptr);/*assert verified*/
	
	switch(_NMEA_GetQueueStatus())
	{
		case QUEUE_STATUS_NORMAL:
			if(cur_in_index == cur_out_index)
			{
				_NMEA_SetQueueStatus(QUEUE_STATUS_FULL);
			}
			break;
		case QUEUE_STATUS_EMPTY:
			_NMEA_SetQueueStatus(QUEUE_STATUS_NORMAL);
			break;
		case QUEUE_STATUS_FULL:

			//NMEA_TRACE:"NMEA: Queue full, index %d"
			SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_NMEA_287_112_2_18_0_33_8_1562,(uint8*)"d", cur_out_index);
			SCI_ASSERT(PNULL != s_frame_queue.queue[cur_out_index]);/*assert verified*/
			SCI_FREE(s_frame_queue.queue[cur_out_index]);
			s_frame_queue.out_index = (++s_frame_queue.out_index) % NMEA_FRAME_CNT_MAX;
			
			break;
			
		case QUEUE_STATUS_OVERFLOW:
			_NMEA_SetQueueStatus(QUEUE_STATUS_OVERFLOW);
			SCI_PASSERT(0,("NEMA: Should not enter overflow"));   /*assert verified*/
			break;
			
		default:
			//SCI_ASSERT(0);
			//SCI_TRACE_LOW:"[DRV_GPS_NMEA]<WARNING> _NMEA_AddFrame2Queue: status is invalid\r\n"
			SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_NMEA_301_112_2_18_0_33_8_1563,(uint8*)"" );
			break;		
	}
	
	// Add frame to queue
	SCI_ASSERT(PNULL == s_frame_queue.queue[cur_in_index]);/*assert verified*/
	s_frame_queue.queue[cur_in_index] = frame_info_ptr;
	s_frame_queue.in_index = (++s_frame_queue.in_index) % NMEA_FRAME_CNT_MAX;

	NMEA_LOG(("_NMEA_AddFrame2Queue: in cnt %d, frame ptr 0x%x", cur_in_index, frame_info_ptr));
	
	return _NMEA_GetQueueStatus();
}

/*****************************************************************************/
//  Description:    This function is used to delete one frame from the queue
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
LOCAL FRAME_INFO_T_PTR  _NMEA_DelFrameFromQueue( void	)
{	
	BOOLEAN 		 is_valid_to_read = SCI_TRUE;
	uint16  		 next_out_index   = ((s_frame_queue.out_index+1)%NMEA_FRAME_CNT_MAX);
	uint16  		 cur_out_index    = s_frame_queue.out_index;
	FRAME_INFO_T_PTR frame_info_ptr   = PNULL;
	
	switch(_NMEA_GetQueueStatus())
	{
		case QUEUE_STATUS_NORMAL:				
			
			if(s_frame_queue.in_index == next_out_index)
			{
				_NMEA_SetQueueStatus(QUEUE_STATUS_EMPTY);
			}
			break;
			
		case QUEUE_STATUS_EMPTY:
			is_valid_to_read = SCI_FALSE;
			break;
			
		case QUEUE_STATUS_FULL:
				
			_NMEA_SetQueueStatus(QUEUE_STATUS_NORMAL);
			break;
			
		case QUEUE_STATUS_OVERFLOW:
			// delete the oldest frame from queue 
			//s_frame_queue.out_index = s_frame_queue.in_index;			
			//_NMEA_SetQueueStatus(QUEUE_STATUS_NORMAL);

			SCI_PASSERT(0,("NEMA: Should not enter overflow"));   /*assert verified*/
			break;
			
		default:
			//SCI_ASSERT(0);
			//SCI_TRACE_LOW:"[DRV_GPS_NMEA]<WARNING> _NMEA_DelFrameFromQueue: status is invalid\r\n"
			SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_NMEA_354_112_2_18_0_33_8_1564,(uint8*)"" );
			break;		
	}	
	
	if(is_valid_to_read)
	{		
		// delete frame from queue
		frame_info_ptr = s_frame_queue.queue[cur_out_index];
		s_frame_queue.queue[cur_out_index] = PNULL;	
		
		s_frame_queue.out_index = (++s_frame_queue.out_index)%NMEA_FRAME_CNT_MAX;		
	}
	else
	{
		//NMEA_TRACE:"NMEA: Queue Empty!"
		SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_NMEA_368_112_2_18_0_33_9_1565,(uint8*)"");
	}
	
	NMEA_LOG(("_NMEA_DelFrameFromQueue: valid_read %d, status %d, out cnt %d, frame ptr 0x%x", 
			is_valid_to_read,
			_NMEA_GetQueueStatus(),
			cur_out_index,
			frame_info_ptr
			));
	
	return frame_info_ptr;
}

/*****************************************************************************/
//  Description:    This function is used to snatch one NMEA frame from data 
//					stream.					
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
LOCAL NMEA_ERR_E _NMEA_SnatchOneFrame(
				uint8*  		data_stream_ptr, 
				uint32  		data_len,
				uint32* 		byte_to_scan,
				NMEA_FRAME_PTR  nmea_frame_ptr,
				uint32* 		nmea_frame_len
			)
{
	uint16		cnt = 0;
	uint16 		i   = 0;
	BOOLEAN 	is_frame_start = SCI_FALSE;
	BOOLEAN 	is_frame_end   = SCI_FALSE;
	uint32  	data_size = (data_len > NMEA_FRAME_LEN_MAX)? NMEA_FRAME_LEN_MAX:data_len;
	NMEA_ERR_E 	err = NMEA_ERR_NO_FRAME;	
	
	NMEA_LOG(("_NMEA_SnatchOneFrame() :start time %d, data size %d", SCI_GetTickCount(), data_size));

	*nmea_frame_len = 0;
		
	for( i = 0; i < data_size; i++)
	{
		if(is_frame_start)
		{
			if ( 0x0D == data_stream_ptr[i] )
			{
				if( 0x0A == data_stream_ptr[i+1] )
				{		
					i++;			
					is_frame_end = SCI_TRUE;
					nmea_frame_ptr[cnt++] = '\0';	
					*nmea_frame_len = cnt;			
					//NMEA_TRACE:"  < NMEA: %s > "
					SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_NMEA_416_112_2_18_0_33_9_1566,(uint8*)"s", nmea_frame_ptr);

					err = NMEA_ERR_NONE;
					break;
				}			
				else
				{
					// drop the frame
					err = NMEA_ERR_DATA_INVALID;
					//NMEA_TRACE:"NMEA: Drop frame - end error"
					SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_NMEA_425_112_2_18_0_33_9_1567,(uint8*)"");
					break;
				}	
			}
			else if( (data_stream_ptr[i] > 0x20 ) && ( data_stream_ptr[i] < 0x7F))
			{
				nmea_frame_ptr[cnt++] = data_stream_ptr[i];
			}
			else
			{
				// drop the frame
				err = NMEA_ERR_DATA_INVALID;
				//NMEA_TRACE:"NMEA: Drop frame - data error"
				SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_NMEA_437_112_2_18_0_33_9_1568,(uint8*)"");
				break;
			}	
			
		}
		else
		{
			if('$' == data_stream_ptr[i])
			{
				is_frame_start = SCI_TRUE;
				NMEA_LOG(("Frame start!"));
			}
		}
	
	}	
	
	*byte_to_scan = i;
	
	if( is_frame_start && !is_frame_end)
	{
		if(NMEA_ERR_DATA_INVALID != err)
		{	
			//NMEA_TRACE:"NMEA: no end"
			SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_NMEA_459_112_2_18_0_33_9_1569,(uint8*)"");
			err = NMEA_ERR_NO_FRAME_END;
			*nmea_frame_len = cnt;
		}		
		
	}
	else if( !is_frame_start)
	{
		//NMEA_TRACE:"NMEA: no start"
		SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_NMEA_467_112_2_18_0_33_9_1570,(uint8*)"");
		err = NMEA_ERR_NO_FRAME_START;
	}
	
	NMEA_LOG(("_NMEA_SnatchOneFrame() : end time %d,  err %d, scan %d, frame %d", SCI_GetTickCount(), err, *byte_to_scan, *nmea_frame_len));
	
	return err;
	
}

/*****************************************************************************/
//  Description:    This function is used to change ascii to number 	
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
LOCAL uint32 _NMEA_Ascii2Num(uint8* ascii_ptr, uint32 ascii_cnt)
{
	uint32 i = 0;
	uint32 result = 0; 
	
	SCI_ASSERT(PNULL != ascii_ptr);/*assert verified*/
	
	while(ascii_cnt)
	{
		result = result* 10 + (ascii_ptr[i] - '0');
		
		i++;
		ascii_cnt--;
	}
	
	return result;
}
/*****************************************************************************/
//  Description:    This function is used to decode NMEA GPGGA frame 	
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
LOCAL BOOLEAN _NMEA_DecodeGPGGA( FRAME_INFO_T_PTR frame_info_ptr )
{
	NMEA_GPS_INFO_T_PTR gps_ptr   = &s_gps_info;	
	NMEA_FRAME_PTR 		frame_ptr = PNULL;
	uint32			 	field_cnt = 0;
	uint32 				i         = 0;

	SCI_ASSERT(PNULL != frame_info_ptr);/*assert verified*/
	SCI_ASSERT(NULL  != frame_info_ptr->frame_len);/*assert verified*/
 
	frame_ptr = (NMEA_FRAME_PTR)frame_info_ptr->frame_buf;
	
	for( i = 0; i < frame_info_ptr->frame_len; i++)
	{
		if(NMEA_FIELD_FLAG == frame_ptr[i] )
		{
			field_cnt++;
			if(NMEA_FIELD_FLAG != frame_ptr[i+1])
			{
				switch(field_cnt)
				{
					case GPGGA_MID:
						break;
					case GPGGA_UTCT:
						break;
					case GPGGA_LAT:
						if('.' == frame_ptr[i+5])
						{
							gps_ptr->latitude_high = _NMEA_Ascii2Num(frame_ptr+i+1, 4);
							gps_ptr->latitude_low  = _NMEA_Ascii2Num(frame_ptr+i+6, 4);
						}
						else
						{
							gps_ptr->latitude_high = 0;
							gps_ptr->latitude_low  = 0;
						}
						break;
					case GPGGA_NSI:
						if(('N' == frame_ptr[i+1]) || ('S' == frame_ptr[i+1]))
						{
							gps_ptr->north_or_south = frame_ptr[i+1];
						}
						else
						{
							gps_ptr->north_or_south = 0xffff;
						}
						break;
					case GPGGA_LON:
						if('.' == frame_ptr[i+6])
						{
							gps_ptr->longitude_high = _NMEA_Ascii2Num(frame_ptr+i+1, 5);
							gps_ptr->longitude_low  = _NMEA_Ascii2Num(frame_ptr+i+7, 4);
						}
						else
						{
							gps_ptr->longitude_high = 0;
							gps_ptr->longitude_low  = 0;
						}
						break;
					case GPGGA_EWI:
						if(('E' == frame_ptr[i+1]) || ('W' == frame_ptr[i+1]))
						{
							gps_ptr->east_or_west= frame_ptr[i+1];
						}
						else
						{
							gps_ptr->east_or_west = 0xffff;
						}
						break;
					case GPGGA_PFI:						
						gps_ptr->is_position_fix = (0 != _NMEA_Ascii2Num(frame_ptr+i+1, 1))?SCI_TRUE:SCI_FALSE;
						break;
					case GPGGA_STU:
						gps_ptr->cnt_of_sat_fix  = _NMEA_Ascii2Num(frame_ptr+i+1, 2);
						break;
					case GPGGA_HDOP:
						break;
					case GPGGA_MSLA:
						break;
					case GPGGA_MUNT:
						break;
					case GPGGA_GES:
						break;
					case GPGGA_GUNT:
						break;
					case GPGGA_ADC:
						break;
					case GPGGA_DRSID:
						break;
					case GPGGA_CHKSM:
						break;
					default:
						//SCI_ASSERT(0);
						//SCI_TRACE_LOW:"[DRV_NMEA]<WARNING> _NMEA_DecodeGPGGA: field_cnt is invalid\r\n"
						SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_NMEA_593_112_2_18_0_33_9_1571,(uint8*)"" );
						break;
				}
			}
			else
			{
				switch(field_cnt)
				{
					case GPGGA_MID:
						break;
					case GPGGA_UTCT:
						break;
					case GPGGA_LAT:
						gps_ptr->latitude_high  = 0;
						gps_ptr->latitude_low   = 0;
						break;
					case GPGGA_NSI:
						gps_ptr->north_or_south = 0xffff;
						break;
					case GPGGA_LON:
						gps_ptr->longitude_high = 0;
						gps_ptr->longitude_low  = 0;
						break;
					case GPGGA_EWI:
						gps_ptr->east_or_west   = 0xffff;
						break;
					case GPGGA_PFI:
						gps_ptr->is_position_fix = SCI_FALSE;
						break;
					case GPGGA_STU:
						gps_ptr->cnt_of_sat_fix = 0;
						break;
					case GPGGA_HDOP:
						break;
					case GPGGA_MSLA:
						break;
					case GPGGA_MUNT:
						break;
					case GPGGA_GES:
						break;
					case GPGGA_GUNT:
						break;
					case GPGGA_ADC:
						break;
					case GPGGA_DRSID:
						break;
					case GPGGA_CHKSM:
						break;
					default:
						//SCI_ASSERT(0);
						//SCI_TRACE_LOW:"[DRV_NMEA]<ERROR> _NMEA_DecodeGPGGA: field_cnt is invalid\r\n"
						SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_NMEA_643_112_2_18_0_33_9_1572,(uint8*)"" );
						break;
				}
			}

		}
	}

	NMEA_LOG(("Decode GPGGA cnt %d", field_cnt));
	
	return SCI_TRUE;
}

/*****************************************************************************/
//  Description:    This function is used to decode NMEA GPGSA frame 	
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
LOCAL BOOLEAN _NMEA_DecodeGPGSA(	FRAME_INFO_T_PTR frame_info_ptr )
{
	return SCI_TRUE;
}

/*****************************************************************************/
//  Description:    This function is used to decode NMEA GPGSV frame 	
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
LOCAL BOOLEAN _NMEA_DecodeGPGSV(	FRAME_INFO_T_PTR frame_info_ptr )
{
	NMEA_GPS_INFO_T_PTR gps_ptr   = &s_gps_info;
	NMEA_FRAME_PTR 		frame_ptr = PNULL;
	uint32 				field_cnt = 0;
	uint32 				i         = 0;
	uint8 				num_of_msg= 0;
	uint8  				msg_num   = 0;
	uint8  				index	  = 0;

	SCI_ASSERT(PNULL != frame_info_ptr);/*assert verified*/
	SCI_ASSERT(PNULL != gps_ptr);/*assert verified*/
	SCI_ASSERT(NULL  != frame_info_ptr->frame_len);/*assert verified*/
 
	frame_ptr = (NMEA_FRAME_PTR)frame_info_ptr->frame_buf;

	if(!gps_ptr->is_hw_work)
	{
		gps_ptr->is_hw_work = SCI_TRUE;
	}
	
	for( i = 0; i < frame_info_ptr->frame_len; i++)
	{
		if(NMEA_FIELD_FLAG == frame_ptr[i] )
		{
			field_cnt++;
			
			if(NMEA_FIELD_FLAG != frame_ptr[i+1])
			{
				switch(field_cnt)
				{
					case GPGSV_MID:
						break;
					case GPGSV_NOM:
						num_of_msg = _NMEA_Ascii2Num(frame_ptr+i+1, 1);
						break;
					case GPGSV_MN:
						msg_num    = _NMEA_Ascii2Num(frame_ptr+i+1, 1);
						SCI_ASSERT(msg_num > 0);/*assert verified*/
						index = (msg_num-1)*4;
						break;
					case GPGSV_SIV:
						gps_ptr->cnt_of_sat_in_view    	= _NMEA_Ascii2Num(frame_ptr+i+1, 2);
						gps_ptr->cnt_of_chn_valid       = gps_ptr->cnt_of_sat_in_view;
						break;
					case GPGSV_SID1:						
						gps_ptr->gps_chn[index].sat_id 	= _NMEA_Ascii2Num(frame_ptr+i+1, 2);
						break;
					case GPGSV_ELE1:						
						break;
					case GPGSV_AZI1:
						break;
					case GPGSV_SNR1:
						if('*' != frame_ptr[i+1])
						{
							gps_ptr->gps_chn[index].snr = _NMEA_Ascii2Num(frame_ptr+i+1, 2);
						}
						else
						{
							gps_ptr->gps_chn[index].snr = 0;
						}
						break;
					case GPGSV_SID2:
						index++;
						gps_ptr->gps_chn[index].sat_id 	= _NMEA_Ascii2Num(frame_ptr+i+1, 2);
						break;
					case GPGSV_ELE2:
						break;
					case GPGSV_AZI2:
						break;
					case GPGSV_SNR2:
						if('*' != frame_ptr[i+1])
						{
							gps_ptr->gps_chn[index].snr = _NMEA_Ascii2Num(frame_ptr+i+1, 2);
						}
						else
						{
							gps_ptr->gps_chn[index].snr = 0;
						}
						break;
					case GPGSV_SID3:
						index++;
						gps_ptr->gps_chn[index].sat_id 	= _NMEA_Ascii2Num(frame_ptr+i+1, 2);
						break;
					case GPGSV_ELE3:
						break;
					case GPGSV_AZI3:
						break;
					case GPGSV_SNR3:
						if('*' != frame_ptr[i+1])
						{
							gps_ptr->gps_chn[index].snr = _NMEA_Ascii2Num(frame_ptr+i+1, 2);
						}
						else
						{
							gps_ptr->gps_chn[index].snr = 0;
						}
						break;
					case GPGSV_SID4:
						index++;
						gps_ptr->gps_chn[index].sat_id 	= _NMEA_Ascii2Num(frame_ptr+i+1, 2);
						break;
					case GPGSV_ELE4:
						break;
					case GPGSV_AZI4:
						break;
					case GPGSV_SNR4:
						if('*' != frame_ptr[i+1])
						{
							gps_ptr->gps_chn[index].snr = _NMEA_Ascii2Num(frame_ptr+i+1, 2);
						}
						else
						{
							gps_ptr->gps_chn[index].snr = 0;
						}
						break;
					case GPGSV_CHKSM:
						break;

					default:
						//SCI_ASSERT(0);
						//SCI_TRACE_LOW:"[DRV_NMEA]<WARNING> _NMEA_DecodeGPGSV:field_cnt is invalid\r\n "
						SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_NMEA_788_112_2_18_0_33_10_1573,(uint8*)"" );
						break;

				}
			}
			else
			{
				switch(field_cnt)
				{
					case GPGSV_MID:
						break;
					case GPGSV_NOM:
						num_of_msg = 0;
						break;
					case GPGSV_MN:
						msg_num    = 0;						
						break;
					case GPGSV_SIV:
						gps_ptr->cnt_of_sat_in_view = 0;
						break;
					case GPGSV_SID1:						
						gps_ptr->gps_chn[index].sat_id = 0xFF;
						break;
					case GPGSV_ELE1:						
						break;
					case GPGSV_AZI1:
						break;
					case GPGSV_SNR1:
						gps_ptr->gps_chn[index].snr = 0;
						break;
					case GPGSV_SID2:
						index++;
						gps_ptr->gps_chn[index].sat_id = 0xFF;
						break;
					case GPGSV_ELE2:
						break;
					case GPGSV_AZI2:
						break;
					case GPGSV_SNR2:
						gps_ptr->gps_chn[index].snr = 0;
						break;
					case GPGSV_SID3:
						index++;
						gps_ptr->gps_chn[index].sat_id = 0xFF;
						break;
					case GPGSV_ELE3:
						break;
					case GPGSV_AZI3:
						break;
					case GPGSV_SNR3:
						gps_ptr->gps_chn[index].snr = 0;
						break;
					case GPGSV_SID4:
						index++;
						gps_ptr->gps_chn[index].sat_id = 0xFF;
						break;
					case GPGSV_ELE4:
						break;
					case GPGSV_AZI4:
						break;
					case GPGSV_SNR4:
						if('*' == frame_ptr[i+1])
						{
							//gps_ptr->gps_chn[index].snr = 0;
						}
						break;
					case GPGSV_CHKSM:
						break;

					default:
						//SCI_ASSERT(0);
						//SCI_TRACE_LOW:"[DRV_NMEA]<ERROR> _NMEA_DecodeGPGSV:field_cnt is invalid\r\n "
						SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_NMEA_859_112_2_18_0_33_10_1574,(uint8*)"" );
						break;
				}
			}
		
		}
	}

	NMEA_LOG(("Decode GPGSV cnt %d", field_cnt));
	
	return SCI_TRUE;

}

/*****************************************************************************/
//  Description:    This function is used to decode NMEA GPRMC frame 	
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
LOCAL BOOLEAN _NMEA_DecodeGPRMC(	FRAME_INFO_T_PTR frame_info_ptr )
{
	return SCI_TRUE;
}

/*****************************************************************************/
//  Description:    This function is used to decode NMEA frame 	
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
LOCAL BOOLEAN _NMEA_Decode( FRAME_INFO_T_PTR frame_info_ptr )
{
	SCI_ASSERT(PNULL != frame_info_ptr);/*assert verified*/
	
	if(frame_info_ptr->frame_type < FRAME_TYPE_MAX)
	{
		if(PNULL != s_nmea_decode_fun_tab[frame_info_ptr->frame_type])
		{
			s_nmea_decode_fun_tab[frame_info_ptr->frame_type](frame_info_ptr);
		}
	}
	

	return SCI_TRUE;
}

/**---------------------------------------------------------------------------*
 **                     PUBLIC Function Prototypes                            *
 **---------------------------------------------------------------------------*/
/*****************************************************************************/
//  Description:    This function is used to initialize NMEA manager 	
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
PUBLIC void NMEA_Init(void)
{		
	_NMEA_ResetFrameQueue();
	
	s_f_info_ptr = PNULL;

	SCI_MEMSET(&s_gps_info, 0, sizeof(NMEA_GPS_INFO_T));
	
	s_nmea_is_init = SCI_TRUE;

	//NMEA_TRACE:"NMEA_Init() !"
	SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_NMEA_915_112_2_18_0_33_10_1575,(uint8*)"");
}

/*****************************************************************************/
//  Description:    This function is used to close NMEA manager 			
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
PUBLIC void NMEA_Close(void)
{
	_NMEA_ResetFrameQueue();
	s_nmea_is_init = SCI_FALSE;

	//NMEA_TRACE:"NMEA_Close() !"
	SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_NMEA_926_112_2_18_0_33_10_1576,(uint8*)"");
}

/*****************************************************************************/
//  Description:    This function is used to snatch all NMEA frame from data 
//					stream, and add the frame to the queue					
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
PUBLIC NMEA_ERR_E NMEA_SnatchFramesFromStream(
				uint8* data_stream_ptr, 		// point to data stream
				uint32 data_stream_len			// lenght of data stream
			)		
{
	uint32 			data_cnt 	 	= data_stream_len;
	uint8* 			cur_data_ptr 	= data_stream_ptr;	
	//uint32 			frame_len    	= 0;
	uint32 			byte_to_scan 	= 0;
	NMEA_ERR_E 		err			 	= NMEA_ERR_NONE;
	FRAME_INFO_T_PTR frame_info_ptr	= PNULL;
	
	if(!_NMEA_IsInit())
	{
		//NMEA_TRACE:"NMEA NOT INIT !!!"
		SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_NMEA_947_112_2_18_0_33_10_1577,(uint8*)"");
		return NMEA_ERR_NOT_INIT;
	}

	if(NULL == data_stream_len)
	{
		//NMEA_TRACE:"No data in NMEA Stream !!"
		SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_NMEA_953_112_2_18_0_33_10_1578,(uint8*)"");
		return NMEA_ERR_NO_DATA;
	}
		
	//NMEA_TRACE:"*** NMEA_SnatchFramesFromStream START, data len %d"
	SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_NMEA_957_112_2_18_0_33_10_1579,(uint8*)"d", data_stream_len);
	
	if(PNULL != s_f_info_ptr)
	{
		//NMEA_TRACE:"NMEA: Split frame ! cur data %d"
		SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_NMEA_961_112_2_18_0_33_10_1580,(uint8*)"d", s_f_info_ptr->frame_len);
		
		SCI_MEMCPY((uint8*)(s_f_info_ptr->frame_buf + s_f_info_ptr->frame_len), data_stream_ptr,\
			(NMEA_FRAME_LEN_MAX - s_f_info_ptr->frame_len));

		frame_info_ptr = (FRAME_INFO_T_PTR)SCI_ALLOC_APP(sizeof(FRAME_INFO_T));
		SCI_ASSERT(PNULL != frame_info_ptr);/*assert verified*/
		err = _NMEA_SnatchOneFrame(
					s_f_info_ptr->frame_buf,
					NMEA_FRAME_LEN_MAX,
					&byte_to_scan,
					frame_info_ptr->frame_buf,
					&frame_info_ptr->frame_len
				);
		if( NMEA_ERR_NONE == err )
		{
			// add the frame to queque, and go to loop
			SCI_ASSERT(NULL != frame_info_ptr->frame_len);/*assert verified*/
			cur_data_ptr += frame_info_ptr->frame_len;

			frame_info_ptr->frame_type = _NMEA_GetFrameType(frame_info_ptr->frame_buf);
			if(FRAME_TYPE_UNKNOWN != frame_info_ptr->frame_type)
			{
				_NMEA_Decode(frame_info_ptr);
				_NMEA_AddFrame2Queue(frame_info_ptr);
				frame_info_ptr = PNULL;
			}
		}	 
	 	
		SCI_FREE(s_f_info_ptr);
	}
	
	while(data_cnt)
	{
		NMEA_LOG(("snatch step 1, data_cnt %d", data_cnt));

		if(PNULL == frame_info_ptr)
		{
			frame_info_ptr = (FRAME_INFO_T_PTR)SCI_ALLOC_APP(sizeof(FRAME_INFO_T));
		}
		SCI_ASSERT(PNULL != frame_info_ptr);/*assert verified*/
		
		err = _NMEA_SnatchOneFrame(
					cur_data_ptr,
					data_cnt,  
					&byte_to_scan,
					frame_info_ptr->frame_buf,
					&frame_info_ptr->frame_len
				);
		SCI_ASSERT(data_cnt >= byte_to_scan);/*assert verified*/
		if( NMEA_ERR_NONE == err )
		{
			// add the frame to queque, and go to loop
			SCI_ASSERT(NULL != frame_info_ptr->frame_len);/*assert verified*/
			SCI_ASSERT(NULL != byte_to_scan);/*assert verified*/
			cur_data_ptr += byte_to_scan;
			data_cnt -= byte_to_scan;

			frame_info_ptr->frame_type = _NMEA_GetFrameType(frame_info_ptr->frame_buf);
			if(FRAME_TYPE_UNKNOWN != frame_info_ptr->frame_type)
			{
				_NMEA_Decode(frame_info_ptr);
				_NMEA_AddFrame2Queue(frame_info_ptr);
				frame_info_ptr = PNULL;
			}
			else
			{
				// do nothing, continus snatch next frame
			}			
			
		}
		else if( NMEA_ERR_NO_FRAME_END == err )
		{
			// copy the data to temp buffer
			SCI_ASSERT(NULL != frame_info_ptr->frame_len);/*assert verified*/
			SCI_ASSERT(PNULL == s_f_info_ptr);/*assert verified*/
			s_f_info_ptr = (FRAME_INFO_T_PTR)SCI_ALLOC_APP(sizeof(FRAME_INFO_T));
			
			SCI_MEMCPY(s_f_info_ptr->frame_buf, frame_info_ptr->frame_buf, frame_info_ptr->frame_len);			
			s_f_info_ptr->frame_len = frame_info_ptr->frame_len;			
			
			data_cnt = 0;
		}
		else if( NMEA_ERR_NO_FRAME_START == err)
		{
			// Do nothing
			cur_data_ptr += byte_to_scan;
			data_cnt -= byte_to_scan;
		}
		else
		{
			// Do nothing
			cur_data_ptr += byte_to_scan;
			data_cnt -= byte_to_scan;
		}
	
	}

	// free memory
	if(PNULL != frame_info_ptr)
	{
		SCI_FREE(frame_info_ptr);
	}
	
	//NMEA_TRACE:"*** NMEA_SnatchFramesFromStream END "
	SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_NMEA_1065_112_2_18_0_33_10_1581,(uint8*)"");
	
	return NMEA_ERR_NONE;
}

/*****************************************************************************/
//  Description:    This function is used to get NMEA frame from the frame queue 			
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
PUBLIC NMEA_ERR_E NMEA_GetFrame(
				NMEA_FRAME_PTR frame_buf_ptr,			// point to frame buffer,
				uint32 		   frame_buf_len,			// lenght of frame buffer
				uint32* 	   data_len_ptr			// point to lenght of return data
			)
{
	NMEA_ERR_E 		 err 			= NMEA_ERR_NONE;	
	FRAME_INFO_T_PTR frame_info_ptr = PNULL;
	
	if(!_NMEA_IsInit())
	{
		return NMEA_ERR_NOT_INIT;
	}
	
	frame_info_ptr = _NMEA_DelFrameFromQueue( );
	if( PNULL != frame_info_ptr )
	{		
	
		SCI_ASSERT(NULL  != frame_info_ptr->frame_len);/*assert verified*/
		SCI_ASSERT(frame_info_ptr->frame_len <= NMEA_FRAME_LEN_MAX);/*assert verified*/
		if( PNULL != frame_buf_ptr)
		{
			SCI_ASSERT(frame_buf_len > *data_len_ptr);/*assert verified*/
			SCI_MEMCPY(frame_buf_ptr, frame_info_ptr->frame_buf, *data_len_ptr);
		}
		SCI_FREE(frame_info_ptr);
	}
	else
	{
		data_len_ptr = 0;
		err = NMEA_ERR_NO_FRAME;
	}
	
	return err;
}			


/*****************************************************************************/
//  Description:    This function is used to get GPS information from NMEA 			
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
PUBLIC NMEA_GPS_INFO_T_PTR NMEA_GetGpsInfo(void)
{
	if(!_NMEA_IsInit())
	{
		return PNULL;
	}
	
	return &s_gps_info;	
}

/********************************* For Test ***********************************/
//#define _NMEA_TEST_

#ifdef _NMEA_TEST_
#include "gps_drv.h"

#define NMEA_TEMP_BUF_LEN							(1024*4)
LOCAL uint32 s_nmea_temp_buf[NMEA_TEMP_BUF_LEN/4];

LOCAL void _NMEA_Test_Task(uint32 argc, void* argv)
{
	//uint32 data_len = 0;
	
	SCI_Sleep(1000);
	
	//NMEA_TRACE:"*******  _NMEA_Test_Task Start *******"
	SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_NMEA_1137_112_2_18_0_33_10_1582,(uint8*)"");
	
	while(SCI_TRUE)
	{
		SCI_Sleep(1000);
		
		//NMEA_TRACE:"-- NMEA_Test_Task Loop --"
		SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_NMEA_1143_112_2_18_0_33_10_1583,(uint8*)"");
		
		GPS_ReadData((uint8*)s_nmea_temp_buf, NMEA_TEMP_BUF_LEN);
		
	}
}

#define  TASK_NMEA_PRI   PRI_NMEA_TASK   //28

PUBLIC void NMEA_Test(void)
{
		
	SCI_CreateThread( "NMEA_Test_Task","NMEA_Test_Queue",_NMEA_Test_Task,0,PNULL,0x8000,10,TASK_NMEA_PRI,SCI_PREEMPT,SCI_AUTO_START );

	GPS_Init();
	GPS_Open(GPS_MODE_TEST_NMEA);
	
}
#else
PUBLIC void NMEA_Test(void)
{

}

#endif // _NMEA_TEST_
/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    }
    
#endif  // End of gps_nmea.c
