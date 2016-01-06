#include <utfstring.h>
#include <vm_common.h>
#include "sci_types.h"
#include "os_api.h"
#include "Sig_code.h"
#include "nativeMyMessageSender.h"

#if defined(ARCH_ARM_SPD)
/** phone number length */
#define PHONE_NUM_MAX_SIZE (32)

//define for test
#define MAX_CONTENT_LENGTH  256

#define JAVA_DATE_MIN_YEAR  1980   //时间设置最小年份

/* Bcd Values for Dial Number storage */
#define   DIALBCD_0             0x0
#define   DIALBCD_1             0x1
#define   DIALBCD_2             0x2
#define   DIALBCD_3             0x3
#define   DIALBCD_4             0x4
#define   DIALBCD_5             0x5
#define   DIALBCD_6             0x6
#define   DIALBCD_7             0x7
#define   DIALBCD_8             0x8
#define   DIALBCD_9             0x9
#define   DIALBCD_STAR          0xA
#define   DIALBCD_HASH          0xB
#define   DIALBCD_PAUSE         0xC
#define   DIALBCD_WILD          0xD
#define   DIALBCD_EXPANSION     0xE
#define   DIALBCD_FILLER        0xF

#define MMI_MEMCPY(_DEST_PTR, _DEST_LEN, _SRC_PTR, _SRC_LEN, _SIZE) \
        SCI_PASSERT((_SIZE) <= (_DEST_LEN), ("MMI_MEMCPY: the _DEST_LEN is too small!")); \
        SCI_PASSERT((_SIZE) <= (_SRC_LEN),  ("MMI_MEMCPY: the _SRC_LEN is too small!")); \
        SCI_MEMCPY((_DEST_PTR), (_SRC_PTR), (_SIZE))

// translate the struct from MMI_PARTY_NUM_T to MN_CALL_NUM_T
#define MMIPARTYNUM_2_MNCALLEDNUM( _MMI_PARTY_NUM, _MN_CALL_NUM_PTR )                            \
        {                                                                                        \
            _MN_CALL_NUM_PTR->number_type = _MMI_PARTY_NUM->number_type;                            \
            _MN_CALL_NUM_PTR->num_len      = MIN( _MMI_PARTY_NUM->num_len, MN_MAX_ADDR_BCD_LEN );    \
            SCI_MEMCPY(                                                                            \
                        (void*)_MN_CALL_NUM_PTR->party_num,                                        \
                        (void*)_MMI_PARTY_NUM->bcd_num ,                                            \
                        _MN_CALL_NUM_PTR->num_len                                                \
                        );                                                                        \
        }

/*****************************************************************************/
//  Description : convert the string to the bcd code(PACKED_MSB_FIRST)
/*****************************************************************************/
PUBLIC void MMIAPICOM_StrToBcdMf(
                    char *pStr,   // String的头指针
                    uint8 strLen, // String的长度
                    uint8 *pBcd   // 转换后BCD码的头指针
                   )
{
    uint8 bcdCode = 0;
    uint8 ascCode = 0;
    int32 i = 0;

  /*memset(pBcd,0xFF,(strLen&1) ? (strLen+1)/2 : strLen/2+1); */

    for(i=0;i < strLen;i++)
    {
        ascCode = pStr[i];
        /*bcdCode = 0xFF;*/

        if(!((ascCode >= '0' && ascCode <= '9')||ascCode == '*'||
              ascCode == '#'||(ascCode == 'P'||ascCode == 'p')||
              ascCode == 'W'||ascCode == 'w'))
            break;
        /*
        bcdCode = ( ascCode == '*' ) ? DIALBCD_STAR:
                  ( ascCode == '#' ) ? DIALBCD_HASH:
                  ( ascCode == 'P'||ascCode == 'p') ? DIALBCD_PAUSE:
                  ( ascCode == 'W' ) ? DIALBCD_WILD:(ascCode - '0');
        */
        switch( ascCode )
        {
            case '*':
                bcdCode = DIALBCD_STAR;
                break;
            case '#':
                bcdCode = DIALBCD_HASH;
                break;
            case 'P':
            case 'p':
                bcdCode = DIALBCD_PAUSE;
                break;
            case 'W':
            case 'w':
                bcdCode = DIALBCD_WILD;
                break;
            default:
                bcdCode = ascCode - '0';
                break;
        }
        pBcd[i/2] = ((i & 1) ? pBcd[i/2] : 0) +
                    (bcdCode << ((i+1 & 1) ? 4 : 0));
    }
    if(i&1)
    {
       pBcd[(i-1)/2] |= 0x0f;
    }

}   /* End Of MMI_StrToBcdMf() */

