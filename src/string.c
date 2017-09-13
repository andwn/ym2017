#include "common.h"

#include "string.h"

// FORWARD
static uint32_t uintToStr_(uint32_t value, char *str, int16_t minsize, int16_t maxsize);

uint32_t strlen(const char *str)
{
    const char *src;

    src = str;
    while (*src++);

    return (src - str) - 1;
}

uint16_t strnlen(const char *str, uint16_t maxlen)
{
    const char *src;

    for (src = str; maxlen-- && *src != '\0'; ++src)
        /* nothing */;

    return src - str;
}

int16_t strcmp(const char *str1, const char *str2)
{
    const uint8_t *p1 = (const uint8_t*) str1;
    const uint8_t *p2 = (const uint8_t*) str2;
    uint8_t c1, c2;

    do
    {
        c1 = *p1++;
        c2 = *p2++;
    }
    while (c1 && (c1 == c2));

    return c1 - c2;
}

char* strclr(char *str)
{
    str[0] = 0;

    return str;
}

char* strcpy(char *to, const char *from)
{
    const char *src;
    char *dst;

    src = from;
    dst = to;
    while ((*dst++ = *src++));

    return to;
}

char* strncpy(char *to, const char *from, uint16_t len)
{
    const char *src;
    char *dst;
    uint16_t i;

    src = from;
    dst = to;
    i = 0;
    while ((i++ < len) && (*dst++ = *src++));

    // end string by null character
    if (i > len) *dst = 0;

    return to;
}

char* strcat(char *to, const char *from)
{
    const char *src;
    char *dst;

    src = from;
    dst = to;
    while (*dst++);

    --dst;
    while ((*dst++ = *src++));

    return to;
}

char *strreplacechar(char *str, char oldc, char newc)
{
    char *s;

    s =  str;
    while(*s)
    {
        if (*s == oldc)
            *s = newc;
        s++;
    }

    return s;
}

void intToStr(int32_t value, char *str, int16_t minsize)
{
    uint32_t v;
    char *dst = str;

    if (value < 0)
    {
        v = -value;
        *dst++ = '-';
    }
    else v = value;

    uintToStr_(v, dst, minsize, 16);
}

void uintToStr(uint32_t value, char *str, int16_t minsize)
{
    uintToStr_(value, str, minsize, 16);
}

static uint32_t uintToStr_(uint32_t value, char *str, int16_t minsize, int16_t maxsize)
{
    uint32_t res;
    int16_t cnt;
    int16_t left;
    char data[16];
    char *src;
    char *dst;

    src = &data[16];
    res = value;
    left = minsize;

    cnt = 0;
    while (res)
    {
        *--src = '0' + (res % 10);
        cnt++;
        left--;
        res /= 10;
    }
    while (left > 0)
    {
        *--src = '0';
        cnt++;
        left--;
    }

    if (cnt > maxsize) cnt = maxsize;

    dst = str;
    while(cnt--) *dst++ = *src++;
    *dst = 0;

    return strlen(str);
}

void intToHex(uint32_t value, char *str, int16_t minsize)
{
    uint32_t res;
    int16_t cnt;
    int16_t left;
    char data[16];
    char *src;
    char *dst;
    const int16_t maxsize = 16;

    src = &data[16];
    res = value;
    left = minsize;

    cnt = 0;
    while (res)
    {
        uint8_t c;

        c = res & 0xF;

        if (c >= 10) c += ('A' - 10);
        else c += '0';

        *--src = c;
        cnt++;
        left--;
        res >>= 4;
    }
    while (left > 0)
    {
        *--src = '0';
        cnt++;
        left--;
    }

    if (cnt > maxsize) cnt = maxsize;

    dst = str;
    while(cnt--) *dst++ = *src++;
    *dst = 0;
}
