/******************************************************************************
 * Quantitative Trading Library                                               *
 *                                                                            *
 * Copyright (C) 2017 Xiaojun Gao                                             *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 ******************************************************************************/

#ifndef CTF_CLOCK_H
#define CTF_CLOCK_H

#include <cymo.h>

#include "reminder_queue.h"

typedef struct clock_s cm_clock_t;

typedef enum {
    CM_CLOCK_TYPE_LOCAL,
    CM_CLOCK_TYPE_EXCHANGE
}clock_type;

typedef enum {
    CM_CLOCK_MODE_REALTIME,
    CM_CLOCK_MODE_SIMULATION
}clock_mode;

int clock_init(cm_clock_t *clock, clock_type type);
int clock_destory(cm_clock_t *clock);

cm_clock_t* clock_new(clock_type type);
void clock_free(cm_clock_t* c);

datetime_t clock_get_datetime(cm_clock_t* c);
void clock_set_datetime(cm_clock_t* c, datetime_t time);
clock_type clock_get_type(cm_clock_t* c);
clock_mode clock_get_mode(cm_clock_t* c);
void clock_set_mode(cm_clock_t* c, clock_mode mode);
void clock_clear(cm_clock_t* c);
reminder_t* clock_add_reminder(cm_clock_t* c, datetime_t time,
    reminder_cb callback, void* data);
void clock_remove_reminder(cm_clock_t* c, datetime_t time,
    reminder_cb callback);
reminder_queue_t* clock_get_queue(cm_clock_t* c);

#endif // CTF_CLOCK_H
