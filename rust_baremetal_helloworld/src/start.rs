core::arch::global_asm!(
    r#"
    .section .text._init_segments
    .global _init_segments
    .arm
    .type _init_segments, %function
    _init_segments:
       // Initialize .bss
       ldr r0, =__sbss
       ldr r1, =__ebss
       mov r2, 0
    _bss_init_loop_start:
       cmp r1,r0
       beq _bss_init_loop_end
       stm r0!,{{r2}}
       b _bss_init_loop_start
    _bss_init_loop_end:
       bx lr
    .section .text._start
    .global _start
    .type _start, %function
    _start:
       bl _init_segments
       ldr sp, = __stack_top
       bl main
       b .
       .size _start, . -_start
    "#);
