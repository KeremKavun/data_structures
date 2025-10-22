#ifndef LINKED_LIST
#define LINKED_LIST

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

// list_item struct
struct list_item
{
    struct list_item* next;
    void* data;
};

struct linked_list
{
    struct list_item* head;
    size_t size;
};

// Init list_item
void list_item_init(struct list_item* li, void* data);
// Get pointer to pointer that points to next element, which can be used for insert or remove methods
struct list_item** list_item_next(struct list_item* li);
// Get the data from list_item
void* list_item_data(struct list_item* li);

// Init linked_list
void list_init(struct linked_list* ll);
// Insert a list_item you own(!!) at pos
void list_insert(struct linked_list* ll, struct list_item** pos, struct list_item* new_item);
// Insert an item at front
void list_insert_front(struct linked_list* ll, struct list_item* new_item);
// Isert an item at back
void list_insert_back(struct linked_list* ll, struct list_item* new_item);
// Insert a list_item you own(!!) after pos
struct list_item* list_remove(struct linked_list* ll, struct list_item** li);
// Remove front
struct list_item* list_remove_front(struct linked_list* ll);
// Remove back
struct list_item* list_remove_back(struct linked_list* ll);
// Get head
struct list_item** list_head(struct linked_list* ll);
// Get tail (returns address of the next pointer of the last node)
struct list_item** list_tail(struct linked_list* ll);
// Find the first list_item with the specific data
struct list_item** list_find(struct linked_list* ll, void* data, int (*cmp) (void* item, void* data));
// Returns 1 if the list is empty, 0 otherwise
int list_empty(const struct linked_list* ll);
// Returns size of the list
size_t list_size(const struct linked_list* ll);
// Traverses the list
void list_walk(struct linked_list* ll, void* userdata, void (*handler) (void* item, void* userdata));
// Reverses the list
void list_reverse(struct linked_list* ll);
// Free the structure of the linked_list (not linked_list itself). Be careful about object pointers stored in the linked_list and
// where the object allocated (stack or heap?). Deallocator runs on every data member of the list_item
void list_free(struct linked_list* ll, void* userdata, void (*deallocator) (void* data, void* userdata));

#ifdef __cplusplus
}
#endif

#endif // LINKED_LIST