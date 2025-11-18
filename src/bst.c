#include "../include/bst.h"
#include <stdlib.h>

struct bst
{
    struct bintree* root;
    struct object_concept* oc;
    int (*cmp) (const void* key, const void* data);
    size_t size;
};

static struct bintree** bst_search_helper(struct bst* tree, const void* data);
static struct bintree** bst_findmin(struct bintree** node_ref);
static struct bintree** bst_findmax(struct bintree** node_ref);

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

struct bst* bst_create(int (*cmp) (const void* key, const void* data), struct object_concept* oc)
{   
    struct bst* tree = malloc(sizeof(struct bst));
    if (!tree)
    {
        LOG(LIB_LVL, CERROR, "Failed to allocate memory for tree");
        return NULL;
    }
    tree->root = NULL;
    tree->oc = oc;
    tree->cmp = cmp;
    tree->size = 0;
    return tree;
}

void bst_destroy(struct bst* tree)
{
    bintree_destroy(tree->root, tree->oc);
    free(tree);
}

/*───────────────────────────────────────────────
 * Operations
 *───────────────────────────────────────────────*/

enum bst_result_status bst_add(struct bst* tree, void* new_data)
{
    LOG(LIB_LVL, CINFO, "Adding new data at %p", new_data);
    struct bintree** curr = bst_search_helper(tree, new_data);
    if (*curr)
    {
        LOG(LIB_LVL, CERROR, "Duplicate key");
        return DUPLICATE_KEY;
    }
    struct bintree* new_node = bintree_create(tree->oc);
    if (!new_node)
    {
        LOG(LIB_LVL, CERROR, "Failed to allocate memory for node");
        return SYSTEM_ERROR;
    }
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->data = new_data;
    *curr = new_node;
    tree->size++;
    return OK;
}

enum bst_result_status bst_remove(struct bst* tree, void* data)
{
    LOG(LIB_LVL, CINFO, "Removing data at %p", data);
    struct bintree** target = bst_search_helper(tree, data);
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
    if (tree->oc->destruct)
        tree->oc->destruct(node->data);
    (tree->oc && tree->oc->allocator) ? tree->oc->free(tree->oc->allocator, node) : free(node);
    tree->size--;
    return OK;
}

void* bst_search(struct bst* tree, const void* data)
{
    struct bintree** target = bst_search_helper(tree, data);
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

int bst_empty(const struct bst* tree)
{
    return tree->root == NULL;
}

size_t bst_size(const struct bst* tree)
{
    return tree->size;
}

void* bst_min(struct bst* tree)
{
    struct bintree** min_ref = bst_findmin(&tree->root);
    return min_ref ? (*min_ref)->data : NULL;
}

void* bst_max(struct bst* tree)
{
    struct bintree** max_ref = bst_findmax(&tree->root);
    return max_ref ? (*max_ref)->data : NULL;
}

/*───────────────────────────────────────────────
 * Iterations
 *───────────────────────────────────────────────*/

void bst_walk(struct bst* tree, void* userdata, void (*handler) (void* item, void* userdata), enum traversal_order order)
{
    bintree_walk(tree->root, userdata, handler, order);
}

// *** Helper functions *** //

static struct bintree** bst_search_helper(struct bst* tree, const void* data)
{
    struct bintree** curr = &tree->root;
    while (*curr)
    {
        int result = tree->cmp(data, (*curr)->data);
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
