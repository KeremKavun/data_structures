#ifndef TREES_BST_H
#define TREES_BST_H

#include <ds/utils/debug.h>
#include <ds/utils/allocator_concept.h>
#include <ds/utils/object_concept.h>
#include "bintree.h"
#include "common/traversals.h"
#include "common/status.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file bst.h
 * @brief Defines the interface for binary search trees.
 */

/**
 * @defgroup BST Binary Search Tree
 * @ingroup BINTREE_CORE
 * @brief Basic operations for binary search tree.
 * 
 * @details
 * ### Global Constraints
 * - **NULL Pointers**: All `struct bst *tree` arguments must be non-NULL.
 * - **Ownership**: Internal nodes are owned by allocator_concept given by user, stored in the bst,
 * - void *references to data are entirely owned by user. @ref bst_destroy might be helpful to destruct remaining
 * - objects in the tree.
 * @{
 */

/**
 * @struct bst
 * @brief Aggregation of generic binary tree.
 */
struct bst {
    struct bintree                  *root;              ///< Root of the tree.
    struct allocator_concept        ac;                 ///< Used by the tree to allocate new nodes to maintain the tree.
    int (*cmp) (const void *key, const void *data);     ///< Pointer to function that returns negative if a<b, 0 if a==b, positive if a>b. 
    size_t                          size;               ///< Count of the objects whose references are stored here.
};

/**
 * @name Create & Destroy
 * Functions for setting up the tree.
 * @{
 */

/**
 * @brief Creates the bst.
 * @param[in] cmp Function pointer to compare keys.
 * @param[in] ac allocator_concept to create tree nodes, must be non-NULL and valid.
 * @return Bst, NULL if not successful.
 */
struct bst *bst_create(int (*cmp) (const void *key, const void *data), struct allocator_concept *ac);

/**
 * @brief Destroys the bst.
 * @param[in] oc object_concept to deinit data references.
 */
void bst_destroy(struct bst *tree, struct object_concept *oc);

/** @return sizeof(struct bintree) */
size_t bst_node_sizeof();

/** @} */ // End of Create & Destroy

/**
 * @name Operations
 * Functions for add, remove and search.
 * @{
 */

/**
 * @brief Adds new data into the bst.
 * @param[in] new_data Reference to the new data.
 * @return enum tree_status, which might indicate
 * duplicate or memory allocation failure.
 */
enum trees_status bst_add(struct bst *tree, void *new_data);

/**
 * @brief Removes data from the bst.
 * @param[in] data Data to be removed.
 * @return Data that was stored in the bst or NULL if doesnt exist.
 * @warning **Lifetime Management**: The tree did NOT take ownership of the memory pointed
 * by `void *new_data` passed in insert functions. It is returned to you back.
 */
void *bst_remove(struct bst *tree, void *data);

/**
 * @brief Searches a given data in the bst.
 * @param[in] data Data that is going to be searched.
 * @return Data that was stored in the bst or NULL if doesnt exist.
 */
void *bst_search(struct bst *tree, const void *data);

/** @} */ // End of Operations

/**
 * @name Inspection
 * Functions to query the tree.
 * @{
 */

/** @return The root of the bst. */
static inline struct bintree* bst_root(struct bst *tree)
{
    return tree->root;
}

/** @return 1 if empty, 0 otherwise. */
static inline int bst_empty(const struct bst *tree)
{
    return tree->root == NULL;
}

/** @return size of the bst */
static inline size_t bst_size(const struct bst *tree)
{
    return tree->size;
}

/** @} */ // End of Inspection

/**
 * @name Traversal
 * Functions for traversals.
 * @{
 */

// Use generic binary tree algorithms, since this is aggregate
// and technically inherits.

/** @} */ // End of Traversal

/** @} */ // End of BST group

#ifdef __cplusplus
}
#endif

#endif // TREES_BST_H