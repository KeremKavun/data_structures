#include <ds/trees/avl.h>
#include "gbst.h"
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

struct avl_node {
    struct bintree      btree;
    struct bintree      *parent;
    int8_t              balance_factor;
};

enum avl_balance {
    LEFT_HIGH = 1,
    EVEN = 0,
    RIGHT_HIGH = -1
};

static void avl_insert_balance_left(struct bintree **root_ptr);
static void avl_insert_balance_right(struct bintree **root_ptr);
static int avl_remove_balance_left(struct bintree **root_ptr);
static int avl_remove_balance_right(struct bintree **root_ptr);
static struct avl_node *rotate_left(struct avl_node *root);
static struct avl_node *rotate_right(struct avl_node *root);

/* =========================================================================
 * Create & Destroy
 * ========================================================================= */

struct avl *avl_create(int (*cmp) (const void *key, const void *data), struct allocator_concept *ac)
{
    struct avl *tree = malloc(sizeof(struct avl));
    if (!tree) {
        LOG(LIB_LVL, CERROR, "Failed to allocate memory for tree");
        return NULL;
    }
    tree->root = NULL;
    tree->ac = *ac;
    tree->cmp = cmp;
    tree->size = 0;
    return tree;
}


void avl_destroy(struct avl *tree, struct object_concept *oc)
{
    bintree_destroy((struct bintree*) tree->root, oc, &tree->ac);
    free(tree);
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
    assert(tree != NULL);
    struct avl_node **curr = &tree->root;
    struct avl_node *parent = NULL;
    while (*curr) {
        parent = *curr;
        int result = tree->cmp(new_data, (*curr)->btree.data);

        if (result < 0) {
            curr = (struct avl_node **)&(*curr)->btree.left;
        } else if (result > 0) {
            curr = (struct avl_node **)&(*curr)->btree.right;
        } else {
            return TREES_DUPLICATE_KEY;
        }
    }
    struct avl_node *new_node = (struct avl_node*) bintree_create(NULL, NULL, new_data, &tree->ac);
    if (!new_node)
        return TREES_SYSTEM_ERROR;
    new_node->parent = (struct bintree*)parent;
    new_node->balance_factor = EVEN;
    *curr = new_node;
    tree->size++;
    // Start from the new node and climb up
    struct avl_node *child = new_node;
    while (parent) {
        // We need the double-pointer to 'parent' to pass to balance functions.
        struct bintree **parent_ptr;
        if (parent->parent) {
            struct avl_node *grandparent = AVL_NODE(parent->parent);
            if (grandparent->btree.left == (struct bintree*)parent)
                parent_ptr = &grandparent->btree.left;
            else
                parent_ptr = &grandparent->btree.right;
        } else {
            parent_ptr = (struct bintree **)&tree->root;
        }
        // Check which side we added to
        if (parent->btree.left == (struct bintree*)child) {
            // Added to Left
            switch (parent->balance_factor) {
            case LEFT_HIGH:
                avl_insert_balance_left(parent_ptr);
                goto done;
                
            case EVEN:
                parent->balance_factor = LEFT_HIGH;
                break;

            case RIGHT_HIGH:
                parent->balance_factor = EVEN;
                goto done;
            }
        } else {
            // Added to Right
            switch (parent->balance_factor) {
            case LEFT_HIGH:
                parent->balance_factor = EVEN;
                goto done; 

            case EVEN:
                parent->balance_factor = RIGHT_HIGH;
                break; 

            case RIGHT_HIGH:
                avl_insert_balance_right(parent_ptr);
                goto done;
            }
        }
        // Move Up
        child = parent;
        parent = AVL_NODE(parent->parent);
    }
done:
    return TREES_OK;
}

