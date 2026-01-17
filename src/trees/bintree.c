#include <ds/trees/bintree.h>
#include <assert.h>
#include <string.h>
#include <ds/queue/lqueue.h>
#include <ds/stack/lstack.h>
#include <limits.h>

static inline void set_parent(struct bintree *child, struct bintree *parent);
static void bintree_size_helper(struct bintree *node, void* context);

/* =========================================================================
 * Create & Destroy
 * ========================================================================= */

void bintree_init(struct bintree *tree, struct bintree *parent, struct bintree *left, struct bintree *right)
{
    assert(tree != NULL);
    tree->parent = parent;
    tree->left = left;
    tree->right = right;
}

void bintree_deinit(struct bintree *tree, struct object_concept *oc) {
    assert(tree != NULL && oc != NULL && oc->deinit != NULL);
    struct bintree *curr = bintree_first_postorder(tree);
    while (curr) {
        struct bintree *next = bintree_postorder_next(curr);
        oc->deinit(curr);
        curr = next;
    }
}

/* =========================================================================
 * Getters & Setters
 * ========================================================================= */

struct bintree *bintree_get_root(struct bintree *node)
{
    assert(node != NULL);
    struct bintree *curr = node;
    while (curr->parent) {
        curr = curr->parent;
    }
    return curr;
}

/* =========================================================================
 * Subtree Ops
 * ========================================================================= */

void bintree_detach(struct bintree *node)
{
    assert(node != NULL);
    if (node->parent) {
        if (node == node->parent->left) {
            node->parent->left = NULL;
        } else {
            node->parent->right = NULL;
        }
        node->parent = NULL;
    }
}

void bintree_replace(struct bintree *old_node, struct bintree *new_node)
{
    assert(old_node != NULL && new_node != NULL);
    *new_node = *old_node;
    if (old_node->parent) {
        if (old_node->parent->left == old_node)
            old_node->parent->left = new_node;
        else
            old_node->parent->right = new_node;
    }
    if (new_node->left)
        new_node->left->parent = new_node;
    if (new_node->right)
        new_node->right->parent = new_node;
    memset(old_node, 0, sizeof(struct bintree));
}

void bintree_swap(struct bintree *n1, struct bintree *n2) {
    assert(n1 != NULL && n2 != NULL);
    if (n1 == n2)
        return;
    // 1. Swap the pointers of the parents pointing DOWN
    // We must find who n1 and n2 are in relation to their parents
    struct bintree **p1_link = (n1->parent) ? (n1->parent->left == n1 ? &n1->parent->left : &n1->parent->right) : NULL;
    struct bintree **p2_link = (n2->parent) ? (n2->parent->left == n2 ? &n2->parent->left : &n2->parent->right) : NULL;
    if (p1_link) *p1_link = n2;
    if (p2_link) *p2_link = n1;
    // 2. Swap the internal state (parent, left, right)
    struct bintree temp = *n1;
    *n1 = *n2;
    *n2 = temp;
    // 3. FIX ADJACENCY (The only logic-essential 'if')
    // If they were parent/child, the pointers are now swapped into a loop
    if (n1->parent == n1) {
        n1->parent = n2;
        if (n2->left == n2) n2->left = n1; else n2->right = n1;
    } else if (n2->parent == n2) {
        n2->parent = n1;
        if (n1->left == n1) n1->left = n2; else n1->right = n2;
    }
    // 4. Update the children's parent pointers to point to the new addresses
    set_parent(n1->left, n1);
    set_parent(n1->right, n1);
    set_parent(n2->left, n2);
    set_parent(n2->right, n2);
}

/* =========================================================================
 * Search
 * ========================================================================= */

struct bintree *bintree_search(struct bintree *tree, const void *key, bintree_cmp_cb cmp)
{
    assert(tree != NULL && cmp != NULL);
    struct bintree *curr = tree;
    while (curr) {
        int result = cmp(key, curr);
        if (result < 0) {
            curr = curr->left;
        } else if (result > 0) {
            curr = curr->right;
        } else {
            break;
        }
    }
    return curr;
}

