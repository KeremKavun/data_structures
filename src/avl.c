#include "../include/avl.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

enum avl_balance
{
    LEFT_HIGH = 1,
    EVEN = 0,
    RIGHT_HIGH = -1
};

struct avl_node
{
    struct bintree btree;
    int8_t balance_factor;
};

struct avl
{
    struct avl_node* root;
    struct object_concept* oc;
    int (*cmp) (const void* key, const void* data);
    size_t size;
};

static struct avl_node* avl_insert_helper(struct avl_node* root, struct avl_node* new_node, bool* taller, int (*cmp) (const void* key, const void* data));
static struct avl_node* avl_insert_balance_left(struct avl_node* root, bool* taller);
static struct avl_node* avl_insert_balance_right(struct avl_node* root, bool* taller);
static struct avl_node* avl_remove_helper(struct avl_node* root, void* data, bool* shorter, bool* success, int (*cmp) (const void* key, const void* data));
static struct avl_node* avl_remove_balance_left(struct avl_node* root, bool* shorter);
static struct avl_node* avl_remove_balance_right(struct avl_node* root, bool* shorter);
static struct avl_node* rotate_left(struct avl_node* root);
static struct avl_node* rotate_right(struct avl_node* root);
static struct avl_node* avl_node_from_btree(struct bintree* btree);

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

