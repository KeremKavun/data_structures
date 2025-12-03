#include "../include/mwaytree.h"
#include <stdlib.h>
#include <string.h>

static inline struct mway_header** children_base(struct mway_header* header);
static inline void** data_base(struct mway_header* header);
static inline size_t children_count(struct mway_header* header);
static inline size_t data_count(struct mway_header* header);

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

struct mway_header* mway_create(size_t child_capacity, size_t data_capacity, size_t footer_size, struct object_concept* oc)
{
    size_t total_size = mway_sizeof(child_capacity, data_capacity, footer_size, ALIGN_REQ);
    void* mway = (oc && oc->allocator) ? oc->alloc(oc->allocator) : malloc(total_size);
    if (!mway)
    {
        LOG(LIB_LVL, CERROR, "Failed to allocate memory for mway");
        return NULL;
    }
    struct mway_header* header = (struct mway_header*) mway;
    header->child_capacity = child_capacity;
    header->data_capacity = data_capacity;
    memset(children_base(header), 0, child_capacity * sizeof(struct mway_header*));
    memset(data_base(header), 0, data_capacity * sizeof(void*));
    memset((char*) header + total_size - footer_size, 0, footer_size);
    return header;
}

void mway_destroy(struct mway_header* header, void* context, struct object_concept* oc)
{
    if (!header)
        return;
    for (size_t i = 0; i < children_count(header); i++)
        mway_destroy(mway_get_child(header, i), context, oc);
    if (oc && oc->destruct)
        for (size_t i = 0; i < data_count(header); i++)
            oc->destruct(mway_get_data(header, i), context);
    (oc && oc->allocator) ? oc->free(oc->allocator, header) : free(header);
}

size_t mway_sizeof(size_t child_capacity, size_t data_capacity, size_t footer_size, size_t align_req)
{
    size_t raw_size = sizeof(struct mway_header) + child_capacity * sizeof(struct mway_header*) + data_capacity * sizeof(void*) + footer_size;
    return ALIGN_UP(raw_size, align_req);
}

/*───────────────────────────────────────────────
 * Getters & Setters
 *───────────────────────────────────────────────*/

struct mway_header* mway_get_child(struct mway_header* header, size_t index)
{
    return children_base(header)[index];
}

const struct mway_header* mway_get_child_const(const struct mway_header* header, size_t index)
{
    return (const struct mway_header*) children_base((struct mway_header*) header)[index];
}

void mway_set_child(struct mway_header* header, size_t index, struct mway_header* child)
{
    children_base(header)[index] = child;
}

void* mway_get_data(struct mway_header* header, size_t index)
{
    return data_base(header)[index];
}

const void* mway_get_data_const(const struct mway_header* header, size_t index)
{
    return data_base((struct mway_header*) header)[index];
}

void mway_set_data(struct mway_header* header, size_t index, void* data)
{
    data_base(header)[index] = data;
}

void* mway_get_footer(struct mway_header* header)
{
    return (void*) (data_base(header) + header->data_capacity);
}

const void* mway_get_footer_const(const struct mway_header* header)
{
    return (const void*) (data_base((struct mway_header*) header) + header->data_capacity);
}

struct mway_header** mway_get_child_addr(struct mway_header* header, size_t index)
{
    return &children_base(header)[index];
}

void** mway_get_data_addr(struct mway_header* header, size_t index)
{
    return &data_base(header)[index];
}

// *** Helper functions *** //

static inline struct mway_header** children_base(struct mway_header* header)
{
    return (struct mway_header**) ((char*) header + sizeof(struct mway_header));
}

static inline void** data_base(struct mway_header* header)
{
    return (void**) ((char*) children_base(header) + children_count(header) * sizeof(struct mway_header*));
}

static inline size_t children_count(struct mway_header* header)
{
    return header->child_capacity;
}

static inline size_t data_count(struct mway_header* header)
{
    return header->data_capacity;
}
