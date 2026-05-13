.intel_syntax noprefix

.section .note.GNU-stack

.section .text
    .global my_strlen_avx512_asm

.equ YMM_SIZE,  32

my_strlen_avx512_asm:
    vpxord      ymm1, ymm1, ymm1
    vmovdqu8    ymm0, [rdi]
    
    vpcmpub     k1, ymm0, ymm1, 0
    
    kmovd       eax, k1 
    tzcnt       eax, eax 
    
    ret
