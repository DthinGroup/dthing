/******************************************************************************
 ** File Name:      I2C_drv.c                                                *
 ** Author:         liuhao                                                   *
 ** DATE:           06/28/2010                                                *
 ** Copyright:      2010 Spreadtrum, Incoporated. All Rights Reserved.         *
 ** Description:    This file define the hal layer of I2C device.      *
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                       *
 ** ------------------------------------------------------------------------- *
 ** DATE           NAME             DESCRIPTION                               *
 ** 06/28/2010     liuhao     Create.                                   *
 ******************************************************************************/

/**---------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **---------------------------------------------------------------------------*/
#include "sci_types.h"
#include "os_api.h"
#include "chip_plf_export.h"
#include "i2c_drvapi.h"

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

/**---------------------------------------------------------------------------*
 **                            Macro Define
 **---------------------------------------------------------------------------*/
typedef struct
{
    SCI_MUTEX_PTR mutex;
    uint32 current_port;
    uint32 current_freq;
    uint32 reference;
    I2C_PHY_FUN *phy_fun;
} I2C_BUS;

#define I2C_GetMutex(mutex)  {\
        uint32 ret = SCI_SUCCESS;\
        if( NULL == mutex )\
        {\
            return ERR_I2C_NO_MUTEX;\
        }\
        if(SCI_InThreadContext())\
        {\
            ret = SCI_GetMutex(mutex, SCI_INVALID_BLOCK_ID != SCI_IdentifyThread() ? SCI_WAIT_FOREVER : 0);\
        }\
        SCI_ASSERT( ret == SCI_SUCCESS );/*assert verified*/\
    }

#define I2C_PutMutex(mutex)  {\
        uint32 ret = SCI_SUCCESS;\
        if( NULL == mutex )\
        {\
            return ERR_I2C_NO_MUTEX;\
        }\
        if(SCI_InThreadContext())\
        {\
            ret = SCI_PutMutex( mutex );\
        }\
        SCI_ASSERT( ret == SCI_SUCCESS );/*assert verified*/\
    }
/**---------------------------------------------------------------------------*
 **                            Local Variables
 **---------------------------------------------------------------------------*/
LOCAL I2C_DEV __i2c_dev[I2C_BUS_MAX *I2C_DEV_MAX];

/**---------------------------------------------------------------------------*
 **                            Global Variables
 **---------------------------------------------------------------------------*/
I2C_BUS __i2c_bus[I2C_BUS_MAX];
extern const I2C_PHY_CFG __i2c_phy_cfg[I2C_ID_MAX];

/**---------------------------------------------------------------------------*
 **                      Function  Definitions
 **---------------------------------------------------------------------------*/
/*********************************************************************************************************
** Function name:I2C_Bus_CFG
** Descriptions:this fun is used to config i2c bus.
** input parameters:
**     logic_id:logic id which presentes as a specific i2c bus and the i2c slave device connects to this bus.
**
**
** output parameters:n/a
** Returned value:0
*********************************************************************************************************/
LOCAL uint32 I2C_Bus_CFG (uint32 logic_id)
{
    uint32 bus_id = __i2c_phy_cfg[logic_id].phy_id;
    /*config bus method*/
    __i2c_bus[bus_id].phy_fun = __i2c_phy_cfg[logic_id].phy_fun;
    return 0;
}
/*********************************************************************************************************
** Function name:I2C_Bus_Init
** Descriptions:this fun is used to init i2c bus.
** input parameters:
**     logic_id:logic id which presentes as a specific i2c bus and the i2c slave device connects to this bus.
**     freq: i2c freq
**     port_id: i2c pad group num.
** output parameters:n/a
** Returned value:0
*********************************************************************************************************/
LOCAL uint32 I2C_Bus_Init (uint32 logic_id, uint32 freq, uint32 port_id)
{
    uint32 bus_id = __i2c_phy_cfg[logic_id].phy_id;
    IIC_PRINT (("[IIC DRV:]I2C_Bus_Init"));
    /*config bus property*/
    __i2c_bus[bus_id].current_freq = freq;
    __i2c_bus[bus_id].current_port = port_id;
    if(NULL == __i2c_bus[bus_id].mutex)
    {
        __i2c_bus[bus_id].mutex = SCI_CreateMutex ("I2C SYNC MUTEX", SCI_INHERIT);
    }
    SCI_PASSERT ( (NULL!= __i2c_bus[bus_id].mutex), ("I2C Great MUTEX fail!"));/*assert verified*/
    /*config bus method*/
    I2C_Bus_CFG (logic_id);
    return 0;
}

