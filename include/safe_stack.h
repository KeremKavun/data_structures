#ifndef STACK_H
#define STACK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../debug/include/debug.h"
#include <stddef.h>

#define size_sst(sst) ((sst)->top)
#define is_sst_empty(sst) (((sst)->top == 0))

struct safe_stack
{
    void** contents;
    size_t capacity;
    size_t top;
};

// init stack, returns 0 if it succeeds, 1 otherwise
int safe_stack_init(struct safe_stack* sst);
// push an item, returns 0 if it succeeds, 1 otherwise
int safe_push(struct safe_stack* sst, const void* new, size_t obj_size);
// pop an item, returns either item or NULL
void* safe_pop(struct safe_stack* sst);
// peek an item, returns either item or NULL
void* safe_stack_peek(const struct safe_stack* sst);
// walk stack by given handler defined by the user according to item type they enqueue
void safe_stack_walk(struct safe_stack* sst, void* userdata, void (*handler) (void* item, void* userdata));
// free stack contents (freeing stack itself, if dynamically allocated, is on you)
void safe_stack_free(struct safe_stack* sst, void* userdata, void (*deallocator) (void* item, void* userdata));

#ifdef __cplusplus
}
#endif

#endif // STACK_H