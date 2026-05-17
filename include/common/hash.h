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
    static inline size_t my_strlen_inline(const char *str) {
        size_t len;

        __asm__ volatile (
            ".intel_syntax noprefix;"

            "vpxord  ymm1, ymm1, ymm1;"
            "vmovdqu8 ymm0, [%[str]];" 
            "vpcmpub k1, ymm0, ymm1, 0;"
            
            "kmovq %[len], k1;"  
            "tzcnt %[len], %[len];"
    
            ".att_syntax prefix;"
            : [len] "=&r" (len)   
            : [str] "r" (str)   
            : "rax", "ymm0", "ymm1", "k1", "cc", "memory"
        );

        return len;
    }

    #define STRING_LEN(str) my_strlen_inline(str)

#else // STRLEN
    #include <string.h>

    #define STRING_LEN(str) strlen(str)

#endif // STRLEN

#endif // HASH_H