
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
  return ((c >= (wint_t)'0' && c <= (wint_t)'9') ||
      (c >= (wint_t)'a' && c <= (wint_t)'f') ||
      (c >= (wint_t)'A' && c <= (wint_t)'F'));
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
