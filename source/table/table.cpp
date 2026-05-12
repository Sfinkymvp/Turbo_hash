#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "table/table.h"
#include "common/hash.h"
#include "common/report.h"

typedef struct {
    Bucket *new_buckets;
    uint64_t new_capacity;
} RehashContext;

static int chain_ht_rehash(ChainHashTable *table);
static int rehash_transfer_node(const char *key, int value, void *user_data);

ChainHashTable *chain_ht_create(uint64_t capacity, double max_load_factor, hash_function hash, equals_function equals)
{
    assert(hash);

    ChainHashTable *table = (ChainHashTable *)calloc(1, sizeof(ChainHashTable));
    if (table == NULL) {
        ERROR("Memory allocation error");
        return NULL;
    }

    table->buckets = (Bucket *)calloc(capacity, sizeof(Bucket));
    if (table->buckets == NULL) {
        ERROR("Memory allocation error");
        chain_ht_destroy(table);
        return NULL;
    }

    table->capacity = capacity;
    table->size = 0;
    table->max_load_factor = max_load_factor;
    table->hash_func = hash;
    table->equals_func = equals;

    return table;
}

int chain_ht_insert(ChainHashTable *table, const char *key, int value)
{
    CHAIN_HT_ASSERT(table); assert(key); assert(table->capacity > 0);

    if ((double)table->size / table->capacity >= table->max_load_factor) {
        int status = chain_ht_rehash(table);
        if (status == -1) {
            return -1;
        }
    }

    uint64_t hash = HASH(key) & (table->capacity - 1);
    if (table->buckets[hash] == NULL) {
        table->buckets[hash] = BUCKET_INIT();
        if (table->buckets[hash] == NULL) {
            return -1;
        }
    }

    int result = BUCKET_INSERT(table->buckets[hash], key, value);
    if (result == -1) {
        return -1;
    }

    if (result == 0) {
        table->size++;
    }

    return 0;
}

int chain_ht_find(const ChainHashTable *table, const char *key, int *result)
{
    CHAIN_HT_ASSERT(table); assert(key); assert(result);

    uint64_t hash = HASH(key) & (table->capacity - 1);
    if (table->buckets[hash] == NULL) {
        return 0;
    }

    return BUCKET_FIND(table->buckets[hash], key, result);
}

int chain_ht_remove(ChainHashTable *table, const char *key)
{
    CHAIN_HT_ASSERT(table); assert(key);

    uint64_t hash = HASH(key) & (table->capacity -1);
    int result = BUCKET_REMOVE(table->buckets[hash], key);
    if (result == -1) {
        return -1;
    }

    if (result == 0) {
        table->size--;
    }

    return 0;
}

void chain_ht_destroy(ChainHashTable *table)
{
    assert(table);

    for (uint64_t i = 0; i < table->capacity; i++) {
        if (table->buckets[i] != NULL) {
            BUCKET_DESTROY(table->buckets[i]);
        }
    }

    free(table->buckets);
    free(table);
}

static int chain_ht_rehash(ChainHashTable *table)
{
    CHAIN_HT_ASSERT(table);

    uint64_t old_capacity = table->capacity;
    uint64_t new_capacity = 2 * old_capacity;
    Bucket *new_buckets = (Bucket *)calloc(new_capacity ,sizeof(Bucket));
    if (new_buckets == NULL) {
        ERROR("memory allocation error");
        return -1;
    }

    RehashContext context = {new_buckets, new_capacity};

    uint64_t i = 0;
    for (i = 0; i < old_capacity; i++) {
        if (table->buckets[i] != NULL) {
            if (BUCKET_FOR_EACH(table->buckets[i], rehash_transfer_node, &context) == -1) {
                break;
            }

            BUCKET_DESTROY(table->buckets[i]);
            table->buckets[i] = NULL;
        }
    }

    if (i != old_capacity) {
        for (uint64_t k = 0; k < new_capacity; k++) {
            if (new_buckets[k] != NULL) {
                BUCKET_DESTROY(new_buckets[k]);
            }
        }
        return -1;
    }

    free(table->buckets);
    table->buckets = new_buckets;
    table->capacity = new_capacity;

    return 0;
}

static int rehash_transfer_node(const char *key, int value, void *user_data)
{
    assert(key); assert(user_data);

    RehashContext *context = (RehashContext *)user_data;

    uint64_t hash = HASH(key) & (context->new_capacity - 1);

    if (context->new_buckets[hash] == NULL) {
        context->new_buckets[hash] = BUCKET_INIT();
        if (context->new_buckets[hash] == NULL) {
            return -1; 
        }
    }

    return BUCKET_INSERT(context->new_buckets[hash], key, value);
}
