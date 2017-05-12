/******************************************************************************
 * Quantitative Trading Library                                               *
 *                                                                            *
 * Copyright (C) 2017 Xiaojun Gao                                             *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 ******************************************************************************/

#ifndef CTF_EVENT_QUEUE_H
#define CTF_EVENT_QUEUE_H

#include "cymo_event.h"
#include <stdbool.h>
#include "spsc_queue.h"

/*
 * lock-free spsc event queue
 */
typedef enum { CM_DATA_QUEUE, CM_EXECUTION_QUEUE, CM_SERVICE_QUEUE, CM_HISTORICAL_QUEUE } cm_queue_type;

typedef struct cm_queue_s {
    cm_queue_type type;
    char *name;

    /* private */
    void *loop;
    spsc_queue_t spsc;
}cm_queue_t;

typedef struct event_bus_s event_bus_t;

cm_queue_t *cm_queue_new(cm_queue_type type, unsigned int size);
void cm_queue_free(cm_queue_t *q);

int cm_queue_init(cm_queue_t *q, cm_queue_type type, unsigned int size);
int cm_queue_destory(cm_queue_t *q);
void cm_queue_clear(cm_queue_t *q);

cm_queue_type cm_queue_get_type(cm_queue_t *q);
char *cm_queue_get_name(cm_queue_t *q);

/* if queue is full, push will return -1
 * 0 means successful
 */
int cm_queue_push(cm_queue_t *q, event_t *);

/* if queue is empty, pop will return -1
 * 0 means successful
 */
int cm_queue_pop(cm_queue_t *q, event_t **);

/* if queue is empty, peek will return null pointer */
event_t *cm_queue_peek(cm_queue_t *q);

/* return 1 means queue is empty
 * 0 means queue NOT empty
 */
int cm_queue_is_empty(cm_queue_t *q);

/* return 1 means queue is full
 * 0 means queue NOT full
 */
int cm_queue_is_full(cm_queue_t *q);

/* If called by consumer, then true size may be more (because producer may
   be adding items concurrently).
   If called by producer, then true size may be less (because consumer may
   be removing items concurrently).
   It is undefined to call this from any other thread.
*/
unsigned int cm_queue_size(cm_queue_t *q);


unsigned int cm_queue_capacity(cm_queue_t *q);

#endif // CTF_EVENT_QUEUE_H
