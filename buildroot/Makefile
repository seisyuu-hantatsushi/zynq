FSBL_SRC=../minimam_platform/SW/minimam_platform/zynq_fsbl/build/fsbl.elf
BITSTREAM_SRC=../minimam_platform/SW/minimam_platform/hw/sdt/minimam_platform.bit
UBOOT_SRC=../../u-boot-xlnx/u-boot.elf
UBOOT_DTB_SRC=../../u-boot-xlnx/u-boot.dtb

release: boot.bin

boot.bin: fsbl.elf bitstream.bit u-boot.elf u-boot.dtb boot.bif
	bootgen -arch zynq -image boot.bif -o $@ -w

fsbl.elf: $(FSBL_SRC)
	cp $< ./$@

bitstream.bit: $(BITSTREAM_SRC)
	cp $< ./$@

u-boot.elf: $(UBOOT_SRC)
	cp $< ./$@

u-boot.dtb: $(UBOOT_DTB_SRC)
	cp $< ./$@

clean:
	rm -rf boot.bin fsbl.elf bitstream.bit u-boot.elf u-boot.dtb
