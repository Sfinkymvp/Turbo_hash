#ifndef GEN_H
#define GEN_H

#include <stdint.h>

typedef struct {
    uint64_t count;
    uint64_t range_min;
    uint64_t range_max;
} GeneratorConfig;

int string_generator(GeneratorConfig *config);

#endif // GEN_H