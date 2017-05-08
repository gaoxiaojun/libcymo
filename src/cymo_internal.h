/******************************************************************************
 * Quantitative Trading Library                                               *
 *                                                                            *
 * Copyright (C) 2017 Xiaojun Gao                                             *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 ******************************************************************************/
#ifndef CM_INTERNAL_H
#define CM_INTERNAL_H

#include <cymo.h>

CM_EXTERN event_bus_t *cm_bus_new();
CM_EXTERN void cm_bus_free();

#endif // CM_INTERNAL_H
