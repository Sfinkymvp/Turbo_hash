.intel_syntax noprefix

.section .note.GNU-stack

.section .text
    .global my_strcmp_avx512_asm

.equ ZMM_SIZE,  64
.equ CMP_EQ,    0
.equ CMP_NEQ,   4

my_strcmp_avx512_asm:
    vmovdqu8   zmm0, [rdi] 
    vmovdqu8   zmm1, [rsi]

    vpcmpub     k1, zmm0, zmm1, CMP_NEQ
    vpcmpub     k3, zmm0, zmm2, CMP_EQ

    korq        k4, k3, k1

    kortestq    k4, k4
    jne         .found_difference

    add         rdi, ZMM_SIZE
    add         rsi, ZMM_SIZE
    jmp         my_strcmp_avx512_asm

.found_difference:
    kmovq       rax, k4
    test        rax, rax
    setnz       al
    movzx       rax, al

    ret
