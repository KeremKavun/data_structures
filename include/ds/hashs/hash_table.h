#ifndef HASHS_HASH_TABLE_H
#define HASHS_HASH_TABLE_H

#include <ds/utils/debug.h>
#include <ds/utils/object_concept.h>
#include <ds/utils/hash_concept.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file hash_table.h
 * @brief Defines the interface for hash table. Inspired by "https://github.com/jamesroutley/write-a-hash-table".
 */

/**
 * @defgroup HASHTABLE Hash Table
 * @ingroup HASHS
 * @brief Key-Value pair container.
 * 
 * @details
 * ### Global Constraints
 * - **NULL Pointers**: All `struct hash_table *ht`, `void *key` and `void *value` poiners must be non-NULL and valid.
 * - **Ownership**: Hash table stores references, objects should not go out of scope or freed before hash_table is freed.
 * @{
 */

/**
 * @struct hash_table
 * @brief Opaque handle for the Hash Table ADT.
 */
struct hash_table;

/**
 * @name Create & Destroy
 * @{
 */

/**
 * @brief Creates the hash table with given concepts.
 * @param[in] hc Pointer to hash_concept. Must be non-NULL and valid.
 * @return Pointer to struct hash_table instance.
 */
struct hash_table* hash_table_create(struct hash_concept *hc);

/**
 * @brief Destroys the table and its contents.
 * @param[in,out] ht The table instance.
 * @param[in] oc Pointer to object_concept to deinit key value pairs,
 * can be NULL if you store POD.
 */
void hash_table_destroy(struct hash_table* ht, struct object_concept *oc);

/** @} */ // End of Create & Destroy

/**
 * @name Insertion & Removal
 * @{
 */

/**
 * @brief Inserts new key-value pair into the hash table.
 * @param[in] key Reference to key object.
 * @param[in] value Reference to value object.
 * @return 0 if succeeds, non-zero otherwise. Resizing might cause
 * error (system error).
 * @note This function provides strong guarantee
 */
int hash_table_insert(struct hash_table* ht, void* key, void* value);

// Deletes given key, returns 0 if it succedds, 1 otherwise
/**
 * @brief Removes given key from the table.
 * @param[in] key Pointer to object whose value is same with
 * target key's value.
 * @return 0 if succeeds, non-zero otherwise. Resizing might cause
 * error (system error).
 * @note This function provides strong guarantee
 */
int hash_table_remove(struct hash_table* ht, const void* key);

/** @} */ // End of Insertion & Removal

/**
 * @name Properties
 * @{
 */

/** @return Count of the objects whose references are stored here */
size_t hash_table_size(const struct hash_table* ht);

/** @return Current internal bucket capacity */
size_t hash_table_capacity(const struct hash_table* ht);

/** @} */ // End of Properties

/**
 * @name Search
 * @{
 */

/**
 * @brief Searches a key.
 * @param[in] key Key to be searched.
 * @return Keys value.
 */
void* hash_table_search(struct hash_table* ht, const void* key);

/** @} */ // End of Search

/**
 * @name Iteration
 * @{
 */

/**
 * @brief Iterates over the the hash table.
 * @param[in] context Pointer to an arbitrary context for ease.
 * @param[in] handler Pointer to a function pointer that executes
 * taking key reference, value reference and context pointer.
 */
void hash_table_walk(struct hash_table* ht, void* context, void (*exec) (void* key, void* value, void* context));

/** @} */ // End of Iteration

/** @} */ // End of HASHS group

// sample hash implementation
/*
#define HT_PRIME_1 151
#define HT_PRIME_2 217

size_t hash_str(const char* s, int p, int capacity)
{
   unsigned long long hash = 0;
   while (*s) {
      hash = hash * p + *s;
      s++;
   }
   return (size_t) hash % capacity;
}

// will be passed into hash_concept as hash algorithm
size_t double_hash(const void* obj, size_t capacity, size_t attempts)
{
   size_t hash_a = hash_str((const char*) obj, HT_PRIME_1, capacity);
   size_t hash_b = hash_str((const char*) obj, HT_PRIME_2, capacity);
   return (hash_a + attempts * (hash_b + 1)) % capacity;
}

// Simple string compare function
int string_cmp(const void* _obj1, const void* _obj2)
{
   return strcmp((const char*)_obj1, (const char*)_obj2);
}

struct hash_concept hc = { .hash = double_hash, .cmp_key = string_cmp };
*/

#ifdef __cplusplus
}
#endif

#endif // HASHS_HASH_TABLE_H