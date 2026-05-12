#ifndef STD_LIST_H
#define STD_LIST_H

typedef struct {
    const char *key;
    int value;
} StdNode;

typedef struct {
    std::forward_list<StdNode> list;
} StdList;

StdList *std_list_init();
int std_list_insert(StdList *list, const char *key, int value);
int std_list_find(StdList *list, const char *key);
int std_list_remove(StdList *list, const char *key);
void std_list_destroy(StdList *bucket);

#endif // STD_LIST_H
