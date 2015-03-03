/******************************************************************************
 ** File Name:    Srf_func.c                                                     *
 ** Author:       David.Jia                                                 *
 ** DATE:         7/11/2007                                                   *
 ** Copyright:    2005 Spreatrum, Incoporated. All Rights Reserved.           *
 ** Description:                                                              *
 *****************************************************************************/
/******************************************************************************
 **                   Edit    History                                         *
 **---------------------------------------------------------------------------* 
 ** DATE          NAME            DESCRIPTION                                 * 
 ** 07/12/2007      David.Jia       add GpsGetString/GpsCommandSrf.
 ** 07/13/2007      David.Jia       GpsTestSrf function.
 ** 07/17/2007      David.Jia       Add GpsTTFFSrf.
 ** 07/18/2007      David.Jia       Add GpsTest4Srf.
 ** 07/20/2007      David.Jia       Add GpsGetTTFFSrf, encapsulated GpsTTFFSrf.
 ** 07/24/2007      David.Jia       GpsGetString return 0 should be overtime,
 **     correct in GetCommandSrf.
 ** 08/02/2007      David.Jia       Change names of COM_Init/COM_Close/Map_Read/Map_Write 
 **     to GPS_ComInit/GPS_ComClose/GPS_ComRead/GPS_ComWrite.
 ** 08/03/2007      David.Jia       modify GpsGetTTFFSrf, switch from NMEA to SiRF binary.
 *****************************************************************************/
/*----------------------------------------------------------------------------*
**                        Dependencies                                        *
**---------------------------------------------------------------------------*/
#include "ms_customize_trc.h"
#include "Gps_SiRF.h"       //@David.Jia 2007.7.11

#include "cmddef.h"
#include "os_api.h"
#include "sci_api.h"

/**---------------------------------------------------------------------------*
**                        Compiler Flag                                       *
**---------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*
**                            Mcaro Definitions                               *
**---------------------------------------------------------------------------*/
#define GPS_GET_DELAY       50      //@David.Jia 2007.7.12 get command fequency 20Hz
#define GPS_ACK_TIMEOUT     1000    //command ack time < 1000 ms 
#define GPS_SIRF_A0         0xA0    //SiRF packet head 1
#define GPS_SIRF_A2         0xA2    //SiRF packet head 2
#define GPS_SIRF_B0B3       0xB0B3  //SiRF packet tail
#define GPS_SIRF_COMMAND_ACK    0x0B    //ack command
#define GPS_SIRF_COMMAND_SIZE   1023    //maximum SiRF command size
#define GPS_SIRF_VISIBLE_LIST   0x0D    //visible list command id
#define GPS_SIRF_MEASURED_TRACKING_DATA 0x04    //measured tracking data command id
#define GPS_SIRF_CNO_MAX    10      //every channel 10 CNo value
#define GPS_SIRF_CH_MAX     12      //SiRF star 3 have 12 channels 

//copy uint16 to a uint8 pointer
#define GPS_INT2PTR_B(ptr, u16)    (*(ptr)++ = ((u16)>>8) & 0xFF); \
                                    (*(ptr)++ = (u16) & 0xFF);    
//copy a uint8 pointer to uint16
#define GPS_PTR2INT_B(u16, ptr)     ((u16) = (uint16)*(ptr)++ << 8); \
                                    ((u16) += *(ptr)++);    

//wether now is timeout
#define GPS_TIMEOUT(begin, timeout)     ((SCI_GetTickCount()-(begin))>(timeout)? 1 : 0)                                    

