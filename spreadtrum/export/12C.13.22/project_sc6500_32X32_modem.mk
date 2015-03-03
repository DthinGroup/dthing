# *************************************************************************
# Feature Options (For User)
# !!!!!!!! The customer value can be modified in this option part.!!!!!!!!!
# !!!!!!!! If modification is in need,please find support from SPRD CPM or FAE.!!!!!!!!!
# !!!!!!!! 此部分Options 的值客户一般可以修改.!!!!!!!!!
# !!!!!!!! 此部分修改，最好与展讯客户经理(CPM)或技术支持工程师(FAE)沟通寻求支持.!!!!!!!!!
# *************************************************************************

MULTI_BIN_RAM = FALSE                                ## Option1  32MBIT_LEVEL1   ----16x32
                                                     ## Option2  32MBIT_LEVEL2   ----32x32
                                                     ## Option3  32MBIT_LEVEL3   ----32x32

RELEASE_INFO = TRUE                                 ### RELEASE version
                                                     # Option1:TRUE
                                                     # Option2:FALSE

FLASH_TYPE = SPI                                     ### FLASH TYPE
                                                     # Option1:NOR        #define NOR_FLASH
                                                     # Option2:SPI

FLASH_SIZE = 32MBIT                                  ### FLASH_SIZE SIZE
                                                     # Option1:NONE       #FALSH_SIZE_NONE
                                                     # Option2:16MBIT     #define FLASH_SIZE_16MBIT
                                                     # Option3:32MBIT     #define FLASH_SIZE_32MBIT
                                                     # Option4:64MBIT     #define FLASH_SIZE_64MBIT
                                                     # Option5:128MBIT    #define FLASH_SIZE_128MBIT

                                                     
SRAM_SIZE = 32MBIT                                   ### SRAM SIZE
                                                     # Option1:NONE       #SRAM_SIZE_NONE
                                                     # Option2:16MBIT     #define SRAM_SIZE_16MBIT
                                                     # Option3:32MBIT     #define SRAM_SIZE_32MBIT
                                                     # Option4:64MBIT     #define SRAM_SIZE_64MBIT

VDDCORE_SUPPLY = LDO                                ### VDDCORE_SUPPLY
                                                     # Option1:DCDC     #define VDDCORE_SUPPLY_DCDC
                                                     # Option2:LDO     #define VDDCORE_SUPPLY_LDO
CUSTOM_CFLAG = -g                                    ### CUSTOM CFLAG
                                                     # Option1:-g

MMI_MULTI_SIM_SYS = SINGLE                             ### Platform Support Multi Sim Card Number
                                                     # Option1:SINGLE    #define MMI_MULTI_SIM_SYS_SINGLE
                                                     # Option2:DUAL      #define MMI_MULTI_SIM_SYS_DUAL
                                                     # Option3:TRI       #define MMI_MULTI_SIM_SYS_TRI
                                                     # OPtion4:QUAD      #define MMI_MULTI_SIM_SYS_QUAD
                                                     
                                                     
PLATFORM_INTERNAL_SIM = TRUE                        ###6500M internal sim card
                                                     # Option1:TRUE
                                                     # Option2:FALSE
                                                     
SIM_CDMA_CARD_SUPPORT = FALSE                        ### CDMA UIM Card Support. Only for CDMA+GSM modem project.
                                                     # Option1:TRUE
                                                     # Option2:FALSE

MULTI_SIM_SYS_QUAD_TO = NONE                         ### Platform Support Multi Sim Card Number
                                                     # Option1:DUAL_WITHOUT6302
                                                     # Option2:TRI_SINGLE6302_BBSIM0
                                                     # Option3:TRI_SINGLE6302_BBSIM1
                                                     # Option4:NONE

DUAL_BATTERY_SUPPORT = FALSE                         ### Dual battery Support
                                                     # Option2:FALSE

SIM_PLUG_IN_SUPPORT = FALSE                          ### Platform Support SIM HOT PLUGIN
                                                     # Option2:FALSE

GPIO_SIMULATE_SPI_SUPPORT = FALSE                    ### To simulate SPI by GPIO
                                                     # Option2:FALSE

