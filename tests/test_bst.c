#include <ds/trees/bst.h>
#include <ds/utils/allocator_concept.h>
#include <ds/utils/object_concept.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAGIC bst_node_sizeof()

/*───────────────────────────────────────────────
 * Data structure
 *───────────────────────────────────────────────*/

typedef struct {
    int id;
    char name[32];
    int age;
} Person;

/*───────────────────────────────────────────────
 * Comparison
 *───────────────────────────────────────────────*/

static int person_cmp(const void* a, const void* b)
{
    const Person* pa = a;
    const Person* pb = b;
    return (pa->id > pb->id) - (pa->id < pb->id);
}

/*───────────────────────────────────────────────
 * Helpers
 *───────────────────────────────────────────────*/

static Person* make_person(int id, const char* name, int age)
{
    Person* p = malloc(sizeof(Person));
    assert(p != NULL);
    p->id = id;
    strncpy(p->name, name, sizeof(p->name));
    p->name[sizeof(p->name) - 1] = '\0';
    p->age = age;
    return p;
}

static void print_person(void* item, void* userdata)
{
    (void)userdata;
    Person* p = item;
    printf("[%d, %s, %d] ", p->id, p->name, p->age);
}

static void person_deallocator(void* item)
{
    Person* p = item;
    printf("Deallocating person: id=%d, name=%s, age=%d\n", p->id, p->name, p->age);
    free(p);
}

struct object_concept oc = { .init = NULL, .deinit = person_deallocator };

/*───────────────────────────────────────────────
 * Test Cases
 *───────────────────────────────────────────────*/

static void init_test_tree(struct bst* tree, struct allocator_concept* ac)
{
    bst_init(tree, person_cmp, ac);
}

static void test_create_destroy(void)
{
    printf("TEST: init/deinit\n");
    struct syspool pool = { .obj_size = MAGIC };

    struct allocator_concept ac = {
        &pool, 
        (GENERIC_ALLOC_SIGN) sysalloc, 
        (GENERIC_FREE_SIGN) sysfree
    };
    
    struct bst tree;
    init_test_tree(&tree, &ac);
    assert(bst_empty(&tree));

    bst_deinit(&tree, NULL);       // No need to free nodes; pool owns memory
    printf(" → PASSED\n\n");
}

static void test_insert_and_search(void)
{
    printf("TEST: insert/search complex objects\n");
    struct syspool pool = { .obj_size = MAGIC };

    struct allocator_concept ac = {
        &pool, 
        (GENERIC_ALLOC_SIGN) sysalloc, 
        (GENERIC_FREE_SIGN) sysfree
    };
    struct bst tree;
    init_test_tree(&tree, &ac);

    Person* alice = make_person(3, "Alice", 30);
    Person* bob   = make_person(1, "Bob", 25);
    Person* carol = make_person(5, "Carol", 28);

    assert(bst_add(&tree, alice) == TREES_OK);
    assert(bst_add(&tree, bob)   == TREES_OK);
    assert(bst_add(&tree, carol) == TREES_OK);

    // Duplicate ID
    Person* dup = make_person(3, "Alice Clone", 99);
    assert(bst_add(&tree, dup) == TREES_DUPLICATE_KEY);
    free(dup); // not inserted

    // Search existing
    Person key = {.id = 1};
    Person* found = bst_search(&tree, &key);
    assert(found && strcmp(found->name, "Bob") == 0);

    // Search missing
    Person notfound = {.id = 42};
    assert(bst_search(&tree, &notfound) == NULL);

    bst_deinit(&tree, &oc);

    printf(" → PASSED\n\n");
}

