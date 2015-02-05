/******************************************************************************
 ** File Name:     spiflash_cfg.c                                              *
 ** Description:														 	 *
 ** This file contains spiflash config info                                  *
 ** Author:         Fei.Zhang                                              	 *
 ** DATE:           01/18/2011                                               *
 ** Copyright:      2011 Spreadtrum, Incoporated. All Rights Reserved.       *
 ** Description:                                                             *
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                      *
 ** -------------------------------------------------------------------------*
 ** DATE           NAME             DESCRIPTION                              *
 ** 01/18/2011      Fei.Zhang       Create									 *
 ******************************************************************************/


/**---------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **---------------------------------------------------------------------------*/
#include "spiflash.h"
/*lint -save -e570 */
//#include "emc_drv.h"
/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    extern   "C" 
    {
#endif

/**---------------------------------------------------------------------------*
 **                         Macro defines                                     
 **---------------------------------------------------------------------------*/


/**--------------------------------------------------------------------------*
 **                         Data Structures                                  *
 **--------------------------------------------------------------------------*/ 


/**--------------------------------------------------------------------------*
 **                         Constant DEFINITION                                *
 **--------------------------------------------------------------------------*/ 

/**--------------------------------------------------------------------------*
 **                         STATIC DEFINITION                                *
 **--------------------------------------------------------------------------*/

/**--------------------------------------------------------------------------*
 **                         EXTERNAL DEFINITION                              *
 **--------------------------------------------------------------------------*/



/**--------------------------------------------------------------------------*
 **                         CUSTOMERIZE DEFINITION                           *
 **--------------------------------------------------------------------------*/  

/**---------------------------------------------------------------------------	*
 **                          cfg struct 1 :config flash/sram to emc 			*
 **---------------------------------------------------------------------------	*/
 


//*********** WARNING: PLEASE CONSULT SPREADTRUM SUPPORT ENGINEER BEFORE MODIFYING THIS FILE !***********
#define  PHY_FLASH_SIZE                MAX_HW_FLASH_SIZE
#define  FLASH_SECTOR_SIZE             0x00008000

#define  MAX_FLASH_SIZE   (PHY_FLASH_SIZE + DEBUG_FLASH_SIZE)

#ifdef  NV_PRTITION_TINY
	#define  RUNNIN_NV_SECTOR_NUM       4    //128KB
#elif  defined(NV_PRTITION_TINY_EX)	
	#define  RUNNIN_NV_SECTOR_NUM       5    //160KB
#elif defined(NV_PRTITION_SMALL)
	#define  RUNNIN_NV_SECTOR_NUM       7    //224KB
#elif defined(NV_PRTITION_NORMAL)
	#define  RUNNIN_NV_SECTOR_NUM       10    //320KB
#elif defined(NV_PRTITION_LARGE)
	#define  RUNNIN_NV_SECTOR_NUM       15   //480KB
#else
	#define  RUNNIN_NV_SECTOR_NUM       10   //320KB
#endif

#ifdef  UMEM_PRTITION_MICRO
	#define  UMEM_SECTOR_NUM            3    //96KB
#elif defined(UMEM_PRTITION_TINY)
	#define  UMEM_SECTOR_NUM            4    //128KB
#elif defined(UMEM_PRTITION_SMALL)
	#define  UMEM_SECTOR_NUM            5    //160KB
#elif defined(UMEM_PRTITION_NORMAL)
	#define  UMEM_SECTOR_NUM            6    //192KB
#elif defined(UMEM_PRTITION_LARGE)
	#define  UMEM_SECTOR_NUM            7    //224KB
#else
	#define  UMEM_SECTOR_NUM            4   //128KB
#endif


#if defined(UMEM_SUPPORT) && defined(UMEM_NEW_MECHNISM)
#define RUNNIN_NV_SECTOR_COUNT    (UMEM_SECTOR_NUM + RUNNIN_NV_SECTOR_NUM)
#else
#define RUNNIN_NV_SECTOR_COUNT     RUNNIN_NV_SECTOR_NUM
#endif

#define RUNNIN_NV_SIZE          (RUNNIN_NV_SECTOR_COUNT*FLASH_SECTOR_SIZE)
#define FIXED_NV_SIZE           0x6C00   //27KB

#define RUNNIN_NV_BASE_ADDR     (MAX_FLASH_SIZE - RUNNIN_NV_SIZE)
#define FIXED_NV_BASE_ADDR      (RUNNIN_NV_BASE_ADDR - FLASH_SECTOR_SIZE)
#define PRODUCT_NV_BASE_ADDR    (FIXED_NV_BASE_ADDR + FIXED_NV_SIZE)

#ifdef  UMEM_SUPPORT
#define UDISK_SIZE              (UMEM_SECTOR_NUM*FLASH_SECTOR_SIZE)
#define UDISK_BASE_ADDR         (MAX_FLASH_SIZE - UDISK_SIZE)
#else
#define UDISK_SIZE                       (-1)
#define UDISK_BASE_ADDR                  (-1)
#endif

#ifdef SIM_LOCK_SUPPORT 
#define SIM_LOCK_SECTOR_NUM       1
#define SIM_LOCK_REGION_SIZE      (SIM_LOCK_SECTOR_NUM*FLASH_SECTOR_SIZE)
#else
#ifdef  MMI_RES_PRTITION_TINY
	#define  MMI_RES_SECTOR_NUM          50
#elif defined(MMI_RES_PRTITION_SMALL)
	#define  MMI_RES_SECTOR_NUM          100
#elif defined(MMI_RES_PRTITION_NORMAL)
	#define  MMI_RES_SECTOR_NUM          150
#elif defined(MMI_RES_PRTITION_LARGE)
	#define  MMI_RES_SECTOR_NUM          200
#else
	#define  MMI_RES_SECTOR_NUM          0
#endif
#endif
/*
#ifdef STONE_IMAGE_SUPPORT
#ifdef SIM_LOCK_SUPPORT 
	#define MMI_RES_SIZE                SIM_LOCK_REGION_SIZE
	#ifdef  UMEM_SUPPORT
		#define MMI_RES_BASE_ADDR       (UDISK_BASE_ADDR - SIM_LOCK_REGION_SIZE) 
	#else
		#define MMI_RES_BASE_ADDR       (FIXED_NV_BASE_ADDR - SIM_LOCK_REGION_SIZE) 
	#endif
#else
    #define MMI_RES_BASE_ADDR    -1
#endif	
#else
	#define MMI_RES_SIZE                (MMI_RES_SECTOR_NUM*FLASH_SECTOR_SIZE)
	#ifdef  UMEM_SUPPORT
		#define MMI_RES_BASE_ADDR       (UDISK_BASE_ADDR - MMI_RES_SIZE) 
	#else
		#define MMI_RES_BASE_ADDR       (FIXED_NV_BASE_ADDR - MMI_RES_SIZE) 
	#endif
#endif //STONE_IMAGE_SUPPORT 
*/
#ifdef MODEM_PLATFORM
    #define MMI_RES_BASE_ADDR          (-1)
#endif
#ifdef NOR_BOOTLOADER_SUPPORT
    #define PS_BASE_ADDR            0x10000  
#endif


#define  FLASH_SECTOR_SIZE_B             0x00010000


#ifdef  NV_PRTITION_TINY
	#define  RUNNIN_NV_SECTOR_NUM_B     2//  4    //128KB
#elif  defined(NV_PRTITION_TINY_EX)	
	#define  RUNNIN_NV_SECTOR_NUM_B     3//  5    //160KB
#elif defined(NV_PRTITION_SMALL)
	#define  RUNNIN_NV_SECTOR_NUM_B     4//  7    //224KB
#elif defined(NV_PRTITION_NORMAL)
	#define  RUNNIN_NV_SECTOR_NUM_B     5//  10    //320KB
#elif defined(NV_PRTITION_LARGE)
	#define  RUNNIN_NV_SECTOR_NUM_B     8//  15   //480KB
#else
	#define  RUNNIN_NV_SECTOR_NUM_B     5//  10   //320KB
#endif

#ifdef  UMEM_PRTITION_MICRO
	#define  UMEM_SECTOR_NUM_B          2//  3    //96KB
#elif defined(UMEM_PRTITION_TINY)
	#define  UMEM_SECTOR_NUM_B          2//  4    //128KB
#elif defined(UMEM_PRTITION_SMALL)
	#define  UMEM_SECTOR_NUM_B          3// 5    //160KB
#elif defined(UMEM_PRTITION_NORMAL)
	#define  UMEM_SECTOR_NUM_B          3//  6    //192KB
#elif defined(UMEM_PRTITION_LARGE)
	#define  UMEM_SECTOR_NUM_B          4//  7    //224KB
#else
	#define  UMEM_SECTOR_NUM_B           2// 4    //128KB
#endif


#if defined(UMEM_SUPPORT) && defined(UMEM_NEW_MECHNISM)
#define RUNNIN_NV_SECTOR_COUNT_B    (UMEM_SECTOR_NUM_B + RUNNIN_NV_SECTOR_NUM_B)
#else
#define RUNNIN_NV_SECTOR_COUNT_B     RUNNIN_NV_SECTOR_NUM_B
#endif

#define RUNNIN_NV_SIZE_B          (RUNNIN_NV_SECTOR_COUNT_B*FLASH_SECTOR_SIZE_B)


#define RUNNIN_NV_BASE_ADDR_B     (MAX_FLASH_SIZE - RUNNIN_NV_SIZE_B)
#define FIXED_NV_BASE_ADDR_B      (RUNNIN_NV_BASE_ADDR_B - FLASH_SECTOR_SIZE_B)
#define PRODUCT_NV_BASE_ADDR_B    (FIXED_NV_BASE_ADDR_B + FIXED_NV_SIZE)

#ifdef  UMEM_SUPPORT
#define UDISK_SIZE_B              (UMEM_SECTOR_NUM_B*FLASH_SECTOR_SIZE_B)
#define UDISK_BASE_ADDR_B         (MAX_FLASH_SIZE - UDISK_SIZE_B)
#else
#define UDISK_SIZE_B                       (-1)
#define UDISK_BASE_ADDR_B                  (-1)
#endif

#ifdef SIM_LOCK_SUPPORT 
#define SIM_LOCK_SECTOR_NUM_B       1
#define SIM_LOCK_REGION_SIZE_B      (SIM_LOCK_SECTOR_NUM_B*FLASH_SECTOR_SIZE_B)
#else
#ifdef  MMI_RES_PRTITION_TINY
	#define  MMI_RES_SECTOR_NUM_B        25//  50
#elif defined(MMI_RES_PRTITION_SMALL)
	#define  MMI_RES_SECTOR_NUM_B        50//  100
#elif defined(MMI_RES_PRTITION_NORMAL)
	#define  MMI_RES_SECTOR_NUM_B        75//  150
#elif defined(MMI_RES_PRTITION_LARGE)
	#define  MMI_RES_SECTOR_NUM_B        100//  200
#else
	#define  MMI_RES_SECTOR_NUM_B        0//  0
#endif
#endif
/*
#ifdef STONE_IMAGE_SUPPORT
#ifdef SIM_LOCK_SUPPORT 
	#define MMI_RES_SIZE                SIM_LOCK_REGION_SIZE
	#ifdef  UMEM_SUPPORT
		#define MMI_RES_BASE_ADDR       (UDISK_BASE_ADDR - SIM_LOCK_REGION_SIZE) 
	#else
		#define MMI_RES_BASE_ADDR       (FIXED_NV_BASE_ADDR - SIM_LOCK_REGION_SIZE) 
	#endif
#else
    #define MMI_RES_BASE_ADDR    -1
#endif	
#else
	#define MMI_RES_SIZE                (MMI_RES_SECTOR_NUM*FLASH_SECTOR_SIZE)
	#ifdef  UMEM_SUPPORT
		#define MMI_RES_BASE_ADDR       (UDISK_BASE_ADDR - MMI_RES_SIZE) 
	#else
		#define MMI_RES_BASE_ADDR       (FIXED_NV_BASE_ADDR - MMI_RES_SIZE) 
	#endif
#endif //STONE_IMAGE_SUPPORT 
*/
#ifdef MODEM_PLATFORM
    #define MMI_RES_BASE_ADDR_B          (-1)
#endif
#ifdef NOR_BOOTLOADER_SUPPORT
    #define PS_BASE_ADDR_B            0x10000  
#endif

/******************************************/
/***********Flash LayeOut******************/
/******************************************/
/*---------------------|
  |Umem Disk           |
  |                    |
  |--------------------|
  |Running NV          |
  |                    |
  |--------------------|
  |Product Info        |
  |--------------------|
  |Fixed NV            |
  |                    |
  |--------------------|
  |Resource            |
  |                    |
  |--------------------|
  |                    |
  |PS                  |
  |--------------------|
******************************************/


#pragma arm section rodata = "SPIFLASH_CFG_TABLE_DETAIL" 
PUBLIC  const NOR_FLASH_CONFIG_T  s_platform_patitiion_config = 
{   
    1,            				/*single-bank*/                                               
    RUNNIN_NV_SECTOR_COUNT,       /*runningNV/EFS sector number*/       
    0,            				/* abandon */ 					              
    FLASH_SECTOR_SIZE,  		/*sector size*/ 					                  
    0x00000000,   	            /*start address*/      			     
    RUNNIN_NV_BASE_ADDR,        /*runningNV/EFS start address*/     
    MAX_FLASH_SIZE,   		    /*flash size*/ 					                  
    FIXED_NV_BASE_ADDR,         /*fixed nv start address*/                                    
    PRODUCT_NV_BASE_ADDR,       /*product info start address*/                                          
    MMI_RES_BASE_ADDR,          /*lint !e570 */ /*resource start address*/                       
    UDISK_BASE_ADDR,            /*lint !e570 */ /*U Disk start address	*/ 		                        
    UDISK_SIZE,                 /*lint !e570 */ /*U Disk Size*/                           
    -1,                         /*blbin*/                                    
#ifdef NOR_BOOTLOADER_SUPPORT
    PS_BASE_ADDR,     //PS Start Addr
#endif
};//lint !e570

PUBLIC  const NOR_FLASH_CONFIG_T  s_platform_patitiion_config_B = 
{   
    1,            				/*single-bank*/                                               
    RUNNIN_NV_SECTOR_COUNT_B,       /*runningNV/EFS sector number*/       
    0,            				/* abandon */ 					              
    FLASH_SECTOR_SIZE_B,  		/*sector size*/ 					                  
    0x00000000,   	            /*start address*/      			     
    RUNNIN_NV_BASE_ADDR_B,        /*runningNV/EFS start address*/     
    MAX_FLASH_SIZE,   		    /*flash size*/ 					                  
    FIXED_NV_BASE_ADDR_B,         /*fixed nv start address*/                                    
    PRODUCT_NV_BASE_ADDR_B,       /*product info start address*/                                          
    MMI_RES_BASE_ADDR_B,          /*lint !e570 */ /*resource start address*/                       
    UDISK_BASE_ADDR_B,            /*lint !e570 */ /*U Disk start address	*/ 		                        
    UDISK_SIZE_B,                 /*lint !e570 */ /*U Disk Size*/                           
    -1,                         /*blbin*/                                    
#ifdef NOR_BOOTLOADER_SUPPORT
    PS_BASE_ADDR_B,     //PS Start Addr
#endif
};//lint !e570
 
#define FLASH_LOGIC_PATITION   &s_platform_patitiion_config
#define FLASH_LOGIC_PATITION_B   &s_platform_patitiion_config_B

static struct SPIFLASH_ExtCfg cfg_GD25LQ128 = {SPIFLASH_VOLTAGE_1V8, 0x01000000,0,0,0,0,0};
static struct SPIFLASH_ExtCfg cfg_GD25LQ64B = {SPIFLASH_VOLTAGE_1V8, 0x00800000,0,0,0,0,0};
static struct SPIFLASH_ExtCfg cfg_GD25LQ32B = {SPIFLASH_VOLTAGE_1V8, 0x00400000,0,0,0,0,0};
static struct SPIFLASH_ExtCfg cfg_GD25LQ16B = {SPIFLASH_VOLTAGE_1V8, 0x00200000,0,0,0,0,0};
static struct SPIFLASH_ExtCfg cfg_GD25Q64B = {SPIFLASH_VOLTAGE_3V, 0x00800000,0,0,0,0,0};
static struct SPIFLASH_ExtCfg cfg_GD25Q32B = {SPIFLASH_VOLTAGE_3V, 0x00400000,0,0,0,0,0};
static struct SPIFLASH_ExtCfg cfg_N25W128 = {SPIFLASH_VOLTAGE_1V8, 0x01000000,0,0,0,0,0};

static struct SPIFLASH_ExtCfg cfg_N25W064 = {SPIFLASH_VOLTAGE_1V8, 0x00800000,0,0,0,0,0};

static struct SPIFLASH_ExtCfg cfg_MX25U3235E = {SPIFLASH_VOLTAGE_1V8, 0x00400000,0,0,0,0,0};
static struct SPIFLASH_ExtCfg cfg_MX25U6435E = {SPIFLASH_VOLTAGE_1V8, 0x00800000,0,0,0,0,0};
static struct SPIFLASH_ExtCfg cfg_MX25U12835E = {SPIFLASH_VOLTAGE_1V8, 0x01000000,0,0,0,0,0};
static struct SPIFLASH_ExtCfg cfg_W25Q64FW = {SPIFLASH_VOLTAGE_1V8, 0x00800000,0,0,0,0,0};
static struct SPIFLASH_ExtCfg cfg_W25Q32FV = {SPIFLASH_VOLTAGE_3V, 0x00400000,0,0,0,0,0};
static struct SPIFLASH_ExtCfg cfg_W25Q64FVSTIM = {SPIFLASH_VOLTAGE_3V, 0x00800000,0,0,0,0,0};
static struct SPIFLASH_ExtCfg cfg_W25Q128FV = {SPIFLASH_VOLTAGE_3V, 0x01000000,0,0,0,0,0};
static struct SPIFLASH_ExtCfg cfg_W25Q256FV = {SPIFLASH_VOLTAGE_3V, 0x02000000,0,0,0,0,0};
static struct SPIFLASH_ExtCfg cfg_FM25Q32A = {SPIFLASH_VOLTAGE_3V, 0x00400000,0,0,0,0,0};
static struct SPIFLASH_ExtCfg cfg_FM25M32A = {SPIFLASH_VOLTAGE_1V8, 0x00400000,0,0,0,0,0};

#pragma arm section

#define _NULL 0xFF

#pragma arm section rodata = "SPIFLASH_CFG_TABLE" 

LOCAL const SPIFLASH_SPEC_T SpiFlashSpec[] = {
    /***************************************************************************/
    /* nMID, nDID1, nDID2, nQEPos, nWIPPos, EnterQPI, ExitQPI, cmd_4pp, cmd_4read, suspend, resume, reset_support, QPI_support, spiflash_cfg, pExtcfg*/
    /***************************************************************************/
    
    { 0xC8, 0x60, 0x18, 0x09, 0x00, 0x38, 0xFF, 0x32, 0xEB, 0x75, 0x7A, TRUE, TRUE, (NOR_FLASH_CONFIG_PTR)FLASH_LOGIC_PATITION, &cfg_GD25LQ128},//GD25LQ128
    { 0xC8, 0x60, 0x17, 0x09, 0x00, 0x38, 0xFF, 0x32, 0xEB, 0x75, 0x7A, TRUE, TRUE, (NOR_FLASH_CONFIG_PTR)FLASH_LOGIC_PATITION, &cfg_GD25LQ64B},//GD25LQ64B
    { 0xC8, 0x60, 0x16, 0x09, 0x00, 0x38, 0xFF, 0x32, 0xEB, 0x75, 0x7A, TRUE, TRUE, (NOR_FLASH_CONFIG_PTR)FLASH_LOGIC_PATITION, &cfg_GD25LQ32B},//GD25LQ32B
    { 0xC8, 0x60, 0x15, 0x09, 0x00, 0x38, 0xFF, 0x32, 0xEB, 0x75, 0x7A, TRUE, TRUE, (NOR_FLASH_CONFIG_PTR)FLASH_LOGIC_PATITION, &cfg_GD25LQ16B},//GD25LQ16

    { 0xC8, 0x40, 0x17, 0x09, 0x00, _NULL, _NULL, 0x32, 0x6B, 0x75, 0x7A, FALSE, FALSE, (NOR_FLASH_CONFIG_PTR)FLASH_LOGIC_PATITION, &cfg_GD25Q64B},//GD25Q64B
    { 0xC8, 0x40, 0x16, 0x09, 0x00, _NULL, _NULL, 0x32, 0x6B, 0x75, 0x7A, FALSE, FALSE, (NOR_FLASH_CONFIG_PTR)FLASH_LOGIC_PATITION, &cfg_GD25Q32B},//GD25Q32B

//    { 0x2C, 0xCB, 0x18, 0xFF, 0x00, _NULL, _NULL, 0x12, 0xEB, 0x75, 0x7A, TRUE, TRUE, (NOR_FLASH_CONFIG_PTR)FLASH_LOGIC_PATITION, &cfg_N25W128},//N25W128
//    { 0x2C, 0xCB, 0x17, 0xFF, 0x00, _NULL, _NULL, 0x12, 0xEB, 0x75, 0x7A, TRUE, TRUE, (NOR_FLASH_CONFIG_PTR)FLASH_LOGIC_PATITION, &cfg_N25W064},//N25W064
    { 0x2C, 0xCB, 0x18, 0xFF, 0x00, _NULL, _NULL, 0x12, 0xEB, 0x75, 0x7A, TRUE, TRUE, (NOR_FLASH_CONFIG_PTR)FLASH_LOGIC_PATITION_B, &cfg_N25W128},//N25W128
    { 0x2C, 0xCB, 0x17, 0xFF, 0x00, _NULL, _NULL, 0x12, 0xEB, 0x75, 0x7A, TRUE, TRUE, (NOR_FLASH_CONFIG_PTR)FLASH_LOGIC_PATITION_B, &cfg_N25W064},//N25W064
    
    { 0xC2, 0x25, 0x38, 0x06, 0x00, 0x35, 0xF5, 0x38, 0xEB, 0xB0, 0x30, TRUE, TRUE, (NOR_FLASH_CONFIG_PTR)FLASH_LOGIC_PATITION, &cfg_MX25U12835E},//MX25U12835E
    { 0xC2, 0x25, 0x37, 0x06, 0x00, 0x35, 0xF5, 0x38, 0xEB, 0xB0, 0x30, TRUE, TRUE, (NOR_FLASH_CONFIG_PTR)FLASH_LOGIC_PATITION, &cfg_MX25U6435E},//MX25U6435E
    { 0xC2, 0x25, 0x36, 0x06, 0x00, 0x35, 0xF5, 0x38, 0xEB, 0xB0, 0x30, TRUE, TRUE, (NOR_FLASH_CONFIG_PTR)FLASH_LOGIC_PATITION, &cfg_MX25U3235E},//MX25U3235E   
    
    { 0xEF, 0x60, 0x17, 0x09, 0x00, 0x38, 0xFF, 0x32, 0xEB, 0x75, 0x7A, TRUE, TRUE, (NOR_FLASH_CONFIG_PTR)FLASH_LOGIC_PATITION, &cfg_W25Q64FW},//W25Q64FW
    { 0xEF, 0x40, 0x16, 0x09, 0x00, 0x38, 0xFF, 0x32, 0xEB, 0x75, 0x7A, TRUE, TRUE, (NOR_FLASH_CONFIG_PTR)FLASH_LOGIC_PATITION, &cfg_W25Q32FV},//W25Q32FV   
    { 0xEF, 0x40, 0x17, 0x09, 0x00, 0x38, 0xFF, 0x32, 0x0B, 0x75, 0x7A, TRUE, TRUE, (NOR_FLASH_CONFIG_PTR)FLASH_LOGIC_PATITION, &cfg_W25Q64FVSTIM},//W25Q64FVSTIM
    { 0xEF, 0x40, 0x18, 0x09, 0x00, 0x38, 0xFF, 0x32, 0x0B, 0x75, 0x7A, TRUE, TRUE, (NOR_FLASH_CONFIG_PTR)FLASH_LOGIC_PATITION, &cfg_W25Q128FV},//W25Q128FV
    { 0xEF, 0x40, 0x19, 0x09, 0x00, 0x38, 0xFF, 0x32, 0x0B, 0x75, 0x7A, TRUE, TRUE, (NOR_FLASH_CONFIG_PTR)FLASH_LOGIC_PATITION, &cfg_W25Q256FV},//W25Q256FV

    { 0xF8, 0x32, 0x16, 0x09, 0x00, _NULL, _NULL, 0x38, 0xEB, 0x75, 0x7A, FALSE, FALSE, (NOR_FLASH_CONFIG_PTR)FLASH_LOGIC_PATITION, &cfg_FM25Q32A},//FM25Q32, SC2112C-V
    
    { 0xF8, 0x42, 0x16, 0x09, 0x00, 0x38, 0xFF, 0x33, 0xEB, 0x75, 0x7A, TRUE, TRUE, (NOR_FLASH_CONFIG_PTR)FLASH_LOGIC_PATITION, &cfg_FM25M32A},//FM25M32

    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, FALSE, FALSE, NULL, NULL}
};

