#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "table/table.h"
#include "common/hash.h"
#include "common/report.h"

static int chain_ht_rehash(ChainHashTable *table);

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
    CHAIN_HT_ASSERT(table); assert(key);

    if ((double)table->size / table->capacity >= table->max_load_factor) {
        int status = chain_ht_rehash(table);
        if (status != 0) {
            return status;
        }
    }

    int result = 0;
    uint64_t hash = HASH(key) & (table->capacity - 1);
    if (table->buckets[hash] == NULL) {
        table->buckets[hash] = BUCKET_INIT();
        if (table->buckets[hash] == NULL) {
            return 1; 
        }
    }

    int result = BUCKET_INSERT(table->buckets[hash], key, value);
    if (result == -1) {
        return 1;
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
    int res = BUCKET_FIND(table->buckets[hash], key);

    return res;
}

int chain_ht_remove(ChainHashTable *table, const char *key)
{
    CHAIN_HT_ASSERT(table); assert(key);

    uint64_t hash = HASH(key) & (table->capacity -1);
    int result = BUCKET_REMOVE(table->buckets[hash], key);
    if (result == -1) {
        return 1;
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

    // uint64_t old_capacity = table->capacity;
    // uint64_t new_capacity = old_capacity  *2;

    // HashNode **old_buckets = table->buckets;
    // HashNode **new_buckets = (HashNode **)calloc(new_capacity, sizeof(HashNode *));
    // if (new_buckets == NULL) {
    //     ERROR("Memory allocation error");
    //     return 1;
    // }

    // for (uint64_t i = 0; i < old_capacity; i++) {
    //     HashNode *current = old_buckets[i];
    //     while (current != NULL) {
    //         HashNode *next_temp = current->next;

    //         uint64_t hash = HASH(current->key) & (table->capacity - 1);

    //         current->next = new_buckets[hash];
    //         new_buckets[hash] = current;
    //         current = next_temp;
    //     }
    // }

    // table->capacity = new_capacity;
    // table->buckets = new_buckets;
    // free(old_buckets);
    return 0;
}
