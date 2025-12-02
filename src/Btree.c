#include "../include/Btree.h"
#include <stdlib.h>

struct Btree
{
    struct mway_header* root;
    struct object_concept* oc;
    int (*cmp) (const void* key, const void* data);
    size_t size;
};

// Return data if found in the current node, else return NULL and store the child in struct mway_header** child ptr, where data might at.
static void* search_node(struct mway_header* node, const void* data, struct mway_header** child, int (*cmp) (const void* key, const void* data));
static void walk_helper(struct mway_header* root, void* context, void (*handler) (void* item, void* context));

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

struct Btree* Btree_create(size_t order, int (*cmp) (const void* key, const void* data), struct object_concept* oc)
{
    struct Btree* tree = malloc(sizeof(struct Btree));
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

void Btree_destroy(struct Btree* tree, void* context)
{
    mway_destroy(tree->root, context, tree->oc);
    free(tree);
}

// Classic mwaytree node size + one size_t attribute to keep track of how many children are in the node. 
size_t Btree_node_sizeof(size_t order)
{
    return mway_sizeof(order, order - 1, sizeof(size_t));
}
 
/*───────────────────────────────────────────────
 * Operations
 *───────────────────────────────────────────────*/

enum trees_status Btree_add(struct Btree* tree, void* new_data)
{
    
}

enum trees_status Btree_remove(struct Btree* tree, void* data)
{

}

void* Btree_search(struct Btree* tree, const void* data)
{
    struct mway_header* curr = tree->root;
    while (curr != NULL)
    {
        void* found = search_node(curr, data, &curr, tree->cmp);
        if (found)
            return found;
    }
    return NULL;
}
 
/*───────────────────────────────────────────────
 * Accessors
 *───────────────────────────────────────────────*/

const struct mway_header* Btree_root(const struct Btree* tree)
{
    return tree->root;
}

int Btree_empty(const struct Btree* tree)
{
    return tree->root == NULL;
}

size_t Btree_size(const struct Btree* tree)
{
    return tree->size;
}

size_t Btree_order(const struct Btree* tree)
{
    return tree->root->child_capacity;
}
 
/*───────────────────────────────────────────────
 * Iterations
 *───────────────────────────────────────────────*/
 
void Btree_walk(struct Btree* tree, void* context, void (*handler) (void* data, void* context))
{
    walk_helper(tree->root, context, handler);
}

// *** Helper functions *** //

static void* search_node(struct mway_header* node, const void* data, struct mway_header** child, int (*cmp) (const void* key, const void* data))
{
    size_t size = *(size_t*) mway_get_footer(node);
    for (size_t i = 0; i < size; i++)
    {
        const void* curr_data = mway_get_data_const(node, i);
        int result = cmp(curr_data, data);
        if (result == 0)
            return mway_get_data(node, i);
        else if (result > 0)
        {
            *child = mway_get_child(node, i);
            return NULL;
        }
    }
    *child = mway_get_child(node, size);
    return NULL;
}

static void walk_helper(struct mway_header* root, void* context, void (*handler) (void* item, void* context))
{
    if (!root)
        return;
    size_t size = *(size_t*) mway_get_footer(root);
    for (size_t i = 0; i < size; i++)
    {
        walk_helper(mway_get_child(root, i), context, handler);
        handler(mway_get_data(root, i), context);
    }
    walk_helper(mway_get_child(root, size), context, handler);
}