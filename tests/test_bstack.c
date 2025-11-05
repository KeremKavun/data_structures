#include "../include/bstack.h"
#include <stdio.h>
#include <stdlib.h>

// Example copy and init handlers
static int int_copy(const void* new_item, void* buffer_item, void* userdata)
{
    (void)userdata;
    *(int*)buffer_item = *(const int*)new_item;
    return 0;
}

static int int_init(void* buffer_item, void* userdata)
{
    static int counter = 100;
    *(int*)buffer_item = counter++;
    (void)userdata;
    return 0;
}

static void print_item(void* item, void* userdata)
{
    (void)userdata;
    printf("%d ", *(int*)item);
}

static void free_item(void* item, void* userdata)
{
    (void)userdata;
    printf("(Freeing %d) ", *(int*)item);
}

int main(void)
{
    struct bstack stack;

    if (bstack_init(&stack, sizeof(int)) != 0)
    {
        fprintf(stderr, "Failed to initialize stack\n");
        return 1;
    }

    printf("Initialized stack (capacity=%zu)\n", bstack_capacity(&stack));

    // Push some items
    for (int i = 0; i < 5; ++i)
    {
        if (bpush(&stack, &i, NULL, int_copy) != 0)
        {
            fprintf(stderr, "Push failed for %d\n", i);
            return 1;
        }
    }
    printf("After push 5: size=%zu\n", bstack_size(&stack));

    // Check top element
    int top_val;
    btop(&stack, &top_val);
    printf("Top: %d\n", top_val);

    // Pop two elements
    for (int i = 0; i < 2; ++i)
    {
        int popped;
        if (bpop(&stack, &popped) == 0)
            printf("Popped: %d\n", popped);
    }
    printf("After pop 2: size=%zu\n", bstack_size(&stack));

    // Emplace push 3 items
    for (int i = 0; i < 3; ++i)
    {
        emplace_bpush(&stack, NULL, int_init);
    }
    printf("After emplace push 3: size=%zu\n", bstack_size(&stack));

    // Walk stack from top to bottom
    printf("Stack (top→bottom): ");
    bstack_walk(&stack, NULL, print_item);
    printf("\n");

    // Empty the stack completely
    printf("Popping remaining:\n");
    while (!bstack_empty(&stack))
    {
        int popped;
        bpop(&stack, &popped);
        printf("Popped: %d\n", popped);
    }

    printf("Stack empty? %s\n", bstack_empty(&stack) ? "yes" : "no");

    // Free stack (also triggers deallocator print)
    printf("Freeing stack:\n");
    bstack_free(&stack, NULL, free_item);
    printf("\nStack freed.\n");

    return 0;
}