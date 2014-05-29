
#include <std_global.h>

int32_t CRTL_wcslen(const uint16_t* s)
{
    const uint16_t* p;
    p = s;

    while(*p)
        p++;

    return (int32_t)(p - s);
}


int32_t CRTL_wcscmp(const uint16_t* s1, const uint16_t* s2)
{
    while(*s1 == *s2++)
        if (*s1++ == 0)
            return (0);
    return (*s1 - *--s2);
}

void *CRTL_realloc(void* mem_addr, unsigned int new_size)
{
    /* doesn't support realloc */
    return NULL;
}


int CRTL_isascii(int c)
{
    return (c >= 0) && (c < 128);
}

int CRTL_isxdigit(int c)
{
  return ((c >= (/*wint_t*/int)'0' && c <= (/*wint_t*/int)'9') ||
      (c >= (/*wint_t*/int)'a' && c <= (/*wint_t*/int)'f') ||
      (c >= (/*wint_t*/int)'A' && c <= (/*wint_t*/int)'F'));
}

int32_t CRTL_wcscpy(uint16_t* s1, const uint16_t* s2)
{
    uint16_t *p;            
    uint16_t *q;     

    *s1 = '\0';            
    p = s1;
    q = s2;
    while (*q)
        *p++ = *q++;
    *p = '\0';

    return s1;
}

int32_t CRTL_wcscat(uint16_t* s1, const uint16_t* s2)
{
    uint16_t *p;
    uint16_t *q;
    uint16_t *r;

    p = s1;
    while (*p)
        p++;
    q = p;
    r = s2;
    while (*r)
        *q++ = *r++;
    *q = '\0';

    return s1;
}

uint32_t CRTL_wstrtoutf8(//utf8 len
                             uint8_t *utf8_ptr,//out
                             uint32_t utf8_buf_len,//in
                             const uint16_t *wstr_ptr,//in
                             uint32_t wstr_len//in
                             )
{
    uint16_t    ucs2_char = 0;    //here, we only consider UCS2. UCS4 is not considered.
    uint32_t  i = 0;
    uint32_t    j = 0;

    if (NULL == utf8_ptr || NULL == wstr_ptr)
    {
        return j;
    }

    for ( ; i < wstr_len; i++ )
    {
        //get unicode character

        ucs2_char = wstr_ptr[i];

        if (ucs2_char <= 0x7F)
        {
            //0xxx xxxx
            //0111 1111 == 0x7F
            if ( j >= utf8_buf_len )
            {
                break;
            }

            utf8_ptr[j++] = (uint8_t)(ucs2_char & 0x7F);
        }
        else if (ucs2_char <= 0x7FF)
        {
            if ( j + 1 >= utf8_buf_len )
            {
                break;
            }

            //110x xxxx 10xx xxxx
            //11??2e3“|5?那?6??

            //0000 0111 1100 0000 == 0x7C0    110 == 0x6    //““?3???5???那?(110 << 5)
            utf8_ptr[j++] = (uint8_t)(((ucs2_char & 0x7C0) >> 6) | (0x6 << 5));

            //0000 0000 0011 1111 = 0x3F    10 == 0x2    //““?3?|足“a6???那?(10 << 6)
            utf8_ptr[j++] = (uint8_t)((ucs2_char & 0x3F) | (0x2 << 6));
        }
        else
        {
            if ( j + 2 >= utf8_buf_len )
            {
                break;
            }

            //1110 xxxx 10xx xxxxxx 10xx xxxxxx
            //16??2e3“|4, 6, 6

            //1111 0000 0000 0000 == 0xF000        1110 == 0xE    //““?3???4?? ?那? (1110 << 4)
            utf8_ptr[j++] = (uint8_t)(((ucs2_char & 0xF000) >> 12) | (0xE << 4));

            //0000 1111 1100 0000 == 0xFC0        10 = 0x2    //?D??6?? ?那? (10 << 6)
            utf8_ptr[j++] = (uint8_t)(((ucs2_char & 0xFC0 ) >> 6) | (0x2 << 6));

            //0000 0000 0011 1111 == 0x3F        10 = 0x2    //?芍?o“?6?? ?那? (10 << 6)
            utf8_ptr[j++] = (uint8_t)((ucs2_char & 0x3F) | (0x2 << 6));
        }
    }

    return j;
}
