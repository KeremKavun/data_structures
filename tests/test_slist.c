#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ds/linkedlists/slist.h>
#include <ds/utils/object_concept.h>

/* =================================================================================
 * 1. TEST HELPERS & MOCKS
 * =================================================================================
 */

// A sample data object with embedded hook
typedef struct {
    int id;
    char name[16];
    struct slist_item hook;  // Intrusive hook
} my_data_t;

// Counter to verify deinit calls
static int counter = 0;

// Deinit function matching deinit_cb signature
void test_deinit_func(void *gitem) {
    // Recover parent structure
    struct slist_item *item = gitem;
    my_data_t *data = slist_entry(item, my_data_t, hook);
    (void)data;  // Would normally free allocated data here
    counter++;
}

// Predicate for finding an item (id == 2)
int find_id_2(my_data_t *data) {
    return (data->id == 2) ? 0 : 1;
}

/* =================================================================================
 * 2. UNIT TESTS
 * =================================================================================
 */

void test_lifecycle_basic() {
    printf("[Test] Lifecycle & Basic Operations... ");

    struct slist list;
    
    // 1. Init
    slist_init(&list);
    assert(slist_empty(&list) == 1);
    assert(slist_size(&list) == 0);

    // 2. Create data on stack (user manages memory in intrusive design)
    my_data_t d1 = {1, "A", {NULL}};
    my_data_t d2 = {2, "B", {NULL}};
    my_data_t d3 = {3, "C", {NULL}};

    // 3. Insert data (list becomes: d1)
    slist_insert(&list, slist_head(&list), &d1.hook);
    assert(slist_size(&list) == 1);
    assert(slist_empty(&list) == 0);
    
    // Insert d2 at head (list becomes: d2 -> d1)
    slist_insert(&list, slist_head(&list), &d2.hook);
    assert(slist_size(&list) == 2);
    
    // Insert d3 at head (list becomes: d3 -> d2 -> d1)
    slist_insert(&list, slist_head(&list), &d3.hook);
    assert(slist_size(&list) == 3);

    // 4. Verify head is d3
    struct slist_item **head = slist_head(&list);
    my_data_t *head_data = slist_entry(*head, my_data_t, hook);
    assert(head_data->id == 3);
    assert(strcmp(head_data->name, "C") == 0);

    // 5. Cleanup (NULL deinit since we don't own the memory)
    slist_deinit(&list, NULL);
    
    assert(slist_size(&list) == 0);
    printf("PASSED\n");
}

void test_safe_iteration_logic() {
    printf("[Test] Safe Iteration & Removal... ");

    struct slist list;
    slist_init(&list);

    // Create data with embedded hooks
    my_data_t d1 = {10, "Ten", {NULL}};
    my_data_t d2 = {20, "Twenty", {NULL}};
    my_data_t d3 = {30, "Thirty", {NULL}};

    // Build list: 10 -> 20 -> 30
    slist_insert(&list, slist_head(&list), &d3.hook);
    slist_insert(&list, slist_head(&list), &d2.hook);
    slist_insert(&list, slist_head(&list), &d1.hook);

    assert(slist_size(&list) == 3);

    struct slist_item **iter;
    struct slist_item *n; // Temp for safe macro

    // Iterate safely and remove '20'
    slist_foreach_safe(iter, n, slist_head(&list), NULL) {
        my_data_t *data = slist_entry(*iter, my_data_t, hook);
        if (data->id == 20) {
            slist_remove(&list, iter);
        }
    }

    assert(slist_size(&list) == 2);

    // Verify remaining: 10 -> 30
    iter = slist_head(&list);
    my_data_t *first = slist_entry(*iter, my_data_t, hook);
    assert(first->id == 10);
    
    iter = slist_item_next(iter);
    my_data_t *second = slist_entry(*iter, my_data_t, hook);
    assert(second->id == 30);

    slist_deinit(&list, NULL);
    printf("PASSED\n");
}

