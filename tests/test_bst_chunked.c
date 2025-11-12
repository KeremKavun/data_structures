#include "../include/bst.h"
#include "../../allocators/include/chunked_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAGIC 24

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

static void person_deallocator(void* item, void* userdata)
{
    (void)userdata;
    Person* p = item;
    printf("Deallocating person: id=%d, name=%s, age=%d\n", p->id, p->name, p->age);
    free(p);
}

/*───────────────────────────────────────────────
 * Test Cases
 *───────────────────────────────────────────────*/

static struct bst* create_test_tree(struct chunked_pool* pool)
{
    return bst_create(person_cmp, pool);
}

static void test_create_destroy(void)
{
    printf("TEST: create/destroy\n");
    struct chunked_pool* pool = chunked_pool_create(10, MAGIC);
    assert(pool);

    struct bst* tree = create_test_tree(pool);
    assert(tree);
    assert(bst_empty(tree));

    bst_destroy(tree);       // No need to free nodes; pool owns memory
    chunked_pool_destroy(pool);
    printf(" → PASSED\n\n");
}

static void test_insert_and_search(void)
{
    printf("TEST: insert/search complex objects\n");
    struct chunked_pool* pool = chunked_pool_create(10, MAGIC);
    struct bst* tree = create_test_tree(pool);

    Person* alice = make_person(3, "Alice", 30);
    Person* bob   = make_person(1, "Bob", 25);
    Person* carol = make_person(5, "Carol", 28);

    assert(bst_add(tree, alice) == OK);
    assert(bst_add(tree, bob)   == OK);
    assert(bst_add(tree, carol) == OK);

    // Duplicate ID
    Person* dup = make_person(3, "Alice Clone", 99);
    assert(bst_add(tree, dup) == DUPLICATE_KEY);
    free(dup); // not inserted

    // Search existing
    Person key = {.id = 1};
    Person* found = bst_search(tree, &key);
    assert(found && strcmp(found->name, "Bob") == 0);

    // Search missing
    Person notfound = {.id = 42};
    assert(bst_search(tree, &notfound) == NULL);

    bst_destroy(tree);
    chunked_pool_destroy(pool);

    free(alice);
    free(bob);
    free(carol);

    printf(" → PASSED\n\n");
}

static void test_min_max(void)
{
    printf("TEST: min/max complex objects\n");
    struct chunked_pool* pool = chunked_pool_create(10, MAGIC);
    struct bst* tree = create_test_tree(pool);

    Person* p1 = make_person(10, "Zoe", 24);
    Person* p2 = make_person(4, "Mark", 45);
    Person* p3 = make_person(20, "Liam", 32);

    bst_add(tree, p1);
    bst_add(tree, p2);
    bst_add(tree, p3);

    Person* min = bst_min(tree);
    Person* max = bst_max(tree);

    assert(min && min->id == 4);
    assert(max && max->id == 20);
    printf(" Min: %s, Max: %s\n", min->name, max->name);

    bst_destroy(tree);
    chunked_pool_destroy(pool);

    free(p1);
    free(p2);
    free(p3);

    printf(" → PASSED\n\n");
}

static void test_traversals(void)
{
    printf("TEST: traversals complex objects\n");
    struct chunked_pool* pool = chunked_pool_create(10, MAGIC);
    struct bst* tree = create_test_tree(pool);

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
        bst_add(tree, arr[i]);

    printf(" Preorder: ");
    bst_walk(tree, NULL, print_person, PREORDER);
    printf("\n Inorder:  ");
    bst_walk(tree, NULL, print_person, INORDER);
    printf("\n Postorder:");
    bst_walk(tree, NULL, print_person, POSTORDER);
    printf("\n");

    bst_destroy(tree);
    chunked_pool_destroy(pool);

    for (int i = 0; i < 7; ++i)
        free(arr[i]);

    printf(" → PASSED\n\n");
}

static void test_removal(void)
{
    printf("TEST: removal complex objects\n");
    struct chunked_pool* pool = chunked_pool_create(20, MAGIC);
    struct bst* tree = create_test_tree(pool);

    int ids[] = {8, 3, 10, 1, 6, 14, 4, 7, 13};
    const char* names[] = {"P8","P3","P10","P1","P6","P14","P4","P7","P13"};

    Person* persons[9];
    for (int i = 0; i < 9; ++i) {
        persons[i] = make_person(ids[i], names[i], 20 + i);
        bst_add(tree, persons[i]);
    }

    // Remove leaf (13)
    Person k1 = {.id = 13};
    assert(bst_remove(tree, &k1) == OK);

    // Remove one-child (14)
    Person k2 = {.id = 14};
    assert(bst_remove(tree, &k2) == OK);

    // Remove two-children (3)
    Person k3 = {.id = 3};
    assert(bst_remove(tree, &k3) == OK);

    // Not found
    Person k4 = {.id = 99};
    assert(bst_remove(tree, &k4) == NOT_FOUND);

    printf(" Inorder after removals: ");
    bst_walk(tree, NULL, print_person, INORDER);
    printf("\n");

    bst_destroy(tree);
    chunked_pool_destroy(pool);

    for (int i = 0; i < 9; ++i)
        free(persons[i]);

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
    test_min_max();
    test_traversals();
    test_removal();
    printf("All complex-object tests PASSED ✅\n");
    return 0;
}