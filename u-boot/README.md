# u-boot build

## source code url
https://github.com/Xilinx/u-boot-xlnx

## How to build
1. copy a configuration file and a device tree file.
```
$ cp uartonly/rk-zynq_uartonly_defconfig <u-boot-xlnx-path>/configs/
$ cp uartonly/rk-zynq-uartonly.dts <u-boot-xlnx-path>/arch/arm/dts/
```
2. load configuration
```
$ cd  <u-boot-xlnx-path>
$ source <xilinx-tool>/Vitis/settings64.sh
$ ARCH=arm CROSS_COMPILE=arm-none-eabi- make rk-zynq_uartonly_defconfig
$ ARCH=arm CROSS_COMPILE=arm-none-eabi- make
```


## load u-boot by openocd
1. call openocd
`openocd -f ft2232-cha-jtag.cfg -f /usr/share/openocd/scripts/target/zynq_7000.cfg `
2. open port 4444 by telnet on another terminal.
`telnet localhost 4444`
3. execute these commands
```
set FSBL_ELF fsbl.elf
set UBOOT_ELF u-boot.elf
set BITSTREAM uart_only.bit
set UBOOT_ENTRY 0x04000000 
source load_uboot.tcl
```



### Example run
```
$ openocd -f ft2232-cha-jtag.cfg -f /usr/share/openocd/scripts/target/zynq_7000.cfg 
Open On-Chip Debugger 0.12.0
Licensed under GNU GPL v2
For bug reports, read
	http://openocd.org/doc/doxygen/bugs.html
none separate

zynqpl_program
Info : Listening on port 6666 for tcl connections
Info : Listening on port 4444 for telnet connections
Info : clock speed 1000 kHz
Info : JTAG tap: zynq_pl.bs tap/device found: 0x23727093 (mfg: 0x049 (Xilinx), part: 0x3727, ver: 0x2)
Info : JTAG tap: zynq.cpu tap/device found: 0x4ba00477 (mfg: 0x23b (ARM Ltd), part: 0xba00, ver: 0x4)
Info : zynq.cpu0: hardware has 6 breakpoints, 4 watchpoints
Info : zynq.cpu1: hardware has 6 breakpoints, 4 watchpoints
Info : starting gdb server for zynq.cpu0 on 3333
Info : Listening on port 3333 for gdb connections
Info : accepting 'telnet' connection on tcp/4444
Info : zynq.cpu0: MPIDR level2 0, cluster 0, core 0, multi core, no SMT
Info : zynq.cpu1: MPIDR level2 0, cluster 0, core 1, multi core, no SMT
target halted in ARM state due to debug-request, current mode: Supervisor
cpsr: 0x000001d3 pc: 0xffffff34
MMU: disabled, D-Cache: disabled, I-Cache: disabled
target halted in ARM state due to debug-request, current mode: System
cpsr: 0x200001df pc: 0xffffff28
MMU: disabled, D-Cache: disabled, I-Cache: disabled
target halted in ARM state due to debug-request, current mode: Supervisor
cpsr: 0x60000193 pc: 0xffffff34
MMU: disabled, D-Cache: disabled, I-Cache: disabled
target halted in ARM state due to debug-request, current mode: System
cpsr: 0x6000001f pc: 0x00007864
MMU: disabled, D-Cache: disabled, I-Cache: disabled
Bitstream: uart_only.bit
PL configured (pld load 0).
```
```
$ telnet localhost 4444
Trying 127.0.0.1...
Connected to localhost.
Escape character is '^]'.
Open On-Chip Debugger
> set FSBL_ELF fsbl.elf
fsbl.elf
> set UBOOT_ELF u-boot.elf
u-boot.elf
> set BITSTREAM uart_only.bit
uart_only.bit
> set UBOOT_ENTRY 0x04000000 
0x04000000
> source load_uboot.tcl
zynq.cpu0: MPIDR level2 0, cluster 0, core 0, multi core, no SMT
zynq.cpu1: MPIDR level2 0, cluster 0, core 1, multi core, no SMT
target halted in ARM state due to debug-request, current mode: Supervisor
cpsr: 0x000001d3 pc: 0xffffff34
MMU: disabled, D-Cache: disabled, I-Cache: disabled
target halted in ARM state due to debug-request, current mode: System
cpsr: 0x200001df pc: 0xffffff28
MMU: disabled, D-Cache: disabled, I-Cache: disabled
target halted in ARM state due to debug-request, current mode: Supervisor
cpsr: 0x60000193 pc: 0xffffff34
MMU: disabled, D-Cache: disabled, I-Cache: disabled
target halted in ARM state due to debug-request, current mode: System
cpsr: 0x6000001f pc: 0x00007864
MMU: disabled, D-Cache: disabled, I-Cache: disabled
Bitstream: uart_only.bit
PL configured (pld load 0).
> targets
    TargetName         Type       Endian TapName            State       
--  ------------------ ---------- ------ ------------------ ------------
 0* zynq.cpu0          cortex_a   little zynq.cpu           running
 1  zynq.cpu1          cortex_a   little zynq.cpu           running
```

## How to build (vinila)
1. `source <tool>/<version>/Vitis/settings64.sh`
1. `ARCH=arm CROSS_COMPILE=arm-none-eabi- make xilinx_zynq_virt_defconfig`
1. moditfy configuration `ARCH=arm CROSS_COMPILE=arm-none-eabi- make menuconfig`

## References
[2022年版U-bootの作り方](https://www.trenz.jp/tutorial/u-boot2022.html)
[ユース ケース 3 - Git ソースからの U-boot の生成と運用](https://docs.amd.com/r/2024.2-%E6%97%A5%E6%9C%AC%E8%AA%9E/Vitis-Tutorials-Embedded-Software/%E3%83%A6%E3%83%BC%E3%82%B9-%E3%82%B1%E3%83%BC%E3%82%B9-3-Git-%E3%82%BD%E3%83%BC%E3%82%B9%E3%81%8B%E3%82%89%E3%81%AE-U-boot-%E3%81%AE%E7%94%9F%E6%88%90%E3%81%A8%E9%81%8B%E7%94%A8)