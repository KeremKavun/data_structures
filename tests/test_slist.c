#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "../include/slist.h"
#include "../../concepts/include/allocator_concept.h"
#include "../../concepts/include/object_concept.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* * =================================================================================
 * 1. SYSTEM IMPLEMENTATION (User side)
 * Implementing the wrappers defined in allocator_concept.h
 * =================================================================================
 */

void *sysalloc(void *allocator) {
    struct syspool *pool = (struct syspool *)allocator;
    return malloc(pool->obj_size);
}

void sysfree(void *allocator, void *ptr) {
    (void)allocator; // Not needed for standard free
    free(ptr);
}

/* * =================================================================================
 * 2. TEST HELPERS & MOCKS
 * =================================================================================
 */

// A sample data object to store in the list
typedef struct {
    int id;
    char name[16];
} my_data_t;

// Counter to verify deinit calls
static int deinit_call_count = 0;

// Deinit function matching object_concept signature
void test_deinit_func(void *object, void *context) {
    (void)object;
    // Context is used here to pass a counter
    int *counter = (int *)context;
    if (counter) {
        (*counter)++;
    }
}

// Predicate for finding an item (id == 2)
int find_id_2(void *data) {
    my_data_t *d = (my_data_t *)data;
    return (d->id == 2) ? 0 : 1;
}

/* * =================================================================================
 * 3. UNIT TESTS
 * =================================================================================
 */

void test_lifecycle_and_allocator() {
    printf("[Test] Lifecycle & Allocator Concept Integration... ");

    struct slist list;
    
    // 1. Setup Allocator for SLIST NODES
    // Important: The list needs to allocate 'struct slist_item', not user data.
    struct syspool node_pool;
    node_pool.obj_size = sizeof(struct slist_item);

    struct allocator_concept ac = {
        .allocator = &node_pool,
        .alloc = sysalloc,
        .free = sysfree
    };

    // 2. Init
    slist_init(&list, &ac);
    assert(slist_empty(&list) == 1);

    // 3. Insert Data
    my_data_t d1 = {1, "A"};
    my_data_t d2 = {2, "B"};

    // Insert d1 at head
    slist_insert(&list, slist_head(&list), &d1);
    // Insert d2 at head (list becomes: d2 -> d1)
    slist_insert(&list, slist_head(&list), &d2);

    assert(slist_size(&list) == 2);
    
    // Verify Head is d2
    my_data_t *head_data = (my_data_t *)slist_item_data(slist_head(&list));
    assert(head_data->id == 2);

    // 4. Cleanup (without object concept)
    // Passing NULL for context and oc since we don't own d1/d2 memory here (stack)
    slist_deinit(&list, NULL, NULL);
    
    assert(slist_size(&list) == 0);
    printf("PASSED\n");
}

void test_safe_iteration_logic() {
    printf("[Test] Safe Iteration & Removal... ");

    struct slist list;
    struct syspool node_pool = { .obj_size = sizeof(struct slist_item) };
    struct allocator_concept ac = { .allocator = &node_pool, .alloc = sysalloc, .free = sysfree };
    
    slist_init(&list, &ac);

    // Create List: 10 -> 20 -> 30
    int vals[] = {10, 20, 30};
    slist_insert(&list, slist_head(&list), &vals[2]); // 30
    slist_insert(&list, slist_head(&list), &vals[1]); // 20 -> 30
    slist_insert(&list, slist_head(&list), &vals[0]); // 10 -> 20 -> 30

    struct slist_item **iter;
    struct slist_item *n; // Temp for safe macro

    // Iterate safely and remove '20'
    slist_foreach_safe(&list, iter, n, slist_head(&list), NULL) {
        int *val = (int *)slist_item_data(iter);
        if (*val == 20) {
            slist_remove(&list, iter);
        }
    }

    assert(slist_size(&list) == 2);

    // Verify remaining: 10 -> 30
    iter = slist_head(&list);
    assert(*(int*)slist_item_data(iter) == 10);
    
    iter = slist_item_next(iter);
    assert(*(int*)slist_item_data(iter) == 30);

    slist_deinit(&list, NULL, NULL);
    printf("PASSED\n");
}

void test_object_concept_deinit() {
    printf("[Test] Object Concept Deinit... ");

    struct slist list;
    struct syspool node_pool = { .obj_size = sizeof(struct slist_item) };
    struct allocator_concept ac = { .allocator = &node_pool, .alloc = sysalloc, .free = sysfree };
    
    slist_init(&list, &ac);

    int dummy1 = 1, dummy2 = 2;
    slist_insert(&list, slist_head(&list), &dummy1);
    slist_insert(&list, slist_head(&list), &dummy2);

    // Setup Object Concept
    struct object_concept oc = {
        .init = NULL, // Not used by slist_deinit
        .deinit = test_deinit_func
    };

    int context_counter = 0;
    
    // Deinit list, passing the counter as context
    slist_deinit(&list, &context_counter, &oc);

    // Should have called deinit twice (once for each item)
    assert(context_counter == 2);
    assert(slist_size(&list) == 0);

    printf("PASSED\n");
}

void test_find_macro() {
    printf("[Test] Find Entry Macro... ");

    struct slist list;
    struct syspool node_pool = { .obj_size = sizeof(struct slist_item) };
    struct allocator_concept ac = { .allocator = &node_pool, .alloc = sysalloc, .free = sysfree };
    slist_init(&list, &ac);

    my_data_t d1 = {1, "A"}, d2 = {2, "B"}, d3 = {3, "C"};
    slist_insert(&list, slist_head(&list), &d3);
    slist_insert(&list, slist_head(&list), &d2);
    slist_insert(&list, slist_head(&list), &d1);

    struct slist_item **result = NULL;
    
    // Search for ID 2
    slist_find_entry(result, &list, slist_head(&list), NULL, find_id_2);

    assert(result != NULL);
    my_data_t *found = (my_data_t *)slist_item_data(result);
    assert(found->id == 2);
    assert(strcmp(found->name, "B") == 0);

    slist_deinit(&list, NULL, NULL);
    printf("PASSED\n");
}

/* * =================================================================================
 * MAIN RUNNER
 * =================================================================================
 */
int main() {
    printf("=== SList with Allocator/Object Concepts Tests ===\n");
    
    test_lifecycle_and_allocator();
    test_safe_iteration_logic();
    test_find_macro();
    test_object_concept_deinit();
    
    printf("=== All Tests Passed ===\n");
    return 0;
}
