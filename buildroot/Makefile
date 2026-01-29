BUILDROOT_HOST_TOOLS_PATH=~/work/buildroot/buildroot-2025.11/output/host/bin
UBOOT_TOOLS_PATH=~/work/github/seisyuu-hantatsushi/u-boot-xlnx/tools

FSBL_SRC=../minimam_platform/SW/minimam_platform/zynq_fsbl/build/fsbl.elf
BITSTREAM_SRC=../minimam_platform/SW/minimam_platform/hw/sdt/minimam_platform.bit
UBOOT_SRC=../../u-boot-xlnx/u-boot.elf
UBOOT_DTB_SRC=../../u-boot-xlnx/u-boot.dtb

ROOTFS_PATH=~/work/buildroot/buildroot-2025.11/output/target/
ZIMAGE_SRC=~/work/buildroot/buildroot-2025.11/output/images/zImage
ROOTFS_EXT2_SRC=~/work/buildroot/buildroot-2025.11/output/images/rootfs.ext2
ROOTFS_CPIO_SRC=~/work/buildroot/buildroot-2025.11/output/images/rootfs.cpio

DTB_SRC=~/work/buildroot/buildroot-2025.11/output/images/rk-zynq-7020.dtb
#DTB_SRC=../../u-boot-xlnx/u-boot.dtb

release: boot.bin

sdcard_img: outputs/sdcard.img

ramroot_img: images/boot.bin ./images/zImage ./images/system.dtb images/rootfs.cpio.gz ramroot_boot.cmd
	PATH=$(UBOOT_TOOLS_PATH):$$PATH mkimage -A arm -T script -C none -n "Zynq boot" -d ramroot_boot.cmd images/boot.scr

outputs/sdcard.img: images/boot.bin images/zImage images/rootfs.ext2 images/system.dtb images/boot.scr sdcardimg.cfg 
	PATH=$(BUILDROOT_HOST_TOOLS_PATH):$$PATH genimage --inputpath ./images --outputpath ./outputs --rootpath $(ROOTFS_PATH) --config ./sdcardimg.cfg

images:
	mkdir ./images

images/boot.scr: images sdroot_boot.cmd
	PATH=$(UBOOT_TOOLS_PATH):$$PATH mkimage -A arm -T script -C none -n "Zynq boot" -d sdroot_boot.cmd images/boot.scr

images/boot.bin: images boot.bin
	cp ./boot.bin images/

images/zImage: images $(ZIMAGE_SRC)
	cp $(ZIMAGE_SRC) images/

images/rootfs.ext2: images $(ROOTFS_EXT2_SRC)
	cp $(ROOTFS_EXT2_SRC) images/

images/system.dtb: images $(DTB_SRC)
	cp $(DTB_SRC) images/system.dtb

images/rootfs.ext2.gz: images $(ROOTFS_EXT2_SRC)
	gzip -c $(ROOTFS_EXT2_SRC) > ./images/rootfs.ext2.gz

images/uRamdisk: images images/rootfs.ext2.gz 
	PATH=$(UBOOT_TOOLS_PATH):$$PATH mkimage -A arm -T ramdisk -C gzip -n "initramfs" -d images/rootfs.ext2.gz images/uRamdisk

images/rootfs.cpio.gz: images $(ROOTFS_CPIO_SRC)
	gzip -c $(ROOTFS_CPIO_SRC) > ./images/rootfs.cpio.gz

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
	rm -rf images/* outputs
