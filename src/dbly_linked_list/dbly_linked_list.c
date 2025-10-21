#include "../../include/dbly_linked_list.h"
#include "../../../debug/include/debug.h"

// *** dbly_list_item implementation *** //

void dbly_list_item_init(struct dbly_list_item* dli, void* data)
{
    dli->prev = NULL;
    dli->next = NULL;
    dli->data = data;
}

struct dbly_list_item* dbly_list_item_prev(struct dbly_list_item* dli)
{
    return dli->prev;
}

struct dbly_list_item* dbly_list_item_next(struct dbly_list_item* dli)
{
    return dli->next;
}

void* dbly_list_item_data(struct dbly_list_item* dli)
{
    return dli->data;
}

// *** dbly_linked_list implementation *** //

void dbly_list_init(struct dbly_linked_list* dll)
{
    dll->head = NULL;
    dll->tail = NULL;
    dll->size = 0;
}

/* Insert before the pointer-to-item 'pos' (pos is a pointer to a struct dbly_list_item*). */
void dbly_list_insert(struct dbly_linked_list* dll, struct dbly_list_item* pos, struct dbly_list_item* new_item)
{
    new_item->next = pos;
    new_item->prev = (pos ? pos->prev : NULL);

    if (pos)
    {
        if (pos->prev)
            pos->prev->next = new_item;
        else
            dll->head = new_item;
        pos->prev = new_item;
    }
    else
    {

    }

    dll->size++;
}

void dbly_list_insert_front(struct dbly_linked_list* dll, struct dbly_list_item* new_item)
{
    struct dbly_list_item** head = dbly_list_head(dll);
    new_item->prev = NULL;
    new_item->next = *head;

    if (*head)
        (*head)->prev = new_item;
    else
    {
        *head = new_item;
        if (dll->tail == NULL)
            dll->tail = new_item;
    }
    dll->size++;
}

void dbly_list_insert_back(struct dbly_linked_list* dll, struct dbly_list_item* new_item)
{
    struct dbly_list_item** tail = dbly_list_tail(dll);
    new_item->next = NULL;
    if (*tail) {
        (*tail)->next = new_item;
        new_item->prev = *tail;
        *tail = new_item;
    } else {
        *tail = new_item;
        new_item->prev = NULL;
        if (dll->head == NULL)
            dll->head = new_item;
    }
    dll->tail = *tail;
    dll->size++;
}

struct dbly_list_item* dbly_list_remove(struct dbly_linked_list* dll, struct dbly_list_item* dli)
{
    struct dbly_list_item* del = dli;

    if (dli->prev)
        dli->prev->next = del->next;
    else
        dll->head = del->next;

    if (dli->next)
        dli->next->prev = del->prev;
    else
        dll->tail = del->prev;

    del->prev = del->next = NULL;
    dll->size--;
    return del;
}

struct dbly_list_item* dbly_list_remove_front(struct dbly_linked_list* dll)
{
    struct dbly_list_item* head = *dbly_list_head(dll);
    if (!head) return NULL;
    return dbly_list_remove(dll, head);
}

struct dbly_list_item* dbly_list_remove_back(struct dbly_linked_list* dll)
{
    struct dbly_list_item* tail = *dbly_list_tail(dll);
    if (!tail) return NULL;
    return dbly_list_remove(dll, tail);
}

struct dbly_list_item** dbly_list_head(struct dbly_linked_list* dll)
{
    return &dll->head;
}

struct dbly_list_item** dbly_list_tail(struct dbly_linked_list* dll)
{
    return &dll->tail;
}

struct dbly_list_item** dbly_list_find(struct dbly_linked_list* dll, void* data, int (*cmp) (struct dbly_list_item* item, void* data))
{
    struct dbly_list_item** cur = &dll->head;
    while (*cur) {
        if (cmp(*cur, data))
            return cur;
        cur = &(*cur)->next;
    }
    return NULL;
}

int dbly_list_empty(struct dbly_linked_list* dll)
{
    return (dll->size == 0);
}

size_t dbly_list_size(struct dbly_linked_list* dll)
{
    return dll->size;
}

void dbly_list_walk(struct dbly_linked_list* dll, void* userdata, void (*handler) (struct dbly_list_item* item, void* userdata))
{
    struct dbly_list_item* it = dll->head;
    while (it) {
        handler(it, userdata);
        it = it->next;
    }
}

void dbly_list_reverse(struct dbly_linked_list* dll)
{
    struct dbly_list_item* curr = dll->head;
    struct dbly_list_item* tmp = NULL;

    while (curr) {
        tmp = curr->prev;
        curr->prev = curr->next;
        curr->next = tmp;
        curr = curr->prev;
    }

    /* swap head and tail */
    tmp = dll->head;
    dll->head = dll->tail;
    dll->tail = tmp;
}