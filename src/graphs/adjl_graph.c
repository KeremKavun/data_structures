#include <ds/graphs/adjl_graph.h>
#include <ds/linkedlists/clist.h>
#include <ds/stack/lstack.h>
#include <ds/queue/lqueue.h>
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
    struct clist        vertices;
    size_t              edge_count;
    int (*cmp) (const void *key, const void *data);
};

static void adjl_vertex_init(struct adjl_vertex *v, void *new_data);
static void set_processeds(struct adjl_graph *gr);

/* =========================================================================
 * Create & Destroy
 * ========================================================================= */

struct adjl_graph *adjl_graph_create(int (*cmp) (const void *key, const void *data))
{
    assert(cmp != NULL);
    struct adjl_graph *gr = malloc(sizeof(struct adjl_graph));
    if (!gr) {
        LOG(LIB_LVL, CERROR, "Could not get a valid memory for the graph");
        return NULL;
    }
    clist_init(&gr->vertices);
    gr->edge_count = 0;
    gr->cmp = cmp;
    return gr;
}

void adjl_graph_destroy(struct adjl_graph *gr, struct object_concept *oc)
{
    assert(gr != NULL);
    while (!clist_empty(&gr->vertices)) {
        struct clist_item *item = clist_pop_front(&gr->vertices);
        struct adjl_vertex *v = clist_entry(item, struct adjl_vertex, hook);
        struct adjl_arc *arc = v->adj_list;
        while (arc) {
            struct adjl_arc *next = arc->next_arc;
            free(arc);
            arc = next;
        }
        if (oc && oc->deinit)
            oc->deinit(v->data);
        free(v);
    }
    free(gr);
}

/* =========================================================================
 * Insertion & Removal
 * ========================================================================= */

int adjl_graph_add_vertex(struct adjl_graph *gr, void *new_data)
{
    assert(gr != NULL);
    assert(new_data != NULL);
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
    gr->edge_count++;
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
            gr->edge_count--;
            free(current);
            return 0;
        }
        indirect = &current->next_arc;
    }
    return 1;
}

/* =========================================================================
 * Vertex API
 * ========================================================================= */

void adjl_vertex_flag(struct adjl_vertex *v, int flag)
{
    assert(v != NULL);
    v->processed = flag;
}

size_t adjl_vertex_indegree(const struct adjl_vertex *v)
{
    assert(v != NULL);
    return v->indegree;
}

size_t adjl_vertex_outdegree(const struct adjl_vertex *v)
{
    assert(v != NULL);
    return v->outdegree;
}

void *adjl_vertex_get_data(struct adjl_vertex *v) {
    assert(v != NULL);
    return v->data;
}

/* =========================================================================
 * Iterators
 * ========================================================================= */

// === Vertex Iterators === //

void adjl_v_iter_init(struct adjl_graph *gr, adjl_vertex_iterator *it)
{
    assert(gr != NULL && it != NULL);
    it->graph = gr;
    it->current = gr->vertices.sentinel.next;
}

void *adjl_v_iter_next(adjl_vertex_iterator *it)
{
    assert(it != NULL);
    if (it->current == &it->graph->vertices.sentinel)
        return NULL;
    struct adjl_vertex *v = clist_entry(it->current, struct adjl_vertex, hook);
    it->current = it->current->next;
    return v->data;
}

// === Neighbour Iterators === //

void adjl_out_iter_init(const struct adjl_vertex *v, adjl_out_iter *it)
{
    assert(v != NULL && it != NULL);
    it->current_arc = v->adj_list;
}

void *adjl_out_iter_next(adjl_out_iter *it)
{
    assert(it != NULL);
    if (it->current_arc == NULL)
        return NULL;
    struct adjl_arc *arc = it->current_arc;
    void *neighbor_data = arc->dest->data;
    it->current_arc = arc->next_arc;
    return neighbor_data;
}

void adjl_in_iter_init(const struct adjl_graph *gr, const struct adjl_vertex *v, adjl_in_iter *it)
{
    assert(gr != NULL && v != NULL && it != NULL);
    it->graph = gr;
    it->target = v;
    it->curr_v_node = gr->vertices.sentinel.next;
    it->curr_arc = NULL;
}