GPIO_SPI_SUPPORT = NONE                              ### Quadsim SPI by GPIO mode:
                                                     # Option1:NONE                 #define GPIO_SPI_SUPPORT_NONE



BB_DRAM_TYPE = NONE                                  ### DRAM Type
                                                     # Option1:NONE           #define BB_DRAM_TYPE_NONE
                                                     # Option1:32MB_16BIT     #define BB_DRAM_TYPE_32MB_16BIT
                                                     # Option2:32MB_32BIT_13R #define BB_DRAM_TYPE_32MB_32BIT_13R
                                                     # Option3:32MB_32BIT_12R #define BB_DRAM_TYPE_32MB_32BIT_12R
                                                     # Option4:64MB_16BIT     #define BB_DRAM_TYPE_64MB_16BIT
                                                     # Option5:64MB_32BIT     #define BB_DRAM_TYPE_64MB_32BIT
PRELOAD_SUPPORT = FALSE                              ### pre-load support
                                                     # Option2:FALSE

WIFI_SUPPORT = NONE                                  ### wifi Support
                                                     # Option1:NONE           #define WIFI_SUPPORT_NONE
                                                     # Option2:CSR6030        #define WIFI_SUPPORT_CSR6030
                                                     # Option3:CSR6027A05     #define WIFI_SUPPORT_CSR6027A05
                                                     # Option4:CSR6027B07     #define WIFI_SUPPORT_CSR6027B07
                                                     # Option5:RTL8723        #defube WIFI_SUPPORT_RTL8723
                                                     # Option6:RTL8189        #defube WIFI_SUPPORT_RTL8189
DEV_MANAGE_SUPPORT = FALSE                           ### device manage support
                                                     # Option1:TRUE
                                                     # Option2:FALSE

QQ_SUPPORT = NONE

PRODUCT_CONFIG = sc6500_modem                        ### MS_Customize/source/product/config;scf file;nv;resource
                                                     # default: project name

PRODUCT_BASELIB_DIR = sc6500_16X32_modem_single_notrace                     ### Dir name for base lib in MS_Code\lib\
                                                     # default: project name

BLUETOOTH_SUPPORT = NONE                             ### Bluetooth Chip Type:
                                                     # Option1:NONE      #define BLUETOOTH_SUPPORT_NONE
                                                     # Option2:CSR_BC4_6 #define BLUETOOTH_SUPPORT_CRS_BC4_6
                                                     # Option3:CSR_BC4_6_LOW_MEM: low mem and code size , only support OPP HFG
                                                     # Option4:RTL8723   Realtek RTL8723AS wifi/bt combo chip
                                                     # Option5:IS1662    ISSC IS1662S bt chip
                                                     # Option6:SPRD_BT			#define BLUETOOTH_SUPPORT_SPRD_BT

MULTI_SIM_GSM_CTA_SUPPORT = FALSE                    ### MULTI SIM GSM CTA Support
                                                     # Option1:TRUE
                                                     # Option2:FALSE


DEMOD_HW = NONE                                      ### demod chip type:
                                                     # Option1:NONE
                                                     # Option1:SIANO      DEMOD_HW_SIANO
                                                     # Option2:INNOFIDEI  DEMOD_HW_INNOFIDEI

MEMORY_DEBUG_SUPPORT = FALSE                         ### MEMORY DEBUG SUPPORT:
                                                     # Option1:TRUE
                                                     # Option2:FALSE

TRACE_INFO_SUPPORT = FALSE                          ### TRACE INFO SUPPORT:
                                                     # Option1:TRUE
                                                     # Option2:FALSE

TRACE_TYPE = ID                                      ### TRACE TYPE SUPPORT:
                                                     # Option1:CLASSIC
                                                     # Option2:ID

TIMER_MSG_SUPPORT = FALSE                             ### time message

OSC32K_CLOCK_SELECT = DCXO                           ### 是否使用外部晶体
                                                     # Option1:DCXO 去掉外部32K晶体
                                                     # Option2:EXTE 使用外部32K晶体
                                                     
EDL_UDISK_IMG_SUPPORT = TRUE                          ### EDL_UDISK_IMG_SUPPORT:
                                                     # Option1:TRUE
                                                     # Option2:FALSE                                                     
