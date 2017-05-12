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

    CM_EVENT_REMINDER,
    CM_EVENT_SIMULATOR_START,
    CM_EVENT_SIMULATOR_STOP,
    CM_EVENT_SIMULATOR_PROGRESS,
    CM_EVENT_LOOP_RESUME,
    CM_EVENT_LOOP_STEP,
    CM_EVENT_LOOP_PAUSE,
    CM_EVENT_LOOP_STOPPED,
    CM_EVENT_CUSTOM_START
} event_type;

#define EVENT_PUBLIC_FIELDS \
    datetime_t timestamp;   \
    uint16_t type;          \
    atomic_ushort ref;

typedef struct event_s {
    EVENT_PUBLIC_FIELDS
} event_t;

typedef void (*event_init)(event_t* e, va_list args);
typedef void (*event_destory)(event_t* e);
typedef void (*event_default_process)(event_t* e);
typedef struct event_class_s {
    size_t size;
    event_init init;
    event_destory destory;
    event_default_process processor;
} event_class_t;

extern event_class_t event_classes[CM_EVENT_CUSTOM_START];

event_t* cm_event_new(event_type type, ...);
void cm_event_free(event_t* e);
void cm_event_process(event_t*e);
unsigned short event_ref(event_t* e);
unsigned short event_unref(event_t* e);
int event_is_single_ref(event_t* e);

typedef struct tick_s {
    EVENT_PUBLIC_FIELDS
    datetime_t exchange_timestamp;
    int16_t provider;
    int16_t instrument;
    double price;
    long size;
} tick_t;

typedef struct ask_s {
    tick_t tick;
} cm_ask_t;

typedef struct bid_s {
    tick_t tick;
} cm_bid_t;

typedef struct trade_s {
    tick_t tick;
} cm_trade_t;

typedef struct {
    EVENT_PUBLIC_FIELDS
} OnSimulationStop;

#endif // CTF_EVENT_H
