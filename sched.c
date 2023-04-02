#include "os.h"

// defined in entry.S
extern void switch_to(context_t *next);

#define STACK_SIZE 1024

uint8_t task_stack[STACK_SIZE];
context_t ctx_task;

/**
 * @brief w_mscratch write mscratch register to x
 * 
 * @param x value set to mscratch register
 */
static void w_mscratch(reg_t x){
    asm volatile (
        "csrw mscratch, %0"
        :
        : "r"(x)
    );
}

void user_task0(void);
void sched_init() {
    w_mscratch(0);
    ctx_task.sp = (reg_t) &task_stack[STACK_SIZE];
    ctx_task.ra = (reg_t) user_task0;
}

void schedule() {
    context_t *next = &ctx_task;
    switch_to(next);
}

void task_delay(volatile int count) {
    count *= 50000;
    while (count--);
}

void user_task0(void){
    uart_puts("Task 0: Created!\n");
    while (1) {
        uart_puts("Task 0: Running...\n");
        task_delay(1000);
    }
}