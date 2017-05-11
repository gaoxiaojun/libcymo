/******************************************************************************
 * Quantitative Trading Library                                               *
 *                                                                            *
 * Copyright (C) 2017 Xiaojun Gao                                             *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 ******************************************************************************/

#include "event_pipe.h"
#include "spsc_queue.h"
#include <assert.h>
#include <uv.h>

typedef struct pipe_node_s {
    RB_ENTRY(pipe_node_s)
    rbentry;
    cm_queue_t *q;
    uint64_t queue_id;
} pipe_node_t;

static int pipe_node_compare(pipe_node_t *lhs, pipe_node_t *rhs)
{
    assert(lhs->q != NULL);
    assert(rhs->q != NULL);
    /*assert(lhs->queue_id != 0);
    assert(rhs->queue_id != 0);*/

    /* using for search duplicate queue */
    if (lhs->q == rhs->q)
	return 0;

    event_t *lhs_e = cm_queue_peek(lhs->q);
    event_t *rhs_e = cm_queue_peek(rhs->q);

    datetime_t lhs_time = lhs_e ? lhs_e->timestamp : MAX_DATE_TIME;
    datetime_t rhs_time = rhs_e ? rhs_e->timestamp : MAX_DATE_TIME;

    if (lhs_time < rhs_time)
	return -1;
    if (lhs_time > rhs_time)
	return 1;

    if (lhs->queue_id < rhs->queue_id)
	return -1;
    if (lhs->queue_id > rhs->queue_id)
	return 1;

    return 0;
}

RB_GENERATE_STATIC(pipe_tree_s, pipe_node_s, rbentry, pipe_node_compare)

/* for custom node alloc and free */
static inline pipe_node_t *node_new() { return malloc(sizeof(pipe_node_t)); }

static inline void node_free(pipe_node_t *node) { free(node); }

/*  convenient red-black tree operator */
static inline pipe_node_t *pipe_get_min_node(event_pipe_t *p)
{
    return RB_MIN(pipe_tree_s, &p->tree);
}

static inline pipe_node_t *pipe_remove(event_pipe_t *p, pipe_node_t *node)
{
    return RB_REMOVE(pipe_tree_s, &p->tree, node);
}

static inline pipe_node_t *pipe_insert(event_pipe_t *p, pipe_node_t *node)
{
    return RB_INSERT(pipe_tree_s, &p->tree, node);
}

static inline pipe_node_t *pipe_find(event_pipe_t *p, cm_queue_t *q)
{
    pipe_node_t node;
    node.q = q;
    return RB_FIND(pipe_tree_s, &p->tree, &node);
}

/* API */

int event_pipe_init(event_pipe_t *p)
{
    assert(p);
    RB_INIT(&p->tree);
    p->queue_counter = 1;
    return 0;
}

int event_pipe_destory(event_pipe_t *p)
{
    assert(p);
    event_pipe_clear(p);
    return 0;
}

/* add new queue
 * if q has been insert return -2
 * if malloc failed return -1
 * other return 0
 */
int event_pipe_add(event_pipe_t *p, cm_queue_t *q)
{
    assert(p);
    assert(q);
    pipe_node_t *node = node_new();
    if (!node)
	return -1;
    node->q = q;
    node->queue_id = p->queue_counter++;
    pipe_node_t *found = pipe_insert(p, node);
    if (found) {
	assert(found->q == q);
	node_free(node);
	return -2;
    }
    return 0;
}

/* remove queue
 * if q not found return -1
 * other return 0
 */
int event_pipe_remove(event_pipe_t *p, cm_queue_t *q)
{
    pipe_node_t *node = pipe_find(p, q);
    if (!node)
	return -1; // not found
    assert(node->q == q);
    pipe_remove(p, node);
    node_free(node);
    return 0;
}

int event_pipe_is_empty(event_pipe_t *p)
{
    pipe_node_t *node = pipe_get_min_node(p);
    if (!node)
	return 1;
    if (cm_queue_is_empty(node->q))
	return 1;
    return 0;
}

event_t *event_pipe_read(event_pipe_t *p)
{
    pipe_node_t *node = pipe_get_min_node(p);
    if (!node)
	return NULL;

    event_t *e;
    int empty = !cm_queue_pop(node->q, &e);
    if (empty)
	return NULL;

    pipe_remove(p, node);

    if (e->type == CM_EVENT_QUEUECLOSED) {
	event_unref(e);
	cm_queue_free(node->q);
	node_free(node);
	return e;
    }

    node->queue_id = p->queue_counter++;
    pipe_insert(p, node);
    return e;
}

void event_pipe_clear(event_pipe_t *p)
{
    pipe_node_t *np, *nn;
    RB_FOREACH_SAFE(np, pipe_tree_s, &p->tree, nn)
    {
	pipe_remove(p, np);
	node_free(np);
    }
}