#define GPS_DEBUG
#ifdef  GPS_DEBUG
#define GPS_LOG     SCI_TraceLow 
#endif
/*----------------------------------------------------------------------------*
**                             Data Structures                                *
**---------------------------------------------------------------------------*/
//Switch Operating Mode
/*
typedef struct {
    uint16 head;    //0xA0A2
    uint16 size;
    uint8   id;
    uint16  mode;   //0x1E54, test mode 4
    uint16  svid;   //0x10
    uint16  period; //0x0A
    uint16  check_sum;
    uint16  tail;   //0xB0B3     
} GPS_SIRF_CMD150, *GPS_SIRF_CMD150_PTR;
*/
#define GPS_SIRF_SVID_OFFSET   7   //svid start from [7]
#define GPS_SIRF_CMD150_CHECKSUM_OFFSET 11  //check sum [11]
#define GPS_SIRF_ID_OFFSET  4       //command id offset [4]

//channel tracking data, used in COM4
/*
typedef struct {
    uint8   svid;
    uint8   azimuth;
    uint8   elev;
    uint16  state;
    uint8   CNo[GPS_SIRF_CNO_MAX];
} SIRF_CH_DATA;
*/
#define GPS_SIRF_CH_DATA_SIZE   15  //every channel data size 15 bytes
#define GPS_SIRF_CH_CNO_OFFSET         5   //CNo value [5]

//Measured Tracking Data
/*
typedef struct {
    uint16 head;    //0xA0A2
    uint16 size;
    uint8   id;
    uint16  week;
    uint32  tow;
    uint8   chans;
    SIRF_CH_DATA    ch[GPS_SIRF_CH_MAX];
    uint16  check_sum;
    uint16  tail;   //0xB0B3 
} GPS_SIRF_CMD4, *GPS_SIRF_CMD4_PTR;
*/
#define GPS_SIRF_CH_OFFSET  12      //channel data offset 12

/*----------------------------------------------------------------------------*
**                             Local Vars                                     *
**---------------------------------------------------------------------------*/
static uint8 s_Command_Buf[GPS_SIRF_COMMAND_SIZE];     //command buffer 

//SiRF binary command, set output NMEA 9600bps
uint8 g_Srf_Bin2NMEA[32] = {
   0xA0, 0xA2, 0x00, 0x18, 0x81, 0x02, 0x01, 0x01, 
   0x00, 0x01, 0x01, 0x01, 0x05, 0x01, 0x01, 0x01, 
   0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 
   0x00, 0x01, 0x25, 0x80, 0x01, 0x3A, 0xB0, 0xB3
}; 

//factory reset and clear memory
uint8 g_Srf_ColdReset[33] = {
   0xA0, 0xA2, 0x00, 0x19, 0x80, 0x00, 0x00, 0x00, 
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
   0x00, 0x00, 0x00, 0x0C, 0x84, 0x01, 0x10, 0xB0, 0xB3
};

//switch to SiRF binary mode, NMEA command
uint8 g_Srf_NMEA2Bin[] = "$PSRF100,0,57600,8,1,0*37\r\n";

//enter test mode 4, track SV 0x10 
uint8 g_Srf_Test4[15] = {
   0xA0, 0xA2, 0x00, 0x07, 0x96, 0x1E, 0x54, 0x00, 
   0x10, 0x00, 0x0A, 0x01, 0x1C, 0xB0, 0xB3
};

//for GpsGetTTFFSrf
static uint8 s_gps_command[GPS_SIRF_COMMAND_SIZE + 8];

/*----------------------------------------------------------------------------*
**                         Local Function Prototype                           *
**---------------------------------------------------------------------------*/
static int GpsGetString(uint8 *pStr, uint32 length, uint32 timeout);
static int GpsCommandSrf(uint8 *pCommand, uint32 timeout);

/*----------------------------------------------------------------------------*
**                         Function Definitions                               *
**---------------------------------------------------------------------------*/