/*****************************************************************************/
//  Description : convert the string to the bcd code(PACKED_LSB_FIRST)
//  Global resource dependence :
//  Author: Great.Tian
//  Note:
/*****************************************************************************/
LOCAL void MMIAPICOM_StrToBcdLf(
                    char *pStr,   // String的头指针
                    uint8 strLen, // String的长度
                    uint8 *pBcd   // 转换后BCD码的头指针
                   )
{
    uint8 bcdCode = 0;
    uint8 ascCode = 0;
    int32 i = 0;

    /*memset(pBcd,0xFF,(strLen&1) ? (strLen+1)/2 : strLen/2+1);*/

    for(i=0;i < strLen;i++)
    {
        ascCode = pStr[i];
        /*bcdCode = 0xFF;*/

        if(!((ascCode >= '0' && ascCode <= '9')||ascCode == '*'||
              ascCode == '#'||(ascCode == 'P'||ascCode == 'p')||
              ascCode == 'W'||ascCode == 'w'||ascCode=='+'))
            break;
        /*
        bcdCode = ( ascCode == '*' ) ? DIALBCD_STAR:
                  ( ascCode == '#' ) ? DIALBCD_HASH:
                  ( ascCode == 'P'||ascCode == 'p') ? DIALBCD_PAUSE:
                  ( ascCode == 'W' ) ? DIALBCD_WILD:(ascCode - '0');
        */
        switch( ascCode )
        {
            case '*':
                bcdCode = DIALBCD_STAR;
                break;
            case '#':
                bcdCode = DIALBCD_HASH;
                break;
            case 'P':
            case 'p':
                bcdCode = DIALBCD_PAUSE;
                break;
            case 'W':
            case 'w':
                bcdCode = DIALBCD_WILD;
                break;
            case '+':
            bcdCode=DIALBCD_EXPANSION;
            break;
            default:
                bcdCode = ascCode - '0';
                break;
        }
        pBcd[i/2] = ((i & 1) ? pBcd[i/2] : 0) +
                    (bcdCode << ((i & 1) ? 4 : 0));
    }
    if(i&1)
    {
       pBcd[(i-1)/2] |= 0xf0;
    }

}   /* End Of MMI_StrToBcdLf() */

/*****************************************************************************/
//  Description : convert the string to the bcd code(UNPACKED)
//  Global resource dependence :
//  Author: Great.Tian
//  Note:
/*****************************************************************************/
LOCAL void MMIAPICOM_StrToBcdUp(
                    char *pStr,   // String的头指针
                    uint8 strLen, // String的长度
                    uint8 *pBcd   // 转换后BCD码的头指针
                   )
{
    uint8 bcdCode = 0;
    uint8 ascCode = 0;
    int32 i = 0;

    /*memset(pBcd,0xFF,strLen+1);*/

    for(i=0;i < strLen;i++)
    {
        ascCode = pStr[i];

        if(!((ascCode >= '0' && ascCode <= '9')||ascCode == '*'||
              ascCode == '#'||(ascCode == 'P'||ascCode == 'p')||
              ascCode == 'W'||ascCode == 'w'))
            break;
        /*
        bcdCode = ( ascCode == '*' ) ? DIALBCD_STAR:
                  ( ascCode == '#' ) ? DIALBCD_HASH:
                  ( ascCode == 'P'||ascCode == 'p') ? DIALBCD_PAUSE:
                  ( ascCode == '+' ) ? DIALBCD_WILD:(ascCode - '0');
        */

        switch( ascCode )
        {
            case '*':
                bcdCode = DIALBCD_STAR;
                break;
            case '#':
                bcdCode = DIALBCD_HASH;
                break;
            case 'P':
            case 'p':
                bcdCode = DIALBCD_PAUSE;
                break;
            case 'W':
            case 'w':
                bcdCode = DIALBCD_WILD;
                break;
            default:
                bcdCode = ascCode - '0';
                break;
        }
        pBcd[i] = bcdCode;
    }
}   /* End Of MMI_StrToBcdUp() */

