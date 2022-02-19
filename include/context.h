/**
 * context.h
 *
 * Copyright (c) 2019 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef CONTEXT_H
#define CONTEXT_H

#include "common.h"
#include "config.h"
#include "hook.h"
#include "keymap.h"
#include "option.h"


typedef struct {
  bool config_loading;
  bool initialized;
} State;

typedef struct {
  GtkWindow* window;
  VteTerminal* vte;
  GtkBox* hbox;
  GtkBox* vbox;
  int uri_tag;
  bool alpha_supported;
} Layout;

typedef struct {
  unsigned id;
  Meta* meta; /* ref */
  Option* option; /* ref */
  Config* config;
  Keymap* keymap;
  Hook* hook;
  GdkDevice* device;
  lua_State* lua;
  Layout layout;
  State state;
} Context;


Context* context_init(unsigned id, Meta* meta, Option* option);
void context_close(Context* context);
int context_start(Context* context, int argc, char **argv);
void context_load_device(Context* context);
void context_load_lua_context(Context* context);
void context_restore_default(Context* context);
void context_override_by_option(Context* context);
char* context_acquire_config_path(Context* context);
char* context_acquire_theme_path(Context* context);
void context_load_config(Context* context);
void context_load_theme(Context* context);
bool context_perform_keymap(Context* context, unsigned key, GdkModifierType mod);
void context_handle_signal(Context* context, const char* signal_name, GVariant* parameters);
void context_build_layout(Context* context);
void context_notify(Context* context, const char* body, const char* title);
void context_launch_uri(Context* context, const char* uri);
GdkWindow* context_get_gdk_window(Context* context);
const char* context_get_str(Context* context, const char* key);
int context_get_int(Context* context, const char* key);
bool context_get_bool(Context* context, const char* key);
void context_set_str(Context* context, const char* key, const char* value);
void context_set_int(Context* context, const char* key, int value);
void context_set_bool(Context* context, const char* key, bool value);

#endif
