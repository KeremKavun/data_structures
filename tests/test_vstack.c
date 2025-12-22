#include "../include/vstack.h"
#include "../../arrays/include/dynarray.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Test counter for tracking test results */
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_ASSERT(condition, msg) do { \
    if (condition) { \
        printf("✓ PASS: %s\n", msg); \
        tests_passed++; \
    } else { \
        printf("✗ FAIL: %s\n", msg); \
        tests_failed++; \
    } \
} while(0)

/* ========== Test Data Structures ========== */

/* Simple integer wrapper for testing */
typedef struct {
    int value;
} IntData;

/* String wrapper for testing object concept */
typedef struct {
    char *str;
} StringData;

/* ========== Object Concepts ========== */

/* Integer object concept */
static int int_init(void *dest, void *src) {
    IntData *d = (IntData *)dest;
    const IntData *s = (const IntData *)src;
    d->value = s->value;
    return 0;
}

static void int_deinit(void *obj) {
    /* Nothing to do for simple int */
    (void)obj;
}

static struct object_concept int_oc = {
    .init = int_init,
    .deinit = int_deinit
};

/* String object concept */
static int string_init(void *dest, void *src) {
    StringData *d = (StringData *)dest;
    const StringData *s = (const StringData *)src;
    
    if (s->str == NULL) {
        d->str = NULL;
        return 0;
    }
    
    d->str = (char *)malloc(strlen(s->str) + 1);
    if (d->str == NULL) {
        return -1;
    }
    strcpy(d->str, s->str);
    return 0;
}

static void string_deinit(void *obj) {
    StringData *s = (StringData *)obj;
    if (s->str != NULL) {
        free(s->str);
        s->str = NULL;
    }
}

static struct object_concept string_oc = {
    .init = string_init,
    .deinit = string_deinit
};

/* ========== Test Functions ========== */

void test_vstack_init_deinit(void) {
    printf("\n=== Testing vstack_init and vstack_deinit ===\n");
    
    struct vstack vs;
    int result;
    
    /* Test initialization */
    result = vstack_init(&vs, sizeof(IntData), &int_oc);
    TEST_ASSERT(result == 0, "vstack_init returns 0");
    TEST_ASSERT(vs.contents != NULL, "vstack contents initialized");
    TEST_ASSERT(vstack_empty(&vs) == 1, "New stack is empty");
    TEST_ASSERT(vstack_size(&vs) == 0, "New stack has size 0");
    
    /* Test deinitialization */
    vstack_deinit(&vs);
    printf("✓ PASS: vstack_deinit completes without error\n");
    tests_passed++;
}

void test_vpush_basic(void) {
    printf("\n=== Testing vpush (basic operations) ===\n");
    
    struct vstack vs;
    vstack_init(&vs, sizeof(IntData), &int_oc);
    
    IntData data1 = {42};
    IntData data2 = {100};
    IntData data3 = {-5};
    
    /* Push first item */
    int result = vpush(&vs, &data1);
    TEST_ASSERT(result == 0, "vpush returns 0 on success");
    TEST_ASSERT(vstack_empty(&vs) == 0, "Stack not empty after push");
    TEST_ASSERT(vstack_size(&vs) == 1, "Stack size is 1 after first push");
    
    /* Push second item */
    result = vpush(&vs, &data2);
    TEST_ASSERT(result == 0, "vpush second item returns 0");
    TEST_ASSERT(vstack_size(&vs) == 2, "Stack size is 2 after second push");
    
    /* Push third item */
    result = vpush(&vs, &data3);
    TEST_ASSERT(result == 0, "vpush third item returns 0");
    TEST_ASSERT(vstack_size(&vs) == 3, "Stack size is 3 after third push");
    
    vstack_deinit(&vs);
}

void test_vpop_basic(void) {
    printf("\n=== Testing vpop (basic operations) ===\n");
    
    struct vstack vs;
    vstack_init(&vs, sizeof(IntData), &int_oc);
    
    IntData data1 = {10};
    IntData data2 = {20};
    IntData data3 = {30};
    IntData popped;
    
    /* Setup: push three items */
    vpush(&vs, &data1);
    vpush(&vs, &data2);
    vpush(&vs, &data3);
    
    /* Pop first item (should be 30) */
    int result = vpop(&vs, &popped);
    TEST_ASSERT(result == 0, "vpop returns 0 on success");
    TEST_ASSERT(popped.value == 30, "Popped value is 30 (LIFO)");
    TEST_ASSERT(vstack_size(&vs) == 2, "Stack size is 2 after first pop");
    
    /* Pop second item (should be 20) */
    result = vpop(&vs, &popped);
    TEST_ASSERT(result == 0, "vpop second item returns 0");
    TEST_ASSERT(popped.value == 20, "Popped value is 20");
    TEST_ASSERT(vstack_size(&vs) == 1, "Stack size is 1 after second pop");
    
    /* Pop third item (should be 10) */
    result = vpop(&vs, &popped);
    TEST_ASSERT(result == 0, "vpop third item returns 0");
    TEST_ASSERT(popped.value == 10, "Popped value is 10");
    TEST_ASSERT(vstack_empty(&vs) == 1, "Stack is empty after popping all");
    
    vstack_deinit(&vs);
}

