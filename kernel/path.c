/* Unit task execution and general task functions.
   Copyright (C) 1992-2000 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/*** DISABLED FOR NOW ***/
#if (0)

#include "conq.h"
#include "kernel.h"

/*
 * Implement a pathfinding algorithm.
 * The Astar ("A*") algorithm is implemented.
 *
 * Optimisations include hashing on open and closed lists,
 * and also optimised dynamic memory allocation.
 *
 * This file attempts to implement only the algorithm, with
 * as much disconnection with xconq as possible.
 * It finds a path through space of some sort.
 * The paths are cached, using a pointer as a key.
 */

#define Dim(x) (sizeof(x)/sizeof(x[0]))

struct node {
    struct node *parent;
    struct node *next;
    struct node *next_hash;
    int g;			/* known cost to get to this point */
    int h;			/* estimated cost to goal. */
    int dir_from_parent;
    struct path_node_data node_data;
};

/* size of hashing arrays. */
#define NL_HASHSIZE 991
#define PL_HASHSIZE 991
/* count between cache garbage collection. */
#define TURNOUT 10
/* size of allocated array for memory allocator. */
#define NODE_ALLOC_COUNT 1024
typedef struct node node_array[NODE_ALLOC_COUNT];

struct nodelist {
    int node_count;
    struct node *hash_table[NL_HASHSIZE];
    struct node *nodes;
};

static struct node *nodes;
static int node_count;
static int node_index;
static struct node *free_nodes;

static int nodes_allocated;
static int nodes_freed;
static int nodes_in_use;

struct path {
    struct node *start_node;
    struct node *last_node;
    struct path_node_data goal_data;
    void *pkey;
    int flags;
    int last_access;
    struct path *next;
};

static struct path *pathhash[PL_HASHSIZE];
static int last_clean_turn;

static struct nodelist open_list;
static struct nodelist closed_list;
static int total_node_memory;

/* Get a free node.
 * The memory allocation grabs and keeps large blocks of memory.
 * There seems little point returning it, as it will only be wanted
 * again later.
 */
static struct node *
get_free_node(void)
{
    struct node *n;
    if (free_nodes) {
	n = free_nodes;
	free_nodes = n->next;
	memset(n, 0, sizeof(*n));
	nodes_freed--;
	assert(nodes_freed >= 0);
    } else {
	if (node_index >= node_count) {
	    node_count = NODE_ALLOC_COUNT;
	    nodes = (struct node *) xmalloc(sizeof(node_array));
	    assert(nodes);
	    node_index = 0;
	    total_node_memory += sizeof(node_array);
	    if (0)
		printf("total node memory %d bytes\n", total_node_memory);
	}
	n = nodes + node_index++;
	nodes_allocated++;
    }
    return n;
}

static void
free_node(struct node *n)
{
    n->next = free_nodes;
    free_nodes = n;
    nodes_freed++;
}

static void
nodelist_clear(struct nodelist *nlist)
{
    memset(nlist->hash_table, 0, sizeof(nlist->hash_table));
    while (nlist->nodes) {
	struct node *n = nlist->nodes;
	nlist->nodes = n->next;
	free_node(n);
    }
    nlist->nodes = 0;
    nlist->node_count = 0;
}

