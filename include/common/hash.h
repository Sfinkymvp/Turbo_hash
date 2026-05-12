#ifndef HASH_H
#define HASH_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef uint64_t (*hash_function)(const char *key);
typedef int (*equals_function)(const char *key1, const char *key2);

int string_equals_naive(const char *str1, const char *str2);

#if defined(HASH_INTR)
    uint64_t hash_string_crc32_intr(const char *key);
    
    #define HASH(str) hash_string_crc32_intr(str)

#else 
    uint64_t hash_string_crc32_naive(const char *key);

    #define HASH(str) hash_string_crc32_naive(str)

#endif // HASH_INTR

#if defined(STRLEN_ASM)
    extern "C" {
        size_t my_strlen_avx512_asm(const char *str);
    }

    #define STRING_LEN(str) my_strlen_avx512_asm(str)
    
#elif defined(STRLEN_INLINE)
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

    #define STRING_LEN(str) my_strlen_inline(str)

#else // STRLEN
    #include <string.h>

    #define STRING_LEN(str) strlen(str)

#endif // STRLEN

#endif // HASH_H