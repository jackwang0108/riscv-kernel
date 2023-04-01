#include "os.h"

extern void uart_init(void);
extern void page_init(void);

void start_kernel(void){

    // init uart
    uart_init();
    uart_puts("Hello JackOS-riscv!\n");

    page_init();

    while (1);
}