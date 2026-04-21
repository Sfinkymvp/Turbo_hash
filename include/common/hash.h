#ifndef HASH_H
#define HASH_H

#include <stddef.h>
#include <stdint.h>


typedef unsigned int (*hash_function)(const char* key, size_t modulus);
typedef int (*equals_function)(const char* key1, const char* key2);


uint64_t hash_string_crc64_naive(const char* key, uint64_t table_size);


#endif // HASH_H