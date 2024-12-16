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
  GtkWindow* window;
  VteTerminal* vte;
  GtkBox* hbox;
  GtkBox* vbox;
  int uri_tag;
  bool alpha_supported;
} Layout;

typedef struct {
  void* object;
  int handler_id;
} HandlerTag;

typedef struct {
  int id;
  bool config_loading;
  bool initialized;
  char* object_path;
  int registration_id;
  int child_pid;
  GList* handler_tags;
  Option* option;
  Config* config;
  Keymap* keymap;
  Hook* hook;
  GdkDevice* device;
  lua_State* lua;
  Layout layout;
} Context;



#define context_signal_connect(context, instance, detailed_signal, c_handler) {\
  context_add_handler_tag(context, instance, g_signal_connect(instance, detailed_signal, c_handler, context)); \
}

Context* context_init(int id, Option* option);
// void context_dispose_only(Context* context);
void context_close(Context* context);
void context_add_handler_tag(Context* context, void* object, int handler_id);
void context_load_device(Context* context);
void context_load_lua_context(Context* context);
void context_log_message(Context* context, bool notify, const char* fmt, ...);
void context_log_warn(Context* context, bool notify, const char* fmt, ...);
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
void context_resize(Context* context, int width, int height);

#endif
