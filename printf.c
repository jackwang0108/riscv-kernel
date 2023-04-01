#include "os.h"

/**
 * @brief _vsnprintf will replace format sign in s with value in vl and output to out
 * 
 * @param out buffer for replaced string, output string will be discard if set to NULL
 * @param n size of out
 * @param s format string
 * @param vl value list
 * @return int length of out
 */
static int _vsnprintf(char *out, size_t n, const char *s, va_list vl){
    int format = 0;
    // longarg is used for mark with 'l', e.g., 'ld', 
    int longarg = 0;
    size_t pos = 0;
    // loop to process string with format marks
    for (; *s; s++){
        // format will be set to 1 if last char is '%'
        if (format){
            switch (*s){
                case 'l': {
                    longarg = 1;
                    break;
                }
                case 'p': {
                    longarg = 1;
                    if (out && pos < n)
                        out[pos] = '0';
                    pos++;
                    if (out && pos < n)
                        out[pos] = 'x';
                    pos++;
                }
                case 'x': {
                    long num = longarg ? va_arg(vl, long) : va_arg(vl, int);
                    int hexdigits = 2 * (longarg ? sizeof(long) : sizeof(int)) - 1;
                    for (int i = hexdigits; i>= 0; i--) {
                        // get each digits
                        int d = (num >> (4 * i)) & 0x0F;
                        //  convert to ascii
                        if (out && pos < n)
                            out[pos] = (d < 10 ? '0' + d : 'a' + d - 10);
                        pos++;
                    }
                    longarg = 0;
                    format = 0;
                    break;
                }
                case 'd': {
                    // 
                    long num = longarg ? va_arg(vl, long) : va_arg(vl, int);
                    // process negative integer, print a minus sign and absolute value
                    if (num < 0){
                        num = -num;
                        if (out && pos < n)
                            out[pos] = '-';
                        pos++;
                    }
                    // print each digits, start from highest digit
                    long digits = 1;
                    for (long nn = num; nn /= 10; digits++);
                    for (int i = digits - 1; i >= 0; i--){
                        if (out && pos + i < n)
                            out[pos + 1] = '0' + (num % 10);
                        num /= 10;
                    }
                    pos += digits;
                    longarg = 0;
                    format = 0;
                    break;
                }
                case 's': {
                    const char *s2 = va_arg(vl, const char*);
                    while (*s2){
                        if (out && pos < n)
                            out[pos] = *s2;
                        pos++;
                        s2++;
                    }
                    longarg = 0;
                    format = 0;
                    break;
                }
                case 'c': {
                    if (out && pos < n)
                        out[pos] = (char) va_arg(vl, int);
                    pos++;
                    longarg = 0;
                    format = 0;
                    break;
                }
                default:
                    break;
            }
        } else if (*s == '%') {
            format = 1;
        } else {
            // just copy if is not format string
            if (out && pos < n)
                out[pos] = *s;
            pos++;
        }
    }

    if (out && pos < n)
        out[pos] = 0;
    else if (out && n)
        out[n - 1] = 0;

    return pos;
}


// buffer for _vprintf()
static char out_buf[1000];

/**
 * @brief _vprintf replace format sign in s with values in vl
 * 
 * @param s format string
 * @param vl value list
 * @return int length of s
 */
static int _vprintf(const char *s, va_list vl){
    // check length
    int res = _vsnprintf(NULL, -1, s, vl);
    if (res + 1 >= sizeof(out_buf)){
        uart_puts("error: output string size overflow\n");
        while (1);
    }
    _vsnprintf(out_buf, res + 1, s, vl);
    uart_puts(out_buf);
    return res;
}

/**
 * @brief printf prints formatted string
 * 
 * @param s format string
 * @param ... value list
 * @return int length of string
 */
int printf(const char*s, ...){
    int res = 0;
    va_list vl;
    va_start(vl, s);
    res = _vprintf(s, vl);
    va_end(vl);
    return res;
}

/**
 * @brief prints panic message and hang the kernel
 * 
 * @param s panic message
 */
void panic(char *s){
    printf("panic: ");
    printf(s);
    printf("\n");
    while (1);
}