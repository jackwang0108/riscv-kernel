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


/**
 * @brief context struct used in task management
 *          
 *  corresponds to ABI names defined in RISC-V Assembly Programmer’s Handbook
 *  in RISC-V Specification: Volume I, Unprivileged Instructions
 */
typedef struct __context_t {
    // ignore x0/zero
    union {
        reg_t _x1;
        reg_t ra;
    };
    union {
        reg_t _x2;
        reg_t sp;
    };
    union {
        reg_t _x3;
        reg_t gp;
    };
    union {
        reg_t _x4;
        reg_t tp;
    };
    union {
        reg_t _x5;
        reg_t t0;
    };
    union {
        reg_t _x6;
        reg_t t1;
    };
    union {
        reg_t _x7;
        reg_t t2;
    };
    union {
        reg_t _x8;
        reg_t s0;
        reg_t fp;
    };
    union {
        reg_t _x9;
        reg_t s1;
    };
    union {
        reg_t _x10;
        reg_t a0;
        reg_t ret0;
    };
    union {
        reg_t _x11;
        reg_t a1;
        reg_t ret1;
    };
    union {
        reg_t _x12;
        reg_t a2;
    };
    union {
        reg_t _x13;
        reg_t a3;
    };
    union {
        reg_t _x14;
        reg_t a4;
    };
    union {
        reg_t _x15;
        reg_t a5;
    };
    union {
        reg_t _x16;
        reg_t a6;
    };
    union {
        reg_t _x17;
        reg_t a7;
    };
    union {
        reg_t _x18;
        reg_t s2;
    };
    union {
        reg_t _x19;
        reg_t s3;
    };
    union {
        reg_t _x20;
        reg_t s4;
    };
    union {
        reg_t _x21;
        reg_t s5;
    };
    union {
        reg_t _x22;
        reg_t s6;
    };
    union {
        reg_t _x23;
        reg_t s7;
    };
    union {
        reg_t _x24;
        reg_t s8;
    };
    union {
        reg_t _x25;
        reg_t s9;
    };
    union {
        reg_t _x26;
        reg_t s10;
    };
    union {
        reg_t _x27;
        reg_t s11;
    };
    union {
        reg_t _x28;
        reg_t t3;
    };
    union {
        reg_t _x29;
        reg_t t4;
    };
    union {
        reg_t _x30;
        reg_t t5;
    };
    union {
        reg_t _x31;
        reg_t t6;
    };
} context_t;

extern int task_create(void (*task)(void));
extern void task_delay(volatile int count);

#endif