void test_object_concept_deinit() {
    printf("[Test] Object Concept Deinit... ");

    struct slist list;
    slist_init(&list);

    // Allocate data on heap to simulate real usage
    my_data_t *d1 = malloc(sizeof(my_data_t));
    my_data_t *d2 = malloc(sizeof(my_data_t));
    my_data_t *d3 = malloc(sizeof(my_data_t));
    
    d1->id = 1; strcpy(d1->name, "First");
    d2->id = 2; strcpy(d2->name, "Second");
    d3->id = 3; strcpy(d3->name, "Third");

    slist_insert(&list, slist_head(&list), &d3->hook);
    slist_insert(&list, slist_head(&list), &d2->hook);
    slist_insert(&list, slist_head(&list), &d1->hook);

    assert(slist_size(&list) == 3);
    
    // Deinit list with custom deinit function
    counter = 0;
    slist_deinit(&list, test_deinit_func);

    // Should have called deinit three times (once for each item)
    assert(counter == 3);
    assert(slist_size(&list) == 0);

    // Note: In real usage, test_deinit_func would call free() on the recovered pointer
    // For this test, we need to manually free since our deinit doesn't actually free
    free(d1);
    free(d2);
    free(d3);

    printf("PASSED\n");
}

void test_find_macro() {
    printf("[Test] Find Entry Macro... ");

    struct slist list;
    slist_init(&list);

    my_data_t d1 = {1, "Alice", {NULL}};
    my_data_t d2 = {2, "Bob", {NULL}};
    my_data_t d3 = {3, "Charlie", {NULL}};
    
    slist_insert(&list, slist_head(&list), &d3.hook);
    slist_insert(&list, slist_head(&list), &d2.hook);
    slist_insert(&list, slist_head(&list), &d1.hook);

    my_data_t *result = NULL;
    
    // Search for ID 2
    slist_find_entry(result, slist_head(&list), NULL, hook, find_id_2);

    assert(result != NULL);
    assert(result->id == 2);
    assert(strcmp(result->name, "Bob") == 0);

    slist_deinit(&list, NULL);
    printf("PASSED\n");
}

void test_iteration() {
    printf("[Test] Basic Iteration... ");

    struct slist list;
    slist_init(&list);

    my_data_t d1 = {1, "A", {NULL}};
    my_data_t d2 = {2, "B", {NULL}};
    my_data_t d3 = {3, "C", {NULL}};

    slist_insert(&list, slist_head(&list), &d3.hook);
    slist_insert(&list, slist_head(&list), &d2.hook);
    slist_insert(&list, slist_head(&list), &d1.hook);

    // Iterate and verify order: 1 -> 2 -> 3
    struct slist_item **iter;
    int expected_ids[] = {1, 2, 3};
    int idx = 0;

    slist_foreach(iter, slist_head(&list), NULL) {
        my_data_t *data = slist_entry(*iter, my_data_t, hook);
        assert(data->id == expected_ids[idx]);
        idx++;
    }

    assert(idx == 3); // Verify we iterated all items

    slist_deinit(&list, NULL);
    printf("PASSED\n");
}

void test_multiple_removals() {
    printf("[Test] Multiple Removals... ");

    struct slist list;
    slist_init(&list);

    my_data_t d1 = {1, "A", {NULL}};
    my_data_t d2 = {2, "B", {NULL}};
    my_data_t d3 = {3, "C", {NULL}};
    my_data_t d4 = {4, "D", {NULL}};

    slist_insert(&list, slist_head(&list), &d4.hook);
    slist_insert(&list, slist_head(&list), &d3.hook);
    slist_insert(&list, slist_head(&list), &d2.hook);
    slist_insert(&list, slist_head(&list), &d1.hook);

    assert(slist_size(&list) == 4);

    // Remove head
    slist_remove(&list, slist_head(&list));
    assert(slist_size(&list) == 3);

    // Verify new head is d2
    my_data_t *head = slist_entry(*slist_head(&list), my_data_t, hook);
    assert(head->id == 2);

    // Remove all remaining
    while (!slist_empty(&list)) {
        slist_remove(&list, slist_head(&list));
    }

    assert(slist_size(&list) == 0);
    assert(slist_empty(&list) == 1);

    slist_deinit(&list, NULL);
    printf("PASSED\n");
}

/* =================================================================================
 * MAIN RUNNER
 * =================================================================================
 */
int main() {
    printf("=== Intrusive SList Tests ===\n");
    
    test_lifecycle_basic();
    test_iteration();
    test_safe_iteration_logic();
    test_find_macro();
    test_object_concept_deinit();
    test_multiple_removals();
    
    printf("=== All Tests Passed ===\n");
    return 0;
}
