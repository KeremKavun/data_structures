#include <ds/trees/avl.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

static void avl_insert_balance_left(struct avl *tree, struct avl_node *root);
static void avl_insert_balance_right(struct avl *tree, struct avl_node *root);
static void avl_remove_balance_left(struct avl *tree, struct avl_node *root);
static void avl_remove_balance_right(struct avl *tree, struct avl_node *root);
// Recklessly updates root and root->rights parent pointers to avoid some instructions
static void avl_rotate_left(struct avl *tree, struct avl_node *root);
// Recklessly updates root and root->lefts parent pointers to avoid some instructions
static void avl_rotate_right(struct avl *tree, struct avl_node *root);
static void avl_deinit_helper(struct avl_node *node, struct object_concept *oc);

// https://stackoverflow.com/questions/17288746/red-black-nodes-struct-alignment-in-linux-kernel?rq=1

/** @enum avl_balance */
enum avl_balance {
    LEFT_HIGH = 1,      ///< represents '01' in our parent pointer.
    EVEN = 0,           ///< represents '00' in our parent pointer.
    RIGHT_HIGH = 3      ///< represents '11' in our parent pointer.
};

#define avl_get_balance(node)   ((enum avl_balance)((uintptr_t)((node)->btree.parent) & BINTREE_TAG_MASK))
#define avl_set_balance(node, bal) do { \
    (node)->btree.parent = (void *)((((uintptr_t)((node)->btree.parent)) & ~BINTREE_TAG_MASK) | (uintptr_t)(bal)); \
} while (0)
// Sets without resetting to 00 state.
#define avl_set_balance_reckless(node, bal) do { \
    (node)->btree.parent = (void *)(((uintptr_t)((node)->btree.parent)) | (uintptr_t)(bal)); \
} while (0)

/* =========================================================================
 * Initialize & Deinitialize
 * ========================================================================= */

void avl_init(struct avl *tree, bst_cmp_cb cmp)
{
    assert(tree != NULL && cmp != NULL);
    bintree_init((struct bintree*) &tree->root, NULL, NULL, NULL);
    tree->size = 0;
    tree->cmp = cmp;
}


void avl_deinit(struct avl *tree, struct object_concept *oc)
{
    assert(tree != NULL);
    avl_deinit_helper(tree->root, oc);
    tree->root = NULL;
    tree->size = 0;
}

/* =========================================================================
 * Operations
 * ========================================================================= */

int avl_add(struct avl *tree, struct avl_node *new_node)
{
    assert(tree != NULL && new_node != NULL);
    struct bintree *parent;
    struct bintree **link = bintree_search_parent((struct bintree **) &tree->root, (struct bintree *) new_node, &parent, tree->cmp);
    if (*link) {
        LOG(LIB_LVL, CERROR, "Duplicate key");
        return 1;
    }
    // CRITICAL: Your avl_set_parent macro does (parent | old_bits). 
    // Since new_node is uninitialized, old_bits is GARBAGE.
    // We must zero the parent field first!
    bintree_init((struct bintree *) new_node, NULL, NULL, NULL);
    bintree_set_parent((struct bintree *) new_node, parent);
    *link = &new_node->btree;
    tree->size++;
    struct avl_node *curr = new_node;
    parent = bintree_get_parent((struct bintree *) curr);
    while (parent) {
        struct avl_node *parent_node = (struct avl_node*) parent;
        if (parent->left == &curr->btree) {
            switch (avl_get_balance(parent_node))
            {
            // Was LEFT_HIGH, adding left makes it unbalanced
            case LEFT_HIGH:
                avl_insert_balance_left(tree, parent_node); 
                return 0;
            // Was EVEN, adding left makes it LEFT_HIGH
            case EVEN:
                avl_set_balance(parent_node, LEFT_HIGH);
                break;
            // Was RIGHT_HIGH, adding left makes it EVEN
            case RIGHT_HIGH:
                avl_set_balance(parent_node, EVEN);
                return 0;
            }
        } else {
            switch (avl_get_balance(parent_node))
            {
            // Was LEFT_HIGH, adding right makes it EVEN
            case LEFT_HIGH:
                avl_set_balance(parent_node, EVEN);
                return 0;
            // Was EVEN, adding right makes it RIGHT_HIGH
            case EVEN:
                avl_set_balance(parent_node, RIGHT_HIGH);
                break;
            // Was RIGHT_HIGH, adding right makes it EVEN
            case RIGHT_HIGH:
                avl_insert_balance_right(tree, parent_node);
                return 0;
            }
        }
        curr = parent_node;
        parent = bintree_get_parent((struct bintree *) curr);
    }
    return 0;
}