void test_vpop_null_output(void) {
    printf("\n=== Testing vpop with NULL output ===\n");
    
    struct vstack vs;
    vstack_init(&vs, sizeof(IntData), &int_oc);
    
    IntData data = {999};
    vpush(&vs, &data);
    
    /* Pop without storing result */
    int result = vpop(&vs, NULL);
    TEST_ASSERT(result == 0, "vpop with NULL output returns 0");
    TEST_ASSERT(vstack_empty(&vs) == 1, "Stack is empty after pop");
    
    vstack_deinit(&vs);
}

void test_vpop_empty_stack(void) {
    printf("\n=== Testing vpop on empty stack ===\n");
    
    struct vstack vs;
    vstack_init(&vs, sizeof(IntData), &int_oc);
    
    IntData popped = {123};
    int result = vpop(&vs, &popped);
    
    TEST_ASSERT(result != 0, "vpop on empty stack returns non-zero");
    
    vstack_deinit(&vs);
}

void test_vtop(void) {
    printf("\n=== Testing vtop ===\n");
    
    struct vstack vs;
    vstack_init(&vs, sizeof(IntData), &int_oc);
    
    IntData data1 = {100};
    IntData data2 = {200};
    IntData top;
    
    /* Push items */
    vpush(&vs, &data1);
    vpush(&vs, &data2);
    
    /* Get top without removing */
    int result = vtop(&vs, &top);
    TEST_ASSERT(result == 0, "vtop returns 0 on success");
    TEST_ASSERT(top.value == 200, "vtop returns correct value (200)");
    TEST_ASSERT(vstack_size(&vs) == 2, "vtop does not modify stack size");
    
    /* Verify top again to ensure non-destructive */
    result = vtop(&vs, &top);
    TEST_ASSERT(result == 0, "vtop can be called multiple times");
    TEST_ASSERT(top.value == 200, "vtop still returns 200");
    
    vstack_deinit(&vs);
}

void test_vstack_empty(void) {
    printf("\n=== Testing vstack_empty ===\n");
    
    struct vstack vs;
    vstack_init(&vs, sizeof(IntData), &int_oc);
    
    TEST_ASSERT(vstack_empty(&vs) == 1, "Empty stack returns 1");
    
    IntData data = {42};
    vpush(&vs, &data);
    TEST_ASSERT(vstack_empty(&vs) == 0, "Non-empty stack returns 0");
    
    IntData popped;
    vpop(&vs, &popped);
    TEST_ASSERT(vstack_empty(&vs) == 1, "Stack empty again after pop");
    
    vstack_deinit(&vs);
}

void test_vstack_size(void) {
    printf("\n=== Testing vstack_size ===\n");
    
    struct vstack vs;
    vstack_init(&vs, sizeof(IntData), &int_oc);
    
    TEST_ASSERT(vstack_size(&vs) == 0, "Initial size is 0");
    
    IntData data = {1};
    for (int i = 1; i <= 10; i++) {
        data.value = i;
        vpush(&vs, &data);
        TEST_ASSERT(vstack_size(&vs) == (size_t)i, "Size increases correctly");
    }
    
    IntData popped;
    for (int i = 9; i >= 0; i--) {
        vpop(&vs, &popped);
        TEST_ASSERT(vstack_size(&vs) == (size_t)i, "Size decreases correctly");
    }
    
    vstack_deinit(&vs);
}

/* Walk context */
typedef struct {
    int sum;
    int count;
} WalkContext;

/* Handler function */
void sum_handler(void *item, void *context) {
    IntData *d = (IntData *)item;
    WalkContext *c = (WalkContext *)context;
    c->sum += d->value;
    c->count++;
}

void test_vstack_walk(void) {
    printf("\n=== Testing vstack_walk ===\n");
    
    struct vstack vs;
    vstack_init(&vs, sizeof(IntData), &int_oc);
    
    /* Push test data */
    for (int i = 1; i <= 5; i++) {
        IntData data = {i * 10};
        vpush(&vs, &data);
    }
    
    WalkContext ctx = {0, 0};
    
    vstack_walk(&vs, &ctx, sum_handler);
    
    TEST_ASSERT(ctx.count == 5, "Walk handler called for each item");
    TEST_ASSERT(ctx.sum == 150, "Walk correctly processes all items (10+20+30+40+50)");
    
    vstack_deinit(&vs);
}

