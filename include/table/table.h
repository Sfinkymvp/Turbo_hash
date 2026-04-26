#ifndef TABLE_H
#define TABLE_H

#include <stddef.h>

#include "common/hash.h"

typedef struct HashNode HashNode;
struct HashNode {
    const char *key;
    int value;
    HashNode *next;
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

#endif // TABLE_H