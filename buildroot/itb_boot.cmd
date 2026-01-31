setenv initrd_high 0x30000000
setenv fdt_high    0x30000000

setenv bootargs 'console=ttyPS0,115200 earlycon=cdns,0xe0000000,115200 loglevel=8 ignore_loglevel'

fatload mmc 0:1 ${loadaddr} image.itb
bootm ${loadaddr}#conf-1