LOCAL void MMIAPICOM_StrToBcd(
                  uint8 bcdFmt, // BCD码的类型，目前有三种: PACKED_LSB_FIRST, PACKED_MSB_FIRST, UNPACKED
                  char *pStr,   // String的头指针
                  uint8 *pBcd   // 转换后BCD码的头指针
                 )
{
    uint8 strLen = 0;

    strLen = strlen(pStr);

    switch(bcdFmt)
    {
        case 0://PACKED_LSB_FIRST:
            MMIAPICOM_StrToBcdLf(pStr,strLen,pBcd);
            break;
        case 1://PACKED_MSB_FIRST:
            MMIAPICOM_StrToBcdMf(pStr,strLen,pBcd);
            break;
        case 2://UNPACKED        :
            MMIAPICOM_StrToBcdUp(pStr,strLen,pBcd);
            break;
        default:
            // DevFail("Wrong bcd format");
            break;
    }
}   /* End Of mmi_StrToBcd() */

/*****************************************************************************/
//  Description : convert the bcd code(PACKED_MSB_FIRST) to the string
//  Global resource dependence :
//  Author: Great.Tian
//  Note:
/*****************************************************************************/
LOCAL void MMIAPICOM_BcdMfToStr(
                    uint8 *pBcd,  // BCD码的头指针
                    uint8 bcdLen, // BCD码的长度
                    char* str     // 转换后String的头指针
                   )
{   /*BCD format - MSB first (1234 = 0x12 0x34)*/
    int32 i = 0;
    uint8 bcdCode = 0;
    uint8 ascCode = 0;

    for(i = 0;i < bcdLen; i++)
    {
        bcdCode = (pBcd[i/2] >> ((i & 1) ? 0 : 4)) & 0x0F;
        if(bcdCode == 0x0f) break;
        ascCode = (bcdCode == DIALBCD_STAR) ? '*':
                  (bcdCode == DIALBCD_HASH) ? '#':
                  (bcdCode == DIALBCD_PAUSE)? 'P':
                  (bcdCode == DIALBCD_WILD) ? 'w': (bcdCode + '0');

        if(!((ascCode >= '0' && ascCode <= '9')||ascCode == '*'||
              ascCode == '#'||(ascCode == 'P'||ascCode == 'p')||
              ascCode == 'W'||ascCode == 'w'))
           ascCode = 0;

        str[i] = ascCode;
    }

    str[i] = 0;
}  /*-- End of MMI_BcdMfToStr( ) --*/

/*****************************************************************************/
//  Description : convert the bcd code(PACKED_LSB_FIRST) to the string
//  Global resource dependence :
//  Author: Great.Tian
//  Note:
/*****************************************************************************/
PUBLIC void MMIAPICOM_BcdLfToStr(
                    uint8 *pBcd,  // BCD码的头指针
                    uint8 bcdLen, // BCD码的长度
                    char* str     // 转换后String的头指针
                   )
{   /*BCD format - LSB first (1234 = 0x21, 0x43)*/
    int32 i = 0;
    uint8 bcdCode = 0;
    uint8 ascCode = 0;

    for(i = 0;i < bcdLen; i++)
    {
        bcdCode = (pBcd[i/2] >> (((i+1) & 1) ? 0 : 4)) & 0x0F;
        if(bcdCode == DIALBCD_FILLER)
        {
            break;
        }
        ascCode = (bcdCode == DIALBCD_STAR) ? '*':
                  (bcdCode == DIALBCD_HASH) ? '#':
                  (bcdCode == DIALBCD_PAUSE)? 'P':
                  (bcdCode == DIALBCD_WILD) ? 'W':
                  (bcdCode == DIALBCD_EXPANSION)?'+':
                  (bcdCode + '0');


        if(!((ascCode >= '0' && ascCode <= '9')||ascCode == '*'||
              ascCode == '#'||(ascCode == 'P'||ascCode == 'p')||
              ascCode == 'W'||ascCode == 'w'||ascCode == '+'))
           ascCode = 0;

        str[i] = ascCode;
    }

    str[i] = 0;
}  /*-- End of MMI_BcdLfToStr( ) --*/

/*****************************************************************************/
//  Description : convert the bcd code(UNPACKED) to the string
//  Global resource dependence :
//  Author: Great.Tian
//  Note:
/*****************************************************************************/
PUBLIC void MMIAPICOM_BcdUpToStr(
                    uint8 *pBcd,  // BCD码的头指针
                    uint8 bcdLen, // BCD码的长度
                    char *str     // 转换后String的头指针
                   )
{   /*BCD format - unpacked (1 digit per byte)*/
    int32 i = 0;
    uint8 bcdCode = 0;
    uint8 ascCode = 0;

    for(i = 0;i < bcdLen; i++)
    {
        bcdCode = pBcd[i];
        ascCode = (bcdCode == DIALBCD_STAR) ? '*':
                  (bcdCode == DIALBCD_HASH) ? '#':
                  (bcdCode == DIALBCD_PAUSE)? 'P':
                  (bcdCode == DIALBCD_WILD) ? 'w': (bcdCode + '0');

        if(!((ascCode >= '0' && ascCode <= '9')||ascCode == '*'||
              ascCode == '#'||(ascCode == 'P'||ascCode == 'p')||
              ascCode == 'W'||ascCode == 'w'))
           ascCode = 0;

        str[i] = ascCode;
    }

    str[i] = 0;
}  /*-- End of MMI_BcdUpToStr( ) --*/

