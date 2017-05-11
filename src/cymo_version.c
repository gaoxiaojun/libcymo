/******************************************************************************
 * Quantitative Trading Library                                               *
 *                                                                            *
 * Copyright (C) 2017 Xiaojun Gao                                             *
 *                                                                            *
 * Distributed under the terms and conditions of the BSD 3-Clause License or  *
 * (at your option) under the terms and conditions of the Boost Software      *
 * License 1.0. See accompanying files LICENSE and LICENSE_ALTERNATIVE.       *
 ******************************************************************************/

#include "cymo_version.h"

#define CYMO_STRINGIFY(v) CYMO_STRINGIFY_HELPER(v)
#define CYMO_STRINGIFY_HELPER(v) #v

#define CYMO_VERSION_STRING_BASE                                               \
    CYMO_STRINGIFY(CYMO_VERSION_MAJOR)                                         \
    "." CYMO_STRINGIFY(CYMO_VERSION_MINOR) "." CYMO_STRINGIFY(                 \
	CYMO_VERSION_PATCH)

#if CYMO_VERSION_IS_RELEASE
#define CYMO_VERSION_STRING CYMO_VERSION_STRING_BASE
#else
#define CYMO_VERSION_STRING CYMO_VERSION_STRING_BASE "-" CYMO_VERSION_SUFFIX
#endif

unsigned int cm_version(void) { return CYMO_VERSION_HEX; }

const char *cm_version_string(void) { return CYMO_VERSION_STRING; }
