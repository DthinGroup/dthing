/******************************************************************************
 ** File Name:      gpio_hal.c                                                *
 ** Author:         Xueliang.Wang                                             *
 ** DATE:           03/14/2003                                                *
 ** Copyright:      2003 Spreadtrum, Incoporated. All Rights Reserved.         *
 ** Description:    This file defines the basic operation interfaces of   GPIO device. *
 **                                                                                                                                    *
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                       *
 ** ------------------------------------------------------------------------- *
 ** DATE            NAME                DESCRIPTION                           *
 ** 03/14/2003     Xueliang.Wang        Create.                               *
 ** 09/05/2003     Zhemin.Lin           Added functions for SM5100B EVB1.0    *
 ** 12/09/2003     Zhemin.Lin           modified according to CR5719          *
 ** 12/22/2003     Xueliang.Wang        modified according to CR6396          *
 ** 02/27/2004     Zhemin.Lin           modified according to CR7605          *
 ** 19/11/2004     Benjamin.Wang    modified according to CR16760             *
 ** 08/12/2004  Benjamin.Wang       modified according to CR17377             *
 ** 08/01/2008     Yiyue.He         modified the code for more generic adptation.
                                Add new code model for GPI GPO, but the code can adpta
                                to our all old platform without any cost
 ** 07/29/2010    Steve.Zhan cleanup code ,and refactoring some functions.
 ******************************************************************************/

/**---------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **---------------------------------------------------------------------------*/
#include "chip_drv_trc.h"
#include "sci_types.h"
#include "os_api.h"
#include "chip_plf_export.h"
#include "task_monitor.h"
#include "gpio_drvapi.h"
#include "gpio_phy.h"
#include "isr_drvapi.h"
#include "boot_drvapi.h"
//#include "priority_kernel.h"
/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
extern   "C"
{
#endif

extern void GPIO_CustomizeInit (void);

/**---------------------------------------------------------------------------*
 **                         Macro Definition                                                                *
 **---------------------------------------------------------------------------*/
/**---------------------------------------------------------------------------*
 **                         Global Variables                                  *
 **---------------------------------------------------------------------------*/
typedef struct
{
    uint32  message1;           // message type if this gpio is set on(1)
    uint32  param1;             // parameter of the message when this gpio is set on(1)
    uint32  message0;           // message type when this gpio is set off (0)
    uint32  param0;             // parameter of the message when this gpio is set on(0)
    uint32  shaking_interval;
    uint32  shaking_check_count;  //In fact,timer will be changed with MIN_DESHAKING_INTERVAL,shaking_check_count = shaking_interval / MIN_DESHAKING_INTERVAL
    uint32  added_check_count;  //the number of neglectable shaking at beginning if necessary
    SCI_TIMER_PTR timer;        // periodic timer for de shaking
    uint32  gpio_id;            // GPIO ID
    GPIO_CALLBACK  gpio_callback_fun;   // CallBack Function
    BOOLEAN de_shaking;         // if SCI_TRUE, need to de shaking, else needn't
    BOOLEAN state;              // latest stable state
    BOOLEAN is_stable;
    BOOLEAN is_timer_active;    // periodic timer state
    BOOLEAN is_check_mode;      // if SCI_TRUE, mean only a message(pressed or released) is send out when an interrupt happen
    //else there are two message are send out when an interrupt happens
    BOOLEAN is_lisr_irq;        // For CSR: flag to support the ISR in LISR context feature
} GPIO_INT_REC_T;


#define GPIO_MAX_INT_REC_NUM            10
#define DESHAKING_TIME  3

//used to record gpios interrupt information
LOCAL GPIO_INT_REC_T s_gpio_int_table[GPIO_MAX_INT_REC_NUM];
LOCAL SCI_TIMER_PTR s_gpio_zeroint_timer = PNULL;

LOCAL void GpioHisrFunc (uint32 cnt, void *pData);

/*****************************************************************************/
//  Description:    This function used to de shaking. if de pin enter to a stable
//                  state, call DRV_Callback
//  Author:         Zhemin.Lin
//  Note:
/*****************************************************************************/
LOCAL void _GPIO_DeShaking (uint32 int_rec_id);

/*****************************************************************************/
//  Description:    This function initialize gpio_int_table
//  Author:         Zhemin.Lin
//  Note:
/*****************************************************************************/
LOCAL void _GPIO_InitIntTable (void);

/*****************************************************************************/
//  Description:    This function initialize gpio_int_table
//  Author:         Zhemin.Lin
//  Note:
/*****************************************************************************/
LOCAL void _GPIO_InitIntTable (void)
{
    uint32 i;

    for (i = 0; i<GPIO_MAX_INT_REC_NUM; ++i)
    {
        s_gpio_int_table[i].gpio_id = GPIO_INVALID_ID;
    }
}

/**---------------------------------------------------------------------------*
 **                     PUBLIC Function Prototypes                             *
 **---------------------------------------------------------------------------*/
/*****************************************************************************/

/*****************************************************************************/
//  Description:    This function Register gpio interrupt handler.
//  Author:         Steve.Zhan
//  Note:
/*****************************************************************************/
//extern void SENSE_Init(void);
LOCAL uint32 GPIO_RegInt (void)
{
    uint32 intNum = 0;
    uint32  status    = SCI_SUCCESS;
    uint32 size = 0;
    GPIO_INT_T *gpio_int_array = NULL;

    gpio_int_array = Gpio_GetCfgGpioIntTable (&size);

    if (NULL != gpio_int_array && size > 0)
    {
        for (intNum = 0; intNum < size; ++intNum)
        {
            status |= ISR_RegHandler_Ex (gpio_int_array[intNum].int_logic_num,
                                         (TB_ISR) (gpio_int_array[intNum].isr_func), GpioHisrFunc, CHIPDRV_HISR_PRIO_2, NULL); /*lint !e611*/
        }

        if (TB_SUCCESS == status)
        {
            GPIO_EnableInt();
        }

    }

    return 0;
}
/*****************************************************************************/
//  Description:    This function initialize GPIO function.
//  Author:         Xueliang.Wang
//  Note:
/*****************************************************************************/
PUBLIC uint32 GPIO_Init (void)
{
    uint32  status    = SCI_SUCCESS;
    uint32 gpio_id;
    uint32 i;
    GPIO_INFO_T gpio_info;

    //GPIO_PRINT:"GPIO_Init: Init GPIO !"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_HAL_147_112_2_17_23_4_42_980,(uint8*)"");

    _GPIO_InitIntTable();

    GPIO_RegInt();



    GPIO_CustomizeInit();

    // Watchdog mode is set only in case of Watchdog reset mode and Debug
    // assert mode, because in this mode the RTOS components are not in-
    // itialized yet, calling them may cause error. Other cases(such as
    // normal power-on, watchdog reset in release assert mode) can enter
    // the branch.
    if (GetWatchdogMode() != SCI_TRUE)
    {
        s_gpio_zeroint_timer = SCI_CreatePeriodTimer ("GPIO_ZEROINT_TIMER",
                               GPIO_ZeroIntDeShaking, 0, DESHAKING_TIME, SCI_NO_ACTIVATE);
    }


    for (gpio_id = 0; gpio_id < GPIO_GetMaxNumber(); ++gpio_id)
    {
        GPIO_PHY_GetBaseInfo (gpio_id, &gpio_info);

        if ( (gpio_info.gpio_type != GPIO_SECTION_GPI) && (gpio_info.gpio_type != GPIO_SECTION_GPIO))
        {
            continue;
        }

        if (GPIO_PHY_GetIntIsMask (&gpio_info))
        {
            for (i = 0; i < GPIO_MAX_INT_REC_NUM; ++i)
            {
                if (s_gpio_int_table[i].gpio_id == gpio_id)
                {
                    break;
                }
            }

            if (i == GPIO_MAX_INT_REC_NUM)
            {
                //GPIO_PRINT:"GPIO_DRIVER: GPIO_%d interrupt has not been registed!\n"
                SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_HAL_190_112_2_17_23_4_42_981,(uint8*)"d", gpio_id);
                GPIO_PHY_CleanIntMask (&gpio_info);
            }
        }
    }

    //SENSE_Init();
    return status;
}

