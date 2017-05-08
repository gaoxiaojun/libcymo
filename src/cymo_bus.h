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

#include "cymo_clock.h"
#include "cymo_event.h"
#include "event_pipe.h"
#include "cymo_queue.h"
#include "reminder_queue.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum { kRealtime, kSimulation } EventBusMode;
typedef enum { kIdleSpin, kIdleSleep, kIdleWait } EventBusIdleMode;
typedef enum {
  kDataPipe,
  kExecutionPipe,
  kServicePipe,
  kHistoricalPipe,
} EventBusPipe;

typedef struct event_bus_s event_bus_t;

event_bus_t *event_bus_new();
void event_bus_free(event_bus_t *bus);

void event_bus_attach(event_bus_t *bus);
void event_bus_deattach(event_bus_t *bus);

event_t *event_bus_dequeue(event_bus_t *bus);
int event_bus_add_queue(event_bus_t *bus, cm_queue_t *q, EventBusPipe pipe);
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

cm_clock_t *event_bus_get_local_clock(event_bus_t *bus);
cm_clock_t *event_bus_get_exchange_clock(event_bus_t *bus);

event_pipe_t *event_bus_get_data_pipe(event_bus_t *bus);
event_pipe_t *event_bus_get_execution_pipe(event_bus_t *bus);
event_pipe_t *event_bus_get_service_pipe(event_bus_t *bus);
event_pipe_t *event_bus_get_historical_pipe(event_bus_t *bus);

void event_bus_wakeup(event_bus_t *bus);

int event_bus_add_timer(event_bus_t *bus, datetime_t time,
                        reminder_cb *callback, void *user_data);
int event_bus_remove_timer(event_bus_t *bus, datetime_t time,
                           reminder_cb *callback);

#endif // CTF_EVENT_BUS_H
