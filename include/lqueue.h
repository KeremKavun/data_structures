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

// init queue, returns 0 if it succeeds, 1 otherwise
int lqueue_init(struct lqueue* lq);
// enqueue an item, returns 0 if it succeeds, 1 otherwise
int lenqueue(struct lqueue* lq, void* new_item);
// dequeue an item, returns either item or NULL
void* ldequeue(struct lqueue* lq);
// peek an item, returns either item or NULL
void* lqueue_peek(const struct lqueue* lq);
// Returns 1 if the lqueue is empty, 0 otherwise
int lqueue_empty(const struct lqueue* bq);
// Returns the size of the lqueue
size_t lqueue_size(const struct lqueue* bq);
// walk queue by given handler defined by the user according to item type they enqueue
void lqueue_walk(const struct lqueue* lq, void* userdata, void (*handler) (void* item, void* userdata));
// free queue contents (freeing queue itself, if dynamically allocated, is on you)
void lqueue_free(struct lqueue* lq, void* userdata, void (*deallocator) (void* item, void* userdata));

#ifdef __cplusplus
}
#endif

#endif // SAFE_QUEUE_H