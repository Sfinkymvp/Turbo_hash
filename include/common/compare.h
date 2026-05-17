#ifndef COMPARE_H
#define COMPARE_H

#if defined(STRCMP_ASM)
    extern "C" {
        int my_strcmp_avx512_asm(const char* str1, const char* str2);
    }

    #define COMPARE_KEYS(str1, str2) my_strcmp_avx512_asm((str1), (str2))

#elif defined(STRCMP_INLINE)
    static inline int my_strcmp_inline(const char *str1, const char *str2) {
        int res;

        __asm__ volatile (
            ".intel_syntax noprefix;"

            "vmovdqu8 ymm0, [%[s1]];"
            "vmovdqu8 ymm1, [%[s2]];"
            
            "vpcmpub k1, ymm0, ymm1, 4;" 

            "kmovq %q[res], k1;"
            "test %q[res], %q[res];"
            "setnz %b[res];"  
            "movzx %[res], %b[res];"

            ".att_syntax prefix"
            : [res] "=&r" (res), [s1] "+r" (str1), [s2] "+r" (str2)
            : 
            : "ymm0", "ymm1","k1", "cc", "memory"
        );

        return res;
    }

    #define COMPARE_KEYS(str1, str2) my_strcmp_inline((str1), (str2))

#else
    #include <string.h>

    #define COMPARE_KEYS(str1, str2) strcmp((str1), (str2))

#endif // STRCMP_ASM

#endif // COMPARE_H
