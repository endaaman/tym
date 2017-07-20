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
  bool running_in_keybinding;
} Context;


Context* context_init(const char* config_file_path);
void context_close(Context* context);

char* context_get_shell(Context* context);
void context_set_app(Context* context, GtkApplication* app);
void context_set_vte(Context* context, VteTerminal* vte);

void context_load_config(Context* context, bool is_startup);
bool context_on_key(Context* context, unsigned key, GdkModifierType mod);

#endif
