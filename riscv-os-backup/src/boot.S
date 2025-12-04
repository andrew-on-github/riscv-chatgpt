    .section .text.entry
    .globl _start
_start:
    la sp, stack_top        # set stack pointer

    # zero BSS
    la   t0, __bss_start
    la   t1, __bss_end
1:
    bge  t0, t1, 2f
    sd   zero, 0(t0)
    addi t0, t0, 8
    j    1b
2:
    call kmain

1:  j 1b                     # if kmain returns, spin forever

    .section .bss
    .align 16
    .global stack_bottom
stack_bottom:
    .skip 4096 * 4           # 16 KB stack
    .global stack_top
stack_top:

