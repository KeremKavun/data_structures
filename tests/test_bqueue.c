#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/bqueue.h"

// Copy function for enqueue
static int copy_int(const void* new_item, void* queue_item, void* userdata) {
    (void)userdata;
    memcpy(queue_item, new_item, sizeof(int));
    return 0;
}

// Emplace initializer
static int init_int(void* queue_item, void* userdata) {
    static int counter = 100;
    (void)userdata;
    *(int*)queue_item = counter++;
    return 0;
}

// Walk handler
static void print_int(void* item, void* userdata) {
    (void)userdata;
    printf("%d ", *(int*)item);
}

int main(void) {
    struct bqueue q;
    if (bqueue_init(&q, sizeof(int)) != 0) {
        fprintf(stderr, "bqueue_init failed\n");
        return 1;
    }

    printf("Initialized queue (capacity=%zu)\n", bqueue_capacity(&q));

    // Enqueue 5 integers
    for (int i = 0; i < 5; ++i) {
        if (benqueue(&q, &i, NULL, copy_int) != 0) {
            fprintf(stderr, "enqueue failed at %d\n", i);
            return 1;
        }
    }

    printf("After enqueue 5: size=%zu\n", bqueue_size(&q));

    // Peek at front and rear
    int front, rear;
    if (bqueue_front(&q, &front) == 0)
        printf("Front=%d\n", front);
    if (bqueue_rear(&q, &rear) == 0)
        printf("Rear=%d\n", rear);

    // Dequeue 3 items
    for (int i = 0; i < 3; ++i) {
        int val;
        if (bdequeue(&q, &val) == 0)
            printf("Dequeued: %d\n", val);
    }

    printf("After dequeue 3: size=%zu\n", bqueue_size(&q));

    // Emplace enqueue 3 new items
    for (int i = 0; i < 3; ++i)
        emplace_benqueue(&q, NULL, init_int);

    printf("After emplace enqueue 3: size=%zu\n", bqueue_size(&q));

    // Walk remaining items
    printf("Remaining queue: ");
    bqueue_walk(&q, NULL, print_int);
    printf("\n");

    // Dequeue everything
    while (!bqueue_empty(&q)) {
        int val;
        bdequeue(&q, &val);
        printf("Dequeued: %d\n", val);
    }

    printf("Queue empty? %s\n", bqueue_empty(&q) ? "yes" : "no");

    bqueue_free(&q, NULL, NULL);
    printf("Freed queue\n");

    return 0;
}