/* separate little function here so debugger can trap here. */
static int
nodelist_verify_fails(void)
{
    return 0;
}
static int
nodelist_verify(struct nodelist *nlist)
{
    int nodecnt = 0;
    int hashcnt = 0;
    struct node *n;
    int i;
    for (n = nlist->nodes; n != 0; n = n->next) {
	if (++nodecnt > nlist->node_count)
	    break;
    }
    if (nodecnt != nlist->node_count)
	return nodelist_verify_fails();

    for (i = 0; i < Dim(nlist->hash_table); i++) {
	if (!nlist->hash_table[i])
	    continue;
	for (n = nlist->hash_table[i]; n != 0; n = n->next_hash) {
	    if (++hashcnt > nlist->node_count)
		break;
	}
    }
    if (hashcnt != nlist->node_count)
	return nodelist_verify_fails();

    return 1;
}
static void
nodelist_add(struct nodelist *nlist, struct node *n)
{
    int f = n->g + n->h;
    unsigned int hindex;
    struct node **pn;
    struct path_node_data *pnd = &n->node_data;

    /* assert(nodelist_verify(nlist)); */

    /* add to priority queue. */
    for (pn = &nlist->nodes; *pn != 0; pn = &((*pn)->next)) {
	struct node *n1 = *pn;

	if (f < n1->g + n1->h) {
	    n->next = n1;
	    *pn = n;
	    break;
	}
    }
    if (!*pn)
	*pn = n;
    /* add to hash tables. */
    hindex = path_node_data_index(pnd, Dim(nlist->hash_table));
    n->next_hash = nlist->hash_table[hindex];
    nlist->hash_table[hindex] = n;
    nlist->node_count++;

    /* assert(nodelist_verify(nlist)); */
}
static int
nodelist_count(struct nodelist *nlist)
{
    return nlist->node_count;
}
static int
nodelist_remove(struct nodelist *nlist, struct node *n)
{
    struct node **pn;
    struct path_node_data *pnd = &n->node_data;
    int hindex;
    int found1 = 0;
    int found2 = 0;
    if (!n)
	return 0;
    /* remove from hash table */
    hindex = path_node_data_index(pnd, Dim(nlist->hash_table));
    for (pn = &(nlist->hash_table[hindex]); *pn != 0;
	 pn = &((*pn)->next_hash)) {
	if (*pn == n) {
	    *pn = n->next_hash;
	    n->next_hash = 0;
	    found1++;
	    break;
	}
    }
    /* remove from priority queue. */
    for (pn = &nlist->nodes; *pn != 0; pn = &((*pn)->next)) {
	if (*pn == n) {
	    *pn = n->next;
	    n->next = 0;
	    found2++;
	    break;
	}
    }
    assert(found1 == found2);
    if (found1 == 1)
	nlist->node_count--;
    return found1;
}

/*
 * Get the best openlist node to test for new exit paths.
 * But if the next two are identical, invoke a preference function,
 * so that all things being equal, the next best is obtained.
 */
static struct node *
nodelist_pop_best(struct nodelist *nlist, void * pkey, int flags)
{
    struct node *n = nlist->nodes;

    if(n && n->next && (n->g + n->h) == (n->next->g + n->next->h)
        && select_better_path_node(pkey, &n->node_data,&n->next->node_data, flags))
	n = n->next;

    if (n)
	nodelist_remove(nlist, n);
    return n;
}
static struct node *
nodelist_find(struct nodelist *nlist, struct path_node_data *p)
{
    struct node *n;
    int hindex;
    hindex = path_node_data_index(p, Dim(nlist->hash_table));
    for (n = nlist->hash_table[hindex]; n != 0; n = n->next_hash) {
	if (same_path_nodes(&n->node_data, p))
	    return n;
    }
    return 0;
}



/* return a reference to the path. */
static struct path **
path_locate(void *pkey)
{
    unsigned index = ((unsigned long) (pkey)) % PL_HASHSIZE;
    struct path **p;
    for (p = &pathhash[index]; *p != 0; p = &((*p)->next)) {
	if ((*p)->pkey == pkey)
	    return p;
    }
    return 0;
}

static void
remove_path(struct path **pp)
{
    struct path *p = *pp;
    struct node *n, *nn;
    *pp = p->next;
    for (n = p->start_node; n != 0; n = nn) {
	nn = n->next;
	free_node(n);
	nodes_in_use--;
    }
    free(p);
}

static void
cache_cleanout(void)
{
    struct path **p, **pnext;
    int i;
    int n = g_turn();
    if (n - last_clean_turn < TURNOUT)
	return;
    last_clean_turn = n;
    for (i = 0; i < PL_HASHSIZE; i++) {
	if (pathhash[i] == 0)
	    continue;
	for (p = &pathhash[i]; *p != 0; p = pnext) {
	    if (n - (*p)->last_access > TURNOUT) {
		remove_path(p);
		pnext = p;
	    } else
		pnext = &((*p)->next);
	}
    }
}

