#include <ds/trees/bst.h>
#include <ds/utils/object_concept.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*───────────────────────────────────────────────
 * Data structure with intrusive node
 *───────────────────────────────────────────────*/

typedef struct {
    struct bintree node;  // Intrusive node MUST be first or use container_of
    int id;
    char name[32];
    int age;
} Person;

/*───────────────────────────────────────────────
 * Comparison functions
 *───────────────────────────────────────────────*/

// For BST operations (node to node comparison)
static int person_cmp(const struct bintree* a, const struct bintree* b)
{
    const Person* pa = (const Person*)a;
    const Person* pb = (const Person*)b;
    return (pa->id > pb->id) - (pa->id < pb->id);
}

// For search operations (key to node comparison)
static int person_search_cmp(const void* key, const struct bintree* node)
{
    const int* key_id = (const int*)key;
    const Person* p = (const Person*)node;
    return (*key_id > p->id) - (*key_id < p->id);
}

/*───────────────────────────────────────────────
 * Helpers
 *───────────────────────────────────────────────*/

static Person* make_person(int id, const char* name, int age)
{
    Person* p = malloc(sizeof(Person));
    assert(p != NULL);
    
    // Initialize intrusive node
    p->node.parent = NULL;
    p->node.left = NULL;
    p->node.right = NULL;
    
    p->id = id;
    strncpy(p->name, name, sizeof(p->name));
    p->name[sizeof(p->name) - 1] = '\0';
    p->age = age;
    return p;
}

static void print_person(struct bintree* node, void* userdata)
{
    (void)userdata;
    Person* p = (Person*)node;
    printf("[%d, %s, %d] ", p->id, p->name, p->age);
}

static void person_deallocator(void* gnode)
{
    struct bintree* node = gnode;
    Person* p = (Person*)node;
    printf("Deallocating person: id=%d, name=%s, age=%d\n", p->id, p->name, p->age);
    free(p);
}

struct object_concept oc = { .init = NULL, .deinit = person_deallocator };

/*───────────────────────────────────────────────
 * Test Cases
 *───────────────────────────────────────────────*/

static void test_create_destroy(void)
{
    printf("TEST: init/deinit\n");
    
    struct bst tree;
    bst_init(&tree, person_cmp);
    assert(bst_empty(&tree));

    // bst_deinit(&tree, NULL);
    printf(" → PASSED\n\n");
}

static void test_insert_and_search(void)
{
    printf("TEST: insert/search complex objects\n");
    
    struct bst tree;
    bst_init(&tree, person_cmp);

    Person* alice = make_person(3, "Alice", 30);
    Person* bob   = make_person(1, "Bob", 25);
    Person* carol = make_person(5, "Carol", 28);

    assert(bst_add(&tree, &alice->node) == 0);
    assert(bst_add(&tree, &bob->node)   == 0);
    assert(bst_add(&tree, &carol->node) == 0);

    // Duplicate ID
    Person* dup = make_person(3, "Alice Clone", 99);
    assert(bst_add(&tree, &dup->node) == 1);  // Returns 1 for duplicate
    free(dup);

    // Search existing - pass key as void pointer
    int key_id = 1;
    struct bintree* found = bst_search(&tree, &key_id, person_search_cmp);
    assert(found != NULL);
    Person* found_person = (Person*)found;
    assert(strcmp(found_person->name, "Bob") == 0);

    // Search missing
    int missing_id = 42;
    assert(bst_search(&tree, &missing_id, person_search_cmp) == NULL);

    bst_deinit(&tree, &oc);

    printf(" → PASSED\n\n");
}

static void test_traversals(void)
{
    printf("TEST: traversals complex objects\n");
    
    struct bst tree;
    bst_init(&tree, person_cmp);

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
        bst_add(&tree, &arr[i]->node);

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
    
    struct bst tree;
    bst_init(&tree, person_cmp);

    int ids[] = {8, 3, 10, 1, 6, 14, 4, 7, 13};
    const char* names[] = {"P8","P3","P10","P1","P6","P14","P4","P7","P13"};

    Person* persons[9];
    for (int i = 0; i < 9; ++i) {
        persons[i] = make_person(ids[i], names[i], 20 + i);
        bst_add(&tree, &persons[i]->node);
    }

    // Search and remove leaf (13)
    int key1 = 13;
    struct bintree* node1 = bst_search(&tree, &key1, person_search_cmp);
    assert(node1 != NULL);
    bst_remove(&tree, node1);
    printf(" Removed: id=%d\n", ((Person*)node1)->id);
    free(node1);

    // Search and remove one-child (14)
    int key2 = 14;
    struct bintree* node2 = bst_search(&tree, &key2, person_search_cmp);
    assert(node2 != NULL);
    bst_remove(&tree, node2);
    printf(" Removed: id=%d\n", ((Person*)node2)->id);
    free(node2);

    // Search and remove two-children (3)
    int key3 = 3;
    struct bintree* node3 = bst_search(&tree, &key3, person_search_cmp);
    assert(node3 != NULL);
    bst_remove(&tree, node3);
    printf(" Removed: id=%d\n", ((Person*)node3)->id);
    free(node3);

    // Search for non-existent
    int key4 = 99;
    struct bintree* node4 = bst_search(&tree, &key4, person_search_cmp);
    assert(node4 == NULL);
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
    
    struct bst tree;
    bst_init(&tree, person_cmp);

    assert(bst_size(&tree) == 0);

    Person* p1 = make_person(1, "One", 20);
    Person* p2 = make_person(2, "Two", 21);
    Person* p3 = make_person(3, "Three", 22);

    bst_add(&tree, &p1->node);
    assert(bst_size(&tree) == 1);

    bst_add(&tree, &p2->node);
    assert(bst_size(&tree) == 2);

    bst_add(&tree, &p3->node);
    assert(bst_size(&tree) == 3);

    int key = 2;
    struct bintree* node = bst_search(&tree, &key, person_search_cmp);
    assert(node != NULL);
    bst_remove(&tree, node);
    assert(bst_size(&tree) == 2);
    free(node);

    printf(" Size tracking correct: 0 → 1 → 2 → 3 → 2\n");

    bst_deinit(&tree, &oc);

    printf(" → PASSED\n\n");
}

/*───────────────────────────────────────────────
 * MAIN
 *───────────────────────────────────────────────*/

int main(void)
{
    printf("────────── BST WITH INTRUSIVE NODES TEST ──────────\n\n");
    test_create_destroy();
    test_insert_and_search();
    test_traversals();
    test_removal();
    test_size_tracking();
    printf("All intrusive-node tests PASSED ✅\n");
    return 0;
}