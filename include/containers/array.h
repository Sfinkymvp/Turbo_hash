#ifndef ARRAY_H
#define ARRAY_H

#include "common/types.h"

typedef struct ArrNode ArrNode;
struct ArrNode {
    const char *key;
    int value;
};

typedef struct {
    ArrNode *data;
    int size;
    int capacity;
} DynamicArray;

DynamicArray *array_init();
int array_insert(DynamicArray *array, const char *key, int value);
int array_find(DynamicArray *array, const char *key);
int array_remove(DynamicArray *array, const char *key);
int array_for_each(DynamicArray *array, action_func action, void *user_data);
void array_destroy(DynamicArray *array);

#endif // ARRAY_H
