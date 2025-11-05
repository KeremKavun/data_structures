#ifndef SAFE_QUEUE_H
#define SAFE_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../debug/include/debug.h"
#include <stddef.h>

struct lqueue
{
    struct dbly_linked_list* contents;
};

typedef struct lqueue lqueue_t;

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

// init queue, returns 0 if it succeeds, 1 otherwise
int lqueue_init(struct lqueue* lq);
// free queue contents (freeing queue itself, if dynamically allocated, is on you)
void lqueue_free(struct lqueue* lq, void* userdata, void (*deallocator) (void* item, void* userdata));

/*───────────────────────────────────────────────
 * Enqueue & Dequeue
 *───────────────────────────────────────────────*/

// enqueue an item, returns 0 if it succeeds, 1 otherwise
int lenqueue(struct lqueue* lq, void* new_item);
// dequeue an item, returns either item or NULL
void* ldequeue(struct lqueue* lq);

/*───────────────────────────────────────────────
 * Accessors
 *───────────────────────────────────────────────*/

// Get the front of the queue
void* lqueue_front(const struct lqueue* lq);
// Get the rear of the queue
void* lqueue_rear(const struct lqueue* lq);
// Returns 1 if the lqueue is empty, 0 otherwise
int lqueue_empty(const struct lqueue* bq);
// Returns the size of the lqueue
size_t lqueue_size(const struct lqueue* bq);

/*───────────────────────────────────────────────
 * Iterations
 *───────────────────────────────────────────────*/

// walk queue by given handler defined by the user according to item type they enqueue
void lqueue_walk(const struct lqueue* lq, void* userdata, void (*handler) (void* item, void* userdata));

#ifdef __cplusplus
}
#endif

#endif // SAFE_QUEUE_H