DRIVERPATH        =  MS_Customize/source/product/driver

CONFIGPATH        =  MS_Customize/source/product/config
COMMONPATH        =  MS_Customize/source/common
MOTION_SENSOR_DRV =  FALSE

PROD_CONFIG := $(shell $(PERL) ./make/perl_script/get_dirname.pl MS_Customize/source/product/config $(PROJECT) $(strip $(PRODUCT_CONFIG)))

ifeq ($(strip $(CMMB_SUPPORT)), TRUE)
MCFLAG_OPT		+=-DSPRD_SUPPORT_MCEX
endif

ifeq ($(strip $(PLATFORM)), SC6530)
ifneq ($(strip $(SRAM_SIZE)), 16MBIT)
MCFLAG_OPT   += -DFLOWRATE_LEVEL_2
else
MCFLAG_OPT   += -DFLOWRATE_LEVEL_1
endif
endif

ifeq ($(strip $(PLATFORM)), SC6600L)
ifneq ($(strip $(WIFI_SUPPORT)), NONE)
MCFLAG_OPT   += -DFLOWRATE_LEVEL_2
else
MCFLAG_OPT   += -DFLOWRATE_LEVEL_1
endif
endif


ifeq ($(strip $(PLATFORM)), SC6600R)
ifneq ($(strip $(WIFI_SUPPORT)), NONE)
MCFLAG_OPT   += -DFLOWRATE_LEVEL_2
else
MCFLAG_OPT   += -DFLOWRATE_LEVEL_1
endif
endif


ifeq ($(strip $(PLATFORM)), SC8800G)
MCFLAG_OPT   += -DFLOWRATE_LEVEL_3
endif


MINCPATH =    $(DRIVERPATH)/fm/inc \
              $(DRIVERPATH)/fm\
              $(DRIVERPATH)/gpio/inc\
              $(DRIVERPATH)/motionsensor/inc\
              $(DRIVERPATH)/norflash/inc\
              $(DRIVERPATH)/spiflash/inc\
              $(DRIVERPATH)/ubot\
              $(DRIVERPATH)/dc/atv_tlg1120 \
              $(DRIVERPATH)/dc/atv_qn8610 \
              $(DRIVERPATH)/dc/atv_nmi601 \
              MS_Ref/source/audioservice/inc \
              MS_Ref/source/audioservice/inc_export \
              MS_Ref/source/aud_config/inc_export \
              MS_Ref/source/aud_dev/inc \
              MS_Ref/source/lcd/inc \
              $(DRIVERPATH)/norflash/normcp_info \
              MS_MMI/source/mmi_app/kernel/h \
              CMMB/export/inc\
              MS_Ref/export/inc \
              MS_Ref/source/image_proc/src/img_proc/inc \
			  connectivity/BT/export/inc

ifeq ($(strip $(DEV_MULTIUSB_SUPPORT)), TRUE)
MINCPATH +=    $(DRIVERPATH)/multi_usb
endif

MINCPATH +=   chip_drv/export/inc/outdated
MINCPATH +=   chip_drv/export/inc
MINCPATH +=$(PS_INCPATH) $(L1_INCPATH)
MSRCPATH =    $(COMMONPATH) \
              $(DRIVERPATH)/sdram \
              $(DRIVERPATH)/nandflash \
              $(DRIVERPATH)/norflash \
              $(DRIVERPATH)/spiflash \
              $(DRIVERPATH)/dc \
              $(DRIVERPATH)/tp \
              $(DRIVERPATH)/lcd \
              $(DRIVERPATH)/seine_lcd \
              $(DRIVERPATH)/autofocus \
              $(DRIVERPATH)/gpio \
              $(DRIVERPATH)/bt \
              $(DRIVERPATH)/ocr/src \
              $(DRIVERPATH)/ubot\
              $(DRIVERPATH)/power \
              $(DRIVERPATH)/motionsensor\
              $(DRIVERPATH)/dualbattery

ifeq ($(strip $(DEV_MULTIUSB_SUPPORT)), TRUE)
MSRCPATH +=    $(DRIVERPATH)/multi_usb
endif

