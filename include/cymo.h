/******************************************************************************
 * Quantitative Trading Library                                               *
 *                                                                            *
 * Copyright (C) 2017 Xiaojun Gao                                             *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 ******************************************************************************/

#ifndef CYMO_H
#define CYMO_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
  /* Windows - set up dll import/export decorators. */
# if defined(BUILDING_CYMO_SHARED)
    /* Building shared library. */
#   define CM_EXTERN __declspec(dllexport)
# elif defined(USING_CYMO_SHARED)
    /* Using shared library. */
#   define CM_EXTERN __declspec(dllimport)
# else
    /* Building static library. */
#   define CM_EXTERN /* nothing */
# endif
#elif __GNUC__ >= 4
# define CM_EXTERN __attribute__((visibility("default")))
#else
# define CM_EXTERN /* nothing */
#endif

#include "cymo_version.h"

#include <stddef.h>
#include <stdio.h>
#include <stdatomic.h>
#include <time.h>
#include <sys/time.h>

#if defined(_MSC_VER) && _MSC_VER < 1600
# include "stdint-msvc2008.h"
#else
# include <stdint.h>
#endif


CM_EXTERN unsigned int cm_version(void);
CM_EXTERN const char* cm_version_string(void);

/* datetime module */

/* datetime type represents a date and time.
   that stores the date and time as the number of microsecond intervals since
   12:00 AM January 1, year 1 A.D (as Day 0). in the proleptic Gregorian Calendar.
*/

typedef uint64_t datetime_t;
typedef int64_t  timespan_t;

#define EPOCH_DATE_TIME UINT64_C(62135596800000000)  /* 1970-01-01T00:00:00:00 */
#define MIN_DATE_TIME UINT64_C(0)                    /* 0001-01-01T00:00:00:00 */
#define MAX_DATE_TIME UINT64_C(315537897599999999)   /* 9999-12-31T23:59:59.999999*/

CM_EXTERN datetime_t datetime_from_ymd(uint16_t year, uint16_t month, uint16_t day);
CM_EXTERN datetime_t datetime_from_hmsu(uint16_t hour, uint16_t minute, uint16_t second, uint32_t usec);
CM_EXTERN datetime_t datetime_from_timeval(struct timeval *t);
/* result = 0 means ok, other means error */
CM_EXTERN int datetime_from_iso8601(const char *str, size_t len, datetime_t *dt);
CM_EXTERN void datetime_decode(datetime_t dt, uint16_t *year, uint16_t *month, uint16_t *day, uint16_t *hour, uint16_t *minute, uint16_t *second, uint32_t *usec);
CM_EXTERN int datetime_to_tm(const datetime_t dt, struct tm *tmp);
CM_EXTERN size_t datetime_format(char *dst, size_t len, datetime_t dt, long offset /* see timeout_offset */);
/* return timezone offset from utc, in minutes [-1439,1439] */
CM_EXTERN long get_timezone_offset(void);
CM_EXTERN void cm_timezone_update(void);
CM_EXTERN datetime_t datetime_now(void);

/* event queue */
/*typedef struct cm_queue_s cm_queue_t;
typedef struct event_bus_s event_bus_t;

CM_EXTERN cm_queue_t* cm_queue_new(size_t capacity, event_bus_t *bus);
CM_EXTERN void cm_queue_free(cm_queue_t *q);
CM_EXTERN int cm_queue_push(cm_queue_t *q, void *data);
CM_EXTERN int cm_queue_is_full(cm_queue_t *q);*/

/* event bus */
typedef enum {
    CM_BUS_DTAT_PIPE,
    CM_BUS_EXECUTION_PIPE,
    CM_BUS_HISTORICAL_PIPE,
    CM_BUS_SERVICE_PIPE
}cm_bus_pipe_type;

typedef enum {
    CM_CLOCK_LOCAL,
    CM_CLOCK_EXCHAGE
}cm_clock_type;

typedef uint64_t timer_id_t;
typedef void (*timer_cb_t)(datetime_t timestamp, void *user_data);
/*CM_EXTERN int cm_bus_add_queue(event_bus_t *bus, cm_queue_t *q, cm_bus_pipe_type which);
CM_EXTERN timer_id_t cm_bus_add_timer(event_bus_t *bus, cm_clock_type type, datetime_t, timer_cb_t callback, void *data);
CM_EXTERN int cm_bus_remove_timer(event_bus_t *bus, timer_id_t id);
CM_EXTERN datetime_t cm_bus_get_time(event_bus_t *bus, cm_clock_type type);
CM_EXTERN void cm_bus_set_time(event_bus_t *bus, cm_clock_type type, datetime_t time);

typedef struct rbtree_node_s rbtree_node_t;

typedef struct cm_cycle_s {
    atomic_flag terminate;  // force shutdown
    atomic_flag quit;       // gracefull shutdown
    atomic_flag exiting;
    rbtree_node_t *root, *sentinel;
}cm_cycle_t;

CM_EXTERN int cm_process_events_and_timers(cm_cycle_t *cycle);
*/
#ifdef __cplusplus
}
#endif
#endif // CYMO_H
