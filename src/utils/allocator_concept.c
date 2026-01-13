#include <ds/utils/allocator_concept.h>
#include <stdlib.h>

void *sysalloc(void *allocator)
{
    struct syspool* sp = allocator;
    return malloc(sp->obj_size);
}

void sysfree(void *allocator, void *ptr)
{
    (void) allocator;
    free(ptr);
}