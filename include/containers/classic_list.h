#ifndef CLASSIC_LIST_H
#define CLASSIC_LIST_H

#include "common/types.h"

typedef struct CNode CNode;
struct CNode {
    const char *key;
    int value;
    CNode *next;
};

typedef struct {
    CNode *root;
} CList;

CList *classic_list_init();
int classic_list_insert(CList *list, const char *key, int value);
int classic_list_find(CList *list, const char *key);
int classic_list_remove(CList *list, const char *key);
int classic_list_for_each(CList *list, action_func action, void *user_data);
void classic_list_destroy(CList *list);

#endif // CLASSIC_LIST_H