//@David.Jia 2007.7.12  begin
/*****************************************************************************/
//  FUNCTION:     GpsGetString
//  Description:    get the length string from COM.
//  return:         >0--ok, bytes number read, <0--timeout error, -(bytes number read).
//  INPUT:          uint8 *pStr--string buffer; uint32 length--want to read string length;
//                  uint32 timeout(ms)--operation timeout.
//  Author:         David.Jia
//  date:           2007.7.12
//	Note:           internal function
/*****************************************************************************/
static int GpsGetString(uint8 *pStr, uint32 length, uint32 timeout)
{
    uint32 time_start, time_current, got_len = 0;
    
    time_start = SCI_GetTickCount();
    
    do
    {
        got_len += GPS_ComRead(pStr + got_len, length - got_len);
        if (got_len == length)
        {
            //SCI_TraceLow("GpsGetString: %d", got_len);
            return got_len;
        }
        
        SCI_Sleep(GPS_GET_DELAY);
        
        time_current = SCI_GetTickCount();
        if ((time_current-time_start) >= timeout)
        {
            //SCI_TraceLow("GpsGetString: -%d", -got_len);
            return -(got_len);  
        }        
    }while (1);
}


/*****************************************************************************/
//  FUNCTION:     GpsCommandSrf
//  Description:    get a SiRF command
//  return:         >0--SiRF command ID, <0--timeout error, -read length.
//  INPUT:          uint8 *pCommand--command buffer; uint32 timeout(ms)--operation timeout.
//  Author:         David.Jia
//  date:           2007.7.12
//	Note:           internal function
/*****************************************************************************/
static int GpsCommandSrf(uint8 *pCommand, uint32 timeout)
{
    uint32 time_start, time_past = 0;
    uint16 length, pack_tail, check_sum, sum = 0;
    uint8  ch = 0, *ptr; 
    uint32 cnt = 0;
    int    t;
    
    time_start = SCI_GetTickCount();
    //GPS_LOG("GpsCommandSrf: start=%dms", time_start);
    
    do              //not found packet head 'A0'
    {   
        //find packet head
        do                  
        {
            do              
            {   
                t = GpsGetString(&ch, 1, timeout - time_past);
                cnt += t >= 0 ? t : -(t);
                if ( t <= 0)
                {
                    return -(cnt);      
                }
                //GPS_LOG("GpsCommandSrf: get 1, %x", ch);
            }while (GPS_SIRF_A0 != ch);
            
            time_past = SCI_GetTickCount() - time_start;
            t = GpsGetString(&ch, 1, timeout - time_past);
            cnt += t >= 0 ? t : -(t);
            if ( t <= 0)
            {
                return -(cnt);          
            }
            //GPS_LOG("GpsCommandSrf: get 2, %x", ch);
        }while (GPS_SIRF_A2 != ch);
        
        //read packet length
        time_past = SCI_GetTickCount() - time_start;
        t = GpsGetString((uint8 *)&length, 2, timeout - time_past);
        cnt += t >= 0 ? t : -(t);
        if ( t <= 0)
        {
            return -(cnt);          
        }
        //GPS_LOG("GpsCommandSrf: get 4, %x", length);
        //length validation?
        if (length > GPS_SIRF_COMMAND_SIZE)
        {
            continue;
        }
        else
        {
            length &= 0x7FFF;           //length only 15 bits
        }
        
        //read packet data
        time_past = SCI_GetTickCount() - time_start;
        t = GpsGetString(s_Command_Buf, length, timeout - time_past);
        cnt += t >= 0 ? t : -(t);
        if ( t <= 0)
        {
            return -(cnt);          
        }
        
        //compute check sum
        for (t = 0; t < length; t++)
        {
            sum += s_Command_Buf[t];
        }
        sum &= 0x7FFF;        //check_sum only 15 bits
        //GPS_LOG("GpsCommandSrf: check sum, %x", sum);
        //read check sum and compare
        time_past = SCI_GetTickCount() - time_start;
        t = GpsGetString((uint8 *)&check_sum, 2, timeout - time_past);
        cnt += t >= 0 ? t : -(t);
        if ( t <= 0)
        {
            return -(cnt);          
        }
        //GPS_LOG("GpsCommandSrf: get check sum, %x", check_sum);
        if (sum != check_sum)
        {
            continue;
        }
        
        //read packet tail and validation
        time_past = SCI_GetTickCount() - time_start;
        t = GpsGetString((uint8 *)&pack_tail, 2, timeout - time_past);
        cnt += t >= 0 ? t : -(t);
        if ( t <= 0)
        {
            return -(cnt);          
        }
        //GPS_LOG("GpsCommandSrf: get tail, %x", pack_tail);
        if (GPS_SIRF_B0B3 != pack_tail)
        {
            continue;
        }
           
        break;      //found a packet, break loop
    }while (1);
    
    //copy command string
    *pCommand++ = GPS_SIRF_A0;
    *pCommand++ = GPS_SIRF_A2;
    
    GPS_INT2PTR_B(pCommand, length)
    
    ptr = s_Command_Buf;
    for (t = 0; t < length; t++)
        *pCommand++ = *ptr++;
    
    GPS_INT2PTR_B(pCommand, check_sum)
    GPS_INT2PTR_B(pCommand, pack_tail)
       
    return (uint32)s_Command_Buf[0];        //command ID
}

