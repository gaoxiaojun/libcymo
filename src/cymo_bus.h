/******************************************************************************
 * Quantitative Trading Library                                               *
 *                                                                            *
 * Copyright (C) 2017 Xiaojun Gao                                             *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 ******************************************************************************/

#ifndef CTF_EVENT_BUS_H
#define CTF_EVENT_BUS_H

#include "cymo_event.h"
#include "cymo_queue.h"
#include "event_pipe.h"
#include "reminder_queue.h"
#include <stdbool.h>
#include <stdint.h>
#include <uv.h>

typedef enum { kRealtime, kSimulation } EventBusMode;
typedef enum { kIdleSpin, kIdleSleep, kIdleWait } EventBusIdleMode;

typedef enum {
    kBefore,
    kAfter
} ReminderOrder;

typedef struct event_bus_s {
    event_pipe_t pipe[4];
    reminder_queue_t clock_queue[2];
    datetime_t clock_datetime[2];
    ReminderOrder reminder_order;
    EventBusMode mode;
    EventBusIdleMode idle_mode;
    int is_simulation_stop;
    int show_warnings;
    int clear_reminders;
    int read_reminders;
    event_t* saved_event;
}event_bus_t;

#define DEFAULT_PENDING_QUEUE_SIZE 1024

#define CYMO_LOCAL_CLOCK    0
#define CYMO_EXCHANGE_CLOCK 1

#define CYMO_DATA_PIPE      0
#define CYMO_EXECUTION_PIPE 1
#define CYMO_HISTORICAL_PIPE 2
#define CYMO_SERVICE_PIPE   3

int event_bus_init(event_bus_t* bus);
int event_bus_destory(event_bus_t* bus);

void event_bus_attach(event_bus_t *bus);
void event_bus_deattach(event_bus_t *bus);

event_t *event_bus_dequeue(event_bus_t *bus);
int event_bus_add_queue(event_bus_t *bus, cm_queue_t *q);
void event_bus_clear(event_bus_t *bus);

datetime_t event_bus_get_datetime(event_bus_t *bus);
void event_bus_set_datetime(event_bus_t *bus, datetime_t time);
datetime_t event_bus_get_exchange_datetime(event_bus_t *bus);
void event_bus_set_exchange_datetime(event_bus_t *bus, datetime_t time);

// void event_bus_set_reminder_queue(event_bus_t *bus);

int event_bus_get_clear_reminders(event_bus_t *bus);
void event_bus_set_clear_reminders(event_bus_t *bus, int clearReminders);

int event_bus_get_show_warnings(event_bus_t *bus);
void event_bus_set_show_warnings(event_bus_t *bus, int showWarings);

EventBusIdleMode event_bus_get_idle_mode(event_bus_t *bus);
void event_bus_set_idle_mode(event_bus_t *bus, EventBusIdleMode mode);

EventBusMode event_bus_get_mode(event_bus_t *bus);
void event_bus_set_mode(event_bus_t *bus, EventBusMode mode);

/*cm_clock_t *event_bus_get_local_clock(event_bus_t *bus);
cm_clock_t *event_bus_get_exchange_clock(event_bus_t *bus);
*/

int event_bus_add_timer(event_bus_t *bus, int clock_type, datetime_t time, reminder_cb callback, void *user_data);
int event_bus_add_local_timer(event_bus_t *bus, datetime_t time,
            reminder_cb callback, void *user_data);
int event_bus_add_exchange_timer(event_bus_t *bus, datetime_t time,
            reminder_cb callback, void *user_data);
int event_bus_remove_timer(event_bus_t *bus, datetime_t time,
               reminder_cb callback, void *user_data);

#endif // CTF_EVENT_BUS_H
