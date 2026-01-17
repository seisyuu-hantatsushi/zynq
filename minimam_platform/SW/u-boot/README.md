# U-Boot for minimam_platform
## Memory Map for U-Boot
|start address| binary |
| :- |:- |
|0x0000_0000| fsbl.elf |
|0x0010_0000| device binary tree |
|0x0400_0000| u-boot   |
## prepare bootable sd card.
1. make "boot.bin"  
   `bootgen -arch zynq -image example_for_zynq.bif -o i boot.bin -w`
1. format sd card by FAT32  
   ` sudo mkfs.vfat -F 32 -n BOOT /dev/sdxy`
1. copy boot.bin to sdcard  
