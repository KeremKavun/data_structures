#ifndef BSTACK_H
#define BSTACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../concepts/include/object_concept.h"
#include "../../debug/include/debug.h"
#include <stddef.h>

struct bstack
{
    struct lbuffer* contents;
};

typedef struct bstack bstack_t;

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

// init bstack, returns 0 if it succeeds, 1 otherwise
int bstack_init(struct bstack* bs, size_t obj_size);
// free bstack contents (freeing bstack itself, if dynamically allocated, is on you)
// if you dont store pointers to dynamic allocated objects, this function with a deallocator might be dangerous !!!
void bstack_deinit(struct bstack* bs, void* context, struct object_concept* oc);

/*───────────────────────────────────────────────
 * Push & Pop
 *───────────────────────────────────────────────*/

// push an item by copying, returns 0 if it succeeds, 1 otherwise
int bpush(struct bstack* bs, const void* new_item);
// push an item by initializing the object in place, bstack will own the object 
// must be careful if the given object stores pointers to the objects in the heap, you must provide a deallocator in this case
int emplace_bpush(struct bstack* bs, void* args, struct object_concept* oc);
// pop an item, returns 1 if queue is empty (failure), 0 if it successfull, copies data into void* result if result is not NULL
int bpop(struct bstack* bs, void* result);

/*───────────────────────────────────────────────
 * Accessors
 *───────────────────────────────────────────────*/

// peek an item, returns 1 if queue is empty (failure), 0 if it successfull copies data into void* result
int btop(const struct bstack* bs, void* result);
// Returns 1 if the bstack is empty, 0 otherwise
int bstack_empty(const struct bstack* bs);
// Returns the size of the bstack
size_t bstack_size(const struct bstack* bs);
// Returns the total number of objects with obj_size passed to the initializer
size_t bstack_capacity(const struct bstack* bs);

/*───────────────────────────────────────────────
 * Iterations
 *───────────────────────────────────────────────*/

// walk bstack by given handler defined by the user according to item type they enqueue
void bstack_walk(struct bstack* bs, void* context, void (*handler) (void* item, void* context));

#ifdef __cplusplus
}
#endif

#endif // BSTACK_H