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
        ".intel_syntax noprefix;"
        "vpxord zmm1, zmm1, zmm1;"
        "xor %[len], %[len];" 
        
        "1:;"
        "vmovdqu8 zmm0, [%[str] + %[len]];" 
        "vpcmpeqb k1, zmm0, zmm1;"
        
        "kortestq k1, k1;" 
        "jnz 2f;"  
        
        "add %[len], 64;" 
        "jmp 1b\n"

        "2:;"
        "kmovq rcx, k1;"  
        "tzcnt rcx, rcx;"  
        "add %[len], rcx;" 
        
        ".att_syntax prefix"
        : [len] "=&r" (len)   
        : [str] "r" (str)   
        : "rcx", "zmm0", "zmm1", "k1", "cc", "memory"
    );

    return len;
}

#endif // HASH_H