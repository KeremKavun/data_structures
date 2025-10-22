#include "../../include/dbly_linked_list.h"
#include "../../../debug/include/debug.h"

static struct dbly_list_item* dbly_list_find_helper
(
    struct dbly_linked_list* dll,
    struct dbly_list_item* entry,
    struct dbly_list_item* (*it) (struct dbly_list_item* dli),
    void* userdata,
    int (*cmp) (struct dbly_list_item* item, void* data)
);

static struct dbly_list_item* dbly_list_walk_helper
(
    struct dbly_linked_list* dll,
    struct dbly_list_item* entry,
    struct dbly_list_item* (*it) (struct dbly_list_item* dli),
    void* userdata,
    void (*handler) (struct dbly_list_item* item, void* data)
);

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

void dbly_list_insert(struct dbly_linked_list* dll, struct dbly_list_item* pos, struct dbly_list_item* new_item)
{
    if (pos == dll->head)
        dbly_list_insert_front(dll, new_item);
    else if (pos == NULL)
        dbly_list_insert_back(dll, new_item);
    else
    {
        new_item->next = pos;
        new_item->prev = pos->prev;

        if (pos->prev)
            pos->prev->next = new_item;
        pos->prev = new_item;
        dll->size++;
    }
}

void dbly_list_insert_front(struct dbly_linked_list* dll, struct dbly_list_item* new_item)
{
    new_item->prev = NULL;

    if (dll->head)
    {
        dll->head->prev = new_item;
        new_item->next = dll->head;
    }
    else
    {
        dll->tail = new_item;
        new_item->next = NULL;
    }
    dll->head = new_item;
    dll->size++;
}

void dbly_list_insert_back(struct dbly_linked_list* dll, struct dbly_list_item* new_item)
{
    new_item->next = NULL;

    if (dll->tail)
    {
        dll->tail->next = new_item;
        new_item->prev = dll->tail;
    }
    else
    {
        dll->head = new_item;
        new_item->prev = NULL;
    }
    dll->tail = new_item;
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
    if (!dll->head)
        return NULL;
    return dbly_list_remove(dll, dll->head);
}

struct dbly_list_item* dbly_list_remove_back(struct dbly_linked_list* dll)
{
    if (!dll->tail)
        return NULL;
    return dbly_list_remove(dll, dll->tail);
}

struct dbly_list_item* dbly_list_head(struct dbly_linked_list* dll)
{
    return dll->head;
}

struct dbly_list_item* dbly_list_tail(struct dbly_linked_list* dll)
{
    return dll->tail;
}

struct dbly_list_item* dbly_list_find_front(struct dbly_linked_list* dll, void* userdata, int (*cmp) (struct dbly_list_item* item, void* data))
{
    return dbly_list_find_helper(dll, dll->head, dbly_list_item_next, userdata, cmp);
}

struct dbly_list_item* dbly_list_find_back(struct dbly_linked_list* dll, void* userdata, int (*cmp) (struct dbly_list_item* item, void* data))
{
    return dbly_list_find_helper(dll, dll->tail, dbly_list_item_prev, userdata, cmp);
}

int dbly_list_empty(struct dbly_linked_list* dll)
{
    return (dll->size == 0);
}

size_t dbly_list_size(struct dbly_linked_list* dll)
{
    return dll->size;
}

void dbly_list_walk_front(struct dbly_linked_list* dll, void* userdata, void (*handler) (struct dbly_list_item* item, void* userdata))
{
    return dbly_list_walk_helper(dll, dll->head, dbly_list_item_next, userdata, handler);
}

void dbly_list_walk_back(struct dbly_linked_list* dll, void* userdata, void (*handler) (struct dbly_list_item* item, void* userdata))
{
    return dbly_list_walk_helper(dll, dll->tail, dbly_list_item_prev, userdata, handler);
}

void dbly_list_reverse(struct dbly_linked_list* dll)
{
    struct dbly_list_item* curr = dll->head;
    while (curr)
    {
        struct dbly_list_item* next = curr->next;
        curr->next = curr->prev;
        curr->prev = next;
        curr = next;
    }
    struct dbly_list_item* temp = dll->head;
    dll->head = dll->tail;
    dll->tail = temp;
}

// *** Helper functions *** //

static struct dbly_list_item* dbly_list_find_helper
(
    struct dbly_linked_list* dll,
    struct dbly_list_item* entry,
    struct dbly_list_item* (*it) (struct dbly_list_item* dli),
    void* data,
    int (*cmp) (struct dbly_list_item* item, void* data)
)
{
    while (entry) {
        if (cmp(entry, data))
            return entry;
        entry = it(entry);
    }
    return NULL;
}

static struct dbly_list_item* dbly_list_walk_helper
(
    struct dbly_linked_list* dll,
    struct dbly_list_item* entry,
    struct dbly_list_item* (*it) (struct dbly_list_item* dli),
    void* userdata,
    void (*handler) (struct dbly_list_item* item, void* data)
)
{
    while (entry)
    {
        handler(it, userdata);
        entry = it(entry);
    }
}