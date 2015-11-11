/******************************************************************************
 ** File Name:    Srf_func.h                                                     *
 ** Author:       David.Jia                                                 *
 ** DATE:         7/31/2007                                                   *
 ** Copyright:    2005 Spreatrum, Incoporated. All Rights Reserved.           *
 ** Description:                                                              *
 *****************************************************************************/
/******************************************************************************
 **                   Edit    History                                         *
 **---------------------------------------------------------------------------* 
 ** DATE            NAME            DESCRIPTION                                 * 
 ** 07/31/2007      David.Jia       GpsGetTTFFSrf, GpsDownloadSrf.
 *****************************************************************************/
 
 #ifndef _SRF_FUNC_H
 #define _SRF_FUNC_H
 
/*****************************************************************************/
//  FUNCTION:     GpsGetTTFFSrf
//  Description:    get the Time To Frist Fix
//  return:         >0--SiRF's TTFF, <0--timeout error
//  INPUT:          uint32 timeout(ms)--operation timeout.
//  Author:         David.Jia
//  date:           2007.7.17
//	Note:           use g_Command_Buf to call GpsTTFFSrf.
/*****************************************************************************/
int GpsGetTTFFSrf(uint32 TimeOut);

/*****************************************************************************/
//  FUNCTION:     GpsDownloadSrf
//  Description:    download firmware
//  return:         0--good, otherwise will be SCI_ASSERT
//  INPUT:          uint32 baudrate on download firmware
//  Author:         David.Jia
//  date:           2007.7.27
//	Note:           prgflash_ss3img.a
/*****************************************************************************/
int GpsDownloadSrf (uint32 download_baudrate);
 
 #endif