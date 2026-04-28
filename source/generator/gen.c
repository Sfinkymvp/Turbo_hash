#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "generator/gen.h"

int string_generator(GeneratorConfig *config)
{
    assert(config);

    char *buffer = (char *)calloc(config->range_max + 1, sizeof(char));
    if (buffer == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        return 1;
    }

    int min_len = (int)config->range_min;
    int max_len = (int)config->range_max;
    int modulus = max_len - min_len + 1;

    for (size_t i = 0; i < config->count; i++) {
        int random_len = min_len + rand() % modulus;

        for (int j = 0; j < random_len; j++) {
            buffer[j] = 'a' + rand() % ('z' - 'a' + 1);
        }

        buffer[random_len] = '\0';
        printf("%s\n", buffer);
    }

    free(buffer);
    return 0;
}
