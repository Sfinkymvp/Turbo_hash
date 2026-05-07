#ifndef TABLE_H
#define TABLE_H

#include <stddef.h>

#include "common/hash.h"

typedef struct HashNode HashNode;
struct HashNode {
    const char *key;
    HashNode *next;
    int value;
};

typedef struct ChainHashTable {
    HashNode **buckets;
    size_t size;
    size_t capacity;
    hash_function hash_func;
    equals_function equals_func;
    double max_load_factor;
} ChainHashTable;

#define CHAIN_HT_ASSERT(table_ptr) \
    assert((table_ptr)->buckets); assert((table_ptr)->hash_func)

ChainHashTable *chain_ht_create(uint64_t capacity, double max_load_factor, hash_function hash, equals_function equals);
int chain_ht_insert(ChainHashTable *table, const char *key, int value);
int chain_ht_find(const ChainHashTable *table, const char *key, int *result);
int chain_ht_remove(ChainHashTable *table, const char *key);
void chain_ht_destroy(ChainHashTable *table);

static inline int my_strcmp_inline_evex(const char *s1, const char *s2) {
    int res;

    __asm__ volatile (
        ".intel_syntax noprefix;"
        "vpxord zmm2, zmm2, zmm2\n"
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
        "jmp 1b\n"

        "2:;"
        "kmovq %q[res], k1;"
        "test %q[res], %q[res];"
        "setnz %b[res];"  
        "movzx %[res], %b[res];"
        ".att_syntax prefix"
        : [res] "=&r" (res), [s1] "+r" (s1), [s2] "+r" (s2)
        : 
        : "zmm0", "zmm1", "zmm2", "k1", "k3", "k4", "cc", "memory"
    );

    return res;
}
#endif // TABLE_H