void test_string_stack(void) {
    printf("\n=== Testing stack with complex objects (strings) ===\n");
    
    struct vstack vs;
    vstack_init(&vs, sizeof(StringData), &string_oc);
    
    StringData str1 = {(char *)"Hello"};
    StringData str2 = {(char *)"World"};
    StringData str3 = {(char *)"Stack"};
    
    /* Push strings */
    int result = vpush(&vs, &str1);
    TEST_ASSERT(result == 0, "Push first string successful");
    
    result = vpush(&vs, &str2);
    TEST_ASSERT(result == 0, "Push second string successful");
    
    result = vpush(&vs, &str3);
    TEST_ASSERT(result == 0, "Push third string successful");
    
    TEST_ASSERT(vstack_size(&vs) == 3, "Stack has 3 strings");
    
    /* Pop and verify */
    StringData popped;
    result = vpop(&vs, &popped);
    TEST_ASSERT(result == 0, "Pop string successful");
    TEST_ASSERT(strcmp(popped.str, "Stack") == 0, "Popped correct string");
    string_deinit(&popped);
    
    result = vpop(&vs, &popped);
    TEST_ASSERT(strcmp(popped.str, "World") == 0, "Popped second string");
    string_deinit(&popped);
    
    result = vpop(&vs, &popped);
    TEST_ASSERT(strcmp(popped.str, "Hello") == 0, "Popped third string");
    string_deinit(&popped);
    
    vstack_deinit(&vs);
}

void test_stress_push_pop(void) {
    printf("\n=== Stress testing push/pop operations ===\n");
    
    struct vstack vs;
    vstack_init(&vs, sizeof(IntData), &int_oc);
    
    const int COUNT = 50;
    
    /* Push many items */
    for (int i = 0; i < COUNT; i++) {
        IntData data = {i};
        int result = vpush(&vs, &data);
        TEST_ASSERT(result == 0, "Stress push successful");
    }
    
    TEST_ASSERT(vstack_size(&vs) == (size_t)COUNT, "Stress test: correct size after pushes");
    
    /* Pop and verify LIFO order */
    int all_correct = 1;
    for (int i = COUNT - 1; i >= 0; i--) {
        IntData popped;
        vpop(&vs, &popped);
        if (popped.value != i) {
            all_correct = 0;
            break;
        }
    }
    
    TEST_ASSERT(all_correct == 1, "Stress test: all items popped in correct LIFO order");
    TEST_ASSERT(vstack_empty(&vs) == 1, "Stress test: stack empty after all pops");
    
    vstack_deinit(&vs);
}

void test_interleaved_operations(void) {
    printf("\n=== Testing interleaved push/pop/top operations ===\n");
    
    struct vstack vs;
    vstack_init(&vs, sizeof(IntData), &int_oc);
    
    IntData data, result_data;
    
    /* Complex sequence of operations */
    data.value = 1;
    vpush(&vs, &data);
    
    data.value = 2;
    vpush(&vs, &data);
    
    vtop(&vs, &result_data);
    TEST_ASSERT(result_data.value == 2, "Top is 2");
    
    vpop(&vs, &result_data);
    TEST_ASSERT(result_data.value == 2, "Popped 2");
    
    data.value = 3;
    vpush(&vs, &data);
    
    data.value = 4;
    vpush(&vs, &data);
    
    vtop(&vs, &result_data);
    TEST_ASSERT(result_data.value == 4, "Top is 4");
    
    TEST_ASSERT(vstack_size(&vs) == 3, "Size is 3 (1, 3, 4)");
    
    vpop(&vs, &result_data);
    TEST_ASSERT(result_data.value == 4, "Popped 4");
    
    vpop(&vs, &result_data);
    TEST_ASSERT(result_data.value == 3, "Popped 3");
    
    vpop(&vs, &result_data);
    TEST_ASSERT(result_data.value == 1, "Popped 1");
    
    TEST_ASSERT(vstack_empty(&vs) == 1, "Stack is empty");
    
    vstack_deinit(&vs);
}

/* ========== Main Test Runner ========== */

int main(void) {
    printf("╔════════════════════════════════════════╗\n");
    printf("║   VSTACK API COMPREHENSIVE TEST SUITE  ║\n");
    printf("╚════════════════════════════════════════╝\n");
    
    test_vstack_init_deinit();
    test_vpush_basic();
    test_vpop_basic();
    test_vpop_null_output();
    test_vpop_empty_stack();
    test_vtop();
    test_vstack_empty();
    test_vstack_size();
    test_vstack_walk();
    test_string_stack();
    test_stress_push_pop();
    test_interleaved_operations();
    
    printf("\n╔════════════════════════════════════════╗\n");
    printf("║          TEST SUMMARY                  ║\n");
    printf("╠════════════════════════════════════════╣\n");
    printf("║  Tests Passed: %-4d                   ║\n", tests_passed);
    printf("║  Tests Failed: %-4d                   ║\n", tests_failed);
    printf("╚════════════════════════════════════════╝\n");
    
    if (tests_failed == 0) {
        printf("\n🎉 All tests passed! 🎉\n");
        return 0;
    } else {
        printf("\n❌ Some tests failed. Please review the output above.\n");
        return 1;
    }
}