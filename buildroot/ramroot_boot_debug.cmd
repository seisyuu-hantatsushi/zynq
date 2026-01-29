# 1) relocationを抑制（このU-Bootでは 0 が効く前提）
setenv initrd_high 0x30000000
setenv fdt_high    0x30000000

# 2) load
fatload mmc 0:1 0x02000000 zImage
fatload mmc 0:1 0x01000000 system.dtb
fatload mmc 0:1 0x04000000 rootfs.cpio
setenv rdsize ${filesize}

# 3) bootargs（ramdisk標準rootfsなので root= は書かない）
setenv bootargs 'console=ttyPS0,115200 earlycon=cdns,0xe0000000,115200 loglevel=8 ignore_loglevel'

# 4) 確認表示（アドレスを一致させる）
echo "BOOTARGS=${bootargs}"
echo "DTB @ 0x01000000, RD @ 0x04000000 size ${rdsize}"
md.b 0x04000000 10

# 5) DTBの最小チェック（存在するパスだけ）
fdt addr 0x01000000
fdt print /memory
fdt print /chosen
fdt print /aliases
fdt print /axi/serial@e0000000

# 6) boot
bootz 0x02000000 0x04000000:${rdsize} 0x01000000
