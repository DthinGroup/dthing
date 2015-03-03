ifeq ($(strip $(CUSTOMER_NAME)), SS)
    MCFLAG_OPT	=	-D_PS_BL7

    ifeq ($(strip $(PRODUCT_CONFIG)),	sc6530_samsung_Kiwi)
        MCFLAG_OPT	+=	-D_ATC_KIWI
    endif

    ifeq ($(strip $(GPRSMPDP_SUPPORT)), TRUE)
        MCFLAG_OPT += -DGPRSMPDP_ENABLE	
    endif
    
    ifeq ($(strip $(_LESSEN_TRACE_SUPPORT)), TRUE)
        MCFLAG_OPT   	+= -DATC_LESSEN_TRACE
    endif   
else
    MCFLAG_OPT	= -D_AUDIO_ENHANCE_

    ifeq ($(strip $(DUALSIM_SUPPORT)),	TRUE)
        MCFLAG_OPT	+=	-D_DUAL_SIM
    endif
    
    ifeq ($(strip $(MODEM_PLATFORM)), TRUE)
        MCFLAG_OPT += -D_ATC_ONLY	
    endif
    
    ifeq ($(strip $(TRACE_INFO_SUPPORT)),	TRUE)
        MCFLAG_OPT	+=	-D_ATC_BEDUG_VERSION_
    endif

    ifeq ($(strip $(MODEM_TYPE)), ZBX)
        MCFLAG_OPT		+= -DATC_DISABLE_SPEQPARA
        MCFLAG_OPT		+= -DATC_SAT_ENABLE
    endif

    ifeq ($(strip $(ORANGE_SUPPORT)), TRUE) 
        MCFLAG_OPT		+= -DATC_ORANGE_ENABLE
    endif 
endif


ifeq ($(strip $(GPRS_SUPPORT)), TRUE)
    MCFLAG_OPT   	+= -D_SUPPORT_GPRS_
else
    MCFLAG_OPT   	+= -D_GSM_ONLY_ -D_BASE_COMPACT_CODE_
endif

ifeq ($(strip $(ATC_SUPPORT)),	COMPACT)
    MCFLAG_OPT	+=	-D_ULTRA_LOW_CODE_ -D_GSM_ONLY_
endif

ifeq ($(strip $(SYNCTOOL_SUPPORT)),	TRUE)
    MCFLAG_OPT	+=	-D_ATC_SYNC_TOOL_
endif

ifeq ($(strip $(PCLINK_SUPPORT)), TRUE)
    MCFLAG_OPT += -DPCLINK_ENABLE	
endif

ifeq ($(strip $(MUX_SUPPORT)), TRUE)
    MCFLAG_OPT   	+= -D_MUX_ENABLE_ -D_MUX_REDUCED_MODE_
endif

ifeq ($(strip $(MODEM_MODE)), AP_PLUS_BP)
    MCFLAG_OPT   	+= -D_ATC_ONLY
endif

ifeq ($(strip $(CUSTOMER_NAME)), A)
MCFLAG_OPT += -DGSM_CUSTOMER_AFP_SUPPORT
endif

ifeq ($(strip $(AUTO_TEST)), TRUE)
MCFLAG_OPT += -AUTO_TEST_SUPPORT
endif

MINCPATH  = Base/PS/export/inc Base/atc/export/inc Base/Layer1/export/inc 
MINCPATH += char_lib/export/inc Base/l4/export/inc Base/sim/export/inc
MINCPATH += chip_drv/export/inc/outdated ms_ref/export/inc
ifeq ($(strip $(CUSTOMER_NAME)), SS)
    MINCPATH += Base/atc/samsung_phone/source/c/aud_file base/l4/source/arm/datamag/h base/l4/source/arm \
                ARMRomCode/seine/arm/drv_sc6600v/inc
endif

ifeq ($(strip $(DUALMIC)), TRUE)
    MINCPATH += MS_Ref/export/inc/dualmic
else
    MINCPATH += MS_Ref/export/inc/singlemic
endif

ifeq ($(strip $(MODEM_PLATFORM)),TRUE)
    ATC_SRC_PATH = Base/atc/modem/
else
    ifeq ($(strip $(CUSTOMER_NAME)), SS)
        ATC_SRC_PATH = Base/atc/samsung_phone/
    else
        ATC_SRC_PATH = Base/atc/feature_phone/
    endif
endif

MINCPATH += $(strip $(ATC_SRC_PATH))source/h
MSRCPATH =  $(strip $(ATC_SRC_PATH))source/c

