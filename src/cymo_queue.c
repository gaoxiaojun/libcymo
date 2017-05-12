/******************************************************************************
 * Quantitative Trading Library                                               *
 *                                                                            *
 * Copyright (C) 2017 Xiaojun Gao                                             *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 ******************************************************************************/

#include "cymo_queue.h"
#include "cymo_event.h"
#include "cymo_loop.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

int cm_queue_init(cm_queue_t *q, cm_queue_type type, unsigned int size)
{
    assert(size >= 2);
    if (spsc_queue_init(&q->spsc, size) < 0)
        return -1;

    q->type = type;
    q->loop = NULL;
    return 0;
}

int cm_queue_destory(cm_queue_t* q)
{
    cm_queue_clear(q);
    spsc_queue_destroy(&q->spsc);
    return 0;
}

cm_queue_t* cm_queue_new(cm_queue_type type, unsigned int size)
{
    assert(size >= 2);
    cm_queue_t* q = malloc(sizeof(cm_queue_t));
    if (!q)
        return NULL;
    if (cm_queue_init(q, type, size)) {
        free(q);
        return NULL;
    }
    return q;
}

void cm_queue_free(cm_queue_t* q)
{
    assert(q);
    cm_queue_destory(q);
    free(q);
}

int cm_queue_push(cm_queue_t* q, event_t* e)
{
    assert(q);
    assert(e);
    int is_empty = spsc_queue_is_empty(&q->spsc);
    int err = spsc_queue_push(&q->spsc, e);
    if (is_empty && !err && q->loop)
        cymo_wakeup(q->loop);
    return err;
}

int cm_queue_pop(cm_queue_t* q, event_t** item)
{
    assert(q);
    assert(item);
    return spsc_queue_pop(&q->spsc, (void**)item);
}

event_t* cm_queue_peek(cm_queue_t* q)
{
    assert(q);
    return spsc_queue_peek(&q->spsc);
}

int cm_queue_is_empty(cm_queue_t* q)
{
    assert(q);
    return spsc_queue_is_empty(&q->spsc);
}

int cm_queue_is_full(cm_queue_t* q)
{
    assert(q);
    return spsc_queue_is_full(&q->spsc);
}

unsigned int cm_queue_size(cm_queue_t* q)
{
    assert(q);
    return spsc_queue_size(&q->spsc);
}

unsigned int cm_queue_capacity(cm_queue_t* q)
{
    assert(q);
    return spsc_queue_capacity(&q->spsc);
}

cm_queue_type cm_queue_get_type(cm_queue_t* q)
{
    assert(q);
    return q->type;
}

char* cm_queue_get_name(cm_queue_t* q)
{
    assert(q);
    return q->name;
}

void cm_queue_clear(cm_queue_t* q)
{
    event_t* e;
    while (!spsc_queue_pop(&q->spsc, (void**)&e)) {
        assert(e);
        event_unref((event_t*)e);
    }
}
