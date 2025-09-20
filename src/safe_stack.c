#include "../include/safe_stack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define INITIAL_CAPACITY 16

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
// safe_stack implementation with multiple types, dynamic allocations and void**                                                            //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

static void default_deallocator(void* item, void* userdata);

int safe_stack_init(struct safe_stack* sst)
{
    sst->contents = malloc(sizeof(void*) * INITIAL_CAPACITY);
    if (!sst->contents)
    {
        LOG(LIB_LVL, CERROR, "Allocation failure");
        return 1;
    }
    sst->capacity = INITIAL_CAPACITY;
    sst->top= 0;
    return 0;
}

int safe_push(struct safe_stack* sst, const void* new, size_t obj_size)
{
    if (sst->capacity == size_sst(sst))
    {
        void** new_contents = realloc(sst->contents, sizeof(void*) * sst->capacity * 2);
        if (!new_contents)
        {
            LOG(LIB_LVL, CERROR, "Allocation failure");
            return 1;
        }
        sst->contents = new_contents;
        sst->capacity *= 2;
    }
    sst->contents[sst->top] = malloc(obj_size);
    if (!sst->contents[sst->top])
    {
        LOG(LIB_LVL, CERROR, "Allocation failure");
        return 1;
    }
    memcpy(sst->contents[sst->top], new, obj_size);
    sst->top++;
    return 0;
}

void* safe_pop(struct safe_stack* sst)
{
    if (is_sst_empty(sst))
        return NULL;
    sst->top--; 
    void* item = sst->contents[sst->top];
    return item;
}

void* safe_stack_peek(const struct safe_stack* sst)
{
    if (is_sst_empty(sst))
        return NULL;
    return sst->contents[sst->top - 1];
}

void safe_stack_walk(struct safe_stack* sst, void* userdata, void (*handler) (void* item, void* userdata))
{
    for (size_t i = 0; i < size_sst(sst); i++)
        handler(sst->contents[i], userdata);
}

void safe_stack_free(struct safe_stack* sst, void* userdata, void (*deallocator) (void* item, void* userdata))
{
    safe_stack_walk(sst, userdata, (deallocator) ? deallocator : default_deallocator);
    free(sst->contents);
    sst->contents = NULL;
    sst->capacity = 0; sst->top= 0;
}

// *** Helper functions *** //

static void default_deallocator(void* item, void* userdata)
{
    free(item);
}