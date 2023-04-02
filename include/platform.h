#ifndef __PLATFORM_H__
#define __PLATFORM_H__

/*
 * QEMU RISC-V Virt machine with 16550a UART and VirtIO MMIO
 */

/* 
 * @brief number of CPUs emulated in qemu-system-riscv32
 * see https://github.com/qemu/qemu/blob/master/include/hw/riscv/virt.h, #define VIRT_CPUS_MAX 8
 */
#define MAXNUM_CPU 8            // actually is hart number for risc-v


/*
 * the cpu emulated by qemu-system-riscv32 accesses devices via memory mapping IO.
 *  
 * Here is the MemoryMap:
 * see https://github.com/qemu/qemu/blob/master/hw/riscv/virt.c, virt_memmap[] for more detailed infomation.
 * 
 * 0x00001000 -- boot ROM, provided by qemu
 * 0x02000000 -- CLINT
 * 0x0C000000 -- PLIC
 * 0x10000000 -- UART0
 * 0x10001000 -- virtio disk
 * 0x80000000 -- boot ROM jumps here in machine mode, where we load our kernel
 */

/**
 * @brief UART resigter mapped address
 */
#define UART0 0x10000000L

#endif
