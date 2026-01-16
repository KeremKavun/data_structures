#ifndef TREES_BINTREE_H
#define TREES_BINTREE_H

#include <ds/utils/debug.h>
#include <ds/utils/allocator_concept.h>
#include <ds/utils/object_concept.h>
#include "common/traversals.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file bintree.h
 * @brief Defines the interface for basic binary tree.
 */

/**
 * @defgroup BINTREE_CORE Binary Trees
 * @ingroup TREES
 * @brief Collection of binary trees (bst, avl, heap)
 */

/**
 * @defgroup BINTREE Basic Binary Tree
 * @ingroup BINTREE_CORE
 * @brief Basic operations for the generic binary tree.
 * 
 * @details
 * ### Global Constraints
 * - **NULL Pointers**: All `struct bintree *tree` arguments must be non-NULL.
 * - **Ownership**: Internal nodes are owned by allocator_concept given by user,
 * - void *references to data are entirely owned by user. @ref bintree_destroy might be helpful to destruct remaining
 * - objects in the tree.
 * @{
 */

/**
 * @struct bintree
 * @brief Simple binary tree/node.
 */
struct bintree {
    struct bintree      *left;          ///< Left node.
    struct bintree      *right;         ///< Right node.
    void                *data;          ///< Reference to the object
};

/**
 * @name Create & Destroy
 * Functions for setting up the tree.
 * @{
 */

/**
 * @brief Creates a binary tree on the heap.
 * @param[in] left Left child of the tree.
 * @param[in] right Right child of the tree.
 * @param[in] data Reference to the data to store.
 * @param[in] ac allocator_concept to create the tree, must be non-NULL and valid.
 * @return Binary tree, NULL if not successful.
 */
struct bintree *bintree_create(struct bintree *left, struct bintree *right, void *data, struct allocator_concept* ac);

/**
 * @brief Destroys the binary tree.
 * @param[in, out] tree Pointer to the binary tree instance.
 * @param[in] oc object_concept to deinit data references. Might be NULL
 * .deinit method might be NULL, .init isnt used.
 * @param[in] ac allocator_concept to return memory back, must be non NULL
 * and valid, and same as passed into the @ref bintree_create method.
 */
void bintree_destroy(struct bintree *tree, struct object_concept *oc, struct allocator_concept *ac);

/** @} */ // End of Create & Destroy

/**
 * @name Getters & Setters
 * Functions for get and set things.
 * @{
 */

/** @return left child. */
static inline struct bintree* bintree_get_left(struct bintree* tree)
{
    return tree->left;
}

/** @return const left child. */
static inline const struct bintree* bintree_get_left_const(const struct bintree* tree)
{
    return tree->left;
}

/** @brief Sets left child. */
static inline void bintree_set_left(struct bintree* tree, struct bintree* left)
{
    tree->left = left;
}

/** @return right child. */
static inline struct bintree* bintree_get_right(struct bintree* tree)
{
    return tree->right;
}

/** @return const right child. */
static inline const struct bintree* bintree_get_right_const(const struct bintree* tree)
{
    return tree->right;
}

/** @brief Sets right child. */
static inline void bintree_set_right(struct bintree* tree, struct bintree* right)
{
    tree->right = right;
}

/** @return reference to the data. */
static inline void* bintree_get_data(struct bintree* tree)
{
    return tree->data;
}

/** @return const reference to the data. */
static inline const void* bintree_get_data_const(const struct bintree* tree)
{
    return tree->data;
}

/** @brief Sets the reference. */
static inline void bintree_set_data(struct bintree* tree, void* data)
{
    tree->data = data;
}

/** @} */ // End of Getters & Setters

/**
 * @name Traversal
 * Functions for traverse binary trees.
 * @{
 */

/**
 * @brief Traverses tree in the given order
 * @param[in] context Context pointer for ease.
 * @param[in] handler Pointer to a function pointer that executes
 * taking data reference and context pointer.
 * @param[in] order Defines the traversel order,
 * preorder, inorder or postorder?
 */
void bintree_traverse(struct bintree *tree, void *context, void (*handler) (void *data, void *context), enum traversal_order order);

/** @brief Traverses the tree in BFS */
int bintree_bfs(struct bintree *tree, void *context, void (*handler) (void *data, void *context));

/** @brief Traverses the tree in DFS */
int bintree_dfs(struct bintree *tree, void *context, void (*handler) (void *data, void *context));

/** @} */ // End of Traversal

/**
 * @name Properties
 * Functions for additional information about the
 * tree, but they might be costful and heavy.
 * @{
 */

size_t bintree_size(const struct bintree *tree);
int bintree_height(const struct bintree *tree);
int bintree_balance_factor(const struct bintree *tree);

/** @} */ // End of Properties

/** @} */ // End of BINTREE group

#ifdef __cplusplus
}
#endif

#endif // TREES_BINTREE_H