/*****************************************************************************/
//  Description : convert the bcd code to string
//  Global resource dependence :
//  Author: Great.Tian
//  Note:
/*****************************************************************************/
PUBLIC void MMIAPICOM_BcdToStr(
                  uint8 bcdFmt, // BCD码的类型，目前有三种: PACKED_LSB_FIRST, PACKED_MSB_FIRST, UNPACKED
                  uint8 *pBcd,  // BCD码的头指针
                  uint8 bcdLen, // BCD码的长度
                  char* str     // 转换后String的头指针
                 )
{
    /* BCD format is defined in applayer.h                    */
    /* NOTE: LSB first 1234 = 0x21 0x43, bcdLen = 4           */
    /*                 123  = 0x21 0xf3, bcdLen = 3           */
    /*       MSB first 1234 = 0x12 0x34, bcdLen = 4           */
    /*                 123  = 0x12 0x3f, bcdLen = 3           */
    /*       unpacked  1234 = 0x01 0x02 0x03 0x04, bcdLen = 4 */
    switch(bcdFmt)
    {
        case 0: //PACKED_LSB_FIRST:
            MMIAPICOM_BcdLfToStr(pBcd,bcdLen,str);
            break;
        case 1: //PACKED_MSB_FIRST:
            MMIAPICOM_BcdMfToStr(pBcd,bcdLen,str);
            break;
        case 2: //UNPACKED:
            MMIAPICOM_BcdUpToStr(pBcd,bcdLen,str);
            break;
        default:
            // DevFail("wrong bcd format");
            break;
    }
}   /* End Of mmi_BcdToStr() */

LOCAL void InitUserDataHeadForJAVA(
                                   int32             srcPort,
                                   int32             dstPort,
                                   MN_SMS_USER_DATA_HEAD_T  *data_head_ptr,
                                   BOOLEAN           is_longmsg
                                   )
{
    if (PNULL == data_head_ptr)
    {
        DVMTraceDbg( "[INFO][MESSAGE]:InitUserDataHeadForJAVA data_head_ptr = PNULL");
        return;
    }

    DVMTraceDbg("[INFO][MESSAGE]: InitUserDataHeadForJAVA (srcPort:%d, dstPort:%d)", srcPort ,dstPort);

    if (!is_longmsg)        //is not a long sms
    {
        data_head_ptr->length                                 = JAVASMS_HEAD_LENGTH_FOR_PORT;
        data_head_ptr->user_header_arr[ JAVASMS_HEAD_IEI_POS ] = JAVASMS_HEAD_IEI_FOR_PORT;
        data_head_ptr->user_header_arr[ JAVASMS_HEAD_IEL_POS ] = JAVASMS_HEAD_IEL_FOR_PORT;

        data_head_ptr->user_header_arr[ JAVASMS_HEAD_SRC_PORTH_POS ] = ((uint32)dstPort>>8)&(0x00ff);
        data_head_ptr->user_header_arr[ JAVASMS_HEAD_SRC_PORTL_POS ] =     dstPort&0x00ff;
        data_head_ptr->user_header_arr[ JAVASMS_HEAD_DST_PORTH_POS ] = ((uint32)srcPort>>8)&(0x00ff);
        data_head_ptr->user_header_arr[ JAVASMS_HEAD_DST_PORTL_POS ] = srcPort&(0x00ff);
    }
    else
    {
        data_head_ptr->length                               += JAVASMS_HEAD_LENGTH_FOR_PORT;
        data_head_ptr->user_header_arr[ JAVASMS_HEAD_IEI_POS+JAVASMS_HEAD_8BIT_LENGTH ] = JAVASMS_HEAD_IEI_FOR_PORT;
        data_head_ptr->user_header_arr[ JAVASMS_HEAD_IEL_POS+JAVASMS_HEAD_8BIT_LENGTH ] = JAVASMS_HEAD_IEL_FOR_PORT;

        data_head_ptr->user_header_arr[ JAVASMS_HEAD_SRC_PORTH_POS+JAVASMS_HEAD_8BIT_LENGTH ] = ((uint32)dstPort>>8)&(0x00ff);
        data_head_ptr->user_header_arr[ JAVASMS_HEAD_SRC_PORTL_POS+JAVASMS_HEAD_8BIT_LENGTH ] = dstPort&0x00ff;
        data_head_ptr->user_header_arr[ JAVASMS_HEAD_DST_PORTH_POS+JAVASMS_HEAD_8BIT_LENGTH ] = ((uint32)srcPort>>8)&(0x00ff);
        data_head_ptr->user_header_arr[ JAVASMS_HEAD_DST_PORTL_POS+JAVASMS_HEAD_8BIT_LENGTH ] = srcPort&(0x00ff);
    }

}