MSRCPATH  += $(CONFIGPATH)/$(strip $(PROD_CONFIG))
MINCPATH  += $(CONFIGPATH)/$(strip $(PROD_CONFIG))

# ******************** COMMON ******************** 
ifneq ($(strip $(MODEM_PLATFORM)), TRUE)								
SOURCES	     = 	adc_parameter.c      \
              	audio_cfg.c          \
              	current_test_prod.c  \
              	production_test.c    \
              	lcd_backlight.c      \
              	prod_param.c         \
              	gpio_prod.c          \
              	gpio_ext_drv.c       \
              	gpio_ext_cfg.c       \
              	mem_prod.c           \
              	ubot_cfg_info.c	     \
              	sensor_cfg.c         \
              	dc_product_cfg.c     \
              	lcm_prod.c           \
              	tcpip_config.c       \
              	power_prod.c
else
SOURCES	     =	adc_parameter.c      \
              	audio_cfg.c          \
              	current_test_prod.c  \
              	production_test.c    \
              	prod_param.c         \
              	gpio_prod.c          \
              	gpio_ext_drv.c       \
              	gpio_ext_cfg.c       \
              	mem_prod.c           \
              	ubot_cfg_info.c	     \
              	tcpip_config.c       \
              	power_prod.c
endif

ifeq ($(strip $(IMG_EDITER_SUPPORT)), TRUE)
SOURCES +=     img_proc_cfg.c
endif

SOURCES +=      nand_PartitionMntTable.c \
		nand_vendor_sw_select.c \
		nand_spec.c


		
# ANALOG TV  DRIVER 


ifeq ($(strip $(ATV_SUPPORT)), TLG1120)
MSRCPATH  += $(DRIVERPATH)/dc/atv_tlg1120 

SOURCES	  += atv_tlg1120.c \
             I2Csample.c \
             tlg1120Api.c \
             tlg1120App.c \
             tlg1120Customize.c \
             tlg1120Mfg.c
             
endif


ifeq ($(strip $(ATV_SUPPORT)), RDA5888)
	MINCPATH	+=$(DRIVERPATH)/dc/atv_rda5888
	MSRCPATH	+=$(DRIVERPATH)/dc/atv_rda5888
	SOURCES	  += atv_rda5888.c \
	             rda5888drv.c \
	             rda5888app.c \
	             rda5888freqtbl.c \
	             rda5888fm.c \
	             rda5888sprd.c
endif

ifeq ($(strip $(ATV_SUPPORT)), QN8610)
MSRCPATH  += $(DRIVERPATH)/dc/atv_qn8610

SOURCES	  += atv_qn8610.c \
             qn8610i2c.c \
             qmaio.c \
             qmafunc.c \
             qmadriver.c \
             fmdriver.c \
             qmachmap.c
             
endif

ifeq ($(strip $(ATV_SUPPORT)), NMI601)
MSRCPATH  += $(DRIVERPATH)/dc/atv_nmi601
NMI601_CHIP_DRV = TRUE
SOURCES   += atv_nmi601.c 
endif

              	
ifeq ($(strip $(FM_SUPPORT)), NMI601)
MSRCPATH  += $(DRIVERPATH)/dc/atv_nmi601
NMI601_CHIP_DRV = TRUE
SOURCES   += fm_nmi601.c 
endif

ifeq ($(strip $(NMI601_CHIP_DRV)), TRUE)
MSRCPATH  += $(DRIVERPATH)/dc/atv_nmi601
SOURCES   += nmiapi.c \
             nmidrv_atv.c \
             nmidrv_fm.c \
             nmidrv_i2c.c \
             nmiasic.c \
             nmiport.c            
endif

ifneq ($(strip $(MODEM_PLATFORM)), TRUE) 
# IMG SENSOR  DRIVER 
SOURCES	   += sensor_ov7670.c \
              sensor_ov5640.c \
              sensor_ov2655.c \
              sensor_ov7675.c  \
              sensor_ov7690.c  \
              sensor_ov2640.c \
              sensor_gc0309.c \
              sensor_gc0307.c \
              sensor_gt2005.c \
              sensor_ov2659.c \
              sensor_sp0838.c \
              sensor_bf3703.c \
              sensor_bf3903.c \
              sensor_gc6113.c \
              sensor_gc2015.c \
              sensor_NT99250.c
              #sensor_NT99040.c \
              #sensor_pas6175.c \
              #sensor_ov2640.c \
              #sensor_ov3640.c \
              #sensor_ov9660.c \

              