/*****************************************************************************/
//  Description:    This function used to set the value of a gpio pin
//  Author:           Lin.Liu
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_SetValue (uint32 gpio_id, BOOLEAN b_on)
{
    GPIO_INFO_T gpio_info;

    SCI_ASSERT (gpio_id < GPIO_GetMaxNumber());/*assert verified*/

    //SCI_TRACE_LOW("===>>gpio_id:%d \n",gpio_id);

    GPIO_PHY_GetBaseInfo (gpio_id, &gpio_info);

    //SCI_TRACE_LOW("===>>GPio info:addr(0x%x),bitNum(%d),type(%d)\n",gpio_info.baseAddr,gpio_info.bit_num,gpio_info.gpio_type);

    GPIO_PHY_SetDataMask(&gpio_info,TRUE);
    if (!GPIO_PHY_GetDataMask (&gpio_info))
    {
        SCI_PASSERT (0, ("[GPIO_DRV]GPIO_%d data mask no opened!",  gpio_id));/*assert verified*/
    }

    GPIO_PHY_SetDirection (&gpio_info, TRUE);
    if (!GPIO_PHY_GetDirection (&gpio_info))
    {
        SCI_PASSERT (0, ("[GPIO_DRV]GPIO_%d dir wrong!",     gpio_id));/*assert verified*/
    }

    GPIO_PHY_SetPinData (&gpio_info, b_on);
}

/*****************************************************************************/
//  Description:    This function used to get the value of a gpio pin
//  Author:           Lin.Liu
//  Note:
/*****************************************************************************/
PUBLIC BOOLEAN  GPIO_GetValue (uint32  gpio_id)
{
    GPIO_INFO_T gpio_info;
    GPIO_PHY_GetBaseInfo (gpio_id, &gpio_info);

    if (!GPIO_PHY_GetDataMask (&gpio_info))
    {
        SCI_PASSERT (0, ("[GPIO_DRV]GPIO_GetValue: GPIO_%d data mask hasn't been opened!\n", gpio_id));/*assert to do*/
    }

    if (GPIO_PHY_GetDirection (&gpio_info))
    {
        SCI_PASSERT (0, ("[GPIO_DRV]GPIO_GetValue: GPIO_%d should be input port!\n", gpio_id));/*assert to do*/
    }

    return GPIO_PHY_GetPinData (&gpio_info);
}

/*****************************************************************************/
//  Description:    This function used to get the direction of a gpio pin
//  Note:              0 - Input
//                     1 - Output
/*****************************************************************************/
PUBLIC BOOLEAN GPIO_GetDirection (uint32  gpio_id)
{
    GPIO_INFO_T gpio_info;
    GPIO_PHY_GetBaseInfo (gpio_id, &gpio_info);

    return GPIO_PHY_GetDirection (&gpio_info);
}

/*****************************************************************************/
//  Description:    This function used to set the direction of a gpio pin
//  Note:              0 - Input
//                        1 - Output
/*****************************************************************************/
PUBLIC void GPIO_SetDirection (uint32  gpio_id, BOOLEAN directions)
{

    GPIO_INFO_T gpio_info;

    GPIO_PHY_GetBaseInfo (gpio_id, &gpio_info);

    GPIO_PHY_SetDirection (&gpio_info, directions);
}


