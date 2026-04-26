#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <sys/stat.h>
#include <getopt.h>
#include <assert.h>

#include "testing/io.h"
#include "common/report.h"
#include "testing/benchmark.h"

static uint64_t get_file_size(FILE *file);

int parse_args(Args *args, int argc, char *const *argv)
{
    assert(args);
    assert(argv);

    args->max_load_factor = DEFAULT_MAX_LOAD_FACTOR;
    args->file_path = NULL;
    args->lookup_iterations = DEFAULT_LOOKUP_ITERATIONS;

    char *end_ptr = NULL;
    int opt = 0;
    while ((opt = getopt(argc, argv, "l:f:i:")) != -1) {
        switch (opt) {
            case 'l': {
                args->max_load_factor = strtod(optarg, &end_ptr);
                if (*end_ptr != '\0') {
                    REPORT(stderr, "Invalid max load factor");
                    return 1;
                }
                break;
            }
            case 'f': {
                args->file_path = optarg;
                break;
            } case 'i': {
                args->lookup_iterations = strtoull(optarg, &end_ptr, 10);
                if (*end_ptr != '\0') {
                    REPORT(stderr, "invalid iteration count");
                    return 1;
                }
                break;
            }
            default: {
                break;
            }
        }
    }

    if (args->file_path) {
        FILE *file = fopen(args->file_path, "r");
        if (file == NULL) {
            REPORT(stderr, "Error opening file %s: %s", args->file_path, strerror(errno));
            return 1;
        }
        fclose(file);
    } else {
        REPORT(stderr, "Option -f (file) is required");
        return 1;
    }

    return 0;
}

int read_file_to_buffer(char **buffer, uint64_t *buffer_size, const char *file_path)
{
    assert(buffer);
    assert(file_path);

    char *temp = NULL;
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        REPORT(stderr, "Error while working with file '%s'", file_path);
        return 1;
    }

    uint64_t file_size = get_file_size(file);
    if (file_size == 0) {
        REPORT(stderr, "File '%s' is empty");
        fclose(file);
        return 1;
    }

    temp = (char *)calloc(file_size + 1, sizeof(char));
    if (temp == NULL) {
        REPORT(stderr, "Memory allocation error");
        fclose(file);
        return 1;
    }

    uint64_t read_count = fread(temp, sizeof(char), file_size, file);
    fclose(file);

    if (read_count != file_size) {
        REPORT(stderr, "File '%s' reading error");
        free(temp);
        return 1;
    }
    
    temp[file_size] = '\0';
    *buffer = temp;
    *buffer_size = file_size + 1;
    return 0;
}


static uint64_t get_file_size(FILE *file)
{
    assert(file != NULL);

    struct stat buf = {};
    if (fstat(fileno(file), &buf) == -1)
        return 0;

    return (uint64_t)buf.st_size;
}

