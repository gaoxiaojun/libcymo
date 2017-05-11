/******************************************************************************
 * Quantitative Trading Library                                               *
 *                                                                            *
 * Copyright (C) 2017 Xiaojun Gao                                             *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 ******************************************************************************/

#ifndef EVENT_PIPE_H
#define EVENT_PIPE_H

#include "adt_tree.h"
#include "cymo_queue.h"
#include <stdbool.h>
#include <stdint.h>

struct pipe_node_s;
RB_HEAD(pipe_tree_s, pipe_node_s);

typedef struct event_pipe_s {
    struct pipe_tree_s tree;
    uint64_t queue_counter;
} event_pipe_t;

int event_pipe_init(event_pipe_t *p);
int event_pipe_destory(event_pipe_t *p);

int event_pipe_add(event_pipe_t *p, cm_queue_t *q);
int event_pipe_remove(event_pipe_t *p, cm_queue_t *q);
void event_pipe_clear(event_pipe_t *p);

int event_pipe_is_empty(event_pipe_t *p);

event_t *event_pipe_read(event_pipe_t *p);

#endif // EVENT_PIPE_H
