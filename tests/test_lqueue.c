#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../include/lqueue.h"
#include "../../allocators/include/chunked_pool.h"
#include "../../concepts/include/object_concept.h"
#include "../../linked_lists/include/dbly_linked_list.h" // Needed for sizeof(struct dbly_list_item)

// Helper to create int data
int* create_int(int value) {
    int* p = (int*)malloc(sizeof(int));
    *p = value;
    return p;
}

// Destructor for int data (called when queue is destroyed or item removed if we were removing nodes, 
// but ldequeue returns the data, so it doesn't call destruct on the data. 
// Wait, ldequeue implementation:
// void* data = del->data;
// (lq->oc && lq->oc->allocator) ? lq->oc->free(lq->oc->allocator, del) : free(del);
// return data;
// So ldequeue does NOT call destruct on the data. It only frees the node.
// lqueue_deinit DOES call destruct on the data for remaining items.
void int_destruct(void* object, void* context) {
    free(object);
}

// Helper to print queue items
void print_int_item(void* item, void* userdata) {
    int* val = (int*)item;
    printf("%d ", *val);
}

void test_lqueue_with_pool() {
    printf("Testing lqueue with chunked_pool...\n");

    // 1. Initialize Chunked Pool
    // We need it to store struct dbly_list_item
    size_t node_size = sizeof(struct dbly_list_item);
    size_t capacity = 10; // Small capacity to test
    struct chunked_pool* pool = chunked_pool_create(capacity, node_size);
    assert(pool);

    // 2. Setup Object Concept
    struct object_concept oc = {
        .allocator = pool,
        .alloc = (GENERIC_ALLOC_SIGN)chunked_pool_alloc,
        .free = (GENERIC_FREE_SIGN)chunked_pool_free,
        .destruct = int_destruct
    };

    // 3. Initialize lqueue
    struct lqueue lq;
    int res = lqueue_init(&lq, &oc);
    assert(res == 0);
    assert(lqueue_empty(&lq) == 1);

    // 4. Enqueue items
    printf("Enqueueing 1, 2, 3...\n");
    lenqueue(&lq, create_int(1));
    lenqueue(&lq, create_int(2));
    lenqueue(&lq, create_int(3));

    assert(lqueue_size(&lq) == 3);
    assert(lqueue_empty(&lq) == 0);
    
    int* front = (int*)lqueue_front(&lq);
    assert(*front == 1);
    int* rear = (int*)lqueue_rear(&lq);
    assert(*rear == 3);

    printf("Queue content: ");
    lqueue_walk(&lq, NULL, print_int_item);
    printf("\n");

    // 5. Dequeue items
    printf("Dequeueing...\n");
    int* val1 = (int*)ldequeue(&lq);
    assert(*val1 == 1);
    free(val1); // We own the data now

    assert(lqueue_size(&lq) == 2);
    front = (int*)lqueue_front(&lq);
    assert(*front == 2);

    // 6. Enqueue more to trigger pool reuse
    printf("Enqueueing 4, 5...\n");
    lenqueue(&lq, create_int(4));
    lenqueue(&lq, create_int(5));
    
    assert(lqueue_size(&lq) == 4);

    printf("Queue content: ");
    lqueue_walk(&lq, NULL, print_int_item);
    printf("\n");

    // 7. Deinit queue (should free remaining items: 2, 3, 4, 5)
    // The nodes should be returned to the pool.
    // The data (ints) should be freed by int_destruct.
    printf("Deinitializing queue...\n");
    lqueue_deinit(&lq, NULL);

    // 8. Deinit pool
    chunked_pool_destroy(pool);

    printf("lqueue with pool test passed.\n");
}

int main() {
    test_lqueue_with_pool();
    return 0;
}