/********************************************************************************/
//  Description:This function query the state( after interrrupt) of (GPIO and GPI)
//  Author:         Zhemin.Lin
//  Retread by :  Steve.Zhan
//  Note: No Using????
/********************************************************************************/
PUBLIC BOOLEAN GPIO_GetGPIOState (uint32 gpio_id)
{
    uint16 i = 0;
    GPIO_INFO_T gpio_info;

    for (i = 0; i < GPIO_MAX_REC_NUM; ++i)
    {
        if (s_gpio_int_table[i].gpio_id == gpio_id)
        {
            return (s_gpio_int_table[i].state);
        }
        else if (s_gpio_int_table[i].gpio_id == GPIO_INVALID_ID)
        {
            continue;
        }
    }

    GPIO_PHY_GetBaseInfo (gpio_id, &gpio_info);

    if (GPIO_PHY_GetDirection (&gpio_info))
    {
        SCI_ASSERT (0);/*assert to do*/
        return SCI_FALSE; /*lint !e527 comfirmed by xuepeng*/
    }

    if ( (gpio_info.gpio_type != GPIO_SECTION_GPI) && (gpio_info.gpio_type != GPIO_SECTION_GPIO))
    {
        SCI_ASSERT (0);/*assert to do*/
        return SCI_FALSE; /*lint !e527 comfirmed by xuepeng*/
    }

    ////SCI_TRACE_LOW:"GPIO_GetGPIOState UnRegister interrupt for this pin."
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_HAL_304_112_2_17_23_4_42_982,(uint8*)"");

    return GPIO_PHY_GetPinData (&gpio_info);
}


/*****************************************************************************/
//  Description:    This function used to know whether the gpio port is enabled.
//  Author:           Yiyue.He
//  notes:
/*****************************************************************************/
PUBLIC BOOLEAN GPIO_GetDataMask (uint32 gpio_id)
{
    GPIO_INFO_T gpio_info;
    GPIO_PHY_GetBaseInfo (gpio_id, &gpio_info);

    return GPIO_PHY_GetDataMask (&gpio_info);
}
/*****************************************************************************/
//  Description:    This function used to set the gpio port to enabled or not
//  Author:           Yiyue.He
//  notes:
/*****************************************************************************/
PUBLIC void GPIO_SetDataMask (uint32 gpio_id, BOOLEAN b_on)
{
    GPIO_INFO_T gpio_info;
    GPIO_PHY_GetBaseInfo (gpio_id, &gpio_info);

    GPIO_PHY_SetDataMask (&gpio_info, b_on);
    return ;
}
/*****************************************************************************/
//  Description:    This function used to enable data w/r mask the given gpio port.
//  Author:           Benjamin.Wang
// retread by :     Yiyue.He
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_Enable (uint32 gpio_id)
{
    GPIO_INFO_T gpio_info;
    GPIO_PHY_GetBaseInfo (gpio_id, &gpio_info);

    GPIO_PHY_SetDataMask (&gpio_info, SCI_TRUE);

    //SCI_TRACE_LOW("===>>gpio_type:%d \n",gpio_info.gpio_type);
}

/*****************************************************************************/
//  Description:    This function used to disable the given gpio port.
//  Author:           Benjamin.Wang
// retread by :     Yiyue.He
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_Disable (uint32 gpio_id)
{
    GPIO_INFO_T gpio_info;
    GPIO_PHY_GetBaseInfo (gpio_id, &gpio_info);

    GPIO_PHY_SetDataMask (&gpio_info, SCI_FALSE);
}

/*****************************************************************************/
//  Description:    This function retrun the max number of gpio
//  Author:         Zhemin.Lin
//  Note:
/*****************************************************************************/
PUBLIC uint32 GPIO_GetMaxNumber (void)
{
    return GPIO_PHY_GetMaxNumber();
}

/*****************************************************************************/
//  Description:    This function used to enable gpio IRQ Interrupt.
//  Author:           Benjamin.Wang
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_EnableInt (void)
{
    CHIPDRV_EnableIRQINT (TB_GPIO_INT);
}

/*****************************************************************************/
//  Description:    This function used to disable gpio IRQ Interrupt.
//  Author:           Benjamin.Wang
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_DisableInt (void)
{
    CHIPDRV_DisableIRQINT (TB_GPIO_INT);
}

/*****************************************************************************/
//  Description:    This function used to enable the given gpio port's interrupt.
//  Author:           Benjamin.Wang
// retread by:        Yiyue.He
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_EnableIntCtl (uint32 gpio_id)
{
    GPIO_INFO_T gpio_info;

    GPIO_PHY_GetBaseInfo (gpio_id, &gpio_info);
    GPIO_PHY_SetIntMask (&gpio_info);
}

/*****************************************************************************/
//  Description:    This function used to disable the given gpio port's interrupt.
//  Author:           Benjamin.Wang
// retread by:        Yiyue.He
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_DisableIntCtl (uint32 gpio_id)
{
    GPIO_INFO_T gpio_info;

    GPIO_PHY_GetBaseInfo (gpio_id, &gpio_info);
    GPIO_PHY_CleanIntMask (&gpio_info);
}





/*****************************************************************************/
//  Description:    This function used to set gpio Interrupt sense type.
//  Author:         Benjamin.Wang
//  Retreat by:     Steve.Zhan
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_SetInterruptSense (uint32 gpio_id, GPIO_INT_TYPE sensetype)
{
    GPIO_INFO_T gpio_info;

    GPIO_PHY_GetBaseInfo (gpio_id, &gpio_info);

    GPIO_PHY_SetInterruptSense (&gpio_info, sensetype);

}
/*****************************************************************************/
//  Description:    This function used to get the intr state of a gpio pin
//  Author:         Zhemin.Lin
//  retread by:    Yiyue.He
//  Retreat by:     Steve.Zhan
//  Note:
/*****************************************************************************/
PUBLIC BOOLEAN GPIO_GetIntState (uint32 gpio_id)
{
    GPIO_INFO_T gpio_info;

    GPIO_PHY_GetBaseInfo (gpio_id, &gpio_info);
    return GPIO_PHY_GetIntState (&gpio_info);
}


