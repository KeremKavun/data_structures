#include "../include/adjlist.h"
#include "../../linkedlists/include/clist.h"
#include <stdlib.h>

struct adjl_vertex {
    void                    *data;
    size_t                  indegree;
    size_t                  outdegree;
    int                     processed;
    struct adjl_arc         *adj_list;
    struct clist_item       hook;
};

struct adjl_arc {
    struct adjl_vertex      *dest;
    struct adjl_arc         *next_arc;
};

struct adjl_graph {
    struct clist vertices;
    int (*cmp) (const void *key, const void *data);
};

static void adjl_vertex_init(struct adjl_vertex *v, void *new_data);

struct adjl_graph *adjl_graph_create(int (*cmp) (const void *key, const void *data))
{
    assert(cmp != NULL);
    struct adjl_graph *gr = malloc(sizeof(struct adjl_graph));
    if (!gr) {
        LOG(LIB_LVL, CERROR, "Could not get a valid memory for the graph");
        return NULL;
    }
    clist_init(&gr->vertices);
    gr->cmp = cmp;
    return gr;
}

void adjl_graph_destroy(struct adjl_graph *gr, struct object_concept *oc)
{

}

int adjl_graph_add_vertex(struct adjl_graph *gr, void *new_data)
{
    assert(gr != NULL);
    struct adjl_vertex *new_vertex = malloc(sizeof(struct adjl_vertex));
    if (!new_vertex) {
        LOG(LIB_LVL, CERROR, "Could not get a valid memory for new vertex");
        return 1;
    }
    adjl_vertex_init(new_vertex, new_data);
    clist_push_back(&gr->vertices, &new_vertex->hook);
    return 0;
}

void *adjl_graph_remove_vertex(struct adjl_graph *gr, struct adjl_vertex *v)
{
    assert(gr != NULL);
    assert(v != NULL);
    if (v->indegree != 0 || v->outdegree != 0) {
        LOG(LIB_LVL, CERROR, "Vertex is still connected (degrees not zero)");
        return NULL;
    }
    clist_remove(&gr->vertices, &v->hook);
    void *data = v->data;
    free(v);
    return data;
}

int adjl_graph_add_arc(struct adjl_graph *gr, struct adjl_vertex *v1, struct adjl_vertex *v2)
{
    assert(gr != NULL);
    assert(v1 != NULL && v2 != NULL);
    struct adjl_arc *new_arc = malloc(sizeof(struct adjl_arc));
    if (!new_arc) {
        LOG(LIB_LVL, CERROR, "Could not get a valid memory for new arc");
        return 1;
    }
    new_arc->dest = v2;
    new_arc->next_arc = v1->adj_list;
    v1->adj_list = new_arc;
    v1->outdegree++;
    v2->indegree++;
    return 0;
}

int adjl_graph_remove_arc(struct adjl_graph *gr, struct adjl_vertex *src, struct adjl_vertex *dest)
{
    assert(gr != NULL);
    assert(src != NULL && dest != NULL);
    struct adjl_arc **indirect = &src->adj_list;
    while (*indirect != NULL) {
        struct adjl_arc *current = *indirect;
        if (current->dest == dest) {
            *indirect = current->next_arc;
            src->outdegree--;
            dest->indegree--;
            free(current);
            return 0;
        }
        indirect = &current->next_arc;
    }
    return 1;
}

size_t adjl_graph_edge_count(struct adjl_graph *gr)
{
    
}

// *** Helper functions *** //

static void adjl_vertex_init(struct adjl_vertex *v, void *new_data)
{
    v->data = new_data;
    v->indegree = 0;
    v->outdegree = 0;
    v->processed = 0;
    v->adj_list = NULL;
    clist_item_init(&v->hook);
}