LOCAL uint32 GUI_UCS2B2UCS2LWithEndPos(//ucs2l len
                              uint8 *ucs2l_ptr,//out
                              uint32 ucs2l_buf_len,//in
                              const uint8 *ucs2b_ptr,//in
                              uint32 ucs2b_len,//in
                              uint32 *src_end_pos
                              )
{
    uint32 ucs2l_len = 0;

    if (PNULL == ucs2l_ptr || PNULL == ucs2b_ptr || ucs2l_buf_len < ucs2b_len)
    {
        return ucs2l_len;
    }

    ucs2l_len = ucs2b_len / 2 * 2;

    while(ucs2b_len > 1)
    {
        *(ucs2l_ptr + 1) = *ucs2b_ptr++;
        *ucs2l_ptr = *ucs2b_ptr++;
        ucs2l_ptr += 2;
        ucs2b_len -= 2;
    }

    if(src_end_pos != PNULL)
    {
        *src_end_pos = (uint32)(ucs2b_ptr);
    }
    return ucs2l_len;
}

LOCAL void JAVASMS_GetUserData(
                                uint8     *sms_data_ptr,
                                int32      sms_data_len,
                                int32       type,
                                JAVA_SMS_USER_DATA_T   *user_data_ptr)
{
    /* Currently we only send a single message.  */
    if (type == MN_SMS_8_BIT_ALPHBET)
    {
      user_data_ptr->user_valid_data_t.length = MIN(MMISMS_JAVA_MAX_8BIT_BYTE_LEN, sms_data_len);
    }
    else //MN_SMS_UCS2_ALPHABET
    {
      user_data_ptr->user_valid_data_t.length = MIN(MMISMS_JAVA_MAX_UCS2_BYTE_LEN, sms_data_len);
    }

#if defined(__BIG_ENDIAN) || defined(__BigEndian)
    SCI_MEMCPY(
      user_data_ptr->user_valid_data_t.user_valid_data_arr,
      sms_data_ptr,
      user_data_ptr->user_valid_data_t.length
      );
#else
      if (MN_SMS_UCS2_ALPHABET == type)
      {
          GUI_UCS2B2UCS2LWithEndPos((uint8 *)user_data_ptr->user_valid_data_t.user_valid_data_arr,
                                     user_data_ptr->user_valid_data_t.length ,
                                     sms_data_ptr,
                                     user_data_ptr->user_valid_data_t.length,
                                     NULL);
      }
      else
      {
          SCI_MEMCPY(
              user_data_ptr->user_valid_data_t.user_valid_data_arr,
              sms_data_ptr,
              user_data_ptr->user_valid_data_t.length);
      }
#endif
}

/*****************************************************************************/
//  Description : for telenumber to translate the string to bcd
/*****************************************************************************/
LOCAL BOOLEAN MMIAPICOM_GenPartyNumber(
                                        uint8               *tele_num_ptr,
                                        int16               tele_len,
                                        JAVA_PARTY_NUMBER_T *party_num
                                        )
{
    uint8   offset   = 0;
    uint8*  tele_num = PNULL;

    //check the param
    if (PNULL == tele_num_ptr || PNULL == party_num)
    {
        DVMTraceDbg("MMIAPICOM_GenPartyNumber PNULL == tele_num_ptr || PNULL == party_num");
        return FALSE;
    }
    if (0 == tele_len)
    {
        // An empty string is given. The length of the BCD number is set to 0
        party_num->num_len = 0;
        DVMTraceDbg("MMI_GenPartyNumber  length = 0\n");
        return FALSE;
    }

    // @Great.Tian, CR12051, begin
    tele_len = MIN(40 /*MMIPB_NUMBER_MAX_STRING_LEN*/, tele_len);
    tele_num = (uint8*)SCI_ALLOC_APP(tele_len + 1);
    SCI_MEMSET(tele_num, 0, (tele_len + 1));
    // SCI_MEMCPY(tele_num, tele_num_ptr, tele_len);
    MMI_MEMCPY(
        tele_num,
        tele_len + 1,
        tele_num_ptr,
        tele_len,
        tele_len);
    // @Great.Tian, CR12051, end

    if('+' == tele_num[0])
    {
        party_num->number_type = MN_NUM_TYPE_INTERNATIONAL;
        offset++;
    }
    else
    {
        party_num->number_type = MN_NUM_TYPE_UNKNOW;
    }

    party_num->num_len = ( tele_len - offset + 1 ) /2;
    SCI_ASSERT( party_num->num_len <=  MN_MAX_ADDR_BCD_LEN);/*assert verified*/
    MMIAPICOM_StrToBcd( 0 /*PACKED_LSB_FIRST*/,
                (char*)( tele_num + offset),
                party_num->bcd_num );

    // @Great.Tian, CR12051, begin
    SCI_FREE(tele_num);
    // @Great.Tian, CR12051, end

    return TRUE;

}