# LCD DRIVER                              
SOURCES   += 	tft_s6d0139.c    \
             	tft_hx8312a.c    \
             	tft_ILI9320.c    \
             	tft_ILI9328.c    \
             	tft_ILI9325.c    \
             	tft_ILI9326.c    \
             	tft_spfd5408.c   \
             	tft_HX8346.c     \
                tft_LGDP4531.c   \
             	tft_LGDP4532.c   \
             	tft_SSD1228.c    \
             	tft_R61505U.c    \
             	tft_hx8347.c     \
             	tft_hx8347D.c    \
             	tft_SPFD54124A.c \
             	tft_SPFD5420A.c	 \
             	tft_R61509V.c    \
                tft_R61581.c     \
                tft_R61529.c  \
                tft_hx8357.c     \
                tft_ILI9335.c    \
                tft_ILI9325B8.c  \
                tft_ST7781RB8.c  \
       	        tft_S6D04H0A.c   \
       	        tft_hx8347G.c    \
       	        tft_ILI9342.c    \
       	        tft_RM68040.c    \
                tft_HX8347G_spi.c    \
                tft_ILI9163C_spi.c \
             	tft_ILI9338B.c   \
             	tft_ILI9225B.c    \
                tft_hx8352C.c  \
		        tft_ILI9481.c    \
             	tft_ILI9486.c    \
             	tft_hx8340B.c
ifeq ($(strip $(MAINLCD_SIZE)),128X64)            
			 SOURCES += fstn_nt7538.c
endif

ifeq ($(strip $(SCREEN_CAPTURE_SUPPORT)), TRUE)
SOURCES   += 	lcd_diag.c    
endif

ifeq ($(strip $(AUTOTEST_DEVICE_SUPPORT)), TRUE)
SOURCES   +=	sensor_parallel_dummy.c \
	            sensor_spi_dummy.c 
endif
else
# LCD DRIVER                           
SOURCES   += 	tft_VTM88828A_spi.c
endif

# NORFLASH+SRAM SPEC
SOURCES   +=		
								
ifeq ($(strip $(CMMB_SUPPORT)), TRUE)
SOURCES   +=    HX8347_SEINE_drv.c \
                ILI9325_SEINE_drv.c \
                SPFD5420A_SEINE_drv.c \
                R61581_SEINE_drv.c \
                R61509v_SEINE_drv.c
endif

ifeq ($(strip $(DEV_MULTIUSB_SUPPORT)), TRUE)
SOURCES	  +=	sc900770.c
endif

# PROD CFG COMMON FILE
ifneq ($(strip $(MODEM_PLATFORM)), TRUE) 
SOURCES   +=	prod_cfg.c    	\
           		freq_cfg.c  	\
           		prod_outport.c  \
           		scm_config.c 	\
           		sfs_config.c    \
           		keymap_cfg.c 	\
           		gpio_cfg.c 	    \
           		pinmap_cfg.c 	\
           		os_cfg.c		\
           		lcm_cfg_info.c	\
           		display_cfg.c 	  
ifdef LOW_MEMORY_SUPPORT
    MCFLAG_OPT       +=    -DDISPLAY_LOW_MEMORY
endif
else
SOURCES   +=	prod_cfg.c    	\
           		prod_outport.c  \
           		scm_config.c 	\
           		keymap_cfg.c 	\
           		gpio_cfg.c 		\
           		pinmap_cfg.c 	\
           		sfs_config.c    \
           		os_cfg.c	
endif

SOURCES	  +=	power_cfg.c sleep_cfg.c

ifeq ($(strip $(CMMB_SUPPORT)), TRUE)
SOURCES   +=	mtv_cfg.c
endif			

ifeq ($(strip $(MBBMS_SUPPORT)), TRUE)
SOURCES    +=	sdua_config.c demod_config.c
endif			