M2M_EVB_SUPPORT = TRUE
#****************************************************************************************#
#  Segment: SPRD MACRO
#  Description: Get all sprd macro list
#****************************************************************************************#
SPRD_MACRO_LIST_START := $(call GET_SPRD_MACRO_START)

# *************************************************************************
# Feature Options FOR SPRD
# !!!!!!!! Attention:please don't modify the customer value of this option part. !!!!!!!!!
# !!!!!!!! If modification is in need, the new version should be released by SPRD. !!!!!!!!!
# !!!!!!!! 注意: 此部分Options 的取值客户请勿修改!!!!!!!!!
# !!!!!!!! 若需要修改，需展讯重新发布版本!!!!!!!!!
# *************************************************************************

SPRD_CUSTOMER = TRUE                                ### SPRD Customer
                                                     # Option1:TRUE
                                                     # Option2:FALSE

SPRD_APP_RELEASE = FALSE                             ### SPRD APP RELEASE
                                                     # Option1:TRUE
                                                     # Option2:FALSE

PLATFORM = SC6530                                    ### Chip Platform Definition:
                                                     # Option1:SC6530   #define PLATFORM_SC6530

CHIP_VER = 6530                                      ### Chip version:
                                                     # Option1:6800H1     #define CHIP_VER_6800H1
                                                     # Option2:6800H3     #define CHIP_VER_6800H3

CHIP_DSP = 6500	                                     #6500：  CHIP_DSP = 6500
                                                     #6530：  CHIP_DSP = 6530
                                                     #6531:   CHIP_DSP = 6531
                                                     #考虑到6530已量产，如果不加宏的话默认使用6530的 dsp bin

FPGA_SUPPORT = FALSE                                 ### FPGA_SUPPORT_SC6530

FPGA_VERIFICATION = FALSE                            ### Debug Version
                                                     # Option1:TRUE
                                                     # Option2:FALSE

CHIP_MARK = NONE                                     ### Chip Mark Number:
                                                     # Option1:NONE       #define CHIP_MARK_NONE

CHIP_ENDIAN = LITTLE                                 ### Chip endianess:
                                                     # Option1: BIG
                                                     # Option2: LITTLE

COMPILER = RVCT                                      ### Compiler version
                                                     # Option1:ADS  ( ADS v1.2 )
                                                     # Option2:RVCT ( RVCT v2.2, RVCT v3.1 )

RVCT_VERSION = RVCT31                                ### RVCT version
                                                     # Option1:NONE
                                                     # Option2:ADS12
                                                     # Option3:RVCT22
                                                     # Option4:RVCT31

NANDBOOT_SUPPORT = FALSE                             ### Boot From Nandflash Support:
                                                     # Option1:TRUE
                                                     # Option2:FALSE

POWER_OFF_PROTECT_SUPPORT = TRUE                     ### Switch power off protect support:
                                                     # Option1:TRUE
                                                     # Option2:FALSE

USB_VCOM_AUTORUN_SUPPORT = FALSE                     ### support to instacll UCOM drivre and ewalk software:
                                                     # Option1:TRUE
                                                     # Option2:FALSE

RF_TYPE = SR528                                      ### RF Chip Type:
                                                     # Option1:QUORUM           #define RF_TYPE_QUORUM
                                                     # Option2:QUORUM518     #define RF_TYPE_QUORUM518
                                                     # Option2:SR528            #define RF_TYPE_SR528

SBC_DSP_SUPPORT = VERSION1                           ### dsp support sbc or mp3-sbc. VERSION1:sbc,VERSION2:sbc-mp3
                                                     # Option1:VERSION1      #define SBC_DSP_SUPPORT_VERSION1
                                                     # Option2:VERSION2    #define SBC_DSP_SUPPORT_VERSION2

SBC_OVERLAY = FALSE                                  ### implement sbc with memory overlay  HR L = false, M = TRUE, default = FALSE
                                                     # Option1:TRUE
                                                     # Option2:FALSE

EXPRESS_PROCESS_IN_DSP = TRUE                        ### Switch Of express process(transamplerate and sbc encode) in dsp
                                                     # Option1:TRUE
                                                     # Option2:FALSE

