#include "linked_list.h"
#include <stdlib.h>
#include <string.h>

#define DEBUG
#include "../debug/debug.h"

void init_ll(struct LinkedList* ll, size_t _obj_size)
{
    ll->size = 0;
    ll->obj_size = _obj_size;
    ll->beginning = NULL;
}

void insert(struct LinkedList* ll, void* _new, size_t index)
{
    if (index > size_ll(ll))
    {
        LOG("Out of index: Linked list is of size %zd at function %s", size_ll(ll), __func__);
        return;
    }

    struct Node* new_node = malloc(sizeof(struct Node));
    if (!new_node)
    {
        LOG("Allocation failure at function %s", __func__);
        return;
    }
    new_node->_data = malloc(ll->obj_size);
    if (!new_node->_data)
    {
        LOG("Allocation failure at function %s", __func__);
        free(new_node);
        return;
    }
    memcpy(new_node->_data, _new, ll->obj_size);

    struct Node** target = &ll->beginning;
    for (size_t n = 0; n < index; n++)
        target = &(*target)->next;
    new_node->next = *target;
    *target = new_node;
    ll->size++;
}

void remove(struct LinkedList* ll, size_t index)
{
    if (index >= size_ll(ll))
    {
        LOG("Out of index: Linked list is of size %zd at function %s", size_ll(ll), __func__);
        return;
    }

    struct Node** target = &ll->beginning;
    for (size_t n = 0; n < index; n++)
        target = &(*target)->next;
    struct Node* del_node = *target;
    *target = del_node->next;
    free(del_node->_data);
    free(del_node);
    ll->size--;
}

void* get_nd(struct LinkedList* ll, size_t index)
{
    if (index >= ll->size)
    {
        LOG("Index %zu out of bounds", index);
        return NULL;
    }

    struct Node* curr = ll->beginning;
    for (size_t i = 0; i < index; ++i)
        curr = curr->next;
    return curr->_data;
}

void free_ll(struct LinkedList* ll)
{
    struct Node* curr = ll->beginning;
    while (curr)
    {
        struct Node* del_node = curr;
        curr = del_node->next;
        free(del_node->_data);
        free(del_node);
    }
    ll->beginning = NULL;
    ll->size = 0;
}