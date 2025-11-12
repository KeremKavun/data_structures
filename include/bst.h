#ifndef BST_H
#define BST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../debug/include/debug.h"
#include "../../allocators/include/chunked_pool.h"
#include <stddef.h>

enum bst_traversal_order
{
    PREORDER,
    INORDER,
    POSTORDER
};

enum bst_result_status
{
    OK,
    NOT_FOUND,
    DUPLICATE_KEY,
    SYSTEM_ERROR
};

struct bst;
typedef struct bst bst_t;

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

// Creates bst and returns, NULL in case of error, if capacity_of_pool is 1, using malloc, else chunked_pool
struct bst* bst_create(int (*cmp) (const void* key, const void* data), struct chunked_pool* pool);
void bst_destroy(struct bst* btree);

/*───────────────────────────────────────────────
 * Operations
 *───────────────────────────────────────────────*/

enum bst_result_status bst_add(struct bst* btree, void* new_data);
enum bst_result_status bst_remove(struct bst* btree, void* data);
void* bst_search(struct bst* btree, const void* data);

/*───────────────────────────────────────────────
 * Accessors
 *───────────────────────────────────────────────*/

int bst_empty(const struct bst* btree);
size_t bst_size(const struct bst* btree);
void* bst_min(struct bst* btree);
void* bst_max(struct bst* btree);

/*───────────────────────────────────────────────
 * Iterations
 *───────────────────────────────────────────────*/

void bst_walk(struct bst* btree, void* userdata, void (*handler) (void* item, void* userdata), enum bst_traversal_order order);

#ifdef __cplusplus
}
#endif

#endif // BST_H