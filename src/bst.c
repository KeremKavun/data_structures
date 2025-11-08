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

static struct bst_node** bst_search_helper(struct bst* btree, void* data);
static void bst_walk_subtree(struct bst_node* node, void* userdata, void (*handler) (void* item, void* userdata));
static struct bst_node** bst_findmin(struct bst_node* node);
static struct bst_node** bst_findmax(struct bst_node* node);

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

struct bst* bst_create(int (*cmp) (const void* key, const void* data), void (*deallocator) (void* item, void* userdata))
{   
    struct chunked_pool* pool = malloc(sizeof(struct chunked_pool));
    if (!pool)
    {
        LOG(LIB_LVL, CERROR, "Failed to allocate memory for chunked pool");
        return NULL;
    }
    chunked_pool_init(pool, CHUNKED_POOL_SIZE, sizeof(struct bst_node), deallocator);
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

void bst_destroy(struct bst* btree)
{
    chunked_pool_destroy(btree->pool);
    free(btree->pool);
    free(btree);
}

/*───────────────────────────────────────────────
 * Operations
 *───────────────────────────────────────────────*/

int bst_add(struct bst* btree, void* new_data)
{
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
            return 1;
        }
    }
    struct bst_node* new_node = chunked_pool_alloc(btree->pool);
    if (!new_node)
    {
        LOG(LIB_LVL, CERROR, "Failed to allocate memory for node");
        return 1;
    }
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->data = new_data;
    *curr = new_node;
    btree->size++;
    return 0;
}

int bst_remove(struct bst* btree, void* data)
{
    struct bst_node** target = bst_search_helper(btree, data);
    if (!target)
        return 1;
    struct bst_node** min = bst_findmin((*target)->right);
    struct bst_node* del = *target;
    *target = *min;
    chunked_pool_free(btree->pool, del);
    *min = NULL;
    btree->size--;
    return 0;
}

void* bst_search(struct bst* btree, void* data)
{
    struct bst_node** target = bst_search_helper(btree, data);
    if (!target)
        return NULL;
    return (*target)->data;
}

/*───────────────────────────────────────────────
 * Accessors
 *───────────────────────────────────────────────*/

int bst_empty(struct bst* btree)
{
    return btree->root == NULL;
}

size_t bst_size(struct bst* btree)
{
    return btree->size;
}

/*───────────────────────────────────────────────
 * Iterations
 *───────────────────────────────────────────────*/

void bst_walk(struct bst* btree, void* userdata, void (*handler) (void* item, void* userdata))
{
    bst_walk_subtree(btree->root, userdata, handler);
}

// *** Helper functions *** //

static struct bst_node** bst_search_helper(struct bst* btree, void* data)
{
    struct bst_node** curr = &btree->root;
    while (*curr)
    {
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

static void bst_walk_subtree(struct bst_node* node, void* userdata, void (*handler) (void* item, void* userdata))
{
    if (!node)
        return;
    bst_walk_subtree(node->left, userdata, handler);
    handler(node->data, userdata);
    bst_walk_subtree(node->right, userdata, handler);
}

static struct bst_node** bst_findmin(struct bst_node* node)
{
    struct bst_node** curr = &node;
    while ((*curr)->left)
        curr = &(*curr)->left;
    return curr;
}

static struct bst_node** bst_findmax(struct bst_node* node)
{
    struct bst_node** curr = &node;
    while ((*curr)->right)
        curr = &(*curr)->right;
    return curr;
}
