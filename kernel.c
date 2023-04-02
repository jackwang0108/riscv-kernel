#include "os.h"

extern void uart_init(void);
extern void page_init(void);
extern void sched_init(void);
extern void schedule(void);

void start_kernel(void){

    // init uart
    uart_init();
    uart_puts("Hello JackOS-riscv!\n");

    page_init();
    sched_init();

    schedule();
    
    uart_puts("Would not be here!\n");

    while (1);
}