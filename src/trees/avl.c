#include <ds/trees/avl.h>
#include "gbst.h"
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#define AVL_MAX_DEPTH 64 // Means 2^64 node max, practically infinity

static void avl_insert_balance_left(struct bintree **root_ptr);
static void avl_insert_balance_right(struct bintree **root_ptr);
static void avl_remove_balance_left(struct bintree **root_ptr);
static void avl_remove_balance_right(struct bintree **root_ptr);
static struct avl_node *rotate_left(struct avl_node *root);
static struct avl_node *rotate_right(struct avl_node *root);

/** @enum avl_balance */
enum avl_balance {
    LEFT_HIGH = 1,
    EVEN = 0,
    RIGHT_HIGH = -1
};

/** @struct avl_node */
struct avl_node {
    struct bintree          btree;              ///< Inheriting from bintree, since avl is conceptually a binary tree.
    enum avl_balance        balance_factor;     ///< Balance factor
};

/* =========================================================================
 * Initialize & Deinitialize
 * ========================================================================= */

void avl_init(struct avl *tree, int (*cmp) (const void *key, const void *data), struct allocator_concept *ac)
{
    assert(tree != NULL && cmp != NULL && ac != NULL);
    tree->root = NULL;
    tree->ac = *ac;
    tree->cmp = cmp;
    tree->size = 0;
}


void avl_deinit(struct avl *tree, struct object_concept *oc)
{
    assert(tree != NULL);
    bintree_destroy((struct bintree*) tree->root, oc, &tree->ac);
    tree->root = NULL;
}

size_t avl_node_sizeof()
{
    return sizeof(struct avl_node);
}

/* =========================================================================
 * Operations
 * ========================================================================= */

enum trees_status avl_add(struct avl *tree, void *new_data)
{
    assert(tree != NULL && new_data != NULL); // Banned NULL data, it makes confusion.
    struct avl_node **stack[AVL_MAX_DEPTH];
    size_t depth = 0;
    struct avl_node **curr = &tree->root;
    while (*curr) {
        if (depth >= AVL_MAX_DEPTH)
            return TREES_SYSTEM_ERROR;
        stack[depth++] = curr;
        int result = tree->cmp(new_data, BIN_NODE(*curr)->data);
        if (result < 0) {
            curr = (struct avl_node **) &BIN_NODE(*curr)->left;
        } else if (result > 0) {
            curr = (struct avl_node **) &BIN_NODE(*curr)->right;
        } else {
            return TREES_DUPLICATE_KEY;
        }
    }
    struct avl_node *new_node = (struct avl_node*) bintree_create(NULL, NULL, new_data, &tree->ac);
    if (!new_node)
        return TREES_SYSTEM_ERROR;
    new_node->balance_factor = EVEN;
    *curr = new_node;
    tree->size++;
    while (depth > 0) {
        struct avl_node **parent_ptr = stack[--depth];
        struct avl_node *parent = *parent_ptr;
        // Added to left of the parent
        if ((void *) curr == (void *) &BIN_NODE(parent)->left) {
            switch (parent->balance_factor)
            {
            // Was LEFT_HIGH, adding left makes it unbalanced
            case LEFT_HIGH:
                avl_insert_balance_left((struct bintree **) parent_ptr);
                goto done;
            // Was EVEN, adding left makes it LEFT_HIGH
            case EVEN:
                parent->balance_factor = LEFT_HIGH;
                break;
            // Was RIGHT_HIGH, adding left makes it EVEN
            case RIGHT_HIGH:
                parent->balance_factor = EVEN;
                goto done;
            }
        // Added to right of the parent
        } else {
            switch (parent->balance_factor)
            {
            // Was LEFT_HIGH, adding right makes it EVEN
            case LEFT_HIGH:
                parent->balance_factor = EVEN;
                goto done;
            // Was EVEN, adding right makes it RIGHT_HIGH
            case EVEN:
                parent->balance_factor = RIGHT_HIGH;
                break;
            // Was RIGHT_HIGH, adding right makes it EVEN
            case RIGHT_HIGH:
                avl_insert_balance_right((struct bintree **) parent_ptr);
                goto done;
            }
        }
        curr = parent_ptr;
    }
done:
    return TREES_OK;
}