struct avl* avl_create(int (*cmp) (const void* key, const void* data), struct object_concept* oc)
{
    struct avl* tree = malloc(sizeof(struct avl));
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


void avl_destroy(struct avl* tree, void* context)
{
    bintree_destroy(tree->root, context, tree->oc);
    free(tree);
}

size_t avl_node_sizeof()
{
    return sizeof(struct avl_node);
}

/*───────────────────────────────────────────────
 * Operations
 *───────────────────────────────────────────────*/

enum trees_status avl_add(struct avl* tree, void* new_data)
{
    struct avl_node* new_node = (struct avl_node*) bintree_create(new_data, tree->oc);
    if (!new_node)
    {
        LOG(LIB_LVL, CERROR, "Could not create avl_node");
        return TREES_SYSTEM_ERROR;
    }
    new_node->balance_factor = EVEN;
    bool taller;
    struct avl_node* new_root = avl_insert_helper(tree->root, new_node, &taller, tree->cmp);
    if (!new_root)
    {
        (tree->oc && tree->oc->allocator) ? tree->oc->free(tree->oc->allocator, tree) : free(tree);
        LOG(LIB_LVL, CERROR, "Error during inserting into avl");
        return TREES_SYSTEM_ERROR;
    }
    tree->root = new_root;
    tree->size++;
    return TREES_OK;
}

enum trees_status avl_remove(struct avl* tree, void* data)
{
    bool shorter;
    bool success;
    struct avl_node* new_root = avl_remove_helper(tree->root, data, &shorter, &success, tree->cmp);
    if (!success)
    {
        LOG(PROJ_LVL, CERROR, "Could not place data to remove");
        return TREES_NOT_FOUND;
    }
    tree->root = new_root;
    tree->size--;
    return TREES_OK;
}

void* avl_search(struct avl* tree, const void* data)
{
    // struct avl_node is compatible with bintree, its first field is bintree, aggregation of bintree
    struct avl_node** target = (struct avl_node**) bintree_search((struct bintree**) &tree->root, data, tree->cmp);
    if (!(*target))
    {
        LOG(LIB_LVL, CERROR, "Key not found");
        return NULL;
    }
    return (*target)->btree.data;
}

/*───────────────────────────────────────────────
 * Accessors
 *───────────────────────────────────────────────*/

int avl_empty(const struct avl* tree)
{
    return tree->root == NULL;
}

size_t avl_size(const struct avl* tree)
{
    return tree->size;
}

/*───────────────────────────────────────────────
 * Iterations
 *───────────────────────────────────────────────*/

void avl_walk(struct avl* tree, void* userdata, void (*handler) (void* data, void* userdata), enum traversal_order order)
{
    // struct avl_node is compatible with bintree, its first field is bintree, aggregation of bintree
    bintree_walk((struct bintree*) tree->root, userdata, handler, order);
}

// *** Helper functions *** //

static struct avl_node* avl_insert_helper(struct avl_node* root, struct avl_node* new_node, bool* taller, int (*cmp) (const void* key, const void* data))
{
    if (!root)
    {
        *taller = true;
        return new_node;
    }
    if (cmp(root->btree.data, new_node->btree.data) > 0)
    {
        struct avl_node* left_child = avl_insert_helper(avl_node_from_btree(root->btree.left), new_node, taller, cmp);
        root->btree.left = (left_child) ? &left_child->btree : NULL;
        if (root->balance_factor > 1)
        {
            if (*taller)
            {
                switch (root->balance_factor)
                {
                case LEFT_HIGH: // Was LEFT_HIGH and adding to the left again, rotate
                    root = avl_insert_balance_left(root, taller);
                    break;

                case EVEN: // Was EVEN, now LEFT_HIGH
                    root->balance_factor = LEFT_HIGH;
                    break;

                case RIGHT_HIGH: // Was RIGHT_HIGH and adding to the left, now EVEN
                    root->balance_factor = EVEN;
                    *taller = false;
                    break;
                }
            }
        }
    }
    else
    {
        struct avl_node* right_child = avl_insert_helper(avl_node_from_btree(root->btree.right), new_node, taller, cmp);
        root->btree.right = (right_child) ? &right_child->btree : NULL;
        if (root->balance_factor < -1)
        {
            if (*taller)
            {
                switch (root->balance_factor)
                {
                case LEFT_HIGH: // Was LEFT_HIGH and adding to the right, now EVEN
                    root->balance_factor = EVEN;
                    *taller = false;
                    break;

                case EVEN: // Was EVEN, now RIGHT_HIGH
                    root->balance_factor = RIGHT_HIGH;
                    break;

                case RIGHT_HIGH: // Was RIGHT_HIGH and adding to the right again, rotate
                    root = avl_insert_balance_right(root, taller);
                    break;
                }
            }
        }
    }
    return root;
}

static struct avl_node* avl_insert_balance_left(struct avl_node* root, bool* taller)
{
    struct avl_node* left_node;
    struct avl_node* right_of_left_node;
    left_node = avl_node_from_btree(root->btree.left);
    // At least one left_node, doesnt check NULL so
    switch (left_node->balance_factor)
    {
    case LEFT_HIGH:
    {
        root->balance_factor = EVEN;
        left_node->balance_factor = EVEN;
        root = rotate_right(root);
        *taller = false;
        break;
    }

    case RIGHT_HIGH:
    {
        right_of_left_node = avl_node_from_btree(left_node->btree.right);
        // At least one right_node of the left_node, doesnt check NULL so
        switch (right_of_left_node->balance_factor)
        {
        case LEFT_HIGH:
            root->balance_factor = RIGHT_HIGH;
            left_node->balance_factor = EVEN;
            break;

        case RIGHT_HIGH:
            root->balance_factor = EVEN;
            left_node->balance_factor = LEFT_HIGH;
            break;

        // (Should not occur) case EVEN: LOG("Critical error, library error or avl invariants were changed!"), exit then
        }
        root->btree.left = rotate_left(root);
        right_of_left_node->balance_factor = EVEN;
        root = rotate_right(right_of_left_node);
        *taller = false;
        break;
    }

    // (Should not occur) case EVEN: LOG("Critical error, library error or avl invariants were changed!"), exit then
    }
    return root;
}

static struct avl_node* avl_insert_balance_right(struct avl_node* root, bool* taller)
{
    struct avl_node* right_node;
    struct avl_node* left_of_right_node;
    right_node = avl_node_from_btree(root->btree.right);
    // At least one right_node, doesnt check NULL so
    switch (right_node->balance_factor)
    {
    case RIGHT_HIGH:
    {
        root->balance_factor = EVEN;
        right_node->balance_factor = EVEN;
        root = rotate_left(root);
        *taller = false;
        break;
    }

    case LEFT_HIGH:
    {
        left_of_right_node = avl_node_from_btree(right_node->btree.left);
        switch (left_of_right_node->balance_factor)
        {
        case RIGHT_HIGH:
            root->balance_factor = LEFT_HIGH;
            right_node->balance_factor = EVEN;
            break;

        case LEFT_HIGH:
            root->balance_factor = EVEN;
            right_node->balance_factor = RIGHT_HIGH;
            break;

        // (Should not occur) case EVEN: LOG("Critical error, library error or avl invariants were changed!"), exit then
        }
        root->btree.right = rotate_right(root);
        left_of_right_node->balance_factor = EVEN;
        root = rotate_left(left_of_right_node);
        *taller = false;
        break;
    }

    // (Should not occur) case EVEN: LOG("Critical error, library error or avl invariants were changed!"), exit then
    }
    return root;
}

static struct avl_node* avl_remove_helper(struct avl_node* root, void* data, bool* shorter, bool* success, int (*cmp) (const void* key, const void* data))
{
    if (!root)
    {
        *shorter = false;
        *success = false;
        return NULL;
    }
    if (cmp(root->btree.data, data) > 0) // using cmp twice to minimize stackoverflow risk
    {
        struct avl_node* left_child = avl_remove_helper(avl_node_from_btree(root->btree.left), data, shorter, success, cmp);
        root->btree.left = (left_child) ? &left_child->btree.left : NULL;
        if (*shorter)
            root = avl_remove_balance_left(root, shorter);
    }
    else if (cmp(root->btree.data, data) < 0) // using cmp twice to minimize stackoverflow risk
    {
        struct avl_node* right_child = avl_remove_helper(avl_node_from_btree(root->btree.right), data, shorter, success, cmp);
        root->btree.left = (right_child) ? &right_child->btree.right : NULL;
        if (*shorter)
            root = avl_remove_balance_right(root, shorter);
    }
    else
    {

    }
    return root;
}

static struct avl_node* avl_remove_balance_left(struct avl_node* root, bool* shorter)
{

}

static struct avl_node* avl_remove_balance_right(struct avl_node* root, bool* shorter)
{

}

static struct avl_node* rotate_left(struct avl_node* root)
{
    struct avl_node* new_root = avl_node_from_btree(root->btree.right);
    struct avl_node* transfer = (new_root) ? avl_node_from_btree(new_root->btree.left) : NULL;
    new_root->btree.left = &root->btree;
    root->btree.right = transfer ? &transfer->btree : NULL;
    return new_root;
}


static struct avl_node* rotate_right(struct avl_node* root)
{
    struct avl_node* new_root = avl_node_from_btree(root->btree.left);
    struct avl_node* transfer = (new_root) ? avl_node_from_btree(new_root->btree.right) : NULL;
    new_root->btree.right = &root->btree;
    root->btree.left = transfer ? &transfer->btree : NULL;
    return new_root;
}

static struct avl_node* avl_node_from_btree(struct bintree* btree)
{
    return (struct avl_node*) btree;
}