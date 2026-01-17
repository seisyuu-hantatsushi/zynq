# Zynq Application
## Target Device
RK-ZYNQ7020-F(TZT)
## Addres Map
|addres range|size|peripehra|
|:-:|:-:|:-:|
|0000_0000 - 0003_FFFF| 256KiB | On Chip Memory(OCM) |
|0004_0000 - 4000_0000| 1GiB   | DDR                 |
## assign space for bearmetal application
|region |start addres| offset byte from DDR Start | image |
|:-:|:-:|:-:|:-:|
|OCM|0000_0000 - | -   | fsbl.elf |
|DDR|0004_0000 - |   0B| DDR start address |
|DDR|0010_0000 - | 1MiB| Device Tree Binary |
|DDR|0020_0000 - | 2MiB| application load space |
|DDR|0400_0000 - |64MiB| u-boot.elf |
