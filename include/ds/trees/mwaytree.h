#ifndef TREES_MWAYTREE_H
#define TREES_MWAYTREE_H

#include <ds/utils/debug.h>
#include <ds/utils/allocator_concept.h>
#include <ds/utils/object_concept.h>
#include <stddef.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file mwaytree.h
 * @brief Defines the interface for M-way trees.
 */

/**
 * @defgroup MWAYTREE_CORE M-Way Trees
 * @ingroup TREES
 * @brief Collection of m-way trees (generic & basic mway, Btree)
 */

/**
 * @defgroup MWAYTREE Basic M-Way Tree
 * @ingroup MWAYTREE_CORE
 * @brief Basic utilites that can be used for mway tree implementations.
 * 
 * @details
 * ### Global Constraints
 * - **NULL Pointers**: All `struct mway_header *header` arguments must be non-NULL.
 * @{
 */

/**
 * @struct mway_header
 * @brief Generic header that allows diferent types of
 * mway tree nodes to operate on same functionality. All
 * mway tree nodes derives from this struct, it should be
 * at first field.
 */
struct mway_header {
    size_t capacity;        ///< Specifies the number of struct mway_entries.
};

/**
 * @struct mway_entry
 * @brief Stores child pointers and data associated
 * with them
 * 
 * @details
 * This struct is used in custom struct definitions as shown in sample macro.
 */
struct mway_entry {
    void                    *data;      ///< Reference to the object.
    struct mway_header      *child;     ///< Pointer to the child node (note that struct mway_header is generic and super parent)
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

/**
 * @name Create & Destroy
 * Functions for setting up the tree.
 * @{
 */
 
/**
 * @brief Creates a mway tree node with given properties.
 * @param[in] capacity Capacity of @ref mway_entry array in an mway tree node.
 * @param[in] footer_size Additional field to generalize more.
 * @param[in] ac allocator_concept to create tree nodes, must be non-NULL and valid.
 * @return mway_header of created mway tree node.
 * @note The mway tree node will look like
 * 
 * [struct mway_header; struct mway_entry[capacity]; footer_size bytes]
 */
struct mway_header *mway_create(size_t capacity, size_t footer_size, struct allocator_concept *ac);

/**
 * @brief 
 * @param[in, out] header Pointer to an mway tree node instance.
 * @param[in] oc object_concept to deinit data references.
 * @warning This function only deletes references stored in @ref struct mway_entry.
 * If you store a pointer to a heap memory in the footer, it is you responsibility
 * to free it. You should free it first, and then call this.
 */
void mway_destroy(struct mway_header *header, struct object_concept *oc, struct allocator_concept *ac);

/**
 * @param[in] capacity Capacity of @ref mway_entry array in an mway tree node.
 * @param[in] footer_size Additional field to generalize more.
 * @return sizeof an mway tree node. 
 */
static inline size_t mway_sizeof(size_t capacity, size_t footer_size)
{
    size_t raw_size = sizeof(struct mway_header) + capacity * sizeof(struct mway_entry) + footer_size;
    return ALIGN_UP(raw_size, ALIGN_REQ);
}

/** @} */ // End of Create & Destroy

/**
 * @name Getters & Setters
 * Functions for get and set things.
 * @{
 */

/** Helper function prototype */
static inline struct mway_entry* mway_base(struct mway_header* header);

/** @return The capacity of the node. */
static inline size_t mway_capacity(struct mway_header* header);

/** @return Copy of the entry at the specified index. */
static inline struct mway_entry mway_get_entry(struct mway_header* header, size_t index)
{
    assert(index < header->capacity);
    return mway_base(header)[index];
}

/** @brief Writes a new entry structure to the specified index. */
static inline void mway_set_entry(struct mway_header* header, size_t index, struct mway_entry* new_entry)
{
    assert(index < header->capacity);
    mway_base(header)[index] = *new_entry;
}

/** @return Pointer to the child node at the specified index. */
static inline struct mway_header* mway_get_child(struct mway_header* header, size_t index)
{
    assert(index < header->capacity);
    return mway_base(header)[index].child;
}

/** @return Const pointer to the child node at the specified index. */
static inline const struct mway_header* mway_get_child_const(const struct mway_header* header, size_t index)
{
    assert(index < header->capacity);
    return (const struct mway_header*) mway_base((struct mway_header*) header)[index].child;
}

/** @brief Sets the child pointer at the specified index. */
static inline void mway_set_child(struct mway_header* header, size_t index, struct mway_header* child)
{
    assert(index < header->capacity);
    mway_base(header)[index].child = child;
}

/** @return Pointer to the data at the specified index. */
static inline void* mway_get_data(struct mway_header* header, size_t index)
{
    assert(index < header->capacity);
    return mway_base(header)[index].data;
}

/** @return Const pointer to the data at the specified index. */
static inline const void* mway_get_data_const(const struct mway_header* header, size_t index)
{
    assert(index < header->capacity);
    return mway_base((struct mway_header*) header)[index].data;
}

/** @brief Sets the data pointer at the specified index. */
static inline void mway_set_data(struct mway_header* header, size_t index, void* data)
{
    assert(index < header->capacity);
    mway_base(header)[index].data = data;
}

/** @return Pointer to the custom footer area following the entries. */
static inline void* mway_get_footer(struct mway_header* header)
{
    return (void*) (mway_base(header) + header->capacity);
}

/** @return Const pointer to the custom footer area. */
static inline const void* mway_get_footer_const(const struct mway_header* header)
{
    return (const void*) (mway_base((struct mway_header*) header) + header->capacity);
}

/** @return Address of the entry structure at the specified index. */
static inline struct mway_entry* mway_get_entry_addr(struct mway_header* header, size_t index)
{
    assert(index < header->capacity);
    return &mway_base(header)[index];
}

/** @} */ // End of Getters & Setters

// *** Helper functions *** //

/** @return Pointer to the start of the entry array. */
static inline struct mway_entry* mway_base(struct mway_header* header)
{
    // Calculate address immediately after header
    size_t header_size = sizeof(struct mway_header);
    // Align that address up to the requirements of the entry struct
    size_t offset = ALIGN_UP(header_size, sizeof(void*));
    return (struct mway_entry*) ((char*) header + offset);
}

/** @return The capacity stored in the header. */
static inline size_t mway_capacity(struct mway_header* header)
{
    return header->capacity;
}

/** @} */ // End of MWAYTREE group

#ifdef __cplusplus
}
#endif

#endif // TREES_MWAYTREE_H