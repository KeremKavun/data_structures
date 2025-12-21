#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "../include/dynarray.h" 

/* =========================================================================
 * MOCK OBJECT & CONCEPTS
 * We use this to verify that constructors and destructors are called correctly.
 * ========================================================================= */

int g_active_objects = 0; // Global counter to track leaks

struct TestObj {
    int value;
};

// Object Concept: Init (Copy Constructor)
int test_obj_init(void *self, void *args) {
    struct TestObj *s = (struct TestObj *)self;
    const struct TestObj *a = (const struct TestObj *)args;
    s->value = a->value;
    g_active_objects++;
    return 0;
}

// Object Concept: Deinit (Destructor)
void test_obj_deinit(void *self) {
    struct TestObj *s = (struct TestObj *)self;
    // Optional: Poison memory to detect use-after-free
    s->value = -999; 
    g_active_objects--;
}

// Global concept definition
struct object_concept TestObjConcept = {
    .init = test_obj_init,
    .deinit = test_obj_deinit
};

// Helper to print array (for debugging)
void print_arr(struct dynarray *arr) {
    printf("[ ");
    struct TestObj *iter = dynarray_iterator_begin(arr);
    struct TestObj *end = dynarray_iterator_end(arr);
    while (iter < end) {
        printf("%d ", iter->value);
        iter++;
    }
    printf("] (Size: %zu, Cap: %zu)\n", dynarray_size(arr), dynarray_capacity(arr));
}

/* =========================================================================
 * DEBUG PRINTER UTILITIES
 * ========================================================================= */

// Type definition for the visitor function
typedef void (*print_visitor_t)(const void *data);

/**
 * @brief Prints the array state: [ elem1, elem2, ... ]
 * Uses public iterator API to remain safe.
 */
void dynarray_print_debug(struct dynarray *arr, print_visitor_t printer) {
    void *curr = dynarray_iterator_begin(arr);
    void *end = dynarray_iterator_end(arr);
    
    printf("Debug Dump (Size: %zu, Cap: %zu):\n", dynarray_size(arr), dynarray_capacity(arr));
    printf("[ ");
    
    int index = 0;
    while (curr != end) {
        if (index > 0) printf(", ");
        printer(curr);
        
        // Move to next. Note: Your API asserts that 'curr' must be < end
        // before calling next, which is satisfied here by the while loop condition.
        curr = dynarray_iterator_next(arr, curr);
        index++;
    }
    printf(" ]\n--------------------------------------------\n");
}

// The specific callback for your TestObj
void print_test_obj(const void *data) {
    const struct TestObj *obj = (const struct TestObj *)data;
    printf("%d", obj->value);
}

/* =========================================================================
 * TEST CASES
 * ========================================================================= */

void test_initialization() {
    printf("Running test_initialization...\n");
    struct dynarray arr;
    assert(dynarray_init(&arr, 4, sizeof(struct TestObj), TestObjConcept) == 0);
    assert(dynarray_size(&arr) == 0);
    assert(dynarray_capacity(&arr) == 4);
    assert(dynarray_empty(&arr) == 1);
    
    dynarray_deinit(&arr);
    assert(g_active_objects == 0); // Verify no leaks
    printf("PASS\n");
}

void test_push_pop() {
    printf("Running test_push_pop...\n");
    struct dynarray arr;
    dynarray_init(&arr, 2, sizeof(struct TestObj), TestObjConcept);

    struct TestObj t1 = {10}, t2 = {20}, t3 = {30};

    // Push Back (trigger growth)
    dynarray_push_back(&arr, &t1);
    dynarray_push_back(&arr, &t2);
    dynarray_push_back(&arr, &t3); // Should trigger realloc (cap 2 -> 4)

    assert(dynarray_size(&arr) == 3);
    assert(dynarray_capacity(&arr) >= 3);
    assert(g_active_objects == 3);

    // Verify Values (Front/Back)
    struct TestObj *front = dynarray_front(&arr);
    struct TestObj *back = dynarray_back(&arr);
    assert(front->value == 10);
    assert(back->value == 30);

    // Pop Back
    dynarray_pop_back(&arr);
    assert(dynarray_size(&arr) == 2);
    assert(g_active_objects == 2); // 1 destroyed
    
    // Check new back
    back = dynarray_back(&arr);
    assert(back->value == 20);

    dynarray_deinit(&arr);
    assert(g_active_objects == 0);
    printf("PASS\n");
}

