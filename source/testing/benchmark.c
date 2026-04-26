#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <immintrin.h>
#include <assert.h>

#include "testing/benchmark.h"
#include "testing/io.h"
#include "common/report.h"
#include "table/table.h"

const double DEFAULT_MAX_LOAD_FACTOR = 10.0;
const uint64_t DEFAULT_LOOKUP_ITERATIONS = 10ULL;
const uint64_t DEFAULT_HT_CAPACITY = 64ULL;

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define BENCHMARK_ASSERT(context_ptr) \
    assert((context_ptr)->buffer); assert((context_ptr)->keys); CHAIN_HT_ASSERT((context_ptr)->table);
   
static int fill_hash_table(BenchmarkContext *context);
static void collect_table_stats(BenchmarkContext* context);
static void shuffle_keys(char **keys, uint64_t key_count);
static void run_lookup_benchmark(BenchmarkContext *context);
static char **tokenize_buffer(char *buffer, uint64_t buf_size, uint64_t *token_count);
static uint64_t count_non_empty_lines(const char *buffer, uint64_t buf_size);

int run_benchmark(BenchmarkContext *context)
{
    assert(context);
    assert(context->buffer);
    assert(context->keys);

    REPORT(stderr, "filling hash table");
    int status = fill_hash_table(context);
    if (status != 0) {
        REPORT(stderr, "Error filling the hash table");
        return status;
    }

    REPORT(stderr, "collecting table stats");
    collect_table_stats(context);

    REPORT(stderr, "shuffling keys");
    shuffle_keys(context->keys, context->key_count);

    REPORT(stderr, "table: cap - %lu; size - %lu", context->table->capacity, context->table->size);
    REPORT(stderr, "lookup iters - %lu", context->lookup_iterations);
    run_lookup_benchmark(context);

   return 0;
}

int create_benchmark_context(BenchmarkContext *context, Args *args)
{
    assert(context);
    assert(args);

    int status = read_file_to_buffer(&context->buffer, &context->buf_size, args->file_path);
    if (status != 0) {
        return 1;
    }
    context->keys = tokenize_buffer(context->buffer, context->buf_size, &context->key_count);
    if (context->keys == NULL) {
        return 1;
    }
    context->lookup_iterations = args->lookup_iterations;
    context->table = chain_ht_create(DEFAULT_HT_CAPACITY, args->max_load_factor, hash_string_crc64_naive, string_equals);
    if (context->table == NULL) {
        return 1;
    }

    context->lookup_iterations = args->lookup_iterations;
    context->lookup_time = 0;
    context->collisions = 0;
    context->max_chain_length = 0;

    return 0;
}

void destroy_benchmark_context(BenchmarkContext *context)
{
    assert(context);

    free(context->buffer);
    free(context->keys);
    if (context->table) {
        chain_ht_destroy(context->table);
    }
}

static int fill_hash_table(BenchmarkContext *context)
{
    BENCHMARK_ASSERT(context);

    for (uint64_t i = 0; i < context->key_count; i++) {
        int status = chain_ht_insert(context->table, context->keys[i], rand());
        if (status != 0) {
            return 1;
        }
    }

    return 0;
}

static void collect_table_stats(BenchmarkContext* context)
{
    BENCHMARK_ASSERT(context);
    assert(context->table->size);
    // Хеш-таблица должна быть заполнена к моменту сбора информации

    for (uint64_t i = 0; i < context->table->capacity; i++) {
        uint64_t chain_length = 0;
        HashNode* curr = context->table->buckets[i];
        while (curr != NULL) {
            chain_length++;
            curr = curr->next;
        }

        context->max_chain_length = MAX(context->max_chain_length, chain_length);
    }
    context->collisions = context->key_count - context->table->size;
}

static void shuffle_keys(char **keys, uint64_t key_count)
{
    srand(42);

    for (uint64_t i = key_count - 1; i > 0; i--) {
        uint64_t j = (uint64_t)rand() % (i + 1);
        char *temp = keys[i];
        keys[i] = keys[j];
        keys[j] = temp;
    }
}


static void run_lookup_benchmark(BenchmarkContext *context)
{
    BENCHMARK_ASSERT(context);

    int result = 0;
    uint64_t start_tick = __rdtsc();
    for (uint64_t i = 0; i < context->lookup_iterations; i++) {
        for (uint64_t k = 0; k < context->key_count; k++) {
            int status = chain_ht_find(context->table, context->keys[k], &result);
            assert(status == 0);
        }
    }
    uint64_t end_tick = __rdtsc();

    if (result) {};

    context->lookup_time = end_tick - start_tick;
}

static char **tokenize_buffer(char *buffer, uint64_t buf_size, uint64_t *token_count)
{
    assert(buffer);
    assert(buf_size);

    uint64_t count = count_non_empty_lines(buffer, buf_size);
    char* *tokens = (char**)calloc(count, sizeof(char*));
    if (tokens == NULL) {
        REPORT(stderr, "Memory allocation error");
        return NULL;
    }

    uint64_t token_idx = 0;
    tokens[token_idx++] = buffer;

    // buf_size > 0 на этом этапе, buffer[buf_size - 1] = \0
    for (uint64_t i = 0; i < buf_size - 1; i++) {
        if (buffer[i] == '\n') {
            if (buffer[i + 1] != '\n' && buffer[i + 1] != '\0') {
                tokens[token_idx++] = buffer + i + 1;
                buffer[i] = '\0';
            }
        }
    }

    // Структура буфера:
    //      "string1\0string2\0...\0string_last\0"

    *token_count = count;
    return tokens;
}

static uint64_t count_non_empty_lines(const char *buffer, uint64_t buf_size)
{
    uint64_t lines = 0;
    uint64_t line_length = 0;

    for (uint64_t i = 0; i < buf_size - 1; i++) {
        if (buffer[i] == '\n' || buffer[i] == '\r') {
            if (line_length > 0) {
                lines++;
                line_length = 0;
            }
        } else {
            line_length++;
        }
    }

    if (line_length > 0) {
        lines++;
    }

    return lines;
}
