# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  "/home/kaz/work/github/seisyuu-hantatsushi/zynq/simple_hello_world/SW/platform/ps7_cortexa9_0/standalone_ps7_cortexa9_0/bsp/include/sleep.h"
  "/home/kaz/work/github/seisyuu-hantatsushi/zynq/simple_hello_world/SW/platform/ps7_cortexa9_0/standalone_ps7_cortexa9_0/bsp/include/xiltimer.h"
  "/home/kaz/work/github/seisyuu-hantatsushi/zynq/simple_hello_world/SW/platform/ps7_cortexa9_0/standalone_ps7_cortexa9_0/bsp/include/xtimer_config.h"
  "/home/kaz/work/github/seisyuu-hantatsushi/zynq/simple_hello_world/SW/platform/ps7_cortexa9_0/standalone_ps7_cortexa9_0/bsp/lib/libxiltimer.a"
  )
endif()