/*****************************************************************************/
//  Description:    This function used to clear the given interrupt status bit.
//  Author:           Benjamin.Wang
// retread by:        Yiyue.He
//  Retreat by:     Steve.Zhan
//  Note:
/*****************************************************************************/
PUBLIC void GPIO_ClearIntStatus (uint32 gpio_id)
{
    GPIO_INFO_T gpio_info;
    GPIO_PHY_GetBaseInfo (gpio_id, &gpio_info);

    GPIO_PHY_ClearIntStatus (&gpio_info);
}


/**---------------------------------------------------------------------------*
 **                         Interrupt Process of  GPIO                        *
 **---------------------------------------------------------------------------*/
/*****************************************************************************/
//  Description:    This function used to Deshaking 0 interrupt
//  Author:         Younger
//  Note:
/*****************************************************************************/
void GPIO_ZeroIntDeShaking (uint32 state)
{
    SCI_ASSERT (NULL != s_gpio_zeroint_timer);/*assert verified*/

    SCI_DeactiveTimer (s_gpio_zeroint_timer);
    GPIO_EnableInt();
}

/*****************************************************************************/
//  Description:    This function use timer to fix 0 interrpt issue
//  Author:         Zhemin.Lin
//  Note:
/*****************************************************************************/
void GPIO_ZeroIntHandler (uint32 int_num)
{
    SCI_ASSERT (NULL != s_gpio_zeroint_timer);/*assert verified*/

    if (!SCI_IsTimerActive (s_gpio_zeroint_timer))
    {
        SCI_ChangeTimer (s_gpio_zeroint_timer, GPIO_ZeroIntDeShaking, DESHAKING_TIME);
        SCI_ActiveTimer (s_gpio_zeroint_timer);
    }

    TM_SendTestPointRequest (0xdead0000, 0);
    GPIO_DisableInt();
}

/* For CSR: Added function to request GPIO interrupts be set in LISR context. If requesting
   GPIO interrupts in LISR context, it becomes the responsibility of the registered
   ISR to handle any switch to HISR context. Note that the responsibility for unmasking
   the interrupt through GPIO_EnableIntCtl() is now also the responsibility of the module
   owning the ISR.
   This function is necessary in order to enable full level-triggered interrupt support,
   which is needed by CSR SPI.
*/
PUBLIC void GPIO_SetLisrInt(uint32 gpio_id, BOOLEAN is_lisr_irq)
{
    uint32 i;
    for (i = 0; i < GPIO_MAX_INT_REC_NUM; ++i)
    {
        if (s_gpio_int_table[i].gpio_id == gpio_id)
        {
            s_gpio_int_table[i].is_lisr_irq = is_lisr_irq;
        }
    }
}

typedef struct
{
    uint32 gpio_id;
    uint32 gpio_ext_data;
} hisr_data;

INPUT_BUFFER_INIT (hisr_data, GPIO_MAX_INT_REC_NUM)

/*****************************************************************************/
//  Description:    This function be called by interrupt handler when is
//                  GPIO interrupt.
//  Author:         Xueliang.Wang
//  Note:   PUBLIC this function  for gpio config file only.
/*****************************************************************************/
#if defined(CHIP_VER_6530)
PUBLIC ISR_EXE_T EICD_DbncIsr( uint32 data );
#endif

PUBLIC ISR_EXE_T GPIO_Handler (uint32 int_num)
{
    uint32 i;
    //BOOLEAN pin_state;
    GPIO_INFO_T gpio_info;
    hisr_data data;

    for (i = 0; i < GPIO_MAX_INT_REC_NUM; ++i)
    {
        if (s_gpio_int_table[i].gpio_id == (uint32) GPIO_INVALID_ID)
        {
            continue;
        }

        GPIO_PHY_GetBaseInfo (s_gpio_int_table[i].gpio_id, &gpio_info);

        if (GPIO_PHY_GetIntState (&gpio_info))
        {
            TM_SendTestPointRequest (0xdeadbeef, s_gpio_int_table[i].gpio_id);
            GPIO_PHY_ClearIntStatus (&gpio_info);

            GPIO_DisableIntCtl (s_gpio_int_table[i].gpio_id);

            if (s_gpio_int_table[i].de_shaking) //need soft deshaking
            {
                if (SCI_TRUE != s_gpio_int_table[i].is_timer_active)
                {
                    //cr67797.We tend to think waveshapes changed stably.
                    s_gpio_int_table[i].is_stable = SCI_TRUE;
                    //calculate checking times
                    s_gpio_int_table[i].shaking_check_count = 0;
                    s_gpio_int_table[i].added_check_count = 0;

                    if (s_gpio_int_table[i].shaking_interval > (uint32) MIN_DESHAKING_INTERVAL)
                    {
                        s_gpio_int_table[i].shaking_check_count =
                            s_gpio_int_table[i].shaking_interval/ (uint32) MIN_DESHAKING_INTERVAL;
                        //additional checking times if necessary
                        s_gpio_int_table[i].added_check_count = s_gpio_int_table[i].shaking_check_count;
                        s_gpio_int_table[i].shaking_check_count--;
                    }

                    //need de shaking
                    //SCI_ChangeTimer(s_gpio_int_table[i].timer, GPIO_DeShaking, s_gpio_int_table[i].shaking_interval);
                    SCI_ChangeTimer (s_gpio_int_table[i].timer, _GPIO_DeShaking, MIN_DESHAKING_INTERVAL);
                    SCI_ActiveTimer (s_gpio_int_table[i].timer);
                    s_gpio_int_table[i].is_timer_active = SCI_TRUE;
                }
            }
            else                            //don't need soft deshaking
            {
                // For CSR: Check flag for LISR ISR feature
                if (s_gpio_int_table[i].is_lisr_irq)
                {
                    (* s_gpio_int_table[i].gpio_callback_fun) (i, GPIO_PHY_GetPinData (&gpio_info));
                    return ISR_DONE;
                }
                else
                {
                    data.gpio_id = i;
                    data.gpio_ext_data = GPIO_PHY_GetPinData (&gpio_info);
                    IsrWriteBuffer (data);
                    return CALL_HISR;
                }
            }

        }

    }

#if defined(CHIP_VER_6530)
    EICD_DbncIsr( i );
#endif

    return ISR_DONE;
}