HALFRATE_SUPPORT = TRUE                              ### halfrate support feature
                                                     # Option1:TRUE
                                                     # Option2:FALSE

ATC_SUPPORT    = FULL                                ### ATC support feature
                                                     # Option1:FULL
                                                     # Option2:COMPACT


PRODUCT_DM = FALSE                                   ### TD/GSM DualModule product
                                                     # Option1:TRUE
                                                     # Option2:FALSE

GPRSMPDP_SUPPORT = TRUE                              ### GPRS Multi PDP support
                                                     # Option1:TRUE

VM_SUPPORT    = TRUE                                 ### VM support:
                                                     # Option1:TRUE
                                                     # Option2:FALSE
VM_TABLE_CONFIG = TINY                               ### VM_TABLE_CONFIG      
                                                     # Option1:TINY
                                                     # Option2:SMALL
                                                     # Option3:NORMAL
BZP_SUPPORT  = TRUE                                  ### new memory management,now only be used in 128X32 Nor 
                                                     # Option1:TRUE
                                                     # Option2:FALSE

FDN_SUPPORT = FALSE                                  ### FDN Support:
                                                     # Option1:TRUE
                                                     # Option2:FALSE

USB_VCOM_SUPPORT = TRUE                              ### USB Virtual COM Support:
                                                     # Option1:TRUE
                                                     # Option2:FALSE

USB_SUPPORT = TRUE                                   ### USB TXer Type:
                                                     # Option1:TRUE
                                                     # Option2:FALSE

UMEM_SUPPORT = TRUE                                  ### Switch Of nor flash udisk support
                                                     # Option1:TRUE
                                                     # Option2:FALSE

UDISK_PC_CONNECT = TRUE                              ### Norflash disk pc access support UMEM_SUPPORT MUST set TRUE
                                                     # Option1:TRUE
                                                     # Option2:FALSE                                       

TCPIP_SUPPORT = TRUE                                 ### TCPIP Profile Support:
                                                     # Option1:TRUE
                                                     
TCPIP_EXPORT  = FALSE                                ### TCPIP Code Export:
                                                     # Option1:TRUE
                                                     # Option2:FALSE                                                     
                                                     
SSL_SUPPORT = TRUE                                   ### SSL Profile Support:
                                                     # Option1:FALSE
                                                     
OMADL_SUPPORT = FALSE                                ### OMA download Support:
                                                     # Option1:TRUE
                                                     # Option2:FALSE
                                                     # work for XML_SUPPORT = TRUE && HTTP_SUPPORT = TRUE

GPRS_SUPPORT = TRUE                                  ### GRPS Support:该宏的取值用户不能修改
                                                     # Option1:TRUE
                                                     # Option2:FALSE

DRM_SUPPORT = NONE                                   ### DRM Support:ONLY VALIDATE WHEN MMI_AUTO_UPDATE_TIME_SUPPORT SET TRUE 
                                                     # Option1:NONE 
                                                     # Option2:V1 
                                                     # Option3:FL
                                                     # Option4:SD
                                                     # Option5:CD
                                                     
STACK_CHECK_SUPPORT = FALSE                          ### STACK CHECK SUPPORT:
                                                     # Option1:TRUE
                                                     # Option2:FALSE

MULTI_SIM_SYS = SINGLE                                  ### Platform Support Multi Sim Card Number:
                                                     # Option1:SINGLE    #define MULTI_SIM_SYS_SINGLE
                                                     # Option2:DUAL      #define MULTI_SIM_SYS_DUAL
                                                     # Option3:TRI       #define MULTI_SIM_SYS_TRI
                                                     # Option4:QUAD      #define MULTI_SIM_SYS_QUAD44
                                                     
                                                     
JTAG_SUPPORT = FALSE                                 ### JTAG Debug Version:
                                                     # Option1: TRUE
                                                     # Option2: FALSE

