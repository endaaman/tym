/**
 * tym.h
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef TYM_H
#define TYM_H

#include "common.h"
#include "regex.h"


#define TYM_APP_ID "me.endaaman.tym"
#define TYM_CONFIG_DIR_NAME "tym"
#define TYM_CONFIG_FILE_NAME "config.lua"
#define TYM_THEME_FILE_NAME "theme.lua"
#define TYM_SYMBOL_NONE "NONE"

typedef enum {
  CONFIG_TYPE_STRING = 1,
  CONFIG_TYPE_INTEGER = 2,
  CONFIG_TYPE_BOOLEAN = 3,
  CONFIG_TYPE_NONE = 4, // not actual, only shown in help
} ConfigType;

typedef struct {
  char* name;
  char short_name;
  ConfigType type;
  GOptionFlags option_flag;
  void* default_value;
  char* arg_desc;
  char* desc;
  void* option_data; // TODO: do not store data here
} ConfigField;

typedef struct {
  GOptionEntry* entries;
  bool version;
  char* config_path;
} Option;

typedef struct {
  GHashTable* data;
} Config;

typedef struct {
  GList* entries;
} Keymap;

typedef struct {
  int title_ref;
  int bell_ref;
  int uri_ref;
  int activated_ref;
  int deactivated_ref;
} Hook;

typedef struct {
  Option* option;
  Config* config;
  Keymap* keymap;
  Hook* hook;
  GtkApplication* app;
  VteTerminal* vte;
  GtkWindow* window;
  GdkDevice* device;
  lua_State* lua;
  char* config_path;
  bool loading;
} Context;


// builtin
int builtin_register_module(lua_State* L);


// option
Option* option_init();
void option_close(Option* option);
bool option_check(Option* option, int* argc, char*** argv, GError** error);


// config
extern GList* config_fields;
extern unsigned config_fields_len;
ConfigField* get_config_field(const char* key);
Config* config_init();
void config_close(Config* config);
void config_reset(Config* config);
void config_load_option_values(Config* config, Option* option);
void config_apply(Config* config, VteTerminal* vte);
void config_apply_theme(Config* config, VteTerminal* vte);
bool config_has_str(Config* config, const char* key);
char* config_get_str(Config* config, const char* key);
bool config_set_str(Config* config, const char* key, const char* value);
int config_get_int(Config* config, const char* key);
bool config_set_int(Config* config, const char* key, int value);
bool config_get_bool(Config* config, const char* key);
bool config_set_bool(Config* config, const char* key, bool value);
char* config_acquire_theme_path(Config* config);


// keymap
Keymap* keymap_init();
void keymap_close(Keymap* keymap);
void keymap_reset(Keymap* keymap);
bool keymap_add_entry(Keymap* keymap, const char* acceralator, int ref);
bool keymap_remove_entry(Keymap* keymap, const char* acceralator);
void keymap_prepare(Keymap* keymap, lua_State* L);
void keymap_load(Keymap* keymap, lua_State* L, char** error);
bool keymap_perform(Keymap* keymap, lua_State* L, unsigned key, GdkModifierType mod, char** error);


// hook
Hook* hook_init();
void hook_close(Hook* hook);
bool hook_set_ref(Hook* hook, const char* name, int ref);
bool hook_perform_title(Hook* hook, lua_State* L, const char* title, char** next_title);
bool hook_perform_bell(Hook* hook, lua_State* L);
bool hook_perform_activated(Hook* hook, lua_State* L);
bool hook_perform_deactivated(Hook* hook, lua_State* L);


// context
Context* context_init();
void context_close(Context* context);
int context_start(Context* context, int argc, char **argv);
void context_prepare_componets(Context* context);
void context_load_config(Context* context);
void context_load_theme(Context* context);
bool context_perform_keymap(Context* context, unsigned key, GdkModifierType mod);
void context_apply_config(Context* context);
void context_apply_theme(Context* context);
void context_notify(Context* context, const char* body, const char* title);


// command
void command_reload(Context* context);
void command_copy_clipboard(Context* context);
void command_paste_clipboard(Context* context);
void command_increase_font_scale(Context* context);
void command_decrease_font_scale(Context* context);
void command_reset_font_scale(Context* context);


// app
void on_open(GtkApplication* app, GFile** files, int n, const char* hint, void* user_data);
void on_activate(GtkApplication* app, void* user_data);

#endif