ifeq ($(strip $(SPLOAD_ENABLE)), TRUE)
SOURCES    += spload_api.c
endif			


# ******************** FEATURE ******************** 

ifeq ($(strip $(GPS_SUPPORT)), TRUE)
MSRCPATH	+=	$(DRIVERPATH)/gps
MSRCPATH	+=	MS_Customize/source/product/driver/gps/sirf
#MSRCPATH  +=  MS_Customize/source/product/driver/gps/sirf/prgflash/src
SOURCES		+=	gps_com.c \
                gps_cfg.c \
                gps_drv.c \
                gps_nmea.c \
                gps_diag.c \
                gps_sirf.c \
                srf_func.c

#include make/custom_drv/gps.mk
								
endif

ifneq ($(strip $(BLUETOOTH_SUPPORT)), NONE)
SOURCES		+= bt_prod.c \
		       bt_cfg.c
endif

ifeq ($(strip $(UMEM_SUPPORT)), TRUE)
MCFLAG_OPT	+= -DUMEM_SUPPORT 
SOURCES		+=umem_cfg.c
endif


ifeq ($(strip $(TOUCHPANEL_TYPE)), MULTITP)
MINCPATH	+= $(DRIVERPATH)/tp/inc \
               $(DRIVERPATH)/tp
MSRCPATH    += $(DRIVERPATH)/tp/src 
SOURCES     += tp_multi_srv.c     \
                tp_multi_drv.c   \
                tp_multi_cfg.c       
ifeq ($(strip $(CAP_TP_SUPPORT)), FT5206)
SOURCES     += tp_ft5206.c  
endif
endif



#ifeq ($(strip $(OCR_SUPPORT)), TRUE)
#MCFLAG_OPT	+=	-DPRODUCT_DRIVER_OCR_DRIVER_SUPPORT 
#MINCPATH	+=	MS_Customize/source/product/driver/ocr/inc \
#                                Third-party/ocr/include
#					
#SOURCES		+=	ocr_driver.c	\
#                thocr_lib.c
#endif

ifneq ($(strip $(FM_SUPPORT)), NONE)
MSRCPATH +=   $(DRIVERPATH)/fm \
              $(DRIVERPATH)/fm/src

SOURCES		+=	fm_cfg.c   \
              fm_drv.c   
endif
			
ifeq ($(strip $(FM_SUPPORT)), CL6012X)
SOURCES		+=	fm_cl6012x.c
endif

ifeq ($(strip $(FM_SUPPORT)), KT0812G)
SOURCES		+=	fm_kt0812g.c
endif

ifeq ($(strip $(FM_SUPPORT)), BK1080)
SOURCES		+=	fm_bk1080.c
endif

ifeq ($(strip $(FM_SUPPORT)), SPRD_V0)
SOURCES		+=	fm_sprd.c \
                  fm_dut.c  
endif

ifeq ($(strip $(NANDBOOT_SUPPORT)), TRUE)
SOURCES		+=  sdram_prod.c
else
ifeq ($(strip $(FLASH_TYPE)),   SPI)
MSRCPATH    +=  $(DRIVERPATH)/spiflash/sf_device
SOURCES     += spiflash_cfg.c \
                spiflash.c    \
                esmt.c      \
                fidelix.c       \
                giga.c         \
                macronix.c       \
                micron.c        \
                spansion.c       \
                sst.c        \
                winbond.c      \
                spi_flash_common.c
else
SOURCES		+=  flash_cfg.c	 \
                intel_drv.c	 \
                intel_m18_drv.c \
                spansion_drv.c   \
                normcp_spec.c
endif
endif
                
ifneq ($(strip $(MV_SUPPORT)), NONE)
MSRCPATH	+=	$(DRIVERPATH)/mv/src
MINCPATH	+=	MS_Customize/source/product/driver/mv/inc
SOURCES		+=	magicvoice_app.c
endif

ifeq ($(strip $(NORBOOT_NAND_UDISK_SUPPORT)), TRUE)
MCFLAG_OPT	+=-DPRODUCT_DRIVER_NAND_UDISK 
endif	
		
