#ifndef CLASSIC_LIST_H
#define CLASSIC_LIST_H

typedef struct CListNode CListNode;
struct CListNode {
    const char *key;
    int value;
    CListNode *next;
};

typedef struct {
    CListNode *root;
} CList;

CList *classic_list_init();
int classic_list_insert(CList *list, const char *key, int value);
int classic_list_find(CList *list, const char *key);
int classic_list_remove(CList *list, const char *key);
void classic_list_destroy(CList *list);

#endif // CLASSIC_LIST_H