//@David.Jia 2007.7.12  end

/**---------------------------------------------------------------------------*/
//                            Global Function Definitions
/**---------------------------------------------------------------------------*/
#define GPS_LINE_SIZE   16
uint8 line[3*GPS_LINE_SIZE+1];
//print a hex string to a printable string
//every 16 bytes a line like: A0 A2 00 18 \00
void print_hex(uint8 *pstring, uint32 length)
{
    uint8 *pline, ch;
    int row, col, t;
    
    //GPS_LOG("print_hex, pstring=%x, length=%d", (uint32)pstring, length);
    //for (t = 0; t < length/4; t++) GPS_LOG("hex: %d, %x", t, ((uint32*)pstring)[t]);

    pline = line;
    for (row = 0; row < length/GPS_LINE_SIZE; row++)
    {
        for (t = 0; t < 3*GPS_LINE_SIZE+1; t++)
            *pline++ = 0x00;
        
        pline = line;
        
        for (col = 0; col < GPS_LINE_SIZE; col++)
        {
            ch = *pstring >> 4;
            *pline++ = (ch >= 10) ? (ch-10+'A') : (ch+'0'); 
            
            ch = (*pstring++) & 0x0F;
            *pline++ = (ch >= 10) ? (ch-10+'A') : (ch+'0');
            
            *pline++ = ' '; 
        }
        
        //SCI_TraceLow:"%s"
        SCI_TRACE_ID(TRACE_TOOL_CONVERT,SRF_FUNC_341_112_2_18_0_33_23_1705,(uint8*)"s", line);
    }
    
    pline = line;
    for (t = 0; t < 3*GPS_LINE_SIZE+1; t++)
        *pline++ = 0x00;
    pline = line;
    
    for (col = 0; col < length - row*GPS_LINE_SIZE; col++)
    {
        ch = *pstring >> 4;
        *pline++ = (ch >= 10) ? (ch-10+'A') : (ch+'0'); 
            
        ch = (*pstring++) & 0x0F;
        *pline++ = (ch >= 10) ? (ch-10+'A') : (ch+'0');
           
        *pline++ = ' '; 
    }
    //SCI_TraceLow:"%s"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,SRF_FUNC_359_112_2_18_0_33_23_1706,(uint8*)"s", line);
}