# modem : direct satellite & common modem
ifeq ($(strip $(ATC_SRC_PATH)), Base/atc/modem/)
    # direct satellite
    ifeq ($(strip $(MODEM_TYPE)), ZBX)
        SOURCES    =    at_common.c atc_gsm.c atc_main.c atc.c atc_gsm_sms.c atc_malloc.c atc_version.c atc_gsm_ss.c \
                        atc_phonebook.c atc_yacc.c atc_basic_cmd.c atc_plus_gprs.c atc_common.c atc_sat_info.c atc_gprs_modem.c \
                        atc_sat_lex.c atc_sleep.c atc_plmntable.c
    # common modem
    else
        SOURCES    =    at_common.c atc_gsm.c atc_main.c atc.c atc_gsm_sms.c atc_malloc.c atc_version.c atc_gsm_ss.c \
	                atc_phonebook.c atc_yacc.c atc_basic_cmd.c atc_plus_gprs.c atc_common.c atc_info.c atc_gprs_modem.c \
	                atc_lex.c atc_sleep.c atc_eng.c atc_stk.c atc_stk_adapt.c atc_stk_func.c atc_stk_decode.c \
	                atc_stk_encode.c atc_mbbms.c atc_plmntable.c modem_fs.c modem_at.c modem_parser.c modem_composer.c modem_control.c
    endif
# feature phone & orange	& ss
else
    ifeq ($(strip $(ATC_SRC_PATH)), Base/atc/samsung_phone/)
        #SS
        ifeq ($(strip $(GPRS_SUPPORT)), TRUE)
        SOURCES			= 	at_common.c atc_gsm.c atc_main.c atc.c atc_gsm_sms.c atc_malloc.c atc_version.c atc_gsm_ss.c \
                  	 		atc_phonebook.c atc_yacc.c atc_basic_cmd.c atc_gsm_stk.c atc_plus_gprs.c atc_common.c atc_info.c atc_gprs_modem.c \
	        							atc_lex.c atc_sleep.c atc_samsung_prod.c atc_sha1.c
        else
        SOURCES			= 	at_common.c atc_gsm.c atc_main.c atc.c atc_gsm_sms.c atc_malloc.c atc_version.c atc_gsm_ss.c \
                  	 		atc_phonebook.c atc_yacc.c atc_basic_cmd.c atc_gsm_stk.c atc_common.c atc_gsmonly_info.c \
	        							atc_gsmonly_lex.c atc_sleep.c atc_samsung_prod.c atc_sha1.c
        endif
    else
        # orange
        ifeq ($(strip $(ORANGE_SUPPORT)), TRUE)
            ifeq ($(strip $(GPRS_SUPPORT)), TRUE)	
                SOURCES    =    atc_orange_prod.c at_common.c atc.c atc_basic_cmd.c atc_common.c atc_gprs_modem.c atc_gsm.c atc_gsm_sms.c atc_gsm_ss.c \
	                        atc_gsm_stk.c atc_org_info.c atc_org_lex.c atc_main.c atc_malloc.c atc_phonebook.c \
	                        atc_plus_gprs.c atc_sleep.c atc_version.c atc_yacc.c			               
            else
                SOURCES    =    atc_orange_prod.c at_common.c atc_gsm.c atc_main.c atc.c atc_gsm_sms.c atc_malloc.c atc_version.c atc_gsm_ss.c \
                                atc_phonebook.c atc_yacc.c atc_basic_cmd.c atc_gsm_stk.c atc_common.c atc_gsmonly_info.c \
                                atc_gsmonly_lex.c atc_sleep.c
            endif
        # feature phone
        else
            ifeq ($(strip $(GPRS_SUPPORT)), TRUE)	
                SOURCES    =    at_common.c atc.c atc_basic_cmd.c atc_common.c atc_gprs_modem.c atc_gsm.c atc_gsm_sms.c atc_gsm_ss.c \
                                atc_gsm_stk.c atc_info.c atc_lex.c atc_main.c atc_malloc.c atc_phonebook.c \
                                atc_plus_gprs.c atc_sleep.c atc_version.c atc_yacc.c			               
            else
                SOURCES    =    at_common.c atc_gsm.c atc_main.c atc.c atc_gsm_sms.c atc_malloc.c atc_version.c atc_gsm_ss.c \
                                atc_phonebook.c atc_yacc.c atc_basic_cmd.c atc_gsm_stk.c atc_common.c atc_gsmonly_info.c \
                                atc_gsmonly_lex.c atc_sleep.c
            endif
        endif
    endif
endif

