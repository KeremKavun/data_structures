#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

// Include your specific headers
// Note: Adjust paths if your directory structure differs
#include "../include/lstack.h"
#include "../../allocators/include/chunked_pool.h"

// ---------------------------------------------------------
// 1. Data Definition
// ---------------------------------------------------------

typedef struct {
    int id;
    char name[32];
} TestObject;

// ---------------------------------------------------------
// 2. Allocator Concept Adapters (Bridging Chunked Pool to Allocator Concept)
// ---------------------------------------------------------

// Wrapper to match: void* (*alloc) (void* allocator);
void* pool_alloc_proxy(void* allocator) {
    return chunked_pool_alloc((struct chunked_pool*)allocator);
}

// Wrapper to match: void (*free) (void* allocator, void* ptr);
void pool_free_proxy(void* allocator, void* ptr) {
    chunked_pool_free((struct chunked_pool*)allocator, ptr);
}

// ---------------------------------------------------------
// 3. Object Concept Adapters (For cleaning up items inside lstack)
// ---------------------------------------------------------

// Called by lstack_deinit for every item remaining in the stack
void test_object_deinit(void* object, void* context) {
    TestObject* item = (TestObject*)object;
    printf("[Object Cleanup] De-initializing Item ID: %d\n", item->id);
    
    // If the TestObjects were dynamically allocated, you would free them here.
    // context can be passed as a pointer to the allocator if needed.
    // For this test, we assume TestObjects are stack-allocated or managed externally,
    // so we just print. If you allocated them via another pool, free them here.
}

// ---------------------------------------------------------
// 4. Stack Walker (Iterator)
// ---------------------------------------------------------

void print_walker(void* item, void* userdata) {
    TestObject* obj = (TestObject*)item;
    printf(" -> Walk Item ID: %d, Name: %s\n", obj->id, obj->name);
}

// ---------------------------------------------------------
// 5. Main Test Execution
// ---------------------------------------------------------

int main() {
    printf("=== Starting LStack with Chunked Pool Test ===\n");

    // --- Step A: Initialize the Memory Allocator (Chunked Pool) ---
    
    // IMPORTANT: lstack will use this allocator to create its INTERNAL nodes.
    // Since we don't see the struct dbly_linked_list_node definition, we must estimate its size.
    // A generic doubly linked list node usually has 2 pointers + 1 data pointer.
    // On 64-bit systems, that's 24 bytes. We use 64 bytes to be safe (padding/debug info).
    const size_t ESTIMATED_NODE_SIZE = 64; 
    const size_t POOL_CAPACITY = 10; // Max 10 items in the stack for this test

    struct chunked_pool* my_pool = chunked_pool_create(POOL_CAPACITY, ESTIMATED_NODE_SIZE);
    if (!my_pool) {
        fprintf(stderr, "Failed to init pool\n");
        return -1;
    }

    // --- Step B: Setup the Allocator Concept ---
    
    struct allocator_concept ac;
    ac.allocator = my_pool;          // The context (the pool instance)
    ac.alloc = pool_alloc_proxy;      // The allocation function
    ac.free = pool_free_proxy;        // The deallocation function

    // --- Step C: Initialize the LStack ---
    
    struct lstack stack;
    if (lstack_init(&stack, &ac) != 0) {
        fprintf(stderr, "Failed to init lstack\n");
        chunked_pool_deinit(my_pool);
        return -1;
    }

    assert(lstack_empty(&stack) == 1);
    printf("[Pass] Stack initialized and empty.\n");

    // --- Step D: Push Items ---
    
    TestObject item1 = {1, "First"};
    TestObject item2 = {2, "Second"};
    TestObject item3 = {3, "Third"};

    lpush(&stack, &item1);
    lpush(&stack, &item2);
    lpush(&stack, &item3);

    printf("[Pass] Pushed 3 items.\n");
    assert(lstack_size(&stack) == 3);
    assert(lstack_empty(&stack) == 0);

    // --- Step E: Peek (Top) ---
    
    TestObject* top = (TestObject*)ltop(&stack);
    if (top) {
        printf("[Pass] Top item is ID: %d (Expected 3)\n", top->id);
        assert(top->id == 3);
    } else {
        fprintf(stderr, "[Fail] Top returned NULL\n");
    }

    // --- Step F: Walk the Stack ---
    
    printf("Walking stack (Top to Bottom):\n");
    lstack_walk(&stack, NULL, print_walker);

    // --- Step G: Pop Item ---
    
    TestObject* popped = (TestObject*)lpop(&stack);
    if (popped) {
        printf("[Pass] Popped Item ID: %d (Expected 3)\n", popped->id);
        assert(popped->id == 3);
    }
    assert(lstack_size(&stack) == 2);

    // --- Step H: De-init (Clean up remaining items) ---
    
    // Setup object concept for cleanup
    struct object_concept oc;
    oc.init = NULL; // Not needed for deinit
    oc.deinit = test_object_deinit;

    printf("De-initializing stack (Cleaning up remaining 2 items)...\n");
    // We pass NULL as context for the object deinit, but you could pass a pool if needed
    lstack_deinit(&stack, NULL, &oc);

    // --- Step I: Cleanup Allocator ---
    
    chunked_pool_destroy(my_pool);
    printf("=== Test Complete: Success ===\n");

    return 0;
}