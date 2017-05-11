/******************************************************************************
 * Quantitative Kit Library                                                   *
 *                                                                            *
 * Copyright (C) 2017 Xiaojun Gao                                             *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 ******************************************************************************/

#include "cymo_event.h"
#include <stdarg.h>
#include <assert.h>

unsigned short event_ref(event_t* e)
{
    return atomic_fetch_add(&e->ref, 1);
}

unsigned short event_unref(event_t* e)
{
    unsigned short count;
    count = atomic_fetch_sub(&e->ref, 1);
    if (count == 1) {
        cm_event_free(e);
    }
    return count;
}

int event_is_single_ref(event_t* e)
{
    return (atomic_load(&e->ref) == 1);
}

static void tick_event_init(event_t* t, va_list args)
{
    tick_t *e = (tick_t *)t;
    e->provider = 0;
    e->instrument = 0;
    e->price = 0;
    e->size = 0;
}

static inline int event_check_type(event_type type)
{
    if (type >= CM_EVENT_CUSTOM_START)
        return 1;
    return 0;
}

event_t* cm_event_new(event_type type, ...)
{
    assert(!event_check_type(type));
    event_t* e = malloc(event_classes[type].size);
    if (!e)
        return NULL;
    e->type = type;
    e->ref = 1;
    va_list args;
    va_start(args, type);
    int successful = 0;
    event_init init = event_classes[type].init;
    if (init)
        init(e, args);
    va_end(args);
        return e;
}

void cm_event_free(event_t* e)
{
    event_destory destory = event_classes[e->type].destory;
    if (destory)
        destory(e);
    free(e);
}

event_class_t event_classes[CM_EVENT_CUSTOM_START] = {
    {
        // ASK
        .init = tick_event_init,
        .destory = NULL,
        .size = sizeof(tick_t)
    },
    {
        // BID
        .init = tick_event_init,
        .destory = NULL,
        .size = sizeof(tick_t)
    },
    {
        // TRADE
        .init = tick_event_init,
        .destory = NULL,
        .size = sizeof(tick_t)
    },
};

void cm_event_process(event_t *e)
{

}
