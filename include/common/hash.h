#ifndef HASH_H
#define HASH_H

#include <stddef.h>
#include <stdint.h>

typedef uint64_t (*hash_function)(const char *key);
typedef int (*equals_function)(const char *key1, const char *key2);

int string_equals_naive(const char *str1, const char *str2);
uint64_t hash_string_crc32_naive(const char *key);
uint64_t hash_string_crc32_intr(const char *key);

#endif // HASH_H