#ifndef STACK_H
#define STACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../debug/include/debug.h"
#include <stddef.h>

struct lstack
{
    struct dbly_linked_list* contents;
};

typedef struct lstack lstack_t;

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

// init bstack, returns 0 if it succeeds, 1 otherwise
int lstack_init(struct lstack* ls);
// free bstack contents (freeing bstack itself, if dynamically allocated, is on you)
void lstack_free(struct lstack* ls, void* userdata, void (*deallocator) (void* item, void* userdata));

/*───────────────────────────────────────────────
 * Push & Pop
 *───────────────────────────────────────────────*/

// push an item, returns 0 if it succeeds, 1 otherwise
int lpush(struct lstack* ls, void* new_item);
// pop an item, returns either item or NULL
void* lpop(struct lstack* ls);

/*───────────────────────────────────────────────
 * Accessors
 *───────────────────────────────────────────────*/

// peek an item, returns either item or NULL
void* ltop(struct lstack* ls);
// Returns 1 if the lstack is empty, 0 otherwise
int lstack_empty(const struct lstack* ls);
// Returns the size of the lstack
size_t lstack_size(const struct lstack* ls);

/*───────────────────────────────────────────────
 * Iterations
 *───────────────────────────────────────────────*/

// walk bstack by given handler defined by the user according to item type they enqueue
void lstack_walk(struct lstack* ls, void* userdata, void (*handler) (void* item, void* userdata));

#ifdef __cplusplus
}
#endif

#endif // STACK_H