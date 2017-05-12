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

#define DEFAULT_PENDING_QUEUE_SIZE 1024

/*static void *realtime_run(cymo_t *loop) {

    // process local clock
  qk_timer_t *local_timer = timer_get_min(loop, QK_LOCAL_TIMER);
  if (local_timer && local_timer->timeout <= loop->datetime[QK_LOCAL_TIMER]) {
      timer_remove(loop, QK_LOCAL_TIMER, local_timer);
      //TODO qk_timer_again(local_timer);
    return local_timer;
  }

  // process exchage clock
  datetime_t exchange_timestamp = get_data_pipe_exchange_time(loop);
  qk_timer_t *exchange_timer = RB_MIN(qk_timer_tree_s, &loop->timers[QK_EXCHANGE_TIMER]);
  if (exchange_timer && exchange_timer->timeout <= exchange_timestamp) {
    timer_remove(loop,QK_EXCHANGE_TIMER, exchange_timer);
    // TODO qk_timer_again(exchange_timer);
    return exchange_timer;
  }

  qk_event_t *e;
  if ((e = get_event_and_update_queue(loop, QK_EXECUTION_QUEUE)))
    return e;

  if ((e = get_event_and_update_queue(loop, QK_SERVICE_QUEUE)))
    return e;

  if ((e = get_event_and_update_queue(loop, CYMO))) {
      process_events(e);
    return e;
  }

  return NULL;
}*/
#define DATA_PIPE 0
#define EXECUTION_PIPE 1
#define HISTORICAL_PIPE 2
#define SERVICE_PIPE 3

static event_t* realtime_dequeue(cymo_t* loop)
{
    while (true) {
        if (!event_pipe_is_empty(&loop->pipe[DATA_PIPE]) && !loop->saved_event) {
            event_t* e = event_pipe_read(&loop->pipe[DATA_PIPE]);

            if (e->type == CM_EVENT_SIMULATOR_STOP) {
                loop->is_simulation_stop = 1;
                continue;
            }

            if (e->timestamp < cymo_get_datetime(loop)) {
                if (e->type != CM_EVENT_QUEUEOPEND && e->type != CM_EVENT_QUEUECLOSED) {
                    if (e->type != CM_EVENT_SIMULATOR_PROGRESS) {
                        if (loop->show_warnings) {
                            //LOG
                            continue;
                        }
                        continue;
                    }
                }
                e->timestamp = cymo_get_datetime(loop);
            }
            loop->saved_event = e;
        }

        if (!event_pipe_is_empty(&loop->pipe[EXECUTION_PIPE])) {
            return event_pipe_read(&loop->pipe[EXECUTION_PIPE]);
        }

        // local clock
        if (!reminder_queue_is_empty(&loop->clock_queue[CYMO_LOCAL_CLOCK])) {
            if (loop->is_simulation_stop) {
                if (loop->clear_reminders) {
                    reminder_queue_clear(&loop->clock_queue[CYMO_LOCAL_CLOCK]);
                } else if (loop->read_reminders) {
                    reminder_t* t;
                    reminder_queue_pop(&loop->clock_queue[CYMO_LOCAL_CLOCK], &t);
                    return (event_t*)t;
                }
            }

            if (loop->saved_event) {
                if (loop->reminder_order == kBefore) {
                    reminder_t* t = reminder_queue_peek(&loop->clock_queue[CYMO_LOCAL_CLOCK]);
                    if (t->timestamp <= loop->saved_event->timestamp)
                        break;
                    else if (t->timestamp < loop->saved_event->timestamp) {
                        reminder_t* t;
                        reminder_queue_pop(&loop->clock_queue[CYMO_LOCAL_CLOCK], &t);
                        return (event_t*)t;
                    }
                }
            }
        }

        // exchage clock

        if (!reminder_queue_is_empty(&loop->clock_queue[CYMO_EXCHANGE_CLOCK]) && loop->saved_event != NULL && (loop->saved_event->type == CM_EVENT_BID || loop->saved_event->type == CM_EVENT_ASK || loop->saved_event->type == CM_EVENT_TRADE)) {
            if (loop->reminder_order == kBefore) {
                reminder_t* t = reminder_queue_peek(&loop->clock_queue[CYMO_EXCHANGE_CLOCK]);
                if (t->timestamp <= ((tick_t*)loop->saved_event)->exchange_timestamp) {
                    reminder_t* t;
                    reminder_queue_pop(&loop->clock_queue[CYMO_EXCHANGE_CLOCK], &t);
                    return (event_t*)t;
                } else if (t->timestamp < ((tick_t*)loop->saved_event)->exchange_timestamp) {
                    reminder_t* t;
                    reminder_queue_pop(&loop->clock_queue[CYMO_EXCHANGE_CLOCK], &t);
                    return (event_t*)t;
                }
            }
        }

        // command
        // service
        if (!event_pipe_is_empty(&loop->pipe[SERVICE_PIPE]))
            return event_pipe_read(&loop->pipe[SERVICE_PIPE]);

        if (loop->saved_event) {
            event_t* e = loop->saved_event;
            loop->saved_event = NULL;
            return e;
        }

        if (loop->is_simulation_stop) {
            loop->saved_event = cm_event_new(CM_EVENT_SIMULATOR_STOP);
            loop->saved_event->timestamp = cymo_get_datetime(loop);
            loop->is_simulation_stop = 0;
            event_t* e = loop->saved_event;
            loop->saved_event = NULL;
            return e;
        }
    }
}