/*********************************************************************************************************
** Function name:I2C_Bus_Exit
** Descriptions:this fun is used to free i2c bus.
** input parameters:
**     bus_id : i2c controller id
**
**
** output parameters:n/a
** Returned value:0
*********************************************************************************************************/
LOCAL uint32 I2C_Bus_Exit (uint32 bus_id)
{
    uint32 status_val = 0;
    __i2c_bus[bus_id].reference =  0;

    if (NULL != __i2c_bus[bus_id].mutex && SCI_InThreadContext())
    {
        status_val = SCI_DeleteMutex (__i2c_bus[bus_id].mutex);
        //SCI_PASSERT((0 != status_val),("I2C delete MUTEX fail!"));
        __i2c_bus[bus_id].mutex = NULL;
    }

    IIC_PRINT (("[IIC DRV:]I2C_Bus_Exit: status=%d", status_val));
    /*release the bus method*/
    __i2c_bus[bus_id].phy_fun = NULL;
    return 0;
}

/*********************************************************************************************************
** Function name: I2C_HAL_Open
** Descriptions: This function will initialize the i2c controller, and return a handler for the i2c slave device.
** input parameters:
**     dev - pointer of i2c slave dev
**
**
** output parameters: n/a
** Returned value:
**     Success:  return a handle, whose value is more than or equal to 0.
**     Fail:  return -1
*********************************************************************************************************/
PUBLIC int32 I2C_HAL_Open (I2C_DEV *dev)
{
    uint32 handle;
    uint32 bus_id;
    uint32 port_id;
    uint32 idx = 0;
    int32 i =0;
    SCI_ASSERT (NULL != dev);/*assert verified*/
    /*check if use the wrong port number*/
    SCI_ASSERT (I2C_ID_MAX > dev->id);/*assert verified*/
    /*now we should get the i2c bus id and the controller's port id*/
    bus_id = __i2c_phy_cfg[dev->id].phy_id;
    port_id = __i2c_phy_cfg[dev->id].port_id;
    /*reference shouldn't larger than I2C_DEV_MAX*/
    SCI_ASSERT (I2C_DEV_MAX > __i2c_bus[bus_id].reference);/*assert verified*/

    /*look for the handle: return the 1st available array member*/
    for (i=0; i< I2C_DEV_MAX; i++)
    {
        if (0 == __i2c_dev[idx + bus_id * I2C_DEV_MAX].freq)
        {
            break;
        }

        idx++;

        if (I2C_DEV_MAX == idx)
        {
            return -1;
        }
    }

    /*caculate handle*/
    handle = idx + bus_id * I2C_DEV_MAX;
    /*set the parameters for i2c slave device*/
    __i2c_dev[handle].id = dev->id;
    __i2c_dev[handle].freq = dev->freq;
    __i2c_dev[handle].bus = bus_id;
    __i2c_dev[handle].slave_addr = dev->slave_addr;
    __i2c_dev[handle].reg_addr_num = dev->reg_addr_num;
    __i2c_dev[handle].check_ack = dev->check_ack;
    __i2c_dev[handle].no_stop = dev->no_stop;

    /*first open, than init i2c controller*/
    if (0 == __i2c_bus[bus_id].reference)
    {
        I2C_Bus_Init (dev->id, dev->freq, port_id);
        __i2c_bus[bus_id].phy_fun->init (bus_id, dev->freq, port_id);
    }

    __i2c_bus[bus_id].reference++;

    IIC_PRINT (("[IIC DRV:]I2C_HAL_Open: handle=%d, ref=%d", handle, __i2c_bus[bus_id].reference));

    return  handle;
}