LOCAL void GpioHisrFunc (uint32 cnt, void *pData)
{
    BOOLEAN pin_state;
    TB_MSG  gpio_msg;
    GPIO_INFO_T gpio_info;
    uint32  i = 0;
    hisr_data data;

    while (!threadReadBuffer ( (hisr_data *) (&data)))
    {
        uint32 gpio_id = 0;

        i = data.gpio_id;
        gpio_id = s_gpio_int_table[i].gpio_id;

        if ((i >=GPIO_MAX_REC_NUM) || (gpio_id >= GPIO_GetMaxNumber()) ||(gpio_id == GPIO_INVALID_ID))
        {
            //Avoid gpio interrupt is canceled by user.
            continue;

        }

        GPIO_PHY_GetBaseInfo (gpio_id, &gpio_info);
        pin_state = data.gpio_ext_data;
        s_gpio_int_table[i].state = pin_state;

        if (NULL != s_gpio_int_table[i].gpio_callback_fun)
        {
            (* s_gpio_int_table[i].gpio_callback_fun) (gpio_id, pin_state);
        }
        else
        {
            //set msg
            if (s_gpio_int_table[i].is_check_mode)
            {
                // if is checked mode, only send a message
                if (pin_state)
                {
                    gpio_msg.message = s_gpio_int_table[i].message1;
                    gpio_msg.wparam  = s_gpio_int_table[i].param1;
                    gpio_msg.lparam  = TB_NULL;
                }
                else
                {
                    gpio_msg.message = s_gpio_int_table[i].message0;
                    gpio_msg.wparam  = s_gpio_int_table[i].param0;
                    gpio_msg.lparam  = TB_NULL;
                }

                ISR_Callback (TB_GPIO_INT,&gpio_msg);
            }
            else
            {
                //need to send two message
                if (pin_state)
                {
                    gpio_msg.message = s_gpio_int_table[i].message1;
                    gpio_msg.wparam  = s_gpio_int_table[i].param1;
                    gpio_msg.lparam  = TB_NULL;
                    ISR_Callback (TB_GPIO_INT,&gpio_msg);

                    gpio_msg.message = s_gpio_int_table[i].message0;
                    gpio_msg.wparam  = s_gpio_int_table[i].param0;
                    gpio_msg.lparam  = TB_NULL;
                    ISR_Callback (TB_GPIO_INT,&gpio_msg);
                }
                else
                {
                    gpio_msg.message = s_gpio_int_table[i].message0;
                    gpio_msg.wparam  = s_gpio_int_table[i].param0;
                    gpio_msg.lparam  = TB_NULL;
                    ISR_Callback (TB_GPIO_INT,&gpio_msg);

                    gpio_msg.message = s_gpio_int_table[i].message1;
                    gpio_msg.wparam  = s_gpio_int_table[i].param1;
                    gpio_msg.lparam  = TB_NULL;
                    ISR_Callback (TB_GPIO_INT,&gpio_msg);
                }
            }
        }

        GPIO_PHY_EnableGPIDetect (&gpio_info);
        GPIO_EnableIntCtl (gpio_id);

    }

}


