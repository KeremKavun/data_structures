#ifndef TREES_BTREE_H
#define TREES_BTREE_H

#include <ds/utils/debug.h>
#include <ds/utils/allocator_concept.h>
#include <ds/utils/object_concept.h>
#include "common/traversals.h"
#include "common/status.h"
#include "mwaytree.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file Btree.h
 * @brief Defines the interface for B-trees.
 */

/**
 * @defgroup BTREE B-Tree
 * @ingroup MWAYTREE_CORE
 * @brief Basic operations for B-trees.
 * 
 * @details
 * ### Global Constraints
 * - **NULL Pointers**: All `struct Btree *tree` arguments must be non-NULL.
 * - **Ownership**: Internal nodes are owned by allocator_concept given by user, stored in the avl,
 * - void *references to data are entirely owned by user. @ref avl_destroy might be helpful to destruct remaining
 * - objects in the tree.
 * @{
 */

/**
 * @struct Btree
 */
struct Btree {
    struct mway_header              *root;              ///< Root of the tree.
    struct allocator_concept        ac;                 ///< Used by the tree to allocate new nodes to maintain the tree.
    int (*cmp) (const void *key, const void *data);     ///< Pointer to function that returns negative if a<b, 0 if a==b, positive if a>b. 
    size_t                          size;               ///< Count of the objects whose references are stored here.
};

/**
 * @name Initialize & Deinitialize
 * Functions for setting up the tree.
 * @{
 */

/**
 * @brief Initializes B-tree.
 * @param[in, out] tree Pointer to Btree instance.
 * @param[in] order Order of B-tree.
 * @param[in] cmp Function pointer to compare keys.
 * @param[in] ac allocator_concept to create tree nodes, must be non-NULL and valid.
 * Use @ref Btree_node_sizeof() to pass object size into your allocator that will allocate Btree nodes.
 * @return 0 on success, non-zero otherwise.
 */
int Btree_init(struct Btree *tree, size_t order, int (*cmp) (const void *key, const void *data), struct allocator_concept *ac);

/**
 * @brief Deinitializes the B-tree.
 * @param[in] oc object_concept to deinit data references.
 * @warning Only root is set to NULL after freeing the internal tree.
 */
void Btree_deinit(struct Btree *tree, struct object_concept *oc);

/**
 * @return @ref mway_sizeof(order - 1, sizeof(struct mway_header*) + sizeof(size_t))
 * @see mway_sizeof
 */
static inline size_t Btree_node_sizeof(size_t order)
{
    // order -1 data and entry, but adding one child to make it B-tree node with one size_t
    return mway_sizeof(order - 1, sizeof(struct mway_header*) + sizeof(size_t));
}

/** @} */ // End of Initialize & Deinitialize

/**
 * @name Operations
 * Functions for add, remove and search.
 * @{
 */

/**
 * @brief Adds new data into the Btree.
 * @param[in] new_data Reference to the new data.
 * @return enum tree_status, which might indicate
 * duplicate or memory allocation failure.
 * @warning Unfortunately, i cant provide strong guarantee here,
 * in case of any memory allocaton failure, tree will be corrupted.
 */
enum trees_status Btree_add(struct Btree *tree, void *new_data);

/**
 * @brief Removes data from the Btree.
 * @param[in] data Data to be removed.
 * @return Data that was stored in the Btree or NULL if doesnt exist.
 * @warning **Lifetime Management**: The tree did NOT take ownership of the memory pointed
 * by `void *new_data` passed in insert functions. It is returned to you back.
 */
void *Btree_remove(struct Btree *tree, void *data);

/**
 * @brief Searches a given data in the Btree.
 * @param[in] data Data that is going to be searched.
 * @return Data that was stored in the Btree or NULL if doesnt exist.
 */
void *Btree_search(struct Btree *tree, const void *data);

/** @} */ // End of Operations

/**
 * @name Inspection
 * Functions to query the tree.
 * @{
 */

/** @return The root of the Btree. */
static inline const struct mway_header* Btree_root(const struct Btree* tree)
{
    return tree->root;
}

/** @return 1 if the tree is empty, 0 otherwise. */
static inline int Btree_empty(const struct Btree* tree)
{
    return tree->size == 0;
}

/** @return The count of the object references stored here. */
static inline size_t Btree_size(const struct Btree* tree)
{
    return tree->size;
}

/** @return Order of the Btree. */
static inline size_t Btree_order(const struct Btree* tree)
{
    return tree->root->capacity + 1;
}

/** @} */ // End of Inspection

/**
 * @name Traversal
 * Functions for traversals.
 * @{
 */

/**
 * @brief Traverses tree in the inorder.
 * @param[in] context Context pointer for ease.
 * @param[in] handler Pointer to a function pointer that executes
 * taking data reference and context pointer.
 */
void Btree_walk(struct Btree *tree, void *context, void (*handler) (void *data, void *context));

/** @} */ // End of Traversal

/** @} */ // End of BTREE group

#ifdef __cplusplus
}
#endif

#endif // TREES_BTREE_H