/******************************************************************************
 ** File Name:      I2C_drv_new.h                                                 *
 ** Author:         liuhao                                                   *
 ** DATE:           06/28/2010                                                *
 ** Copyright:      2010 Spreadtrum, Incoporated. All Rights Reserved.         *
 ** Description:    This file define the basic interfaces of I2C device.      *
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                       *
 ** ------------------------------------------------------------------------- *
 ** DATE           NAME             DESCRIPTION                               *
 ** 06/28/2010     liuhao     Create.                                   *
 ******************************************************************************/
#ifndef _I2C_DRVAPI_H_
#define _I2C_DRVAPI_H_

/**---------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **---------------------------------------------------------------------------*/

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
**                               Micro Define                                                           **
**---------------------------------------------------------------------------*/
typedef struct
{
    /*logic id, which presentes as a specific i2c bus and the i2c slave device connects to this bus. */
    uint32 id;
    /*i2c slave device's working freqency */
    uint32 freq;
    /*Don't care it, i2c driver will calculate correct value and assign it. */
    uint32 bus;
    /*i2c slave device's write address, whose length is 8 bits, not 7bits.*/
    uint8 slave_addr;
    /*i2c slave device's internal register length */
    uint8 reg_addr_num;
    /*set to 1, i2c controller will check i2c slave device's ACK signal, otherwise, set it to 0*/
    uint8 check_ack;
    /*set to 1, i2c controller will send restart signal to i2c slave device in read transantion. */
    uint8 no_stop;
} I2C_DEV;

/**---------------------------------------------------------------------------*
 **                      I2C IOCTL
 **---------------------------------------------------------------------------*/
#define I2C_CTL_G_FREQ 0x20 /*get frequency*/
#define I2C_CTL_S_FREQ 0x21 /*set frequency*/
#define I2C_CTL_G_PORT 0x22 /*get port*/
#define I2C_CTL_S_PORT 0x23 /*set port*/
#define I2C_CTL_STOP_BUS 0x24 /*stop i2c bus*/
#define I2C_CTL_S_SLAVE_ADDR 0x25 /* set slave address */

/**---------------------------------------------------------------------------*
 **                      Function  Prototype
 **---------------------------------------------------------------------------*/
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
int32 I2C_HAL_Open (I2C_DEV *dev);

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
int32 I2C_HAL_Close (uint32 handle);


/*********************************************************************************************************
** Function name:I2C_HAL_ReadEx
** Descriptions:This function is used to start a read transfer.
** input parameters:
**     Handle:  an i2c slave device has gotten by calling i2c_hal_open.
**     reg_addr:  the buffer to store i2c slave device's internal register address.
**     bytes:  the number of reading data.
**     buffer:These buffers will store the data, read from i2c slave device
** output parameters:
** Returned value:
**     Success:  return the number of reading data.
**     Fail:  return 0.
*********************************************************************************************************/
uint32 I2C_HAL_ReadEx (uint32 handle, uint8 *reg_addr, uint8 *buffer, uint32 bytes);

/*********************************************************************************************************
** Function name:I2C_HAL_Read
** Descriptions:This function is used to start a read transfer.
** input parameters:
**     Handle:  an i2c slave device has gotten by calling i2c_hal_open.
**     reg_addr:  the buffer to store i2c slave device's internal register address.
**     bytes:  the number of reading data.
**     buffer:These buffers will store the data, read from i2c slave device
** output parameters:
** Returned value:
**     Success:  return the number of reading data.
**     Fail:  return 0.
*********************************************************************************************************/
uint32 I2C_HAL_Read (uint32 handle, uint8 *reg_addr, uint8 *buffer, uint32 bytes);

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
uint32 I2C_HAL_Write (uint32 handle, uint8 *reg_addr, uint8 *buffer, uint32 bytes);

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

uint32 I2C_HAL_Ioctl (uint32 handle, uint32 cmd, uint32 *arg);

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
}
#endif

#endif

/* End Of File  */
