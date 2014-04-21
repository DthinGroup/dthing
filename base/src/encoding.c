/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2013/07/18 $
 * Last modified:	$Date: 2014/04/18 $
 * Version:         $ID: encoding.c#1
 */


#include <std_global.h>
#include <encoding.h>



/*=========================================================================*/
/*===                    Character conversions                          ===*/
/*=========================================================================*/

/* See encoding.h */
int32_t convertUcs2ToAscii(const uint16_t* src, int32_t srcChars, char* dst, int32_t dstBytes)
{
    if (dstBytes == 0 || dst == NULL)
    {
        //no output buffer.
        return -1;
    }

    dstBytes--; /* Now its the number of bytes excluding the terminator */
    if (dstBytes >= 0)
    {
        char*  dstEnd = dst + ((dstBytes < srcChars) ? dstBytes : srcChars);

        while (dst < dstEnd)
        {
            uint16_t ch = *src++;
            if (ch == 0 || ch > 255)
                ch = '?';
            *dst++ = (char)ch;
        }
        *dst = '\0';

        if (dstBytes < srcChars)
            return -1;      /* Truncation occurred */
    }
    return srcChars + 1;    /* Success, indicate number of bytes required */
}

/* See encoding.h */
int32_t convertAsciiToUcs2(const char* src, int32_t srcBytes, uint16_t* dst, int32_t dstBytes)
{
    const char* srcEnd;
    uint16_t*   dstStart;
    uint16_t*   dstEnd;

    if (dstBytes == 0 || dst == NULL)
    {
        //no output buffer.
        return -1;
    }
    dstStart = dst;
    dstEnd   = (uint16_t*)(dstBytes + (char*)dst);

    /* Use src != srcEnd to allow srcBytes=-1 to mean convert upto first '\0' */
    for (srcEnd = src + srcBytes; src != srcEnd && *src != '\0'; src++, dst++)
    {
        if (dst < dstEnd)
        {
            *dst = *(uint8_t*)src;
        }
        else if (dstStart != NULL)
        {
            return -1;      /* Not enough space in destination buffer */
        }
    }

    return (int32_t)(dst - dstStart);  /* Return number of _characters_ used, not bytes */
}

/* See encoding.h */
int32_t convertUcs2ToUtf8(const uint16_t* src, int32_t srcChars, char* dst, int32_t dstBytes)
{
    const uint16_t* srcEnd;
    char*           dstStart = dst;
    char*           dstEnd   = dst + dstBytes;

    if (dstBytes == 0 || dst == NULL)
    {
        //no output buffer.
        return -1;
    }

    for (srcEnd = src + srcChars; src < srcEnd; src++)
    {
        uint16_t ch = *src;
        if (ch > 0 && ch <= 0x7f)  /* COMMON CASE */
        {
            if ((dst + 1) < dstEnd)
            {
                dst[0] = (char)ch;
            }
            else if (dstStart != NULL)
            {
                break;
            }
            dst = dst + 1;
        }
        else if (ch <= 0x7ff)  /* includes case ch == 0 */
        {
            /* Two-byte encoding. */
            if ((dst + 2) < dstEnd)
            {
                dst[0] = (char)(0xc0 | (ch >> 6));
                dst[1] = (char)(0x80 | (ch & 0x3f));
            }
            else if (dstStart != NULL)
            {
                break;
            }
            dst = dst + 2;
        }
        else /* Three-byte encoding. */
        {
            if ((dst + 3) < dstEnd)
            {
                dst[0] = (char)(0xe0 | (ch >> 12));
                dst[1] = (char)(0x80 | ((ch >> 6) & 0x3f));
                dst[2] = (char)(0x80 | (ch & 0x3f));
            }
            else if (dstStart != NULL)
            {
                break;
            }
            dst = dst + 3;
        }
    }

    if (dstStart != NULL)
        *dst = 0;               /* Terminate the (possibly truncated) string */

    if (src < srcEnd)
        return -1;              /* Truncation occurred */

    return (int32_t)(dst + 1 - dstStart);  /* Success: return number of bytes required */
}

static const char *convert_single_utf_char(const char *str, uint16_t* chsite)
{
    uint32_t    ch, ch2, ch3;

    if ((ch = *str) < 0x80)     /* Single byte encoding  - COMMON CASE */
    {
        if (ch != 0)            /* non-terminating chars - COMMON CASE */
        {
            if (chsite != NULL)
            {
                *chsite = (uint16_t)ch;
            }
            return str+1;
        }
    }
    else if ((ch & 0xE0) == 0xC0)               /* Two byte encoding */
    {
        if (((ch2 = *(str+1)) & 0xC0) == 0x80)  /* Valid two byte encoding */
        {
            if (chsite != NULL)
            {
                *chsite = (uint16_t)(((ch & 0x1F)<<6)|(ch2 & 0x3F));
            }
            return str+2;
        }
    }
    else if ((ch & 0xF0) == 0xE0)               /* Three byte encoding */
    {
        if (((ch2 = *(str+1)) & 0xC0) == 0x80 &&
            ((ch3 = *(str+2)) & 0xC0) == 0x80)  /* Valid two byte encoding */
        {
            if (chsite != NULL)
            {
                *chsite = (uint16_t)(((ch & 0xF)<<12)
                                        | ((ch2 & 0x3F)<<6)
                                        | (ch3 & 0x3F));
            }
            return str+3;
        }
    }

    /*
     * Otherwise, we have an invalid encoding.
     */
    return NULL;
}

/* See encoding.h */
int32_t convertUtf8ToUcs2(const char* src, int32_t srcBytes, uint16_t* dst, int32_t dstBytes)
{
    const char * srcPos = src;
    const char * srcEnd;
    int32_t      count = 0;
    int32_t      retVal;

    if (srcBytes == -1)
        srcBytes = (int32_t)CRTL_strlen(src);
    /* First of all, work out how many unicode characters there will be */
    for (srcEnd = srcPos + srcBytes; srcPos < srcEnd; count++)
    {
        srcPos = convert_single_utf_char(srcPos, NULL);
        if (srcPos == NULL || srcPos > srcEnd)
            break;
    }

    retVal = count;

    /* If the caller was just interested in the length, return it. */
    if (dst != NULL)
    {
        /* Limit the number of characters to the dst buffer space available */
        if ((count * 2) > dstBytes)
        {
            count = dstBytes >> 1;
            retVal = -1;    /* Truncation occurred */
        }

        /* Now we have exactly the right number of (valid) destination
         * characters so extract them.
         */
        for (srcPos = src; count > 0; count--, dst++)
        {
            srcPos = convert_single_utf_char(srcPos, dst);
        }
    }

    return retVal;
}




