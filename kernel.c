extern void uart_init(void);
extern void uart_puts(char *s);

void start_kernel(void){

    // init uart
    uart_init();
    uart_puts("Hello JackOS-riscv!\n");

    while (1);
}