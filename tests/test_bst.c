#include "../include/bst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*───────────────────────────────────────────────
 * Data structure
 *───────────────────────────────────────────────*/

typedef struct {
    int id;
    char name[32];
    int age;
} Person;

/*───────────────────────────────────────────────
 * Comparison and deallocator
 *───────────────────────────────────────────────*/

static int person_cmp(const void* a, const void* b)
{
    const Person* pa = a;
    const Person* pb = b;
    return (pa->id > pb->id) - (pa->id < pb->id);
}

static void person_deallocator(void* item, void* userdata)
{
    (void)userdata;
    Person* per = (Person*) item;
    printf("Deallocating person at %p\n", per);
    printf("id: %d, name: %s, age: %d\n", per->id, per->name, per->age);
    free(item);  // Free the Person allocated on heap
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

/*───────────────────────────────────────────────
 * Test Cases
 *───────────────────────────────────────────────*/

static void test_create_destroy(void)
{
    printf("TEST: create/destroy\n");
    struct bst* tree = bst_create(person_cmp);
    assert(tree);
    assert(bst_empty(tree));
    bst_destroy(tree, person_deallocator);
    printf(" → PASSED\n\n");
}

static void test_insert_and_search(void)
{
    printf("TEST: insert/search complex objects\n");
    struct bst* tree = bst_create(person_cmp);

    Person* alice = make_person(3, "Alice", 30);
    Person* bob   = make_person(1, "Bob", 25);
    Person* carol = make_person(5, "Carol", 28);

    assert(bst_add(tree, alice) == OK);
    assert(bst_add(tree, bob)   == OK);
    assert(bst_add(tree, carol) == OK);

    // Duplicate ID
    Person* dup = make_person(3, "Alice Clone", 99);
    assert(bst_add(tree, dup) == DUPLICATE_KEY);
    free(dup); // not inserted, must free manually

    // Search existing
    Person key = {.id = 1};
    Person* found = bst_search(tree, &key);
    assert(found && strcmp(found->name, "Bob") == 0);

    // Search missing
    Person notfound = {.id = 42};
    assert(bst_search(tree, &notfound) == NULL);

    bst_destroy(tree, person_deallocator);
    printf(" → PASSED\n\n");
}

static void test_min_max(void)
{
    printf("TEST: min/max complex objects\n");
    struct bst* tree = bst_create(person_cmp);

    bst_add(tree, make_person(10, "Zoe", 24));
    bst_add(tree, make_person(4, "Mark", 45));
    bst_add(tree, make_person(20, "Liam", 32));

    Person* min = bst_min(tree);
    Person* max = bst_max(tree);

    assert(min && min->id == 4);
    assert(max && max->id == 20);

    printf(" Min: %s, Max: %s\n", min->name, max->name);

    bst_destroy(tree, person_deallocator);
    printf(" → PASSED\n\n");
}

static void test_traversals(void)
{
    printf("TEST: traversals complex objects\n");
    struct bst* tree = bst_create(person_cmp);

    bst_add(tree, make_person(4, "Mia", 23));
    bst_add(tree, make_person(2, "Eve", 29));
    bst_add(tree, make_person(6, "Tom", 31));
    bst_add(tree, make_person(1, "Bob", 25));
    bst_add(tree, make_person(3, "Nina", 33));
    bst_add(tree, make_person(5, "Alex", 40));
    bst_add(tree, make_person(7, "Ray", 22));

    printf(" Preorder: ");
    bst_walk(tree, NULL, print_person, PREORDER);
    printf("\n Inorder:  ");
    bst_walk(tree, NULL, print_person, INORDER);
    printf("\n Postorder:");
    bst_walk(tree, NULL, print_person, POSTORDER);
    printf("\n");

    bst_destroy(tree, person_deallocator);
    printf(" → PASSED\n\n");
}

static void test_removal(void)
{
    printf("TEST: removal complex objects\n");
    struct bst* tree = bst_create(person_cmp);

    int ids[] = {8, 3, 10, 1, 6, 14, 4, 7, 13};
    const char* names[] = {"P8","P3","P10","P1","P6","P14","P4","P7","P13"};

    for (int i = 0; i < 9; ++i)
        bst_add(tree, make_person(ids[i], names[i], 20 + i));

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

    bst_destroy(tree, person_deallocator);
    printf(" → PASSED\n\n");
}

/*───────────────────────────────────────────────
 * MAIN
 *───────────────────────────────────────────────*/

int main(void)
{
    printf("────────── BST COMPLEX OBJECT TEST ──────────\n\n");
    test_create_destroy();
    test_insert_and_search();
    test_min_max();
    test_traversals();
    test_removal();
    printf("All complex-object tests PASSED ✅\n");
    return 0;
}