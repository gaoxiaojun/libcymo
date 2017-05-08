/******************************************************************************
 * Quantitative Trading Library                                               *
 *                                                                            *
 * Copyright (C) 2017 Xiaojun Gao                                             *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 ******************************************************************************/

#include "cymo_clock.h"
#include <assert.h>

struct clock_s {
    clock_type type;
    clock_mode mode;
    datetime_t time;
    reminder_queue_t queue;
};

int clock_init(cm_clock_t *c, clock_type type)
{
    c->type = type;
    c->mode = CM_CLOCK_MODE_SIMULATION;
    reminder_queue_init(&c->queue);
    return 0;
}

int clock_destory(cm_clock_t *c)
{
    reminder_queue_destory(&c->queue);
    return 0;
}

cm_clock_t* clock_new(clock_type type)
{
    cm_clock_t* c = malloc(sizeof(cm_clock_t));
    if (!c)
        return NULL;

    clock_init(c, type);

    return c;
}

void clock_free(cm_clock_t* c)
{
    assert(c);
    clock_destory(c);
    free(c);
}

datetime_t clock_get_datetime(cm_clock_t* c)
{
    if (c->type == CM_CLOCK_TYPE_LOCAL)
        return c->time;
    if (c->mode != CM_CLOCK_MODE_REALTIME)
        return c->time;

    return datetime_now();
}

void clock_set_datetime(cm_clock_t* c, datetime_t time)
{
    if (c->type == CM_CLOCK_TYPE_EXCHANGE) {
        if (time < c->time) {
            printf("(Exchange) incorrect set orde\n");
            return;
        }
        c->time = time;
    } else {
        if (c->mode != CM_CLOCK_MODE_SIMULATION) {
            printf(
                "Can not set dateTime because Clock is not in the Simulation mode\n");
            return;
        }
        if (time < c->time) {
            printf("(Local) incorrect set orde\n");
            return;
        }
        c->time = time;
    }
}

clock_type clock_get_type(cm_clock_t* c) { return c->type; }

clock_mode clock_get_mode(cm_clock_t* c) { return c->mode; }

void clock_set_mode(cm_clock_t* c, clock_mode mode)
{
    c->mode = mode;
    if (mode == CM_CLOCK_MODE_SIMULATION)
        c->time = MIN_DATE_TIME;
}

void clock_clear(cm_clock_t* c)
{
    reminder_queue_clear(&c->queue);
    c->time = MIN_DATE_TIME;
}

reminder_t* clock_add_reminder(cm_clock_t* c, datetime_t time,
    reminder_cb callback, void* data)
{
    reminder_t *r = malloc(sizeof(reminder_t));
    if(!r) return NULL;

    if(reminder_queue_push(&c->queue, r))  {// fail
        free(r);
        return NULL;
    }

    return r;
}

void clock_remove_reminder(cm_clock_t* c, datetime_t time,
    reminder_cb callback) {}

reminder_queue_t* clock_get_queue(cm_clock_t* c) { return &c->queue; }