LOCAL void JAVASMS_GetMnCalledNum(
                                    uint8 *dest_ptr,
                                    uint8 dest_len,
                                    JAVA_PARTY_NUMBER_T *party_number_ptr,    //OUT:
                                    MN_CALLED_NUMBER_T *dest_number_ptr     //OUT:
                                    )
{
    uint8 max_dest_num[MMISMS_RECIEVER_NUM_MAX] = {0};

    if (PNULL == dest_ptr || PNULL == party_number_ptr || PNULL == dest_number_ptr)
    {
        DVMTraceDbg( "[INFO][MESSAGE]:JAVASMS_GetMnCalledNum dest_ptr party_number_ptr dest_number_ptr = PNULL");
        return;
    }
    // init the destination number
    dest_number_ptr->num_len     = 0;
    dest_number_ptr->number_plan = MN_NUM_PLAN_UNKNOW;
    dest_number_ptr->number_type = MN_NUM_TYPE_UNKNOW;

    if(dest_len > MMISMS_RECIEVER_NUM_MAX)
    {
        dest_len = MMISMS_RECIEVER_NUM_MAX;
    }

    SCI_MEMSET(max_dest_num, 0x00, sizeof(max_dest_num));
    SCI_MEMCPY(max_dest_num, dest_ptr, dest_len);

    if (dest_len > 0)
    {
        MMIAPICOM_GenPartyNumber((uint8*)max_dest_num, dest_len, party_number_ptr);
        MMIPARTYNUM_2_MNCALLEDNUM(party_number_ptr, dest_number_ptr);
    }
}

/*****************************************************************************/
//     Description : to get the dest addr
/*****************************************************************************/
LOCAL void MMISMS_GetMnCalledNum(
                                            uint8 *dest_ptr,
                                            uint8 dest_len,
                                            JAVA_PARTY_NUMBER_T *party_number_ptr,    //OUT:
                                            MN_CALLED_NUMBER_T *dest_number_ptr     //OUT:
                                            )
{
    uint8 max_dest_num[MMISMS_RECIEVER_NUM_MAX] = {0};

    if (PNULL == dest_ptr || PNULL == party_number_ptr || PNULL == dest_number_ptr)
    {
        DVMTraceDbg("MMISMS:MMISMS_GetMnCalledNum dest_ptr party_number_ptr dest_number_ptr = PNULL");
        return;
    }
    // init the destination number
    dest_number_ptr->num_len     = 0;
    dest_number_ptr->number_plan = MN_NUM_PLAN_UNKNOW;
    dest_number_ptr->number_type = MN_NUM_TYPE_UNKNOW;

    if(dest_len > MMISMS_RECIEVER_NUM_MAX)
    {
        dest_len = MMISMS_RECIEVER_NUM_MAX;
    }

    SCI_MEMSET(max_dest_num, 0x00, sizeof(max_dest_num));
    SCI_MEMCPY(max_dest_num, dest_ptr, dest_len);

    if (dest_len > 0)
    {
        MMIAPICOM_GenPartyNumber((uint8*)max_dest_num, dest_len, party_number_ptr);
        MMIPARTYNUM_2_MNCALLEDNUM(party_number_ptr, dest_number_ptr);
    }
}