SOUNDBANK_VER = NONE                                 ### Soundbank Talbe Type
                                                     # Option1:ARM_512KB #define SOUNDBANK_VER_ARM_512KB
                                                     # Option2:ARM_2MB   #define SOUNDBANK_VER_ARM_2MB
                                                     # Option3:ARM_256KB #define SOUNDBANK_VER_ARM_256KB
                                                     # Option4:ARM_700KB #define SOUNDBANK_VER_ARM_700KB
                                                     # Option5:ARM_300KB #define SOUNDBANK_VER_ARM_300KB
                                                     # Option6:NEWMIDI_75KB #define SOUNDBANK_VER_NEWMIDI_75KB
                                                     # Option7:NEWMIDI_170KB #define SOUNDBANK_VER_NEWMIDI170KB
                                                     # Option8:NEWMIDI_400KB #define SOUNDBANK_VER_NEWMIDI_400KB


TA8LBR_SUPPORT = FALSE                               ### Switch RA8 Support:
                                                     # Option1:TRUE
                                                     # Option2:FALSE

NULL_CODEC_SUPPORT = TRUE                            ### NULL_CODEC  Support:
                                                     # Option1:TRUE
                                                     # Option2:FALSE

ARM_AD_ADP_SUPPORT = TRUE                            ### ARM_AD_ADP CODEC Support:
                                                     # Option1:TRUE
                                                     # Option2:FALSE

AD_OUTPUT_ADP_SUPPORT = TRUE                         ### AD_OUTPUT_ADP CODEC Support:
                                                     # Option1:TRUE
                                                     # Option2:FALSE

DYNAMIC_MODULE_SUPPORT = FALSE                       ### caf Support:
                                                     # Option1:TRUE
                                                     # Option2:FALSE

AUDIO_PARAM_ARM = TRUE                               #### move audio param from dsp audio nv to arm audio nv

STRM_SUPPORT_REDUCE_MEMORY = TRUE                    ### To reduce memory for low memory system
                                                     # Option1:TRUE
                                                     # Option2:FALSE

NEW_MEMORY_MM = TRUE                                 ###new memory management，now only be used in 128X32 Nor
                                                     # Option1:TRUE
                                                     # Option2:FALSE

STRM_SUPPORT_LOW_MEMORY = BUFFER                     ### streaming low memory type
                                                     # Option1:BOTH
                                                     # Option2:BUFFER     #define STRM_SUPPORT_LOW_MEMORY_BUFFER
                                                     # Option3:CONFLICT   #define STRM_SUPPORT_LOW_MEMORY_CONFLICT
                                                     # Option4:NONE


DSP_HW_COMPRESS = TRUE                               ### DSP HW Compress
                                                     # Option1:TRUE
                                                     # Option2:FALSE


SMART_PAGING_SUPPORT = FALSE                         ### Smart Paging Support Option:
                                                     # Option1:TRUE
                                                     # Option2:FALSE

ORANGE_SUPPORT = FALSE                               ### Orange Support:
                                                     # Option1:TRUE
                                                     # Option2:FALSE

NXP_LVVE_SUPPORT = FALSE                             ### NXP Support
                                                     # Option1:TRUE
                                                     # Option2:FALSE

NV_PRTITION = TINY_EX                                   ### NV Prtition
                                                     # Option1:TINY
                                                     # Option2:SMALL
                                                     # Option3:NORMAL
                                                     # Option4:LARGE
                                                     # Option5:TINY_EX																									 
																										 
SIM_LOCK_SUPPORT = FALSE                             ### SIM Lock Support 
                                                     # Option1:TRUE
                                                     # Option2:FALSE

MULTI_BIN_SUPPORT = FALSE                            ### ZIP method only open in 32x64
                                                     # Option1:TRUE
                                                     # Option2:FALSE
SPLOAD_ENABLE = FALSE                                ### lcd and sensor seperate bin dload support
                                                     # Option1:TRUE
                                                     # Option2:FALSE
UMEM_NEW_MECHNISM = TRUE			                    	 ### Switch Of nor flash udisk new mechnism,depend on UMEM_SUPPORT
                                                     # Option1:TRUE
                                                     # Option2:FALSE    
                                                     
EFS_SPEED_UP = TRUE                                  #Samsung EFS SPEED UP
                                                     # Option1: TRUE (default)
                                                     # Option1: FALSE                                                      
