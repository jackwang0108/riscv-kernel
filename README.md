# riscv-kernel
A tiny risc-v 32 kernel, for learning risc-v.


## Usage

### 1. Compile

Run
```shell
make all/full
```

Target `all` compiles the kernel, generating `os.elf` and `os.bin`.

Target `full` compiles and disassembles the kernel, generating `os.elf`, `os.bin`, `os.code` and `os.machine`.


### 2. Debug

Run
```shell
make debug-gdb
```
will run qemu as background task and gdb-multiarch as foreground task. You will enter gdb debugging console directly.

Run
```shell
make debug-vscode
```
will run qemu as foreground task and export gdbserver port to 1234. You can run vscode as gdb front-end after connecting to gdbserver.

> If you run vscode and qemu (`make debug-vscode`) on same machine, change ip address of `miDebuggerServerAddress` in .vscode/launch.json to `localhost`.
>
> If you run vscode and qemu (`make debug-vscode`) on different machine, change ip address of `miDebuggerServerAddress` in .vscode/launch.json to ip address of host running qemu.