/*********************************************************************************************************
** Function name:I2C_HAL_Close
** Descriptions:This function will close the i2c controller.
** input parameters:
**     handle:the i2c slave device has gotten by calling i2c_hal_open
**
**
** output parameters:n/a
** Returned value:
**     Success:  return 0.
**     Fail:  return -1
*********************************************************************************************************/
PUBLIC int32 I2C_HAL_Close (uint32 handle)
{
    uint32 bus_id;
    /*handle shouldn't larger than I2C_BUS_MAX*I2C_DEV_MAX*/
    SCI_ASSERT (I2C_BUS_MAX * I2C_DEV_MAX > handle);/*assert verified*/

    /*this handle is already closed*/
    if (0 == __i2c_dev[handle].freq)
    {
        return -1;
    }

    bus_id = __i2c_dev[handle].bus;

    if (1 == __i2c_bus[bus_id].reference)
    {
        I2C_Bus_Exit (bus_id);
    }
    else
    {
        __i2c_bus[bus_id].reference--;
    }

    /*release the handle*/
    __i2c_dev[handle].id = 0;
    __i2c_dev[handle].freq = 0;
    __i2c_dev[handle].slave_addr = 0;
    __i2c_dev[handle].reg_addr_num = 0;
    __i2c_dev[handle].check_ack = 0;
    __i2c_dev[handle].no_stop = 0;

    IIC_PRINT (("[IIC DRV:]I2C_HAL_Close: handle=%d, ref=%d", handle, __i2c_bus[bus_id].reference));

    return  0;
}

/*********************************************************************************************************
** Function name:I2C_HAL_Read
** Descriptions:This function is used to start a read transfer.
** input parameters:
**     Handle:  an i2c slave device has gotten by calling i2c_hal_open.
**     reg_addr:  the buffer to store i2c slave device's internal register address.
**     bytes:  the number of reading data.
** output parameters:buffer:These buffers will store the data, read from i2c slave device
** Returned value:
**     Success:  return the number of reading data.
**     Fail:  return 0.
*********************************************************************************************************/
PUBLIC uint32 I2C_HAL_Read (uint32 handle, uint8 *reg_addr, uint8 *buffer, uint32 bytes)
{
    uint32 ret_val;
    uint32 bus_id;
    uint32 port_id;
    uint32 err_code;
    /*handle shouldn't larger than I2C_BUS_MAX*I2C_DEV_MAX*/
    SCI_ASSERT (I2C_BUS_MAX * I2C_DEV_MAX > handle);/*assert verified*/
    SCI_ASSERT (NULL != buffer);/*assert verified*/
    SCI_ASSERT (0 < bytes);/*assert verified*/
    /*the handle has destroied*/
    SCI_ASSERT (0 != __i2c_dev[handle].freq);/*assert verified*/
    bus_id = __i2c_dev[handle].bus;
    port_id = __i2c_phy_cfg[__i2c_dev[handle].id].port_id;
    /*get i2c bus*/
    I2C_GetMutex (__i2c_bus[bus_id].mutex);

    /*set i2c controller*/
    if ( (__i2c_bus[bus_id].current_freq != __i2c_dev[handle].freq) ||
            (__i2c_bus[bus_id].current_port != port_id))
    {
        __i2c_bus[bus_id].phy_fun->init (bus_id,  __i2c_dev[handle].freq, port_id);
        __i2c_bus[bus_id].current_freq = __i2c_dev[handle].freq;
        __i2c_bus[bus_id].current_port = port_id;
    }

    do
    {
        err_code = 0;

        /*this is for combined format*/
        if (0 < __i2c_dev[handle].reg_addr_num)
        {
            SCI_ASSERT (NULL != reg_addr);/*assert verified*/
            /*start i2c bus*/
            ret_val = __i2c_bus[bus_id].phy_fun->start (bus_id, __i2c_dev[handle].slave_addr, 0, __i2c_dev[handle].check_ack);

            if (ERR_I2C_NONE != ret_val)
            {
                break;
            }

            /*write i2c slave device's sub address*/
            ret_val = __i2c_bus[bus_id].phy_fun->write (bus_id, reg_addr, __i2c_dev[handle].reg_addr_num, __i2c_dev[handle].check_ack, __i2c_dev[handle].no_stop);

            if (ERR_I2C_NONE != ret_val)
            {
                break;
            }
        }

        /*restart i2c bus*/
        ret_val = __i2c_bus[bus_id].phy_fun->start (bus_id, __i2c_dev[handle].slave_addr, 1, __i2c_dev[handle].check_ack);

        if (ERR_I2C_NONE != ret_val)
        {
            break;
        }

        /*read data and stop bus*/
        ret_val = __i2c_bus[bus_id].phy_fun->read (bus_id, buffer, bytes, __i2c_dev[handle].check_ack);

        if (ERR_I2C_NONE != ret_val)
        {
            break;
        }

        err_code = bytes;
    }
    while (0);

    /*release i2c bus*/
    I2C_PutMutex (__i2c_bus[bus_id].mutex);
    return  err_code;
}

