#ifndef MWAYTREE_H
#define MWAYTREE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../debug/include/debug.h"
#include "../../concepts/include/object_concept.h"
#include "../internals/traversals.h"
#include <stddef.h>

struct mway_header
{
    size_t capacity;
    size_t size;
};

#define GENERATE_MWAYTREE(N)                    \
    struct m##N##tree                           \
    {                                           \
        struct mway_header header;              \
        struct mway_header* children[N];        \
        void* data[N];                          \
    };                                          \
                                                \
    typedef struct m##N##tree m##N##tree##_t;

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

struct mway_header* mway_create(size_t m, struct object_concept* oc);
void mway_destroy(struct mway_header* header, void* context, struct object_concept* oc);

/*───────────────────────────────────────────────
 * Operations
 *───────────────────────────────────────────────*/



#ifdef __cplusplus
}
#endif

#endif // MWAYTREE_H