void *avl_remove(struct avl *tree, void *data)
{
    assert(tree != NULL && data != NULL);
    struct avl_node **stack[AVL_MAX_DEPTH];
    size_t depth = 0;
    struct avl_node **curr = &tree->root;
    struct avl_node *to_free = NULL;
    void *removed = NULL;
    while (*curr) {
        if (depth >= AVL_MAX_DEPTH)
            return NULL;
        int result = tree->cmp(data, BIN_NODE(*curr)->data);
        if (result < 0) {
            stack[depth++] = curr;
            curr = (struct avl_node **) &BIN_NODE(*curr)->left;
        } else if (result > 0) {
            stack[depth++] = curr;
            curr = (struct avl_node **) &BIN_NODE(*curr)->right;
        } else {
            struct avl_node *target = *curr;
            removed = BIN_NODE(target)->data;
            if (BIN_NODE(target)->left && BIN_NODE(target)->right) {
                stack[depth++] = curr;
                curr = (struct avl_node **) &BIN_NODE(target)->left;
                while (BIN_NODE(*curr)->right) {
                    stack[depth++] = curr;
                    curr = (struct avl_node **) &BIN_NODE(*curr)->right;
                }
                BIN_NODE(target)->data = BIN_NODE(*curr)->data;
            }
            to_free = *curr;
            struct avl_node *replacement = NULL;
            if (BIN_NODE(*curr)->left) {
                replacement = (struct avl_node *) BIN_NODE(*curr)->left;
            } else {
                // If left is NULL, we must take the right (which might be NULL, that's fine)
                replacement = (struct avl_node *) BIN_NODE(*curr)->right;
            }
            *curr = replacement;
            break;
        }
    }
    if (!removed)
        return NULL;
    tree->ac.free(tree->ac.allocator, to_free);
    tree->size--;
    while (depth > 0) {
        struct avl_node **parent_ptr = stack[--depth];
        struct avl_node *parent = *parent_ptr;
        if (curr == (struct avl_node **) &BIN_NODE(parent)->left) {
            switch (parent->balance_factor) {
            // Was LEFT_HIGH, removing from left makes it EVEN
            // and height reduces, continue.
            case LEFT_HIGH:
                parent->balance_factor = EVEN;
                break;  // Height reduced, continue
            // Was EVEN, removing from left makes it RIGHT_HIGH, height doesnt change
            case EVEN:
                parent->balance_factor = RIGHT_HIGH;
                goto done;  // Height unchanged
            // Was RIGHT_HIGH, removing from left makes it unbalanced
            case RIGHT_HIGH:
                avl_remove_balance_right((struct bintree **) parent_ptr);
                if ((*parent_ptr)->balance_factor != EVEN)
                    goto done;  // Height unchanged after rotation
                break;
            }
        } else {
            switch (parent->balance_factor) {
            // Was LEFT_HIGH, removing from right makes it unbalanced
            case LEFT_HIGH:
                avl_remove_balance_left((struct bintree **) parent_ptr);
                // If EVEN, that means height reduced, probe continues to make sure parents remain balanced
                if ((*parent_ptr)->balance_factor != EVEN)
                    goto done;
                break;
            // Was EVEN, removing from right makes it LEFT_HIGH, height doesnt change
            case EVEN:
                parent->balance_factor = LEFT_HIGH;
                goto done;
            // Was RIGHT_HIGH, removing from right makes it EVEN
            // and height reduces, continue.
            case RIGHT_HIGH:
                parent->balance_factor = EVEN;
                break;
            }
        }
        curr = parent_ptr;
    }
done:
    return removed;
}

void *avl_search(struct avl *tree, const void *data)
{
    assert(tree != NULL);
    struct avl_node** target = (struct avl_node**) gbst_search((struct bintree**) &tree->root, data, tree->cmp);
    if (!(*target)) {
        LOG(LIB_LVL, CERROR, "Key not found");
        return NULL;
    }
    return (*target)->btree.data;
}

/* =========================================================================
 * Traversal
 * ========================================================================= */

// *** Helper functions *** //

static void avl_insert_balance_left(struct bintree **root_ptr)
{
    struct avl_node *root = AVL_NODE(*root_ptr);
    struct avl_node *left_node = AVL_NODE(root->btree.left);
    assert(left_node != NULL);
    switch (left_node->balance_factor) {
    case LEFT_HIGH: // LL Case: Single Right Rotation
    {
        root->balance_factor = EVEN;
        left_node->balance_factor = EVEN;
        *root_ptr = (struct bintree*) rotate_right(root);
        break;
    }
    case EVEN: // Should not occur
    case RIGHT_HIGH: // LR Case: Double Rotation
    {
        struct avl_node *right_of_left = AVL_NODE(left_node->btree.right);
        switch (right_of_left->balance_factor) {
        case LEFT_HIGH:
            root->balance_factor = RIGHT_HIGH;
            left_node->balance_factor = EVEN;
            break;
        case RIGHT_HIGH:
            root->balance_factor = EVEN;
            left_node->balance_factor = LEFT_HIGH;
            break;
        case EVEN:
            root->balance_factor = EVEN;
            left_node->balance_factor = EVEN;
            break;
        }
        right_of_left->balance_factor = EVEN;
        root->btree.left = (struct bintree*) rotate_left(left_node);
        *root_ptr = (struct bintree*) rotate_right(root);
        break;
    }
    }
}