struct bintree **bintree_search_parent(struct bintree **tree, const struct bintree *node, struct bintree **parent, bst_cmp_cb cmp)
{
    assert(tree != NULL);
    struct bintree *parent_ptr = (*tree) ? (*tree)->parent : NULL;
    struct bintree **curr = tree;
    while (*curr) {
        parent_ptr = *curr;
        int result = cmp(node, *curr);
        if (result < 0) {
            curr = &(*curr)->left;
        } else if (result > 0) {
            curr = &(*curr)->right;
        } else {
            break;
        }
    }
    *parent = parent_ptr;
    return curr;
}

/* =========================================================================
* Traversals
* ========================================================================= */

struct bintree *bintree_first_inorder(struct bintree *node)
{
    assert(node != NULL);
    while (node->left) {
        node = node->left;
    }
    return node;
}

struct bintree *bintree_first_postorder(struct bintree *node)
{
    assert(node != NULL);
    while (node->left || node->right) {
        node = node->left ? node->left : node->right;
    }
    return node;
}

struct bintree *bintree_preorder_prev(struct bintree *node)
{
    assert(node != NULL);
    struct bintree *p = node->parent;
    if (!p)
        return NULL;
    /* If we are the left child, or if the parent only has us (right child 
       but left is null), then the parent is the previous. */
    if (node == p->left || !p->left)
        return p;
    /* If we are the right child and there is a left subtree, the previous
       is the last node of that left subtree in pre-order. */
    node = p->left;
    while (node->right || node->left) {
        node = node->right ? node->right : node->left;
    }
    return node;
}

struct bintree *bintree_preorder_next(struct bintree *node) {
    assert(node != NULL);
    if (node->left)
        return node->left;
    if (node->right)
        return node->right;
    // If no children, go up until we find a parent with a right child 
    // that isn't the path we just came from.
    struct bintree *parent = node->parent;
    // parent->right == node is to avoid path we just handled
    while (parent && (parent->right == node || !parent->right)) {
        node = parent;
        parent = parent->parent;
    }
    return parent ? parent->right : NULL;
}

struct bintree *bintree_inorder_prev(struct bintree *node)
{
    assert(node != NULL);
    if (node->left) {
        node = node->left;
        while (node->right) {
            node = node->right;
        }
        return node;
    }
    while (node->parent && node == node->parent->left) {
        node = node->parent;
    }
    return node->parent;
}

struct bintree *bintree_inorder_next(struct bintree *node)
{
    assert(node != NULL);
    if (node->right) {
        node = node->right;
        while (node->left)
            node = node->left;
        return node;
    }
    while (node->parent && node == node->parent->right) {
        node = node->parent;
    }
    return node->parent;
}

struct bintree *bintree_postorder_prev(struct bintree *node)
{
    assert(node != NULL);
    /* In post-order, children come before the parent. 
       So the 'previous' node is the right child, or failing that, the left. */
    if (node->right)
        return node->right;
    if (node->left)
        return node->left;
    /* If it's a leaf, we need to go up until we find a node that is a 
       right child whose parent has a left child. */
    struct bintree *p = node->parent;
    while (p && (node == p->left || !p->left)) {
        node = p;
        p = p->parent;
    }
    return p ? p->left : NULL;
}

struct bintree *bintree_postorder_next(struct bintree *node)
{
    assert(node != NULL);
    struct bintree *p = node->parent;
    if (!p) return NULL; // Root is last in post-order
    // If we are the left child and parent has a right child, 
    // the next node is the "start" (deepest leaf) of the right subtree.
    if (p->right && p->left == node) {
        node = p->right;
        while (node->left || node->right) {
            node = node->left ? node->left : node->right;
        }
        return node;
    }
    return p;
}

