fatload mmc 0:1 0x02000000 zImage
fatload mmc 0:1 0x02A00000 system.dtb
setenv bootargs 'console=ttyPS0,115200 earlycon=cdns,0xe0000000,115200 earlyprintk ignore_loglevel loglevel=8 panic=1 root=/dev/mmcblk0p2 rootfstype=ext2 rw rootwait'
bootz 0x02000000 - 0x02A00000
