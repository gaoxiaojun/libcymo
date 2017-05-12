/******************************************************************************
 * Quantitative Kit Library                                                   *
 *                                                                            *
 * Copyright (C) 2017 Xiaojun Gao                                             *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 ******************************************************************************/
#ifndef CYMO_LOOP_H
#define CYMO_LOOP_H

#include <stdint.h>
#include <uv.h>
#include "spsc_queue.h"
#include "cymo_event.h"
#include "event_pipe.h"
#include "reminder_queue.h"
#include "cymo_queue.h"

typedef enum {
    CYMO_LOCAL_CLOCK,
    CYMO_EXCHANGE_CLOCK
}clock_type;

typedef enum {
    CYMO_SIMULATOR,
    CYMO_REALTIME
}cymo_mode;

typedef enum {
    kBefore,
    kAfter
} ReminderOrder;


/* status
 *       +--> runing   ----+
 * init--+--> paused   ----+--> stopping --> stopped
 */
typedef enum {
    CYMO_STAUTS_INIT,
    CYMO_STATUS_RUNNING,
    CYMO_STATUS_PAUSED,
    CYMO_STATUS_STOPPING,
    CYMO_STATUS_STOPPED
}cymo_status;

typedef struct cymo_loop_s {
    uv_thread_t self;
    spsc_queue_t pending;
    cymo_mode mode;
    //event_bus_t bus;
    uv_mutex_t mutex;
    uv_cond_t wakeup_cond;
    volatile int quit;
    cymo_status status;
    int stepping;
    event_type step_event_type;
    /* bus */
    event_pipe_t pipe[4];
    reminder_queue_t clock_queue[2];
    datetime_t clock_datetime[2];
    ReminderOrder reminder_order;

    int is_simulation_stop;
    int show_warnings;
    int clear_reminders;
    int read_reminders;
    event_t* saved_event;
}cymo_t;



int cymo_init(cymo_t *loop);
int cymo_destory(cymo_t *loop);

void cymo_start(cymo_t *loop, cymo_mode mode);
void cymo_stop(cymo_t *loop);
void cymo_pause(cymo_t *loop);
void cymo_timed_pause(cymo_t *loop, datetime_t timeout);
void cymo_step(cymo_t *loop, event_type type);
void cymo_resume(cymo_t *loop);

void cymo_wakeup(cymo_t *loop);
void cymo_sleep(uint64_t millisecondsTimeout);

int cymo_add_queue(cymo_t *loop, cm_queue_t *q);
int cymo_add_timer(cymo_t *loop, datetime_t time_at, timer_cb_t cb, void *data);

datetime_t cymo_get_datetime(cymo_t *loop);
void cymo_set_datetime(cymo_t *loop, datetime_t time);


#endif // CYMO_LOOP_H
