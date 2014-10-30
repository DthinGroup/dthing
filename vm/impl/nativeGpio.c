#include <vm_common.h>
#include "nativeGpio.h"

#if defined(ARCH_ARM_SPD)
#include "sci_types.h"
#include "os_api.h"
#include "gpio_drvapi.h"
#endif

#define GPIO_FALSE 0
#define GPIO_TRUE 1

#define GPIO_INPUT_DIRECTION  0
#define GPIO_OUTPUT_DIRECTION 1

#define GPIO_VIBRA_ID 8

#define GPIO_WRITE_MODE          0
#define GPIO_READ_MODE           1
#define GPIO_RDWR_MODE           2

#define GPIO_INT_TYPE_LOW        0
#define GPIO_INT_TYPE_HIGH       1
#define GPIO_INT_TYPE_FALLING    2
#define GPIO_INT_TYPE_RISING     3
#define GPIO_INT_TYPE_BOTH_EDGES 4

#if defined(ARCH_ARM_SPD)
void vibra_gpio_callback(uint32 gpio_id, uint32 gpio_state);

/*
 * @brief cpl_gpio_open(uint32 gpio_id)
 * @note open selected gpio
 * @param gpio_id uint32 gpio id
 *
 * @return negative if failed, otherwise when success
 */
static int cpl_gpio_open(uint32 gpio_id);

/*
 * @brief cpl_gpio_close(uint32 gpio_id)
 * @note close selected gpio
 * @param gpio_id uint32 gpio id
 *
 * @return negative if failed, otherwise when success
 */
static int cpl_gpio_close(uint32 gpio_id);

/*
 * @brief cpl_gpio_setMode(uint32 gpio_id, int mode)
 * @note set gpio mode
 * @param gpio_id uint32 gpio id
 * @param mode uint32 gpio mode to set
 *
 * @return negative if failed, otherwise when success
 */
static int cpl_gpio_setMode(uint32 gpio_id, uint32 mode);

/*
 * @brief cpl_gpio_getMode(uint32 gpio_id)
 * @note get gpio mode
 * @param gpio_id uint32 gpio id
 *
 * @return negative if failed, GPIO_READ_MODE or GPIO_WRITE_MODE when success
 */
static int cpl_gpio_getMode(uint32 gpio_id);

/*
 * @brief cpl_gpio_read(uint32 gpio_id)
 * @note read out gpio value
 * @param gpio_id uint32 gpio id
 *
 * @return negative if failed, gpio value when success
 */
static int cpl_gpio_read(uint32 gpio_id);

/*
 * @brief cpl_gpio_read(uint32 gpio_id)
 * @note write in gpio value
 * @param gpio_id uint32 gpio id
 * @param value bool_t gpio value to write in
 *
 * @return negative if failed, otherwise when success
 */
static int cpl_gpio_write(uint32 gpio_id, bool_t value);

/*
 * @brief cpl_gpio_registerInt
 * @note register gpio int
 * @param gpio_id uint32 gpio id
 *
 * @return negative if failed, otherwise when success
 */
static int cpl_gpio_registerInt(uint32 gpio_id, GPIO_INT_TYPE type, GPIO_CALLBACK cb);

/*
 * @brief cpl_gpio_unregisterInt(uint32 gpio_id)
 * @note unregister gpio int
 * @param gpio_id uint32 gpio id
 *
 * @return negative if failed, otherwise when success
 */
static int cpl_gpio_unregisterInt(uint32 gpio_id);
#endif

/**
 * Class:     iot_oem_gpio_Gpio
 * Method:    open0
 * Signature: (I)I
 */
void Java_iot_oem_gpio_Gpio_open0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint number = (jint) args[1];
    jint ret = 0;
#if defined(ARCH_ARM_SPD)
    ret = cpl_gpio_open(number);
#endif
    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_gpio_Gpio
 * Method:    setCurrentMode0
 * Signature: (II)I
 */
