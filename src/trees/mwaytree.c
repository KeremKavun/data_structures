#include <ds/trees/mwaytree.h>
#include <stdlib.h>
#include <string.h>

/* =========================================================================
 * Create & Destroy
 * ========================================================================= */

struct mway_header* mway_create(size_t capacity, size_t footer_size, struct allocator_concept *ac)
{
    void* mway = ac->alloc(ac->allocator);
    if (!mway) {
        LOG(LIB_LVL, CERROR, "Failed to allocate memory for mway");
        return NULL;
    }
    struct mway_header* header = (struct mway_header*) mway;
    header->capacity = capacity;
    memset(mway_base(header), 0, capacity * sizeof(struct mway_entry));
    memset(mway_get_footer(header), 0, footer_size);
    return header;
}

void mway_destroy(struct mway_header* header, struct object_concept *oc, struct allocator_concept *ac)
{
    if (!header)
        return;
    for (size_t i = 0; i < mway_capacity(header); i++) {
        mway_destroy(mway_get_child(header, i), oc, ac);
    }
    if (oc && oc->deinit) {
        for (size_t i = 0; i < mway_capacity(header); i++) {
            void* data = mway_get_data(header, i);
            if (data)
                oc->deinit(data);
        }
    }
    ac->free(ac->allocator, header);
}

/* =========================================================================
 * Getters & Setters
 * ========================================================================= */