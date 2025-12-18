#ifndef HEAP_DEFINITION_H
#define HEAP_DEFINITION_H

#include "../../buffers/include/lbuffer.h"

struct heap
{
    struct lbuffer buffer;
    int (*cmp) (const void* a, const void* b);
};

#endif // HEAP_DEFINITION_H