ifeq ($(strip $(CMMB_SUPPORT)), TRUE)
ifeq ($(strip $(MTV_MIDDLEWARE_ON_BB)), FALSE)
MCFLAG_OPT	+=-DPRODUCT_DRIVER_MTV_ENABLE
endif
endif					

ifneq ($(strip $(MOTION_SENSOR_TYPE)),)				   
ifneq ($(strip $(MOTION_SENSOR_TYPE)), NONE)
MOTION_SENSOR_DRV = TRUE
MSRCPATH +=   $(DRIVERPATH)/motionsensor
	   
MSRCPATH +=   $(DRIVERPATH)/motionsensor/accelerometer

ifeq ($(strip $(MOTION_SENSOR_TYPE)), ST_LIS35DE)
SOURCES     += msensor_lis35de.c
endif

ifeq ($(strip $(MOTION_SENSOR_TYPE)), MEMS_DTOS)
SOURCES     += msensor_dtos.c
endif

ifeq ($(strip $(MOTION_SENSOR_TYPE)), TS1003)
SOURCES     += msensor_ts1003.c
endif

ifeq ($(strip $(MOTION_SENSOR_TYPE)), ADXL346)
SOURCES     += msensor_adxl346.c
endif

endif  ##MOTION_SENSOR_TYPE != NONE
endif  ##MOTION_SENSOR_TYPE != ''

ifneq ($(strip $(MOTION_SENSOR_TYPE_OPTICAL)),)
ifneq ($(strip $(MOTION_SENSOR_TYPE_OPTICAL)), NONE)
MSRCPATH +=   $(DRIVERPATH)/motionsensor
MOTION_SENSOR_DRV = TRUE
MSRCPATH +=   $(DRIVERPATH)/motionsensor/optical
			   
ifeq ($(strip $(MOTION_SENSOR_TYPE_OPTICAL)), SCM013)
SOURCES	 +=  msensor_scm013.c
endif

ifeq ($(strip $(MOTION_SENSOR_TYPE_OPTICAL)), LTR501)
SOURCES	 +=  msensor_ltr501.c
endif
endif  ##MOTION_SENSOR_TYPE_OPTICAL != NONE
endif  ##MOTION_SENSOR_TYPE_OPTICAL != ''

ifneq ($(strip $(MOTION_SENSOR_TYPE_OFN)),)
ifneq ($(strip $(MOTION_SENSOR_TYPE_OFN)), NONE)
MSRCPATH +=   $(DRIVERPATH)/motionsensor/ofn
MSRCPATH +=   $(DRIVERPATH)/motionsensor
MOTION_SENSOR_DRV = TRUE
			   
ifeq ($(strip $(MOTION_SENSOR_TYPE_OFN)), TRULY_A320)
SOURCES	 +=  msensor_truly_a320.c
endif
endif  ##MOTION_SENSOR_TYPE_OFN != NONE
endif  ##MOTION_SENSOR_TYPE_OFN != ''

ifeq ($(strip $(MOTION_SENSOR_DRV)), TRUE)
SOURCES     += msensor_drv.c \
			   msensor_cfg.c
endif
			   
