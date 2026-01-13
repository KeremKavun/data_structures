#ifndef TREES_COMMON_TRAVERSALS_H
#define TREES_COMMON_TRAVERSALS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file traversals.h
 * @brief Standard function signatures for tree traversals.
 */

/**
 * @addtogroup TREE_COMMON
 * @{
 */

/**
 * @enum traversal_order
 * @brief Defines codes for traversal orders for functions.
 */
enum traversal_order {
    PREORDER,       ///< Preorder traversal, visit root, then left child, then right child.
    INORDER,        ///< Postorder traversal, visit left child, then root, then right child.
    POSTORDER       ///< Postorder traversal, visit left child, then right child, then root.
};

/** @} */

#ifdef __cplusplus
}
#endif

#endif // TREES_COMMON_TRAVERSALS_H