static void avl_insert_balance_right(struct bintree **root_ptr)
{
    struct avl_node *root = AVL_NODE(*root_ptr);
    struct avl_node *right_node = AVL_NODE(root->btree.right);
    assert(right_node != NULL);
    switch (right_node->balance_factor) {
    case RIGHT_HIGH: // RR Case: Single Left Rotation
    {
        root->balance_factor = EVEN;
        right_node->balance_factor = EVEN;
        *root_ptr = (struct bintree*) rotate_left(root);
        break;
    }
    case EVEN: // Should not occur
    case LEFT_HIGH: // RL Case: Double Rotation
    {
        struct avl_node *left_of_right = AVL_NODE(right_node->btree.left);
        
        switch (left_of_right->balance_factor) {
        case RIGHT_HIGH:
            root->balance_factor = LEFT_HIGH;
            right_node->balance_factor = EVEN;
            break;
        case LEFT_HIGH:
            root->balance_factor = EVEN;
            right_node->balance_factor = RIGHT_HIGH;
            break;
        case EVEN:
            root->balance_factor = EVEN;
            right_node->balance_factor = EVEN;
            break;
        }
        left_of_right->balance_factor = EVEN;
        root->btree.right = (struct bintree*) rotate_right(right_node);
        *root_ptr = (struct bintree*) rotate_left(root);
        break;
    }
    }
}

static void avl_remove_balance_left(struct bintree **root_ptr)
{
    struct avl_node *root = AVL_NODE(*root_ptr);
    struct avl_node *left_node = AVL_NODE(root->btree.left);
    switch (left_node->balance_factor)
    {
    case LEFT_HIGH:
    {
        root->balance_factor = EVEN;
        left_node->balance_factor = EVEN;
        *root_ptr = (struct bintree *) rotate_right(root);
        break;
    }
    case EVEN:
    {
        root->balance_factor = LEFT_HIGH;
        left_node->balance_factor = RIGHT_HIGH;
        *root_ptr = (struct bintree *) rotate_right(root);
        break;
    }
    case RIGHT_HIGH:
    {
        struct avl_node *right_of_left = AVL_NODE(left_node->btree.right);
        switch (right_of_left->balance_factor)
        {
        case RIGHT_HIGH:
            root->balance_factor = EVEN;
            left_node->balance_factor = LEFT_HIGH;
            break;
        case EVEN:
            root->balance_factor = EVEN;
            left_node->balance_factor = EVEN;
            break;
        case LEFT_HIGH:
            root->balance_factor = RIGHT_HIGH;
            left_node->balance_factor = EVEN;
            break;
        }
        right_of_left->balance_factor = EVEN;
        root->btree.left = (struct bintree *) rotate_left(left_node);
        *root_ptr = (struct bintree *) rotate_right(root);
        break;
    }
    }
}

static void avl_remove_balance_right(struct bintree **root_ptr)
{
    struct avl_node *root = AVL_NODE(*root_ptr);
    struct avl_node *right_node = AVL_NODE(root->btree.right);
    switch (right_node->balance_factor)
    {
    case RIGHT_HIGH:
    {
        root->balance_factor = EVEN;
        right_node->balance_factor = EVEN;
        *root_ptr = (struct bintree *) rotate_left(root);
        break;
    }
    case EVEN:
    {
        root->balance_factor = RIGHT_HIGH;
        right_node->balance_factor = LEFT_HIGH;
        *root_ptr = (struct bintree *) rotate_left(root);
        break;
    }
    case LEFT_HIGH:
    {
        struct avl_node *left_of_right = AVL_NODE(right_node->btree.left);
        switch (left_of_right->balance_factor)
        {
        case LEFT_HIGH:
            root->balance_factor = EVEN;
            right_node->balance_factor = RIGHT_HIGH;
            break;
        case EVEN:
            root->balance_factor = EVEN;
            right_node->balance_factor = EVEN;
            break;
        case RIGHT_HIGH:
            root->balance_factor = LEFT_HIGH;
            right_node->balance_factor = EVEN;
            break;
        }
        left_of_right->balance_factor = EVEN;
        root->btree.right = (struct bintree *) rotate_right(right_node);
        *root_ptr = (struct bintree *) rotate_left(root);
        break;
    }
    }
}

static struct avl_node *rotate_left(struct avl_node *root)
{
    struct avl_node *new_root = AVL_NODE(root->btree.right);
    struct avl_node *transfer = AVL_NODE(new_root->btree.left);
    // Perform rotation
    new_root->btree.left = (struct bintree*)root;
    root->btree.right = (struct bintree*)transfer;
    return new_root;
}

static struct avl_node *rotate_right(struct avl_node *root)
{
    struct avl_node *new_root = AVL_NODE(root->btree.left);
    struct avl_node *transfer = AVL_NODE(new_root->btree.right);
    // Perform rotation
    new_root->btree.right = (struct bintree*)root;
    root->btree.left = (struct bintree*)transfer;
    return new_root;
}