LOCAL BOOLEAN JAVASMS_GetDestNumber(
                                    uint8 *addr_ptr,  //IN
                                    uint8 addr_len,
                                    MN_CALLED_NUMBER_T    *dest_number_ptr    //OUT:
                                    )
{
    BOOLEAN   ret_val = FALSE;
    uint8 addr_id = 0;
    uint8 *dest_ptr = PNULL;
    uint8 dest_len = 0;
    JAVA_PARTY_NUMBER_T party_number = {MN_NUM_TYPE_UNKNOW,0,0};

    if (PNULL == dest_number_ptr)
    {
        DVMTraceDbg( "[INFO][MESSAGE]:MMISMS_GetDestNumber dest_number_ptr = PNULL");
        return FALSE;
    }

    // init the destination number
    dest_number_ptr->num_len     = 0;
    dest_number_ptr->number_plan = MN_NUM_PLAN_UNKNOW;
    dest_number_ptr->number_type = MN_NUM_TYPE_UNKNOW;

    dest_ptr = addr_ptr;
    dest_len = addr_len;

    if (dest_len > 0)
    {
        MMISMS_GetMnCalledNum(dest_ptr, dest_len, &party_number, dest_number_ptr);
        ret_val = TRUE;
    }

    return (ret_val);
}

LOCAL void JAVASMS_EncodeMoSmsData(
                                   MN_DUAL_SYS_E    dual_sys,
                                   MN_SMS_MO_SMS_T            *mo_sms_ptr,        //OUT:
                                   MN_SMS_ALPHABET_TYPE_E    alphabet_type,        //IN:
                                   JAVA_SMS_USER_DATA_T         *user_data_ptr        //IN:
                                   )
{
    MN_SMS_USER_VALID_DATA_T data_buf_t = {0};

    if (PNULL == mo_sms_ptr || PNULL == user_data_ptr)
    {
        DVMTraceDbg( "[INFO][MESSAGE]:MMISMS_EncodeMoSmsData mo_sms_ptr user_data_ptr = PNULL");
        return;
    }

    data_buf_t = user_data_ptr->user_valid_data_t;

    if (user_data_ptr->user_data_head_t.length > 0)
    {
        mo_sms_ptr->user_head_is_exist = TRUE;
    }
    else
    {
        mo_sms_ptr->user_head_is_exist = FALSE;
    }

    DVMTraceDbg("[INFO][MESSAGE]: MMISMS_EncodeMoSmsData user_head_is_exist = %d", mo_sms_ptr->user_head_is_exist);
    MNSMS_EncodeUserDataEx(
        dual_sys,
        mo_sms_ptr->user_head_is_exist,
        alphabet_type,
        &user_data_ptr->user_data_head_t,
        &user_data_ptr->user_valid_data_t,
        &(mo_sms_ptr->user_data_t)
        );
}

LOCAL BOOLEAN Java_send_sms_to_mn(
                      uint8     *sms_data_ptr,
                      int32      sms_data_len,
                      uint8      *dest_addr_ptr,
                      uint8      dest_addr_len,
                      int32      srcPort,
                      int32      dstPort,
                      int32      type)
{
    MN_SMS_USER_DATA_HEAD_T tpdu_head;
    JAVA_SMS_USER_DATA_T user_data = {0};
    MN_CALLED_NUMBER_T dest_num = {MN_NUM_TYPE_UNKNOW, MN_NUM_PLAN_UNKNOW, 0, 0};
    ERR_MNSMS_CODE_E mn_err_code = ERR_MNSMS_NONE;
    MN_SMS_MO_SMS_T mo_sms = {0};
    SCI_TIME_T time = {0};
    SCI_DATE_T date = {0};
    MN_SMS_PATH_E send_path = MN_SMS_GSM_PATH;
    MN_DUAL_SYS_E dual_sys= MN_DUAL_SYS_1;
    MN_SMS_STORAGE_E  storage = MN_SMS_NO_STORED;

    if (sms_data_ptr == NULL || sms_data_len == 0 || dest_addr_ptr == NULL)
    {
        DVMTraceDbg("[INFO][MESSAGE]:Failed Java_send_sms_to_mn parameter is not right!");
        return FALSE;
    }

    if (dstPort > 0)
    {
        InitUserDataHeadForJAVA(srcPort, dstPort, &tpdu_head, FALSE);
    }
    else
    {
        SCI_MEMSET(&tpdu_head, 0, sizeof(MN_SMS_USER_DATA_HEAD_T) );
    }

    JAVASMS_GetDestNumber(dest_addr_ptr, dest_addr_len, &dest_num);
    JAVASMS_GetUserData(sms_data_ptr, sms_data_len, type, &user_data);

    // the destination address
    mo_sms.dest_addr_t = dest_num;
    mo_sms.dest_addr_t.number_plan = MN_NUM_PLAN_ISDN_TELE;
    mo_sms.dest_addr_present = TRUE;
    // set the param of mo_sms
    mo_sms.dcs.alphabet_type = type;
    mo_sms.dcs.class_is_present = FALSE;
    mo_sms.time_format_e = MN_SMS_TP_VPF_RELATIVE_FORMAT;
    mo_sms.time_valid_period.time_second = 50000000;
    mo_sms.status_report_is_request = FALSE;
    mo_sms.pid_present = TRUE;
    mo_sms.pid_e = MN_SMS_PID_DEFAULT_TYPE;
    TM_GetSysTime(&time);
    TM_GetSysDate(&date);
    mo_sms.time_stamp_t.day = date.mday;
    mo_sms.time_stamp_t.hour = time.hour;
    mo_sms.time_stamp_t.minute = time.min;
    mo_sms.time_stamp_t.month = date.mon;
    mo_sms.time_stamp_t.second = time.sec;
    mo_sms.time_stamp_t.timezone = 0;    //待完善
    mo_sms.time_stamp_t.year = date.year - JAVA_DATE_MIN_YEAR;    //@cr116674

    // user data
    JAVASMS_EncodeMoSmsData(
        dual_sys,
        &mo_sms,
        type,
        &user_data
        );

    DVMTraceDbg("[INFO][MESSAGE]: SendMsgReqToMN send_path=%d", send_path);

    // call the MN API to send request
    mn_err_code = MNSMS_AppSendSmsEx(
        dual_sys,
        &mo_sms,
        storage,
        send_path,
        FALSE    // for cr66039(34.2.9.1_1900 fta fail)
        );

    DVMTraceDbg("[INFO][MESSAGE]: Java_send_sms_to_mn mn_err_code = %d", mn_err_code);

    return (mn_err_code == ERR_MNSMS_NONE);
}

