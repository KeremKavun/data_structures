#include "../include/mwaytree.h"
#include <stdlib.h>
#include <string.h>

static inline struct mway_entry* base(struct mway_header* header);
static inline size_t count(struct mway_header* header);

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

struct mway_header* mway_create(size_t capacity, size_t footer_size, struct object_concept* oc)
{
    size_t total_size = mway_sizeof(capacity, footer_size);
    void* mway = (oc && oc->allocator) ? oc->alloc(oc->allocator) : malloc(total_size);
    if (!mway)
    {
        LOG(LIB_LVL, CERROR, "Failed to allocate memory for mway");
        return NULL;
    }
    struct mway_header* header = (struct mway_header*) mway;
    header->capacity = capacity;
    memset(base(header), 0, capacity * sizeof(struct mway_entry));
    memset((char*) header + total_size - footer_size, 0, footer_size);
    return header;
}

void mway_destroy(struct mway_header* header, void* context, struct object_concept* oc)
{
    if (!header)
        return;
    for (size_t i = 0; i < count(header); i++)
        mway_destroy(mway_get_child(header, i), context, oc);
    if (oc && oc->destruct)
        for (size_t i = 0; i < data_count(header); i++)
            oc->destruct(mway_get_data(header, i), context);
    (oc && oc->allocator) ? oc->free(oc->allocator, header) : free(header);
}

size_t mway_sizeof(size_t capacity, size_t footer_size)
{
    size_t raw_size = sizeof(struct mway_header) + capacity * sizeof(struct mway_entry) + footer_size;
    return ALIGN_UP(raw_size, ALIGN_REQ);
}

/*───────────────────────────────────────────────
 * Getters & Setters
 *───────────────────────────────────────────────*/

struct mway_entry mway_get_entry(struct mway_header* header, size_t index)
{
    return base(header)[index];
}

void mway_set_entry(struct mway_header* header, size_t index, struct mway_entry* new_entry)
{
    base(header)[index] = *new_entry;
}

struct mway_header* mway_get_child(struct mway_header* header, size_t index)
{
    return base(header)[index].child;
}

const struct mway_header* mway_get_child_const(const struct mway_header* header, size_t index)
{
    return (const struct mway_header*) base((struct mway_header*) header)[index].child;
}

void mway_set_child(struct mway_header* header, size_t index, struct mway_header* child)
{
    base(header)[index].child = child;
}

void* mway_get_data(struct mway_header* header, size_t index)
{
    return base(header)[index].data;
}

const void* mway_get_data_const(const struct mway_header* header, size_t index)
{
    return base((struct mway_header*) header)[index].data;
}

void mway_set_data(struct mway_header* header, size_t index, void* data)
{
    base(header)[index].data = data;
}

void* mway_get_footer(struct mway_header* header)
{
    return (void*) (base(header) + header->capacity);
}

const void* mway_get_footer_const(const struct mway_header* header)
{
    return (const void*) (base((struct mway_header*) header) + header->capacity);
}

struct mway_entry* mway_get_entry_addr(struct mway_header* header, size_t index)
{
    return &base(header)[index];
}

// *** Helper functions *** //

static inline struct mway_entry* base(struct mway_header* header)
{
    return (struct mway_entry*) ((char*) header + sizeof(struct mway_header));
}

static inline size_t count(struct mway_header* header)
{
    return header->capacity;
}