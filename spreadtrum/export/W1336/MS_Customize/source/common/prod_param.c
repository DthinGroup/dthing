/******************************************************************************
 ** File Name:      prod_param.c                                               *
 ** Author:         Benjamin.Wang                                             *
 ** DATE:           31/12/2004                                                *
 ** Copyright:      2004 Spreadtrum, Incoporated. All Rights Reserved.         *
 ** Description:    This file contain some product parameter defination.                    *
 ******************************************************************************

 ******************************************************************************
 **                        Edit History                                       *
 ** ------------------------------------------------------------------------- *
 ** DATE           NAME             DESCRIPTION                               *
 ** 09/03/2003     Benjamin.Wang   Create.                                   *
 ******************************************************************************/

/**---------------------------------------------------------------------------*
 **                         Dependencies                                      *
 **---------------------------------------------------------------------------*/
#include "ms_customize_trc.h"
#include "os_api.h"
#include "threadx_os.h"
#include "nv_type.h"
#include "nvitem.h"
#include "nv_productionparam_type.h"
#include "tb_dal.h"
#include "sfs.h"
#include "ubot_api.h"
#include "UCOM_api.h"
#include "UPCC_api.h"
#include "sci_ftlmnt_Engin.h"
#include "tb_hal.h"
#include "ref_param.h"
#include "scm_api.h"
#include "production_test.h"
#include "gpio_prod_cfg.h"
//#include "gpio_ext_drv.h"
#include "gps_drv.h"
#include "dal_dcamera.h"
#include "dal_lcd.h"

#include "DoIdleOnPage.h"
#include "tp_srv.h"
#include "deep_sleep.h"
#include "chip.h"
#include "chng_freq.h"
#include "freq_cfg.h"
#include "lcd_backlight.h"
#include "prod_param.h"
#include "scm_api.h"
#include "gpio_prod_api.h"
#include "tp_api.h"
#include "sc_reg.h"
#include "power.h"
#include "dal_chr.h"
#include "chg_drv.h"
#include "usbservice_api.h"
#include "mem_prod.h"
#include "wifi_drv.h"
#include "msensor_drv.h"
#include "admm_III.h"
#include "flash.h"
#include "efs.h"
#include "BT_abs.h"
#include "resparam.h"
#include "dal_display.h"
#include "power_cfg.h"
#include "fm_drv.h"
#include "ref_import.h"
#include "chip_drvapi.h"

#ifdef TOUCHPANEL_TYPE_MULTITP
#include "tp_multi_drv.h"
#endif

#ifndef MODEM_PLATFORM
#include "layer1_engineering.h"
#endif

#ifdef NANDBOOT_SUPPORT
#include "sdram_cfg.h"
#endif

#ifdef SCREEN_CAPTURE_SUPPORT
#include "lcd_diag.h"
#endif
#include "sensor_cfg.h"

#include "flash.h"
#include "sci_ftl_api.h"