/*****************************************************************************/
//  Description:    This function add an callback function to gpio_int_table
//  Author:
//  Retread by: Yiyue.He
//  Note:            This function and GPIO_AddToIntTable() can't be called simultaneously.
/*****************************************************************************/
PUBLIC uint32 GPIO_AddCallbackToIntTable (
    uint32  gpio_id,            // GPIO ID
    BOOLEAN de_shaking,         // if SCI_TRUE, need de shaking, else needn't
    uint32  shaking_interval,   // interval for de shaking
    GPIO_CALLBACK gpio_callback_fun         // Callback function
)
{
    uint32 i;
    GPIO_INFO_T gpio_info;
    GPIO_PHY_GetBaseInfo (gpio_id, &gpio_info);

    if (GPIO_PHY_GetDirection (&gpio_info) && (gpio_info.gpio_type == GPIO_SECTION_GPIO))
    {
        return SCI_ERROR;
    }

    if ( (gpio_info.gpio_type != GPIO_SECTION_GPI) && (gpio_info.gpio_type != GPIO_SECTION_GPIO))
    {
        return SCI_ERROR;
    }

    for (i = 0; i < GPIO_MAX_INT_REC_NUM; ++i)
    {
        if (s_gpio_int_table[i].gpio_id == gpio_id)
        {
            //GPIO_PRINT:"GPIO_AddToIntTable: GPIO_%d has been added !\n"
            SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_HAL_708_112_2_17_23_4_43_983,(uint8*)"d", gpio_id);
            SCI_ASSERT (0);/*assert to do*/
            return SCI_ERROR; /*lint !e527 comfirmed by xuepeng*/
        }
    }

    for (i = 0; i < GPIO_MAX_INT_REC_NUM; ++i)
    {
        if (s_gpio_int_table[i].gpio_id == (uint32) GPIO_INVALID_ID)
        {
            break;
        }
    }

    if (i >= GPIO_MAX_INT_REC_NUM)
    {
        return SCI_ERROR;
    }

    // Set fields of this record
    s_gpio_int_table[i].gpio_id             = gpio_id;
    s_gpio_int_table[i].is_check_mode       = 0;
    s_gpio_int_table[i].de_shaking          = de_shaking;
    s_gpio_int_table[i].shaking_interval        = shaking_interval;
    s_gpio_int_table[i].message1            = 0;
    s_gpio_int_table[i].param1          = 0;
    s_gpio_int_table[i].message0            = 0;
    s_gpio_int_table[i].param0          = 0;
    s_gpio_int_table[i].gpio_callback_fun   = gpio_callback_fun;
    s_gpio_int_table[i].is_lisr_irq = 0; // For CSR: Initialize flag for ISR in LISR feature

    //now gpio_drv layer check if need soft_deshaking
    //in this step , the meaning of de_shaking is changed to if need soft deshaking
    if (s_gpio_int_table[i].de_shaking == SCI_TRUE)
    {
        if (gpio_info.gpio_type == GPIO_SECTION_GPI) //if GPI ,should modify the shaking_interval &s_gpio_int_table[i].de_shaking
        {
            if (shaking_interval <= (uint32) GPI_MAX_DEBOUCE_INTERVAL)
            {
                s_gpio_int_table[i].de_shaking = SCI_FALSE;
                GPIO_PHY_SetGPIDebounce (&gpio_info, (shaking_interval>= (uint32) GPI_DEBOUNCE_PERIED) ? (uint32) GPI_DEBOUNCE_PERIED:shaking_interval);
            }
            else
            {
                //GPIO_PRINT:"[GPIO_DRV]GPIO_AddToIntTable: We strongly suggest that the deshaking for GPI should be less than 256!,otherwise refer to GPIO\n"
                SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_HAL_752_112_2_17_23_4_43_984,(uint8*)"");
                shaking_interval     -= (uint32) GPI_DEBOUNCE_PERIED;
            }
        }
    }
    else
    {
        if (gpio_info.gpio_type == GPIO_SECTION_GPI)
        {
            GPIO_PHY_SetGPIDebounce (&gpio_info,0);
        }
    }

    //if need soft deshaking, create the periodic timer
    if (s_gpio_int_table[i].de_shaking == SCI_TRUE)
    {
        // Watchdog mode is set only in case of Watchdog reset mode and Debug
        // assert mode, because in this mode the RTOS components are not in-
        // itialized yet, calling them may cause error. Other cases(such as
        // normal power-on, watchdog reset in release assert mode) can enter
        // the branch.
        if (GetWatchdogMode() != SCI_TRUE)
        {
            s_gpio_int_table[i].timer = SCI_CreatePeriodTimer ("GPIO_Shaking_Timer",
                                        _GPIO_DeShaking, i, shaking_interval, SCI_NO_ACTIVATE);
        }

        s_gpio_int_table[i].is_timer_active = SCI_FALSE;
    }

    // set initial state
    s_gpio_int_table[i].state = GPIO_PHY_GetPinData (&gpio_info);

    GPIO_EnableIntCtl (gpio_id);
    GPIO_PHY_TrigGPIDetect (&gpio_info);

    return SCI_SUCCESS;
}


//******************the func below are referenced rarely**********************

/*****************************************************************************/
//  Description:    This function deletes an int from gpio_int_table
//  Author:           Benjamin.Wang
//  Retread by: Yiyue.He
//  Note:           If the GPIO INT is triggered by LEVEL, you have to delete it from int table in
//                   interrupt callback functions. Otherwise, the interrupt will be triggered ceaselessly.
/*****************************************************************************/
PUBLIC uint32 GPIO_DeleteFromIntTable (uint32 gpio_id)
{
    uint32 i = 0;
    GPIO_INFO_T gpio_info;
    GPIO_PHY_GetBaseInfo (gpio_id, &gpio_info);

    if (GPIO_PHY_GetDirection (&gpio_info) && (gpio_info.gpio_type == GPIO_SECTION_GPIO))
    {
        return SCI_ERROR;
    }

    if ( (gpio_info.gpio_type != GPIO_SECTION_GPI) && (gpio_info.gpio_type != GPIO_SECTION_GPIO))
    {
        return SCI_ERROR;
    }

    for (i = 0; i < GPIO_MAX_INT_REC_NUM; ++i)
    {
        if (s_gpio_int_table[i].gpio_id == gpio_id)
        {
            SCI_DisableIRQ();

            if (s_gpio_int_table[i].de_shaking)
            {
                if (SCI_SUCCESS == SCI_DeleteTimer (s_gpio_int_table[i].timer))
                {
                    s_gpio_int_table[i].timer = NULL;
                }
                else
                {
                    //GPIO_PRINT:"Can't delete deshaking timer of GPIO_%d!\n"
                    SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_HAL_829_112_2_17_23_4_43_985,(uint8*)"d", gpio_id);
                    SCI_ASSERT (0);/*assert to do*/
                }
            }

            s_gpio_int_table[i].gpio_id = GPIO_INVALID_ID;
            GPIO_DisableIntCtl (gpio_id);
            SCI_RestoreIRQ();
            break;
        }
    }

    if (i >= GPIO_MAX_INT_REC_NUM)
    {
        GPIO_PRINT ( ("Can't find the given GPIO_%d!\n", gpio_id)); //but we think this is normal.
    }

    return SCI_SUCCESS;

}

