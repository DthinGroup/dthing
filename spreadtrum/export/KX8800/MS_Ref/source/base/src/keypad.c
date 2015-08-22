/******************************************************************************
 ** File Name:      keypad.c                                                  *
 ** Author:         Richard Yang                                              *
 ** DATE:           21/11/2001                                                *
 ** Copyright:      2001 Spreatrum, Incoporated. All Rights Reserved.         *
 ** Description:                                                              *
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                       *
 ** ------------------------------------------------------------------------- *
 ** DATE           NAME             DESCRIPTION                               *
 ** 21/11/2001     Richard.Yang     Create.                                   *
 ** 12/09/2002     Xueliang.Wang    Modified.                                 *
 ** 09/16/2003     Xueliang.Wang    Modify CR4013                             *
 ******************************************************************************/

/**---------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **---------------------------------------------------------------------------*/
#include "ms_ref_base_trc.h"
#include "tasks_id.h" 
#include "keypad.h"
#include "dal_lcd.h"
#include "server.h"
#include "power.h"
#include "sci_service.h"
#include "driver_export.h"
#include "kpd_drv.h"

#ifdef WIN32
    #include "bridgelayer.h"
#else
    #include "sc_reg.h"
    #include "tb_hal.h"
	#include "arm_reg.h"
//	#include "gpio_drv.h"
#endif

