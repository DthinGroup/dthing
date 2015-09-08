#include <vm_common.h>
#include "nativeI2CManager.h"

#if defined(ARCH_ARM_SPD)
#include "sci_types.h"
#include "i2c_drvapi.h"
#include "os_api.h"

#define I2C_FALSE 0
#define I2C_TRUE 1

#define I2C_MAX_BUSID 5
#define I2C_DEFAULT_BUSID 1

#define STATIC_iot_oem_i2c_I2CManager_DATA_RATE_STANDARD 0
#define STATIC_iot_oem_i2c_I2CManager_DATA_RATE_FAST 1
#define STATIC_iot_oem_i2c_I2CManager_DATA_RATE_HIGH 2

/*
 * @brief cpl_i2c_open(uint32 busId, uint32 freq, uint8 addr)
 * @note open i2c handle by busid
 * @param busId uint32 bus id or logic id
 * @param freq  uint32 frequency of bus to set
 * @param addr  uint8  slave address to set
 * @param regAddrNum uint32 number of reg address, 0 as default
 *
 * @return negative if failed, i2c handle when success
 */
static int cpl_i2c_open(uint32 busId, uint32 freq, uint8 addr, uint32 regAddrNum);

/*
 * @brief cpl_i2c_close(uint32 handle)
 * @note close i2c handle
 * @param handle uint32 i2c instance handle
 *
 * @return negative if failed, otherwise success
 */
static int cpl_i2c_close(uint32 handle);

/*
 * @brief cpl_i2c_setRate(uint32 handle, int rate)
 * @note set i2c frequency
 * @param handle uint32 i2c instance handle
 * @param rate   uint32 frequency of bus to set
 *
 * @return negative if failed, otherwise success
 */
static int cpl_i2c_setRate(uint32 handle, uint32 rate);

/*
 * @brief cpl_i2c_getRate(uint32 handle)
 * @note get i2c frequency
 * @param handle uint32 i2c instance handle
 *
 * @return negative if failed, current frequency when success
 */
static int cpl_i2c_getRate(uint32 handle);

/*
 * @brief cpl_i2c_read(uint32 handle, uint8 addr, char *buffer, uint32 len)
 * @note read data from i2c
 * @param handle uint32 i2c instance handle
 * @param addr uint8* sub address to set
 * @param[OUT] buffer char * buffer to read out
 * @param len uint32 data length to read out
 *
 * @return negative if failed, read length when success
 */
static int cpl_i2c_read(uint32 handle, uint8 *addr, char *buffer, uint32 len);

/*
 * @brief cpl_i2c_write(uint32 handle, uint8 addr, char *data, uint32 len)
 * @note write data to i2c
 * @param handle uint32 i2c instance handle
 * @param addr uin8* sub address to set
 * @param data char * data to write
 * @param len uint32 data length to write in
 *
 * @return negative if failed, otherwise when success
 */
static int cpl_i2c_write(uint32 handle, uint8 *addr, char *data, uint32 len);

/*
 * @brief cpl_i2c_setSlaveAddress(uint32 handle, uint8 addr)
 * @note set slave address for i2c before read/write
 * @param handle uint32 i2c instance handle
 * @param addr  uint8  slave address to set
 *
 * @return negative if failed, otherwise when success
 */
static int cpl_i2c_setSlaveAddress(uint32 handle, uint8 addr);

static bool_t cpl_i2c_isValidBusId(uint32 busId);
static int cpl_i2c_Wakeup(uint32 handle);
#endif

/**
 * Class:     iot_oem_i2c_I2CManager
 * Method:    open0
 * Signature: (IIII)I
 */
void Java_iot_oem_i2c_I2CManager_open0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint busId = (jint) args[1];
    jint freq = (jint) args[2];
    jint addr = (jint) args[3];
    jint regAddrNum = (jint) args[4];
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    int myRate = 0;

    switch(freq)
    {
    case STATIC_iot_oem_i2c_I2CManager_DATA_RATE_STANDARD:
        myRate = 100 * 1000;
        break;
    case STATIC_iot_oem_i2c_I2CManager_DATA_RATE_FAST:
        myRate = 400 * 1000;
        break;
    case STATIC_iot_oem_i2c_I2CManager_DATA_RATE_HIGH:
        myRate = 3.4 * 1000 * 1000;
        break;
    default:
        break;
    }
  ret = cpl_i2c_open(busId, myRate, addr, regAddrNum);