#pragma arm section

#pragma arm section code = "SPIFLASH_CFG_CODE"
SPIFLASH_SPEC_T *SPIFLASH_SPEC_Get(uint16 nMID, uint16 nDID1, uint16 nDID2)
{
   uint32 i;
   
   for(i=0; SpiFlashSpec[i].nMID!=0x0; i++)
   {
       if((nDID1 == SpiFlashSpec[i].nDID1) && (nMID == SpiFlashSpec[i].nMID) && (nDID2 == SpiFlashSpec[i].nDID2))
       {
           //Find Nand Type
           return (SPIFLASH_SPEC_T *)&SpiFlashSpec[i];
       }
   }
   //Not find
   return NULL;
}
#pragma arm section code


#pragma arm section rodata = "DFILE_DOWNLOAD_CFG"
const DFILE_CONFIG_T  s_download_file_config =
{
   DFILE_MAGIC_FIRST,
   DFILE_MAGIC_SECOND, 	
   DFILE_INVAILD_ADDR,
   DFILE_INVAILD_ADDR,
   DFILE_INVAILD_ADDR,
   DFILE_INVAILD_ADDR,	
   DFILE_INVAILD_ADDR,
   DFILE_INVAILD_ADDR,
   DFILE_INVAILD_ADDR,
   DFILE_INVAILD_ADDR,
   DFILE_INVAILD_ADDR,
   DFILE_INVAILD_ADDR,
   DFILE_INVAILD_ADDR,
   DFILE_INVAILD_ADDR,
   DFILE_INVAILD_ADDR,
   DFILE_INVAILD_ADDR,
   DFILE_INVAILD_ADDR,
   DFILE_INVAILD_ADDR,
   DFILE_INVAILD_ADDR,
   DFILE_INVAILD_ADDR,
   DFILE_INVAILD_ADDR,
   DFILE_INVAILD_ADDR,
   DFILE_INVAILD_ADDR,
   DFILE_INVAILD_ADDR,
   DFILE_INVAILD_ADDR,
   DFILE_INVAILD_ADDR,
   DFILE_INVAILD_ADDR,
   DFILE_INVAILD_ADDR,
   DFILE_INVAILD_ADDR,
   DFILE_INVAILD_ADDR,
   DFILE_INVAILD_ADDR,    	
   DFILE_MAGIC_END	
};

