#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "containers/array.h"
#include "common/compare.h"
#include "common/report.h"

static const int DEFAULT_CAPACITY = 8;

static int array_resize(DynamicArray *array);

DynamicArray *array_init()
{
    DynamicArray *array = (DynamicArray *)calloc(1, sizeof(DynamicArray));
    if (array == NULL) {
        ERROR("memory allocation error");
        return NULL;
    }

    array->data = (ArrNode *)calloc(DEFAULT_CAPACITY, sizeof(ArrNode));
    if (array->data == NULL) {
        ERROR("memory allocation error");
        free(array);
        return NULL;
    }

    array->size = 0;
    array->capacity = DEFAULT_CAPACITY;

    return array;
}

int array_insert(DynamicArray *array, const char *key, int value)
{
    assert(array);

    for (int i = 0; i < array->size; i++) {
        if (COMPARE_KEYS(array->data[i].key, key) == 0) {
            return 1;
        }
    }

    if (array->size == array->capacity) {
        if (array_resize(array) != 0) {
            return -1;
        }
    }

    array->data[array->size].key = key;
    array->data[array->size].value = value;
    array->size++;
    return 0;
}

int array_find(DynamicArray *array, const char *key, int *result)
{
    assert(array); assert(key);

    for (int i = 0; i < array->size; i++) {
        if (COMPARE_KEYS(array->data[i].key, key) == 0) {
            if (result) {
                *result = array->data[i].value;
            }
            return 1;
        }
    }

    return 0;
}

int array_remove(DynamicArray *array, const char *key)
{
    assert(array);
   
    int i = 0;
    for (i = 0; i < array->size; i++) {
        if (COMPARE_KEYS(array->data[i].key, key) == 0) {
            break;
        }
    }

    if (i == array->size) {
        return 1;
    }

    if (array->size == 1 || i == array->size - 1) {
        array->data[i].key = NULL;
        array->data[i].value = 0;
    } else {
        array->data[i].key = array->data[array->size - 1].key;
        array->data[i].value = array->data[array->size - 1].value;
    }
    array->size--;

    return 0;
}

void array_destroy(DynamicArray *array)
{
    assert(array);

    free(array->data);
    free(array);
}

int array_for_each(DynamicArray *array, action_func action, void *user_data)
{
    assert(array); assert(action);

    for (int curr = 0; curr < array->size; curr++) {
        if (action(array->data[curr].key, array->data[curr].value, user_data) != 0) {
            return -1;
        }
    }

    return 0;
}

static int array_resize(DynamicArray *array)
{
    assert(array); assert(array->data); assert(array->capacity > 0);

    int new_capacity = 2 * array->capacity;
    ArrNode *temp = (ArrNode *)realloc(array->data, new_capacity * sizeof(ArrNode));
    if (temp == NULL) {
        ERROR("memory allocation error");
        return -1;
    }

    array->data = temp;
    array->capacity = new_capacity;
    return 0;
}
