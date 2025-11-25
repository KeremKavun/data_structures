#include "../../include/mwaytree.h"
#include <stdlib.h>
#include <string.h>

static inline struct mway_header** mway_children_base(struct mway_header* header);
static inline void** mway_data_base(struct mway_header* header);
static inline struct mway_header* mway_get_child(struct mway_header* header, size_t index);
static inline void mway_set_child(struct mway_header* header, size_t index, struct mway_header* child);
static inline void* mway_get_data(struct mway_header* header, size_t index);
static inline void mway_set_data(struct mway_header* header, size_t index, void* data);

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

struct mway_header* mway_create(size_t m, struct object_concept* oc)
{
    size_t total_size = sizeof(struct mway_header) + m * (sizeof(void*) + sizeof(struct mway_header*));
    void* mway = (oc && oc->allocator) ? oc->alloc(oc->allocator) : malloc(total_size);
    if (!mway)
    {
        LOG(LIB_LVL, CERROR, "Failed to allocate memory for mway");
        return NULL;
    }
    struct mway_header* header = (struct mway_header*) mway;
    header->capacity = m;
    header->size = 0;
    memset(mway_children_base(header), 0, m * sizeof(struct mway_header*));
    memset(mway_data_base(header), 0, m * sizeof(void*));
    return header;
}

void mway_destroy(struct mway_header* header, void* context, struct object_concept* oc)
{
    if (!header)
        return;
    for (size_t i = 0; i < header->size; i++)
        mway_destroy(mway_get_child(header, i), context, oc);
    if (oc && oc->destruct)
    {
        for (size_t i = 0; i < header->size; i++)
            oc->destruct(mway_get_data(header, i), context);
    }
    (oc && oc->allocator) ? oc->free(oc->allocator, header) : free(header);
}

// *** Helper functions *** //

static inline struct mway_header** mway_children_base(struct mway_header* header)
{
    return (struct mway_header**) ((char*) header + sizeof(struct mway_header));
}

static inline void** mway_data_base(struct mway_header* header)
{
    return (void**) ((char*) mway_children_base(header) + header->capacity * sizeof(struct mway_header*));
}

static inline struct mway_header* mway_get_child(struct mway_header* header, size_t index)
{
    return mway_children_base(header)[index];
}

static inline void mway_set_child(struct mway_header* header, size_t index, struct mway_header* child)
{
    mway_children_base(header)[index] = child;
}

static inline void* mway_get_data(struct mway_header* header, size_t index)
{
    return mway_data_base(header)[index];
}

static inline void mway_set_data(struct mway_header* header, size_t index, void* data)
{
    mway_data_base(header)[index] = data;
}