void *avl_remove(struct avl *tree, void *data)
{
    if (!tree || !tree->root)
        return NULL;
    struct avl_node *curr = tree->root;
    while (curr) {
        int cmp = tree->cmp(data, curr->btree.data);
        if (cmp < 0) curr = AVL_NODE(curr->btree.left);
        else if (cmp > 0) curr = AVL_NODE(curr->btree.right);
        else break;
    }
    if (!curr) {
        LOG(PROJ_LVL, CWARNING, "Data not found in tree");
        return NULL;
    }
    void *return_data = curr->btree.data;
    // Handle Two Children (Swap Strategy)
    if (curr->btree.left && curr->btree.right) {
        struct avl_node *succ = AVL_NODE(curr->btree.right);
        while (succ->btree.left) {
            succ = AVL_NODE(succ->btree.left);
        }
        curr->btree.data = succ->btree.data;
        curr = succ;
    }
    // Physical Unlink
    struct avl_node *parent = AVL_NODE(curr->parent);
    struct avl_node *child = AVL_NODE(curr->btree.left ? curr->btree.left : curr->btree.right);
    bool is_left_child = false; 
    if (parent) {
        if (parent->btree.left == (struct bintree*)curr) {
            parent->btree.left = (struct bintree*)child;
            is_left_child = true;
        } else {
            parent->btree.right = (struct bintree*)child;
            is_left_child = false;
        }
    } else {
        tree->root = child;
    }
    if (child)
        child->parent = (struct bintree*)parent;
    tree->ac.free(tree->ac.allocator, curr);
    tree->size--;
    // Iterative Backtracking
    while (parent) {
        struct bintree **root_ptr;
        if (parent->parent) {
            if (AVL_NODE(parent->parent)->btree.left == (struct bintree*)parent)
                root_ptr = &AVL_NODE(parent->parent)->btree.left;
            else
                root_ptr = &AVL_NODE(parent->parent)->btree.right;
        } else {
            root_ptr = (struct bintree **)&tree->root;
        }
        int height_reduced = 0;
        if (is_left_child) {
            switch (parent->balance_factor) {
                case LEFT_HIGH:
                    parent->balance_factor = EVEN;
                    height_reduced = 1;
                    break;
                case EVEN:
                    parent->balance_factor = RIGHT_HIGH;
                    height_reduced = 0;
                    break;
                case RIGHT_HIGH:
                    height_reduced = avl_remove_balance_left(root_ptr);
                    break;
            }
        } else {
            switch (parent->balance_factor) {
                case RIGHT_HIGH:
                    parent->balance_factor = EVEN;
                    height_reduced = 1;
                    break;
                case EVEN:
                    parent->balance_factor = LEFT_HIGH;
                    height_reduced = 0;
                    break;
                case LEFT_HIGH:
                    height_reduced = avl_remove_balance_right(root_ptr);
                    break;
            }
        }
        if (!height_reduced)
            break;
        struct avl_node *child_processed = AVL_NODE(*root_ptr);
        parent = AVL_NODE(child_processed->parent);
        if (parent)
            is_left_child = (parent->btree.left == (struct bintree*)child_processed);
    }
    return return_data;
}

void *avl_search(struct avl *tree, const void *data)
{
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

struct avl_node *avl_node_prev(struct avl_node *node)
{
    if (!node)
        return NULL;
    if (node->btree.left) {
        struct bintree *curr = node->btree.left;
        while (curr->right) {
            curr = curr->right;
        }
        return AVL_NODE(curr);
    }
    struct bintree *curr = &node->btree;
    struct bintree *parent = node->parent;
    while (parent && curr == parent->left) {
        curr = parent;
        parent = AVL_NODE(parent)->parent;
    }
    return AVL_NODE(parent);
}

struct avl_node *avl_node_next(struct avl_node *node)
{
    if (!node)
        return NULL;
    if (node->btree.right) {
        struct bintree *curr = node->btree.right;
        while (curr->left) {
            curr = curr->left;
        }
        return AVL_NODE(curr);
    }
    struct bintree *curr = &node->btree;
    struct bintree *parent = node->parent;
    while (parent && curr == parent->right) {
        curr = parent;
        parent = AVL_NODE(parent)->parent;
    }
    return AVL_NODE(parent);
}

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
        AVL_NODE(root->btree.left)->parent = (struct bintree*)root;
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
        AVL_NODE(root->btree.right)->parent = (struct bintree*)root;
        *root_ptr = (struct bintree*) rotate_left(root);
        break;
    }
    }
}

