#ifndef TREES_BST_H
#define TREES_BST_H

#include <ds/utils/debug.h>
#include <ds/utils/object_concept.h>
#include <ds/utils/macros.h>
#include "bintree.h"
#include "common/traversals.h"
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
 * - **Ownership**: Internal nodes are owned by you, since nodes are intrusive.
 * - YOU KNOW HOW YOU MANAGE YOUR DATA, so you should what to pass into .deinit method
 * - @ref bst_deinit might be helpful to destruct remaining objects in the tree.
 * @{
 */

/**
 * @struct bst
 * @brief Aggregation of generic binary tree.
 */
struct bst {
    struct bintree      *root;      ///< Root of the tree.
    size_t              size;       ///< Count of the objects whose references are stored here.
    bst_cmp_cb          cmp;        ///< Pointer to function that returns negative if a<b, 0 if a==b, positive if a>b.    
};

/**
 * @name Initialize & Deinitialize
 * Functions for setting up the tree.
 * @{
 */

/**
 * @brief Initializes the bst.
 * @param[in, out] tree Pointer to bst instance.
 * @param[in] cmp Function pointer to compare nodes.
 */
void bst_init(struct bst *tree, bst_cmp_cb cmp);

/**
 * @brief Deinitializes the bst.
 * @param[in] oc object_concept to deinit data references.
 * @warning Same warning with bintree module here. Since this
 * module uses intrusive nodes, you should be aware that this function
 * passes bintree into .deinit method, which you supllied, it is entirely
 * up to you how to use that passed node.
 * @warning Only passed trees root attributes and size are set to NULL.
 * @see bintree_deinit
 */
void bst_deinit(struct bst *tree, struct object_concept *oc);

/** @} */ // End of Initialize & Deinitialize

/**
 * @name Operations
 * Functions for add, remove and search.
 * @{
 */

/**
 * @brief Adds new node into the bst.
 * @param[in] new_node Hook to the new node.
 * @return 0 if success, 1 if duplicate.
 */
int bst_add(struct bst *tree, struct bintree *new_node);

/**
 * @brief Removes node from the bst.
 * @param[in] node Node to be removed, must be
 * obtained by the search function or be sure that
 * this is in the tree.
 */
void bst_remove(struct bst *tree, struct bintree *node);

/**
 * @brief Searches a given node in the bst.
 * @param[in] data Data that is going to be searched.
 * @return struct bintree * (hook to your data) or NULL if it doesnt exist.
 */
struct bintree *bst_search(struct bst *tree, const void *data, bintree_cmp_cb cmp);

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
    return tree->size == 0;
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