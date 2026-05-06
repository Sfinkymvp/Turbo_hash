#ifndef HASH_H
#define HASH_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef uint64_t (*hash_function)(const char *key);
typedef int (*equals_function)(const char *key1, const char *key2);

int string_equals_naive(const char *str1, const char *str2);
uint64_t hash_string_crc32_naive(const char *key);
uint64_t hash_string_crc32_intr(const char *key);

static inline size_t my_strlen_inline_asm(const char *str) {
    size_t len;

    __asm__ volatile (
        ".intel_syntax noprefix\n\t"
        "vpxord zmm1, zmm1, zmm1\n\t"
        "xor %[len], %[len]\n\t" 
        
        "1:\n\t"
        "vmovdqu8 zmm0, [%[str] + %[len]]\n\t" 
        "vpcmpeqb k1, zmm0, zmm1\n\t"
        
        "kortestq k1, k1\n\t" 
        "jnz 2f\n\t"  
        
        "add %[len], 64\n\t" 
        "jmp 1b\n"

        "2:\n\t"
        "kmovq rcx, k1\n\t"  
        "tzcnt rcx, rcx\n\t"  
        "add %[len], rcx\n\t" 
        
        ".att_syntax prefix"
        : [len] "=&r" (len)   
        : [str] "r" (str)   
        : "rcx", "zmm0", "zmm1", "k1", "cc", "memory"
    );

    return len;
}

#endif // HASH_H