PUBLIC void GPIO_SetInitState (uint32  gpio_id, uint32 state)
{
    uint32 i = 0;
    GPIO_INFO_T gpio_info;

    GPIO_PHY_GetBaseInfo (gpio_id, &gpio_info);

    if (GPIO_PHY_GetDirection (&gpio_info) && (gpio_info.gpio_type == GPIO_SECTION_GPIO))
    {
        return ;
    }

    if ( (gpio_info.gpio_type != GPIO_SECTION_GPI) && (gpio_info.gpio_type != GPIO_SECTION_GPIO))
    {
        return;
    }

    for (i = 0; i < GPIO_MAX_INT_REC_NUM; ++i)
    {
        if (s_gpio_int_table[i].gpio_id == gpio_id)
        {
            s_gpio_int_table[i].state = state;
            break;
        }
        else if (s_gpio_int_table[i].gpio_id == (uint32) GPIO_INVALID_ID)
        {
            ////SCI_TRACE_LOW:"GPIO_SetInitState ERROR gpio_id=%d,i=%d"
            SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_HAL_875_112_2_17_23_4_44_986,(uint8*)"dd",gpio_id,i);
        }
    }
}

PUBLIC uint32 GPIO_GetInitState (uint32  gpio_id)
{
    uint32 i;
    GPIO_INFO_T gpio_info;

    GPIO_PHY_GetBaseInfo (gpio_id, &gpio_info);

    if (GPIO_PHY_GetDirection (&gpio_info) && (gpio_info.gpio_type == GPIO_SECTION_GPIO))
    {
        return SCI_ERROR;
    }

    if ( (gpio_info.gpio_type != GPIO_SECTION_GPI) && (gpio_info.gpio_type != GPIO_SECTION_GPIO))
    {
        return SCI_ERROR;
    }

    for (i = 0; i < GPIO_MAX_INT_REC_NUM; ++i)
    {
        if (s_gpio_int_table[i].gpio_id == gpio_id)
        {
            return (s_gpio_int_table[i].state);
        }
        else if (s_gpio_int_table[i].gpio_id == (uint32) GPIO_INVALID_ID)
        {
            ////SCI_TRACE_LOW:"GPIO_GetInitState ERROR gpio_id=%d,i=%d"
            SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_HAL_905_112_2_17_23_4_44_987,(uint8*)"dd",gpio_id,i);
        }
    }

    return SCI_SUCCESS;

}

