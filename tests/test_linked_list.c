#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../include/linked_list.h"
#include "../../concepts/include/allocator_concept.h"
#include "../../concepts/include/object_concept.h"

// Helper to create a list item with integer data
struct list_item* create_int_item(int value) {
    struct list_item* item = (struct list_item*)malloc(sizeof(struct list_item));
    int* data = (int*)malloc(sizeof(int));
    *data = value;
    list_item_init(item, data);
    return item;
}

// Helper to compare integers
// Returns 0 on match (standard C convention, and what list_find expects based on implementation)
int compare_int(void* item_data, void* search_data) {
    int* val = (int*)item_data;
    int* search_val = (int*)search_data;
    return !(*val == *search_val); // Return 0 if equal
}

// Helper to print list
void print_int_item(void* item, void* userdata) {
    struct list_item* li = (struct list_item*)item;
    int* val = (int*)li->data;
    printf("%d ", *val);
}

// Mock destructor for object_concept
void int_destruct(void* object, void* context) {
    free(object);
}

void test_init() {
    printf("Testing Init...\n");
    struct linked_list ll;
    list_init(&ll);
    assert(list_empty(&ll) == 1);
    assert(list_size(&ll) == 0);
    assert(*list_head(&ll) == NULL);
    assert(*list_tail(&ll) == NULL); 
    printf("Init passed.\n");
}

void test_insert_front_back() {
    printf("Testing Insert Front/Back...\n");
    struct linked_list ll;
    list_init(&ll);

    struct list_item* item1 = create_int_item(10);
    list_insert_front(&ll, item1);
    
    assert(list_size(&ll) == 1);
    assert(list_empty(&ll) == 0);
    assert(*list_head(&ll) == item1);

    struct list_item* item2 = create_int_item(20);
    list_insert_back(&ll, item2);

    assert(list_size(&ll) == 2);
    assert(*list_head(&ll) == item1);
    assert(item1->next == item2);

    struct list_item* item3 = create_int_item(5);
    list_insert_front(&ll, item3);
    
    assert(list_size(&ll) == 3);
    assert(*list_head(&ll) == item3);
    assert(item3->next == item1);

    // Cleanup
    free(item1->data); free(item1);
    free(item2->data); free(item2);
    free(item3->data); free(item3);
    printf("Insert Front/Back passed.\n");
}

void test_remove() {
    printf("Testing Remove...\n");
    struct linked_list ll;
    list_init(&ll);

    struct list_item* item1 = create_int_item(1);
    struct list_item* item2 = create_int_item(2);
    struct list_item* item3 = create_int_item(3);

    list_insert_back(&ll, item1);
    list_insert_back(&ll, item2);
    list_insert_back(&ll, item3);

    // Remove front
    struct list_item* removed = list_remove_front(&ll);
    assert(removed == item1);
    assert(list_size(&ll) == 2);
    assert(*list_head(&ll) == item2);

    // Remove back
    removed = list_remove_back(&ll);
    assert(removed == item3);
    assert(list_size(&ll) == 1);
    assert(*list_head(&ll) == item2);
    assert(item2->next == NULL);

    // Remove specific (item2 is now head)
    // list_remove takes struct list_item**
    // We need to find it first or get the pointer to it.
    // Since item2 is head, &ll.head points to it.
    struct list_item** ptr_to_item2 = list_head(&ll);
    removed = list_remove(&ll, ptr_to_item2);
    assert(removed == item2);
    assert(list_size(&ll) == 0);
    assert(list_empty(&ll) == 1);

    free(item1->data); free(item1);
    free(item2->data); free(item2);
    free(item3->data); free(item3);
    printf("Remove passed.\n");
}

void test_find() {
    printf("Testing Find...\n");
    struct linked_list ll;
    list_init(&ll);

    struct list_item* item1 = create_int_item(100);
    struct list_item* item2 = create_int_item(200);
    list_insert_back(&ll, item1);
    list_insert_back(&ll, item2);

    int target = 200;
    struct list_item** found_ptr = list_find(&ll, &target, compare_int);
    assert(found_ptr != NULL);
    assert(*found_ptr == item2);

    target = 100;
    found_ptr = list_find(&ll, &target, compare_int);
    assert(found_ptr != NULL);
    assert(*found_ptr == item1);

    target = 300;
    found_ptr = list_find(&ll, &target, compare_int);
    // list_find returns address of NULL pointer if not found?
    // Implementation: while (*target && ...) target = &(*target)->next; return target;
    // If not found, *target will be NULL.
    assert(found_ptr != NULL); // It returns a pointer to a pointer (which is NULL)
    assert(*found_ptr == NULL);

    free(item1->data); free(item1);
    free(item2->data); free(item2);
    printf("Find passed.\n");
}

void test_reverse() {
    printf("Testing Reverse...\n");
    struct linked_list ll;
    list_init(&ll);

    struct list_item* item1 = create_int_item(1);
    struct list_item* item2 = create_int_item(2);
    struct list_item* item3 = create_int_item(3);

    list_insert_back(&ll, item1);
    list_insert_back(&ll, item2);
    list_insert_back(&ll, item3);

    list_reverse(&ll);

    assert(*list_head(&ll) == item3);
    assert(item3->next == item2);
    assert(item2->next == item1);
    assert(item1->next == NULL);

    free(item1->data); free(item1);
    free(item2->data); free(item2);
    free(item3->data); free(item3);
    printf("Reverse passed.\n");
}

void test_walk() {
    printf("Testing Walk...\n");
    struct linked_list ll;
    list_init(&ll);

    struct list_item* item1 = create_int_item(10);
    struct list_item* item2 = create_int_item(20);
    list_insert_back(&ll, item1);
    list_insert_back(&ll, item2);

    printf("Walk (expect 10 20): ");
    list_walk(&ll, NULL, print_int_item);
    printf("\n");

    free(item1->data); free(item1);
    free(item2->data); free(item2);
    printf("Walk passed.\n");
}

void test_free() {
    printf("Testing Free...\n");
    struct linked_list ll;
    list_init(&ll);

    struct list_item* item1 = create_int_item(10);
    struct list_item* item2 = create_int_item(20);
    list_insert_back(&ll, item1);
    list_insert_back(&ll, item2);

    struct allocator_concept ac = { .allocator = NULL, .alloc = NULL, .free = NULL };

    struct object_concept oc = { .init = NULL, .deinit = int_destruct};
    
    list_free(&ll, NULL, &oc, &ac);
    
    assert(list_size(&ll) == 0);
    assert(list_empty(&ll) == 1);
    assert(ll.head == NULL);
    printf("Free passed.\n");
}

int main() {
    test_init();
    test_insert_front_back();
    test_remove();
    test_find();
    test_walk();
    test_reverse();
    test_free();
    printf("All tests passed!\n");
    return 0;
}
