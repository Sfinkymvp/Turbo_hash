#ifndef TABLE_H
#define TABLE_H

#include <stddef.h>

#include "common/hash.h"

#if defined(CF_LIST)
    #include "table/cf_list.h"
    
    typedef CFList* Bucket;
    
    #define BUCKET_INIT(bucket)               cf_list_init(bucket)
    #define BUCKET_INSERT(bucket, key, value) cf_list_insert(bucket, key, value)
    #define BUCKET_FIND(bucket, key)          cf_list_find(bucket, key)
    #define BUCKET_REMOVE(bucket, key)        cf_list_remove(bucket, key)
    #define BUCKET_DESTROY(bucket)            cf_list_destroy(bucket)

#elif defined(STD_LIST)
    #include "table/std_list.h"

    typedef StdList* Bucket;

    #define BUCKET_INIT(bucket)               std_list_init(bucket)
    #define BUCKET_INSERT(bucket, key, value) std_list_insert(bucket, key, value)
    #define BUCKET_FIND(bucket, key)          std_list_find(bucket, key)
    #define BUCKET_REMOVE(bucket, key)        std_list_remove(bucket, key)
    #define BUCKET_DESTROY(bucket)            std_list_destroy(bucket)

#elif defined(CUSTOM_ARRAY)
    #include "table/array.h"

    typedef DynamicArray* Bucket;

    #define BUCKET_INIT(bucket)               array_init(bucket)
    #define BUCKET_INSERT(bucket, key, value) array_insert(bucket, key, value)
    #define BUCKET_FIND(bucket, key)          array_find(bucket, key)
    #define BUCKET_REMOVE(bucket, key)        array_remove(bucket, key)
    #define BUCKET_DESTROY(bucket)            array_destroy(bucket)

#else // CLASSIC_LIST
    #include "table/classic_list.h"
    
    typedef CList* Bucket;
    
    #define BUCKET_INIT(bucket)               classic_list_init(bucket)
    #define BUCKET_INSERT(bucket, key, value) classic_list_insert(bucket, key, value)
    #define BUCKET_FIND(bucket, key)          classic_list_find(bucket, key)
    #define BUCKET_REMOVE(bucket, key)        classic_list_remove(bucket, key)
    #define BUCKET_DESTROY(bucket)            classic_list_destroy(bucket)

#endif // CLASSIC_LIST

typedef struct ChainHashTable {
    Bucket *buckets;
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
