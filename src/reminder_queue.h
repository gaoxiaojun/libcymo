/******************************************************************************
 * Quantitative Trading Library                                               *
 *                                                                            *
 * Copyright (C) 2017 Xiaojun Gao                                             *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 ******************************************************************************/

#ifndef __TIMER_QUEUE_H__
#define __TIMER_QUEUE_H__

#include "adt_tree.h"
#include "cymo_event.h"
#include <stdbool.h>

RB_HEAD(timer_tree_s, timer_node_s);

typedef struct reminder_queue_s {
  struct timer_tree_s tree;
  uint64_t timer_counter;
} reminder_queue_t;

typedef void (*reminder_cb)(datetime_t time, void *data);

typedef struct {
  EVENT_PUBLIC_FIELDS
  reminder_cb callback;
  void *user_data;
} reminder_t;

int reminder_queue_init(reminder_queue_t *q);
int reminder_queue_destory(reminder_queue_t *q);

reminder_queue_t *reminder_queue_new();
void reminder_queue_free(reminder_queue_t *q);

int reminder_queue_push(reminder_queue_t *q, reminder_t *);

int reminder_queue_pop(reminder_queue_t *q, reminder_t **);

reminder_t *reminder_queue_peek(reminder_queue_t *q);

int reminder_queue_is_empty(reminder_queue_t *q);

void reminder_queue_clear(reminder_queue_t *q);

int reminder_queue_remove(reminder_queue_t *q, reminder_t *e);

#endif // __TIMER_QUEUE_H__