ifneq ($(strip $(WIFI_SUPPORT)),NONE) 
  MINCPATH += $(DRIVERPATH)/wifi/wifisupp
  MSRCPATH += $(DRIVERPATH)/wifi/wifisupp
  SOURCES  += wifi_drv.c wifisupp_api.c wifisupp_main.c wifisupp_eut.c
  
  ifeq ($(strip $(WIFI_SUPPORT)),AR6302)
  MSRCPATH    += $(DRIVERPATH)/wifi/ar6302
  SOURCES     += athr_wifi_adapter.c wifisupp_adap_atheros.c wifisupp_eut_ar6302.c wifi_cfg_ar6302.c 
  endif  
  
  ifeq ($(strip $(WIFI_SUPPORT)),CSR6030)
  MCFLAG_OPT += -DIMPLEMENT_ETHER_INTERFACE -DCSP_SPI_SHARE_WITH_OTHER_DEVICE -DCSR_WIFI_6030 \
                 -DCSR_WIFI_NME_ENABLE -DCSR_THROUGHPUT_TEST_TOOL -DCSR_WIFI_PTEST_ENABLE
  #CSR_LOG:=TRUE
  ifeq ($(strip $(CSR_LOG)),TRUE)
    MCFLAG_OPT += -DCSR_LOG_ENABLE -DCSR_LOG_INCLUDE_FILE_NAME_AND_LINE_NUMBER 
  endif
  MINCPATH += Third-party/wifi/CSR6030/include \
              Third-party/wifi/CSR6030/frw_csr/include \
              Third-party/wifi/CSR6030/frw_csr/include/platform
  MSRCPATH    += $(DRIVERPATH)/wifi/csr6030/frw_csr \
              += $(DRIVERPATH)/wifi/csr6030
  SOURCES     += csr_spi_master.c \
                 csr_debug_layer.c \
                 csr_ip_ether_sef.c \
                 csr_sfs_filesystem.c \
                 csr_time_utc.c \
                 csr_time.c \
                 csr_framework_ext.c \
                 bt_wifi_transport.c \
                 csr_spt_log_porting.c \
                 wifisupp_adap_csr.c \
                 wifisupp_adap_csr_sef.c \
                 wifisupp_eut_csr6030.c \
		 wifi_cfg_csr6030.c \
                 wifisupp_adap_csr_config.c
  endif
  
  ifeq ($(findstring CSR6027,$(WIFI_SUPPORT)), CSR6027)
  MINCPATH += Third-party/wifi/CSR6027/include \
              Third-party/wifi/CSR6027/frw_csr/include \
              Third-party/wifi/CSR6027/frw_csr/include/platform
  MSRCPATH += $(DRIVERPATH)/wifi/csr6027/frw_csr \
              $(DRIVERPATH)/wifi/csr6027
  SOURCES  += csr_debug_layer.c \
              csr_sfs_filesystem.c \
              csr_time_utc.c \
              csr_time.c \
              csr_framework_ext.c \
              csr_spi_master.c \
              csr_ip_ether_sef.c \
              bt_wifi_transport.c \
              wifi_cfg_csr6027.c \
              wifisupp_adap_csr.c \
              wifisupp_adap_csr_sef.c \
              wifisupp_eut_csr6027.c \
              wifisupp_adap_csr_config.c
  MCFLAG_OPT += -DCSR_WIFI_NME_ENABLE -DIMPLEMENT_ETHER_INTERFACE \
                -DCSR_THROUGHPUT_TEST_TOOL -DCSP_SPI_SHARE_WITH_OTHER_DEVICE 
  #CSR_LOG:=TRUE
  ifeq ($(strip $(CSR_LOG)),TRUE)
    MCFLAG_OPT  += -DCSR_LOG_ENABLE -DCSR_LOG_INCLUDE_FILE_NAME_AND_LINE_NUMBER
  endif
  endif  
    
  ifeq ($(strip $(WIFI_VENDOR)), REALTEK)
  MSRCPATH += $(DRIVERPATH)/wifi/rtl8723
  SOURCES	 += rtl8723_wifi_adapter.c wifisupp_adap_realtek.c wifisupp_eut_realtek.c wifi_cfg_realtek.c
  endif  
    
endif #WIFI_SUPPORT==NONE

ifneq ($(strip $(SUBLCD_DEV_SIZE)), NONE)
MCFLAG_OPT	+=-DPRODUCT_DRIVER_SUB_LCD_ENABLE
endif	

ifeq ($(strip $(MULTI_SIM_SYS)), QUAD)
SOURCES     += sim_cfg.c
endif

ifeq ($(strip $(DUAL_BATTERY_SUPPORT)), TRUE)
SOURCES +=  dualbat_prod_cfg.c\
            dualbat_prod_aw3312.c              
endif

ifeq ($(strip $(TF_LOAD_SUPPORT)), TRUE)
MINCPATH	+=	fdl_bootloader/tf_fdl/inc
MSRCPATH +=  $(DRIVERPATH)/tf_load/src
SOURCES	 +=  tf_load.c \
			 tf_cfg.c
endif

ifneq ($(strip $(MODEM_PLATFORM)), TRUE)
SOURCES     += sensor_prod.c
endif

# ******************** PLATFORM ******************** 

# ******************** PROJECT ******************** 
