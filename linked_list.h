#ifndef LINKED_LIST
#define LINKED_LIST

#define size_ll(ll) ((ll)->size)

#include <ctype.h>

struct Node
{
    void* _data;
    struct Node* next;
};

struct LinkedList
{
    size_t size;
    size_t obj_size;
    struct Node* beginning;
};

void init_ll(struct LinkedList* ll, size_t _obj_size);
void insert(struct LinkedList* ll, void* _new, size_t index);
void remove(struct LinkedList* ll, size_t index);
void* get_nd(struct LinkedList* ll, size_t index);
void free_ll(struct LinkedList* ll);

#endif // LINKED_LIST