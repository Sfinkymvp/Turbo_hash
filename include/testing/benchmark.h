#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stdint.h>

#include "common/hash.h"

typedef struct Args Args;
typedef struct ChainHashTable ChainHashTable;

extern const double DEFAULT_MAX_LOAD_FACTOR;
extern const uint64_t DEFAULT_LOOKUP_ITERATIONS;
extern const uint64_t DEFAULT_SAMPLE_COUNT;
extern const uint64_t DEFAULT_HT_CAPACITY;
extern const hash_function DEFAULT_HASH_FUNCTION;
extern const equals_function DEFAULT_EQUALS_FUNCTION;

typedef struct {
    char *aligned_pool;
    uint64_t pool_size;

    char **keys;
    uint64_t key_count;

    ChainHashTable *table;

    char *queries_pool;
    char **test_queries;

    uint64_t* lookup_indices;
    uint64_t lookup_iterations;
    uint64_t lookup_time;

    uint64_t collisions;
    uint64_t max_chain_length;

    uint64_t *results;
    uint64_t sample_count;
} BenchmarkContext;

int run_benchmark(BenchmarkContext *context);
int create_benchmark_context(BenchmarkContext *context, Args *args);
void print_results(BenchmarkContext *context);
void destroy_benchmark_context(BenchmarkContext *context);

#endif // BENCHMARK_H