/*****************************************************************************/
//  FUNCTION:     GpsTTFFSrf
//  Description:    get the Time To Frist Fix
//  return:         >0--SiRF's TTFF, <0--timeout error
//  INPUT:          uint8 *pCommand--lastest received command; uint32 timeout(ms)--operation timeout.
//  Author:         David.Jia
//  date:           2007.7.17
//	Note:           in SiRF binary mode, send Factory Reset and wait first Visible List.
/*****************************************************************************/
int GpsTTFFSrf(uint8 *pCmd, uint32 TimeOut)
{
    uint32 begin, TTFF_begin;
    int id, t;
    
    begin = SCI_GetTickCount();
    
    t = GPS_ComWrite(g_Srf_ColdReset, sizeof(g_Srf_ColdReset));
    TTFF_begin = SCI_GetTickCount();
    while (sizeof(g_Srf_ColdReset) != t)
    {
        SCI_Sleep(GPS_ACK_TIMEOUT);
        t = GPS_ComWrite(g_Srf_ColdReset, sizeof(g_Srf_ColdReset));
        if (GPS_TIMEOUT(begin, TimeOut))
        {
            return -1;
        } 
    }
    //GPS_LOG:"GpsTTFFSrf, begin time %d"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,SRF_FUNC_387_112_2_18_0_33_23_1707,(uint8*)"d", TTFF_begin);
    
    t = 0;  //NO get it
    do
    {
        id = GpsCommandSrf(pCmd, GPS_ACK_TIMEOUT);
        //GPS_LOG:"GpsTTFFSrf, response 0x%x"
        SCI_TRACE_ID(TRACE_TOOL_CONVERT,SRF_FUNC_393_112_2_18_0_33_23_1708,(uint8*)"d", id);
        if (GPS_TIMEOUT(begin, TimeOut))
        {
            return -2;        
        }
        
        //get visible list 
        if (GPS_SIRF_VISIBLE_LIST == id)    t = 1;
        
        //get validated lat/long
        
    }while (0 == t); 
    
    //GPS_LOG:"GpsTTFFSrf, end time %d"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,SRF_FUNC_406_112_2_18_0_33_23_1709,(uint8*)"d", SCI_GetTickCount());
    return SCI_GetTickCount() - begin;
}

/*****************************************************************************/
//  FUNCTION:     GpsTest4Srf
//  Description:    switch test mode 4 and get 12 channels CNo value
//  return:         >0--good, <0--timeout error
//  INPUT:          uint8 *pCmd--lastest received command 4; 
//      uint16   sv--SV id, all channel to recieved;
//      uint32 timeout(ms)--operation timeout.
//  Author:         David.Jia
//  date:           2007.7.18
//	Note:           in SiRF binary mode, send SiRF CMD 150 and wait for CMD 4.
/*****************************************************************************/
int GpsTest4Srf(uint8 *pCmd, uint16 SV, uint32 TimeOut)
{
    uint32 begin;
    int id, t;
    uint8 *pCMD150;
    uint16 check_sum = 0;
        
    begin = SCI_GetTickCount();
    
    //build command 150 with new SV and check sum
    pCMD150 = &g_Srf_Test4[GPS_SIRF_SVID_OFFSET];
    SCI_ASSERT(SV <= 32);/*assert verified*/
    GPS_INT2PTR_B(pCMD150, SV);
    for (t = 0; t < GPS_SIRF_CMD150_CHECKSUM_OFFSET - GPS_SIRF_ID_OFFSET; t++)
    {
        check_sum += g_Srf_Test4[t + GPS_SIRF_ID_OFFSET];
    }
    pCMD150 = &g_Srf_Test4[GPS_SIRF_CMD150_CHECKSUM_OFFSET];
    GPS_INT2PTR_B(pCMD150, check_sum);
    
    //GPS_LOG:"GPS test mode 4:"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,SRF_FUNC_439_112_2_18_0_33_23_1710,(uint8*)"");
    print_hex(g_Srf_Test4, sizeof(g_Srf_Test4));

GPS_send_test4:
   
    t = GPS_ComWrite(pCMD150, sizeof(g_Srf_Test4));

    while (sizeof(g_Srf_Test4) != t)
    {
        SCI_Sleep(GPS_ACK_TIMEOUT);
        t = GPS_ComWrite(pCMD150, sizeof(g_Srf_Test4));
        if (GPS_TIMEOUT(begin, TimeOut))
        {
            return -1;
        } 
    }
    
    id = GpsCommandSrf(pCmd, GPS_ACK_TIMEOUT);
    if ((GPS_SIRF_COMMAND_ACK!=id) && !GPS_TIMEOUT(begin, TimeOut))
    {
        //GPS_LOG:"GpsTest4Srf: reset No ack."
        SCI_TRACE_ID(TRACE_TOOL_CONVERT,SRF_FUNC_459_112_2_18_0_33_23_1711,(uint8*)"");
        goto GPS_send_test4;
    }
    else
    {
        return -2;
    }
    
    //GPS_LOG:"GpsTest4Srf, begin %d"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,SRF_FUNC_467_112_2_18_0_33_23_1712,(uint8*)"d", begin);
    
    do
    {
        id = GpsCommandSrf(pCmd, GPS_ACK_TIMEOUT);
        //GPS_LOG:"GpsTest4Srf, response 0x%x"
        SCI_TRACE_ID(TRACE_TOOL_CONVERT,SRF_FUNC_472_112_2_18_0_33_23_1713,(uint8*)"d", id);
        if (GPS_TIMEOUT(begin, TimeOut))
        {
            return -3;        
        }
    }while (GPS_SIRF_MEASURED_TRACKING_DATA != id); 
    
    //GPS_LOG:"GpsTest4Srf, end %d"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,SRF_FUNC_479_112_2_18_0_33_23_1714,(uint8*)"d", SCI_GetTickCount());
    return SCI_GetTickCount() - begin;
}