void test_insert_delete() {
    printf("Running test_insert_delete...\n");
    struct dynarray arr;
    dynarray_init(&arr, 5, sizeof(struct TestObj), TestObjConcept);

    // Setup: [10, 20, 30]
    struct TestObj vals[] = {{10}, {20}, {30}};
    dynarray_push_back(&arr, &vals[0]);
    dynarray_push_back(&arr, &vals[1]);
    dynarray_push_back(&arr, &vals[2]);
    
    printf("Initial state:\n");
    dynarray_print_debug(&arr, print_test_obj);

    // Insert [99, 99] at index 1 -> EXPECTED: [10, 99, 99, 20, 30]
    struct TestObj ins[] = {{99}, {99}};
    void *ins_end = (char*)&ins[0] + 2 * sizeof(struct TestObj); // Manual end pointer
    dynarray_insert(&arr, 1, &ins[0], ins_end); 

    printf("After Insert (Should be [10, 99, 99, 20, 30]):\n");
    dynarray_print_debug(&arr, print_test_obj); // <--- CHECK OUTPUT HERE

    assert(dynarray_size(&arr) == 5);
    struct TestObj check;
    dynarray_get(&arr, 1, &check); assert(check.value == 99);
    dynarray_get(&arr, 3, &check); assert(check.value == 20);

    // Delete indices [1, 3) -> EXPECTED: [10, 20, 30]
    dynarray_delete(&arr, 1, 3);
    
    printf("After Delete (Should be [10, 20, 30]):\n");
    dynarray_print_debug(&arr, print_test_obj); // <--- CHECK OUTPUT HERE

    assert(dynarray_size(&arr) == 3);
    
    // The failing line:
    dynarray_get(&arr, 1, &check); 
    printf("Index 1 contains: %d\n", check.value); 
    assert(check.value == 20);

    dynarray_deinit(&arr);
    printf("PASS\n");
}

/**
 * @brief CRITICAL TEST: Self-Insertion
 * Tests if the array can handle inserting a part of itself into itself,
 * which involves potential pointer invalidation during realloc.
 */
void test_self_insertion() {
    printf("Running test_self_insertion...\n");
    struct dynarray arr;
    dynarray_init(&arr, 2, sizeof(struct TestObj), TestObjConcept); // Small cap to force realloc

    // Setup: [1, 2]
    struct TestObj v1={1}, v2={2};
    dynarray_push_back(&arr, &v1);
    dynarray_push_back(&arr, &v2);

    // Insert entire array [1, 2] at index 1
    // Expected result: [1, 1, 2, 2]
    // This forces:
    // 1. Growth (cap 2 -> 4) (realloc invalidates old pointers!)
    // 2. Logic to handle that 'begin' and 'end' were pointing to old buffer
    void *begin = dynarray_iterator_begin(&arr);
    void *end = dynarray_iterator_end(&arr);
    
    int res = dynarray_insert(&arr, 1, begin, end);
    assert(res == 0);
    dynarray_print_debug(&arr, print_test_obj);

    // Verify Data
    assert(dynarray_size(&arr) == 4);
    int expected[] = {1, 1, 2, 2};
    for(int i=0; i<4; i++) {
        struct TestObj temp;
        dynarray_get(&arr, i, &temp);
        assert(temp.value == expected[i]);
    }
    
    assert(g_active_objects == 4);

    dynarray_deinit(&arr);
    printf("PASS\n");
}

void test_resize_clear() {
    printf("Running test_resize_clear...\n");
    struct dynarray arr;
    dynarray_init(&arr, 4, sizeof(struct TestObj), TestObjConcept);

    struct TestObj def = {777};

    // Resize Up: [777, 777, 777]
    dynarray_resize(&arr, 3, &def);
    assert(dynarray_size(&arr) == 3);
    assert(g_active_objects == 3);
    struct TestObj *last = dynarray_back(&arr);
    assert(last->value == 777);

    // Resize Down: [777]
    dynarray_resize(&arr, 1, NULL);
    assert(dynarray_size(&arr) == 1);
    assert(g_active_objects == 1);

    // Clear
    dynarray_clear(&arr);
    assert(dynarray_size(&arr) == 0);
    assert(dynarray_capacity(&arr) >= 4); // Capacity preserved
    assert(g_active_objects == 0); // All objects destroyed

    dynarray_deinit(&arr);
    printf("PASS\n");
}

void test_set_and_iter() {
    printf("Running test_set_and_iter...\n");
    struct dynarray arr;
    dynarray_init(&arr, 4, sizeof(struct TestObj), TestObjConcept);
    
    struct TestObj v = {0};
    dynarray_resize(&arr, 3, &v); // [0, 0, 0]

    struct TestObj new_val = {100};
    dynarray_set(&arr, 1, &new_val); // [0, 100, 0]

    struct TestObj check;
    dynarray_get(&arr, 1, &check);
    assert(check.value == 100);
    // Ensure old object at index 1 was destroyed and new one created
    // Total should still be 3
    assert(g_active_objects == 3); 

    dynarray_deinit(&arr);
    printf("PASS\n");
}

int main() {
    printf("=== DYNARRAY TEST SUITE ===\n");
    
    test_initialization();
    test_push_pop();
    test_insert_delete();
    test_self_insertion();
    test_resize_clear();
    test_set_and_iter();

    printf("\nAll tests passed successfully.\n");
    return 0;
}