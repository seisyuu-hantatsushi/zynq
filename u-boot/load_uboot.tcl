# How to use.
# in telnet console.
#
# set FSBL_ELF /path/fsbl.elf
# set BITSTREAM /path/bitstream.bit # optional
# set UBOOT_ELF /path/u-boot.elf
# set UBOOT_ENTRY 0x04000000
# source run_zynq.tcl 
#

proc try_load_bitstream {bitfile} {
    echo "Bitstream: $bitfile"

    # 1) Try pld load with device index 0 (works if target cfg already created a PLD)
    if {![catch {pld load 0 $bitfile} msg]} {
        echo "PL configured (pld load 0)."
        return
    }
    echo "pld load 0 failed: $msg"

    # 2) Try to create a PLD device and retry by name
    # The TAP name is often 'zynq_pl.bs' (as shown in your scan output).
    if {[catch {pld create pl0 virtex2 -chain-position zynq_pl.bs -no_jstart} cmsg]} {
        echo "pld create failed: $cmsg"
    } else {
        echo "Created PLD device 'pl0'."
        if {![catch {pld load pl0 $bitfile} msg2]} {
            echo "PL configured (pld load pl0)."
            return
        }
        echo "pld load pl0 failed: $msg2"
    }

    echo "ERROR: Bitstream download failed."
    shutdown error
}

if {![info exists FSBL_ELF] || ![info exists UBOOT_ELF]} {
    echo "Set FSBL_ELF and UBOOT_ELF via -c. Example:"
    echo "  -c \"set FSBL_ELF {/path/fsbl.elf}\" -c \"set UBOOT_ELF {/path/u-boot.elf}\""
    shutdown error
}

if {[info exists BITSTREAM]} {
    set LOAD_BITSTREAM 1
} else {
    set LOAD_BITSTREAM 0
}

init
targets zynq.cpu0
halt
cortex_a dbginit

# FSBL
load_image $FSBL_ELF
resume 0x00000000
sleep 500
halt

# DDRチェック
#mww 0x04000000 0x12345678
#mdw 0x04000000 1

if {$LOAD_BITSTREAM} {
    try_load_bitstream $BITSTREAM
}

# Default U-Boot entry
if {![info exists UBOOT_ENTRY]} {
    set UBOOT_ENTRY 0x04000000
    echo "UBOOT_ENTRY not set -> defaulting to $UBOOT_ENTRY"
}

# U-Boot
load_image $UBOOT_ELF
resume $UBOOT_ENTRY
