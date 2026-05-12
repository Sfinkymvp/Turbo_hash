.intel_syntax noprefix

.section .note.GNU-stack

.section .text
    .global my_strlen_avx512_asm

.equ ZMM_SIZE,  64

my_strlen_avx512_asm:
    vpxord      zmm1, zmm1, zmm1 
    xor         rax, rax   

.loop:
    vmovdqu8   zmm0, [rdi + rax]  
    vpcmpeqb    k1, zmm0, zmm1 
    
    kortestq    k1, k1    
    jnz         .found_null  
    
    add         rax, ZMM_SIZE
    jmp         .loop

.found_null:
    kmovq       rcx, k1    
    tzcnt       rcx, rcx    
    add         rax, rcx 

    ret
