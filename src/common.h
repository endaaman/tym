/**
 * common.h
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <vte/vte.h>
#include "../config.h"

#define UNUSED(x) (void)(x)

#if VTE_MAJOR_VERSION == 0
#if VTE_MINOR_VERSION >= 48
#define USE_ASYNC_SPAWN
#endif
#endif

#if VTE_MAJOR_VERSION == 0
#if VTE_MINOR_VERSION >= 50
#define USE_COPY_CLIPBOARD_FORMAT
#endif
#endif

#ifdef DEBUG
#define dd( fmt, ... ) \
  g_print("[%s:%u] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define dd( ... ) ((void)0)
#endif

#ifdef DEBUG
#define dump_stack(x) debug_dump_stack(x)
#else
#define dump_stack(...) ((void)0)
#endif

#endif