/**---------------------------------------------------------------------------*
 **                         Debugging Flag                                    *
 **---------------------------------------------------------------------------*/

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    extern   "C" 
    {
#endif


/* @jim.zhang CR:MS10763 2004-07-21 */
extern BOOLEAN RM_GetPinTestFlag(void);
/* end CR:MS10763 */

//@lin.liu(2004-03-31). CR7972.
// Change the PB pin function when enter/leave Deep sleep mode.
//
//void PowerButton_SetMode( BOOLEAN  bGPIO_Mode );
//End.


/**---------------------------------------------------------------------------*
 **                         Global Variables                                  *
 **---------------------------------------------------------------------------*/

#ifdef WIN32
    #define KPD_INVALID_KEY  0x0FFFF
#else
    #define KPD_INVALID_KEY  TB_KPD_INVALID_KEY
#endif  // WIN32

LOCAL uint32 s_vk_state[KPD_VK_NUM>>5] = {0};
LOCAL BOOLEAN s_is_greenkey_pressed = FALSE;

/**---------------------------------------------------------------------------*
 **                     Local Function Prototypes                             *
 **---------------------------------------------------------------------------*/
#ifndef WIN32
/*****************************************************************************/
//  Description:    This function is used to register to TBIOS. It will be
//                  called to handle the message from lower layer.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
PUBLIC void KPDSVR_Callback(TB_MSG * msg_ptr);/*lint -esym(765,KPDSVR_Callback)*/
#endif

/*****************************************************************************/
//  Description:    This function initialize the keypad, register callback 
//                  function to TBIOS, and initialize local variable.
//  Author:         
//	Note:           
/*****************************************************************************/
LOCAL void Init(void);

/*****************************************************************************/
//  Description:    This function check if the MS is ready. If the state of MS
//                  is POWER_MS_STATUS_POWER_ON or POWER_MS_STATUS_POWER_OFF,
//                  it return SCI_TRUE, else return SCI_FALSE.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
LOCAL BOOLEAN IsMsReady(void);

/*****************************************************************************/
//  Description:    This function is used to send signal to client. It is a
//                  callback function.
//  Author:         Richard.Yang
//	Note:           
/*****************************************************************************/
LOCAL void KPDSVR_SendKey(BLOCK_ID id, uint32 argc, void *argv);

/*****************************************************************************/
//  Description:    set green key is pressed.it is used by ui com test.bad method.
//	Global resource dependence: 
//  Author:        yuehz
//	Note:			
//  History:        2006.04.18   
/*****************************************************************************/
PUBLIC void KPD_SetGreenKeyPressed(BOOLEAN is_pressed)/*lint -esym(765,KPD_SetGreenKeyPressed)*/
{
    s_is_greenkey_pressed=is_pressed;
}

/*****************************************************************************/
//  Description:    return green key is pressed.it is used by ui com test.bad method.
//	Global resource dependence: 
//  Author:        yuehz
//	Note:			
//  History:        2006.04.18   
/*****************************************************************************/
PUBLIC BOOLEAN KPD_GetGreenKeyPressed(void)/*lint -esym(765,KPD_GetGreenKeyPressed)*/
{
    return s_is_greenkey_pressed;
}

/**---------------------------------------------------------------------------*
 **                         Function Definitions                              *
 **---------------------------------------------------------------------------*/
/*****************************************************************************/
//  Description:    Main thread of KPDSVR task. It manages the keypad signal
//                  from driver, and send it to the tasks which registered to 
//                  it before. It also processes PowerOn/PowerOff operation.
//  Author:         Richard.Yang
//	Note:           
/*****************************************************************************/
THREAD_ENTRY(KPDSVR)/*lint -esym(765, thread_entry_KPDSVR)*/
{
    uint16          key             = 0;
    KPDSVR_SIG_T    *key_sig_ptr    = SCI_NULL;	
    //FIXME: If allow default KPDSVR callback. It may continously send key signal to service sometimes.
    SCI_CreateClientList(KEYPAD_SERVICE, (KPD_MSG_MAX_NUM & 0x0ff), 
        (REG_CALLBACK) KPDSVR_SendKey);
 
    // Initialize
    Init();
#ifdef M2M_EVB_SUPPORT
    //CUST_RegisterRefService();
#endif 
	//The more conventional form of
    //  infinite loop prefix is for(;;)
	//while (1)
	for(;;)
	{
	    // Receive signal.
		key_sig_ptr = (KPDSVR_SIG_T *)SCI_GetSignal(KPDSVR);
		
		// Get key.
		key = key_sig_ptr->key;
		
		if(key >= KPD_VK_NUM)
		{
		    SCI_FREE(key_sig_ptr);
		    //SCI_TRACE_LOW:"KPD_SVR: Invalid Key: 0x%x"
		    SCI_TRACE_ID(TRACE_TOOL_CONVERT,KEYPAD_149_112_2_18_1_5_25_43,(uint8*)"d", key);
		    continue;
		}
		// If the MS is not ready, ignore this signal.
		//if ((!IsMsReady()) || (KPD_INVALID_KEY == key) )
		if (SCI_FALSE == IsMsReady())
		{
		    SCI_FREE(key_sig_ptr);
		    continue;
		}
		
		// Process it according to the signal code.
    	switch (key_sig_ptr->SignalCode)
    	{   
    	case KPD_DOWN:
    	
    		//Record vk state for async state requirement
    		s_vk_state[key>>5] |= 0x1<<(key&0x1F);
    		
       	    // Send KPD_DOWN message to client.
        	//SCI_TRACE_LOW:"KPD_SVR: Virtual Key: 0x%x is DOWN\n"
        	SCI_TRACE_ID(TRACE_TOOL_CONVERT,KEYPAD_169_112_2_18_1_5_25_44,(uint8*)"d", key);
        	SCI_SendEventToClient(KEYPAD_SERVICE, KPD_DOWN, (void *)key);
    	    break;
    
    	case KPD_UP:
    	
    		//remove vk state for async state requirement    	
    		s_vk_state[key>>5] &= ~(uint32)(0x1<<(key&0x1F));
    	
   	        //SCI_TRACE_LOW:"KPD_SVR: Virtual Key: 0x%x is UP\n"
   	        SCI_TRACE_ID(TRACE_TOOL_CONVERT,KEYPAD_178_112_2_18_1_5_25_45,(uint8*)"d", key);
   	        
#ifdef REF_HANDLE_POWER_KEY
            if((SCI_VK_POWER == key) && (POWER_GetMsStatus() == POWER_MS_STATUS_POWER_OFF))
        	{
        		// Power On MS.
			    POWER_PowerOn();
        	}
            else if(SCI_VK_CALL == key)
	        {
	            KPD_SetGreenKeyPressed(TRUE);
	        }
        	else
#endif  // REF_HANDLE_POWER_KEY
        	{
   		        KPD_SetGreenKeyPressed(FALSE);
        	    SCI_SendEventToClient(KEYPAD_SERVICE, KPD_UP, (void *)key);
        	}
	        break;

        case KPD_SOFT_POWER_ON:
#ifdef REF_HANDLE_POWER_KEY
			// Power On MS.
			POWER_PowerOn();
#endif  // REF_HANDLE_POWER_KEY
			break;

    	case KPD_SOFT_POWER_OFF:
			// Power Off MS.
#ifdef REF_HANDLE_POWER_KEY
			// Send a event to Power OFF MS.
		    SCI_SendEventToClient(KEYPAD_SERVICE, KPD_UP, (void *)SCI_VK_POWER);
#else  // REF_HANDLE_POWER_KEY

            //@lin.liu(2004-03-18). don't call LCD_Clr() now.
            // If LCD_Clear() can't be found, defined in ssd18xx.c or ssd1815.c
		#ifndef WIN32
            LCD_Clear(0, 0x00); // first parameter is MAIN_LCD_ID, next parameter is color
		#endif
            
            //LCD_SetBacklightMode(LCD_BACKLIGHT_OFF); @jim
            POWER_Reset();
#endif  // REF_HANDLE_POWER_KEY
    	    break;
    
    	default:
    		break;
    	}
   		
        SCI_FREE(key_sig_ptr);		
	}
}

#ifndef WIN32

static KPD_SYNC_T *pSyncReadKeyBuf = NULL;

/*****************************************************************************/
//  Description:    This function read key from hardware that using synchronously mode.
//    Args:           
//                       count:   need read keys count.
//                       buf:      Store the value read.. The size of buf need >= count value. 
//                       wait_TicksOption:  if it is 0xffffffff, wait forever. Or when the ticks elapse, the
//                                                  function will return.
//   return value:  return the key count  has been read. 
//   Author:         steve.zhan
//	Note:           
/*****************************************************************************/
PUBLIC uint32 KPD_SyncReadKey(int  count, KPD_SYNC_T buf[], int wait_TicksOption)/*lint -esym(765,KPD_SyncReadKey)*/
{
    volatile int  des_tick = (int)TIMER_GetSystemCounterReg() + wait_TicksOption;

    if (count > 0 && (buf != NULL) )
    {
        KPD_SetKpdReadMode(KPD_SYNC_READ);

        pSyncReadKeyBuf = buf;
        while ((pSyncReadKeyBuf < buf + count) && ((wait_TicksOption - (int)0xffffffff == 0)? TRUE : ((int)TIMER_GetSystemCounterReg() < des_tick)))
        {
            KPD_SyncGetKey();    
        }        
    
        KPD_SetKpdReadMode(KPD_NORMAL_INTERRUPT);

        return (uint32)(pSyncReadKeyBuf - buf);   				
    }
    else
    {
        return 0;
    }

}

/*****************************************************************************/
//  Description:    This function is used to register to TBIOS. It will be
//                  called to handle the message from lower layer.
//   Author:         Xueliang.Wang, Steve.zhan, Support synchronous read mode.
//	Note:           
/*****************************************************************************/
PUBLIC void KPDSVR_Callback(TB_MSG * msg_ptr)/*lint -esym(765, KPDSVR_Callback)*/
{
    KPDSVR_SIG_T    *key_sig_ptr    = SCI_NULL;
    uint32          size            = sizeof(KPDSVR_SIG_T);
   
    if (KPD_GetKpdReadMode() ==  KPD_SYNC_READ)
    {
        uint16          key             = 0;
        key        = (uint16)msg_ptr->wparam;
        
        if (TB_KPD_RELEASED == msg_ptr->message)
        {
            pSyncReadKeyBuf->status  = KPD_UP;
        }
        else if(TB_KPD_PRESSED == msg_ptr->message)
        {		 
            pSyncReadKeyBuf->status = KPD_DOWN;
        }
        
        pSyncReadKeyBuf->key = key;
        pSyncReadKeyBuf ++;	
        return ;
    }
    else
    {
        key_sig_ptr = (KPDSVR_SIG_T *)SCI_ALLOC_APP(size);    
        SCI_ASSERT(SCI_NULL != key_sig_ptr);/*assert verified*/
        /*lint -esym(613, key_sig_ptr) */
        key_sig_ptr->SignalSize = size;
        key_sig_ptr->Sender     = KPDSVR;
        key_sig_ptr->key        = (uint16)msg_ptr->wparam;
        
        // Set signal code.
        if (TB_KPD_RELEASED == msg_ptr->message)
        {
            key_sig_ptr->SignalCode = KPD_UP;
        }
        else if(TB_KPD_PRESSED == msg_ptr->message)   // (KPD_PRESSED == msg_ptr->message)
        {		 
        	key_sig_ptr->SignalCode = KPD_DOWN;
        }
        
        // Send signal to Keypad Server.
        SCI_SendSignal((xSignalHeader)key_sig_ptr, KPDSVR);
    }
}
#endif  // WIN32

/*****************************************************************************/
//  Description:    This function initialize the keypad, register callback 
//                  function to TBIOS, and initialize local variable.
//  Author:         
//	Note:           
/*****************************************************************************/
LOCAL void Init(void)
{
	uint32 i = 0;
#ifdef WIN32    
	KPD_Create(KPDSVR);
#else

    // Register Keypad Server callback function to TBIOS.
    HAL_RegCallback(TB_KPD_INT, KPDSVR_Callback);

    //@lin.liu(2003-11-21).  CR: 4267
    if (RM_GetPinTestFlag())
    {    
        // Don't init kpd. 
        // When UI-test, the keypad is be connected each other.
        // and keypad interrupt will generate again and again.
        return ;        
    }
    // End @Lin.liu

    // Initialize keypad driver.
  	HAL_InitKpd(); 

    // Register Keypad Server callback function to TBIOS.
    HAL_RegCallback(TB_GPIO_INT, KPDSVR_Callback);
    //@lin.liu(2004-02-04). CR7018    

	// Init Virtual Key state for sync keypad state
	for (i=0; i<(KPD_VK_NUM>>5); i++)
	{
		s_vk_state[i] = 0;
	}
	
#endif  // WIN32
}

/*****************************************************************************/
//  Description:    This function check if the MS is ready. If the state of MS
//                  is POWER_MS_STATUS_POWER_ON or POWER_MS_STATUS_POWER_OFF,
//                  it return SCI_TRUE, else return SCI_FALSE.
//  Author:         Xueliang.Wang
//	Note:           
/*****************************************************************************/
LOCAL BOOLEAN IsMsReady(void)
{
    BOOLEAN status = SCI_FALSE;
    POWER_MS_STATUS_E   ms_state;

    // Get the MS current state.
    ms_state = POWER_GetMsStatus();
    
    if ((POWER_MS_STATUS_POWER_ON == ms_state) || (POWER_MS_STATUS_POWER_OFF == ms_state) )
    {
        status = SCI_TRUE;
    }
    else
    {
        status = SCI_FALSE;
    }
    
    return status;
}

/*****************************************************************************/
//  Description:    This function is used to send signal to client. It is a
//                  callback function.
//  Author:         Richard.Yang
//	Note:           
/*****************************************************************************/
LOCAL void KPDSVR_SendKey(BLOCK_ID id, uint32 argc, void *argv)
{
    KPDSVR_SIG_T    *key_sig_ptr    = SCI_NULL;
    uint32          size            = sizeof(KPDSVR_SIG_T);

    key_sig_ptr = (KPDSVR_SIG_T *)SCI_ALLOC_APP(size);
    SCI_ASSERT(SCI_NULL != key_sig_ptr);/*assert verified*/
    key_sig_ptr->SignalSize = (uint16)size;
    key_sig_ptr->Sender     = KPDSVR;
    key_sig_ptr->SignalCode = (uint16)(argc & 0xFFFF);
    key_sig_ptr->key        = (int)argv;

    // Send signal to Keypad Server.
    SCI_SendSignal((xSignalHeader)key_sig_ptr, id);
}

/*****************************************************************************/
//  Description:    This function retrieves the status of the specified virtual key
//                  The status specifies whether the key is up, down.
//  Author:         Jimmy.Jia
//	Note:           
//  Input:			// virtual-key code should less than KPD_VK_NUM
//  Return:			1: down 0: up
/*****************************************************************************/
PUBLIC uint32 KPDSVR_GetKeyState (uint16 nVirtKey)
{
	if (nVirtKey>=KPD_VK_NUM)	
		return 0;
	else
		return (s_vk_state[nVirtKey>>5] & (0x1<<(nVirtKey&0x1F)))? 1 : 0;
}


#ifdef M2M_EVB_SUPPORT
PUBLIC void CUST_RegisterRefService( void )
{
    //Register key event
    SCI_RegisterMsg( KEYPAD_SERVICE, 
        (uint8)(KPD_DOWN & 0xff), 
        (uint8)(KPD_UP & 0xff), 
        SCI_NULL);

}
  
#endif 
/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    }
#endif

