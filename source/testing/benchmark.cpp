#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <immintrin.h>
#include <AMDProfileController.h>
#include <assert.h>

#include "testing/benchmark.h"
#include "testing/io.h"
#include "common/report.h"
#include "table/table.h"

const double DEFAULT_MAX_LOAD_FACTOR = 10.0;
const uint64_t DEFAULT_LOOKUP_ITERATIONS = 10UL;
const uint64_t DEFAULT_SAMPLE_COUNT = 10UL;
const uint64_t DEFAULT_HT_CAPACITY = 64UL;

// Максимальный размер должен быть кратен степени двойки (16, 32, 64)
const uint64_t KEYWORD_MAX_SIZE = 32UL;

#if defined(HASH_INTR)
    const hash_function DEFAULT_HASH_FUNCTION = hash_string_crc32_intr;
#else 
    const hash_function DEFAULT_HASH_FUNCTION = hash_string_crc32_naive;
#endif // HASH_INTR

const equals_function DEFAULT_EQUALS_FUNCTION = string_equals_naive;

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define BENCHMARK_ASSERT(context_ptr) \
    assert((context_ptr)->aligned_pool); assert((context_ptr)->keys); CHAIN_HT_ASSERT((context_ptr)->table); \
    assert((context_ptr)->lookup_indices); assert((context_ptr)->test_queries); \
    assert((context_ptr)->results);
   
static uint64_t run_lookup_sample(BenchmarkContext *context);
static int fill_hash_table(BenchmarkContext *context);
static void collect_table_stats(BenchmarkContext* context);
static uint64_t *generate_shuffled_indices(uint64_t count, uint64_t modulus);
static int generate_random_queries(BenchmarkContext *context, uint64_t *random_indices);
static int tokenize_buffer(BenchmarkContext *context, char *buffer, uint64_t buf_size);
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

    uint64_t start_tick = __rdtsc();
    for (uint64_t i = 0; i < context->lookup_iterations; i++) {
        chain_ht_find(context->table, context->test_queries[i], NULL);
    }
    uint64_t end_tick = __rdtsc();

    return end_tick - start_tick;
}

int create_benchmark_context(BenchmarkContext *context, Args *args)
{
    assert(context);
    assert(args);

    char *buffer = NULL;
    uint64_t buffer_size = 0;
    int status = read_file_to_buffer(&buffer, &buffer_size, args->file_path);
    if (status != 0) {
        destroy_benchmark_context(context);
        return status;
    }

    status = tokenize_buffer(context, buffer, buffer_size);
    if (status != 0) {
        destroy_benchmark_context(context);
        return status;
    }

    free(buffer);

    context->lookup_iterations = args->lookup_iterations;
    uint64_t *random_indices = generate_shuffled_indices(context->lookup_iterations, context->key_count);
    if (random_indices == NULL) {
        destroy_benchmark_context(context);
        return -1;
    }

    status = generate_random_queries(context, random_indices);
    if (status != 0) {
        destroy_benchmark_context(context);
        return status;
    }
    free(random_indices);

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

    free(context->aligned_pool);
    free(context->keys);
    free(context->lookup_indices);
    free(context->queries_pool);
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

    (void)context;
}

static uint64_t *generate_shuffled_indices(uint64_t count, uint64_t modulus)
{
    uint64_t *indices = (uint64_t *)calloc(count, sizeof(uint64_t));
    if (indices == NULL) {
        ERROR("Memory allocation error");
        return NULL;
    }

    for (uint64_t i = 0; i < count; i++) {
        indices[i] = rand() % modulus;
    }

    return indices;
}

static int generate_random_queries(BenchmarkContext *context, uint64_t *random_indices)
{
    assert(context);

    uint64_t iters = context->lookup_iterations;

    char **test_queries = (char **)calloc(iters, sizeof(char *));
    if (test_queries == NULL) {
        ERROR("memory allocation error");
        return -1;
    }

    char *queries_pool = (char *)aligned_alloc(KEYWORD_MAX_SIZE, iters * KEYWORD_MAX_SIZE);
    if (queries_pool == NULL) {
        ERROR("memory allocation error");
        free(test_queries);
        return -1;
    }

    memset(queries_pool, 0, iters * KEYWORD_MAX_SIZE);

    for (uint64_t i = 0; i < iters; i++) {
        uint64_t index = random_indices[i];
        char *dest = &queries_pool[i * KEYWORD_MAX_SIZE];
        char *src = context->keys[index];

        if (index % 2 == 0) {
            memcpy(dest, src, KEYWORD_MAX_SIZE);
        } else {
            memcpy(dest, src + 1, KEYWORD_MAX_SIZE - 1);
        }

        test_queries[i] = dest;
    }

    context->queries_pool = queries_pool;
    context->test_queries = test_queries;
    return 0;
}

static int tokenize_buffer(BenchmarkContext *context, char *buffer, uint64_t buf_size)
{
    assert(context); assert(buffer); 

    uint64_t count = count_non_empty_lines(buffer, buf_size);
    char **tokens = (char **)calloc(count, sizeof(char *));
    if (tokens == NULL) {
        ERROR("memory allocation error");
        return -1;
    }
    
    char *aligned_pool = (char *)aligned_alloc(KEYWORD_MAX_SIZE, count * KEYWORD_MAX_SIZE);
    memset(aligned_pool, 0, count * KEYWORD_MAX_SIZE);

    uint64_t token_idx = 0;
    uint64_t current_len = 0;
    char *current_start = buffer;

    // buf_size > 0 на этом этапе, buffer[buf_size - 1] = \0
    for (uint64_t i = 0; i < buf_size; i++) {
        if (buffer[i] == '\n' || buffer[i] == '\r' || buffer[i] == '\0') {
            if (current_len > 0) {
                memcpy(&aligned_pool[token_idx * KEYWORD_MAX_SIZE], current_start, current_len);
                tokens[token_idx] = &aligned_pool[token_idx * KEYWORD_MAX_SIZE];
                token_idx++;
                current_len = 0;
            }
        } else {
            if (current_len == 0) {
                current_start = &buffer[i];
            }
            current_len++;
        }
    }

    context->key_count = count;
    context->keys = tokens;
    context->aligned_pool = aligned_pool;

    return 0;
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