void *adjl_in_iter_next(adjl_in_iter *it)
{
    assert(it != NULL);
    struct clist_item *sentinel = (struct clist_item *) &it->graph->vertices.sentinel;
    while (it->curr_v_node != sentinel) {
        struct adjl_vertex *scanner = clist_entry(it->curr_v_node, struct adjl_vertex, hook);
        if (it->curr_arc == NULL)
            it->curr_arc = scanner->adj_list;
        while (it->curr_arc != NULL) {
            struct adjl_vertex *dest = it->curr_arc->dest;
            it->curr_arc = it->curr_arc->next_arc; 
            if (dest == it->target) {
                if (it->curr_arc == NULL)
                    it->curr_v_node = clist_item_next(it->curr_v_node);
                return scanner->data;
            }
        }
        it->curr_v_node = clist_item_next(it->curr_v_node);
        it->curr_arc = NULL;
    }
    return NULL;
}

/* =========================================================================
 * Retrieval
 * ========================================================================= */

struct adjl_vertex *adjl_graph_search(struct adjl_graph *gr, const void *key)
{
    assert(gr != NULL);
    assert(key != NULL);
    struct adjl_vertex *v = NULL;
    clist_foreach_entry(v, &gr->vertices, hook) {
        if (gr->cmp(key, v->data) == 0)
            return v;
    }
    return NULL;
}

/* =========================================================================
 * Traversal
 * ========================================================================= */

void adjl_graph_bfs(struct adjl_graph *gr, void *start_key, void *context, void (*handler) (void *item, void *context))
{
    assert(gr != NULL);
    assert(handler != NULL);
    if (adjl_graph_empty(gr))
        return;
    // TODO: Use a real pool after updating allocators repository.
    struct syspool item_pool = { lqueue_node_sizeof() };
    struct allocator_concept ac = { .allocator = &item_pool, .alloc = sysalloc, .free = sysfree };
    struct lqueue *lq = lqueue_create(&ac);
    set_processeds(gr);
    struct adjl_vertex *v = adjl_graph_search(gr, start_key);
    if (!v)
        return;
    lenqueue(lq, v); 
    v->processed = 1;
    while (!lqueue_empty(lq)) {
        v = ldequeue(lq);
        handler(v->data, context);
        struct adjl_arc *arc = v->adj_list;
        while (arc) {
            if (arc->dest->processed == 0) {
                arc->dest->processed = 1;
                lenqueue(lq, arc->dest);
            }
            arc = arc->next_arc;
        }
        v->processed = 2;
    }
    lqueue_destroy(lq, NULL);
}

void adjl_graph_dfs(struct adjl_graph *gr, void *start_key, void *context, void (*handler) (void *item, void *context))
{
    assert(gr != NULL);
    assert(handler != NULL);
    if (adjl_graph_empty(gr))
        return;
    // TODO: Use a real pool after updating allocators repository.
    struct syspool item_pool = { lstack_node_sizeof() };
    struct allocator_concept ac = { .allocator = &item_pool, .alloc = sysalloc, .free = sysfree };
    struct lstack *ls = lstack_create(&ac);
    set_processeds(gr);
    struct adjl_vertex *v = adjl_graph_search(gr, start_key);
    if (!v)
        return;
    lpush(ls, v); 
    v->processed = 1;
    while (!lstack_empty(ls)) {
        v = lpop(ls);
        handler(v->data, context);
        struct adjl_arc *arc = v->adj_list;
        while (arc) {
            if (arc->dest->processed == 0) {
                arc->dest->processed = 1;
                lpush(ls, arc->dest);
            }
            arc = arc->next_arc;
        }
        v->processed = 2;
    }
    lstack_destroy(ls, NULL);
}

/* =========================================================================
 * Inspection
 * ========================================================================= */

int adjl_graph_empty(const struct adjl_graph *gr)
{
    assert(gr != NULL);
    return clist_empty(&gr->vertices);
}

size_t adjl_graph_vertex_count(const struct adjl_graph *gr)
{
    assert(gr != NULL);
    return clist_size(&gr->vertices);
}

size_t adjl_graph_edge_count(const struct adjl_graph *gr)
{
    assert(gr != NULL);
    return gr->edge_count;
}

// *** Helper functions *** //

static void adjl_vertex_init(struct adjl_vertex *v, void *new_data)
{
    v->data = new_data;
    v->indegree = 0;
    v->outdegree = 0;
    v->adj_list = NULL;
    // we do not set processed bool, since it is set whenever we need
    // in search functions
    clist_item_init(&v->hook);
}

static void set_processeds(struct adjl_graph *gr)
{
    assert(gr != NULL);
    struct clist_item *curr;
    clist_foreach_cw(curr, &gr->vertices) {
        struct adjl_vertex *v = clist_entry(curr, struct adjl_vertex, hook);
        v->processed = 0;
    }
}