#ifndef STACK_H
#define STACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../debug/include/debug.h"
#include <stddef.h>

struct bstack
{
    char* contents;
    size_t capacity;
    size_t size;
    size_t obj_size;
};

typedef struct bstack bstack_t;

// init bstack, returns 0 if it succeeds, 1 otherwise
int bstack_init(struct bstack* bs, size_t obj_size);
// push an item by copying, returns 0 if it succeeds, 1 otherwise
int bpush(struct bstack* bs, const void* new_item, void* userdata, int (*copy) (const void* new_item, void* queue_item, void* userdata));
// push an item by initializing the object in place, bstack will own the object 
// must be careful if the given object stores pointers to the objects in the heap, you must provide a deallocator in this case
int emplace_bpush(struct bstack* bs, void* userdata, int (*init) (void* item, void* userdata));
// pop an item, returns 1 if queue is empty (failure), 0 if it successfull copies data into void* result
int bpop(struct bstack* bs, void* result);
// peek an item, returns 1 if queue is empty (failure), 0 if it successfull copies data into void* result
int btop(const struct bstack* bs, void* result);
// Returns 1 if the bstack is empty, 0 otherwise
int bstack_empty(const struct bstack* bs);
// Returns the size of the bstack
size_t bstack_size(const struct bstack* bs);
// Returns the total number of objects with obj_size passed to the initializer
size_t bstack_capacity(const struct bstack* bs);
// walk bstack by given handler defined by the user according to item type they enqueue
void bstack_walk(struct bstack* bs, void* userdata, void (*handler) (void* item, void* userdata));
// free bstack contents (freeing bstack itself, if dynamically allocated, is on you)
// if you dont store pointers to dynamic allocated objects, this function with a deallocator might be dangerous !!!
void bstack_free(struct bstack* bs, void* userdata, void (*deallocator) (void* item, void* userdata));

#ifdef __cplusplus
}
#endif

#endif // STACK_H