IMEI_OTP_SUPPORT = NONE                              ### IMEI Write protect support 
                                                     # Option1:NONE     
                                                     # Option2:EFUSE   打开一定要慎重!! 写一次IMEI就不能再写！
                                                     # depend on    MMI_MULTI_SIM_SYS_SINGLE or  MMI_MULTI_SIM_SYS_DUAL

CARD_LOG = TRUE                                     ### CARD SAVE LOG SUPPORT:
                                                     # Option1:TRUE
                                                     # Option2:FALSE 


##########################################################################################
#  OLD MACRO OR NO USE
#
##########################################################################################
ASSERT_TYPE_SUPPORT = ASSERT                         ### ASSERT TYPE SUPPORT
                                                     # Option1:ASSERT
                                                     # Option2:WARNING

NOR_SINGLEBANK_SUPPORT = FALSE                       ### Support Single Bank Type Norflash:
                                                     # Option1:FALSE
                                                     # Option1:TRUE

KURO_SUPPORT = FALSE                                 ### KURO Player Support:
                                                     # Option1:TRUE
                                                     # Option2:FALSE

LOW_MEMORY_SUPPORT = 32X32                           ### Low Memory Size Requirement:
                                                     # Option1:256X64
                                                     # Option1:128X64
                                                     # Option2:128X32
                                                     # Option3:64X64
                                                     # Option4:32X64
                                                     # Option5:16X16
                                                     # Option6:NONE

CSC_SUPPORT = FALSE                                  ### Customer Specific Customization support:
                                                     # Option1:TRUE
                                                     # Option2:FALSE

CSC_XML_SUPPORT = FALSE                              ### Customer Specific Customization XML File:
                                                     # Option1:TRUE
                                                     # Option2:FALSE
##########################################################################################
#  FOR MODEM
#
##########################################################################################
MODEM_PLATFORM = TRUE                                ### it is MODEM platform only, contain specific modules
                                                     # Option1:TRUE : for modem
                                                     # Option2:FALSE: for feature phone

MODEM_TYPE   =  MDM                                  ### Modem Type Support
                                                     # Option1:NONE  : Feature Phone Option
                                                     # Option2:MDM   : default modem
                                                     # Option3:ZBX   : For Zhi Bo Xing (Direct Broadcasting Satellite) only
                                                     # Option4:
                                                     # depend on MODEM_PLATFORM= TRUE

MUX_SUPPORT = TRUE                                   ### Mux function for AT and PPP
                                                     # Option1:TRUE
                                                     # Option2:FALSE

DUAL_MUX_SUPPORT    = FALSE                          ### Dual Mux function: two AT(data) port, no log(debug) port
                                                     # Option1:TRUE
                                                     # Option2:FALSE 

NOR_BOOTLOADER_SUPPORT  = FALSE                      ### Nor Bootloader Support
                                                     # Option1:TRUE
                                                     # Option2:FALSE

CALI_MODE_PORT = UART0                               ### Calibration MODE uart port selection
                                                     # Option1:UART1 (defaut)
                                                     # Option2:UART0 (ZBX recommand)

UART0_HWFLOW_SUPPORT = FALSE                         ### UART 0 HW Flow support 
                                                     # Option1:TURE : NOTE: it will use U0CTS/RTS, which is default as UART1_0 TX/RX, so please use UART1_PIN_SEL_UART1_1 in following MACRO
                                                     # Option2:FALSE (default)

UART1_PIN_SEL = UART1_0                              ### UART 1 pin selection
                                                     # Option1:UART1_0 (defaut) : PIN_U0CTS_REG,  PIN_U0RTS_REG
                                                     # Option2:UART1_1 :          PIN_SD_CMD_REG, PIN_SD_D0_REG

WMMP_SDK_ENABLE = TRUE                               ### wmmp sdk的编译开关，再这种情况下,pinmap 要重新映射

MODEM_WAKEUP_SUPPORT = FALSE     					           ### MODEM_WAKEUP_SUPPORT
                                                     # Option1:TRUE
                                                     # Option2:FALSE

GPS_SUPPORT = TRUE                                   ### GPS Module Support
                                                     # Option1:TRUE
                                                     # Option2:FALSE