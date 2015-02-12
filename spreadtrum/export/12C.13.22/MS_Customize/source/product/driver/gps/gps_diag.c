/******************************************************************************
 ** File Name:      gps_diag.c                                                *
 ** Author:         Liangwen.Zhen                                             *
 ** DATE:           11/29/2007                                                *
 ** Copyright:      2007 Spreadtrum, Incoporated. All Rights Reserved.        *
 ** Description:    This file defines the basic operation interfaces of GPS   *
 ** 				Diag communication										  *
 **                                                                           *
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                       *
 ** ------------------------------------------------------------------------- *
 ** DATE           NAME             DESCRIPTION                               *
 ** 11/29/2007     Liangwen.Zhen    Create.                         		  *
 ******************************************************************************/

/**---------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **---------------------------------------------------------------------------*/ 


/**---------------------------------------------------------------------------*
 **                         Debugging Flag                                    *
 **---------------------------------------------------------------------------*/

#include "ms_customize_trc.h"
#include "os_api.h"
#include "cmddef.h"
#include "gps_diag.h"
#include "gps_drv.h"

#define DEBUG_GPS_DIAG
#ifdef  DEBUG_GPS_DIAG
	#define DIAG_TRACE   SCI_TRACE_LOW 
#else
	#define DIAG_TRACE( _format_string )  
#endif


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
#define GPS_API_ENABLE
/**---------------------------------------------------------------------------*
 **                         Global Variables                                  *
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 **                         Local Variables                                   *
 **---------------------------------------------------------------------------*/
LOCAL GPS_DIAG_MODE_E s_gps_diag_mode = GPS_DIAG_MODE_MAX;

/**---------------------------------------------------------------------------*
 **                         Constant Variables                                *
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 **                     Local Function Prototypes                             *
 **---------------------------------------------------------------------------*/
