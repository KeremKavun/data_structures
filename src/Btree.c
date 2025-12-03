#include "../include/Btree.h"
#include "../../stack/include/bstack.h"
#include <stdlib.h>

/*
    Btree nodes are classic mway node concept introduced in mwaytree.h + size_t size to track the count of data 
*/

struct Btree
{
    struct mway_header* root;
    struct object_concept* oc;
    int (*cmp) (const void* key, const void* data);
    size_t size;
};

// Insert node somewhere and return NULL if not in overflow, save last data before shifting to return in overflow to pass it into split_node
static void* insert_data(struct mway_header* node, void* new_data, int (*cmp) (const void* key, const void* data));
// Split node by creating a new node and copying ((data_capacity / 2) - 1) into the new node and add last_data to the new node.
// Return median as last data of the original node.
static void* split_node(struct mway_header* node, void* last_data, struct object_concept* oc);
static size_t search_node(struct mway_header* node, const void* data, void** result, int (*cmp) (const void* key, const void* data));
static void walk_helper(struct mway_header* root, void* context, void (*handler) (void* item, void* context));
static int copy_header_ptr(const void* new_item, void* stack_item);

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
    struct mway_header* root = mway_create(order, order - 1, sizeof(size_t), ALIGN_REQ);
    if (!root)
    {
        LOG(LIB_LVL, CERROR, "Could not allocate B-tree root");
        return NULL;
    }
    tree->root = root;
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
    return mway_sizeof(order, order - 1, sizeof(size_t), ALIGN_REQ);
}
 
/*───────────────────────────────────────────────
 * Operations
 *───────────────────────────────────────────────*/

enum trees_status Btree_add(struct Btree* tree, void* new_data)
{
    struct bstack bs;
    bstack_init(&bs, NULL, sizeof(struct mway_header*));
    // Store all mway nodes
    struct mway_header* curr = tree->root;
    while (curr != NULL)
    {
        bpush(&bs, curr, NULL, copy_header_ptr);
        void* result;
        size_t index = search_node(curr, new_data, &result, tree->cmp);
        if (result)
        {
            LOG(LIB_LVL, CERROR, "Duplicate key attempted to be inserted");
            return TREES_DUPLICATE_KEY;
        }
        struct mway_header* curr = mway_get_child(curr, index);
    }
    // Just pop last NULL node, will consider the first while loop again
    bpop(&bs, NULL);
    void* curr_data = new_data;
    while (!bstack_empty(&bs))
    {
        struct mway_header* curr_node;
        bpop(&bs, curr_node);
        void* overflowed = insert_data(curr_node, new_data, tree->cmp);
        if (overflowed)
            curr_data = split_node(curr_node, overflowed, tree->oc);
        else
            break;
    }
}

enum trees_status Btree_remove(struct Btree* tree, void* data)
{

}

void* Btree_search(struct Btree* tree, const void* data)
{
    void* result;
    struct mway_header* curr = tree->root;
    while (curr != NULL)
    {
        size_t index = search_node(curr, data, &result, tree->cmp);
        if (result)
            return result;
        struct mway_header* curr = mway_get_child(curr, index);
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

static void* insert_data(struct mway_header* node, void* new_data, int (*cmp) (const void* key, const void* data))
{
    size_t* size_ptr = (size_t*) mway_get_footer(node);
    size_t size = *size_ptr;
    for (size_t i = 0; i < size; i++)
    {
        if (cmp(mway_get_data_const(node, i), new_data) > 0)
        {
            // Insert void* new_data and return last
            if (size == node->data_capacity)
            {
                void* last_data = mway_get_data(node, size - 1);
                // i is where we encounter first data that is bigger than new_data. We want to insert here, thus, shifting by 1.
                // dest is i + 1, src is i. i is zero indexed while size is not, so we use size - i gives data num from i to the last
                // instead of size - i + 1. But we need to substract 1 too, because we will discard the last data.
                memmove(mway_get_data_addr(node, i + 1), mway_get_data_addr(node, i), sizeof(void*) * (node->data_capacity - i - 1));
                mway_set_data(node, i, new_data);
                return last_data;
            }
            // This time, we dont substract size_t __n by one, since we have space
            memmove(mway_get_data_addr(node, i + 1), mway_get_data_addr(node, i), sizeof(void*) * (size - i));
            (*size_ptr)++;
            return NULL;
        }
    }
    return new_data;
}

static void* split_node(struct mway_header* node, void* last_data, struct object_concept* oc)
{
    struct mway_header* new_node = mway_create(node->child_capacity, node->data_capacity, sizeof(size_t), oc);
    if (!new_node)
    {
        LOG(LIB_LVL, CERROR, "Could not allocate new B-tree node");
        return NULL;
    }
    size_t median_index = node->data_capacity / 2;
    void* median = mway_get_data(node, median_index);
    // Move data AFTER median to new_node. Start copying from median_index + 1
    size_t copy_items_start_index = median_index + 1;
    size_t items_from_node = node->data_capacity - copy_items_start_index;
    if (items_from_node > 0)
        memcpy(mway_get_data_addr(new_node, 0), mway_get_data_addr(node, copy_items_start_index), sizeof(void*) * items_from_node);
    // Add the 'last_data' (overflow) to the end of new_node
    mway_set_data(new_node, items_from_node, last_data);
    // Move child nodes
    size_t copy_nodes_start_index = median_index + 1;
    size_t children_from_node = items_from_node + 2;
    if (children_from_node > 0 && mway_get_child(node, copy_nodes_start_index) != NULL)
        memcpy(mway_get_child_addr(new_node, 0), mway_get_child_addr(node, copy_nodes_start_index), sizeof(struct mway_header*) * children_from_node);
    // New node has (items_from_node) + 1 (last_data)
    *(size_t*) mway_get_footer(new_node) = items_from_node + 1;
    // Old node is reduced to just the items before the median
    *(size_t*) mway_get_footer(node) = median_index;
    return median;
}

static size_t search_node(struct mway_header* node, const void* data, void** found, int (*cmp) (const void* key, const void* data))
{
    size_t size = *(size_t*) mway_get_footer(node);
    size_t i = 0;
    while (i < size)
    {
        int result = cmp(mway_get_data_const(node, i), data);
        if (result == 0)
        {
            *found = mway_get_data(node, i);
            return i;
        }
        else if (result > 0)
        {
            *found = NULL;
            return i;
        }
        i++;
    }
    return i;
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

static int copy_header_ptr(const void* new_item, void* stack_item)
{
    *(struct mway_header*) stack_item = *(const struct mway_header*) new_item;
    return 0;
}