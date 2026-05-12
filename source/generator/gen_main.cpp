#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <getopt.h>

#include "generator/gen.h"

static const int ARGS_COUNT = 7;

static int parse_args(GeneratorConfig *config, int argc, char *const *argv);

int main(int argc, char *const *argv)
{
    srand(time(NULL));

    GeneratorConfig config = {};
    int status = 0;

    status = parse_args(&config, argc, argv);
    if (status != 0){ 
        return status;
    }

    status = string_generator(&config);
    if (status != 0) {
        return status;
    }

    return status;
}

static int parse_args(GeneratorConfig *config, int argc, char *const *argv)
{
    assert(config);
    assert(argv);

    if (argc != ARGS_COUNT) {
        fprintf(stderr, "Invalid args count (Count: %d)\n", argc);
        fprintf(stderr, "Usage: %s <count> <from> <to>\n", argv[0]);
        return 1;
    }

    char *endptr = NULL;
    int opt = 0;
    while ((opt = getopt(argc, argv, "c:f:t:")) != -1) {
        switch (opt) {
            case 'c': {
                config->count = strtoul(optarg, &endptr, 10);
                if (*endptr != '\0') {
                    fprintf(stderr, "Invalid count: %s\n", optarg);
                    return 1;
                }
                break;
            }
            case 'f': {
                config->range_min = strtoul(optarg, &endptr, 10);
                if (*endptr != '\0') {
                    fprintf(stderr, "Invalid min range: %s\n", optarg);
                    return 1;
                }
                break;
            }
            case 't': {
                config->range_max = strtoul(optarg, &endptr, 10);
                if (*endptr != '\0') {
                    fprintf(stderr, "Invalid max range: %s\n", optarg);
                    return 1;
                }
                break;
            }
        }
    }

    if (config->range_min > config->range_max) {
        fprintf(stderr, "Invalid range\n");
        return 1;
    }

    return 0;
}