static event_t* simulator_dequeue(cymo_t* loop)
{
    while (true) {
        if (!event_pipe_is_empty(&loop->pipe[DATA_PIPE]) && !loop->saved_event) {
            loop->saved_event = event_pipe_read(&loop->pipe[DATA_PIPE]);
        }



        // local clock
        if (!reminder_queue_is_empty(&loop->clock_queue[CYMO_LOCAL_CLOCK])) {

                if (loop->reminder_order == kBefore) {
                    reminder_t* t = reminder_queue_peek(&loop->clock_queue[CYMO_LOCAL_CLOCK]);
                    if (t->timestamp <= loop->saved_event->timestamp)
                        break;
                    else if (t->timestamp < loop->saved_event->timestamp) {
                        reminder_t* t;
                        reminder_queue_pop(&loop->clock_queue[CYMO_LOCAL_CLOCK], &t);
                        return (event_t*)t;
                    }

            }
        }

        // exchage clock

        if (!reminder_queue_is_empty(&loop->clock_queue[CYMO_EXCHANGE_CLOCK]) && loop->saved_event != NULL
                && (loop->saved_event->type == CM_EVENT_BID
                    || loop->saved_event->type == CM_EVENT_ASK
                    || loop->saved_event->type == CM_EVENT_TRADE)) {
            if (loop->reminder_order == kBefore) {
                reminder_t* t = reminder_queue_peek(&loop->clock_queue[CYMO_EXCHANGE_CLOCK]);
                if (t->timestamp <= ((tick_t*)loop->saved_event)->exchange_timestamp) {
                    reminder_t* t;
                    reminder_queue_pop(&loop->clock_queue[CYMO_EXCHANGE_CLOCK], &t);
                    return (event_t*)t;
                } else if (t->timestamp < ((tick_t*)loop->saved_event)->exchange_timestamp) {
                    reminder_t* t;
                    reminder_queue_pop(&loop->clock_queue[CYMO_EXCHANGE_CLOCK], &t);
                    return (event_t*)t;
                }
            }
        }

        // execution
        if (!event_pipe_is_empty(&loop->pipe[EXECUTION_PIPE])) {
            return event_pipe_read(&loop->pipe[EXECUTION_PIPE]);
        }
        // service
        if (!event_pipe_is_empty(&loop->pipe[SERVICE_PIPE]))
            return event_pipe_read(&loop->pipe[SERVICE_PIPE]);

        if (loop->saved_event) {
            event_t* e = loop->saved_event;
            loop->saved_event = NULL;
            return e;
        }
    }
}

