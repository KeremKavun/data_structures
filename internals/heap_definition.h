#ifndef HEAP_DEFINITION_H
#define HEAP_DEFINITION_H

#include "../../buffers/internal/lbuffer_definition.h"

struct heap
{
    struct lbuffer buffer;
    int (*cmp) (const void* a, const void* b);
};

#endif // HEAP_DEFINITION_H