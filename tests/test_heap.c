#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../include/heap.h" 

/*───────────────────────────────────────────────
 * Test Helpers & Mock Data
 *───────────────────────────────────────────────*/

// We will store pointers to integers in the heap
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

// 2. Deallocator Function
// Used to verify heap_destroy cleans up user data
int free_counter = 0;
void custom_deallocator(void* data, void* context) {
    if (data) {
        free(data);
        free_counter++;
    }
}

// 3. Walker Handler
// Used to test heap_walk
void print_node(void* data, void* userdata) {
    item_t* item = (item_t*)data;
    printf("Heap Node Value: %d\n", item->value);
    (void)userdata; 
    // printf("Walking node: %d\n", item->value);
}

// Helper to allocate an integer for the heap
item_t* create_item(int value) {
    item_t* item = malloc(sizeof(item_t));
    assert(item != NULL);
    item->value = value;
    LOG(LIB_LVL, CINFO, "Created item %d at %p", value, item);
    return item;
}

/*───────────────────────────────────────────────
 * Test Cases
 *───────────────────────────────────────────────*/

void test_lifecycle_and_properties() {
    printf("Test: Lifecycle and Heap Property...\n");

    // Initialize Heap (Capacity 10, Resize 1/True)
    char array[10 * sizeof(void*)];
    struct heap* h = heap_create(array, sizeof(array) / (sizeof(array[0]) * sizeof(void*)), NO_RESIZE, compare_ints);
    assert(h != NULL);
    assert(heap_empty(h) == 1);
    assert(heap_size(h) == 0);

    // Add items in random order
    // We expect them to come out: 100, 20, 30, 10
    heap_add(h, create_item(30));
    heap_add(h, create_item(10));
    heap_add(h, create_item(100));
    heap_add(h, create_item(20));

    heap_walk(h, NULL, print_node);

    assert(heap_size(h) == 4);
    assert(heap_empty(h) == 0);

    // Verify Min-Heap Property (Pop items)
    item_t* popped;

    // 1st Pop -> 100
    popped = (item_t*)heap_remove(h);
    assert(popped->value == 100);
    free(popped); 

    // 2nd Pop -> 30
    popped = (item_t*)heap_remove(h);
    assert(popped->value == 30);
    free(popped);

    // 3rd Pop -> 20
    popped = (item_t*)heap_remove(h);
    assert(popped->value == 20);
    free(popped);

    // 4th Pop -> 10
    popped = (item_t*)heap_remove(h);
    assert(popped->value == 10);
    free(popped);

    assert(heap_empty(h) == 1);

    // Destroy empty heap
    heap_destroy(h, NULL, custom_deallocator);
    
    printf("PASSED\n");
}

void test_resizing_and_cleanup() {
    printf("Test: Resizing and Cleanup... \n");
    
    // Reset counter
    free_counter = 0;

    // Create small heap (Capacity 2)
    struct heap* h = heap_create(NULL, 2, AUTO_RESIZE, compare_ints);
    
    // Add 5 items (triggering resize)
    for (int i = 0; i < 5; i++) {
        heap_add(h, create_item(i));
    }

    assert(heap_size(h) == 5);

    // Test Walker
    heap_walk(h, NULL, print_node);

    // Destroy heap with items still in it
    // This should trigger custom_deallocator 5 times
    heap_destroy(h, NULL, custom_deallocator);

    assert(free_counter == 5);

    printf("PASSED\n");
}

/*───────────────────────────────────────────────
 * Main Entry
 *───────────────────────────────────────────────*/

int main() {
    printf("================================\n");
    printf("Starting Heap API Tests\n");
    printf("================================\n");

    test_lifecycle_and_properties();
    test_resizing_and_cleanup();

    printf("================================\n");
    printf("All Tests Passed Successfully.\n");
    printf("================================\n");

    return 0;
}