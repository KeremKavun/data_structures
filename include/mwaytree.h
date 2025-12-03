#ifndef MWAYTREE_H
#define MWAYTREE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../debug/include/debug.h"
#include "../../concepts/include/object_concept.h"
#include <stddef.h>

// child_capacity is the total number of children nodes.
// data_capacity is the total number of data pointers.
// It isnt reasonable to have capacities other than equal child and data capacities or
// one less data capacity than child capacity (for example, B-tree).
struct mway_header
{
    size_t child_capacity;
    size_t data_capacity;
};

// This is sample macro to generate mwaytree types, not a must to use.
// For example, B-tree implementation can alter void* data[N] to void* data[N-1].
// Only requirement is that every implementation that uses m-way tree semantic
// shall put struct mway_header as the first member of the struct and
// child_capacity and data_capacity members shall be the first members of the struct.
#define GENERATE_MWAYTREE(N)                    \
    struct m##N##tree                           \
    {                                           \
        struct mway_header header;              \
        struct mway_header* children[N];        \
        void* data[N];                          \
    };                                          \
                                                \
    typedef struct m##N##tree m##N##tree##_t;

// Macro to round a size up to the next multiple of 'align'. We will need this if we are aggregating standart mway tree node.
#define ALIGN_UP(size, align) (((size) + (align) - 1) & ~((align) - 1))
#define ALIGN_REQ sizeof(void*)

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/
 
struct mway_header* mway_create(size_t child_capacity, size_t data_capacity, size_t footer_size, struct object_concept* oc);
void mway_destroy(struct mway_header* header, void* context, struct object_concept* oc);
size_t mway_sizeof(size_t child_capacity, size_t data_capacity, size_t footer_size, size_t align_req);

/*───────────────────────────────────────────────
 * Getters & Setters
 *───────────────────────────────────────────────*/

struct mway_header* mway_get_child(struct mway_header* header, size_t index);
const struct mway_header* mway_get_child_const(const struct mway_header* header, size_t index);
void* mway_get_data(struct mway_header* header, size_t index);
const void* mway_get_data_const(const struct mway_header* header, size_t index);
void mway_set_data(struct mway_header* header, size_t index, void* data);
void* mway_get_footer(struct mway_header* header);
const void* mway_get_footer_const(const struct mway_header* header);
struct mway_header** mway_get_child_addr(struct mway_header* header, size_t index);
void** mway_get_data_addr(struct mway_header* header, size_t index);

#ifdef __cplusplus
}
#endif

#endif // MWAYTREE_H