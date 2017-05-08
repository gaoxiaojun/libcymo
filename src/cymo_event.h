/******************************************************************************
 * Quantitative Trading Library                                               *
 *                                                                            *
 * Copyright (C) 2017 Xiaojun Gao                                             *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 ******************************************************************************/

#ifndef CTF_EVENT_H
#define CTF_EVENT_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <cymo.h>

typedef enum {
  CM_EVENT_ASK,
  CM_EVENT_BID,
  CM_EVENT_TRADE,

  CM_EVENT_QUEUEOPEND,
  CM_EVENT_QUEUECLOSED,

  CM_EVENT_SIMULATORSTART,
  CM_EVENT_SIMULATORSTOP,
} event_type;

#define EVENT_PUBLIC_FIELDS                                                    \
  datetime_t timestamp;                                                        \
  uint16_t type;                                                               \
  atomic_ushort ref;

typedef struct event_s { EVENT_PUBLIC_FIELDS } event_t;

static inline void event_init(event_t *e, event_type type) {
  e->type = type;
  e->ref = 1;
}

static inline datetime_t event_get_time(event_t *e) { return e->timestamp; }

static inline void event_set_time(event_t *e, datetime_t time) {
  e->timestamp = time;
}

static inline event_type event_get_type(event_t *e) {
  return (event_type)(e->type);
}

static inline unsigned short event_ref(event_t *e) {
  return atomic_fetch_add(&e->ref, 1);
}

static inline unsigned short event_unref(event_t *e) {
  unsigned short count;
  count = atomic_fetch_sub(&e->ref, 1);
  if (count == 1) {
    free(e);
  }
  return count;
}

static inline int event_is_single_ref(event_t *e) {
  return (atomic_load(&e->ref) == 1);
}

typedef struct tick_s {
  event_t head;
  int16_t provider;
  int16_t instrument;
  double price;
  long size;
} tick_t;

typedef struct _ask { tick_t tick; } ask_t;

typedef struct _bid { tick_t tick; } bid_t;

typedef struct _trade { tick_t tick; } trade_t;

static inline void tick_event_init(tick_t *e, event_type type) {
  event_init((event_t *)e, type);
  e->provider = 0;
  e->instrument = 0;
  e->price = 0;
  e->size = 0;
}

static inline void ask_event_init(ask_t *e) {
  tick_event_init((tick_t *)e, CM_EVENT_ASK);
}

static inline void bid_event_init(ask_t *e) {
  tick_event_init((tick_t *)e, CM_EVENT_BID);
}

static inline void trade_event_init(ask_t *e) {
  tick_event_init((tick_t *)e, CM_EVENT_TRADE);
}

typedef struct { EVENT_PUBLIC_FIELDS } OnSimulationStop;

static inline void on_simulation_stop_event_init(event_t *e) {
  event_init((event_t *)e, CM_EVENT_SIMULATORSTOP);
  e->timestamp = MIN_DATE_TIME;
}

static inline OnSimulationStop *new_on_simulation_stop_event() {
  OnSimulationStop *e = malloc(sizeof(OnSimulationStop));
  on_simulation_stop_event_init((event_t *)e);
  return e;
}

#endif // CTF_EVENT_H
