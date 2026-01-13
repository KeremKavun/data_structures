#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ds/trees/array_heap.h>

/*───────────────────────────────────────────────
 * Test Helpers & Mock Data
 *───────────────────────────────────────────────*/

// We will store the struct BY VALUE in the heap
typedef struct {
    int value;
} item_t;

// 1. Comparator Function (Max-Heap logic)
// Returns < 0 if a < b, 0 if equal, > 0 if a > b
int compare_ints(const void* a, const void* b) {
    const item_t* item_a = (const item_t*) a;
    const item_t* item_b = (const item_t*) b;
    return item_a->value - item_b->value;
}

// 2. Walker Handler
void print_node(void* data, void* userdata) {
    item_t* item = (item_t*)data;
    printf("Heap Node Value: %d\n", item->value);
    (void)userdata; 
}

/*───────────────────────────────────────────────
 * Object concept
 *───────────────────────────────────────────────*/

int copy_item(void *dest, void *src)
{
    assert(dest != NULL && src != NULL);
    *(item_t *) dest = *(const item_t *) src;
    return 0;
}

struct object_concept oc = { .init = copy_item, .deinit = NULL };

/*───────────────────────────────────────────────
 * Test Cases
 *───────────────────────────────────────────────*/

void test_lifecycle_and_properties() {
    printf("Test: Lifecycle and Heap Property...\n");

    struct array_heap h;
    
    // Initialize Heap
    // We pass NULL for object_concept assuming POD (Plain Old Data) copy semantics
    int status = array_heap_init(&h, sizeof(item_t), &oc, compare_ints);
    assert(status == 0);
    assert(array_heap_empty(&h) == 1);
    assert(array_heap_size(&h) == 0);

    // Add items in random order
    // We expect Max-Heap order: 100, 30, 20, 10
    item_t items[] = { {30}, {10}, {100}, {20} };
    
    array_heap_add(&h, &items[0]);
    array_heap_add(&h, &items[1]);
    array_heap_add(&h, &items[2]);
    array_heap_add(&h, &items[3]);

    printf("--- Current Heap Content ---\n");
    array_heap_walk(&h, NULL, print_node);
    printf("----------------------------\n");

    assert(array_heap_size(&h) == 4);
    assert(array_heap_empty(&h) == 0);

    // Verify Max-Heap Property (Pop items)
    item_t popped;

    // 1st Pop -> 100
    array_heap_remove(&h, &popped);
    assert(popped.value == 100);
    printf("Popped: %d\n", popped.value);

    // 2nd Pop -> 30
    array_heap_remove(&h, &popped);
    assert(popped.value == 30);
    printf("Popped: %d\n", popped.value);

    // 3rd Pop -> 20
    array_heap_remove(&h, &popped);
    assert(popped.value == 20);
    printf("Popped: %d\n", popped.value);

    // 4th Pop -> 10
    array_heap_remove(&h, &popped);
    assert(popped.value == 10);
    printf("Popped: %d\n", popped.value);

    assert(array_heap_empty(&h) == 1);

    // Cleanup
    array_heap_deinit(&h);
    
    printf("PASSED\n");
}

void test_resizing_stress() {
    printf("Test: Resizing and Stress... \n");
    
    struct array_heap h;
    // Initialize
    array_heap_init(&h, sizeof(item_t), &oc, compare_ints);
    
    int count = 50;

    // Add 50 items (0 to 49)
    // The underlying dynarray should handle resizing automatically
    for (int i = 0; i < count; i++) {
        item_t item = { i };
        array_heap_add(&h, &item);
    }

    assert(array_heap_size(&h) == (size_t)count);

    // Removing them should yield 49 down to 0
    item_t popped;
    for (int i = count - 1; i >= 0; i--) {
        array_heap_remove(&h, &popped);
        assert(popped.value == i);
    }

    assert(array_heap_empty(&h) == 1);

    array_heap_deinit(&h);

    printf("PASSED\n");
}

/*───────────────────────────────────────────────
 * Main Entry
 *───────────────────────────────────────────────*/

int main() {
    printf("================================\n");
    printf("Starting Array Heap API Tests\n");
    printf("================================\n");

    test_lifecycle_and_properties();
    test_resizing_stress();

    printf("================================\n");
    printf("All Tests Passed Successfully.\n");
    printf("================================\n");

    return 0;
}