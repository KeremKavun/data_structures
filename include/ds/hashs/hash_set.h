#ifndef HASH_SET_H
#define HASH_SET_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ds/utils/debug.h>
#include <stddef.h>

struct hash_set;
typedef struct hash_set hash_set_t;

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

int hash_set_init(struct hash_set* hs, size_t (*hash) (const void* key, size_t capacity, size_t attempts), int (*cmp_key) (const void* obj1, const void* obj2));
struct hash_set* hash_set_create(size_t (*hash) (const void* key, size_t capacity, size_t attempts), int (*cmp_key) (const void* obj1, const void* obj2));
// Frees contents of the hash_set, but it shouldnt be used if a key or value is allocated in the heap and
// pointer to that space is stored only here, otherwise you will lose the pointer and never able to free the resource
void hash_set_deinit(struct hash_set* hs, void* userdata, void (*deallocator) (void* item, void* userdata));
void hash_set_destroy(struct hash_set* hs, void* userdata, void (*deallocator) (void* item, void* userdata));

/*───────────────────────────────────────────────
 * Insertion & removal
 *───────────────────────────────────────────────*/

// Inserts new item into hash table, returns 0 if it succedds, 1 otherwise
int hash_set_insert(struct hash_set* hs, void* item);
// Deletes given key, returns 0 if it succedds, 1 otherwise
int hash_set_delete(struct hash_set* hs, const void* item);

/*───────────────────────────────────────────────
 * Properties
 *───────────────────────────────────────────────*/

// Returns size of the hash_set
size_t hash_set_size(const struct hash_set* hs);
// Returns capacity of the hash_set
size_t hash_set_capacity(const struct hash_set* hs);

/*───────────────────────────────────────────────
 * Search
 *───────────────────────────────────────────────*/

// Searches given key, returns void* value if it succedds, NULL otherwise
void* hash_set_search(struct hash_set* hs, const void* item);

/*───────────────────────────────────────────────
 * Iteration
 *───────────────────────────────────────────────*/

// Walks through hash_set and executes given function ptr on objects' pointers stored in the hash_set
void hash_set_walk(struct hash_set* hs, void* userdata, void (*exec) (void* item, void* userdata));

#ifdef __cplusplus
}
#endif

#endif // HASH_SET_H