void bintree_traverse(struct bintree *tree, void *context, bintree_handle_cb handler, enum traversal_order order) 
{
    assert(tree != NULL && handler != NULL);
    struct bintree *curr = NULL;
    switch (order) {
        case INORDER:
            curr = bintree_first_inorder(tree);
            while (curr) {
                handler(curr, context);
                curr = bintree_inorder_next(curr);
            }
            break;
        case PREORDER:
            curr = bintree_first_preorder(tree);
            while (curr) {
                handler(curr, context);
                curr = bintree_preorder_next(curr);
            }
            break;

        case POSTORDER:
            curr = bintree_first_postorder(tree);
            while (curr) {
                handler(curr, context);
                curr = bintree_postorder_next(curr);
            }
            break;
    }
}

/**
 * Would it be good practice trying to reuse graph implementation's bfs and dfs?
 * Should i establish a hierarchy between trees and graphs so that conversions from
 * trees to graphs become very easy, since trees are special graphs? I have no idea,
 * currently but will ponder. Gemini says no in short.
 */

void bintree_bfs(struct bintree *tree, void *context, bintree_handle_cb handler)
{
    assert(tree != NULL && handler != NULL);
    // TODO: Use a real pool after updating allocators repository.
    struct syspool item_pool = { lqueue_node_sizeof() };
    struct allocator_concept ac = { .allocator = &item_pool, .alloc = sysalloc, .free = sysfree };
    struct lqueue *lq = lqueue_create(&ac);
    struct bintree *curr = tree;
    lenqueue(lq, curr); 
    while (!lqueue_empty(lq)) {
        curr = ldequeue(lq);
        handler(curr, context);
        if (curr->left)
            lenqueue(lq, curr->left);
        if (curr->right)
            lenqueue(lq, curr->right);
    }
    lqueue_destroy(lq, NULL);
}

void bintree_dfs(struct bintree *tree, void *context, bintree_handle_cb handler)
{
    assert(tree != NULL && handler != NULL);
    // TODO: Use a real pool after updating allocators repository.
    struct syspool item_pool = { lstack_node_sizeof() };
    struct allocator_concept ac = { .allocator = &item_pool, .alloc = sysalloc, .free = sysfree };
    struct lstack *ls = lstack_create(&ac);
    struct bintree *curr = tree;
    lpush(ls, curr);
    while (!lstack_empty(ls)) {
        curr = lpop(ls);
        handler(curr, context);
        if (curr->right)
            lpush(ls, curr->right);
        if (curr->left)
            lpush(ls, curr->left);
    }
    lstack_destroy(ls, NULL);
}

/* =========================================================================
* Properties
* ========================================================================= */

size_t bintree_size(const struct bintree* tree)
{
    if (tree == NULL)
        return 0;
    size_t count = 0;
    bintree_traverse((struct bintree*) tree, &count, bintree_size_helper, INORDER);
    return count;
}

/**
 * I think trying to reuse very similar logic with @ref bintree_get_root
 * would be mental illness.
 */
size_t bintree_level(struct bintree *node, struct bintree **root)
{
    assert(node != NULL);
    size_t level = 0;
    struct bintree *curr = node;
    while (curr->parent) {
        level++;
        curr = curr->parent;
    }
    if (root)
        *root = curr;
    return level;
}

int bintree_height(const struct bintree* tree)
{
    if (tree == NULL)
        return -1; 
    int left_height = bintree_height(tree->left);
    int right_height = bintree_height(tree->right);
    return 1 + (left_height > right_height ? left_height : right_height);
}

int bintree_balance_factor(const struct bintree* tree)
{
    if (tree == NULL)
        return 0;
    return bintree_height(tree->left) - bintree_height(tree->right);
}

// *** Helper functions *** //

static inline void set_parent(struct bintree *child, struct bintree *parent)
{
    if (child)
        child->parent = parent;
}

static void bintree_size_helper(struct bintree *node, void* context)
{
    (void) node;
    size_t* count = (size_t*) context;
    (*count)++;
}