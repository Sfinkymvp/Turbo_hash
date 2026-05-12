#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <forward_list>

#include "containers/classic_list.h"
#include "common/compare.h"
#include "common/report.h"

CList *classic_list_init()
{
    CList *list = (CList *)calloc(1, sizeof(CList));
    if (list == NULL) {
        ERROR("memory allocation error");
        return NULL;
    }

    list->root = NULL;

    return list;
}

int classic_list_insert(CList *list, const char *key, int value)
{
    assert(list); assert(key);

    CNode *curr = list->root;
    while (curr != NULL) {
        if (COMPARE_KEYS(curr->key, key) == 0) {
            return 1;
        }
        curr = curr->next;
    }

    CNode *new_node = (CNode *)calloc(1, sizeof(CNode));
    if (new_node == NULL) {
        ERROR("memory allocation error");
        return -1;
    }

    new_node->key = key;
    new_node->value = value;

    new_node->next= list->root;
    list->root = new_node;

    return 0;
}

int classic_list_find(CList *list, const char *key, int *result)
{
    assert(list); assert(key);

    CNode *curr = list->root;
    while (curr != NULL) {
        if (COMPARE_KEYS(curr->key, key) == 0) {
            if (result) {
                *result = curr->value;
            }
            return 1;
        }
        curr = curr->next;
    }

    return 0;
}

int classic_list_remove(CList *list, const char *key)
{
    assert(list);

    CNode *prev = NULL;
    CNode *curr = list->root;
    while (curr != NULL) {
        if (COMPARE_KEYS(curr->key, key) == 0) {
            break;
        }
        prev = curr;
        curr = curr->next;
    }

    if (curr == NULL) {
        return 1;
    }

    if (prev == NULL) {
        list->root = curr->next;
    } else {
        prev->next = curr->next;
    }
    free(curr);

    return 0;
}

int classic_list_for_each(CList *list, action_func action, void *user_data)
{
    assert(list); assert(action);

    CNode *curr = list->root;
    while (curr != NULL) {
        if (action(curr->key, curr->value, user_data) != 0) {
            return -1;
        }

        curr = curr->next;
    }

    return 0;
}

void classic_list_destroy(CList *list)
{
    assert(list);

    CNode *deleted = NULL;
    CNode *curr = list->root;
    while (curr != NULL) {
        deleted = curr;
        curr = curr->next;
        free(deleted);
    }

    free(list);
}
