#include <ds/trees/avl.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>


#define AVL_MAX_DEPTH 64 // Means 2^64 node max, practically infinity

static void avl_insert_balance_left(struct avl *tree, struct avl_node *root);
static void avl_insert_balance_right(struct avl *tree, struct avl_node *root);
static void avl_remove_balance_left(struct avl *tree, struct avl_node *root);
static void avl_remove_balance_right(struct avl *tree, struct avl_node *root);
static struct avl_node *avl_rotate_left(struct avl *tree, struct avl_node *root);
static struct avl_node *avl_rotate_right(struct avl *tree, struct avl_node *root);
static void avl_deinit_helper(struct avl_node *node, struct object_concept *oc);

// https://stackoverflow.com/questions/17288746/red-black-nodes-struct-alignment-in-linux-kernel?rq=1

/** @enum avl_balance */
enum avl_balance {
    LEFT_HIGH = 1,      ///< represents '01' in our parent pointer.
    EVEN = 0,           ///< represents '00' in our parent pointer.
    RIGHT_HIGH = 3      ///< represents '11' in our parent pointer.
};

#define avl_get_parent(node)   ((struct bintree *)((uintptr_t)((node)->btree.parent) & ~3))
#define avl_set_parent(node, new_parent) do { \
    (node)->btree.parent = (void *)(((uintptr_t)(new_parent)) | ((uintptr_t)((node)->btree.parent) & 3UL)); \
} while (0)
#define avl_get_balance(node)   ((enum avl_balance)((uintptr_t)((node)->btree.parent) & 3UL))
#define avl_set_balance(node, bal) do { \
    (node)->btree.parent = (void *)((((uintptr_t)((node)->btree.parent)) & ~3UL) | (uintptr_t)(bal)); \
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
    avl_set_balance(new_node, EVEN);
    avl_set_parent(new_node, parent);
    *link = &new_node->btree;
    tree->size++;
    struct avl_node *curr = new_node;
    parent = avl_get_parent(curr);
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
        parent = avl_get_parent(curr);
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
        while (s->left)
            s = s->left;
        struct avl_node *sn = (struct avl_node *)s;
        // A. Save State (Balances of nodes and their children)
        enum avl_balance b_n = avl_get_balance(node);
        enum avl_balance b_s = avl_get_balance(sn);
        // We must save n's children's balances because bintree_swap will overwrite 
        // their parent pointers with raw pointers, erasing their tags.
        enum avl_balance b_n_left = avl_get_balance((struct avl_node*)n->left);
        // If s is directly n->right, n->right becomes n (whose balance is handled by b_s logic below)
        enum avl_balance b_n_right = (n->right == s) ? EVEN : avl_get_balance((struct avl_node*)n->right);
        // B. Sanitize Parents (Strip tags so bintree_swap works safely)
        n->parent = avl_get_parent(node);
        s->parent = avl_get_parent(sn);
        // C. Generic Swap
        bintree_swap(n, s);
        // D. Fix Tree Root (Crucial Step!)
        // bintree_swap can't see tree->root. If s moved to the top, we must update it.
        if (s->parent == NULL) tree->root = sn;
        // E. Restore Node Balances (Swapped positions)
        avl_set_balance(node, b_s); // node is now at s's old spot (bottom)
        avl_set_balance(sn, b_n);   // sn is now at n's old spot (top)
        // F. Restore Children Balances
        // sn now has n's old children. We must re-tag them.
        if (sn->btree.left) avl_set_balance((struct avl_node*)sn->btree.left, b_n_left);
        if (sn->btree.right && sn->btree.right != n) avl_set_balance((struct avl_node*)sn->btree.right, b_n_right);
    }
    struct bintree *child = n->left ? n->left : n->right;
    struct bintree *parent = avl_get_parent(node);
    bool from_left = parent ? (parent->left == n) : false;
    if (child)
        avl_set_parent((struct avl_node*)child, parent);
    if (!parent) {
        tree->root = (struct avl_node*)child;
    } else {
        if (from_left) parent->left = child;
        else parent->right = child;
    }
    bintree_init(n, NULL, NULL, NULL);
    tree->size--;
    struct bintree *curr = parent;
    while (curr) {
        struct avl_node *node_curr = (struct avl_node*)curr;
        struct bintree *next = avl_get_parent(node_curr);
        // Calculate direction for the NEXT iteration before rotation messes up links
        bool next_from_left = next ? (next->left == curr) : false;
        bool stop = false;
        if (from_left) { // Left side shortened
            switch (avl_get_balance(node_curr)) {
                case LEFT_HIGH:  
                    avl_set_balance(node_curr, EVEN); 
                    break; // Height reduced, continue up
                case EVEN:       
                    avl_set_balance(node_curr, RIGHT_HIGH); 
                    stop = true; // Height unchanged, stop
                    break; 
                case RIGHT_HIGH: 
                    avl_remove_balance_left(tree, node_curr);
                    // Get new root of this subtree
                    {
                        struct bintree *new_sub = !next ? (struct bintree*)tree->root : (next_from_left ? next->left : next->right);
                        // FIX: If EVEN, height reduced -> Continue. If NOT EVEN, height same -> Stop.
                        if (avl_get_balance((struct avl_node*)new_sub) != EVEN) stop = true;
                    }
                    break;
            }
        } else { // Right side shortened
            switch (avl_get_balance(node_curr)) {
                case RIGHT_HIGH: 
                    avl_set_balance(node_curr, EVEN); 
                    break; // Height reduced, continue up
                case EVEN:       
                    avl_set_balance(node_curr, LEFT_HIGH); 
                    stop = true; // Height unchanged, stop
                    break;
                case LEFT_HIGH:  
                    avl_remove_balance_right(tree, node_curr);
                    {
                        struct bintree *new_sub = !next ? (struct bintree*)tree->root : (next_from_left ? next->left : next->right);
                        // FIX: If EVEN, height reduced -> Continue. If NOT EVEN, height same -> Stop.
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
    switch (avl_get_balance(left_node)) {
    case LEFT_HIGH:
    {
        avl_set_balance(root, EVEN);
        avl_set_balance(left_node, EVEN);
        avl_rotate_right(tree, root); 
        break;
    }
    case EVEN: // Should not happen during insertion rebalancing
        break;
    case RIGHT_HIGH:
    {
        struct avl_node *right_of_left = (struct avl_node *)left_node->btree.right;
        assert(right_of_left != NULL);
        switch (avl_get_balance(right_of_left)) {
        case LEFT_HIGH:
            avl_set_balance(root, RIGHT_HIGH);
            avl_set_balance(left_node, EVEN);
            break;
        case RIGHT_HIGH:
            avl_set_balance(root, EVEN);
            avl_set_balance(left_node, LEFT_HIGH);
            break;
        case EVEN:
            avl_set_balance(root, EVEN);
            avl_set_balance(left_node, EVEN);
            break;
        }
        avl_set_balance(right_of_left, EVEN);
        avl_rotate_left(tree, left_node);
        avl_rotate_right(tree, root);
        break;
    }
    }
}

static void avl_insert_balance_right(struct avl *tree, struct avl_node *root)
{
    struct avl_node *right_node = (struct avl_node *)root->btree.right;
    assert(right_node != NULL);
    switch (avl_get_balance(right_node)) {
    case RIGHT_HIGH:
    {
        avl_set_balance(root, EVEN);
        avl_set_balance(right_node, EVEN);
        avl_rotate_left(tree, root);
        break;
    }
    case EVEN:
        break;
    case LEFT_HIGH:
    {
        struct avl_node *left_of_right = (struct avl_node *)right_node->btree.left;
        assert(left_of_right != NULL);
        switch (avl_get_balance(left_of_right)) {
        case RIGHT_HIGH:
            avl_set_balance(root, LEFT_HIGH);
            avl_set_balance(right_node, EVEN);
            break;
        case LEFT_HIGH:
            avl_set_balance(root, EVEN);
            avl_set_balance(right_node, RIGHT_HIGH);
            break;
        case EVEN:
            avl_set_balance(root, EVEN);
            avl_set_balance(right_node, EVEN);
            break;
        }
        avl_set_balance(left_of_right, EVEN);
        avl_rotate_right(tree, right_node);
        avl_rotate_left(tree, root);
        break;
    }
    }
}

static void avl_remove_balance_left(struct avl *tree, struct avl_node *root)
{
    struct avl_node *right_node = (struct avl_node *)root->btree.right;
    assert(right_node != NULL);
    switch (avl_get_balance(right_node))
    {
    case RIGHT_HIGH:
    {
        avl_set_balance(root, EVEN);
        avl_set_balance(right_node, EVEN);
        avl_rotate_left(tree, root);
        break;
    }
    case EVEN:
    {
        avl_set_balance(root, RIGHT_HIGH);
        avl_set_balance(right_node, LEFT_HIGH);
        avl_rotate_left(tree, root);
        break;
    }
    case LEFT_HIGH:
    {
        struct avl_node *left_of_right = (struct avl_node *)right_node->btree.left;
        switch (avl_get_balance(left_of_right))
        {
        case LEFT_HIGH:
            avl_set_balance(root, EVEN);
            avl_set_balance(right_node, RIGHT_HIGH);
            break;
        case EVEN:
            avl_set_balance(root, EVEN);
            avl_set_balance(right_node, EVEN);
            break;
        case RIGHT_HIGH:
            avl_set_balance(root, LEFT_HIGH);
            avl_set_balance(right_node, EVEN);
            break;
        }
        avl_set_balance(left_of_right, EVEN);
        avl_rotate_right(tree, right_node);
        avl_rotate_left(tree, root);
        break;
    }
    }
}

static void avl_remove_balance_right(struct avl *tree, struct avl_node *root)
{
    struct avl_node *left_node = (struct avl_node *)root->btree.left;
    assert(left_node != NULL);
    switch (avl_get_balance(left_node))
    {
    case LEFT_HIGH:
    {
        avl_set_balance(root, EVEN);
        avl_set_balance(left_node, EVEN);
        avl_rotate_right(tree, root);
        break;
    }
    case EVEN:
    {
        avl_set_balance(root, LEFT_HIGH);
        avl_set_balance(left_node, RIGHT_HIGH);
        avl_rotate_right(tree, root);
        break;
    }
    case RIGHT_HIGH:
    {
        struct avl_node *right_of_left = (struct avl_node *)left_node->btree.right;
        switch (avl_get_balance(right_of_left))
        {
        case RIGHT_HIGH:
            avl_set_balance(root, EVEN);
            avl_set_balance(left_node, LEFT_HIGH);
            break;
        case EVEN:
            avl_set_balance(root, EVEN);
            avl_set_balance(left_node, EVEN);
            break;
        case LEFT_HIGH:
            avl_set_balance(root, RIGHT_HIGH);
            avl_set_balance(left_node, EVEN);
            break;
        }
        avl_set_balance(right_of_left, EVEN);
        avl_rotate_left(tree, left_node);
        avl_rotate_right(tree, root);
        break;
    }
    }
}

static struct avl_node *avl_rotate_left(struct avl *tree, struct avl_node *root)
{
    struct bintree *root_bt = &root->btree;
    struct bintree *new_root_bt = root_bt->right;
    struct bintree *grandparent_bt = avl_get_parent(root);
    struct bintree *transfer_bt = new_root_bt->left;
    root_bt->right = transfer_bt;
    if (transfer_bt)
        avl_set_parent((struct avl_node*)transfer_bt, root_bt);
    new_root_bt->left = root_bt;
    avl_set_parent(root, new_root_bt);
    avl_set_parent((struct avl_node*)new_root_bt, grandparent_bt);
    if (grandparent_bt) {
        if (grandparent_bt->right == root_bt) {
            grandparent_bt->right = new_root_bt;
        } else {
            grandparent_bt->left = new_root_bt;
        }
    } else {
        tree->root = (struct avl_node*)new_root_bt;
    }
    return (struct avl_node*)new_root_bt;
}

static struct avl_node *avl_rotate_right(struct avl *tree, struct avl_node *root)
{
    struct bintree *root_bt = &root->btree;
    struct bintree *new_root_bt = root_bt->left;
    struct bintree *grandparent_bt = avl_get_parent(root);
    struct bintree *transfer_bt = new_root_bt->right;
    // Handle transferred node
    root_bt->left = transfer_bt;
    if (transfer_bt)
        avl_set_parent((struct avl_node*) transfer_bt, root_bt);
    // Set new node
    new_root_bt->right = root_bt;
    avl_set_parent(root, new_root_bt);
    avl_set_parent((struct avl_node*) new_root_bt, grandparent_bt);
    if (grandparent_bt) {
        if (grandparent_bt->left == root_bt) {
            grandparent_bt->left = new_root_bt;
        } else {
            grandparent_bt->right = new_root_bt;
        }
    } else {
        tree->root = (struct avl_node*) new_root_bt;
    }
    return (struct avl_node*) new_root_bt;
}

static void avl_deinit_helper(struct avl_node *node, struct object_concept *oc)
{
    if (!node)
        return;
    avl_deinit_helper((struct avl_node *) node->btree.left, oc);
    avl_deinit_helper((struct avl_node *) node->btree.right, oc);
    oc->deinit(node);
}