void avl_remove(struct avl *tree, struct avl_node *node)
{
    assert(tree && node);
    struct bintree *n = &node->btree;
    // 1. Swap with successor if 2 children
    if (n->left && n->right) {
        struct bintree *s = n->right;
        while (s->left) s = s->left;
        bintree_swap(n, s);
        if (tree->root == node) tree->root = (struct avl_node*)s;
    }
    // 2. Physical Removal
    struct bintree *child = n->left ? n->left : n->right;
    struct bintree *parent = bintree_get_parent(n);
    // CRITICAL FIX: Determine direction before unlinking 'n'
    bool from_left = parent && (parent->left == n);
    if (child) bintree_set_parent(child, parent);
    if (!parent) {
        tree->root = (struct avl_node*)child;
    } else {
        if (from_left) parent->left = child;
        else parent->right = child;
    }
    // 3. Rebalance
    tree->size--;
    bintree_init(n, NULL, NULL, NULL); // Safe to clear now
    struct bintree *curr = parent;
    while (curr) {
        struct avl_node *node_curr = (struct avl_node*)curr;
        struct bintree *next = bintree_get_parent((struct bintree *) node_curr);
        // Capture next direction before potential rotation changes structure
        bool next_from_left = next ? (next->left == curr) : false;
        bool stop = false;
        if (from_left) { // Left side shortened
            switch (avl_get_balance(node_curr)) {
                case LEFT_HIGH:  
                    avl_set_balance(node_curr, EVEN); 
                    break; 
                case EVEN:       
                    avl_set_balance(node_curr, RIGHT_HIGH); 
                    stop = true; 
                    break; 
                case RIGHT_HIGH: 
                    avl_remove_balance_left(tree, node_curr);
                    {
                        // Calculate new root of this subtree to check if height changed
                        struct bintree *new_sub = !next ? (struct bintree*)tree->root : (next_from_left ? next->left : next->right);
                        if (avl_get_balance((struct avl_node*)new_sub) != EVEN) stop = true;
                    }
                    break;
            }
        } else { // Right side shortened
            switch (avl_get_balance(node_curr)) {
                case RIGHT_HIGH: 
                    avl_set_balance(node_curr, EVEN); 
                    break; 
                case EVEN:       
                    avl_set_balance(node_curr, LEFT_HIGH); 
                    stop = true; 
                    break;
                case LEFT_HIGH:  
                    avl_remove_balance_right(tree, node_curr);
                    {
                        struct bintree *new_sub = !next ? (struct bintree*)tree->root : (next_from_left ? next->left : next->right);
                        if (avl_get_balance((struct avl_node*)new_sub) != EVEN) stop = true;
                    }
                    break;
            }
        }
        if (stop) break;
        curr = next;
        from_left = next_from_left;
    }
}

struct avl_node *avl_search(struct avl *tree, const void *data, bintree_cmp_cb cmp)
{
    assert(tree != NULL);
    if (tree->root == NULL)
        return NULL;
    return (struct avl_node *) bintree_search((struct bintree *) tree->root, data, cmp);
}

/* =========================================================================
 * Traversal
 * ========================================================================= */

// *** Helper functions *** //

