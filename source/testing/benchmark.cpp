#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <immintrin.h>
// #include <ittnotify.h>
#include <AMDProfileController.h>
#include <assert.h>

#include "testing/benchmark.h"
#include "testing/io.h"
#include "common/report.h"
#include "table/table.h"

const double DEFAULT_MAX_LOAD_FACTOR = 10.0;
const uint64_t DEFAULT_LOOKUP_ITERATIONS = 10ULL;
const uint64_t DEFAULT_HT_CAPACITY = 64ULL;
const hash_function DEFAULT_HASH_FUNCTION = hash_string_crc32_naive;
const equals_function DEFAULT_EQUALS_FUNCTION = string_equals_naive;

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define BENCHMARK_ASSERT(context_ptr) \
    assert((context_ptr)->buffer); assert((context_ptr)->keys); CHAIN_HT_ASSERT((context_ptr)->table); \
    assert((context_ptr)->lookup_indices); assert((context_ptr)->test_queries); \
    assert((context_ptr)->results);
   
static uint64_t run_lookup_sample(BenchmarkContext *context);
static int fill_hash_table(BenchmarkContext *context);
static void collect_table_stats(BenchmarkContext* context);
static uint64_t *generate_shuffled_indices(uint64_t count, uint64_t modulus);
static char** generate_random_queries(char **keys, uint64_t *indices, uint64_t count);
static char **tokenize_buffer(char *buffer, uint64_t buf_size, uint64_t *token_count);
static uint64_t count_non_empty_lines(const char *buffer, uint64_t buf_size);

int run_benchmark(BenchmarkContext *context)
{
    BENCHMARK_ASSERT(context);
    INFO("started benchmark function");

    int status = fill_hash_table(context);
    if (status != 0) {
        ERROR("Error filling the hash table");
        return status;
    }
    INFO("filled hash table");

    DEBUG("table: cap - %lu; size - %lu",
        context->table->capacity, context->table->size);
    DEBUG("table: max_load_factor - %g", context->table->max_load_factor);
    DEBUG("lookup iters - %lu", context->lookup_iterations);

    collect_table_stats(context);
    INFO("collected table stats");

    DEBUG("collisions - %lu; max_chain_length - %lu", context->collisions,
        context->max_chain_length);
    DEBUG("current load factor: %lf", (double)context->table->size / context->table->capacity);

    DEBUG("key count: %lu", context->key_count);
    // Прогрев кешей
    run_lookup_sample(context);

    // Основной тест
    for (uint64_t i = 0; i < context->sample_count; i++) {
        amdProfileResume();
        context->results[i] = run_lookup_sample(context);
        amdProfilePause();
    }

    return 0;
}

static uint64_t run_lookup_sample(BenchmarkContext *context)
{
    BENCHMARK_ASSERT(context);

    int result = 0;
    uint64_t start_tick = __rdtsc();
    for (uint64_t i = 0; i < context->lookup_iterations; i++) {
        chain_ht_find(context->table, context->test_queries[i], &result);
    }
    uint64_t end_tick = __rdtsc();

    return end_tick - start_tick;
}

int create_benchmark_context(BenchmarkContext *context, Args *args)
{
    assert(context);
    assert(args);

    int status = read_file_to_buffer(&context->buffer, &context->buf_size, args->file_path);
    if (status != 0) {
        destroy_benchmark_context(context);
        return 1;
    }
    context->keys = tokenize_buffer(context->buffer, context->buf_size, &context->key_count);
    if (context->keys == NULL) {
        ERROR("Memory allocation error");
        destroy_benchmark_context(context);
        return 1;
    }

    context->lookup_iterations = args->lookup_iterations;
    context->lookup_indices = generate_shuffled_indices(context->lookup_iterations, context->key_count);
    if (context->lookup_indices == NULL) {
        ERROR("Memory allocation error");
        destroy_benchmark_context(context);
        return 1;
    }

    context->test_queries = generate_random_queries(context->keys, 
        context->lookup_indices, context->lookup_iterations);
    if (context->test_queries == NULL) {
        ERROR("Memory allocation error");
        destroy_benchmark_context(context);
        return 1;
    }

    context->sample_count = args->sample_count;
    context->results = (uint64_t *)calloc(context->sample_count, sizeof(uint64_t));
    if (context->results == NULL) {
        ERROR("Memory allocation error");
        destroy_benchmark_context(context);
        return 1;
    }

    context->table = chain_ht_create(DEFAULT_HT_CAPACITY, args->max_load_factor,
        args->hash_func, args->equals_func);
    if (context->table == NULL) {
        destroy_benchmark_context(context);
        return 1;
    }

    context->lookup_iterations = args->lookup_iterations;
    context->lookup_time = 0;
    context->collisions = 0;
    context->max_chain_length = 0;

    return 0;
}

void print_results(BenchmarkContext *context)
{
    BENCHMARK_ASSERT(context);

    for (uint64_t i = 0; i < context->sample_count; i++) {
        printf("%lu\n", context->results[i]);
    }
}

void destroy_benchmark_context(BenchmarkContext *context)
{
    assert(context);

    free(context->buffer);
    free(context->keys);
    free(context->lookup_indices);
    free(context->test_queries);
    free(context->results);
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

        if (chain_length > 0) {
            context->collisions += chain_length - 1;
        }
        context->max_chain_length = MAX(context->max_chain_length, chain_length);
    }
}

static uint64_t *generate_shuffled_indices(uint64_t count, uint64_t modulus)
{
    uint64_t *indices = (uint64_t *)calloc(count, sizeof(uint64_t));
    if (indices == NULL) {
        return NULL;
    }

    for (uint64_t i = 0; i < count; i++) {
        indices[i] = rand() % modulus;
    }

    return indices;
}

static char** generate_random_queries(char **keys, uint64_t *indices, uint64_t count)
{
    assert(keys);
    assert(indices);

    char **random_queries = (char **)calloc(count, sizeof(char *));
    if (random_queries == NULL) {
        return NULL;
    }

    for (uint64_t i = 0; i < count; i++) {
        uint64_t index = indices[i];
        random_queries[i] = keys[index] + index % 2;
    }

    return random_queries;
}

static char **tokenize_buffer(char *buffer, uint64_t buf_size, uint64_t *token_count)
{
    assert(buffer);
    assert(buf_size);

    uint64_t count = count_non_empty_lines(buffer, buf_size);
    char* *tokens = (char**)calloc(count, sizeof(char*));
    if (tokens == NULL) {
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
