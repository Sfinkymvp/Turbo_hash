#ifndef IO_H
#define IO_H

#include <stdint.h>

#include "common/hash.h"

typedef struct Args {
    const char *file_path;
    uint64_t lookup_iterations;
    double max_load_factor; 
    uint64_t sample_count;
    hash_function hash_func;
    equals_function equals_func;
} Args;

int parse_args(Args *args, int argc, char *const *argv);

int read_file_to_buffer(char **buffer, uint64_t *buffer_size, const char *file_path);

#endif // IO_H