/*********************************************************************************************************
** Function name:I2C_HAL_ReadEx
** Descriptions:This function is used to start a read transfer. and verify the data.
** input parameters:
**     Handle:  an i2c slave device has gotten by calling i2c_hal_open.
**     reg_addr:  the buffer to store i2c slave device's internal register address.
**     bytes:  the number of reading data.
** output parameters:buffer:These buffers will store the data, read from i2c slave device
** Returned value:
**     Success:  return the number of reading data.
**     Fail:  return 0.
*********************************************************************************************************/
PUBLIC uint32 I2C_HAL_ReadEx (uint32 handle, uint8 *reg_addr, uint8 *buffer, uint32 bytes)
{
    uint32 ret_val = ERR_I2C_NONE;
    uint8 buf[100] = {0};
    static uint32 i2c_err_nr = 0;

    SCI_ASSERT (bytes <= 100);/*assert verified*/
    ret_val = I2C_HAL_Read(handle, reg_addr, buf, bytes);
    ret_val = I2C_HAL_Read(handle, reg_addr, buffer, bytes);
    if(0 != memcmp(buf, buffer, bytes))
    {
        i2c_err_nr++;
        IIC_PRINT (("[IIC DRV:]I2C Read Result is error, try again!"));
        ret_val = I2C_HAL_Read(handle, reg_addr, buffer, bytes);
    }

    return  ret_val;
}

PUBLIC uint32 I2C_HAL_Wakeup (uint32 handle)
{
    uint32 ret_val;
    uint32 bus_id;
    uint32 port_id;
    uint32 err_code;
    /*handle shouldn't larger than I2C_BUS_MAX*I2C_DEV_MAX*/
    SCI_ASSERT (I2C_BUS_MAX * I2C_DEV_MAX > handle);/*assert verified*/
    /*the handle has destroied*/
    SCI_ASSERT (0 != __i2c_dev[handle].freq);/*assert verified*/
    bus_id = __i2c_dev[handle].bus;
    port_id = __i2c_phy_cfg[__i2c_dev[handle].id].port_id;
    /*get i2c bus*/
    I2C_GetMutex (__i2c_bus[bus_id].mutex);

    /*set i2c controller*/
    if ( (__i2c_bus[bus_id].current_freq != __i2c_dev[handle].freq) ||
            (__i2c_bus[bus_id].current_port != port_id))
    {
        __i2c_bus[bus_id].phy_fun->init (bus_id, __i2c_dev[handle].freq, port_id);
        __i2c_bus[bus_id].current_freq = __i2c_dev[handle].freq;
        __i2c_bus[bus_id].current_port = port_id;
    }

    do
    {
        err_code = 0;
        /*start i2c bus*/
        ret_val = __i2c_bus[bus_id].phy_fun->start (bus_id, __i2c_dev[handle].slave_addr, 0, __i2c_dev[handle].check_ack);

        if (ERR_I2C_NONE != ret_val)
        {
            //SCI_TRACE_LOW("I2C_HAL_Wakeup:==Humidity==ERR_I2C_NONE != ret_val 1 ret_val=%d",ret_val);
            break;
        }

        /* wait 800us to 3 ms */
        SCI_Sleep(2);
        /* Stop command */
        ret_val = __i2c_bus[bus_id].phy_fun->stop (bus_id);

        if (ERR_I2C_NONE != ret_val)
        {
            //SCI_TRACE_LOW("I2C_HAL_Wakeup:==Humidity==ERR_I2C_NONE != ret_val 3 ret_val=%d",ret_val);
            break;
        }

        err_code = ret_val;
    }
    while (0);

    /*release i2c bus*/
    I2C_PutMutex (__i2c_bus[bus_id].mutex);
    return  err_code;
}

