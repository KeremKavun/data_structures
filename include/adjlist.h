#ifndef GRAPHS_ADJLIST_H
#define GRAPHS_ADJLIST_H

#include "../../concepts/include/object_concept.h"
#include "../../concepts/include/allocator_concept.h"
#include "../../linkedlists/include/clist.h"
#include <stddef.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

struct adjl_vertex;
struct adjl_arc;
struct adjl_graph;

/**
 * @defgroup Graph with adjacency list.
 * 
 * @brief Graph ADT.
 * * ### Global Constraints
 * - **NULL Pointers**: All `struct adjlgraph *adjlg` must be non-NULL nor invalid. Methods of `struct object_concept`
 * - must be given and implemented.
 * - **Ownership**: Internal nodes are owned by the underlying list and managed by allocator_concept given by user,
 * - void *references to data are entirely owned by user. adjlist_deinit might be helpful to destruct remaining
 * - objects in the stack.
 * @{
 */

struct adjl_graph *adjl_graph_create(int (*cmp) (const void *key, const void *data));
void adjl_graph_destroy(struct adjl_graph *gr, struct object_concept *oc);

int adjl_graph_add_vertex(struct adjl_graph *gr, void *new_data);
void *adjl_graph_remove_vertex(struct adjl_graph *gr, struct adjl_vertex *v);
int adjl_graph_add_arc(struct adjl_graph *gr, struct adjl_vertex *v1, struct adjl_vertex *v2);

static inline int adjl_graph_empty(struct adjl_graph *gr)
{
    assert(gr != NULL);
    return clist_empty(&gr->vertices);
}

static inline size_t adjl_graph_size(struct adjl_graph *gr)
{
    assert(gr != NULL);
    return clist_size(&gr->vertices);
}

static inline size_t adjl_graph_vertex_count(struct adjl_graph *gr)
{
    assert(gr != NULL);
    return clist_size(&gr->vertices);
}

size_t adjl_graph_edge_count(struct adjl_graph *gr);

/** @} */ // End of Global group

#ifdef __cplusplus
}
#endif

#endif // GRAPHS_ADJLIST_H