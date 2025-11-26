#ifndef MWAYTREE_H
#define MWAYTREE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../debug/include/debug.h"
#include "../../concepts/include/object_concept.h"
#include <stddef.h>

struct mway_header
{
    size_t capacity;
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

int mway_init(struct mway_header* header, size_t m);
struct mway_header* mway_create(size_t m, struct object_concept* oc);
void mway_deinit(struct mway_header* header, void* context, void (*deallocator) (void* item, void* context));
void mway_destroy(struct mway_header* header, void* context, struct object_concept* oc);
size_t mway_sizeof(size_t m);

/*───────────────────────────────────────────────
 * Operations
 *───────────────────────────────────────────────*/

struct mway_header* mway_get_child(struct mway_header* header, size_t index);
const struct mway_header* mway_get_child_const(const struct mway_header* header, size_t index);
void* mway_get_data(struct mway_header* header, size_t index);
const void* mway_get_data_const(const struct mway_header* header, size_t index);
void mway_set_data(struct mway_header* header, size_t index, void* data);

#ifdef __cplusplus
}
#endif

#endif // MWAYTREE_H