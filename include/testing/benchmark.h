#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stdint.h>

typedef struct Args Args;
typedef struct ChainHashTable ChainHashTable;

extern const double DEFAULT_MAX_LOAD_FACTOR;
extern const uint64_t DEFAULT_LOOKUP_ITERATIONS;
extern const uint64_t DEFAULT_HT_CAPACITY;

typedef struct {
    char *buffer;
    uint64_t buf_size;
    char **keys;
    uint64_t key_count;

    ChainHashTable *table;

    uint64_t lookup_iterations;
    uint64_t lookup_time;
    uint64_t collisions;
    uint64_t max_chain_length;
} BenchmarkContext;

int run_benchmark(BenchmarkContext *context);
int create_benchmark_context(BenchmarkContext *context, Args *args);
void destroy_benchmark_context(BenchmarkContext *context);

#endif // BENCHMARK_H