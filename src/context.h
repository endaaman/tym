/**
 * context.h
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef CONTEXT_H
#define CONTEXT_H

#include "common.h"
#include "config.h"
#include "keymap.h"

typedef struct {
  GtkApplication* app;
  VteTerminal* vte;
  lua_State* lua;
  char* config_file_path;
  Config* config;
  Keymap* keymap;
  bool loading;
} Context;


Context* context_init(const char* config_file_path, GtkApplication* app, VteTerminal* vte);
void context_close(Context* context);
void context_load_config(Context* context);
bool context_on_key(Context* context, unsigned key, GdkModifierType mod);

#endif
