#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <forward_list>

#include "containers/std_list.h"
#include "common/compare.h"
#include "common/report.h"

StdList *std_list_init()
{
    return new StdList();
}

int std_list_insert(StdList *list, const char *key, int value)
{
    assert(list); assert(key);

    std::forward_list<StdNode>::iterator it;
    
    for (it = list->list.begin(); it != list->list.end(); it++) {
        if (COMPARE_KEYS(it->key, key) == 0) {
            return 1;
        }
    }

    list->list.push_front((StdNode){key, value});
    return 0;
}

int std_list_find(StdList *list, const char *key)
{
    assert(list); assert(key);

    std::forward_list<StdNode>::iterator it;
    
    for (it = list->list.begin(); it != list->list.end(); it++) {
        if (COMPARE_KEYS(it->key, key) == 0) {
            return 1; 
        }
    }

    return 0;
}

int std_list_remove(StdList *list, const char *key)
{
    assert(list); assert(key);

    std::forward_list<StdNode>::iterator prev = list->list.before_begin();
    std::forward_list<StdNode>::iterator curr = list->list.begin();

    while (curr != list->list.end()) {
        if (COMPARE_KEYS(curr->key, key) == 0) {
            list->list.erase_after(prev);
            return 1; 
        }

        prev = curr;
        curr++;
    }

    return 0;
}

int std_list_for_each(StdList *list, action_func action, void *user_data)
{
    assert(list); assert(action);

    std::forward_list<StdNode>::iterator prev = list->list.before_begin();
    std::forward_list<StdNode>::iterator curr = list->list.begin();

    while (curr != list->list.end()) {
        if (action(curr->key, curr->value, user_data) != 0) {
            return -1;
        }

        prev = curr;
        curr++;
    }

    return 0;
}

void std_list_destroy(StdList *bucket) 
{
    if (bucket != NULL) {
        delete bucket;
    }
}
