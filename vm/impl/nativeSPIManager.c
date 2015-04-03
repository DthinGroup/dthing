#include <vm_common.h>
#include "nativeSPIManager.h"

#if defined(ARCH_ARM_SPD)
#include "sci_types.h"
#include "os_api.h"
#include "spi_drvapi.h"
#endif

#define SPI_DEFAULT_MODE CPOL0_CPHA0 //TODO: re-define it later
#define SPI_DEFAULT_TX_BIT_LEN 8 //TODO: re-define it later

/**
 * Class:     iot_oem_spi_SPIManager
 * Method:    open0
 * Signature: (IIII)I
 */
void Java_iot_oem_spi_SPIManager_open0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint busId = (jint) args[1];
    jint rate = (jint) args[2];
    jint mode = (jint) args[3];
    jint bitlen = (jint) args[4];
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    SPI_DEV dev = {0};
  
    dev.id = busId;
    dev.freq = rate;
    dev.mode = mode;
    dev.tx_bit_length = bitlen;
    dev.spi_cs_cb = NULL;
    dev.spi_rx_cb = NULL;
    dev.spi_tx_cb = NULL;

    ret = SPI_HAL_Open(&dev);
    DthingTraceD("[INFO][SPI] do open bus %d with rate%d with result %d\n", busId, rate, ret);
#endif

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_spi_SPIManager
 * Method:    getRate0
 * Signature: (I)I
 */
void Java_iot_oem_spi_SPIManager_getRate0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint busId = (jint) args[1];
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    uint32 freq = 0;
    ret = SPI_HAL_Ioctl(busId, SPI_CTL_G_FREQ, &freq);

    if (ret >= 0)
    {
        ret = freq;
    }

    DthingTraceD("[INFO][SPI] do get rate %d\n", ret);
#endif

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_spi_SPIManager
 * Method:    setRate0
 * Signature: (II)I
 */
void Java_iot_oem_spi_SPIManager_setRate0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint busId = (jint) args[1];
    jint rate = (jint) args[2];
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    uint32 freq = rate;

    ret = SPI_HAL_Ioctl(busId, SPI_CTL_S_FREQ, &freq);
    DthingTraceD("[INFO][SPI] do set rate %d with result %d\n", rate, ret);
#endif

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_spi_SPIManager
 * Method:    close0
 * Signature: (I)I
 */
void Java_iot_oem_spi_SPIManager_close0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint busId = (jint) args[1];
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    ret = SPI_HAL_Close(busId);
    DthingTraceD("[INFO][SPI] do close busId %d with result %d\n", busId, ret);
#endif

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_spi_SPIManager
 * Method:    read0
 * Signature: (II[BI)I
 */
void Java_iot_oem_spi_SPIManager_read0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint busId = (jint) args[1];
    jint addr = (jint) args[2];
    ArrayObject * bufferArr = (ArrayObject *)args[3];
    jbyte * bufferArrPtr = (jbyte *)(KNI_GET_ARRAY_BUF(args[3]));
    int bufferArrLen = KNI_GET_ARRAY_LEN(args[3]);
    jint len = (jint) args[4];
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    //TODO: How to use the address
    if (bufferArrPtr != NULL)
    {
        ret = SPI_HAL_Read(busId, (uint8 *)bufferArrPtr, len);
    }
    DthingTraceD("[INFO][SPI] do read data with result %d\n", ret);
#endif

    RETURN_INT(ret);
}

/**
 * Class:     iot_oem_spi_SPIManager
 * Method:    write0
 * Signature: (II[BI)I
 */
void Java_iot_oem_spi_SPIManager_write0(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    jint busId = (jint) args[1];
    jint addr = (jint) args[2];
    ArrayObject * dataArr = (ArrayObject *)args[3];
    jbyte * dataArrPtr = (jbyte *)(KNI_GET_ARRAY_BUF(args[3]));
    int dataArrLen = KNI_GET_ARRAY_LEN(args[3]);
    jint len = (jint) args[4];
    jint ret = 0;

#if defined(ARCH_ARM_SPD)
    //TODO: How to use the address
    if (dataArrPtr != NULL)
    {
        ret = SPI_HAL_Write(busId, (uint8 *)dataArrPtr, len);
    }
    DthingTraceD("[INFO][SPI] do wirte data with result %d\n", ret);
#endif

    RETURN_INT(ret);
}

