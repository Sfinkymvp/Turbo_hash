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

.fast_path_check:
    mov         rax, rdi
    and         eax, PAGE_MASK
    cmp         eax, 0xFC0
    ja          .calculate_safe_chunks

    mov         rcx, rsi
    and         ecx, PAGE_MASK
    cmp         ecx, 0xFC0
    ja          .calculate_safe_chunks

.fast_compare:
    vmovdqu64   zmm0, [rdi]
    vmovdqu64   zmm1, [rsi]

    vpcmpub     k1, zmm0, zmm1, CMP_EQ
    knotq       k2, k1
    vpcmpub     k3, zmm0, zmm2, CMP_EQ
    korq        k4, k3, k2
    
    kortestq    k4, k4
    je          .calculate_safe_chunks

    kmovq       rax, k4
    tzcnt       rcx, rax
    movzx       eax, byte ptr [rdi + rcx]
    movzx       edx, byte ptr [rsi + rcx]
    sub         eax, edx
    vzeroupper
    ret

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

    mov         r8, rax
    shr         r8, 8  
    je          .check_64_chunks

.main_loop_unroll_x4:
    vmovdqu64   zmm0, [rdi]
    vmovdqu64   zmm3, [rdi + 64]
    vmovdqu64   zmm5, [rdi + 128]
    vmovdqu64   zmm7, [rdi + 192]

    vmovdqu64   zmm1, [rsi]
    vmovdqu64   zmm4, [rsi + 64]
    vmovdqu64   zmm6, [rsi + 128]
    vmovdqu64   zmm8, [rsi + 192]

    vpcmpub     k1, zmm0, zmm1, CMP_EQ
    vpcmpub     k2, zmm3, zmm4, CMP_EQ
    vpcmpub     k3, zmm5, zmm6, CMP_EQ
    vpcmpub     k4, zmm7, zmm8, CMP_EQ

    knotq       k1, k1
    knotq       k2, k2
    knotq       k3, k3
    knotq       k4, k4

    vpcmpub     k5, zmm0, zmm2, CMP_EQ
    vpcmpub     k6, zmm3, zmm2, CMP_EQ
    vpcmpub     k7, zmm5, zmm2, CMP_EQ
    vpcmpub     k0, zmm7, zmm2, CMP_EQ  

    korq        k1, k1, k5
    korq        k2, k2, k6
    korq        k3, k3, k7
    korq        k4, k4, k0   

    korq        k5, k1, k2
    korq        k6, k3, k4
    korq        k7, k5, k6

    kortestq    k7, k7
    jne         .resolve_unrolled_diff 

    add         rdi, 256
    add         rsi, 256

    dec         r8
    jnz         .main_loop_unroll_x4

    jmp         .calculate_safe_chunks


.check_64_chunks:
    mov         r8, rax
    shr         r8, 6  
    je          .edge_case_handling

.loop_64:
    vmovdqu64   zmm0, [rdi] 
    vmovdqu64   zmm1, [rsi]

    vpcmpub     k1, zmm0, zmm1, CMP_EQ
    knotq       k2, k1

    vpcmpub     k3, zmm0, zmm2, CMP_EQ
    korq        k4, k3, k2
    kortestq    k4, k4
    jne         .do_tzcnt_k4

    add         rdi, ZMM_SIZE
    add         rsi, ZMM_SIZE

    dec         r8
    jnz         .loop_64

    jmp         .calculate_safe_chunks


.resolve_unrolled_diff:
    kortestq    k1, k1
    jne         .diff_chunk_0
    kortestq    k2, k2
    jne         .diff_chunk_1
    kortestq    k3, k3
    jne         .diff_chunk_2

.diff_chunk_3:
    add         rdi, 192
    add         rsi, 192
    kmovq       rax, k4
    jmp         .do_tzcnt

.diff_chunk_2:
    add         rdi, 128
    add         rsi, 128
    kmovq       rax, k3
    jmp         .do_tzcnt

.diff_chunk_1:
    add         rdi, 64
    add         rsi, 64
    kmovq       rax, k2
    jmp         .do_tzcnt

.diff_chunk_0:
    kmovq       rax, k1
    jmp         .do_tzcnt


.do_tzcnt_k4:
    kmovq       rax, k4
.do_tzcnt:
    tzcnt       rcx, rax
    movzx       eax, byte ptr [rdi + rcx]
    movzx       edx, byte ptr [rsi + rcx]
    sub         eax, edx
    vzeroupper
    ret


.edge_case_handling:
    mov         rax, -1
    mov         rcx, ZMM_SIZE
    sub         rcx, rdx
    shr         rax, cl
    kmovq       k1, rax

    vmovdqu8    zmm0 {k1}{z}, [rdi]
    vmovdqu8    zmm1 {k1}{z}, [rsi]

    vpcmpub     k1, zmm0, zmm1, CMP_NEQ
    vpcmpub     k3 {k1}, zmm0, zmm2, CMP_EQ
    korq        k4, k1, k3

    kortestq    k4, k4
    jne         .do_tzcnt_k4

    add         rdi, rdx
    add         rsi, rdx
    jmp         .calculate_safe_chunks
