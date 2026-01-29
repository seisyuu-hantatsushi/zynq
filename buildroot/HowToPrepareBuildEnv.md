# How to prepare building buildroot for zynq
## Buildroot uri
https://buildroot.org/
- This document uses Series 2025.11.x.
## How to prepare buildroot. (For now, until we can confirm that the build succeeds.)
1. build u-boot
1. expanding buidroot archive.
1. `cd <buildroot>`
1. configuration
```
	BR2_arm=y
    BR2_cortex_a9=y
    BR2_ARM_EABIHF=y
    BR2_KERNEL_HEADERS_CUSTOM_GIT=y
    BR2_KERNEL_HEADERS_CUSTOM_REPO_URL="https://github.com/Xilinx/linux-xlnx.git"
    BR2_KERNEL_HEADERS_CUSTOM_REPO_VERSION="xilinx-v2025.2"
    BR2_LINUX_KERNEL_CUSTOM_GIT=y
    BR2_LINUX_KERNEL_CUSTOM_REPO_URL="https://github.com/Xilinx/linux-xlnx.git"
    BR2_LINUX_KERNEL_CUSTOM_REPO_VERSION="xilinx-v2025.2"
    BR2_LINUX_KERNEL_CUSTOM_REPO_GIT_SUBMODULES=y
    BR2_LINUX_KERNEL_VERSION="xilinx-v2025.2"
    BR2_LINUX_KERNEL_DEFCONFIG="xilinx_zynq"
    BR2_LINUX_KERNEL_INTREE_DTS_NAME="xilinx/zynq-zed"
	BR2_PACKAGE_HOST_GENIMAGE=y
    BR2_PACKAGE_HOST_MTOOLS=y
```
1. `make`
1. `cd <this dir>`
1. `make`

## How to build a Linux system image with the root filesystem stored on an SD card.
1. `make sdcard_img`

