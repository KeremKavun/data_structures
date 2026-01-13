#ifndef HASH_CONCEPT_H
#define HASH_CONCEPT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup HASH_CONCEPT Hash Concept
 * @ingroup UTILS
 * @brief Metadata for hashing and key comparison.
 * @{
 */

/**
 * @brief Signature for a hash function.
 * @param key The key to hash.
 * @param capacity Current table capacity.
 * @param attempts Number of collisions encountered (for open addressing).
 */
typedef size_t (*hash_fn)(const void* key, size_t capacity, size_t attempts);

/**
 * @struct hash_concept
 * @brief Bundles hashing and comparison logic for keys.
 */
struct hash_concept {
    hash_fn hash;                                       ///< The hashing algorithm.
    int (*cmp_key) (const void* a, const void* b);      ///< Key equality comparator.
};

/** @} */

#ifdef __cplusplus
}
#endif

#endif // HASH_CONCEPT_H