#endif

/**
 * Class:     jp_co_cmcc_message_sms_MyMessageSender
 * Method:    nSend
 * Signature: (Ljava/lang/String;III[BI)Z
 */
void Java_jp_co_cmcc_message_sms_MyMessageSender_nSend(const u4* args, JValue* pResult) {
    ClassObject* thisObj = (ClassObject*) args[0];
    StringObject * addressObj = (StringObject *) args[1];
    const jchar* address = dvmGetStringData(addressObj);
    int addressLen = dvmGetStringLength(addressObj);
    jint srcPort = (jint) args[2];
    jint dstPort = (jint) args[3];
    jint type = (jint) args[4];
    ArrayObject * dataBAArr = (ArrayObject *)args[5];
    jbyte * dataBAArrPtr = (jbyte *)(KNI_GET_ARRAY_BUF(args[5]));
    int dataBAArrLen = KNI_GET_ARRAY_LEN(args[5]);
    jint dataLen = (jint) args[6];
    jboolean ret = FALSE;

#if defined(ARCH_ARM_SPD)
    {        
        uint8_t  dstAddr[PHONE_NUM_MAX_SIZE] = {0x0,};
        uint8_t  msgData[MAX_CONTENT_LENGTH] = {0x0,};
        int32_t  tgtType = 0;

        if (dataLen < MAX_CONTENT_LENGTH)
        {
            CRTL_memcpy(msgData, dataBAArrPtr, dataLen);
        }
        else
        {
            CRTL_memcpy(msgData, dataBAArrPtr, (MAX_CONTENT_LENGTH - 1));
        }
        
        DVMTraceDbg("[INFO][MESSAGE] MySmsConnectionThread nSend dstAddr = %s, srcPort =%d, desport=%d, encoding=%d, msgData=%s.",
                        dstAddr, srcPort, dstPort, type, msgData);
        switch(type)
        {
            case JAVA_ENC_8BIT_BIN:
                tgtType = MN_SMS_8_BIT_ALPHBET;
                break;
        
            case JAVA_ENC_UCS_2:
                tgtType = MN_SMS_UCS2_ALPHABET;
                break;
        
            case JAVA_ENC_GSM_7BIT:
            default:
                tgtType = MN_SMS_DEFAULT_ALPHABET;
                break;
        }
        
        ret = Java_send_sms_to_mn((uint8_t *)dataBAArrPtr, dataLen, dstAddr,
                                      CRTL_strlen((char*)dstAddr), srcPort, dstPort, tgtType);
        DVMTraceDbg("[INFO][MESSAGE] nSend - INFO: send %s!", ret ? "success" : "fail");
    }
#endif

    RETURN_BOOLEAN(ret);
}

