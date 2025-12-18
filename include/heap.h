#ifndef HEAP_H
#define HEAP_H

#include "../../concepts/include/object_concept.h"
#include "../../debug/include/debug.h"
#include "../internals/status.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct heap;
typedef struct heap heap_t;

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

int heap_init(struct heap* tree, size_t capacity, int (*cmp) (const void* a, const void* b));
void heap_deinit(struct heap* tree, void* context, struct object_concept* oc);
size_t heap_sizeof();

/*───────────────────────────────────────────────
 * Operations
 *───────────────────────────────────────────────*/
int heap_add(struct heap* tree, void* new_data);
void* heap_remove(struct heap* tree);

/*───────────────────────────────────────────────
 * Accessors
 *───────────────────────────────────────────────*/

int heap_empty(const struct heap* tree);
size_t heap_size(const struct heap* tree);

/*───────────────────────────────────────────────
 * Iterations
 *───────────────────────────────────────────────*/

void heap_walk(struct heap* tree, void* context, void (*handler) (void* data, void* context));

#ifdef __cplusplus
}
#endif

#endif // HEAP_H