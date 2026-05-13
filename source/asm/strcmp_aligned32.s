.intel_syntax noprefix

.section .note.GNU-stack

.section .text
    .global my_strcmp_avx512_asm

.equ CMP_NEQ, 4

my_strcmp_ymm_fixed:
    vmovdqu8    ymm0, [rdi] 
    vmovdqu8    ymm1, [rsi]

    vpcmpub     k1, ymm0, ymm1, CMP_NEQ
    
    kmovd       eax, k1
    test        eax, eax
    setnz       al
    movzx       rax, al
    
    ret