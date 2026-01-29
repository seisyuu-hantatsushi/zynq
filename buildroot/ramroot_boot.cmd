setenv initrd_high 0x30000000
setenv fdt_high    0x30000000

fatload mmc 0:1 0x02000000 zImage
fatload mmc 0:1 0x01000000 system.dtb
fatload mmc 0:1 0x04000000 rootfs.cpio
setenv rdsize ${filesize}

setenv bootargs 'console=ttyPS0,115200 earlycon=cdns,0xe0000000,115200 loglevel=8 ignore_loglevel'

bootz 0x02000000 0x04000000:${rdsize} 0x01000000
