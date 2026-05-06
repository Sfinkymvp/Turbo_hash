#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "table/table.h"
#include "common/hash.h"
#include "common/report.h"

static int chain_ht_rehash(ChainHashTable *table);
static void list_destroy(HashNode *head);

#ifdef STRCMP_ASM
extern int my_strcmp_avx512_asm(const char* str1, const char* str2);
#endif // STRCMP_ASM

ChainHashTable *chain_ht_create(uint64_t capacity, double max_load_factor, hash_function hash, equals_function equals)
{
    assert(hash);

    ChainHashTable *table = (ChainHashTable *)calloc(1, sizeof(ChainHashTable));
    if (table == NULL) {
        ERROR("Memory allocation error");
        return NULL;
    }

    table->buckets = (HashNode **)calloc(capacity, sizeof(HashNode*));
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

#ifdef HASH_INTR
    uint64_t hash = hash_string_crc32_intr(key) & (table->capacity - 1);
#else 
    uint64_t hash = hash_string_crc32_naive(key) & (table->capacity - 1);
#endif // HASH_INTR

    HashNode **head = &table->buckets[hash];

    HashNode *current = *head;
    while (current) {
#ifdef STRCMP_ASM
        int res = my_strcmp_avx512_asm(current->key, key);
#elif defined STRCMP_INLINE
        int res = my_strcmp_inline_evex(current->key, key);
#else
        int res = strcmp(current->key, key);
#endif // STRCMP_ASM

        if (res == 0) {
            return 0;
        }
 
        current = current->next;
    }

    HashNode *new_node = (HashNode *)calloc(1, sizeof(HashNode));
    if (new_node == NULL) {
        ERROR("Memory allocation error");
        return 1;
    }
    new_node->key = key;
    new_node->next = *head;
    new_node->value = value;

    *head = new_node;
    table->size++;
    return 0;  
}

int chain_ht_find(const ChainHashTable *table, const char *key, int *result)
{
    CHAIN_HT_ASSERT(table); assert(key); assert(result);

#ifdef HASH_INTR
    uint64_t hash = hash_string_crc32_intr(key) & (table->capacity - 1);
#elif defined INDIRECT
    uint64_t hash = table->hash_func(key) & (table->capacity - 1);
#else 
    uint64_t hash = hash_string_crc32_naive(key) & (table->capacity - 1);
#endif // HASH_INTR

    HashNode *current = table->buckets[hash];

    while (current != NULL) {
#ifdef STRCMP_ASM
        int res = my_strcmp_avx512_asm(current->key, key);
#elif defined STRCMP_INLINE
        int res = my_strcmp_inline_evex(current->key, key);
#elif defined INDIRECT
        int res = table->equals_func(current->key, key);
#else
        int res = strcmp(current->key, key);
#endif // STRCMP_ASM

        if (res == 0) {
            *result = current->value;
            return 0;
        }
        
        current = current->next;
    }
    return 1;
}

int chain_ht_remove(ChainHashTable *table, const char *key)
{
    CHAIN_HT_ASSERT(table); assert(key);

#ifdef HASH_INTR
    uint64_t hash = hash_string_crc32_intr(key) & (table->capacity - 1);
#else 
    uint64_t hash = hash_string_crc32_naive(key) & (table->capacity - 1);
#endif // HASH_INTR

    HashNode **head = &table->buckets[hash];

    HashNode *current = *head;
    HashNode *prev = NULL;

    while (current != NULL) {
#ifdef STRCMP_ASM
        int res = my_strcmp_avx512_asm(current->key, key);
#elif defined STRCMP_INLINE
        int res = my_strcmp_inline_evex(current->key, key);
#else
        int res = strcmp(current->key, key);
#endif // STRCMP_ASM

        if (res == 0) {
            if (prev == NULL) {
                *head = current->next;
            } else {
                prev->next = current->next;
            }

            free(current);
            table->size--;
            return 0;
        }

        prev = current;
        current = current->next;
    }

    return 1;
}

void chain_ht_destroy(ChainHashTable *table)
{
    assert(table);

    for (uint64_t i = 0; i < table->capacity; i++) {
        if (table->buckets[i] != NULL) {
            list_destroy(table->buckets[i]);
        }
    }

    free(table->buckets);
    free(table);
}


static int chain_ht_rehash(ChainHashTable *table)
{
    CHAIN_HT_ASSERT(table);

    uint64_t old_capacity = table->capacity;
    uint64_t new_capacity = old_capacity  *2;

    HashNode **old_buckets = table->buckets;
    HashNode **new_buckets = (HashNode **)calloc(new_capacity, sizeof(HashNode *));
    if (new_buckets == NULL) {
        ERROR("Memory allocation error");
        return 1;
    }

    for (uint64_t i = 0; i < old_capacity; i++) {
        HashNode *current = old_buckets[i];
        while (current != NULL) {
            HashNode *next_temp = current->next;

#ifndef HASH_INTR
            uint64_t hash = hash_string_crc32_intr(current->key) & (table->capacity - 1);
#else 
            uint64_t hash = hash_string_crc32_naive(current->key) & (table->capacity - 1);
#endif // HASH_INTR

            current->next = new_buckets[hash];
            new_buckets[hash] = current;
            current = next_temp;
        }
    }

    table->capacity = new_capacity;
    table->buckets = new_buckets;
    free(old_buckets);
    return 0;
}

static void list_destroy(HashNode *head)
{
    assert(head);

    HashNode *current = head;
    HashNode *temp = NULL;
    while (current) {
        temp = current->next;
        free(current);
        current = temp;
    }
}
