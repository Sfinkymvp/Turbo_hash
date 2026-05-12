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
            "vpxord zmm2, zmm2, zmm2;"
            "1:;"
            "vmovdqu8 zmm0, [%[s1]];"
            "vmovdqu8 zmm1, [%[s2]];"
            
            "vpcmpub k1, zmm0, zmm1, 4;" 
            "vpcmpub k3, zmm0, zmm2, 0;" 
            "korq k4, k3, k1;"
            "kortestq k4, k4;"
            "jne 2f;"

            "add %[s1], 64;"
            "add %[s2], 64;"
            "jmp 1b;"

            "2:;"
            "kmovq %q[res], k1;"
            "test %q[res], %q[res];"
            "setnz %b[res];"  
            "movzx %[res], %b[res];"
            ".att_syntax prefix"
            : [res] "=&r" (res), [s1] "+r" (str1), [s2] "+r" (str2)
            : 
            : "zmm0", "zmm1", "zmm2", "k1", "k3", "k4", "cc", "memory"
        );

        return res;
    }

    #define COMPARE_KEYS(str1, str2) my_strcmp_inline((str1), (str2))

#else
    #include <string.h>

    #define COMPARE_KEYS(str1, str2) strcmp((str1), (str2))

#endif // STRCMP_ASM

#endif // COMPARE_H