#pragma arm section rodata


#pragma arm section rodata = "SPIFLASH_TYPE_CFG"
const SPIFLASH_TYPE_CONFIG_T  s_spiflash_type_config =
{
   SPIFLASH_MAGIC_FIRST,
   SPIFLASH_MAGIC_SECOND, 	
   SPIFLASH_INVAILD_DATA,	
   SPIFLASH_INVAILD_DATA,
   SPIFLASH_INVAILD_DATA,
   SPIFLASH_INVAILD_DATA,
   SPIFLASH_INVAILD_DATA,
   SPIFLASH_INVAILD_DATA,
   SPIFLASH_INVAILD_DATA,
   SPIFLASH_INVAILD_DATA,
   SPIFLASH_INVAILD_DATA,
   SPIFLASH_MAGIC_END	
};

#pragma arm section rodata

/*****************************************************************************/
//  Description :
//      Get the DFILE relative information of ps/res bin files.
//  Global resource dependence :
//  Input  : None
//  Output : None
//  Return : the pointer for the relative information of ps/bin files 
//  Author : 
//  Note   : Only for the combination of the bin files 
//
/*****************************************************************************/
PUBLIC DFILE_CONFIG_T*  SPIFLASH_GetDFileConfig(void)
{
   if( (DFILE_MAGIC_FIRST == s_download_file_config.magic_first) &&
 	   (DFILE_MAGIC_SECOND == s_download_file_config.magic_second) &&
 	   (DFILE_MAGIC_END == s_download_file_config.magic_end))
   	{
        return (DFILE_CONFIG_T* )&s_download_file_config;
    }
    else
    {
    	return NULL;
    }
}

PUBLIC SPIFLASH_TYPE_CONFIG_T*  SPIFLASH_GetFlashType(void)
{
   if( (SPIFLASH_MAGIC_FIRST == s_spiflash_type_config.magic_first) &&
 	   (SPIFLASH_MAGIC_SECOND == s_spiflash_type_config.magic_second) &&
 	   (SPIFLASH_MAGIC_END == s_spiflash_type_config.magic_end))
   	{
        return (SPIFLASH_TYPE_CONFIG_T* )&s_spiflash_type_config;
    }
    else
    {
    	return NULL;
    }
}

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef   __cplusplus
    }
#endif 
// end flash_cfg.c
