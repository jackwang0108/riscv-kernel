#include "types.h"
#include "platform.h"

/*
 * UART_REG(reg) macro returns the memory address of given registers.
 * In QEMU System, the UART control registers are memory-mapped at address UART0.
 * See: https://github.com/qemu/qemu/blob/master/hw/riscv/virt.c
 */
#define UART_REG(reg) ((volatile uint8_t *)(UART0 + reg))

/*
 * UART control registers map. see [1] "PROGRAMMING TABLE"
 * note some are reused by multiple functions
 * 
 * 0 (write mode): THR/DLL
 * 1 (write mode): IER/DLM
 * 
 * Reference:
 *  [1]: TECHNICAL DATA ON 16550, http://byterunner.com/16550.html
 */
#define RHR 0	// Receive Holding Register (read mode)
#define THR 0	// Transmit Holding Register (write mode)
#define IER 1	// Interrupt Enable Register (write mode)

#define DLL 0	// LSB of Divisor Latch (write mode)
#define DLM 1	// MSB of Divisor Latch (write mode)

#define FCR 2	// FIFO Control Register (write mode)
#define ISR 2	// Interrupt Status Register (read mode)
#define LCR 3	// Line Control Register
#define MCR 4	// Modem Control Register
#define LSR 5	// Line Status Register
#define MSR 6	// Modem Status Register
#define SPR 7	// ScratchPad Register

/*
 * POWER UP DEFAULTS
 * IER = 0: TX/RX holding register interrupts are both disabled
 * ISR = 1: no interrupt penting
 * LCR = 0
 * MCR = 0
 * LSR = 60 HEX
 * MSR = BITS 0-3 = 0, BITS 4-7 = inputs
 * FCR = 0
 * TX = High
 * OP1 = High
 * OP2 = High
 * RTS = High
 * DTR = High
 * RXRDY = High
 * TXRDY = Low
 * INT = Low
 */

/*
 * LINE STATUS REGISTER (LSR)
 * LSR BIT 0:
 * 0 = no data in receive holding register or FIFO.
 * 1 = data has been receive and saved in the receive holding register or FIFO.
 * ......
 * LSR BIT 5:
 * 0 = transmit holding register is full. 16550 will not accept any data for transmission.
 * 1 = transmitter hold register (or FIFO) is empty. CPU can load the next character.
 * ......
 */
#define LSR_RX_READY (1 << 0)
#define LSR_TX_IDLE  (1 << 5)

/**
 * @brief uart_read_reg(reg) macros reads register
 */
#define uart_read_reg(reg) (*(UART_REG(reg)))
/**
 * @brief uart_write_reg(reg) macros writes register
 */
#define uart_write_reg(reg, v) (*(UART_REG(reg)) = (v))

void uart_init(){
	// disable interrupts
	uart_write_reg(IER, 0x00);

    // Setting baud rate.
	/*
	 * Notice that the divisor register DLL (divisor latch least) and DLM (divisor
	 * latch most) register have the same base address as the receiver/transmitter
     * register and the interrupt enable register. 
     * 
     * The bits in LCR (Line Control Register) dicides what registers the base address points to.
	 * The 7-th bit of LCR register is called DLAB bit (Divisor Latch Access Bit). Setting DLAB bit
     * will make the base address points to DLL and DLM register
     * 
	 *
	 * Regarding the baud rate value, see [1] "BAUD RATE GENERATOR PROGRAMMING TABLE".
	 * We use 38.4K when 1.8432 MHZ crystal, so the corresponding value is 3.
	 * And due to the divisor register is two bytes (16 bits), so we need to
	 * split the value of 3(0x0003) into two bytes, DLL stores the low byte,
	 * DLM stores the high byte.
	 */
	uint8_t lcr = uart_read_reg(LCR);
	uart_write_reg(LCR, lcr | (1 << 7));
	uart_write_reg(DLL, 0x03);
	uart_write_reg(DLM, 0x00);

	/*
	 * Continue setting the asynchronous data communication format.
	 * - number of the word length: 8 bits
	 * - number of stop bits：1 bit when word length is 8 bits
	 * - no parity
	 * - no break control
	 * - disabled baud latch
	 */
	lcr = 0;
	uart_write_reg(LCR, lcr | (0b00000011 << 0));
}

/**
 * @brief uart_putc puts a byte to THR register (Transmit Holding Register), i.e., sends a char.
 * 
 * @param ch 
 * @return int 
 */
int uart_putc(char ch){
    // polling until THR register is empty, i.e., the previous byte has been sent.
    // LSR BIT 5:
    //    1. 0 = transmit holding register is full. 16550 will not accept any data for transmission.
    //    2. 1 = transmitter hold register (or FIFO) is empty. CPU can load the next character.
	while ((uart_read_reg(LSR) & LSR_TX_IDLE) == 0);
	return uart_write_reg(THR, ch);
}

/**
 * @brief uart_puts puts a string via uart.
 * 
 * @param s string (null-terminated) to put
 */
void uart_puts(char *s){
    while (*s)
		uart_putc(*s++);
}

