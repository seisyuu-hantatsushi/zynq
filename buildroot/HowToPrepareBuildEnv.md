# How to prepare building buildroot for zynq
## Buildroot uri
https://buildroot.org/
- This document uses Series 2025.11.x.
## How to prepare building env.
1. expanding archive.
1. configuration
   - BR2_arm=y
   - BR2_cortex_a9=y
   - BR2_ARM_EABIHF=y
   - BR2_KERNEL_HEADERS_CUSTOM_GIT=y
   - BR2_KERNEL_HEADERS_CUSTOM_REPO_URL="https://github.com/Xilinx/linux-xlnx.git"
   - BR2_KERNEL_HEADERS_CUSTOM_REPO_VERSION="xilinx-v2025.2"
   - BR2_LINUX_KERNEL_CUSTOM_GIT=y
   - BR2_LINUX_KERNEL_CUSTOM_REPO_URL="https://github.com/Xilinx/linux-xlnx.git"
   - BR2_LINUX_KERNEL_CUSTOM_REPO_VERSION="xilinx-v2025.2"
   - BR2_LINUX_KERNEL_CUSTOM_REPO_GIT_SUBMODULES=y
   - BR2_LINUX_KERNEL_VERSION="xilinx-v2025.2"
   - BR2_LINUX_KERNEL_DEFCONFIG="xilinx_zynq"
1. make
