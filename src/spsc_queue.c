/******************************************************************************
 * Quantitative Trading Library                                               *
 *                                                                            *
 * Copyright (C) 2017 Xiaojun Gao                                             *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 ******************************************************************************/

#include <assert.h>
#include <stdlib.h>

#include <stdatomic.h>
#include <stdint.h>

#include "spsc_queue.h"

int spsc_queue_init(spsc_queue_t* q, unsigned int size)
{
    assert(size >= 2);
    q->records = malloc(size * sizeof(void*));
    if (!q->records)
        return -1;

    q->capacity = size;
    q->read_pos = 0;
    q->write_pos = 0;

    return 0;
}

void spsc_queue_destroy(spsc_queue_t* q) { free(q->records); }

int spsc_queue_push(spsc_queue_t* q, void* e)
{
    unsigned int currentWrite = atomic_load_explicit(&q->write_pos, memory_order_relaxed);
    unsigned int nextRecord = currentWrite + 1;
    if (nextRecord == q->capacity)
        nextRecord = 0;

    if (nextRecord != atomic_load_explicit(&q->read_pos, memory_order_acquire)) {
        q->records[currentWrite] = e;
        atomic_store_explicit(&q->write_pos, nextRecord, memory_order_release);
        return 1;
    }

    // queue if full
    return 0;
}

int spsc_queue_pop(spsc_queue_t* q, void** item)
{
    unsigned int currentRead = atomic_load_explicit(&q->read_pos, memory_order_relaxed);
    if (currentRead == atomic_load_explicit(&q->write_pos, memory_order_acquire)) {
        // queue is empty
        return 0;
    }

    unsigned int nextRecord = currentRead + 1;
    if (nextRecord == q->capacity)
        nextRecord = 0;

    *item = q->records[currentRead];
    atomic_store_explicit(&q->read_pos, nextRecord, memory_order_release);
    return 1;
}

void* spsc_queue_peek(spsc_queue_t* q)
{
    unsigned int currentRead = atomic_load_explicit(&q->read_pos, memory_order_relaxed);
    if (currentRead == atomic_load_explicit(&q->write_pos, memory_order_acquire)) {
        // queue is empty
        return NULL;
    }

    return q->records[currentRead];
}

int spsc_queue_is_empty(spsc_queue_t* q)
{
    return atomic_load_explicit(&q->read_pos, memory_order_acquire) == atomic_load_explicit(&q->write_pos, memory_order_acquire);
}

int spsc_queue_is_full(spsc_queue_t* q)
{
    unsigned int nextRecord = atomic_load_explicit(&q->write_pos, memory_order_acquire) + 1;
    if (nextRecord == q->capacity)
        nextRecord = 0;

    if (nextRecord != atomic_load_explicit(&q->read_pos, memory_order_acquire)) {
        return 0;
    }

    // queue is full
    return 1;
}

unsigned int spsc_queue_size(spsc_queue_t* q)
{
    int ret = atomic_load_explicit(&q->write_pos, memory_order_acquire) - atomic_load_explicit(&q->read_pos, memory_order_acquire);
    if (ret < 0)
        ret += q->capacity;
    return ret;
}

unsigned int spsc_queue_capacity(spsc_queue_t* q) { return q->capacity; }
