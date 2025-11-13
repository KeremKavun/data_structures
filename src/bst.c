#include "../include/bst.h"
#include <stdlib.h>

struct bst_node
{
    struct bst_node* left;
    struct bst_node* right;
    void* data;
};

struct bst
{
    struct allocator_concept ac;
    struct bst_node* root;
    int (*cmp) (const void* key, const void* data);
    size_t size;
};

static struct bst_node** bst_search_helper(struct bst* btree, const void* data);
static void bst_walk_subtree(struct bst_node* node, void* userdata, void (*handler) (void* item, void* userdata), int *func_index_array);
static struct bst_node** bst_findmin(struct bst_node** node_ref);
static struct bst_node** bst_findmax(struct bst_node** node_ref);
static void free_node(void* item, void* userdata);

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

struct bst* bst_create(int (*cmp) (const void* key, const void* data), struct allocator_concept ac)
{   
    struct bst* btree = malloc(sizeof(struct bst));
    if (!btree)
    {
        LOG(LIB_LVL, CERROR, "Failed to allocate memory for btree");
        return NULL;
    }
    btree->ac = ac;
    btree->root = NULL;
    btree->cmp = cmp;
    btree->size = 0;
    return btree;
}

void bst_destroy(struct bst* btree)
{
    if (!btree->ac.allocator)
        bst_walk(btree, NULL, free_node, INORDER);
    free(btree);
}

/*───────────────────────────────────────────────
 * Operations
 *───────────────────────────────────────────────*/

enum bst_result_status bst_add(struct bst* btree, void* new_data)
{
    LOG(LIB_LVL, CINFO, "Adding new data at %p", new_data);
    struct bst_node** curr = bst_search_helper(btree, new_data);
    if (*curr)
    {
        LOG(LIB_LVL, CERROR, "Duplicate key");
        return DUPLICATE_KEY;
    }
    struct bst_node* new_node = (btree->ac.allocator) ? btree->ac.alloc(btree->ac.allocator) : malloc(sizeof(struct bst_node));
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
    if (!(*target))
    {
        LOG(LIB_LVL, CERROR, "Key not found");
        return NOT_FOUND;
    }
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
    (btree->ac.allocator) ? btree->ac.free(btree->ac.allocator, node) : free(node);
    btree->size--;
    return OK;
}

void* bst_search(struct bst* btree, const void* data)
{
    struct bst_node** target = bst_search_helper(btree, data);
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

static void free_node(void* item, void* userdata)
{
    (void)userdata;
    free(item);
}