#ifdef DUAL_BATTERY_SUPPORT
#include "dualbat_drvapi.h"
#include "dualbat_prod.h"
#endif //DUAL_BATTERY_SUPPORT
#include "spiflash.h"
#include "analog_drvapi.h"
/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef __cplusplus
    extern   "C"
    {
#endif
 /**---------------------------------------------------------------------------*
 **                         Macro Definition                                   *
 **---------------------------------------------------------------------------*/
//Support  new VBATADC calibration after this calibrtion version
#define NEW_VBATADC_CAL_VER 				0xFF05   

//Default VBATADC slope
#define DEFAULT_VBATADC_SLOPE 				20

// Support the ADC calibration after this phone module version.
#define CHR_PM_VER_SUPPORT              	0x0201

// The invalid calibration value
#define CHR_INVALID_CALIBRATION             0xFFFFFFFF

#define PROD_INIT_STEP1                     0x01
/*logo res related MACRO*/

//endian related
#ifdef CHIP_ENDIAN_LITTLE
#define LOGO_FLAG                           0x6f676f6CUL  /*"ogol"*/
#define SRES_FLAG                           0x73657273UL  /*"SERS"*/
#define TRES_FLAG                           0x73657274UL  /*"SERT"*/
#else
#define LOGO_FLAG                           0x6C6f676fUL  /*"logo"*/
#define SRES_FLAG                           0x73726573UL  /*"SRES"*/
#define TRES_FLAG                           0x74726573UL  /*"TRES"*/
#endif

/* macor will and must keep no change in later version!!!!!*/
#define SRES_HEAD_RESEVED_SIZE               0x11c 
#define TRES_HEAD_RESEVED_SIZE               0x10 

/**---------------------------------------------------------------------------*
 **                         Local Variables                                   *
 **---------------------------------------------------------------------------*/
LOCAL uint16 			s_audio_device_mode_number 	= 0;
LOCAL AUDMOD_DEVMOD_T* 	s_audio_device_modes_list 	= SCI_NULL;
LOCAL PROD_PARAM_T 		t_prod_param 				= {0};      //used for product parameters
LOCAL BOOLEAN 			s_prod_nv_is_init 			= SCI_FALSE;

/**---------------------------------------------------------------------------*
 **                     Extern PUBLIC Variables Prototypes                    *
 **---------------------------------------------------------------------------*/
#ifdef DUAL_BATTERY_SUPPORT    
void REF_GetCalibrationPara_Adc(uint32  *adc_ptr,uint32 len);

uint32 REF_GetCalibrationPara_PM_Version(void);
#endif
/**---------------------------------------------------------------------------*
 **                         Date Structures                                   * 
 **---------------------------------------------------------------------------*/
#ifndef MODEM_PLATFORM 
typedef struct
{
    int16 					   	x;         		   	/*!<point coordinate x */
    int16 					   	y;                  /*!<point coordinate y */
}GUI_POINT_T;

typedef struct _logo_res_st
{
    uint32   magic;   /*flag*/
    uint32   offset;  /*offset*/
    uint32   size;    /*res size*/
}GUI_LOGO_RES_T;

typedef struct _logo_res_head_info
{
    struct _logo_res_st main_lcd_logo_t;
    struct _logo_res_st sub_lcd_logo_t;
}RES_Logo_Info_T;

typedef struct _res_head_info
{
    uint32 flag;  
    uint8  reserved[12];
    uint32 offset;     
    RES_Logo_Info_T * data;  //data is not in resouce ,only used for soft purpose
}RES_Head_Info_T;

/**---------------------------------------------------------------------------*
 **                         Local Function declare                            *
 **---------------------------------------------------------------------------*/
 /*****************************************************************************/
// 	Description : find the logo from resource.
//	Global resource dependence : none
//  Author:       nandflash or norflash support
//	Note: 
/*****************************************************************************/
LOCAL uint8 * _PROD_CalcLogoResAddr(LCD_ID_E id,IMG_RES_SRC_T *img_src)
{
	uint8 * logo_addr = PNULL;
    RES_Head_Info_T * res_info;
    RES_Logo_Info_T* logo_res_ptr;

    res_info = (RES_Head_Info_T *)SCI_ALLOCA(sizeof(RES_Head_Info_T));
    if(NULL == res_info)
    {
        return NULL;
    }

    SCI_MEMSET((uint8*)res_info, 0 , sizeof(RES_Head_Info_T));
    

    /*read the head info from reset file excepte data ptr*/
#ifdef NANDBOOT_SUPPORT 
    if(!ResParam_Read(res_info, 0, sizeof(RES_Head_Info_T)- 4))
        goto error;
#else
    SCI_MEMCPY((uint8*)res_info, PROD_GetResMemAdd(), sizeof(RES_Head_Info_T)- 4);
#endif

    /*according with the first 4 byte in logo to judge the "TRES or SRES"*/
    res_info->data = (RES_Logo_Info_T*)SCI_ALLOCA(sizeof(RES_Logo_Info_T));
    
    if(NULL == res_info->data )
    {
        goto error;
    }

    if(SRES_FLAG == res_info->flag)
    {
        /*for SRES
         the logo offset is located at address 0x11c from start
         */
        res_info->offset = SRES_HEAD_RESEVED_SIZE;
    }
    else if(TRES_FLAG == res_info->flag)
    {
        /*for TRES
         the logo offset is located at address 0x10
         */
         /*nothing need be done*/
    }
    else
    {
        /*resource error*/
        goto error1;
    }

    logo_res_ptr = res_info->data;

    /*read the logo res head info from res file*/
#ifdef NANDBOOT_SUPPORT 
    if(!ResParam_Read(logo_res_ptr, res_info->offset, sizeof(RES_Logo_Info_T)))
        goto error;
#else
    SCI_MEMCPY((uint8*)logo_res_ptr, PROD_GetResMemAdd()+ res_info->offset, sizeof(RES_Logo_Info_T));
#endif

	if(MAIN_LCD_ID == id)
	{
        if(logo_res_ptr->main_lcd_logo_t.magic == LOGO_FLAG)
        {
            /*main logo is valid*/
#ifdef NANDBOOT_SUPPORT 
            logo_addr = SCI_ALLOCA(logo_res_ptr->main_lcd_logo_t.size);
            if(NULL != logo_addr)
            {
                if(!ResParam_Read(logo_addr, logo_res_ptr->main_lcd_logo_t.offset, logo_res_ptr->main_lcd_logo_t.size))
                {
                    SCI_FREE(logo_addr);
                    logo_addr = NULL;
                }
            }
#else
            logo_addr = PROD_GetResMemAdd() + logo_res_ptr->main_lcd_logo_t.offset;
#endif
           img_src->imgstreamsize=logo_res_ptr->main_lcd_logo_t.size;
        }
	}
	else if(SUB_LCD_ID == id )
	{
        if(logo_res_ptr->sub_lcd_logo_t.magic == LOGO_FLAG)
        {
            /*sub logo is valid*/
#ifdef NANDBOOT_SUPPORT 
            logo_addr = SCI_ALLOCA(logo_res_ptr->sub_lcd_logo_t.size);
            if(NULL != logo_addr)
            {
                if(!ResParam_Read(logo_addr, logo_res_ptr->sub_lcd_logo_t.offset, logo_res_ptr->sub_lcd_logo_t.size))
                {
                    SCI_FREE(logo_addr);
                    logo_addr = NULL;
                }
            }
#else
            logo_addr = PROD_GetResMemAdd() + logo_res_ptr->sub_lcd_logo_t.offset;
#endif
            img_src->imgstreamsize=logo_res_ptr->sub_lcd_logo_t.size;
        }
	}
	else
	{
		logo_addr = (uint8*)NULL;
	}
    
error1:
    SCI_FREE(res_info->data);
error:
    SCI_FREE(res_info);

    img_src->imgstream_ptr=(uint8 *)logo_addr;
    return (uint8 *)logo_addr;

}

/*****************************************************************************/
//  Description:    This function free logo temp variable, maily for nand platform
//  param:          id: main lcd /sub lcd
//  return          the address of adress
//	Note:           
/*****************************************************************************/
LOCAL BOOLEAN _PROD_FreeLogoAddr(uint8* addr)
{
    /*
    for norflash, res can be read directly
    in nandflash, the res should be read to mem first, so
      1.first alloc memory
      2. copy res to memory
      3. last free memory
    */

#ifdef NANDBOOT_SUPPORT 
    BOOLEAN ret = SCI_FALSE;
    if(NULL != addr)
    {
        SCI_FREE(addr);

        ret = SCI_TRUE;
    }
    
    return ret;
#else
    return SCI_TRUE;
#endif
}

 /*****************************************************************************/
// 	Description : draw logo.      bmp,sjpg,gif are supportted.
//	Global resource dependence : 
//    Author: Eddy.Wei
//	Note: 
 /*****************************************************************************/
LOCAL void _PROD_DisPlayLogo(LCD_ID_E lcd_id,GUI_POINT_T startpoint,IMG_RES_SRC_T  *img_src_ptr,IMG_RES_INFO_T *img_info_ptr)
{

	IMG_RES_DST_T img_dst={0};

	IMG_RES_DST_CTL_T img_distctl={0};
	
	LCD_INFO_T 	 lcd_info={0};

	LCD_GetInfo(lcd_id, &lcd_info);

	img_src_ptr->imgrect.top	= 0;
	img_src_ptr->imgrect.left	= 0;
	img_src_ptr->imgrect.bottom	= (img_info_ptr->height-1) > lcd_info.lcd_height ? (lcd_info.lcd_height-1) : (img_info_ptr->height-1);
	img_src_ptr->imgrect.right	= (img_info_ptr->width-1) > lcd_info.lcd_width ? (lcd_info.lcd_width-1) : (img_info_ptr->width-1);
	

	
	
	LCD_GetInfo(lcd_id, &lcd_info);
	img_dst.dstrect.top			= startpoint.y;
	img_dst.dstrect.left			= startpoint.x;
	img_dst.dstrect.bottom		= startpoint.y + img_info_ptr->height-1;
	img_dst.dstrect.right			= startpoint.x+ img_info_ptr->width-1;

	img_dst.dstrect.top 		= (img_dst.dstrect.top	< lcd_info.lcd_height)? (img_dst.dstrect.top)	  : (lcd_info.lcd_height-1);
	img_dst.dstrect.left 		= (img_dst.dstrect.left	< lcd_info.lcd_width) ? (img_dst.dstrect.left )	  : (lcd_info.lcd_width -1);
	img_dst.dstrect.bottom 		= (img_dst.dstrect.bottom	< lcd_info.lcd_height)? (img_dst.dstrect.bottom): (lcd_info.lcd_height-1);
	img_dst.dstrect.right 		= (img_dst.dstrect.right	< lcd_info.lcd_width) ? (img_dst.dstrect.right )   : (lcd_info.lcd_width -1);
	

	img_dst.dstctl=&img_distctl;
	img_dst.dstctl->height		= lcd_info.lcd_height;
	img_dst.dstctl->width		= lcd_info.lcd_width;
	img_dst.dstctl->format		=IMGREF_FORMAT_RGB565;

	img_dst.dstctl->dstmem 	= lcd_info.lcdbuff_ptr;
	
	IMG_RES_Display_Img(img_src_ptr, &img_dst);
}

/*****************************************************************************/
//  Description:    The function update the logo picture on the specified LCD
//  Author:         Eddy.Wei
//	Note:           
/*****************************************************************************/
LOCAL void _PROD_UpdateLCDPicture(LCD_ID_E lcd_id)
{
        LCD_INFO_T          lcd_info ;
        GUI_POINT_T 		start_point = {0};
        DISPLAY_LAYER_INFO_T *layer_ptr[1];
        DISPLAY_LAYER_INFO_T layer = {0};
        DISPLAY_INVALIDATE_POS_T display_rect = {0};
        IMG_RES_SRC_T	img_src={0};
        IMG_RES_INFO_T	img_info={0};
        const uint8* icon_ptr = PNULL;

        icon_ptr = PROD_GetLogoAddrFromRes(lcd_id,&img_src);

        if(NULL != icon_ptr )
        {
            if(IMG_RES_SUCCESS==IMG_RES_GetImageInfo(&img_src, &img_info))
            {
            _PROD_DisPlayLogo(lcd_id,start_point,&img_src,&img_info);
            }
            _PROD_FreeLogoAddr((uint8 *)icon_ptr);        
        }
#ifndef RELEASE_INFO
        else
        {
        SCI_PASSERT(FALSE, ("logo resource is error! lcd id: %d",lcd_id));/*assert verified*/
        }
#endif

        // Invalidate the welcome logo on Main LCD
        LCD_GetInfo(lcd_id,&lcd_info);
        display_rect.rect.left = 0;
        display_rect.rect.top = 0;
        display_rect.rect.right = lcd_info.lcd_width-1;
        display_rect.rect.bottom = lcd_info.lcd_height-1;
        display_rect.rot_angle = 0;
        layer_ptr[0] = &layer;
        layer.buf_addr = (uint32)lcd_info.lcdbuff_ptr;
        layer.width = lcd_info.lcd_width;
        layer.height = lcd_info.lcd_height;
        layer.is_enable = 1;
        layer.layer_id = 0;
        layer.pos.x = 0;
        layer.pos.y = 0;
        layer.layer_data.alpha = 255;
        layer.layer_data.alpha_sel = 1;
        layer.layer_data.colorkey_en = 0;
        layer.layer_data.type = 3;
        DISPLAY_InvalidateRect(lcd_id,&layer_ptr[0],1,&display_rect);
	
}

/*****************************************************************************/
//  Description:    The function display init logo picture before mmi get scheduled
//	Global resource dependence: 
//					According to hardware pcb layout, sp7320 has two lcd
//  Author:         Daniel.Ding
//	Note:           
/*****************************************************************************/
LOCAL void _PROD_InitLCDPictures (void)
{
    POWER_RESTART_CONDITION_E restart_condition;

    restart_condition = POWER_GetRestartCondition();

    //request remodified, alarm open, don't display logo, 3.27, 134121, !! //@cr124123  alarm open, should display logo
    if ((RESTART_BY_ASSERT != restart_condition)&&(RESTART_BY_ALARM != restart_condition))
    {

        _PROD_UpdateLCDPicture(MAIN_LCD_ID);

#ifdef PRODUCT_DRIVER_SUB_LCD_ENABLE
        _PROD_UpdateLCDPicture(SUB_LCD_ID);
#endif

        // open back light.
        GPIO_SetLcdBackLight(SCI_TRUE);
        LCD_SetBackLightBrightness(REFPARAM_GetLCDBrightless());

    }

}



#ifdef DUAL_BATTERY_SUPPORT
/*******************************************************************/
//  Function name:   REF_GetAdcCalibrationParaEx
//  Discription: Get the dual battery's adc calibration from the nvitem
//  Author:  jackey.ling
//  Note:
/*******************************************************************/
LOCAL void REF_GetAdcCalibrationParaEx(
    uint32 battery_main[2],
    uint32 battery_aux[2],
    uint32 invalid_calibration,
    uint32 chr_pm_ver_support
)

{
    ADC_T *adc_ptr = PNULL;  
    
    // Set default value      
    battery_main[0] = invalid_calibration;
    battery_main[1] = invalid_calibration;

    battery_aux[0] = invalid_calibration;
    battery_aux[1] = invalid_calibration;
    
    adc_ptr = SCI_ALLOC_APP(sizeof(ADC_T));

    if(!adc_ptr)
    {
      //SCI_TRACE_LOW:"[chr_drv] REF_GetAdcCalibrationParaEx adc_ptr alloc fail!!"
      SCI_TRACE_ID(TRACE_TOOL_CONVERT,PROD_PARAM_483_112_2_18_0_26_17_9,(uint8*)"");
      return;
    }
    
    SCI_MEMSET(adc_ptr,0,sizeof(ADC_T));
    
    REF_GetCalibrationPara_Adc((uint32*)adc_ptr,sizeof(ADC_T)); 

    if(REF_GetCalibrationPara_PM_Version() >= chr_pm_ver_support)
    {
        if((0x7FFFFFFF > adc_ptr->reserved[0]) ||
                (0x7FFFFFFF > adc_ptr->reserved[1]))
        {
            battery_main[0] = adc_ptr->reserved[0];
            battery_main[1] = adc_ptr->reserved[1];
        }

        if((0x7FFFFFFF > adc_ptr->reserved[2]) ||
                (0x7FFFFFFF > adc_ptr->reserved[3]))
        {
            battery_aux[0] = adc_ptr->reserved[2];
            battery_aux[1] = adc_ptr->reserved[3];
        }      
    }

    SCI_Free(adc_ptr);
}

#endif

#endif /* MODEM_PLATFROM */
/*****************************************************************************/
//  Description:    The function set charge parameters from nvitem.
//	Global resource dependence: 
//  Author:         Benjamin.Wang
//	Note:           
/*****************************************************************************/
LOCAL void _PROD_SetChargeParam(CHGMNG_PARAM_PROD_T* chr_param)
{
    CHGMNG_ADC_CAL_T adc_cal_param;
    uint32  battery[2] = {0};
    uint32  general_adc[2] = {0};
    int32   x0 = 0, y0 = 0, x1 = 0, y1 = 0;
    int32   ADC0_slope = 0, ADC0_p = 0;
    const  int32 Zoom_in = 1000; // used to enlarge slope value to store in int32
    
    if(REF_GetCalibrationParamVersion() >= NEW_VBATADC_CAL_VER)
    {
        //Get battery cal infor from calibaration parameters. general_adc is no use.
        REF_GetAdcCalibrationPara(battery, general_adc, CHR_INVALID_CALIBRATION, CHR_PM_VER_SUPPORT);

        //SCI_TRACE_LOW:"[chr_drv] PRODINIT: battery0 = 0x%X, battery1 = 0x%X, "
        SCI_TRACE_ID(TRACE_TOOL_CONVERT,PROD_PARAM_534_112_2_18_0_26_17_10,(uint8*)"dddd", battery[0], battery[1], general_adc[0], general_adc[1]);
                      
        x0 = (int32)((battery[0] & 0xFFFF0000) >> 16);
        y0 = (int32)( battery[0] & 0xFFFF);   
        x1 = (int32)((battery[1] & 0xFFFF0000) >> 16);
        y1 = (int32)( battery[1] & 0xFFFF); 
        
        ADC0_slope = (int32)((general_adc[0] & 0xFFFF0000) >> 16);
        ADC0_p     = (int32)( general_adc[0] & 0xFFFF); 
        
        if( (ADC0_slope!=0) && (ADC0_p!=0) && (battery[0]!=0))
        {
                    adc_cal_param.slope = Zoom_in*ADC0_p/ADC0_slope; //calculate the adc slope
                    adc_cal_param.zoom_in = Zoom_in;
                    adc_cal_param.x0_adc = x0;
                    adc_cal_param.y0_vol = y0;          
        }
        else
        {      
            if(battery[0] != 0 && battery[1] != 0)          //two points calibration
            {
                if( (battery[0] != battery[1]) && ((x0 -x1) != 0))
                {                
                    adc_cal_param.slope = Zoom_in * (y0 - y1) / (x0 - x1); //calculate the adc slope
                    adc_cal_param.zoom_in = Zoom_in;
                    adc_cal_param.x0_adc = x0;
                    adc_cal_param.y0_vol = y0;
                
                }
                else
                {                
                    adc_cal_param.slope = 0; 
                    adc_cal_param.zoom_in = 0;
                    adc_cal_param.x0_adc = 0;
                    adc_cal_param.y0_vol = 0;
                }			
            }              
        }

      
#ifdef DUAL_BATTERY_SUPPORT
        {
            CHGMNG_ADC_CAL_T Dbat_adc_ref_main;
            CHGMNG_ADC_CAL_T Dbat_adc_ref_aux;
     
            //Get dual battery cal infor from calibaration parameters. 
            REF_GetAdcCalibrationParaEx(battery, general_adc, CHR_INVALID_CALIBRATION, CHR_PM_VER_SUPPORT);   
            
            //SCI_TRACE_LOW:"[chr_drv] PRODINIT2: battery0 = 0x%X, battery1 = 0x%X, "
            SCI_TRACE_ID(TRACE_TOOL_CONVERT,PROD_PARAM_584_112_2_18_0_26_17_11,(uint8*)"dddd", battery[0], battery[1], general_adc[0], general_adc[1]);
                      
            x0 = (int32)((battery[0] & 0xFFFF0000) >> 16);
            y0 = (int32)( battery[0] & 0xFFFF);   
            x1 = (int32)((battery[1] & 0xFFFF0000) >> 16);
            y1 = (int32)( battery[1] & 0xFFFF); 
            

            if(battery[0] != 0 && battery[1] != 0)          //two points calibration
            {
                if((battery[0] != battery[1]) && 
                    ((x0 -x1) != 0) && (x0 >= 200))
                {
                    Dbat_adc_ref_main.slope = Zoom_in * (y0 - y1) / (x0 - x1); //calculate the adc slope
                    Dbat_adc_ref_main.zoom_in = Zoom_in;
                    Dbat_adc_ref_main.x0_adc = x0;
                    Dbat_adc_ref_main.y0_vol = y0;
                    
                }
                else
                {
                    Dbat_adc_ref_main.slope = 0;      // the slope is default   
                    Dbat_adc_ref_main.zoom_in = 0;
                    Dbat_adc_ref_main.x0_adc = 0;
                    Dbat_adc_ref_main.y0_vol = 0;                   
                }    			
    	    }
    	    else
    	    {
                Dbat_adc_ref_main.slope = 0;      // the slope is default   
                Dbat_adc_ref_main.zoom_in = 0;
                Dbat_adc_ref_main.x0_adc = 0;
                Dbat_adc_ref_main.y0_vol = 0;   
            }

            x0 = (int32)((general_adc[0] & 0xFFFF0000) >> 16);
            y0 = (int32)( general_adc[0] & 0xFFFF);   
            x1 = (int32)((general_adc[1] & 0xFFFF0000) >> 16);
            y1 = (int32)( general_adc[1] & 0xFFFF); 
            
            if(general_adc[0] != 0 && general_adc[1] != 0)          //two points calibration
            {
                if( (general_adc[0] != general_adc[1]) && 
                    ((x0 -x1) != 0) && (x0 >= 200))
                {
                    Dbat_adc_ref_aux.slope = Zoom_in * (y0 - y1) / (x0 - x1); //calculate the adc slope
                    Dbat_adc_ref_aux.zoom_in = Zoom_in;
                    Dbat_adc_ref_aux.x0_adc = x0;
                    Dbat_adc_ref_aux.y0_vol = y0;                
                }
                else
                {
                    Dbat_adc_ref_aux.slope = 0;      
                    Dbat_adc_ref_aux.zoom_in = 0;
                    Dbat_adc_ref_aux.x0_adc = 0;
                    Dbat_adc_ref_aux.y0_vol = 0;                
                }    			
    	    }
    	    else
    	    {
                Dbat_adc_ref_aux.slope = 0;      
                Dbat_adc_ref_aux.zoom_in = 0;
                Dbat_adc_ref_aux.x0_adc = 0;
                Dbat_adc_ref_aux.y0_vol = 0;  
            }


            if((Dbat_adc_ref_main.slope != 0)&&
                    (Dbat_adc_ref_aux.slope == 0))
            {
                Dbat_adc_ref_aux = Dbat_adc_ref_main; //copy main bat cali to aux bat
            }
            else if((Dbat_adc_ref_main.slope == 0)&&
                    (Dbat_adc_ref_aux.slope != 0))
            {
                Dbat_adc_ref_main = Dbat_adc_ref_aux; //copy aux bat cali to main bat
            }

            //SCI_TRACE_LOW:"[chr_drv] slope_m =%d,slope_aux =%d"
            SCI_TRACE_ID(TRACE_TOOL_CONVERT,PROD_PARAM_663_112_2_18_0_26_17_12,(uint8*)"dd",Dbat_adc_ref_main.slope, Dbat_adc_ref_aux.slope);
            
            DBAT_SetBatAdcCal(DBAT_MAIN_BAT,&Dbat_adc_ref_main);            
            DBAT_SetBatAdcCal(DBAT_AUX_BAT,&Dbat_adc_ref_aux);        
        }
#endif /*#ifdef DUAL_BATTERY_SUPPORT*/   
    }
    else
    {
        SCI_ASSERT(0);  /*assert verified*/
    }
/*lint -esym(718, REF_GetMarkerValue) */
/*lint -esym(746, REF_GetMarkerValue) */
    if(REF_GetMarkerValue() & 0x200)    //check adc calibration
    {
        adc_cal_param.cal_flag = CAL_TYPE_NV;
    }
    else
    {
        adc_cal_param.cal_flag = CAL_TYPR_NO;
    }
   
    CHGMNG_SetVBatAdcCal((CHGMNG_ADC_CAL_T *)&adc_cal_param);     //must be first!
    //End CR22713 
    CHGMNG_SetOtpParam((CHGMNG_OTP_PARAM_T*) &chr_param->otp_param );
    CHGMNG_SetOvpParam ((CHGMNG_OVP_PARAM_T*) &chr_param->ovp_param);
    
    CHGMNG_SetChgParam ((CHGMNG_CHARGE_PARAM_T*) &chr_param->chg_param);

    CHGMNG_SetDischgParam ((CHGMNG_DISCHARGE_PARAM_T*) &chr_param->dischg_param);
    
    /* Switch point is set to 0x19 if chiptype is SC8800H */
    if(CHIP_GetChipType() == SC8800H)
    {
        CHGMNG_SetHWSwitchPoint(CHG_SWITPOINT_26);
    }
    else
    {
        CHGMNG_SetHWSwitchPoint((CHG_SWITPOINT_E)chr_param->hw_switch_point);
    }	
    
    CHGMNG_SetVBatCapTable(chr_param->dischg_bat_capacity_table,chr_param->chg_bat_capacity_table);
    //charge param init end
}  




#ifndef STACK_CHECK_SUPPORT
/*
thread stack check , valid when MACOR" STACK_CHECK_SUPPORT = FALSE
*/
LOCAL void _prod_thread_stack_overflow_handle(void* ptr)
{
    SCI_THREAD_T *thread_ptr;

    thread_ptr = (SCI_THREAD_T *)ptr;

    SCI_PASSERT (0, ("Stack Overflow,thread:0x%x,sp overflow addr:0x%x,thread ID:0x%x,Tx Name:%s", /*assert verified*/
                     thread_ptr, thread_ptr->thread.tx_thread_stack_highest_ptr, thread_ptr->thread_id, thread_ptr->thread.tx_thread_name));
}
#endif

/*****************************************************************************/
//  Description:    This function set peripheral nv parameter.
//  Author:           Yi.Qiu
//	Note:           
/*****************************************************************************/
PUBLIC BOOLEAN PROD_SetChargeNVParam(CHG_SWITPOINT_E 	point)
{
    if(t_prod_param.chr_param.hw_switch_point == point) 
    {
        //SCI_TRACE_LOW:"PROD_SetChargeNVParam need't update !"
        SCI_TRACE_ID(TRACE_TOOL_CONVERT,PROD_PARAM_722_112_2_18_0_26_17_13,(uint8*)"");
        return SCI_TRUE;
    }
    
    //SCI_TRACE_LOW:"PROD_SetChargeNVParam update point = 0x%x"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,PROD_PARAM_726_112_2_18_0_26_17_14,(uint8*)"d", point);
    t_prod_param.chr_param.hw_switch_point = point;
    
    EFS_NvitemWrite(NV_PROD_PARAM, sizeof(t_prod_param), (uint8 *)(&t_prod_param), 0);
    
    return SCI_TRUE;
}


/**---------------------------------------------------------------------------*
 **                         Function Definitions                              *
 **---------------------------------------------------------------------------*/

#ifndef MODEM_PLATFORM 
/*****************************************************************************/
//  Description:    This function return logo address from resource
//  param:          id: main lcd /sub lcd
//  return          the address of logo res
//	Note:           
/*****************************************************************************/
PUBLIC uint8 * PROD_GetLogoAddrFromRes(LCD_ID_E id,IMG_RES_SRC_T* img_src)
{
	return _PROD_CalcLogoResAddr(id,img_src);
}
#endif

/*****************************************************************************/
//  Description:    This function get peripheral nv parameter.
//  Author:         Liangwen.Zhen
//	Note:           
/*****************************************************************************/
PUBLIC uint16 PROD_GetPeripheralNVParam(PROD_NV_PERIPHERAL_ID_E id)
{
	uint16* param_ptr = (uint16*)&t_prod_param.peripheral_param;
	
	SCI_ASSERT(id < PROD_NV_ID_MAX);/*assert verified*/
	
	PROD_InitProdParam();

	return param_ptr[id]; /*lint !e661*/
}

/*****************************************************************************/
//  Description:    This function set peripheral nv parameter.
//  Author:         Liangwen.Zhen
//	Note:           
/*****************************************************************************/
PUBLIC BOOLEAN PROD_SetPeripheralNVParam(
					PROD_NV_PERIPHERAL_ID_E id,
					uint16 					param
				)
{
	uint16* param_ptr = (uint16*)&t_prod_param.peripheral_param;
	uint32 status = 0;

	SCI_ASSERT(id < PROD_NV_ID_MAX);/*assert verified*/

	param_ptr[id] = param; /*lint !e661*/

	status = EFS_NvitemWrite(
        NV_PROD_PARAM, sizeof(t_prod_param), (uint8 *)(&t_prod_param), 0);
    
    if(NVERR_NONE != status)    
    {
        SCI_PASSERT(0, ("PROD_SetPeripheralNVParam: error !"));/*assert verified*/
        return SCI_FALSE;   /*lint !e527 */
    }
	else
	{
		return SCI_TRUE;
	}
}

/*****************************************************************************/
//  Description:    This function do some initialization work needed by products.
//  Author:         Benjamin.Wang
//	Note:           
/*****************************************************************************/
PUBLIC void PROD_Init(void)
{
    uint16	reset_mode = POWER_GetResetMode();
    

    //SCI_TRACE_LOW:"PROD_Init() Enter !"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,PROD_PARAM_800_112_2_18_0_26_17_15,(uint8*)"");

    LDO_Prod_Init();

#ifndef MODEM_PLATFORM 	
    //set the customized freq config	
    CHNG_FREQ_SetCustomFreqCfg(PROD_GetCustomFreqCfg());
#endif

#ifdef TOUCHPANEL_TYPE_SW
	{
		void TP_InitProd(void);
    	TP_InitProd();	
	}
#endif

#ifndef MODEM_PLATFORM     
#ifdef DUAL_BATTERY_SUPPORT
    DBAT_Init(DBAT_Prod_GetFun(), DBAT_Prod_GetCfg());
#endif
    // display lcd init pictures 
    if( (reset_mode != CALIBRATION_MODE) && (reset_mode != CALIBRATION_POST_NO_LCM_MODE) )
    {
        // display lcd init pictures 
        _PROD_InitLCDPictures ();
    }

    USB_RegService (USB_SERVICE_UDISK,	(USB_GADGET_DRIVER_T *)UBOT_GetHandler( )); 
#endif /* MODEM_PLATFORM */

#ifdef USB_SUPPORT	

#ifdef USB_VCOM_SUPPORT	
    USB_RegService (USB_SERVICE_UCOM,	(USB_GADGET_DRIVER_T *)UCOM_GetHandler( )); 
#endif

#ifdef USB_WEBCAMERA_SUPPORT       
    USB_RegService (USB_SERVICE_UPCC,	(USB_GADGET_DRIVER_T *)UPCC_GetHandler( )); 
#endif   

    USB_RegService (USB_SERVICE_LOG,	(USB_GADGET_DRIVER_T *)UCOM_GetHandler( )); 
    
#ifdef USB_MTP_SUPPORT       
    USB_RegService (USB_SERVICE_MTP,	(USB_GADGET_DRIVER_T *)MTP_GetHandler( ));  
#endif 
#ifdef UCOM_UMS_SUPPORT       
    USB_RegService (USB_SERVICE_UCOM_UMS,	(USB_GADGET_DRIVER_T *)UCOM_UMS_GetHandler( )); 
#endif 
#endif

	if(CHIP_DetectMemType()) // NAND + SDRAM
    {
#ifdef NANDBOOT_SUPPORT
    
	    //Init SDRAM Parameters
	    HAL_SDRAM_InitPara(SDRAM_GetTimingPara());

	    if ((CALIBRATION_MODE != reset_mode)&&(DSP_CODE_DOWNLOAD_BACK_CALIBRATION != reset_mode))
	    {
	        /*second param is not used anymore in 10A latest verison
	          the exit condition is info->name string is null
            */
    	}
#endif		
	}

#ifndef MODEM_PLATFORM
	SCM_Init();
#endif	

#ifndef POWERON_DEBUG	
	GPIO_ConfigRfReset();

	if((CALIBRATION_MODE != reset_mode) && (CALIBRATION_POST_MODE != reset_mode))
	{
		typedef void (* L1_RFRESET_REG_CALLBACK) (void);
		PUBLIC void L1API_RegisterRFResetFunc(L1_RFRESET_REG_CALLBACK func);
	
		L1API_RegisterRFResetFunc(GPIO_ConfigRfReset);		/*lint !e718*/

	}
#endif	

#ifdef GPS_SUPPORT
    //GPS_Init();
#endif

#if defined (FM_SUPPORT_KT0812G) || defined(FM_SUPPORT_BK1080)
	FM_Entersleep(); 
#endif

#if defined(MOTION_SENSOR_TYPE)||defined(MOTION_SENSOR_TYPE_OPTICAL)
	MSensor_InitModule();	//Init gsensor module
#endif

#ifdef SCREEN_CAPTURE_SUPPORT
	LCD_DIAG_RegDiagCmdRoutine();
#endif
#ifndef MODEM_PLATFORM
    #ifdef	CAMERA_SUPPORT    
	    Sensor_RegisterCmdRoutine();
    #endif 
#endif

#if defined(CHIP_VER_6531) && (!defined(BLUETOOTH_SUPPORT_SPRD_BT) || defined(MODEM_PLATFORM))
    SLEEP_BT_PowerDomain_Set(FALSE);
#endif
	PROD_CusCfgInit();	

#ifndef STACK_CHECK_SUPPORT
    SCI_SetThreadStackErrorNotify(_prod_thread_stack_overflow_handle);
#endif    

    if(MEM_IsRWRegionOverflow())
    {
        SCI_PASSERT(SCI_FALSE, ("sactter region is more than max ram size"));/*assert verified*/
    }
	//SCI_TRACE_LOW:"PROD_Init() Exit !"
	SCI_TRACE_ID(TRACE_TOOL_CONVERT,PROD_PARAM_911_112_2_18_0_26_18_16,(uint8*)"");
	
}

#ifdef CHIP_VER_6531
/*****************************************************************************/
//  Description:    The function used to config TP XL/XR/YU/YD PIN default down.
//	Global resource dependence: 
//  Author:         
//	Note:           Config XL/XR/YU/YD into PIN_O/PIN_SPD/FUNC_DEF/PIN_FPD/DS_1
//	It is used to config TP pins into power down during calibration mode.
/*****************************************************************************/
PUBLIC void PROD_CaliTPPinCfg(void)
{
    CHIP_Init_Calibration();
}
#endif

PUBLIC  uint32 PROD_InitByStep(uint32  step)
{
    uint16	reset_mode = POWER_GetResetMode();
    
    if(PROD_INIT_STEP1 == step)
    {
       if (CALIBRATION_MODE == reset_mode || CALIBRATION_POST_NO_LCM_MODE == reset_mode)
       {
#ifdef CHIP_VER_6531
            PROD_CaliTPPinCfg();
#endif
    
#ifndef BLUETOOTH_SUPPORT_NONE
        	BT_TestInit();
#endif 
#ifdef TOUCHPANEL_TYPE_MULTITP
            TPC_Init();
            GPIO_TPIRQCtrl(SCI_FALSE);
            TPC_Suspend();
#endif
        }
#ifdef WIFI_SUPPORT
        {
            extern BOOLEAN WIFISUPP_EUTInit(void); //declare
        	(void)WIFISUPP_EUTInit();
        }        	
#endif 

//add by luther.li 2012-12-28 

#if defined (FM_SUPPORT_SPRD_V0)
    {
          extern void FM_DUT_Init(void);
		  FM_DUT_Init();
    }
#endif
        return 0;
    }
    else
    {
        return 1;
    }
}

LOCAL ANA_DEV_PARAM_PROD_T* PROD_GetLCMParam()
{
    return &(t_prod_param.lcm_param);
}

LOCAL ANA_DEV_PARAM_PROD_T* PROD_GetKPDParam()
{
    return &(t_prod_param.kpd_param);
}

LOCAL void _PROD_SetKPDParam(ANA_DEV_PARAM_PROD_T* param)
{
    ANA_DEV_PARAM_T ana_param = {0};
    ana_param.tuning_step = param->tuning_step;
    ana_param.tuning_interval = param->tuning_interval;
    ANA_SetDevParams(ANA_DEV_ID_KPD_BL, &ana_param);
}

LOCAL void _PROD_SetLCMParam(ANA_DEV_PARAM_PROD_T* param)
{
    ANA_DEV_PARAM_T ana_param = {0};
    ana_param.tuning_step = param->tuning_step;
    ana_param.tuning_interval = param->tuning_interval;
    ANA_SetDevParams(ANA_DEV_ID_LCM_BL, &ana_param);
}
/*****************************************************************************/
//  Description:    The function initialize PROD parameters which on NVItem.
//	Global resource dependence: 
//  Author:         Benjamin.Wang
//	Note:           
/*****************************************************************************/
PUBLIC void PROD_InitProdParam(void)
{
    uint32  status;
    int32 i;
    
    if(s_prod_nv_is_init)
   	{
   		//SCI_TRACE_LOW:"PROD_InitProdParam has been called !"
   		SCI_TRACE_ID(TRACE_TOOL_CONVERT,PROD_PARAM_963_112_2_18_0_26_18_17,(uint8*)"");
		return;
   	}
	
    // Read REF parameter initial values from NVItem.
    status = EFS_NvitemRead(NV_PROD_PARAM, sizeof(t_prod_param), (uint8 *)(&t_prod_param));    
    // If the initial values don't exist.
    if (NVERR_NOT_EXIST == status)
    {
#ifdef WIN32
        EFS_NvitemWrite(NV_PROD_PARAM, sizeof(t_prod_param), (uint8 *)(&t_prod_param), 1);
#else
        if (!NVITEM_IsSimulate())
        {
    	    SCI_PASSERT(0, ("Please download new NVItem !"));/*assert verified*/
        }
#endif  // WIN32        
    }
    /* set lcm and kpd params */
    _PROD_SetKPDParam(PROD_GetKPDParam());
    _PROD_SetLCMParam(PROD_GetLCMParam());

    _PROD_SetChargeParam(PROD_GetChargeParam());
    // Read audio device mode number
    status = EFS_NvitemRead(NV_AUDIO_DEVMOD_NUM, sizeof(uint16), (uint8*)(&s_audio_device_mode_number));
    if(NVERR_NOT_EXIST==status)
    {
#ifdef WIN32
        EFS_NvitemWrite(NV_AUDIO_DEVMOD_NUM, sizeof(uint16), (uint8 *)(&s_audio_device_mode_number), 1);
#else
        if (!NVITEM_IsSimulate())
        {
    	    SCI_PASSERT(0, ("Please download new NVItem !"));/*assert verified*/
        }
#endif  // WIN32           
    }

    s_audio_device_modes_list = (AUDMOD_DEVMOD_T*)SCI_ALLOC_APP(sizeof(AUDMOD_DEVMOD_T) * s_audio_device_mode_number);
    if(s_audio_device_modes_list == SCI_NULL)
    {
        SCI_PASSERT(SCI_FALSE, ("PROD: Can't assign enough memory for device mode reading!"));/*assert verified*/
    }
    // Read and add audio device modes
    status = EFS_NvitemRead(
                    NV_AUDIO, 
                    sizeof(AUDMOD_DEVMOD_T) * s_audio_device_mode_number, 
                    (uint8*)s_audio_device_modes_list);
    if(NVERR_NOT_EXIST==status)
    {
#ifdef WIN32
        EFS_NvitemWrite(
            NV_AUDIO, 
            sizeof(AUDMOD_DEVMOD_T * s_audio_device_mode_number), 
            (uint8 *)(s_audio_device_modes_list), 
            1);
#else
        if (!NVITEM_IsSimulate())
        {
    	    SCI_PASSERT(0, ("Please download new NVItem !"));/*assert verified*/
        }
#endif  // WIN32           
    }
    
    for(i = 0; i < s_audio_device_mode_number; i++)
    {
        AUDMOD_AddDevMode(s_audio_device_modes_list + i);
    }

    SCI_FREE(s_audio_device_modes_list);

	s_prod_nv_is_init = SCI_TRUE;

	//SCI_TRACE_LOW:"PROD_InitProdParam Exit !"
	SCI_TRACE_ID(TRACE_TOOL_CONVERT,PROD_PARAM_1032_112_2_18_0_26_18_18,(uint8*)"");
	
}

/*****************************************************************************/
//  Description:    The function read PROD parameters about audio from NVItem .
//	Global resource dependence: 
//  Author:         Benjamin.Wang
//	Note:           
/*****************************************************************************/
PUBLIC uint32 PROD_ReadAudioParam(  
             NV_AUDIO_E   audio_mode,
             void*        mode_param_ptr
             )
{
    uint32  status = 0;
	int32   i = 0;
	AUDMOD_DEVMOD_T* audio_device_modes_list = PNULL;
	uint8*  mode_name_ptr = PNULL;
	
	
	// to get the audio device mode number
	if( NULL == s_audio_device_mode_number )
    {
    	 // Read audio device mode number
        status = EFS_NvitemRead(NV_AUDIO_DEVMOD_NUM, sizeof(uint16), (uint8*)(&s_audio_device_mode_number));
        if(NVERR_NOT_EXIST==status)
        {
             return SCI_ERROR;          
        }
    }

	mode_name_ptr = (uint8*)AUDMOD_GetAudioDeviceModeNameById(audio_mode);

	// Malloc buffer to store audio device mode parameter   
    audio_device_modes_list = (AUDMOD_DEVMOD_T*)SCI_ALLOC_APP(sizeof(AUDMOD_DEVMOD_T) * s_audio_device_mode_number);
    if(audio_device_modes_list == SCI_NULL)
    {
        SCI_PASSERT(SCI_FALSE, ("PROD: Can't assign enough memory for device mode reading!"));/*assert verified*/
    }
    
	
    // Read and add audio device modes
    status = EFS_NvitemRead(
                    NV_AUDIO, 
                    sizeof(AUDMOD_DEVMOD_T) * s_audio_device_mode_number, 
                    (uint8*)audio_device_modes_list);
    if(NVERR_NOT_EXIST==status)
    {
		return SCI_ERROR;        
    }

	//compare the mode name.
    for(i=s_audio_device_mode_number-1; i>=0; i--)
    {
        if(0 == strcmp((char *)(mode_name_ptr), (char *)((audio_device_modes_list+i)->ucModeName)))/*lint !e613*/
        {
            SCI_MEMCPY( mode_param_ptr, (audio_device_modes_list+i), sizeof(AUDMOD_DEVMOD_T));/*lint !e613*/
            break;
        }
    }

	SCI_FREE(audio_device_modes_list);

	return SCI_SUCCESS;
	
}
/*****************************************************************************/
//  Description:    The function write PROD parameters about audio to NVItem .
//	Global resource dependence: 
//  Author:         Benjamin.Wang
//	Note:           is_flush：SCI_FALSE-〉表示数据只写入CACHE，SCI_TRUE->表示数据写入NV，请flush数据Cache
/*****************************************************************************/
PUBLIC uint32 PROD_WriteAudioParam(
    //const uint8* pucModeName, 
    NV_AUDIO_E   audio_mode, 
    void*        mode_param_ptr,
    BOOLEAN      is_flush
    )
{
    uint32  status = 0;
    int32   i;
	uint8*  mode_name_ptr = PNULL;
    AUDMOD_DEVMOD_T* ptDeviceMode = PNULL;
    
    ptDeviceMode = (AUDMOD_DEVMOD_T*)mode_param_ptr;
    
    if(ptDeviceMode == SCI_NULL)
    {
        return AUDMOD_INPUT_PARA_ERROR;
    }

	  mode_name_ptr = (uint8*)AUDMOD_GetAudioDeviceModeNameById(audio_mode);

	  //SCI_TRACE_LOW:"PROD_WriteAudioParam: mode_name = %s, is_flush = %d, start_time = %d"
	  SCI_TRACE_ID(TRACE_TOOL_CONVERT,PROD_PARAM_1122_112_2_18_0_26_18_19,(uint8*)"sdd", mode_name_ptr, is_flush, SCI_GetTickCount());
	
    // If the setting mode name doesn't match with input device mode, we will do nothing.
    if(0 != strcmp((char*)ptDeviceMode->ucModeName, (char*)mode_name_ptr))
    {
        return AUDMOD_INPUT_PARA_ERROR;
    }    

    if( PNULL == s_audio_device_modes_list)
    {
        s_audio_device_modes_list = (AUDMOD_DEVMOD_T*)SCI_ALLOC_APP(sizeof(AUDMOD_DEVMOD_T) * s_audio_device_mode_number);
        if(s_audio_device_modes_list == SCI_NULL)
        {
            SCI_PASSERT(SCI_FALSE, ("PROD_WriteProdParam: Can't assign enough memory for device mode reading!"));/*assert verified*/
        }
   	
        // Read and add audio device modes
        status = EFS_NvitemRead(
                        NV_AUDIO, 
                        sizeof(AUDMOD_DEVMOD_T) * s_audio_device_mode_number, 
                        (uint8*)s_audio_device_modes_list);
        
        if(NVERR_NONE != status)
        {
        	SCI_FREE(s_audio_device_modes_list);
            return SCI_ERROR; 
        }    
	}	
  
    //compare the mode name.
    for(i=s_audio_device_mode_number-1; i>=0; i--)
    {
        if(strcmp((char *)(mode_name_ptr), (char *)((s_audio_device_modes_list+i)->ucModeName))==0)
        {
            SCI_MEMCPY((s_audio_device_modes_list+i), ptDeviceMode, sizeof(AUDMOD_DEVMOD_T));

			      //SCI_TRACE_LOW:"PROD_WriteAudioParam: Write to Cache, end time = %d"
			      SCI_TRACE_ID(TRACE_TOOL_CONVERT,PROD_PARAM_1158_112_2_18_0_26_18_20,(uint8*)"d", SCI_GetTickCount());
            break;
        }        
    }
    
    if(i<0)//(pucModeName is not exist in s_audio_device_modes_list
    {
        return SCI_ERROR; 
    }

	  if(is_flush)
    {
    		BOOLEAN ret = SCI_FALSE;
    		
        ret = NVITEM_UpdateNvParam_Protect(
                 NV_AUDIO, 
                 (sizeof(AUDMOD_DEVMOD_T) * s_audio_device_mode_number), 
                 (uint8 *)(s_audio_device_modes_list)
                 );
    
        if(ret)
        {
            for(i=s_audio_device_mode_number-1; i>=0; i--)
            {
                AUDMOD_RemoveDevMode((s_audio_device_modes_list + i)->ucModeName);
            }
            for(i=s_audio_device_mode_number-1; i>=0 ; i--)
            {
                AUDMOD_AddDevMode(s_audio_device_modes_list + i);
            }                   
        } 
        else
        {
            SCI_PASSERT(0, ("PROD_WriteProdParam: EFS_NvitemWrite error!"));  /*assert verified*/
        }         

        SCI_FREE(s_audio_device_modes_list);

		    //SCI_TRACE_LOW:"PROD_WriteAudioParam: Write to Nv,end time = %d"
		    SCI_TRACE_ID(TRACE_TOOL_CONVERT,PROD_PARAM_1196_112_2_18_0_26_18_21,(uint8*)"d", SCI_GetTickCount());
    }

	

    return SCI_SUCCESS;    
}

/*****************************************************************************/
//  Description:    The function gets charge nvitem parameters.
//	Global resource dependence: 
//  Author:         Benjamin.Wang
//	Note:           
/*****************************************************************************/
PUBLIC CHGMNG_PARAM_PROD_T* PROD_GetChargeParam(void)
{
    return &(t_prod_param.chr_param);
}
   
//
// Get the Mobile serial number. if the bufferLen is less than the serial number len,
// only the byte 
//
PUBLIC void PROD_GetMobileSerialNumber( uint8 *serialNumberBuffer, uint32  bufferLen )
{
    uint8   * ptr = NULL;
    uint32  address;

    TEST_TRACK_HEADER_T *header_ptr = NULL;
    uint32  len;
    uint8   val;
    
    SCI_ASSERT(serialNumberBuffer != NULL);/*assert verified*/
    
    len     = sizeof (TEST_DATA_INFO_T);

    address = FLASH_GetProductInfoAddr() ; /*useless now */

	ptr = (uint8 *)SCI_ALLOC_APP(len);
	SCI_ASSERT(SCI_NULL != ptr);/*assert verified*/
    
	/* Read product control information */
	if (EFS_Direct( SCI_TRUE, ptr,address, len)) {	      
        SCI_FREE(ptr);
        return ;
	}
   
    header_ptr = (TEST_TRACK_HEADER_T *)(ptr);
 
    /* Get SerialNum */
    val = header_ptr->SN[0]; /*lint !e613*/
    if((val >= 0x30)&&(val <= 0x39)) /*valid char*/
    {
    	SCI_MEMCPY(serialNumberBuffer,&header_ptr->SN[6],bufferLen); /*lint !e613*/
    }else
    {
	    SCI_MEMSET(serialNumberBuffer,0,bufferLen);
    }
    
    SCI_FREE(ptr);
    
}
/*****************************************************************************/
//  Description:    The function reads one mode of audio parameters from flash .
//	Global resource dependence: 
//  Author:         Cherry.Liu
//	Note:           
/*****************************************************************************/
PUBLIC AUDMOD_RESULT_E PROD_ReadAudioParamFromFlash(
    const uint8* pucModeName, 
    AUDMOD_DEVMOD_T* pModeParam //out ptr
    )
{
    uint32  status;
    int32 i;
    AUDMOD_DEVMOD_T* ptDeviceMode;
    
    ptDeviceMode = pModeParam; 
    if(ptDeviceMode == SCI_NULL)
    {
        return AUDMOD_INPUT_PARA_ERROR;
    }

    s_audio_device_modes_list = (AUDMOD_DEVMOD_T*)SCI_ALLOC_APP(sizeof(AUDMOD_DEVMOD_T) * s_audio_device_mode_number);
    if(s_audio_device_modes_list == SCI_NULL)
    {
        return AUDMOD_ALLOC_ERROR;
    }
    
    // Read and add audio device modes
    status = EFS_NvitemRead(
                    NV_AUDIO, 
                    sizeof(AUDMOD_DEVMOD_T) * s_audio_device_mode_number, 
                    (uint8*)s_audio_device_modes_list);
    if(NVERR_NONE != status)
    {
        return AUDMOD_ERROR;
    }    
   
    //compare the mode name.
    for(i=s_audio_device_mode_number-1; i>=0; i--)
    {
        if(strcmp((char *)(pucModeName), (char *)((s_audio_device_modes_list+i)->ucModeName))==0)
        {
            SCI_MEMCPY( ptDeviceMode,
                (s_audio_device_modes_list+i),
                sizeof(AUDMOD_DEVMOD_T));
            break;
        }        
    }
    
    if(i<0)//(pucModeName is not exist in s_audio_device_modes_list
    {
        return AUDMOD_MODE_NO_EXIST;            
    }
    
    SCI_Free(s_audio_device_modes_list);

    return AUDMOD_NO_ERROR;    
}


/*****************************************************************************/
//  Description:    The function writes one mode of audio parameters to flash .
//	Global resource dependence: 
//  Author:         Cherry.Liu
//	Note:           
/*****************************************************************************/
PUBLIC AUDMOD_RESULT_E PROD_WriteAudioParamToFlash(
    const uint8* pucModeName, 
    AUDMOD_DEVMOD_T* pModeParam
    )
{
    uint32  status;
    BOOLEAN  ret = SCI_FALSE;
    int32 i;
    AUDMOD_DEVMOD_T* ptDeviceMode;

    //check input error
    ptDeviceMode = pModeParam;
    if(ptDeviceMode == SCI_NULL)
    {
        return AUDMOD_INPUT_PARA_ERROR;
    }

    // If the setting mode name doesn't match with input device mode, we will do nothing.
    if(0 != strcmp((char*)ptDeviceMode->ucModeName, (char*)pucModeName))
    {
        return AUDMOD_INPUT_PARA_ERROR;
    }    

   
    s_audio_device_modes_list = (AUDMOD_DEVMOD_T*)SCI_ALLOC_APP(sizeof(AUDMOD_DEVMOD_T) * s_audio_device_mode_number);
    if(s_audio_device_modes_list == SCI_NULL)
    {
        return AUDMOD_ALLOC_ERROR;
    }
    
    // Read and add audio device modes
    status = EFS_NvitemRead(
                    NV_AUDIO, 
                    sizeof(AUDMOD_DEVMOD_T) * s_audio_device_mode_number, 
                    (uint8*)s_audio_device_modes_list);
    
    if(NVERR_NONE != status)
    {
        return AUDMOD_ERROR;
    }    
   
   
    //compare the mode name.
    for(i=s_audio_device_mode_number-1; i>=0; i--)
    {
        if(strcmp((char *)(pucModeName), (char *)((s_audio_device_modes_list+i)->ucModeName))==0)
        {
            SCI_MEMCPY((s_audio_device_modes_list+i), 
                ptDeviceMode, 
                sizeof(AUDMOD_DEVMOD_T));
            
            break;
        }        
    }
    
    if(i<0)//(pucModeName is not exist in s_audio_device_modes_list
    {
        return AUDMOD_MODE_NO_EXIST;                  
    }

    //write to flash
    ret = NVITEM_UpdateNvParam_Protect(
        NV_AUDIO, 
        (sizeof(AUDMOD_DEVMOD_T) * s_audio_device_mode_number), 
        (uint8 *)(s_audio_device_modes_list));

    if(!ret)
    {
        return AUDMOD_ERROR;
    }


    SCI_Free(s_audio_device_modes_list);
    
    return AUDMOD_NO_ERROR;    
}

/*****************************************************************************/
//  Description:    The function read PROD parameters about testing result of the
//                  engine mode.
//	Global resource dependence: 
//  Author:         Liangwen.zhen
//	Note:           
/*****************************************************************************/
PUBLIC uint32 PROD_ReadEngModeTestResult(  void* result_buf_ptr, uint32 buf_len)
{
	uint32 status = 0;
	uint32 nv_len = NVITEM_GetLength(NV_ENG_TEST_RESULT_ID);

	if((0 != nv_len) && (0 != buf_len) && (buf_len >= nv_len)&& (PNULL != result_buf_ptr))
	{
		status = EFS_NvitemRead(NV_ENG_TEST_RESULT_ID, nv_len, (uint8*)result_buf_ptr);
	    if(NVERR_NOT_EXIST == status)
	    {
	        return SCI_ERROR;          
	    }
	}
	else
	{
		//SCI_TRACE_LOW:"PROD_ReadEngModeTestResult: nv_len %d, buf_len %d"
		SCI_TRACE_ID(TRACE_TOOL_CONVERT,PROD_PARAM_1415_112_2_18_0_26_19_22,(uint8*)"dd", nv_len, buf_len);
		return SCI_ERROR;
	}

	return SCI_SUCCESS;
}

/*****************************************************************************/
//  Description:    The function write PROD parameters about testing result of the
//                  engine mode.
//	Global resource dependence: 
//  Author:         Liangwen.zhen
//	Note:           
/*****************************************************************************/
PUBLIC uint32 PROD_WriteEngModeTestResult(  void* result_buf_ptr, uint32 buf_len)
{
	BOOLEAN status = SCI_FALSE;
	uint32 nv_len = NVITEM_GetLength(NV_ENG_TEST_RESULT_ID);

	if((0 != nv_len) && (0 != buf_len) && (buf_len <= nv_len) && (PNULL != result_buf_ptr))
	{	
		status = NVITEM_UpdateNvParam_Protect(
	                 NV_ENG_TEST_RESULT_ID, 
	                 nv_len, 
	                 (uint8 *)result_buf_ptr
	                 );
	}
	else
	{
		//SCI_TRACE_LOW:"PROD_WriteEngModeTestResult: nv_len %d, buf_len %d"
		SCI_TRACE_ID(TRACE_TOOL_CONVERT,PROD_PARAM_1442_112_2_18_0_26_19_23,(uint8*)"dd", nv_len, buf_len);
	}

	if(status)
	{
		return SCI_SUCCESS;
	}
	else
	{
		return SCI_ERROR;
	}
}


/*****************************************************************************/
//  Description:    The function read Flash ID .
//	Global resource dependence: 
//  Author:         fei.zhang
//	Note:           
/*****************************************************************************/
PUBLIC void PROD_GetFlashID(uint32 *pManu_id, uint32 *pDev_id, uint32 *pExtend_id)
{
#ifdef NANDBOOT_SUPPORT
	uint32 nandId = 0x0;
	SCI_FTL_NAND_HW_INFO ftlHwInfo;

	if(SCI_FTL_ERROR_NONE != SCI_FTL_IOCtl((void*)nandId, SCI_FTL_Get_NandPhysicalInfo, 0, 0, &ftlHwInfo, sizeof(SCI_FTL_NAND_HW_INFO), 0))
	{
		*pManu_id = 0;
		*pDev_id = 0;
		*pExtend_id = 0;
		return;
	}

	*pManu_id = (uint32)(ftlHwInfo.ID&0xFF00)>>8;
	*pDev_id = (uint32)(ftlHwInfo.ID&0xFF);
	*pExtend_id = 0;
#else
#ifdef FLASH_TYPE_SPI
        SPIFLASH_GetID(0, (uint8 *)pManu_id, (uint8 *)pDev_id, (uint8 *)pExtend_id);
#else
	FLASH_GetID (pManu_id, pDev_id, pExtend_id);
#endif
#endif
}

/*****************************************************************************/
//  Description:   update NV, then update necessary module
//	Note:      20110808: Will.Jiang_NEWMS00111228 : added
/*****************************************************************************/
PUBLIC void PROD_UpdateWithNewNV(void)
{

    // if not init, init it first
    PROD_InitProdParam();
    // set charge parameters from nvitem
    _PROD_SetChargeParam(PROD_GetChargeParam());

}

/**---------------------------------------------------------------------------*
 **                         Compiler Flag                                     *
 **---------------------------------------------------------------------------*/
#ifdef __cplusplus
    }
#endif