static void avl_insert_balance_left(struct avl *tree, struct avl_node *root)
{
    struct avl_node *left_node = (struct avl_node *)root->btree.left;
    assert(left_node != NULL);
    // EVEN should not occur
    if (avl_get_balance(left_node) == LEFT_HIGH) {
        avl_rotate_right(tree, root); 
    } else {
        // Eliminated EVEN check and set_balance calls, since it is automatically
        // handled in rotaters. 
        struct avl_node *sub_right = (struct avl_node *) left_node->btree.right;
        int sub_right_balance = avl_get_balance(sub_right);
        avl_rotate_left(tree, left_node);
        avl_rotate_right(tree, root);
        switch (sub_right_balance)
        {
        //case LEFT_HIGH: avl_set_balance(root, RIGHT_HIGH); break;
        //case RIGHT_HIGH: avl_set_balance(left_node, LEFT_HIGH); break;
        case LEFT_HIGH: avl_set_balance_reckless(root, RIGHT_HIGH); break;
        case RIGHT_HIGH: avl_set_balance_reckless(left_node, LEFT_HIGH); break;
        default: break;
        }
    }
}

static void avl_insert_balance_right(struct avl *tree, struct avl_node *root)
{
    struct avl_node *right_node = (struct avl_node *)root->btree.right;
    assert(right_node != NULL);
    // EVEN should not occur
    if (avl_get_balance(right_node) == RIGHT_HIGH) {
        avl_rotate_left(tree, root); 
    } else {
        // Eliminated EVEN check and set_balance calls, since it is automatically
        // handled in rotaters. 
        struct avl_node *sub_left = (struct avl_node *) right_node->btree.left;
        int sub_left_balance = avl_get_balance(sub_left);
        avl_rotate_right(tree, right_node);
        avl_rotate_left(tree, root);
        switch (sub_left_balance)
        {
        //case RIGHT_HIGH: avl_set_balance(root, LEFT_HIGH); break;
        //case LEFT_HIGH: avl_set_balance(right_node, RIGHT_HIGH); break;
        case RIGHT_HIGH: avl_set_balance_reckless(root, LEFT_HIGH); break;
        case LEFT_HIGH: avl_set_balance_reckless(right_node, RIGHT_HIGH); break;
        default: break;
        }
    }
}

// See comments in insert balancers above.

static void avl_remove_balance_left(struct avl *tree, struct avl_node *root)
{
    struct avl_node *right_node = (struct avl_node *)root->btree.right;
    assert(right_node != NULL);
    switch (avl_get_balance(right_node))
    {
    case RIGHT_HIGH: avl_rotate_left(tree, root); break;
    case EVEN:
    {
        avl_rotate_left(tree, root);
        //avl_set_balance(root, RIGHT_HIGH);
        //avl_set_balance(right_node, LEFT_HIGH);
        avl_set_balance_reckless(root, RIGHT_HIGH);
        avl_set_balance_reckless(right_node, LEFT_HIGH);
        break;
    }
    case LEFT_HIGH:
    {
        struct avl_node *sub_left = (struct avl_node *) right_node->btree.left;
        int sub_left_balance = avl_get_balance(sub_left);
        avl_rotate_right(tree, right_node);
        avl_rotate_left(tree, root);
        switch (sub_left_balance)
        {
        //case RIGHT_HIGH: avl_set_balance(root, LEFT_HIGH); break;
        //case LEFT_HIGH: avl_set_balance(right_node, RIGHT_HIGH); break;
        case RIGHT_HIGH: avl_set_balance_reckless(root, LEFT_HIGH); break;
        case LEFT_HIGH: avl_set_balance_reckless(right_node, RIGHT_HIGH); break;
        default: break;
        }
    }
    }
}