/*****************************************************************************/
//  Description:    This function is diag command routine function about GPS    
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
LOCAL uint32 _GPS_DIAG_CmdRoutine(
    uint8 **	dest_ptr,       // Pointer of the response message.
    uint16 *	dest_len_ptr,   // Pointer of size of the response message in uin8.
    const uint8 *src_ptr,   	// Pointer of the input message. 
    uint16 		src_len)        // Size of the source buffer in uint8. 
{
 
    GPS_DIAG_MSG_PTR 	comm_ptr 		 = PNULL;
    GPS_DIAG_MSG_PTR	res_ptr  		 = PNULL;
#ifndef GPS_API_ENABLE	
    COM_OBJ 			*pCom 	 		 = &com1_ins;
#endif
    GPS_NV_INFO_T		*gps_nv_info_ptr = PNULL;
	GPS_INFO_T_PTR		gps_info_ptr	 = PNULL;
    
    comm_ptr = (GPS_DIAG_MSG_PTR)(src_ptr + sizeof(MSG_HEAD_T));
    
    //DIAG_TRACE:"_GPS_DIAG_CmdRoutine() begin: cmd=%d, num=%d, val_list=%s"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_DIAG_78_112_2_18_0_33_6_1551,(uint8*)"dds", comm_ptr->cmd, comm_ptr->param_num, comm_ptr->param_val_list);
  
    //deal with GPS calibration command
    switch (comm_ptr->cmd)
    {
        case GPS_DIAG_CMD_SEND:  
			#ifdef GPS_API_ENABLE
			comm_ptr->param_num = GPS_WriteData( comm_ptr->param_val_list, comm_ptr->param_num );
			#else
            UART_Tx_Int_Enable(pCom->port, FALSE);
            comm_ptr->param_num = QueueInsert(&Output_Q, comm_ptr->param_val_list, comm_ptr->param_num);
            UART_Tx_Int_Enable(pCom->port, TRUE);
			#endif
            break;
		
        case GPS_DIAG_CMD_RECEIVE:
			#ifdef GPS_API_ENABLE
			comm_ptr->param_num = GPS_ReadData( comm_ptr->param_val_list, comm_ptr->param_num );
			#else
            UART_Rx_Int_Enable(pCom->port, FALSE);
            comm_ptr->param_num = QueueInsert(&Input_Q, comm_ptr->param_val_list, comm_ptr->param_num);
            UART_Rx_Int_Enable(pCom->port, TRUE);
			#endif
            break;
			
        case GPS_DIAG_CMD_OPEN:
			#ifdef GPS_API_ENABLE
			s_gps_diag_mode = comm_ptr->param_val_list[0];
			if((GPS_DIAG_MODE_WR_DATA == s_gps_diag_mode) && (GPS_DIAG_MODE_GET_INFO == s_gps_diag_mode))
			{
				GPS_Open(GPS_MODE_NORMAL);
				GPS_WriteData((uint8 *)"GPSCalCmd open.\r\n", 17);
			}
			#else
            QueueClean(&Input_Q);
            QueueClean(&Output_Q);
            GPS_ComInit(9600);
            GPS_ComWrite((uint8 *)"GPSCalCmd open.\r\n", 17);
			#endif
            break;
		
        case GPS_DIAG_CMD_CLOSE:
			#ifdef GPS_API_ENABLE
			if((GPS_DIAG_MODE_WR_DATA == s_gps_diag_mode) && (GPS_DIAG_MODE_GET_INFO == s_gps_diag_mode))
			{
				GPS_Close();
			}
			s_gps_diag_mode = GPS_DIAG_MODE_MAX;
			#else
            GPS_ComClose();
			#endif
            break;
		
        case GPS_DIAG_CMD_WRITE_NV:
        	SCI_ASSERT(sizeof(GPS_NV_INFO_T) == comm_ptr->param_num);/*assert verified*/
        	GPS_WriteNVInfo((GPS_NV_INFO_T*)comm_ptr->param_val_list);
	        break;
			
	    case GPS_DIAG_CMD_READ_NV:
	    	SCI_ASSERT(sizeof(GPS_NV_INFO_T) == comm_ptr->param_num);/*assert verified*/
	    	gps_nv_info_ptr = GPS_GetNVInfo();
	    	SCI_ASSERT(PNULL != gps_nv_info_ptr);/*assert verified*/
	    	SCI_MEMCPY(comm_ptr->param_val_list, gps_nv_info_ptr, sizeof(GPS_NV_INFO_T));
	    	break;

		case GPS_DIAG_CMD_GET_GPS_INFO:
			SCI_ASSERT(sizeof(GPS_INFO_T) == comm_ptr->param_num);/*assert verified*/
	    	gps_info_ptr = GPS_GetGpsInfo();
	    	SCI_ASSERT(PNULL != gps_info_ptr);/*assert verified*/
	    	SCI_MEMCPY(comm_ptr->param_val_list, gps_info_ptr, sizeof(GPS_INFO_T));
			break;
        default:
            break;    
    }

	// Build response to channel server
    *dest_ptr 	  = SCI_ALLOC_APP(sizeof(MSG_HEAD_T) + sizeof(GPS_DIAG_MSG_T));
    *dest_len_ptr = sizeof(MSG_HEAD_T) + sizeof(GPS_DIAG_MSG_T);
    SCI_MEMCPY(*dest_ptr, src_ptr, sizeof(MSG_HEAD_T)+ sizeof(GPS_DIAG_MSG_T));
    ((MSG_HEAD_T *) (*dest_ptr))->len = *dest_len_ptr;
    
    //DIAG_TRACE:"GPSCalCmd end: cmd=%d, num=%d, val_list=%s"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_DIAG_159_112_2_18_0_33_7_1552,(uint8*)"dds", res_ptr->cmd, res_ptr->param_num, res_ptr->param_val_list);
	
    return SCI_TRUE;
} 


/**---------------------------------------------------------------------------*
 **                     PUBLIC Function Prototypes                             *
 **---------------------------------------------------------------------------*/
/*****************************************************************************/
//  Description:    This function is register diag command routine
//  Author:         Liangwen.Zhen
//  Note:           
/*****************************************************************************/
PUBLIC void GPS_DIAG_RegDiagCmdRoutine(void)
{    
    //DIAG_TRACE:"GPS: GPS_DIAG_RegDiagCmdRoutine"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPS_DIAG_172_112_2_18_0_33_7_1553,(uint8*)"" );
	
    DIAG_RegisterCmdRoutine(DIAG_GPS_F, _GPS_DIAG_CmdRoutine);
}
/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    }
    
#endif  // End of gsp_diag.c