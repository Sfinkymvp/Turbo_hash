.intel_syntax noprefix

.section .note.GNU-stack

.section .text
    .global my_strcmp

.equ PAGE_SIZE, 4096
.equ PAGE_MASK, 0xFFF
.equ ZMM_SIZE,  64
.equ CMP_EQ,    0
.equ CMP_NEQ,   4

# строка 1 в rdi
# строка 2 в rsi
my_strcmp:
    vpxord      zmm2, zmm2, zmm2

.calculate_safe_chunks:

    mov         rcx, rdi
    and         rcx, PAGE_MASK
    mov         rax, PAGE_SIZE
    sub         rax, rcx 

    mov         rcx, rsi
    and         rcx, PAGE_MASK
    mov         r10, PAGE_SIZE
    sub         r10, rcx

    cmp         rax, r10
    cmova       rax, r10

    mov         rdx, rax

    shr         rax, 6
    je          .edge_case_handling

.main_loop:
# Сравнение блоками по 64 байта
    vmovdqu64   zmm0, [rdi] 
    vmovdqu64   zmm1, [rsi]

    vpcmpub     k1, zmm0, zmm1, CMP_EQ
    knotq       k2, k1

    vpcmpub     k3, zmm0, zmm2, CMP_EQ
    korq        k4, k3, k2
    kortestq    k4, k4
    jne         .found_difference

    add         rdi, ZMM_SIZE
    add         rsi, ZMM_SIZE

    dec         rax
    jnz         .main_loop

    jmp         .calculate_safe_chunks

.edge_case_handling:
# В rdx наименьшее из расстояний до новой страницы среди строк
# 0 < rdx < 64
# Нам нужно получить число, у которого первые rdx бит единичные
    mov         rax, -1
    mov         rcx, ZMM_SIZE
    sub         rcx, rdx
    shr         rax, cl
    kmovq       k1, rax

# Сравнение блоками < 64 байт
    vmovdqu8    zmm0 {k1}{z}, [rdi]
    vmovdqu8    zmm1 {k1}{z}, [rsi]

    vpcmpub     k1, zmm0, zmm1, CMP_NEQ

    vpcmpub    k3 {k1}, zmm0, zmm2, CMP_EQ

    korq        k4, k1, k3

    kortestq    k4, k4
    jne         .found_difference

    add         rdi, rdx
    add         rsi, rdx
    jmp         .calculate_safe_chunks

.found_difference:
    kmovq       rax, k4
    tzcnt       rcx, rax

    movzx       eax, byte ptr [rdi + rcx]
    movzx       edx, byte ptr [rsi + rcx]
    sub         eax, edx

.exit:
    vzeroupper
    ret
