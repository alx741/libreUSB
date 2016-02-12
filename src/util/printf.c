#include "printf.h"

extern void putchar(char c);

static char *bf;
static char buf[12];
static unsigned int num;
static char uc;
static char zs;


void out(char c)
{
    *bf++ = c;
}


void out_digit(char digit)
{
    out(digit + (digit < 10 ? '0' : (uc ? 'A' : 'a') - 10));
    zs = 1;
}


void out_div(unsigned int div)
{
    unsigned char digit = 0;
    num &= 0xffff;
    while (num >= div)
    {
        num -= div;
        digit++;
    }
    if (zs || digit > 0)
    {
        out_digit(digit);
    }
}


void printf(char *fmt, ...)
{
    va_list va;
    char ch;
    char *p;

    va_start(va, fmt);

    while ((ch = *(fmt++)))
    {
        if (ch != '%')
        {
            putchar(ch);
        }
        else
        {
            char lz = 0;
            char w = 0;
            ch = *(fmt++);
            if (ch == '0')
            {
                ch = *(fmt++);
                lz = 1;
            }
            if (ch >= '0' && ch <= '9')
            {
                w = 0;
                while (ch >= '0' && ch <= '9')
                {
                    w = (((w << 2) + w) << 1) + ch - '0';
                    ch = *fmt++;
                }
            }
            bf = buf;
            p = bf;
            zs = 0;
            switch (ch)
            {
                case 0:
                    va_end(va);
                    return;
                case 'u':
                case 'd' :
                    num = va_arg(va, unsigned int);
                    if (ch == 'd' && (int)num < 0)
                    {
                        num = -(int)num;
                        out('-');
                    }
                    out_div(10000);
                    out_div(1000);
                    out_div(100);
                    out_div(10);
                    out_digit(num);
                    break;
                case 'x':
                case 'X' :
                    uc = ch == 'X';
                    num = va_arg(va, unsigned int);
                    out_div(0x1000);
                    out_div(0x100);
                    out_div(0x10);
                    out_digit(num);
                    break;
                case 'c' :
                    out((char)(va_arg(va, int)));
                    break;
                case 's' :
                    p = va_arg(va, char *);
                    break;
                case '%' :
                    out('%');
                default:
                    break;
            }
            *bf = 0;
            bf = p;
            while (*bf++ && w > 0)
            {
                w--;
            }
            while (w-- > 0)
            {
                putchar(lz ? '0' : ' ');
            }
            while ((ch = *p++))
            {
                putchar(ch);
            }
        }
    }
}