/*********************************************************************************************************
** Function name:I2C_HAL_Write
** Descriptions:This function is used to start a write transfer.
** input parameters:
**     Handle:  an i2c slave device has gotten by calling i2c_hal_open.
**     reg_addr:  the buffer to store i2c slave device's internal register address.
**     buffer:  These buffers will store the data, write to i2c slave device
**     bytes:  the number of writing data.
** output parameters:n/a
** Returned value:
**     Success:  return the number of writing data.
**     Fail:  return 0.
*********************************************************************************************************/
PUBLIC uint32 I2C_HAL_Write (uint32 handle, uint8 *reg_addr, uint8 *buffer, uint32 bytes)
{
    uint32 ret_val;
    uint32 bus_id;
    uint32 port_id;
    uint32 err_code;
    /*handle shouldn't larger than I2C_BUS_MAX*I2C_DEV_MAX*/
    SCI_ASSERT (I2C_BUS_MAX * I2C_DEV_MAX > handle);/*assert verified*/
    SCI_ASSERT (NULL != buffer);/*assert verified*/
    SCI_ASSERT (0 < bytes);/*assert verified*/
    /*the handle has destroied*/
    SCI_ASSERT (0 != __i2c_dev[handle].freq);/*assert verified*/
    bus_id = __i2c_dev[handle].bus;
    port_id = __i2c_phy_cfg[__i2c_dev[handle].id].port_id;
    /*get i2c bus*/
    I2C_GetMutex (__i2c_bus[bus_id].mutex);

    /*set i2c controller*/
    if ( (__i2c_bus[bus_id].current_freq != __i2c_dev[handle].freq) ||
            (__i2c_bus[bus_id].current_port != port_id))
    {
        __i2c_bus[bus_id].phy_fun->init (bus_id, __i2c_dev[handle].freq, port_id);
        __i2c_bus[bus_id].current_freq = __i2c_dev[handle].freq;
        __i2c_bus[bus_id].current_port = port_id;
    }

    do
    {
        err_code = 0;
        /*start i2c bus*/
        ret_val = __i2c_bus[bus_id].phy_fun->start (bus_id, __i2c_dev[handle].slave_addr, 0, __i2c_dev[handle].check_ack);

        if (ERR_I2C_NONE != ret_val)
        {
            break;
        }

        /*write i2c slave device's sub address*/
        if (0 < __i2c_dev[handle].reg_addr_num)
        {
            SCI_ASSERT (NULL != reg_addr);/*assert verified*/
            ret_val = __i2c_bus[bus_id].phy_fun->write (bus_id, reg_addr, __i2c_dev[handle].reg_addr_num, __i2c_dev[handle].check_ack, 2);

            if (ERR_I2C_NONE != ret_val)
            {
                break;
            }
        }

        /*write i2c slave device's data and stop bus*/
        ret_val = __i2c_bus[bus_id].phy_fun->write (bus_id, buffer, bytes, __i2c_dev[handle].check_ack, 0);

        if (ERR_I2C_NONE != ret_val)
        {
            break;
        }

        err_code = bytes;
    }
    while (0);

    /*release i2c bus*/
    I2C_PutMutex (__i2c_bus[bus_id].mutex);
    return  err_code;
}

/*********************************************************************************************************
** Function name:I2C_HAL_Ioctl
** Descriptions:This function will set i2c controller or get some status.
** input parameters:
**    Handle:  a i2c slave device has gotten by calling i2c_hal_open.
**    command:  Command type is defined in i2c_drvapi.h
**    arg:  command parameter
** output parameters:n/a
** Returned value:Don't care the return value, currently this function will always succeed.
*********************************************************************************************************/
PUBLIC uint32 I2C_HAL_Ioctl (uint32 handle, uint32 cmd, uint32 *arg)
{
    uint32 ret_val = 0;
    uint32 bus_id;
    /*handle shouldn't larger than I2C_BUS_MAX*I2C_DEV_MAX*/
    SCI_ASSERT (I2C_BUS_MAX * I2C_DEV_MAX > handle);/*assert verified*/
    SCI_ASSERT (NULL != arg);/*assert verified*/
    bus_id = __i2c_dev[handle].bus;

    switch (cmd)
    {
        case I2C_CTL_G_FREQ:
            *arg = __i2c_dev[handle].freq;
            break;
        case I2C_CTL_S_FREQ:
            __i2c_dev[handle].freq = *arg;
            break;
        case I2C_CTL_G_PORT:
            *arg = __i2c_dev[handle].id;
            break;
        case I2C_CTL_S_PORT:
            __i2c_dev[handle].id = *arg;
            break;
        case I2C_CTL_S_SLAVE_ADDR:
            __i2c_dev[handle].slave_addr = *arg;
            break;
        case I2C_CTL_STOP_BUS:
            /*get i2c bus*/
            I2C_GetMutex (__i2c_bus[bus_id].mutex);
            ret_val = __i2c_bus[bus_id].phy_fun->stop (bus_id);
            /*release i2c bus*/
            I2C_PutMutex (__i2c_bus[bus_id].mutex);
            break;
        default:
            break;
    }

    return  ret_val;
}

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
}
#endif
/*  End Of File */
