#ifndef HASH_H
#define HASH_H

#include <stddef.h>
#include <stdint.h>

typedef uint64_t (*hash_function)(const char *key, uint64_t modulus);
typedef int (*equals_function)(const char *key1, const char *key2);

int string_equals(const char* str1, const char* str2);
uint64_t hash_string_crc64_naive(const char *key, uint64_t modulus);

#endif // HASH_H