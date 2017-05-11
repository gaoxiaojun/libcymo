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
typedef enum { kData, kExecution, kService, kHistorical } EventQueueId;
typedef enum { kMaster, kSlave } EventQueueType;
typedef enum { kHighest, kHigh, kNormal, kLow, kLowest } EventQueuePriority;

typedef struct cm_queue_s {
    EventQueueId id;
    EventQueueType type;
    EventQueuePriority priority;
    char *name;
    //event_bus_t *bus;
    void *bus;
    void *loop;
    int is_synced;
    spsc_queue_t spsc;
}cm_queue_t;

typedef struct event_bus_s event_bus_t;

cm_queue_t *cm_queue_new(EventQueueId id, EventQueueType type,
			 EventQueuePriority priority, unsigned int size,
			 event_bus_t *bus);
void cm_queue_free(cm_queue_t *q);

int cm_queue_init(cm_queue_t *q, EventQueueId id, EventQueueType type,
		  EventQueuePriority priority, unsigned int size,
		  event_bus_t *bus);
int cm_queue_destory(cm_queue_t *q);
void cm_queue_clear(cm_queue_t *q);

EventQueueId cm_queue_get_id(cm_queue_t *q);
EventQueueType cm_queue_get_type(cm_queue_t *q);
EventQueuePriority cm_queue_get_priority(cm_queue_t *q);
char *cm_queue_get_name(cm_queue_t *q);
int cm_queue_is_synched(cm_queue_t *q);
void cm_queue_set_synched(cm_queue_t *q, int is_synched);

/* if queue is full, push will return false */
int cm_queue_push(cm_queue_t *q, event_t *);

/* if queue is empty, pop will return false */
int cm_queue_pop(cm_queue_t *q, event_t **);

/* if queue is empty, peek will return null pointer */
event_t *cm_queue_peek(cm_queue_t *q);

int cm_queue_is_empty(cm_queue_t *q);
int cm_queue_is_full(cm_queue_t *q);

/* If called by consumer, then true size may be more (because producer may
   be adding items concurrently).
   If called by producer, then true size may be less (because consumer may
   be removing items concurrently).
   It is undefined to call this from any other thread.
*/
unsigned int cm_queue_size(cm_queue_t *q);
unsigned int cm_queue_capacity(cm_queue_t *q);

void cm_queue_set_bus(cm_queue_t *q, event_bus_t *bus);
event_bus_t *cm_queue_get_bus(cm_queue_t *q);

#endif // CTF_EVENT_QUEUE_H
