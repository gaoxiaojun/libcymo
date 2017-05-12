/******************************************************************************
 * Quantitative Trading Library                                               *
 *                                                                            *
 * Copyright (C) 2017 Xiaojun Gao                                             *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 ******************************************************************************/

#include "cymo_bus.h"
#include "cymo_event.h"
#include "reminder_queue.h"
#include "spsc_queue.h"
#include "util.h"
#include <stdlib.h>
#include <unistd.h>

int event_bus_init(event_bus_t* bus)
{
    bus->show_warnings = true;
    bus->read_reminders = true;
    bus->idle_mode = kIdleWait;
    bus->mode = kSimulation;

    for (int i = 0; i < 4; i++)
        event_pipe_init(&bus->pipe[i]);

    reminder_queue_init(&bus->clock_queue[0]);
    reminder_queue_init(&bus->clock_queue[1]);

    return 0;
}

int event_bus_destory(event_bus_t* bus)
{
    for (int i = 0; i < 4; i++)
        event_pipe_destory(&bus->pipe[i]);

    for (int i = 0; i < 2; i++)
        reminder_queue_destory(&bus->clock_queue[i]);

    return 0;
}

void event_bus_attach(event_bus_t* bus) {}

void event_bus_deattach(event_bus_t* bus) {}

static event_t* enqueue_attach(event_bus_t* bus)
{
    event_t* e = bus->saved_event;
    bus->saved_event = NULL;
    /*for (int i = 0; i < bus->queue_count; i++) {
  EventType etype = event_get_type(e);
  if (etype != kOnQueueClosed && etype != kOnQueueOpen) {
    bus->queues[i]->enqueue(e);
  }
}*/
    return e;
}

static event_t* simulation_dequeue(event_bus_t* bus)
{
    while (true) {
        int data_is_empty = event_pipe_is_empty(&bus->pipe[CYMO_DATA_PIPE]);
        if (!data_is_empty && !bus->saved_event) {
            event_t* e = event_pipe_read(&bus->pipe[CYMO_DATA_PIPE]);
            event_type etype = e->type;
            datetime_t etime = e->timestamp;
            if (etype == CM_EVENT_SIMULATOR_STOP) {
                bus->is_simulation_stop = true;
                continue;
            }
            if (etime < event_bus_get_datetime(bus)) {
            }
            bus->saved_event = e;
        }

        if (!event_pipe_is_empty(&bus->pipe[CYMO_EXECUTION_PIPE])) {
            return event_pipe_read(&bus->pipe[CYMO_EXECUTION_PIPE]);
        }

        if (!reminder_queue_is_empty(&bus->clock_queue[CYMO_LOCAL_CLOCK])) {
            reminder_t* r;
            if (!reminder_queue_pop(&bus->clock_queue[CYMO_LOCAL_CLOCK], &r))
                return (event_t*)r;
        }

        if (!reminder_queue_is_empty(&bus->clock_queue[CYMO_EXCHANGE_CLOCK])) {
            reminder_t* r;
            if (!reminder_queue_pop(&bus->clock_queue[CYMO_EXCHANGE_CLOCK], &r))
                return (event_t*)r;
        }

        if (!event_pipe_is_empty(&bus->pipe[CYMO_SERVICE_PIPE])) {
            return event_pipe_read(&bus->pipe[CYMO_SERVICE_PIPE]);
        }

        if (bus->saved_event) {
            return enqueue_attach(bus);
        }

        if (bus->is_simulation_stop) {
            bus->saved_event = cm_event_new(CM_EVENT_SIMULATOR_STOP); //(event_t*)new_on_simulation_stop_event();
            bus->saved_event->timestamp = event_bus_get_datetime(bus);
            bus->is_simulation_stop = false;
            return enqueue_attach(bus);
        }
    }
}

static event_t* realtime_dequeue(event_bus_t* bus)
{
    while (true) {
        int data_is_empty = event_pipe_is_empty(&bus->pipe[CYMO_DATA_PIPE]);

        if (!data_is_empty && !bus->saved_event) {
            bus->saved_event = event_pipe_read(&bus->pipe[CYMO_DATA_PIPE]);
        }

        if (!reminder_queue_is_empty(&bus->clock_queue[CYMO_LOCAL_CLOCK])) {
            reminder_t* r;
            if (!reminder_queue_pop(&bus->clock_queue[CYMO_LOCAL_CLOCK], &r))
                return (event_t*)r;
        }

        if (!reminder_queue_is_empty(&bus->clock_queue[CYMO_EXCHANGE_CLOCK])) {
            reminder_t* r;
            if (!reminder_queue_pop(&bus->clock_queue[CYMO_EXCHANGE_CLOCK], &r))
                return (event_t*)r;
        }

        if (!event_pipe_is_empty(&bus->pipe[CYMO_EXECUTION_PIPE])) {
            return event_pipe_read(&bus->pipe[CYMO_EXECUTION_PIPE]);
        }

        if (!event_pipe_is_empty(&bus->pipe[CYMO_SERVICE_PIPE])) {
            return event_pipe_read(&bus->pipe[CYMO_SERVICE_PIPE]);
        }

        if (bus->saved_event) {
            event_t* e = bus->saved_event;
            bus->saved_event = NULL;
            return e;
        }
    }
}

