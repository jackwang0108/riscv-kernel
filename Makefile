# riscv-gcc cross-compiler options
#		1. -notstdlib: do not link libraries in LD_LIBRARY_PATH, only link given libraries
#		2. -fno_builtin: do not recognize library functions 
#		3. -march: which instuction set to use when generating assembly codes
#		4. -mabi: how to pass function arguments when calling function between C and assembly
#		5. -g: generates debug infomation and writing them into compiled binary program when compiling
#		6. -Wall: print all warning messages when compiling
CFLAGS = -nostdlib -fno-builtin -march=rv32ima -mabi=ilp32 -g -Wall

# QEMU options
#		1. -nographic: do not display a screen
#		2. -smp: set CPU number
#		3. -machine: set devices of the emulation system. QEMU has two mode of emulation: USER and SYSTEM.
#					 User mode emulation: QEMU can launch Linux processes compiled for one CPU on another CPU, translating syscalls on the fly.
#				     Full system emulation: QEMU emulates a full system (virtual machine), including a processor and various peripherals such as disk, ethernet controller etc.
#					 When running system emulation, you can decide what devices the system has.
#					 For example, when you run qemu-system-aarch64, you can set -machine option as raspi3b, which means the devices on the virtual machine should be the same as Raspberry Pi 3B.
#					 Here we simply use a default virtual machine setups.
#		4. -bios: set your bios program. None means using QEMU bios.
QFLAGS = -nographic -smp 1 -machine virt -bios none
# QFLAGS = -smp 1 -machine virt -bios none

# QEMU
QEMU = qemu-system-riscv32

# GDB
GDB = gdb-multiarch

# cross-compiler prefix
CROSS_COMPILE = riscv64-unknown-elf-

# GCC
CC = ${CROSS_COMPILE}gcc

# OBJCOPY
OBJCOPY = ${CROSS_COMPILE}objcopy

# OBJDUMP
OBJDUMP = ${CROSS_COMPILE}objdump

# source files
SRCS_ASM = \
	start.S \

SRCS_C = \
	kernel.c \
	uart.c \

MKP := $(abspath $(lastword $(MAKEFILE_LIST)))  #获取当前正在执行的makefile的绝对路径
# DIR :=  $(patsubst$(%/, %, dir $(MKP)))
DIR=$(shell dirname ${MKP})/build/

# set objects
OBJS = $(SRCS_ASM:.S=.o)	# change all files end with .S in SRCS_ASM into .o and assign them to OBJS
OBJS += $(SRCS_C:.c=.o)		# change all files end with .S in SRCS_ASM into .o and assign them to OBJS
OBJS := $(addprefix ${DIR}, ${OBJS})


# default target
# all will:
#		1. create build 
.DEFAULT_GOAL := all
all: mkdir os.elf 

# full 
full : all hex code 

.PHONY : mkdir
mkdir:
	@mkdir -p ${DIR}

# compile all .c file into .o
${DIR}%.o: %.c
	${CC} ${CFLAGS} -c -o $@ $<

# compile all .S file into .o
${DIR}%.o: %.s
	${CC} ${CFLAGS} -c -o $@ $<

# target os.elf depends on OBJS and will:
#		1. link all objective files into os.elf whose start addr is 0x8000_0000
#		2. convert os.elf into os.bin, this will remove useless segment in os.elf and prepare for objdump to disassemble
os.elf: ${OBJS}
	@${CC} ${CFLAGS} ${SRC} -Ttext=0x80000000 -o ${DIR}os.elf $^
	@${OBJCOPY} -O binary ${DIR}os.elf ${DIR}os.bin


# kill phony target kills all running QEMU processes
.PHONY : kill
kill:
	@echo "Kill all QEMU process..."
	ps aux | grep -e 'qemu' | grep -v 'grep' | awk '{print $$2}' | xargs --no-run-if-empty kill -9

# run phony target depends on target all, which runs kernel directly
.PHONY : run
run: all
	@echo "Press Ctrl-A and then X to exit QEMU"
	@echo "------------------------------------"
	@echo "No output, please run 'make debug' to see details"
	@${QEMU} ${QFLAGS} -kernel ${DIR}os.elf

# debug phony target depends on target all, which debugs the kenel
#		1. qemu -kernel: kernel (*.elf) to debug
#		2. qemu -s: debug kernel with QEMU builtin GDB server, port by default is 1234
#		3. qemu -S: freeze CPU at start, until GDB client connects and starts debugging
#		4. qemu &: run qemu in background since GDB will running in foreground
#		5. GDB -q: slience some output when starting
#		6. GDB -x: GDB debugging commands running right after GDB starts, usually connects to GDB server, set breakpoint at _start, etc.
.PHONY : debug-gdb
debug-gdb: all
	@echo "Press Ctrl-C and then input 'quit' to exit GDB and QEMU"
	@echo "-------------------------------------------------------"
	@${QEMU} ${QFLAGS} -kernel ${DIR}os.elf -s -S &
	@${GDB} ${DIR}os.elf -q -x ./gdbinit

.PHONY : debug-vscode
debug-vscode:
	@echo "QEMU will automatically exit once you stop VSCode debugging"
	@echo "-----------------------------------------------------------"
	@${QEMU} ${QFLAGS} -kernel ${DIR}os.elf -gdb 1234 -S


# code phony target depends on target all, which disasseble the kernel
.PHONY : code
code: all
	@echo "Disassmbly code can be found in" ${DIR}os.code
	@${OBJDUMP} -S ${DIR}os.elf > ${DIR}os.code

# hex phony target depends on target all, which prints binary content of the kernel
.PHONY : hex
hex: all
	@echo "Machine code can be found in" ${DIR}os.machine
	@hexdump -C ${DIR}os.bin > ${DIR}os.machine

# clean phony target clean all build files
.PHONY : clean
clean:
	rm -rf ${DIR}/*

