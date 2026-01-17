# U-Boot for minimam_platform
## Memory Map for U-Boot
|start address| binary |
| :- |:- |
|0x0000_0000| fsbl.elf |
|0x0010_0000| device binary tree |
|0x0400_0000| u-boot   |
## How to build u-boot
1. download u-boot-xlnx
   1. `git clone https://github.com/Xilinx/u-boot-xlnx.git <u-boot-xlnx-dir>`
   1. `cd <u-boot-xlnx-dir>`
   1. `git fetch --tags`
   1. `git checkout tags/<stable-tag>`
2. copy configuation file and device tree file
   1. `copy rk-zynq-minimam_defconfig <u-boot-xlnx-dir>/configs/`
   1. `copy -rf dts/* <u-boot-xlnx-dir>/arch/arm/dts/`
   1. `cd <u-boot-xlnx-dir>`
   1. `ARM=arm CROSS_COMPILE=arm-none-eabi- make rk-zynq-minimam_defconfig`
   1. `ARM=arm CROSS_COMPILE=arm-none-eabi- make`
   1. `u-boot.elf` and `u-boot.dtb` are used for building `boot.bin`
### How to build u-boot for debugging
   1. `copy rk-zynq-minimam-debug_defconfig <u-boot-xlnx-dir>/configs/`
   1. `cd <u-boot-xlnx-dir>`
   1. `ARM=arm CROSS_COMPILE=arm-none-eabi- make distclean`
   1. `ARM=arm CROSS_COMPILE=arm-none-eabi- make`
   1. `u-boot` has debug symbol. It is used for debug.
   1. copy or create symbolic link of `ft2232-cha-jtag.cfg`,`uboot_load.gdb`, `<bitstream.bit>` and `fsbl.elf` in <u-boot-xlnx-dir>.
   1. `openocd -f ft2232-cha-jtag.cfg -f /usr/share/openocd/scripts/target/zynq_7000.cfg`
   1. `arm-none-eabi-gdb -x uboot_load.gdb` in another terminal.
   ```
    arm-none-eabi-gdb -x uboot_load.gdb 
	GNU gdb (GDB) 14.2
Copyright (C) 2023 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "--host=x86_64-oesdk-linux --target=arm-xilinx-eabi".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<https://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word".
warning: No executable has been specified and target does not support
determining executable automatically.  Try using the "file" command.
0xffffff28 in ?? ()
Loading section .text, size 0x11d04 lma 0x0
Loading section .handoff, size 0x4c lma 0x11d04
Loading section .note.gnu.build-id, size 0x24 lma 0x11d50
Loading section .init, size 0xc lma 0x11d74
Loading section .fini, size 0xc lma 0x11d80
Loading section .rodata, size 0x669 lma 0x11d8c
--Type <RET> for more, q to quit, c to continue without paging-- 
Loading section .data, size 0x28c8 lma 0x123f8
Loading section .mmu_tbl, size 0x4000 lma 0x18000
Loading section .ARM.exidx, size 0x8 lma 0x1c000
Loading section .init_array, size 0x8 lma 0x1c008
Loading section .fini_array, size 0x4 lma 0x1c010
Loading section .drvcfg_sec, size 0x124 lma 0x1f614
Start address 0x00000000, load size 101877
Transfer rate: 20 KB/sec, 5992 bytes/write.
target halted in ARM state due to debug-request, current mode: Supervisor
cpsr: 0x60000193 pc: 0xffffff34
MMU: disabled, D-Cache: disabled, I-Cache: disabled
target halted in ARM state due to debug-request, current mode: System
cpsr: 0x6000001f pc: 0x00011d24
MMU: disabled, D-Cache: disabled, I-Cache: disabled
loaded file ./minimam_platform.bit to pld device 0 in 32s 483436us
Restoring binary file ./u-boot.dtb into memory (0x100000 to 0x105728)
warning: could not convert 'main' from the host encoding (ANSI_X3.4-1968) to UTF-32.
This normally should not happen, please file a bug report.
Loading section .text, size 0x3a8 lma 0x4000000
Loading section .efi_runtime, size 0x13b0 lma 0x40003a8
Loading section .text_rest, size 0xe2cbc lma 0x4001760
Loading section .rodata, size 0x30d54 lma 0x40e4420
Loading section .hash, size 0x18 lma 0x4115174
Loading section .data, size 0x8ca0 lma 0x4115190
Loading section .got.plt, size 0xc lma 0x411de30
Loading section __u_boot_list, size 0x296c lma 0x411de3c
Loading section .efi_runtime_rel, size 0x130 lma 0x41207a8
Loading section .rel.dyn, size 0x1b500 lma 0x41208d8
Start address 0x04000000, load size 1293768
Transfer rate: 30 KB/sec, 14870 bytes/write.
Breakpoint 1 at 0x4000000: file arch/arm/lib/vectors.S, line 89.
target halted in ARM state due to debug-request, current mode: Supervisor
cpsr: 0x60000193 pc: 0xffffff34
MMU: disabled, D-Cache: disabled, I-Cache: disabled

Breakpoint 1, _start () at arch/arm/lib/vectors.S:89
89		ARM_VECTORS
(gdb) list
	84	
	85	_start:
	86	#ifdef CFG_SYS_DV_NOR_BOOT_CFG
	87		.word	CFG_SYS_DV_NOR_BOOT_CFG
	88	#endif
	89		ARM_VECTORS
	90	#endif /* !defined(CONFIG_ENABLE_ARM_SOC_BOOT0_HOOK) */
	91	
	92	#if !CONFIG_
	```

## prepare bootable sd card.
1. make "boot.bin"  
   `bootgen -arch zynq -image example_for_zynq.bif -o i boot.bin -w`
1. format sd card by FAT32  
   ` sudo mkfs.vfat -F 32 -n BOOT /dev/sdxy`
1. copy boot.bin to sdcard  
## 
