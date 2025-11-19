#ifndef AVL_H
#define AVL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../debug/include/debug.h"
#include "../../concepts/include/object_concept.h"
#include "../include/bintree.h"
#include <stddef.h>

struct avl;
typedef struct avl avl_t;

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

// Creates avl and returns, NULL in case of error, if capacity_of_pool is 1, using malloc, else chunked_pool
struct avl* avl_create(int (*cmp) (const void* key, const void* data), struct object_concept* oc);
void avl_destroy(struct avl* btree, void* context);

/*───────────────────────────────────────────────
 * Operations
 *───────────────────────────────────────────────*/

enum trees_status avl_add(struct avl* btree, void* new_data);
enum trees_status avl_remove(struct avl* btree, void* data);
void* avl_search(struct avl* btree, const void* data);

/*───────────────────────────────────────────────
 * Accessors
 *───────────────────────────────────────────────*/

int avl_empty(const struct avl* btree);
size_t avl_size(const struct avl* btree);
void* avl_min(struct avl* btree);
void* avl_max(struct avl* btree);

/*───────────────────────────────────────────────
 * Iterations
 *───────────────────────────────────────────────*/

void avl_walk(struct avl* btree, void* userdata, void (*handler) (void* item, void* userdata), enum traversal_order order);

#ifdef __cplusplus
}
#endif

#endif // AVL_H