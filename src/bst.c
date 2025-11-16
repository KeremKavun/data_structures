#include "../include/bst.h"
#include <stdlib.h>

struct bst
{
    struct bintree* root;
    struct allocator_concept* ac;
    int (*cmp) (const void* key, const void* data);
    size_t size;
};

static struct bintree** bst_search_helper(struct bst* btree, const void* data);
static struct bintree** bst_findmin(struct bintree** node_ref);
static struct bintree** bst_findmax(struct bintree** node_ref);
static void free_node(void* item, void* userdata);

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

struct bst* bst_create(int (*cmp) (const void* key, const void* data), struct allocator_concept* ac)
{   
    struct bst* btree = malloc(sizeof(struct bst));
    if (!btree)
    {
        LOG(LIB_LVL, CERROR, "Failed to allocate memory for btree");
        return NULL;
    }
    btree->root = NULL;
    btree->ac = ac;
    btree->cmp = cmp;
    btree->size = 0;
    return btree;
}

void bst_destroy(struct bst* btree)
{
    void (*deallocator) (void* item, void* userdata);
    if (!btree->ac)
        bst_walk(btree, NULL, free_node, POSTORDER);
    else if (btree->ac && !btree->ac->allocator)
        bst_walk(btree, NULL, (btree->ac && !btree->ac->free) ? free_node : btree->ac->free, POSTORDER);
    free(btree);
}

/*───────────────────────────────────────────────
 * Operations
 *───────────────────────────────────────────────*/

enum bst_result_status bst_add(struct bst* btree, void* new_data)
{
    LOG(LIB_LVL, CINFO, "Adding new data at %p", new_data);
    struct bintree** curr = bst_search_helper(btree, new_data);
    if (*curr)
    {
        LOG(LIB_LVL, CERROR, "Duplicate key");
        return DUPLICATE_KEY;
    }
    struct bintree* new_node = bintree_create(btree->ac);
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
    struct bintree** target = bst_search_helper(btree, data);
    if (!(*target))
    {
        LOG(LIB_LVL, CERROR, "Key not found");
        return NOT_FOUND;
    }
    struct bintree* node = *target;
    if (!node->left)
        *target = node->right;
    else if (!node->right)
        *target = node->left;
    else
    {
        struct bintree** min = bst_findmin(&node->right);
        struct bintree* successor = *min;
        *min = successor->right;
        node->data = successor->data;
        successor->data = NULL;
        node = successor;
    }
    (btree->ac && btree->ac->allocator) ? btree->ac->free(btree->ac->allocator, node) : free(node);
    btree->size--;
    return OK;
}

void* bst_search(struct bst* btree, const void* data)
{
    struct bintree** target = bst_search_helper(btree, data);
    if (!(*target))
    {
        LOG(LIB_LVL, CERROR, "Key not found");
        return NULL;
    }
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
    struct bintree** min_ref = bst_findmin(&btree->root);
    return min_ref ? (*min_ref)->data : NULL;
}

void* bst_max(struct bst* btree)
{
    struct bintree** max_ref = bst_findmax(&btree->root);
    return max_ref ? (*max_ref)->data : NULL;
}

/*───────────────────────────────────────────────
 * Iterations
 *───────────────────────────────────────────────*/

void bst_walk(struct bst* btree, void* userdata, void (*handler) (void* item, void* userdata), enum traversal_order order)
{
    bintree_walk(btree->root, userdata, handler, order);
}

// *** Helper functions *** //

static struct bintree** bst_search_helper(struct bst* btree, const void* data)
{
    struct bintree** curr = &btree->root;
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
    return curr;
}

static struct bintree** bst_findmin(struct bintree** node_ref)
{
    if (!node_ref || !*node_ref)
        return NULL;
    struct bintree** curr = node_ref;
    while ((*curr)->left)
        curr = &(*curr)->left;
    return curr;
}

static struct bintree** bst_findmax(struct bintree** node_ref)
{
    if (!node_ref || !*node_ref)
        return NULL;
    struct bintree** curr = node_ref;
    while ((*curr)->right)
        curr = &(*curr)->right;
    return curr;
}

static void free_node(void* item, void* userdata)
{
    (void)userdata;
    free(item);
}
