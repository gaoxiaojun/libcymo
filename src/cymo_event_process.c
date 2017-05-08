/******************************************************************************
 * Quantitative Trading Library                                               *
 *                                                                            *
 * Copyright (C) 2017 Xiaojun Gao                                             *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 ******************************************************************************/
#include "adt_queue.h"
#include "cymo_event.h"

typedef void (*bar_factory_item_cb)(tick_t* t);
typedef struct bar_factory_item_s {
    bar_factory_item_cb cb;
} bar_factory_item_t;

typedef int (*event_processor_cb)(event_t*);

typedef struct event_processor_s {
    TAILQ_ENTRY(event_processor_s)
    entry;
    event_processor_cb cb;
} event_processor_t;

event_processor_t events_processor_list[] = { { TAILQ_INIT(entry), NULL },
    { TAILQ_INIT(entry), default_ask };
}
;

void cm_events_process(event_t* e)
{
    assert(e->type >= 0 && e->type < MAX_EVENT);
    void* q = event_list[e->type];
    while (q->next)
        *cb(e);
}

void process_tick(tick_t* t) { process_bar_factory(tick_t * t); }

cm_cycle_process_events_and_timers()
{
    for (;;) {

        update_real_time();

        if (cycle->terminate || cycle->quit)
            do_some_clear();

        while (pending_) {
            queue_t* q = pending_.pop();
            pipe.add(q);
        }

        event_t* e = bus.pop();

        if (e)
            cm_events_process(e);

        timeout = get_timeout();

        if (!e)
            cond_var_timewait(cond_var);
    }
}
