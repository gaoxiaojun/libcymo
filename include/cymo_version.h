/******************************************************************************
 * Quantitative Trading Library                                               *
 *                                                                            *
 * Copyright (C) 2017 Xiaojun Gao                                             *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 ******************************************************************************/

#ifndef CYMO_VERSION_H
#define CYMO_VERSION_H

// Quantitative Trading Library

#define CYMO_VERSION_MAJOR 0
#define CYMO_VERSION_MINOR 0
#define CYMO_VERSION_PATCH 1
#define CYMO_VERSION_IS_RELEASE 0
#define CYMO_VERSION_SUFFIX "dev"

#define CYMO_VERSION_HEX  ((CYMO_VERSION_MAJOR << 16) | \
                         (CYMO_VERSION_MINOR <<  8) | \
                         (CYMO_VERSION_PATCH))

#endif // CYMO_VERSION_H