static void
add_path_to_cache(struct path *p)
{
    struct path **pp;
    struct node *n;
    unsigned index = ((unsigned long) (p->pkey)) % PL_HASHSIZE;
    if ((pp = path_locate(p->pkey)) != 0)
	remove_path(pp);
    p->next = pathhash[index];
    pathhash[index] = p;
    for (n = p->start_node; n != 0; n = n->next)
	nodes_in_use++;
}

static int
get_dir_from_cache(void *pkey, struct path_node_data *pn_goal, int flags,
		   int *pdir)
{
    struct path **pp, *p;
    struct node *n, *n0, *n1;
    struct path_node_data node_data;

    if ((pp = path_locate(pkey)) == 0)
	return 0;
    p = *pp;
    if (flags != p->flags || !same_path_nodes(pn_goal, &p->goal_data)) {
	remove_path(pp);
	return 0;
    }
    if (!p->last_node) {
	*pdir = -1;
	p->last_access = g_turn();
	return 1;		/* nowhere to go. */
    }
    set_path_node_from_key(pkey, &node_data, flags);
    for (n = p->start_node; n != 0 && n != p->last_node; n = n->next) {
	if (same_path_nodes(&n->node_data, &node_data))
	    break;
    }
    if (!n || !n->next) {
	/* the start point is not on the path.
	 * or currently at goal.
	 * delete from cache and return.
	 */
	remove_path(pp);
	return 0;
    }

    /* Remove all points up to the current start point,
     * as a form of optimization.
     */
    for (n0 = p->start_node; n0 != n; n0 = n1) {
	n1 = n0->next;
	free_node(n0);
	nodes_in_use--;
    }
    p->start_node = n;

    *pdir = n->next->dir_from_parent;
    p->last_access = g_turn();
    return 1;
}

int
path_get_next_cached_move(void *pkey, struct path_node_data *pn_goal)
{
    struct path **pp, *p;
    struct path_node_data node_data;
    
    assert(nodes_freed + nodes_in_use == nodes_allocated);
    if ((pp = path_locate(pkey)) == 0)
	return -1;
    p = *pp;

    if (!same_path_nodes(pn_goal, &p->goal_data))
	return -1;

    set_path_node_from_key(pkey, &node_data, p->flags);
    if (same_path_nodes(&p->start_node->node_data, &node_data)
	&& p->start_node->next)
	return p->start_node->next->dir_from_parent;
    return -1;
}

struct path_node_data *
path_get_next_cached_node(void *pkey, struct path_node_data *pn_goal,
			  int *pflags)
{
    struct path **pp, *p;
    struct path_node_data node_data;
    
    assert(nodes_freed + nodes_in_use == nodes_allocated);
    if ((pp = path_locate(pkey)) == 0)
	return 0;
    p = *pp;

    if (!same_path_nodes(pn_goal, &p->goal_data))
	return 0;

    if (pflags)
	*pflags = p->flags;

    set_path_node_from_key(pkey, &node_data, p->flags);
    if (same_path_nodes(&p->start_node->node_data, &node_data)
	&& p->start_node->next)
	return &(p->start_node->next->node_data);
    return 0;
}

/*
 * want to ensure a path recalculation.
 */
void
path_clear_cache(void * pkey)
{
    struct path ** pp;
    if ((pp = path_locate(pkey)) != 0)
	remove_path(pp);
}

/*
 * Return dir for the direction to go in to reach the given destination.
 * return -1 if cant get there.
 */