static void avl_remove_balance_right(struct avl *tree, struct avl_node *root)
{
    struct avl_node *left_node = (struct avl_node *)root->btree.left;
    assert(left_node != NULL);
    switch (avl_get_balance(left_node))
    {
    case LEFT_HIGH: avl_rotate_right(tree, root); break;
    case EVEN:
    {
        avl_rotate_right(tree, root);
        //avl_set_balance(root, LEFT_HIGH);
        //avl_set_balance(left_node, RIGHT_HIGH);
        avl_set_balance_reckless(root, LEFT_HIGH);
        avl_set_balance_reckless(left_node, RIGHT_HIGH);
        break;
    }
    case RIGHT_HIGH:
    {
        struct avl_node *sub_right = (struct avl_node *) left_node->btree.right;
        int sub_right_balance = avl_get_balance(sub_right);
        avl_rotate_left(tree, left_node);
        avl_rotate_right(tree, root);
        switch (sub_right_balance)
        {
        //case LEFT_HIGH: avl_set_balance(root, RIGHT_HIGH); break;
        //case RIGHT_HIGH: avl_set_balance(left_node, LEFT_HIGH); break;
        case LEFT_HIGH: avl_set_balance_reckless(root, RIGHT_HIGH); break;
        case RIGHT_HIGH: avl_set_balance_reckless(left_node, LEFT_HIGH); break;
        default: break;
        }
    }
    }
}

static void avl_rotate_left(struct avl *tree, struct avl_node *root)
{
    struct bintree *root_bt = &root->btree;
    struct bintree *new_root_bt = root_bt->right;
    struct bintree *grandparent_bt = bintree_get_parent(root_bt);
    struct bintree *transfer_bt = new_root_bt->left;
    // 1. Move Transfer Node (MUST PRESERVE TAGS - it's an innocent bystander)
    root_bt->right = transfer_bt;
    if (transfer_bt)
        bintree_set_parent(transfer_bt, root_bt);
    // 2. Link Root and New Root (RECKLESS - Wipes tags to EVEN)
    // root_bt is given by bintree_get_parent, which wipes the tag out.
    // so new_root_bt->right becomes automatically EVEN
    new_root_bt->left = root_bt;
    root_bt->parent = new_root_bt; // Since new_root_bt is left of root_bt and left and right childs never tagged, this becomes EVEN
    // 3. Link to Grandparent (RECKLESS - Wipes tags to EVEN)
    new_root_bt->parent = grandparent_bt; // bintree_get_parent wiped tag out. Reset new_root to EVEN
    if (grandparent_bt) {
        if (grandparent_bt->right == root_bt)
            grandparent_bt->right = new_root_bt;
        else
            grandparent_bt->left = new_root_bt;
    } else {
        tree->root = (struct avl_node*)new_root_bt;
    }
}

static void avl_rotate_right(struct avl *tree, struct avl_node *root)
{
    struct bintree *root_bt = &root->btree;
    struct bintree *new_root_bt = root_bt->left;
    struct bintree *grandparent_bt = bintree_get_parent(root_bt);
    struct bintree *transfer_bt = new_root_bt->right;
    // 1. Move Transfer Node (MUST PRESERVE TAGS)
    root_bt->left = transfer_bt;
    if (transfer_bt)
        bintree_set_parent(transfer_bt, root_bt);
    // 2. Link Root and New Root (RECKLESS - Wipes tags to EVEN)
    // root_bt is given by bintree_get_parent, which wipes the tag out.
    // so new_root_bt->right becomes automatically EVEN
    new_root_bt->right = root_bt;
    root_bt->parent = new_root_bt; // Since new_root_bt is left of root_bt and left and right childs never tagged, this becomes EVEN
    // 3. Link to Grandparent (RECKLESS - Wipes tags to EVEN)
    new_root_bt->parent = grandparent_bt; // bintree_get_parent wiped tag out. Reset new_root to EVEN
    if (grandparent_bt) {
        if (grandparent_bt->left == root_bt)
            grandparent_bt->left = new_root_bt;
        else
            grandparent_bt->right = new_root_bt;
    } else {
        tree->root = (struct avl_node*) new_root_bt;
    }
}

static void avl_deinit_helper(struct avl_node *node, struct object_concept *oc)
{
    if (!node)
        return;
    avl_deinit_helper((struct avl_node *) node->btree.left, oc);
    avl_deinit_helper((struct avl_node *) node->btree.right, oc);
    oc->deinit(node);
}