static int avl_remove_balance_left(struct bintree **root_ptr)
{
    struct avl_node *root = AVL_NODE(*root_ptr);
    struct avl_node *right = AVL_NODE(root->btree.right);
    if (right->balance_factor == RIGHT_HIGH) {
        root->balance_factor = EVEN;
        right->balance_factor = EVEN;
        *root_ptr = (struct bintree*) rotate_left(root);
        return 1;
    }
    
    if (right->balance_factor == EVEN) {
        root->balance_factor = RIGHT_HIGH;
        right->balance_factor = LEFT_HIGH;
        *root_ptr = (struct bintree*) rotate_left(root);
        return 0;
    }
    
    struct avl_node *left_of_right = AVL_NODE(right->btree.left);
    switch (left_of_right->balance_factor) {
        case RIGHT_HIGH:
            root->balance_factor = LEFT_HIGH;
            right->balance_factor = EVEN;
            break;
        case LEFT_HIGH:
            root->balance_factor = EVEN;
            right->balance_factor = RIGHT_HIGH;
            break;
        case EVEN:
            root->balance_factor = EVEN;
            right->balance_factor = EVEN;
            break;
    }
    left_of_right->balance_factor = EVEN;
    root->btree.right = (struct bintree*) rotate_right(right);
    AVL_NODE(root->btree.right)->parent = (struct bintree*)root;
    *root_ptr = (struct bintree*) rotate_left(root);
    return 1;
}

static int avl_remove_balance_right(struct bintree **root_ptr)
{
    struct avl_node *root = AVL_NODE(*root_ptr);
    struct avl_node *left = AVL_NODE(root->btree.left);
    if (left->balance_factor == LEFT_HIGH) {
        root->balance_factor = EVEN;
        left->balance_factor = EVEN;
        *root_ptr = (struct bintree*) rotate_right(root);
        return 1;
    }
    
    if (left->balance_factor == EVEN) {
        root->balance_factor = LEFT_HIGH;
        left->balance_factor = RIGHT_HIGH;
        *root_ptr = (struct bintree*) rotate_right(root);
        return 0;
    }
    
    struct avl_node *right_of_left = AVL_NODE(left->btree.right);
    switch (right_of_left->balance_factor) {
        case LEFT_HIGH:
            root->balance_factor = RIGHT_HIGH;
            left->balance_factor = EVEN;
            break;
        case RIGHT_HIGH:
            root->balance_factor = EVEN;
            left->balance_factor = LEFT_HIGH;
            break;
        case EVEN:
            root->balance_factor = EVEN;
            left->balance_factor = EVEN;
            break;
    }
    right_of_left->balance_factor = EVEN;
    root->btree.left = (struct bintree*) rotate_left(left);
    AVL_NODE(root->btree.left)->parent = (struct bintree*)root;
    *root_ptr = (struct bintree*) rotate_right(root);
    return 1;
}

static struct avl_node *rotate_left(struct avl_node *root)
{
    struct avl_node *new_root = AVL_NODE(root->btree.right);
    struct avl_node *transfer = AVL_NODE(new_root->btree.left);
    // Update parent pointers
    new_root->parent = root->parent;
    root->parent = (struct bintree*)new_root;
    // Perform rotation
    new_root->btree.left = (struct bintree*)root;
    root->btree.right = (struct bintree*)transfer;
    // Update transfer's parent
    if (transfer)
        transfer->parent = (struct bintree*)root;
    return new_root;
}

static struct avl_node *rotate_right(struct avl_node *root)
{
    struct avl_node *new_root = AVL_NODE(root->btree.left);
    struct avl_node *transfer = AVL_NODE(new_root->btree.right);
    // Update parent pointers
    new_root->parent = root->parent;
    root->parent = (struct bintree*)new_root;
    // Perform rotation
    new_root->btree.right = (struct bintree*)root;
    root->btree.left = (struct bintree*)transfer;
    // Update transfer's parent
    if (transfer)
        transfer->parent = (struct bintree*)root;
    return new_root;
}