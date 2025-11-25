#ifndef HEAP_H
#define HEAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../debug/include/debug.h"
#include "../internals/status.h"
#include "../../buffers/internal/buffers_common.h"
#include <stddef.h>

struct heap;
typedef struct heap heap_t;

/*───────────────────────────────────────────────
 * Lifecycle
 *───────────────────────────────────────────────*/

int heap_init(struct heap* tree, char* stack_ptr, size_t capacity, int resize, int (*cmp) (const void* a, const void* b));
void heap_deinit(struct heap* tree, void* context, void (*deallocator) (void* item, void* context));
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

void heap_walk(struct heap* tree, void* userdata, void (*handler) (void* data, void* userdata));

#ifdef __cplusplus
}
#endif

#endif // HEAP_H