/*****************************************************************************/
//  Description:    This function used to de shaking. if de pin enter to a stable
//                  state, call DRV_Callback
//  Author:         Zhemin.Lin
//  Note:           Flip_On pin is GPIO27
//  Modified:
//  frank.yang      2007.01.12  enhance the de-shaking mechanism
//  Check pin state per MIN_DESHAKING_INTERVAL-10ms,until shaking_interval is expired.
//  We will ignore the beginning shaking if necessary.But the number of total checking
//  times is not larger than (normal checking times)*2.
//  Strictly,we regard it stable only when current pin state differs from initial pin state
//  in continous normal checking times.
/*****************************************************************************/
LOCAL void _GPIO_DeShaking (uint32 int_rec_id)
{
    TB_MSG  gpio_msg;
    BOOLEAN pin_state;
    uint32 normal_check_times = 0;
    uint32 temp;
    GPIO_INFO_T gpio_info;

    GPIO_PHY_GetBaseInfo (s_gpio_int_table[int_rec_id].gpio_id, &gpio_info);

    //cr67797,enhance the deshaking mechanism
    if (s_gpio_int_table[int_rec_id].shaking_check_count != 0)
    {
        pin_state = GPIO_PHY_GetPinData (&gpio_info);

        if (pin_state == s_gpio_int_table[int_rec_id].state)
        {
            s_gpio_int_table[int_rec_id].is_stable = SCI_FALSE;

            if (s_gpio_int_table[int_rec_id].added_check_count > 0)
            {
                s_gpio_int_table[int_rec_id].added_check_count--;
                //re calculate check times
                normal_check_times = s_gpio_int_table[int_rec_id].shaking_interval/ (uint32) MIN_DESHAKING_INTERVAL;
                temp = s_gpio_int_table[int_rec_id].added_check_count + s_gpio_int_table[int_rec_id].shaking_check_count;

                if (temp >= normal_check_times)
                {
                    s_gpio_int_table[int_rec_id].is_stable = SCI_TRUE;  //restart the checking procedure
                    s_gpio_int_table[int_rec_id].shaking_check_count = normal_check_times;
                    s_gpio_int_table[int_rec_id].added_check_count = temp - normal_check_times;
                }
                else
                {
                    //Currently,the number of total checking times is less than normal checking times.
                    //We think it a shaking and quit the checking procedure.
                    s_gpio_int_table[int_rec_id].shaking_check_count = 0;
                    s_gpio_int_table[int_rec_id].added_check_count = 0;
                }
            }
            else
            {
                s_gpio_int_table[int_rec_id].is_stable = SCI_TRUE;
                s_gpio_int_table[int_rec_id].shaking_check_count--;
            }
        }
        else
        {
            s_gpio_int_table[int_rec_id].shaking_check_count--;
        }
    }
    else
    {
        //deactive timer
        SCI_DeactiveTimer (s_gpio_int_table[int_rec_id].timer);
        s_gpio_int_table[int_rec_id].is_timer_active = SCI_FALSE;

        //get pin state
        pin_state = GPIO_PHY_GetPinData (&gpio_info);

        //pin is not in a stable state
        if ( (pin_state == s_gpio_int_table[int_rec_id].state)
                || (SCI_FALSE == s_gpio_int_table[int_rec_id].is_stable)
           )
        {
            ////SCI_TRACE_LOW:"GPIO_DRIVER: GPIO_%d is shaking(INVALID)!\n"
            SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_HAL_989_112_2_17_23_4_44_988,(uint8*)"d", s_gpio_int_table[int_rec_id].gpio_id);
            GPIO_EnableIntCtl (s_gpio_int_table[int_rec_id].gpio_id);
            GPIO_PHY_TrigGPIDetect (&gpio_info);
            return;
        }

        //update pin state
        s_gpio_int_table[int_rec_id].state = pin_state;
        ////SCI_TRACE_LOW:"GPIO_DRIVER: GPIO_%d state is %d!\n"
        SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_HAL_997_112_2_17_23_4_44_989,(uint8*)"dd",s_gpio_int_table[int_rec_id].gpio_id, pin_state);

        if (NULL != s_gpio_int_table[int_rec_id].gpio_callback_fun)
        {
            (* s_gpio_int_table[int_rec_id].gpio_callback_fun) (s_gpio_int_table[int_rec_id].gpio_id, pin_state);
        }
        else
        {
            if (s_gpio_int_table[int_rec_id].is_check_mode)
            {
                // if is checked mode, only send a message
                if (pin_state)
                {
                    gpio_msg.message = s_gpio_int_table[int_rec_id].message1;
                    gpio_msg.wparam  = s_gpio_int_table[int_rec_id].param1;
                    gpio_msg.lparam  = TB_NULL;
                }
                else
                {
                    gpio_msg.message = s_gpio_int_table[int_rec_id].message0;
                    gpio_msg.wparam  = s_gpio_int_table[int_rec_id].param0;
                    gpio_msg.lparam  = TB_NULL;
                }

                ISR_Callback (TB_GPIO_INT,&gpio_msg);
            }
            else
            {
                //need to send two message
                if (pin_state)
                {
                    gpio_msg.message = s_gpio_int_table[int_rec_id].message1;
                    gpio_msg.wparam  = s_gpio_int_table[int_rec_id].param1;
                    gpio_msg.lparam  = TB_NULL;
                    ISR_Callback (TB_GPIO_INT,&gpio_msg);

                    gpio_msg.message = s_gpio_int_table[int_rec_id].message0;
                    gpio_msg.wparam  = s_gpio_int_table[int_rec_id].param0;
                    gpio_msg.lparam  = TB_NULL;
                    ISR_Callback (TB_GPIO_INT,&gpio_msg);
                }
                else
                {
                    gpio_msg.message = s_gpio_int_table[int_rec_id].message0;
                    gpio_msg.wparam  = s_gpio_int_table[int_rec_id].param0;
                    gpio_msg.lparam  = TB_NULL;
                    ISR_Callback (TB_GPIO_INT,&gpio_msg);

                    gpio_msg.message = s_gpio_int_table[int_rec_id].message1;
                    gpio_msg.wparam  = s_gpio_int_table[int_rec_id].param1;
                    gpio_msg.lparam  = TB_NULL;
                    ISR_Callback (TB_GPIO_INT,&gpio_msg);
                }
            }
        }

        GPIO_EnableIntCtl (s_gpio_int_table[int_rec_id].gpio_id);
        GPIO_PHY_TrigGPIDetect (&gpio_info);
    }
}
PUBLIC uint32 GPIO_GetShakeInterval (uint32 gpio_id)
{
    uint32 i;
    GPIO_INFO_T gpio_info;

    GPIO_PHY_GetBaseInfo (gpio_id, &gpio_info);

    if (GPIO_PHY_GetDirection (&gpio_info) && (gpio_info.gpio_type == GPIO_SECTION_GPIO))
    {
        return SCI_ERROR;
    }

    if ( (gpio_info.gpio_type != GPIO_SECTION_GPI) && (gpio_info.gpio_type != GPIO_SECTION_GPIO))
    {
        return SCI_ERROR;
    }

    for (i = 0; i < GPIO_MAX_INT_REC_NUM; ++i)
    {
        if (s_gpio_int_table[i].gpio_id == gpio_id)
        {
            return (s_gpio_int_table[i].shaking_interval);
        }
        else if (s_gpio_int_table[i].gpio_id == (uint32) GPIO_INVALID_ID)
        {
            ////SCI_TRACE_LOW:"GPIO_GetShakeInterval ERROR gpio_id=%d,i=%d"
            SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_HAL_1082_112_2_17_23_4_44_990,(uint8*)"dd",gpio_id,i);
            SCI_ASSERT (0);/*assert to do*/
        }
    }

    return SCI_SUCCESS;
}

PUBLIC void GPIO_GetCallBack (uint32 gpio_id, uint32 gpio_state)
{
    uint32 i;
    GPIO_INFO_T gpio_info;

    GPIO_PHY_GetBaseInfo (gpio_id, &gpio_info);

    if (GPIO_PHY_GetDirection (&gpio_info) && (gpio_info.gpio_type == GPIO_SECTION_GPIO))
    {
        return;
    }

    if ( (gpio_info.gpio_type != GPIO_SECTION_GPI) && (gpio_info.gpio_type != GPIO_SECTION_GPIO))
    {
        return;
    }

    for (i = 0; i < GPIO_MAX_INT_REC_NUM; ++i)
    {
        if (s_gpio_int_table[i].gpio_id == gpio_id)
        {
            if (NULL != s_gpio_int_table[i].gpio_callback_fun)
            {
                (* s_gpio_int_table[i].gpio_callback_fun) (gpio_id, gpio_state);
                break;
            }
        }
        else if (s_gpio_int_table[i].gpio_id == (uint32) GPIO_INVALID_ID)
        {
            ////SCI_TRACE_LOW:"GPIO_GetCallBack ERROR gpio_id=%d,i=%d"
            SCI_TRACE_ID(TRACE_TOOL_CONVERT,GPIO_HAL_1119_112_2_17_23_4_44_991,(uint8*)"dd",gpio_id,i);
        }
    }
}

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
}

#endif  // End of gpio_drv.c