void Java_iot_oem_gpio_Gpio_setCurrentMode0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint number = (jint) args[1];
    jint mode = (jint) args[2];
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    ret = cpl_gpio_setMode(number, mode);
#endif

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_gpio_Gpio
 * Method:    getCurrentMode0
 * Signature: (I)I
 */
void Java_iot_oem_gpio_Gpio_getCurrentMode0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint number = (jint) args[1];
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    ret = cpl_gpio_getMode(number);
#endif

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_gpio_Gpio
 * Method:    read0
 * Signature: (I)I
 */
void Java_iot_oem_gpio_Gpio_read0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint number = (jint) args[1];
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    ret = cpl_gpio_read(number);
#endif

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_gpio_Gpio
 * Method:    write0
 * Signature: (IZ)I
 */
void Java_iot_oem_gpio_Gpio_write0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint number = (jint) args[1];
    jboolean value = (jboolean) args[2];
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    ret = cpl_gpio_write(number, value);
#endif

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_gpio_Gpio
 * Method:    registerInt0
 * Signature: (II)I
 */
void Java_iot_oem_gpio_Gpio_registerInt0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint number = (jint) args[1];
    jint type = (jint) args[2];
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    GPIO_INT_TYPE intType = 0;

    switch(type)
    {
    case GPIO_INT_TYPE_LOW:
        intType = GPIO_INT_LEVEL_LOW;
        break;
    case GPIO_INT_TYPE_HIGH:
        intType = GPIO_INT_LEVEL_HIGH;
        break;
    case GPIO_INT_TYPE_FALLING:
        intType = GPIO_INT_EDGES_FALLING;
        break;
    case GPIO_INT_TYPE_RISING:
        intType = GPIO_INT_EDGES_RISING;
        break;
    case GPIO_INT_TYPE_BOTH_EDGES:
        intType = GPIO_INT_EDGES_BOTH;
        break;
    default:
        intType = GPIO_INT_DISABLE;
        break;
    }
    //TODO: register callback for current gpio int
    ret = cpl_gpio_registerInt(number, intType, vibra_gpio_callback); 
#endif
    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_gpio_Gpio
 * Method:    unregisterInt0
 * Signature: (I)I
 */
void Java_iot_oem_gpio_Gpio_unregisterInt0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint number = (jint) args[1];
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    ret = cpl_gpio_unregisterInt(number);
#endif

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_gpio_Gpio
 * Method:    close0
 * Signature: (I)I
 */
void Java_iot_oem_gpio_Gpio_close0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint number = (jint) args[1];
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    ret = cpl_gpio_close(number);
#endif

    RETURN_INT(ret);
}

#if defined(ARCH_ARM_SPD)
static int cpl_gpio_open(uint32 gpio_id)
{
  int result = 0;
  bool_t isEnabled = GPIO_FALSE;

  if (gpio_id > GPIO_GetMaxNumber())
  {
    result = -1;
    goto end;
  }

  isEnabled = GPIO_GetDataMask(gpio_id);

  if (!isEnabled)
  {
    GPIO_Enable(gpio_id);
  }

  if(!GPIO_GetDataMask(gpio_id))
  {
    result = -1;
  }
end:
  DthingTraceD("[INFO][GPIO] cpl_gpio_open with gpio_id[%d] and result[%d]\n", gpio_id, result);
  return result;
}

static int cpl_gpio_close(uint32 gpio_id)
{
  int result = 0;
  bool_t isEnabled = GPIO_FALSE;

  if (gpio_id > GPIO_GetMaxNumber())
  {
    result = -1;
    goto end;
  }

  isEnabled = GPIO_GetDataMask(gpio_id);

  if (isEnabled)
  {
    GPIO_Disable(gpio_id);
  }

  if(GPIO_GetDataMask(gpio_id))
  {
    result = -1;
  }
end:
  DthingTraceD("[INFO][GPIO] cpl_gpio_close with gpio_id[%d] and result[%d]\n", gpio_id, result);
  return result;

}

