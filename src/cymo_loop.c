/******************************************************************************
 * Quantitative Kit Library                                                   *
 *                                                                            *
 * Copyright (C) 2017 Xiaojun Gao                                             *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 ******************************************************************************/
#include "cymo_loop.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h> // sleep/usleep
#endif

static event_t* dequeue(cymo_t* loop)
{
    cm_queue_t* q;
    while (spsc_queue_pop(&loop->pending, (void**)&q)) {
        q->loop = loop;
        event_bus_add_queue(&loop->bus, q);
    }

    return event_bus_dequeue(&loop->bus);
}

static inline void timewait(cymo_t* loop, uint64_t timeout)
{
    uv_mutex_lock(&loop->mutex);
    uv_cond_timedwait(&loop->wakeup_cond, &loop->mutex, timeout);
    uv_mutex_unlock(&loop->mutex);
}

static inline int64_t get_next_timeout(cymo_t* loop)
{
    return MIN_DATE_TIME;
}

static void cymo_clear(cymo_t *loop)
{

}

static void cymo_main(void* arg)
{
    cymo_t* loop = (cymo_t*)arg;
    loop->status = CYMO_STATUS_RUNNING;
    while(!loop->quit){
        if(loop->status != CYMO_STATUS_RUNNING && (loop->status != CYMO_STATUS_PAUSED || !loop->stepping))
            cymo_sleep(1);
        else {
            event_t *e = dequeue(loop);
            cm_event_process(e);
        }

    }
    cymo_clear(loop);
    loop->status = CYMO_STATUS_STOPPING;
}

int cymo_init(cymo_t* loop)
{
    spsc_queue_init(&loop->pending, DEFAULT_PENDING_QUEUE_SIZE);
    uv_mutex_init(&loop->mutex);
    uv_cond_init(&loop->wakeup_cond);
    return 0;
}

int cymo_destory(cymo_t* loop)
{
    spsc_queue_destroy(&loop->pending);
    uv_mutex_destroy(&loop->mutex);
    uv_cond_destroy(&loop->wakeup_cond);
    return 0;
}

void cymo_start(cymo_t* loop, cymo_mode mode)
{
    uv_thread_create(&loop->self, cymo_main, loop);
    loop->quit = 0;
}

void cymo_stop(cymo_t* loop)
{
    if (loop->status != CYMO_STATUS_STOPPED) {
        loop->quit = 1;
        loop->status = CYMO_STATUS_STOPPING;
        cymo_wakeup(loop);
        uv_thread_join(&loop->self);
    }
    loop->status = CYMO_STATUS_STOPPED;
    event_bus_clear(&loop->bus);
    cm_event_process(cm_event_new(CM_EVENT_LOOP_STOPPED));
}

void cymo_pause(cymo_t* loop)
{
    if (loop->status != CYMO_STATUS_PAUSED) {
        loop->status = CYMO_STATUS_PAUSED;
        cm_event_process(cm_event_new(CM_EVENT_LOOP_PAUSE));
    }
}

void cymo_step(cymo_t* loop, event_type type)
{
    loop->stepping = 1;
    loop->step_event_type = type;
    cm_event_process(cm_event_new(CM_EVENT_LOOP_STEP));
}

void cymo_wakeup(cymo_t* loop)
{
    uv_mutex_lock(&loop->mutex);
    uv_cond_signal(&loop->wakeup_cond);
    uv_mutex_unlock(&loop->mutex);
}

int cymo_add_queue(cymo_t* loop, cm_queue_t* q)
{
    uv_thread_t this_thread = uv_thread_self();
    if (uv_thread_equal(&loop->self, &this_thread)) {
        q->loop = loop;
        return event_bus_add_queue(&loop->bus, q);
    } else {
        int err = spsc_queue_push(&loop->pending, q);

        if (!err) {
            cymo_wakeup(loop);
        }
        return err;
    }
}

int cymo_add_reminder(cymo_t* loop, int clock_type, datetime_t time_at, timer_cb_t cb, void* data)
{
    return event_bus_add_timer(&loop->bus, clock_type, time_at, cb, data);
}


void cymo_sleep(uint64_t millisecondsTimeout)
{
#ifdef _WIN32
    Sleep(millisecondsTimeout);
#else
    int sec;
    int usec;

    sec = millisecondsTimeout / 1000;
    usec = (millisecondsTimeout % 1000) * 1000;
    if (sec > 0)
      sleep(sec);
    if (usec > 0)
      usleep(usec);
#endif
}

void cymo_resume(cymo_t *loop)
{
    if (loop->status != CYMO_STATUS_RUNNING) {
        loop->status = CYMO_STATUS_RUNNING;
        cm_event_process(cm_event_new(CM_EVENT_LOOP_RESUME));
    }
}

static void _pause(datetime_t timestamp, void *user_data)
{
    cymo_pause((cymo_t *)user_data);
}

void cymo_timed_pause(cymo_t *loop, datetime_t timeout)
{
    cymo_add_reminder(loop, CM_CLOCK_LOCAL, timeout, &_pause, loop);
}
