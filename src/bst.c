#include "../include/bst.h"
#include "../../allocators/include/chunked_pool.h"
#include <stdlib.h>

#define CHUNKED_POOL_SIZE 256

struct bst_node
{
    struct bst_node* left;
    struct bst_node* right;
    void* data;
};

struct bst
{
    struct chunked_pool* pool;
    struct bst_node* root;
    int (*cmp) (const void* key, const void* data);
    size_t size;
};

struct mini_lambda
{
    void (*func) (void* item, void* userdata);
};

static struct bst_node** bst_search_helper(struct bst* btree, const void* data);
static void bst_walk_subtree(struct bst_node* node, void* userdata, void (*handler) (void* item, void* userdata), int *func_index_array);
static struct bst_node** bst_findmin(struct bst_node** node_ref);
static struct bst_node** bst_findmax(struct bst_node** node_ref);
static void bst_node_deleter(void* item, void* userdata);

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

struct bst* bst_create(int (*cmp) (const void* key, const void* data))
{   
    struct chunked_pool* pool = malloc(sizeof(struct chunked_pool));
    if (!pool)
    {
        LOG(LIB_LVL, CERROR, "Failed to allocate memory for chunked pool");
        return NULL;
    }
    chunked_pool_init(pool, CHUNKED_POOL_SIZE, sizeof(struct bst_node), bst_node_deleter);
    struct bst* btree = malloc(sizeof(struct bst));
    if (!btree)
    {
        LOG(LIB_LVL, CERROR, "Failed to allocate memory for btree");
        free(pool);
        return NULL;
    }
    btree->pool = pool;
    btree->root = NULL;
    btree->cmp = cmp;
    btree->size = 0;
    return btree;
}

void bst_destroy(struct bst* btree, void (*deallocator) (void* item, void* userdata))
{
    struct mini_lambda lambda = {deallocator};
    chunked_pool_destroy(btree->pool, &lambda);
    free(btree->pool);
    free(btree);
}

/*───────────────────────────────────────────────
 * Operations
 *───────────────────────────────────────────────*/

enum bst_result_status bst_add(struct bst* btree, void* new_data)
{
    LOG(LIB_LVL, CINFO, "Adding new data at %p", new_data);
    struct bst_node** curr = &btree->root;
    while (*curr)
    {
        int result = btree->cmp(new_data, (*curr)->data);
        if (result < 0)
            curr = &(*curr)->left;
        else if (result > 0)
            curr = &(*curr)->right;
        else
        {
            LOG(LIB_LVL, CERROR, "Duplicate key");
            return DUPLICATE_KEY;
        }
    }
    struct bst_node* new_node = chunked_pool_alloc(btree->pool);
    if (!new_node)
    {
        LOG(LIB_LVL, CERROR, "Failed to allocate memory for node");
        return SYSTEM_ERROR;
    }
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->data = new_data;
    *curr = new_node;
    btree->size++;
    return OK;
}

enum bst_result_status bst_remove(struct bst* btree, void* data)
{
    LOG(LIB_LVL, CINFO, "Removing data at %p", data);
    struct bst_node** target = bst_search_helper(btree, data);
    if (!target)
        return NOT_FOUND;
    struct bst_node* node = *target;
    if (!node->left)
        *target = node->right;
    else if (!node->right)
        *target = node->left;
    else
    {
        struct bst_node** min = bst_findmin(&node->right);
        struct bst_node* successor = *min;
        *min = successor->right;
        node->data = successor->data;
        successor->data = NULL;
        node = successor;
    }
    chunked_pool_free(btree->pool, node);
    btree->size--;
    return OK;
}

void* bst_search(struct bst* btree, const void* data)
{
    struct bst_node** target = bst_search_helper(btree, data);
    if (!target)
        return NULL;
    return (*target)->data;
}

/*───────────────────────────────────────────────
 * Accessors
 *───────────────────────────────────────────────*/

int bst_empty(const struct bst* btree)
{
    return btree->root == NULL;
}

size_t bst_size(const struct bst* btree)
{
    return btree->size;
}

void* bst_min(struct bst* btree)
{
    struct bst_node** min_ref = bst_findmin(&btree->root);
    return min_ref ? (*min_ref)->data : NULL;
}

void* bst_max(struct bst* btree)
{
    struct bst_node** max_ref = bst_findmax(&btree->root);
    return max_ref ? (*max_ref)->data : NULL;
}

/*───────────────────────────────────────────────
 * Iterations
 *───────────────────────────────────────────────*/

void bst_walk(struct bst* btree, void* userdata, void (*handler) (void* item, void* userdata), enum bst_traversal_order order)
{
    int func_index_array[3] = {0, 0, 0};
    func_index_array[order] = 1;
    bst_walk_subtree(btree->root, userdata, handler, func_index_array);
}

// *** Helper functions *** //

static struct bst_node** bst_search_helper(struct bst* btree, const void* data)
{
    struct bst_node** curr = &btree->root;
    while (*curr)
    {
        printf("key: %d, curr: %d\n", *(int*)data, *(int*)(*curr)->data);
        int result = btree->cmp(data, (*curr)->data);
        if (result < 0)
            curr = &(*curr)->left;
        else if (result > 0)
            curr = &(*curr)->right;
        else
            return curr;
    }
    LOG(LIB_LVL, CERROR, "Key not found");
    return NULL;
}

static void bst_walk_subtree(struct bst_node* node, void* userdata, void (*handler) (void* item, void* userdata), int* func_index_array)
{
    if (!node)
        return;
    if (func_index_array[0])
        handler(node->data, userdata);
    bst_walk_subtree(node->left, userdata, handler, func_index_array);
    if (func_index_array[1])
        handler(node->data, userdata);
    bst_walk_subtree(node->right, userdata, handler, func_index_array);
    if (func_index_array[2])
        handler(node->data, userdata);
}

static struct bst_node** bst_findmin(struct bst_node** node_ref)
{
    if (!node_ref || !*node_ref)
        return NULL;
    struct bst_node** curr = node_ref;
    while ((*curr)->left)
        curr = &(*curr)->left;
    return curr;
}

static struct bst_node** bst_findmax(struct bst_node** node_ref)
{
    if (!node_ref || !*node_ref)
        return NULL;
    struct bst_node** curr = node_ref;
    while ((*curr)->right)
        curr = &(*curr)->right;
    return curr;
}

static void bst_node_deleter(void* item, void* userdata)
{
    struct bst_node* node = item;
    struct mini_lambda* lambda = userdata;
    lambda->func(node->data, NULL);
}
