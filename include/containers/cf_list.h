#ifndef CF_LIST_H
#define CF_LIST_H

#include "common/types.h"
#include "list.h"

typedef List CFList;

CFList *cf_list_init();
int cf_list_insert(CFList *list, const char *key, int value);
int cf_list_find(CFList *list, const char *key, int *result);
int cf_list_remove(CFList *list, const char *key);
int cf_list_for_each(CFList *list, action_func action, void *user_data);
void cf_list_destroy(CFList *list);

#endif // CF_LIST_H
