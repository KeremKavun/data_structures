#ifndef STACK_BASIC_STACK_H
#define STACK_BASIC_STACK_H

#include <stddef.h>
#include <assert.h>
#include <ds/utils/macros.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file basic_stack.h
 * @brief Defines the interface for very simple array with zero abstraction.
 */

/**
 * @defgroup BASICSTACK Basic Stack (with macros)
 * @ingroup STACK
 * @brief Lightweight, zero-allocation stacks for local scope, using c99 VLA.
 * @{
 */

/**
 * @def basic_stack(name, obj_type, cap_count)
 * @brief Creates a generic, strongly-typed stack on the local execution stack.
 * This macro declares a Variable Length Array (VLA) to hold the data and an 
 * anonymous struct to manage the state. It involves zero heap allocation 
 * and provides zero-abstraction access to the underlying array.
 * @param name      The variable name for the stack instance.
 * @param obj_type  The data type of the elements (e.g., int, Node*, struct Foo).
 * @param cap_count The maximum capacity of the stack (size_t).
 * @note This uses a VLA. Ensure cap_count is reasonable to avoid stack overflow.
 * @warning Do not pass the resulting struct by value if the compiler does not 
 * support copying structs containing pointers to local VLAs reliably.
 */
#define basic_stack(name, obj_type, cap_count)          \
    obj_type UNIQUE_NAME(tbuf_)[cap_count];             \
    struct {                                            \
        size_t size;                                    \
        size_t capacity;                                \
        obj_type *stack;                                \
    } name = {                                          \
        .size = 0,                                      \
        .capacity = cap_count,                          \
        .stack = UNIQUE_NAME(tbuf_)                     \
    }

/**
 * @def basic_stack_push(name, new_obj)
 * @brief Pushes an element onto the stack.
 * @param name    The name of the stack instance.
 * @param new_obj The value to push. Must match obj_type.
 * @note Triggers an assertion failure if the stack is full.
 */
#define basic_stack_push(name, new_obj)         \
    do {                                        \
        assert((name).size < (name).capacity);  \
        (name).stack[(name).size++] = (new_obj);\
    } while (0)

/**
 * @def basic_stack_pop(name, popped_obj)
 * @brief Pops an element from the stack into a destination variable.
 * @param name       The name of the stack instance.
 * @param popped_obj The variable where the popped value will be stored.
 * @note Triggers an assertion failure if the stack is empty.
 */
#define basic_stack_pop(name, popped_obj)       \
    do {                                        \
        assert(0 < (name).size);                \
        (popped_obj) = (name).stack[--(name).size]; \
    } while (0)

/**
 * @def basic_stack_top(name, top_obj)
 * @brief Peeks at the top element without removing it.
 * @param name    The name of the stack instance.
 * @param top_obj The variable where the top value will be stored.
 * @note Triggers an assertion failure if the stack is empty.
 */
#define basic_stack_top(name, top_obj)          \
    do {                                        \
        assert((name).size != 0);               \
        (top_obj) = (name).stack[(name).size - 1]; \
    } while (0)

#ifdef __cplusplus
}
#endif

/** @} */

#endif // STACK_BASIC_STACK_H