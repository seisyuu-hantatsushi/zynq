# GDB script to load FSBL, optional bitstream, then U-Boot and break at entry.
# Usage:
#   arm-none-eabi-gdb -x uboot_load.gdb
#   (set FSBL/UBOOT paths by editing below or overriding via environment and `set` in GDB)

# Connect to OpenOCD GDB server
# (Assumes OpenOCD is already running and listening on :3333)
target extended-remote :3333

# Halt and initialize Cortex-A debug
# Note: reset is disabled in ft2232-cha-jtag.cfg (reset_config none)
monitor halt
monitor cortex_a dbginit

# ---- FSBL ----
# Update this path to your FSBL ELF
file ./fsbl.elf
load
monitor resume 0x00000000
monitor sleep 500
monitor halt

# ---- Bitstream (optional) ----
# Uncomment if you want to load bitstream via OpenOCD
# monitor pld load 0 ./minimam_platform.bit

# ---- U-Boot ----
# Update this path to your U-Boot ELF
file ./u-boot
load

# Set entry address from ELF symbol (uses _start)
# If your ELF doesn't define _start, replace with a concrete address.
set $uboot_entry = &_start
b *$uboot_entry

# Continue into U-Boot
c