static event_t* dequeue(cymo_t* loop)
{
    cm_queue_t* q;
    while (spsc_queue_pop(&loop->pending, (void**)&q)) {
        q->loop = loop;
        event_pipe_add(&loop->pipe[q->type], q);
    }

    if (loop->mode == CYMO_SIMULATOR)
        return simulator_dequeue(loop);
    else
        return realtime_dequeue(loop);
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

static void cymo_clear(cymo_t* loop)
{
    free(loop->saved_event);
    loop->saved_event = NULL;

    for (int i = 0; i < 2; i++)
        reminder_queue_clear(&loop->clock_queue[i]);

    for (int i = 0; i < 4; i++)
        event_pipe_clear(&loop->pipe[i]);

    loop->is_simulation_stop = false;
}

static void cymo_main(void* arg)
{
    cymo_t* loop = (cymo_t*)arg;
    loop->status = CYMO_STATUS_RUNNING;
    while (!loop->quit) {
        if (loop->status != CYMO_STATUS_RUNNING && (loop->status != CYMO_STATUS_PAUSED || !loop->stepping))
            cymo_sleep(1);
        else {
            event_t* e = dequeue(loop);
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

    loop->show_warnings = true;
    loop->read_reminders = true;
    loop->mode = CYMO_SIMULATOR;

    for (int i = 0; i < 4; i++)
        event_pipe_init(&loop->pipe[i]);

    for (int i = 0; i < 2; i++)
        reminder_queue_init(&loop->clock_queue[i]);

    return 0;
}

int cymo_destory(cymo_t* loop)
{
    spsc_queue_destroy(&loop->pending);
    uv_mutex_destroy(&loop->mutex);
    uv_cond_destroy(&loop->wakeup_cond);

    for (int i = 0; i < 4; i++)
        event_pipe_destory(&loop->pipe[i]);

    for (int i = 0; i < 2; i++)
        reminder_queue_destory(&loop->clock_queue[i]);

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
    cymo_clear(loop);
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
        return event_pipe_add(&loop->pipe[q->type], q);
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
    reminder_t* timer = malloc(sizeof(reminder_t));
    if (!timer)
        return -1;
    timer->callback = cb;
    timer->user_data = data;
    timer->type = CM_EVENT_REMINDER;
    timer->timestamp = time_at;

    if (reminder_queue_push(&loop->clock_queue[clock_type], timer, NULL)) {
        free(timer);
        return -2;
    }

    return 0;
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

void cymo_resume(cymo_t* loop)
{
    if (loop->status != CYMO_STATUS_RUNNING) {
        loop->status = CYMO_STATUS_RUNNING;
        cm_event_process(cm_event_new(CM_EVENT_LOOP_RESUME));
    }
}

static void _pause(datetime_t timestamp, void* user_data)
{
    cymo_pause((cymo_t*)user_data);
}

void cymo_timed_pause(cymo_t* loop, datetime_t timeout)
{
    cymo_add_reminder(loop, CM_CLOCK_LOCAL, timeout, &_pause, loop);
}

int cymo_get_clear_reminders(cymo_t* loop)
{
    return loop->clear_reminders;
}

void cymo_set_clear_reminders(cymo_t* loop, int clearReminders)
{
    loop->clear_reminders = clearReminders;
}

int cymo_get_show_warnings(cymo_t* loop) { return loop->show_warnings; }

void cymo_set_show_warnings(cymo_t* loop, int showWarings)
{
    loop->show_warnings = showWarings;
}

datetime_t cymo_get_datetime(cymo_t* loop)
{
    if (loop->mode == CYMO_REALTIME)
        return loop->clock_datetime[CYMO_LOCAL_CLOCK];
    else
        return datetime_now();
}

void cymo_set_datetime(cymo_t* loop, datetime_t time)
{
    if (loop->mode != CYMO_SIMULATOR) {
        printf("Can not set dateTime because Clock is not in the Simulation "
               "mode\n");
        return;
    }

    if (time < loop->clock_datetime[CYMO_LOCAL_CLOCK]) {
        printf("%s\n", "local clock datetime out of order");
        return;
    }

    loop->clock_datetime[CYMO_LOCAL_CLOCK] = time;
}

datetime_t cymo_get_exchange_datetime(cymo_t* loop)
{
    return loop->clock_datetime[CYMO_EXCHANGE_CLOCK];
}

void cymo_set_exchange_datetime(cymo_t* loop, datetime_t time)
{
    if (time < loop->clock_datetime[CYMO_EXCHANGE_CLOCK]) {
        printf("%s\n", "exchange clock datetime out of order");
        return;
    }
    loop->clock_datetime[CYMO_EXCHANGE_CLOCK] = time;
}
