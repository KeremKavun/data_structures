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
    size_t capacity;
};

struct mway_entry
{
    void* data;
    struct mway_header* child;
};

// This is sample macro to generate mwaytree types, not a must to use.
// Other mway node types might allocate additional things in footer area
// for example B-trees allocate one more child and size_t attribute to track
// child nodes.
#define GENERATE_MWAYTREE(N)                            \
    struct m##N##tree                                   \
    {                                                   \
        struct mway_header header;                      \
        struct mway_entry entries[N];                   \
        footer objects, pay attention to alignments     \
    };                                                  \
                                                        \
    typedef struct m##N##tree m##N##tree##_t;

// Macro to round a size up to the next multiple of 'align'. We will need this if we are aggregating standart mway tree node.
#define ALIGN_UP(size, align) (((size) + (align) - 1) & ~((align) - 1))
#define ALIGN_REQ sizeof(void*)

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/
 
struct mway_header* mway_create(size_t capacity, size_t footer_size, struct object_concept* oc);
// This function only destroys datas and nodes in the 'entries' (see sample) array
void mway_destroy(struct mway_header* header, void* context, struct object_concept* oc);
size_t mway_sizeof(size_t capacity, size_t footer_size);

/*───────────────────────────────────────────────
 * Getters & Setters
 *───────────────────────────────────────────────*/

struct mway_entry mway_get_entry(struct mway_header* header, size_t index);
void mway_set_entry(struct mway_header* header, size_t index, struct mway_entry* new_entry);
struct mway_header* mway_get_child(struct mway_header* header, size_t index);
const struct mway_header* mway_get_child_const(const struct mway_header* header, size_t index);
void* mway_get_data(struct mway_header* header, size_t index);
const void* mway_get_data_const(const struct mway_header* header, size_t index);
void mway_set_data(struct mway_header* header, size_t index, void* data);
void* mway_get_footer(struct mway_header* header);
const void* mway_get_footer_const(const struct mway_header* header);
struct mway_entry* mway_get_entry_addr(struct mway_header* header, size_t index);

#ifdef __cplusplus
}
#endif

#endif // MWAYTREE_H