/*****************************************************************************/
//  FUNCTION:     GpsGetTTFFSrf
//  Description:    get the Time To Frist Fix
//  return:         >0--SiRF's TTFF, <0--timeout error
//  INPUT:          uint32 timeout(ms)--operation timeout.
//  Author:         David.Jia
//  date:           2007.7.17
//	Note:           use g_Command_Buf to call GpsTTFFSrf.
/*****************************************************************************/
int GpsGetTTFFSrf(uint32 TimeOut)
{
    int TTFF;
    
    //@David.Jia 2007.8.3   
    //switch from NMEA 9600bps mode to SiRF binary
    GPS_ComWrite(g_Srf_NMEA2Bin, sizeof(g_Srf_NMEA2Bin));
    SCI_Sleep(2000);
    UART_SetBaudSpeed(COM_1, BAUD_57600);
    
    TTFF = GpsTTFFSrf(s_gps_command, TimeOut);
    
    //@David.Jia 2007.8.3   
    //switch from SiRF binary to NMEA 9600bps mode
    GPS_ComWrite(g_Srf_Bin2NMEA, sizeof(g_Srf_Bin2NMEA));
    SCI_Sleep(2000);
    UART_SetBaudSpeed(COM_1, BAUD_9600);
    
    return TTFF;
}


