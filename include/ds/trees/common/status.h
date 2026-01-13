#ifndef TREES_COMMON_STATUS_H
#define TREES_COMMON_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file status.h
 * @brief Common return codes for tree operations.
 */

/**
 * @defgroup TREE_COMMON Common Utilities
 * @ingroup TREES
 * @brief Shared enums, macros, and traversal definitions used across all tree implementations.
 * @{
 */

/**
 * @enum trees_status
 * @brief Defines return status of *_add functions.
 */
enum trees_status {
    TREES_OK,                   ///< Operation successful.
    TREES_NOT_FOUND,            ///< Key was not found in the tree.
    TREES_DUPLICATE_KEY,        ///< Insertion failed; key already exists.
    TREES_SYSTEM_ERROR          ///< Malloc failed or other system issue.
};

/** @} */ // End of TREE_COMMON

#ifdef __cplusplus
}
#endif

#endif // TREES_COMMON_STATUS_H