event_t* event_bus_dequeue(event_bus_t* bus)
{
    if (bus->mode == kSimulation)
        return simulation_dequeue(bus);
    else
        return realtime_dequeue(bus);
}

int event_bus_add_queue(event_bus_t* bus, cm_queue_t* q)
{
    event_pipe_add(&bus->pipe[q->type], q);
    return true;
}

void event_bus_clear(event_bus_t* bus)
{
    free(bus->saved_event);
    bus->saved_event = NULL;

    for (int i = 0; i < 2; i++)
        reminder_queue_clear(&bus->clock_queue[i]);

    for (int i = 0; i < 4; i++)
        event_pipe_clear(&bus->pipe[i]);

    bus->is_simulation_stop = false;
}

int event_bus_get_clear_reminders(event_bus_t* bus)
{
    return bus->clear_reminders;
}

void event_bus_set_clear_reminders(event_bus_t* bus, int clearReminders)
{
    bus->clear_reminders = clearReminders;
}

int event_bus_get_show_warnings(event_bus_t* bus) { return bus->show_warnings; }

void event_bus_set_show_warnings(event_bus_t* bus, int showWarings)
{
    bus->show_warnings = showWarings;
}

EventBusIdleMode event_bus_get_idle_mode(event_bus_t* bus)
{
    return bus->idle_mode;
}

void event_bus_set_idle_mode(event_bus_t* bus, EventBusIdleMode mode)
{
    bus->idle_mode = mode;
}

EventBusMode event_bus_get_mode(event_bus_t* bus) { return bus->mode; }

void event_bus_set_mode(event_bus_t* bus, EventBusMode mode)
{
    bus->mode = mode;
}

datetime_t event_bus_get_datetime(event_bus_t* bus)
{
    if (bus->mode == kRealtime)
        return bus->clock_datetime[CYMO_LOCAL_CLOCK];
    else
        return datetime_now();
}

void event_bus_set_datetime(event_bus_t* bus, datetime_t time)
{
    if (bus->mode != kSimulation) {
        printf("Can not set dateTime because Clock is not in the Simulation "
               "mode\n");
        return;
    }

    if (time < bus->clock_datetime[CYMO_LOCAL_CLOCK]) {
        printf("%s\n", "local clock datetime out of order");
        return;
    }

    bus->clock_datetime[CYMO_LOCAL_CLOCK] = time;
}

datetime_t event_bus_get_exchange_datetime(event_bus_t* bus)
{
    return bus->clock_datetime[CYMO_EXCHANGE_CLOCK];
}

void event_bus_set_exchange_datetime(event_bus_t* bus, datetime_t time)
{
    if (time < bus->clock_datetime[CYMO_EXCHANGE_CLOCK]) {
        printf("%s\n", "exchange clock datetime out of order");
        return;
    }
    bus->clock_datetime[CYMO_EXCHANGE_CLOCK] = time;
}

int event_bus_add_timer(event_bus_t* bus, int clock_type, datetime_t time, reminder_cb callback, void* user_data)
{
    reminder_t* timer = malloc(sizeof(reminder_t));
    if (!timer)
        return -1;
    timer->callback = callback;
    timer->user_data = user_data;
    timer->type = CM_EVENT_REMINDER;
    timer->timestamp = time;

    if (reminder_queue_push(&bus->clock_queue[clock_type], timer, NULL)) {
        free(timer);
        return -2;
    }

    return 0;
}

int event_bus_add_local_timer(event_bus_t* bus, datetime_t time, reminder_cb callback, void* user_data)
{
    return event_bus_add_timer(bus, CYMO_LOCAL_CLOCK, time, callback, user_data);
}

int event_bus_add_exchange_timer(event_bus_t* bus, datetime_t time, reminder_cb callback, void* user_data)
{
    return event_bus_add_timer(bus, CYMO_EXCHANGE_CLOCK, time, callback, user_data);
}
