#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "containers/cf_list.h"
#include "common/compare.h"
#include "common/report.h"

CFList *cf_list_init()
{
    CFList *list = (CFList *)calloc(1, sizeof(CFList));
    if (list == NULL) {
        ERROR("memory allocation error");
        return NULL;
    }

#ifdef DEBUG
    list->debug.creation = (ListCreationInfo){
        "cf_list", __FILE__, __func__, __LINE__
    };
#endif // DEBUG

    if (listConstructor(list) != LIST_OK) {
        ERROR("failed to initialize list");
        free(list);
        return NULL;
    }

    return list;
}

int cf_list_insert(CFList *list, const char *key, int value)
{
    assert(list); assert(key);
    
    int curr = list->storage[0].next;
    while (curr != 0) {
        if (COMPARE_KEYS(list->storage[curr].value.key, key) == 0) {
            return 1;
        }
        curr = list->storage[curr].next;
    }

    DataType new_data = {key, value};

    if (listInsertAfter(list, 0, new_data) != LIST_OK) {
        return -1;
    }

    return 0;
}

int cf_list_find(CFList *list, const char *key, int *result)
{
    assert(list); assert(key);

    int curr = list->storage[0].next;

    while (curr != 0) {
        if (COMPARE_KEYS(list->storage[curr].value.key, key) == 0) {
            if (result) {
                *result = list->storage[curr].value.value;
            }
            return 1;
        }
        curr = list->storage[curr].next;
    }
    
    return 0;
}

int cf_list_remove(CFList *list, const char *key)
{
    assert(list); assert(key);

    int curr = list->storage[0].next;

    while (curr != 0) {
        if (COMPARE_KEYS(list->storage[curr].value.key, key) == 0) {
            if (listDelete(list, curr) != LIST_OK) {
                return -1;
            }
            return 0;
        }
        curr = list->storage[curr].next;
    }

    return 1;
}

int cf_list_for_each(CFList *list, action_func action, void *user_data)
{
    assert(list); assert(action);

    int curr = list->storage[0].next;

    while (curr != 0) {
        if (action(list->storage[curr].value.key, list->storage[curr].value.value, user_data) != 0) {
            return -1;
        }
        curr = list->storage[curr].next;
    }

    return 0;
}

void cf_list_destroy(CFList *list)
{
    assert(list);
    
    listDestructor(list);
    free(list);
}
