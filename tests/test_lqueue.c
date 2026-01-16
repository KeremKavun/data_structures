#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <ds/queue/lqueue.h>
#include <ds/utils/allocator_concept.h>
#include <ds/utils/object_concept.h>

/*───────────────────────────────────────────────
 * Test Helpers & Mock Data
 *───────────────────────────────────────────────*/

// Helper to create int data
int* create_int(int value) {
    int* p = (int*)malloc(sizeof(int));
    *p = value;
    return p;
}

// Destructor for int data
// This matches the void (*deinit) (void *object) signature in object_concept
void int_destruct(void* object) {
    free(object);
}

// Helper to print queue items
void print_int_item(void* item, void* userdata) {
    int* val = (int*)item;
    printf("%d ", *val);
    (void)userdata;
}

/*───────────────────────────────────────────────
 * Test Routine
 *───────────────────────────────────────────────*/

void test_lqueue_with_pool() {
    printf("Testing lqueue with chunked_pool...\n");
    // 2. Setup Allocator Concept
    // This tells lqueue how to allocate its internal nodes
    struct syspool pool = { lqueue_node_sizeof() };
    struct allocator_concept ac = {
        .allocator = &pool,
        .alloc = sysalloc,
        .free = sysfree,
    };

    // 3. Create lqueue
    // New API returns a pointer to the queue instance
    struct lqueue *lq = lqueue_create(&ac);
    assert(lq != NULL);
    assert(lqueue_empty(lq) == 1);

    // 4. Enqueue items
    printf("Enqueueing 1, 2, 3...\n");
    lenqueue(lq, create_int(1));
    lenqueue(lq, create_int(2));
    lenqueue(lq, create_int(3));

    assert(lqueue_size(lq) == 3);
    assert(lqueue_empty(lq) == 0);
    
    int* front = (int*)lqueue_front(lq);
    assert(*front == 1);
    int* rear = (int*)lqueue_rear(lq);
    assert(*rear == 3);

    printf("Queue content: ");
    lqueue_walk(lq, NULL, print_int_item);
    printf("\n");

    // 5. Dequeue items
    printf("Dequeueing...\n");
    int* val1 = (int*)ldequeue(lq);
    assert(*val1 == 1);
    free(val1); // We own the data now

    assert(lqueue_size(lq) == 2);
    front = (int*)lqueue_front(lq);
    assert(*front == 2);

    // 6. Enqueue more to trigger pool reuse
    printf("Enqueueing 4, 5...\n");
    lenqueue(lq, create_int(4));
    lenqueue(lq, create_int(5));
    
    assert(lqueue_size(lq) == 4);

    printf("Queue content: ");
    lqueue_walk(lq, NULL, print_int_item);
    printf("\n");

    // 7. Destroy queue
    // We pass object_concept here so lqueue can free the user data (ints)
    // inside the remaining nodes (2, 3, 4, 5).
    // The internal nodes are returned to the pool via the allocator_concept passed at creation.
    printf("Destroying queue...\n");
    
    // Note: object_concept is no longer entangled with allocator.
    // It only handles the user data lifecycle.
    struct object_concept oc = { .init = NULL, .deinit = int_destruct }; 
    
    lqueue_destroy(lq, &oc);

    printf("lqueue with pool test passed.\n");
}

int main() {
    test_lqueue_with_pool();
    return 0;
}
