# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  "/home/kaz/work/github/seisyuu-hantatsushi/zynq/simple_hello_world/SW/platform/zynq_fsbl/zynq_fsbl_bsp/include/diskio.h"
  "/home/kaz/work/github/seisyuu-hantatsushi/zynq/simple_hello_world/SW/platform/zynq_fsbl/zynq_fsbl_bsp/include/ff.h"
  "/home/kaz/work/github/seisyuu-hantatsushi/zynq/simple_hello_world/SW/platform/zynq_fsbl/zynq_fsbl_bsp/include/ffconf.h"
  "/home/kaz/work/github/seisyuu-hantatsushi/zynq/simple_hello_world/SW/platform/zynq_fsbl/zynq_fsbl_bsp/include/sleep.h"
  "/home/kaz/work/github/seisyuu-hantatsushi/zynq/simple_hello_world/SW/platform/zynq_fsbl/zynq_fsbl_bsp/include/xilffs.h"
  "/home/kaz/work/github/seisyuu-hantatsushi/zynq/simple_hello_world/SW/platform/zynq_fsbl/zynq_fsbl_bsp/include/xilffs_config.h"
  "/home/kaz/work/github/seisyuu-hantatsushi/zynq/simple_hello_world/SW/platform/zynq_fsbl/zynq_fsbl_bsp/include/xilrsa.h"
  "/home/kaz/work/github/seisyuu-hantatsushi/zynq/simple_hello_world/SW/platform/zynq_fsbl/zynq_fsbl_bsp/include/xiltimer.h"
  "/home/kaz/work/github/seisyuu-hantatsushi/zynq/simple_hello_world/SW/platform/zynq_fsbl/zynq_fsbl_bsp/include/xtimer_config.h"
  "/home/kaz/work/github/seisyuu-hantatsushi/zynq/simple_hello_world/SW/platform/zynq_fsbl/zynq_fsbl_bsp/lib/libxilffs.a"
  "/home/kaz/work/github/seisyuu-hantatsushi/zynq/simple_hello_world/SW/platform/zynq_fsbl/zynq_fsbl_bsp/lib/libxilrsa.a"
  "/home/kaz/work/github/seisyuu-hantatsushi/zynq/simple_hello_world/SW/platform/zynq_fsbl/zynq_fsbl_bsp/lib/libxiltimer.a"
  )
endif()
