#ifndef LINKED_LIST
#define LINKED_LIST

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#define size_ll(ll) ((ll)->size)

struct list_item
{
    void* data;
    struct list_item* next;
};

struct linked_list
{
    struct list_item* head;
    size_t size;
};

// Init list_item
void list_item_init(struct list_item* li, void* data);

// Init linked_list
void list_init(struct linked_list* ll);
// Push an item at front
void list_push_front(struct linked_list* ll, struct list_item* new_item);
// Push an item at back
void list_push_back(struct linked_list* ll, struct list_item* new_item);
// Insert a list_item you own(!!) after pos
void list_insert(struct linked_list* ll, struct list_item* pos, struct list_item* new_item);
// Remove a list_item
void list_remove(struct linked_list* ll, struct list_item* li);
// Find the first list_item with the specific data
struct list_item* list_find(struct linked_list* ll, void* data, int (*cmp) (const void* data, const void* li_data));
// Free the structure of the linked_list (not linked_list itself). Be careful about object pointers stored in the linked_list and
// where the object allocated (stack or heap?). Deallocator runs on every data member of the list_item
void list_free(struct linked_list* ll, void* userdata, void (*deallocator) (void* data, void* userdata));

#ifdef __cplusplus
}
#endif

#endif // LINKED_LIST