static int cpl_gpio_setMode(uint32 gpio_id, uint32 mode)
{
  int result = 0;

  if (gpio_id > GPIO_GetMaxNumber())
  {
    result = -1;
    goto end;
  }

  switch(mode)
  {
    case GPIO_WRITE_MODE:
      GPIO_SetDirection(gpio_id, GPIO_OUTPUT_DIRECTION);
      break;

    case GPIO_READ_MODE:
      GPIO_SetDirection(gpio_id, GPIO_INPUT_DIRECTION);
      break;

    default:
      result = -1;
      break;
  }
end:
  DthingTraceD("[INFO][GPIO] cpl_gpio_setMode with gpio_id[%d] mode[%d] and result[%d]\n", gpio_id, mode, result);
  return result;
}

static int cpl_gpio_getMode(uint32 gpio_id)
{
  int mode = -1;

  if (gpio_id > GPIO_GetMaxNumber())
  {
    goto end;
  }

  mode = GPIO_GetDirection(gpio_id)? GPIO_WRITE_MODE : GPIO_READ_MODE;

end:
  DthingTraceD("[INFO][GPIO] cpl_gpio_getMode with gpio_id[%d] and mode[%d]\n", gpio_id, mode);
  return mode;
}

static int cpl_gpio_read(uint32 gpio_id)
{
  int result = -1;

  if (gpio_id > GPIO_GetMaxNumber())
  {
    goto end;
  }

  result = (int)GPIO_GetValue(gpio_id);

end:
  DthingTraceD("[INFO][GPIO] cpl_gpio_read with gpio_id[%d] and result[%d]\n", gpio_id, result);
  return result;
}

static int cpl_gpio_write(uint32 gpio_id, bool_t value)
{
  int result = 0;

  if (gpio_id > GPIO_GetMaxNumber())
  {
    result = -1;
    goto end;
  }

  GPIO_SetValue(gpio_id, value);

end:
  DthingTraceD("[INFO][GPIO] cpl_gpio_write with gpio_id[%d] value[%d] and result[%d]\n", gpio_id, value, result);
  return result;
}

static int cpl_gpio_registerInt(uint32 gpio_id, GPIO_INT_TYPE type, GPIO_CALLBACK cb)
{
  int result = 0;

  if (gpio_id > GPIO_GetMaxNumber())
  {
    result = -1;
    goto end;
  }

  GPIO_SetInterruptSense(gpio_id, type);
  GPIO_EnableIntCtl(gpio_id);
  result = GPIO_AddCallbackToIntTable(gpio_id, GPIO_FALSE, 0, cb);

end:
  DthingTraceD("[INFO][GPIO] cpl_gpio_registerInt with gpio_id[%d] int_type[%d] and result[%d]\n", gpio_id, type, result);
  return result;
}

static int cpl_gpio_unregisterInt(uint32 gpio_id)
{
  int result = 0;

  if (gpio_id > GPIO_GetMaxNumber())
  {
    result = -1;
    goto end;
  }

  GPIO_DisableIntCtl(gpio_id);
  result = GPIO_AddCallbackToIntTable(gpio_id, GPIO_FALSE, 0, (void *)0);

end:
  DthingTraceD("[INFO][GPIO] cpl_gpio_unregisterInt with gpio_id[%d] and result[%d]\n", gpio_id, result);
  return result;
}

//FIXME: Currently, java api not provides the way to access some native method.
//       As a workaround fix, just call these method in native.
void vibra_gpio_callback(uint32 gpio_id, uint32 gpio_state)
{
  int state = gpio_state;
  DthingTraceD("[INFO][GPIO] gpio id:%d, gpio_state:%d\n", gpio_id, gpio_state);

  if (GPIO_VIBRA_ID == gpio_id)
  {
    DthingTraceD("[INFO][GPIO] Enter callback for vibra gpio\n");
	//Jbed_setProperty("yarlung.vibra", state? "true" : "false"); //TODO:any property access apis in dvm?
    GPIO_SetInterruptSense(GPIO_VIBRA_ID, GPIO_INT_EDGES_RISING);
    GPIO_ClearIntStatus(GPIO_VIBRA_ID);
    GPIO_EnableIntCtl(GPIO_VIBRA_ID);
  }
}
#endif