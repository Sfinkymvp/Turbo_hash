#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <forward_list>

#include "table/cf_list.h"
#include "common/compare.h"
#include "common/report.h"

static const int EMPTY = -1;
static const int DEFAULT_CAPACITY = 8;

static int cf_list_resize(CFList *list);
static void initialize_free_nodes(CFList *list, int start_idx, int end_idx);

CFList *cf_list_init()
{
    CFList *list = (CFList *)calloc(1, sizeof(CFList));
    if (list == NULL) {
        ERROR("memory allocation error");
        return NULL;
    }

    list->storage = (CFNode *)calloc(DEFAULT_CAPACITY, sizeof(CFNode));
    if (list->storage == NULL) {
        ERROR("memory allocation error");
        free(list);
        return NULL;
    }

    list->capacity = DEFAULT_CAPACITY;
    list->size = 0;
    list->head = EMPTY;
    list->free_head = 0;

    return list;
}

int cf_list_insert(CFList *list, const char *key, int value)
{
    assert(list); assert(key);

    int curr = list->head;
    while (curr != EMPTY) {
        if (COMPARE_KEYS(list->storage[curr].key, key) == 0) {
            return 1;
        }
        curr = list->storage[curr].next;
    }

    if (list->free_head == EMPTY) {
        if (cf_list_resize(list) != 0) {
            return -1;
        }
    }

    int new_idx = list->free_head;
    list->free_head = list->storage[list->free_head].next;
    list->storage[new_idx].key = key;
    list->storage[new_idx].value = value;

    list->storage[new_idx].next = list->head;
    list->head = new_idx;

    return 0;
}

int cf_list_find(CFList *list, const char *key)
{
    assert(list);

    int curr = list->head;
    while (curr != EMPTY) {
        if (COMPARE_KEYS(list->storage[curr].key, key) == 0) {
            return 1;
        }
        curr = list->storage[curr].next;
    }
    
    return 0;
}

int cf_list_remove(CFList *list, const char *key)
{
    assert(list);

    int prev = EMPTY;
    int curr = list->head;

    while (curr != EMPTY) {
        if (COMPARE_KEYS(list->storage[curr].key, key) == 0) {
            break;
        }

        prev = curr;
        curr = list->storage[curr].next;
    }

    if (curr == EMPTY) {
        return 1;
    }

    if (prev == EMPTY) {
        list->head = list->storage[curr].next;
    } else {
        list->storage[prev].next = list->storage[curr].next;
    }

    list->storage[curr].key = NULL;
    list->storage[curr].value = 0;

    list->storage[curr].next = list->free_head;
    list->free_head = curr;

    return 0;
}

void cf_list_destroy(CFList *list)
{
    assert(list);

    free(list->storage);
    free(list);
}

static int cf_list_resize(CFList *list)
{
    assert(list); assert(list->storage); assert(list->capacity > 0);

    if (list->free_head != EMPTY) {
        assert(list->size != list->capacity);
        return 0;
    }

    int new_capacity = 2 * list->capacity;
    CFNode *temp = (CFNode *)realloc(list->storage, new_capacity * sizeof(CFNode));
    if (temp == NULL) {
        ERROR("cache-friendly list resize error");
        return -1;
    }

    list->storage = temp;
    list->free_head = list->capacity;
    list->capacity = new_capacity;

    initialize_free_nodes(list, list->capacity / 2, list->capacity - 1);
    return 0;
}

static void initialize_free_nodes(CFList *list, int start_idx, int end_idx)
{
    assert(list); assert(list->storage);

    int curr = list->free_head;
    if (curr != EMPTY) {
        while (list->storage[curr].next != EMPTY) {
            curr = list->storage[curr].next;
        }
    }

    for (int i = start_idx; i <= end_idx; i++) {
        list->storage[i].key = NULL;
        list->storage[i].value = 0;
        list->storage[i].next = i + 1;
    }
    list->storage[end_idx].next = EMPTY;

    if (curr == EMPTY) {
        list->free_head = start_idx;
    } else {
        list->storage[curr].next = start_idx;
    }
}