static void test_traversals(void)
{
    printf("TEST: traversals complex objects\n");
    struct syspool pool = { .obj_size = MAGIC };

    struct allocator_concept ac = {
        &pool, 
        (GENERIC_ALLOC_SIGN) sysalloc, 
        (GENERIC_FREE_SIGN) sysfree
    };
    struct bst tree;
    init_test_tree(&tree, &ac);

    Person* arr[] = {
        make_person(4, "Mia", 23),
        make_person(2, "Eve", 29),
        make_person(6, "Tom", 31),
        make_person(1, "Bob", 25),
        make_person(3, "Nina", 33),
        make_person(5, "Alex", 40),
        make_person(7, "Ray", 22)
    };

    for (int i = 0; i < 7; ++i)
        bst_add(&tree, arr[i]);

    printf(" Preorder: ");
    bintree_traverse(bst_root(&tree), NULL, print_person, PREORDER);
    printf("\n Inorder:  ");
    bintree_traverse(bst_root(&tree), NULL, print_person, INORDER);
    printf("\n Postorder:");
    bintree_traverse(bst_root(&tree), NULL, print_person, POSTORDER);
    printf("\n");

    bst_deinit(&tree, &oc);

    printf(" → PASSED\n\n");
}

static void test_removal(void)
{
    printf("TEST: removal complex objects\n");
    struct syspool pool = { .obj_size = MAGIC };

    struct allocator_concept ac = {
        &pool, 
        (GENERIC_ALLOC_SIGN) sysalloc, 
        (GENERIC_FREE_SIGN) sysfree
    };
    struct bst tree;
    init_test_tree(&tree, &ac);

    int ids[] = {8, 3, 10, 1, 6, 14, 4, 7, 13};
    const char* names[] = {"P8","P3","P10","P1","P6","P14","P4","P7","P13"};

    Person* persons[9];
    for (int i = 0; i < 9; ++i) {
        persons[i] = make_person(ids[i], names[i], 20 + i);
        bst_add(&tree, persons[i]);
    }

    // Remove leaf (13)
    Person k1 = {.id = 13};
    void* removed1 = bst_remove(&tree, &k1);
    assert(removed1 != NULL);
    printf(" Removed: id=%d\n", ((Person*)removed1)->id);

    // Remove one-child (14)
    Person k2 = {.id = 14};
    void* removed2 = bst_remove(&tree, &k2);
    assert(removed2 != NULL);
    printf(" Removed: id=%d\n", ((Person*)removed2)->id);

    // Remove two-children (3)
    Person k3 = {.id = 3};
    void* removed3 = bst_remove(&tree, &k3);
    assert(removed3 != NULL);
    printf(" Removed: id=%d\n", ((Person*)removed3)->id);

    // Not found
    Person k4 = {.id = 99};
    void* removed4 = bst_remove(&tree, &k4);
    assert(removed4 == NULL);
    printf(" Not found: id=99 (expected)\n");

    printf(" Inorder after removals: ");
    bintree_traverse(bst_root(&tree), NULL, print_person, INORDER);
    printf("\n");

    bst_deinit(&tree, &oc);

    printf(" → PASSED\n\n");
}

static void test_size_tracking(void)
{
    printf("TEST: size tracking\n");
    struct syspool pool = { .obj_size = MAGIC };

    struct allocator_concept ac = {
        &pool, 
        (GENERIC_ALLOC_SIGN) sysalloc, 
        (GENERIC_FREE_SIGN) sysfree
    };
    struct bst tree;
    init_test_tree(&tree, &ac);

    assert(bst_size(&tree) == 0);

    Person* p1 = make_person(1, "One", 20);
    Person* p2 = make_person(2, "Two", 21);
    Person* p3 = make_person(3, "Three", 22);

    bst_add(&tree, p1);
    assert(bst_size(&tree) == 1);

    bst_add(&tree, p2);
    assert(bst_size(&tree) == 2);

    bst_add(&tree, p3);
    assert(bst_size(&tree) == 3);

    Person k = {.id = 2};
    bst_remove(&tree, &k);
    assert(bst_size(&tree) == 2);

    printf(" Size tracking correct: 0 → 1 → 2 → 3 → 2\n");

    bst_deinit(&tree, &oc);

    printf(" → PASSED\n\n");
}

/*───────────────────────────────────────────────
 * MAIN
 *───────────────────────────────────────────────*/

int main(void)
{
    printf("────────── BST WITH CHUNKED POOL TEST ──────────\n\n");
    test_create_destroy();
    test_insert_and_search();
    test_traversals();
    test_removal();
    test_size_tracking();
    printf("All complex-object tests PASSED ✅\n");
    return 0;
}