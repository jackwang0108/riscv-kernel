#ifndef __OS_H__
#define __OS_H__

#include "types.h"
#include "platform.h"

#include <stddef.h>
#include <stdarg.h>

// uart.c
extern int uart_putc(char ch);
extern void uart_puts(char *s);

// printf.c
extern int printf(const char *s, ...);
extern void panic(char *s);

// page.c
extern void *page_alloc(int pages);
extern void page_free(void *p);


#endif