/*****************************************************************************/
//  FUNCTION:     GpsTestSrf(void)
//  Description:    test SiRF GPS
//  return:         0--ok, <0--error
//  INPUT:          
//  Author:         David.Jia
//  date:           2007.7.13
//	Note:           
/*****************************************************************************/
uint8 g_command[1031];
int GpsTestSrf(void)
{    
    uint8 id, *ptr = g_command;
    uint16 t;
    
#if 0    //NMEA output 

    t = Map_Write(g_Srf_Bin2NMEA, sizeof(g_Srf_Bin2NMEA));
    //GPS_LOG:"GPS: write command %d"
    SCI_TRACE_ID(TRACE_TOOL_CONVERT,SRF_FUNC_528_112_2_18_0_33_23_1715,(uint8*)"d", t);
    
    SCI_Sleep(1000);
    UART_SetBaudSpeed(COM_1, BAUD_9600);
    
    while (1)
    {
        t = Map_Read(g_command, 256);
        
        if (0 != t)
        {
            g_command[t] = '\0';
            //GPS_LOG:"GPS: NMEA, %s"
            SCI_TRACE_ID(TRACE_TOOL_CONVERT,SRF_FUNC_540_112_2_18_0_33_23_1716,(uint8*)"s", g_command);
        }
        else
        {
            SCI_Sleep(100);
        }
    }

#endif  //NMEA output    

#if 0   //SiRF binary get
    for (;;)
    {
        //SCI_TraceLow:"GpsTestSrf loop"
        SCI_TRACE_ID(TRACE_TOOL_CONVERT,SRF_FUNC_553_112_2_18_0_33_23_1717,(uint8*)"");
        id = GpsCommandSrf(g_command, 1000);
        
        if ( id > 0)
        {
            //SCI_TraceLow:"GPS: id=%x"
            SCI_TRACE_ID(TRACE_TOOL_CONVERT,SRF_FUNC_558_112_2_18_0_33_23_1718,(uint8*)"d", id);
            
            ptr += 2;
            GPS_PTR2INT_B(t, ptr)
            
            print_hex(g_command, t);
        }
        else
        {
            //SCI_TraceLow:"GPS: skip"
            SCI_TRACE_ID(TRACE_TOOL_CONVERT,SRF_FUNC_567_112_2_18_0_33_23_1719,(uint8*)"");
        }
        SCI_Sleep(1000);
    }

#endif  //SiRF binary get

#if 1    //cold reset and TTFF time
    for (;;)
    {
        uint32 TTFF_time = 0;
        
        TTFF_time = GpsGetTTFFSrf(100*1000);
        if (TTFF_time > 0)
        {
            //GPS_LOG:"GpsTestSrf, TTFF time %d"
            SCI_TRACE_ID(TRACE_TOOL_CONVERT,SRF_FUNC_582_112_2_18_0_33_23_1720,(uint8*)"d", TTFF_time);
        }
        else
        {
            //GPS_LOG:"GpsTestSrf, GpsTTFFSrf error %d"
            SCI_TRACE_ID(TRACE_TOOL_CONVERT,SRF_FUNC_586_112_2_18_0_33_23_1721,(uint8*)"d", TTFF_time);
        }
    }   //end for

#endif  //cold reset and TTFF time    

#if 0    //test mode 4 
    //GPS_LOG("sizeof 150/4/CH: %d/%d/%d", sizeof(GPS_SIRF_CMD150), sizeof(GPS_SIRF_CMD4), sizeof(SIRF_CH_DATA));
    
    for (;;)
    {
        uint8 *pCMD4;
        int t;
        
        do
        {
            t = GpsCommandSrf(g_command, 1);
            //GPS_LOG:"GpsTestSrf, skip %d"
            SCI_TRACE_ID(TRACE_TOOL_CONVERT,SRF_FUNC_603_112_2_18_0_33_23_1722,(uint8*)"d", t);
        }while ( t > 0);
        
        t = GpsTest4Srf(g_command, 16, 3*1000);
        pCMD4 = g_command;
        
        if (t >= 0)
        {
            //GPS_LOG:"\"#GPS, GpsTest4Srf"
            SCI_TRACE_ID(TRACE_TOOL_CONVERT,SRF_FUNC_611_112_2_18_0_33_23_1723,(uint8*)"");
            print_hex(pCMD4, 196);     
        }
        else
        {
            //GPS_LOG:"GpsTest4Srf, error %d"
            SCI_TRACE_ID(TRACE_TOOL_CONVERT,SRF_FUNC_616_112_2_18_0_33_23_1724,(uint8*)"d", t);
        }
        
        for (t = 0; t < 1000; t++)
        {
            id = GpsCommandSrf(g_command, GPS_ACK_TIMEOUT);
             
            //GPS_LOG:"GpsTest4Srf, response 0x%x"
            SCI_TRACE_ID(TRACE_TOOL_CONVERT,SRF_FUNC_623_112_2_18_0_33_23_1725,(uint8*)"d", id);
            
            if (GPS_SIRF_MEASURED_TRACKING_DATA == id)
            {
                print_hex(g_command, 196);    
            }                
        }  
    }   //end for

#endif  //cold reset and TTFF time            
    return 0;
}




