#ifndef BST_H
#define BST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../debug/include/debug.h"
#include <stddef.h>

struct bst;
typedef struct bst bst_t;

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

struct bst* bst_create(int (*cmp) (const void* key, const void* data), void (*deallocator) (void* item, void* userdata));
void bst_destroy(struct bst* btree);

/*───────────────────────────────────────────────
 * Operations
 *───────────────────────────────────────────────*/

int bst_add(struct bst* btree, void* new_data);
int bst_remove(struct bst* btree, void* data);
void* bst_search(struct bst* btree, void* data);

/*───────────────────────────────────────────────
 * Accessors
 *───────────────────────────────────────────────*/

int bst_empty(struct bst* btree);
size_t bst_size(struct bst* btree);

/*───────────────────────────────────────────────
 * Iterations
 *───────────────────────────────────────────────*/

void bst_walk(struct bst* btree, void* userdata, void (*handler) (void* item, void* userdata));

#ifdef __cplusplus
}
#endif

#endif // BST_H