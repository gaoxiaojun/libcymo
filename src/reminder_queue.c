/******************************************************************************
 * Quantitative Trading Library                                               *
 *                                                                            *
 * Copyright (C) 2017 Xiaojun Gao                                             *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 ******************************************************************************/

#include "reminder_queue.h"
#include "adt_tree.h"
#include <assert.h>

typedef struct timer_node_s {
    RB_ENTRY(timer_node_s)
    rbentry;
    reminder_t* reminder;
    uint64_t timer_id;
} timer_node_t;

int timer_node_compare(timer_node_t* lhs, timer_node_t* rhs)
{
    /*assert(lhs->reminder != NULL);
    assert(rhs->reminder != NULL);
    assert(lhs->timer_id != 0);
    assert(rhs->timer_id != 0);*/

    if (lhs->reminder == rhs->reminder && lhs->reminder != NULL && rhs->reminder != NULL)
        return 0;

    if (lhs->timer_id == rhs->timer_id && lhs->timer_id != 0 && rhs->timer_id != 0)
        return 0;

    if (lhs->reminder->timestamp == rhs->reminder->timestamp
        && lhs->reminder->callback == rhs->reminder->callback
        && lhs->reminder->user_data == rhs->reminder->user_data)
        return 0;

    datetime_t lhs_time = lhs->reminder->timestamp;
    datetime_t rhs_time = rhs->reminder->timestamp;

    if (lhs_time < rhs_time)
        return -1;
    if (lhs_time > rhs_time)
        return 1;

    if (lhs->timer_id < rhs->timer_id)
        return -1;
    if (lhs->timer_id > rhs->timer_id)
        return 1;

    return 0;
}

RB_GENERATE_STATIC(timer_tree_s, timer_node_s, rbentry, timer_node_compare)

/* for custom node alloc and free */
static inline timer_node_t* node_new() { return malloc(sizeof(timer_node_t)); }

static inline void node_free(timer_node_t* node) { free(node); }

/*  convenient red-black tree operator */
static inline timer_node_t* get_min_node(reminder_queue_t* q)
{
    return RB_MIN(timer_tree_s, &q->tree);
}

static inline timer_node_t* timer_remove(reminder_queue_t* q,
    timer_node_t* node)
{
    return RB_REMOVE(timer_tree_s, &q->tree, node);
}

static inline timer_node_t* timer_insert(reminder_queue_t* q,
    timer_node_t* node)
{
    return RB_INSERT(timer_tree_s, &q->tree, node);
}

static inline timer_node_t* timer_find(reminder_queue_t* q, reminder_t* e)
{
    timer_node_t node;
    node.reminder = e;
    return RB_FIND(timer_tree_s, &q->tree, &node);
}

/* API */

int reminder_queue_init(reminder_queue_t* q)
{
    assert(q);
    RB_INIT(&q->tree);
    q->timer_counter = 1;
    return 0;
}

int reminder_queue_destory(reminder_queue_t* q)
{
    assert(q);
    reminder_queue_clear(q);
    return 0;
}

reminder_queue_t* reminder_queue_new()
{
    reminder_queue_t* q = malloc(sizeof(reminder_queue_t));
    if (!q)
        return NULL;
    reminder_queue_init(q);
    return q;
}

void reminder_queue_free(reminder_queue_t* q)
{
    reminder_queue_destory(q);
    free(q);
}

/* add reminder
 * if e has been insert return -2
 * if malloc failed return -1
 * other return 0
 */
int reminder_queue_push(reminder_queue_t* q, reminder_t* e, uint64_t* timer_id)
{
    assert(q);
    assert(e);
    timer_node_t* node = node_new();
    if (!node)
        return -1;
    node->reminder = e;
    node->timer_id = q->timer_counter++;
    timer_node_t* found = timer_insert(q, node);
    if (found) {
        assert(found->reminder == e);
        node_free(node);
        return -2;
    }
    if (timer_id)
        *timer_id = node->timer_id;
    return node->timer_id;
}

int reminder_queue_pop(reminder_queue_t* q, reminder_t** ritem)
{
    timer_node_t* node = get_min_node(q);

    if (node) {
        *ritem = node->reminder;
        timer_remove(q, node);
        node_free(node);
        return 0;
    } else {
        *ritem = NULL;
        return -1;
    }
}

reminder_t* reminder_queue_peek(reminder_queue_t* q)
{
    timer_node_t* node = get_min_node(q);

    return node != NULL ? node->reminder : NULL;
}

int reminder_queue_is_empty(reminder_queue_t* q) { return RB_EMPTY(&q->tree); }

void reminder_queue_clear(reminder_queue_t* q)
{
    timer_node_t *np, *nn;
    RB_FOREACH_SAFE(np, timer_tree_s, &q->tree, nn)
    {
        timer_remove(q, np);
        node_free(np);
    }
}

int reminder_queue_remove(reminder_queue_t* q, datetime_t timeout, reminder_cb callback, void* data)
{
    reminder_t r;
    r.timestamp = timeout;
    r.callback = callback;
    r.user_data = data;
    timer_node_t fnode;
    fnode.reminder = &r;
    fnode.timer_id = 0; // UNUSED
    timer_node_t* node = RB_FIND(timer_tree_s, &q->tree, &fnode);
    if (!node)
        return -1; // not found

    //assert(node->reminder == timeout);
    timer_remove(q, node);
    node_free(node);
    return 0;
}

int reminder_queue_remove_by_id(reminder_queue_t* q, uint64_t timer_id)
{
    timer_node_t fnode;
    fnode.reminder = NULL; // UNUSED
    fnode.timer_id = timer_id;
    timer_node_t* node = RB_FIND(timer_tree_s, &q->tree, &fnode);
    if (!node)
        return -1; // not found

    //assert(node->reminder->timestamp == timeout);
    timer_remove(q, node);
    node_free(node);
    return 0;
}