int
path_get_next_move(void *pkey, struct path_node_data *pn_goal, int flags)
{
    struct path *ppath = 0;
    struct node *node_start;
    struct node *node_current = 0;
    int i;
    int ntests = 0;

    assert(nodes_freed + nodes_in_use == nodes_allocated);

	/* Causes desynch of network games. */
#ifdef USE_LR_PATHING
#ifdef USE_LR_PATHING_SOLO
    if (numremotes == 0)
#endif
    if (get_dir_from_cache(pkey, pn_goal, flags, &i))
	return i;
#endif

    cache_cleanout();
    assert(nodes_freed + nodes_in_use == nodes_allocated);

    node_start = get_free_node();
    set_path_node_from_key(pkey, &(node_start->node_data), flags);

    node_start->h =
	get_path_cost_estimate(&node_start->node_data, pn_goal, flags);

    nodelist_add(&open_list, node_start);
    while (open_list.nodes) {
	struct node *node_parent;
	int n_dir;
	node_current = nodelist_pop_best(&open_list, pkey, flags);
	if (node_current->h <= 0)
	    break;

	assert(nodes_freed + nodes_in_use + nodelist_count(&open_list)
	       + nodelist_count(&closed_list) + 1 == nodes_allocated);

	node_parent = node_current->parent;
	/*generate successor nodes, iterate 6 directions. */
	i = 0;
	n_dir =
	    get_path_direction_count(pkey, &node_current->node_data,
				     flags);
	for (i = 0; i < n_dir; i++) {
	    int g, h;
	    struct path_node_data node_data;
	    struct node *n_successor;
	    struct node *n_open;
	    struct node *n_closed;

	    set_dir_path_node_data(pkey, &node_current->node_data, i,
				   &node_data, flags);

	    if (node_parent
		&& same_path_nodes(&node_parent->node_data, &node_data))
		continue;

	    ntests++;
	    if ((g = cost_to_move_on_path(pkey,
					  &node_current->node_data,
					  &node_data, flags)) < 0)
		continue;

	    g += node_current->g;
	    h = get_path_cost_estimate(&node_data, pn_goal, flags);

	    if ((n_open = nodelist_find(&open_list, &node_data)) != 0
		&& (n_open->g + n_open->h) <= (g + h))
		continue;	/* existing open-list is equal or better. */

	    if ((n_closed = nodelist_find(&closed_list, &node_data)) != 0
		&& (n_closed->g + n_closed->h) <= (g + h))
		continue;	/* existing closed-list is equal or better. */

	    if (n_closed) {
		nodelist_remove(&closed_list, n_closed);
		free_node(n_closed);
	    }
	    if (n_open) {
		nodelist_remove(&open_list, n_open);
		n_successor = n_open;
		memset(n_successor, 0, sizeof(*n_open));
	    } else
		n_successor = get_free_node();

	    n_successor->g = g;
	    n_successor->h = h;
	    n_successor->parent = node_current;
	    n_successor->dir_from_parent = i;
	    n_successor->node_data = node_data;
	    nodelist_add(&open_list, n_successor);
	}
	nodelist_add(&closed_list, node_current);

	assert(nodes_freed + nodes_in_use + nodelist_count(&open_list)
	       + nodelist_count(&closed_list) == nodes_allocated);

	node_current = 0;
    }

    ppath = (struct path *) xmalloc(sizeof(*ppath));

    ppath->last_node = node_current;
    ppath->start_node = node_start;
    ppath->pkey = pkey;
    ppath->goal_data = *pn_goal;
    ppath->flags = flags;


    if (node_current) {
	struct node *n;
	for (n = node_current; n != 0; n = n->parent)
	    if (!nodelist_remove(&closed_list, n))
		nodelist_remove(&open_list, n);

	for (n = node_current; n != 0; n = n->parent) {
	    if (n->parent)
		n->parent->next = n;
	}
    } else {
	if (!nodelist_remove(&closed_list, node_start))
	    nodelist_remove(&open_list, node_start);
	node_start->next = 0;
    }

#if 0
    assert(nodelist_verify(&open_list));
    assert(nodelist_verify(&closed_list));
#endif

    nodelist_clear(&open_list);
    nodelist_clear(&closed_list);

    add_path_to_cache(ppath);


    assert(nodes_freed + nodes_in_use == nodes_allocated);

    if (node_start && node_start->next && ppath->last_node)
	return node_start->next->dir_from_parent;
    else
	return -1;
}

#endif
/*** DISABLED FOR NOW ***/
