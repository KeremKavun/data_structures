#include "../../include/mwaytree.h"
#include <stdlib.h>
#include <string.h>

static inline struct mway_header** mway_children_base(struct mway_header* header);
static inline void** mway_data_base(struct mway_header* header);
static void mway_walk_preorder(struct mway_header* header, void* userdata, void (*handler) (void* data, void* userdata));
static void mway_walk_inorder(struct mway_header* header, void* userdata, void (*handler) (void* data, void* userdata));
static void mway_walk_postorder(struct mway_header* header, void* userdata, void (*handler) (void* data, void* userdata));

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

int mway_init(struct mway_header* header, size_t m)
{
    header->capacity = m;
    memset(mway_children_base(header), 0, m * sizeof(struct mway_header*));
    memset(mway_data_base(header), 0, m * sizeof(void*));
    return 0;
}

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
    mway_init(header, m);
    return header;
}

void mway_deinit(struct mway_header* header, void* context, void (*deallocator) (void* item, void* context))
{
    if (!header)
        return;
    for (size_t i = 0; i < header->capacity; i++)
        mway_deinit(mway_get_child(header, i), context, deallocator);
    memset(mway_children_base(header), 0, header->capacity * sizeof(struct mway_header*));
    memset(mway_data_base(header), 0, header->capacity * sizeof(void*));
}

void mway_destroy(struct mway_header* header, void* context, struct object_concept* oc)
{
    if (!header)
        return;
    for (size_t i = 0; i < header->capacity; i++)
        mway_destroy(mway_get_child(header, i), context, oc);
    if (oc && oc->destruct)
    {
        for (size_t i = 0; i < header->capacity; i++)
            oc->destruct(mway_get_data(header, i), context);
    }
    (oc && oc->allocator) ? oc->free(oc->allocator, header) : free(header);
}

size_t mway_sizeof(size_t m)
{
    return sizeof(struct mway_header) + m * (sizeof(void*) + sizeof(struct mway_header*));
}

/*───────────────────────────────────────────────
 * Operations
 *───────────────────────────────────────────────*/

struct mway_header* mway_get_child(struct mway_header* header, size_t index)
{
    return mway_children_base(header)[index];
}

const struct mway_header* mway_get_child_const(const struct mway_header* header, size_t index)
{
    return (const struct mway_header*) mway_children_base((struct mway_header*) header)[index];
}

void mway_set_child(struct mway_header* header, size_t index, struct mway_header* child)
{
    mway_children_base(header)[index] = child;
}

void* mway_get_data(struct mway_header* header, size_t index)
{
    return mway_data_base(header)[index];
}

const void* mway_get_data_const(const struct mway_header* header, size_t index)
{
    return mway_data_base((struct mway_header*) header)[index];
}

void mway_set_data(struct mway_header* header, size_t index, void* data)
{
    mway_data_base(header)[index] = data;
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