#endif

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_i2c_I2CManager
 * Method:    getRate0
 * Signature: (I)I
 */
void Java_iot_oem_i2c_I2CManager_getRate0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint handle = (jint) args[1];
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    int rate = cpl_i2c_getRate(handle);

    switch(rate)
    {
    case 100000:
        ret = STATIC_iot_oem_i2c_I2CManager_DATA_RATE_STANDARD;
        break;
    case 400000:
        ret = STATIC_iot_oem_i2c_I2CManager_DATA_RATE_FAST;
        break;
    case 3400000:
        ret = STATIC_iot_oem_i2c_I2CManager_DATA_RATE_HIGH;
        break;
    default:
        ret = -1;
        break;
    }
#endif

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_i2c_I2CManager
 * Method:    setRate0
 * Signature: (II)I
 */
void Java_iot_oem_i2c_I2CManager_setRate0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint handle = (jint) args[1];
    jint rate = (jint) args[2];
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
#endif

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_i2c_I2CManager
 * Method:    close0
 * Signature: (I)I
 */
void Java_iot_oem_i2c_I2CManager_close0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint handle = (jint) args[1];
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    ret = cpl_i2c_close(handle);
#endif

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_i2c_I2CManager
 * Method:    read0
 * Signature: (III[BI)I
 */
void Java_iot_oem_i2c_I2CManager_read0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint handle = (jint) args[1];
    jint addr = (jint) args[2];
    jint subAddr = (jint) args[3];
    ArrayObject * bufferArr = (ArrayObject *)args[4];
    jbyte * bufferArrPtr = (jbyte *)(KNI_GET_ARRAY_BUF(args[4]));
    int bufferArrLen = KNI_GET_ARRAY_LEN(args[4]);
    jint len = (jint) args[5];
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    uint8 address[4] = {0};

    ret = cpl_i2c_setSlaveAddress(handle, addr);

    if (ret >= 0)
  {
        address[0] = (uint8)(subAddr & 0x000000FF);
        address[1] = (uint8)((subAddr & 0x0000FF00) >> 8);
        address[2] = (uint8)((subAddr & 0x00FF0000) >> 16);
        address[3] = (uint8)((subAddr & 0xFF000000) >> 24);

        ret = cpl_i2c_read(handle, address, (uint8_t *)bufferArrPtr, len);
  }
#endif

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_i2c_I2CManager
 * Method:    write0
 * Signature: (III[BI)I
 */
void Java_iot_oem_i2c_I2CManager_write0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint handle = (jint) args[1];
    jint addr = (jint) args[2];
    jint subAddr = (jint) args[3];
    ArrayObject * dataArr = (ArrayObject *)args[4];
    jbyte * dataArrPtr = (jbyte *)(KNI_GET_ARRAY_BUF(args[4]));
    int dataArrLen = KNI_GET_ARRAY_LEN(args[4]);
    jint len = (jint) args[5];
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    uint8 address[4] = {0};
    ret = cpl_i2c_setSlaveAddress(handle, addr);

    if (ret >= 0)
    {
        address[0] = (uint8)(subAddr & 0x000000FF);
        address[1] = (uint8)((subAddr & 0x0000FF00) >> 8);
        address[2] = (uint8)((subAddr & 0x00FF0000) >> 16);
        address[3] = (uint8)((subAddr & 0xFF000000) >> 24);

        ret = cpl_i2c_write(handle, address, (uint8 *)dataArrPtr, len);
  }
#endif

    RETURN_INT(ret);
}

#if defined(ARCH_ARM_SPD)
static int cpl_i2c_open(uint32 busId, uint32 freq, uint8 addr, uint32 regAddrNum)
{
  int result = -1;
  I2C_DEV dev = {0};

  if(!cpl_i2c_isValidBusId(busId))
  {
    goto end;
  }

  //Workaround: hard-code the address for I2CDemo, because address
  //    need to be set when calling I2C_HAL_Open, as device act.
  dev.slave_addr = addr;

  //Since logic id is the same with bus id, we treat busId as logic id
  dev.id = busId;
  dev.freq = freq;
  dev.reg_addr_num = regAddrNum;
  dev.check_ack = 1;
  dev.no_stop = 1;

  result = I2C_HAL_Open(&dev);

end:
  DVMTraceDbg("[INFO][I2C] cpl_i2c_open %s with busId[%d] freq[%d] addr[%d] and result\n",
             (result < 0)? "failed" : "success", busId, freq, addr, result);
  return result;
}

static int cpl_i2c_close(uint32 handle)
{
  int result = 0;
  result = I2C_HAL_Close(handle);
  DVMTraceDbg("[INFO][I2C] cpl_i2c_close handle %d with result %d\n", handle, result);
  return result;
}

static int cpl_i2c_setRate(uint32 handle, uint32 rate)
{
  int result = 0;
  uint32 freq = rate;

  result = I2C_HAL_Ioctl(handle, I2C_CTL_S_FREQ, &freq);
  //DVMTraceDbg("[INFO][I2C] cpl_i2c_setRate %d with handle %d and result %d\n", rate, handle, result);
  return result;
}

static int cpl_i2c_getRate(uint32 handle)
{
  int result = 0;
  int freq = 0;

  result = I2C_HAL_Ioctl(handle, I2C_CTL_G_FREQ, &freq);

  if((result >= 0) && (freq > 0))
  {
    result = freq;
  }

  //DVMTraceDbg("[INFO][I2C] cpl_i2c_getRate with handle %d and result %d\n", handle, result);
  return result;
}

static int cpl_i2c_read(uint32 handle, uint8 *addr, char *buffer, uint32 len)
{
  int result = -1;

  if ((NULL == addr) || (NULL == buffer) || (len == 0))
  {
    goto end;
  }

  result = I2C_HAL_Read(handle, addr, buffer, len);
  result = (result == 0)? -1 : result;

  //Log for demo
  {
    int i = 0;
    for (i = 0; i < len; i++)
    {
      DVMTraceDbg("[INFO][I2C] read tmp[%d] %x\n", i, buffer[i]);
    }
  }

end:
  DVMTraceDbg("[INFO][I2C] cpl_i2c_read with handle[%d] addr[%s] buffer[%s] and result %d\n", handle, addr, buffer, result);
  return result;
}

static int cpl_i2c_write(uint32 handle, uint8 *addr, char *data, uint32 len)
{
  int result = -1;

  if ((NULL == addr) || (NULL == data) || (len == 0))
  {
    goto end;
  }

  //FIXME: Temperature/Humidity sensor need be waked up before action
  result = cpl_i2c_Wakeup(handle);

  result = I2C_HAL_Write(handle, addr, data, len);
  result = (result == 0)? -1 : result;

end:
  //DVMTraceDbg("[INFO][I2C] cpl_i2c_write with handle[%d] addr[%s] data[%s] and result %d\n", handle, addr, data, result);
  return result;
}

static int cpl_i2c_setSlaveAddress(uint32 handle, uint8 addr)
{
  int result = 0;
  uint8 slave_addr = addr;

  //FIXME: Temperature/Humidity sensor need be waked up before action
  //result = cpl_i2c_Wakeup(handle);

  result = I2C_HAL_Ioctl(handle, I2C_CTL_S_SLAVE_ADDR, &slave_addr);
  //DVMTraceDbg("[INFO][I2C] cpl_i2c_setSlaveAddress with handle[%d] addr[%s] and result %d\n", handle, addr, result);
  return result;
}

static bool_t cpl_i2c_isValidBusId(uint32 busId)
{
  return (busId < I2C_MAX_BUSID);
}

static int cpl_i2c_Wakeup(uint32 handle)
{
  int result = -1;

  DVMTraceDbg("[INFO][I2C] cpl_i2c_Wakeup with handle[%d]", handle);
  result = I2C_HAL_Wakeup(handle);
  return result;
}

#endif