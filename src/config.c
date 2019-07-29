/**
 * config.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "tym.h"


typedef enum {
  VTE_CJK_WIDTH_NARROW = 1,
  VTE_CJK_WIDTH_WIDE = 2
} VteCjkWidth;

#define TYM_FALL_BACK_SHELL "/bin/sh"
#define TYM_CURSOR_SHAPE_BLOCK "block"
#define TYM_CURSOR_SHAPE_IBEAM "ibeam"
#define TYM_CURSOR_SHAPE_UNDERLINE "underline"
#define TYM_CURSOR_BLINK_MODE_SYSTEM "system"
#define TYM_CURSOR_BLINK_MODE_ON "on"
#define TYM_CURSOR_BLINK_MODE_OFF "off"
#define TYM_CJK_WIDTH_NARROW "narrow"
#define TYM_CJK_WIDTH_WIDE "wide"

static const char* TYM_DEFAULT_TITLE = "tym";
static const char* TYM_DEFAULT_ICON = "utilities-terminal";
static const char* TYM_DEFAULT_TERM = "xterm-256color";
static const char* TYM_DEFAULT_CURSOR_SHAPE = TYM_CURSOR_SHAPE_BLOCK;
static const char* TYM_DEFAULT_CURSOR_BLINK_MODE = TYM_CURSOR_BLINK_MODE_SYSTEM;
static const char* TYM_DEFAULT_CJK = TYM_CJK_WIDTH_NARROW;
static const int TYM_DEFAULT_WIDTH = 80;
static const int TYM_DEFAULT_HEIGHT = 22;
static const int TYM_DEFAULT_SCROLLBACK = 512;


static char* get_default_shell()
{
  const char* shell_env = g_getenv("SHELL");
  if (shell_env) {
    return g_strdup(shell_env);
  }
  char* user_shell = vte_get_user_shell();
  if (user_shell) {
    return user_shell;
  }
  return g_strdup(TYM_FALL_BACK_SHELL);
}

static void free_config_field(void* data) {
  g_free(((ConfigField*)data)->default_value);
  g_free(data);
}

static GHashTable* config_fields = NULL;

__attribute__((constructor))
static void initialize() {
  char* default_theme_path = g_build_path(
    G_DIR_SEPARATOR_S,
    g_get_user_config_dir(),
    TYM_CONFIG_DIR_NAME,
    TYM_THEME_FILE_NAME,
    NULL
  );
#define color_special(name) { ("color_" name), 0, T_STR, F_N, sdup(""), "", ("value of color_" name ), }
#define color_normal(name) { ("color_" name), 0, T_STR, F_HIDDEN, sdup(""), NULL, NULL, }
  const ConfigType T_STR = CONFIG_TYPE_STRING;
  const ConfigType T_INT = CONFIG_TYPE_INTEGER;
  const ConfigType T_BOOL = CONFIG_TYPE_BOOLEAN;
  const ConfigType T_NONE = CONFIG_TYPE_NONE;
  const GOptionFlags F_N = G_OPTION_FLAG_NONE;
  const GOptionFlags F_HIDDEN = G_OPTION_FLAG_HIDDEN;

  char* (*sdup)(const char*) = g_strdup;
  void* (*mdup)(const void*, unsigned) = g_memdup;
  const bool v_false = false;
  const int v_zero = 0;

  // name, short, type, group, flag, default, desc
  ConfigField c[] = {
    { "theme",               't', T_STR,  F_N, default_theme_path, "<path>", "<path> to theme file. Set '" TYM_SYMBOL_NONE "' to start without loading theme.", },
    { "shell",               'e', T_STR,  F_N, get_default_shell(), "<shell path>", "Shell to be used", },
    { "title",                 0, T_STR,  F_N, sdup(TYM_DEFAULT_TITLE), "", "Window title", },
    { "font",                  0, T_STR,  F_N, sdup(""), "", "Font to render(e.g. 'Ubuntu Mono 12')", },
    { "icon",                  0, T_STR,  F_N, sdup(TYM_DEFAULT_ICON), "", "Name of icon", },
    { "cursor_shape",          0, T_STR,  F_N, sdup(TYM_DEFAULT_CURSOR_SHAPE), "", "'block', 'ibeam' or 'underline'", },
    { "cursor_blink_mode",     0, T_STR,  F_N, sdup(TYM_DEFAULT_CURSOR_BLINK_MODE), "", "'system', 'on' or 'off'", },
    { "term",                  0, T_STR,  F_N, sdup(TYM_DEFAULT_TERM), "$TERM", "Value to override $TERM", },
    { "role",                  0, T_STR,  F_N, sdup(""), "", "Unique identifier for the window", },
    { "cjk_width",             0, T_STR,  F_N, sdup(TYM_DEFAULT_CJK), "", "'narrow' or 'wide'", },
    { "width",                 0, T_INT,  F_N, mdup(&TYM_DEFAULT_WIDTH, sizeof(int)), "<int>", "Initial columns", },
    { "height",                0, T_INT,  F_N, mdup(&TYM_DEFAULT_HEIGHT, sizeof(int)), "<int>", "Initial rows", },
    { "padding_horizontal",    0, T_INT,  F_N, mdup(&v_zero, sizeof(int)), "<int>", "Horizontal padding", },
    { "padding_vertical",      0, T_INT,  F_N, mdup(&v_zero, sizeof(int)), "<int>", "Vertical padding", },
    { "scrollback_length",     0, T_INT,  F_N, mdup(&TYM_DEFAULT_SCROLLBACK, sizeof(int)), "<int>", "Scrollback buffer length", },
    { "transparent",           0, T_BOOL, F_N, mdup(&v_false, sizeof(bool)), NULL, "Whether to disable drawing terminal background",  },
    { "ignore_default_keymap", 0, T_BOOL, F_N, mdup(&v_false, sizeof(bool)), NULL, "Whether to use default keymap", },
    { "ignore_bold",           0, T_BOOL, F_N, mdup(&v_false, sizeof(bool)), NULL, "Whether to ignore bold style", },
    { "autohide",              0, T_BOOL, F_N, mdup(&v_false, sizeof(bool)), NULL, "Whether to hide mouse cursor when key is pressed", },
    { "silent",                0, T_BOOL, F_N, mdup(&v_false, sizeof(bool)), NULL, "Whether to beep when bell sequence is sent", },
    color_special("window_background"),
    color_special("foreground"),
    color_special("background"),
    color_special("cursor"),
    color_special("cursor_foreground"),
    color_special("highlight"),
    color_special("highlight_foreground"),
    color_special("bold"),
    color_normal("0"),  color_normal("1"),  color_normal("2"),  color_normal("3"),
    color_normal("4"),  color_normal("5"),  color_normal("6"),  color_normal("7"),
    color_normal("8"),  color_normal("9"),  color_normal("10"), color_normal("11"),
    color_normal("12"), color_normal("13"), color_normal("14"), color_normal("15"),
    { "color_1..15", 0, T_NONE, F_N, NULL, "", "value of color from color_1 to color_15", },
  };
#undef color_special
#undef color_normal

  config_fields = g_hash_table_new_full(
    g_str_hash,
    g_str_equal,
    NULL,
    (GDestroyNotify)free_config_field
  );
  unsigned i = 0;
  unsigned config_fields_len = sizeof(c) / sizeof(ConfigField);
  while (i < config_fields_len) {
    g_hash_table_insert(config_fields, c[i].name, g_memdup(&c[i], sizeof(c[i])));
    i++;
  }
}

__attribute__((destructor))
static void finalize() {
  g_hash_table_destroy(config_fields);
}

GHashTable* get_config_fields() {
  return config_fields;
}

unsigned get_config_fields_count() {
  return g_hash_table_size(config_fields);
}

ConfigField* get_config_field(const char* key) {
  ConfigField* field = (ConfigField*)g_hash_table_lookup(config_fields, key);
  if (!field) {
    return NULL;
  }
  ConfigType t= field->type;
  if (t == CONFIG_TYPE_STRING || t == CONFIG_TYPE_INTEGER || t == CONFIG_TYPE_BOOLEAN) {
    return field;
  }
  return NULL;
}

static VteCursorShape match_cursor_shape(const char* str)
{
  if (0 == g_strcmp0(str, TYM_CURSOR_SHAPE_IBEAM)) {
    return VTE_CURSOR_SHAPE_IBEAM;
  }
  if (0 == g_strcmp0(str, TYM_CURSOR_SHAPE_UNDERLINE)) {
    return VTE_CURSOR_SHAPE_UNDERLINE;
  }
  return VTE_CURSOR_SHAPE_BLOCK;
}

static VteCursorBlinkMode match_cursor_blink_mode(const char* str)
{
  if (0 == g_strcmp0(str, TYM_CURSOR_BLINK_MODE_ON)) {
    return VTE_CURSOR_BLINK_ON;
  }
  if (0 == g_strcmp0(str, TYM_CURSOR_BLINK_MODE_OFF)) {
    return VTE_CURSOR_BLINK_OFF;
  }
  return VTE_CURSOR_BLINK_SYSTEM;
}

static unsigned match_cjk_width(const char* str)
{
  if (0 == g_strcmp0(str, TYM_CJK_WIDTH_WIDE)) {
    return VTE_CJK_WIDTH_WIDE;
  }
  return VTE_CJK_WIDTH_NARROW;
}

Config* config_init()
{
  Config* config = g_malloc0(sizeof(Config));
  config->data = g_hash_table_new_full(
    g_str_hash,
    g_str_equal,
    (GDestroyNotify)g_free,
    (GDestroyNotify)g_free
  );
  config_reset(config);
  return config;
}

void config_close(Config* config)
{
  g_hash_table_destroy(config->data);
  g_free(config);
}

static void* config_get_raw(Config* config, const char* key)
{
  void* ptr = g_hash_table_lookup(config->data, key);
  if (!ptr) {
    dd("tried to refer null field: '%s'", key);
  }
  return ptr;
}

static void config_add_raw(Config* config, const char* key, void* value)
{
  void* ptr = g_hash_table_lookup(config->data, key);
  if (ptr) {
    dd("tried to overwrite field: '%s'", key);
    return;
  }
  g_hash_table_insert(config->data, g_strdup(key), value);
}

static bool config_set_raw(Config* config, const char* key, void* value)
{
  if (!value) {
    dd("tried to set null field: '%s'", key);
    return false;
  }
  void* old_key = NULL;
  bool has_value = g_hash_table_lookup_extended(config->data, key, &old_key, NULL);
  if (!has_value) {
    dd("tried to add new field: '%s'", key);
    return false;
  }
  g_hash_table_remove(config->data, old_key);
  g_hash_table_insert(config->data, g_strdup(key), value);
  return true;
}

bool config_has_str(Config* config, const char* key)
{
  char* value = (char*)config_get_raw(config, key);
  if (!value) {
    return false;
  }
  if (0 == g_strcmp0(value, "")) {
    return false;
  }
  return true;
}

static void config_add_str(Config* config, const char* key, const char* value)
{
  config_add_raw(config, key, g_strdup(value));
}

bool config_set_str(Config* config, const char* key, const char* value)
{
  return config_set_raw(config, key, g_strdup(value));
}

char* config_get_str(Config* config, const char* key)
{
  char* v = (char*)config_get_raw(config, key);
  if (v) {
    return v;
  }
  dd("string config of '%s' is null. falling back to \"\"", key);
  return "";
}

int config_get_int(Config* config, const char* key)
{
  int* v = (int*)config_get_raw(config, key);
  if (v) {
    return *v;
  }
  dd("int config of '%s' is null. falling back to 0", key);
  return 0;
}

bool config_is_none(Config* config, const char* key)
{
  char* v = (char*)config_get_raw(config, key);
  if (!v) {
    return false;
  }
  return 0 == g_strcmp0(v, TYM_SYMBOL_NONE);
}

static void config_add_int(Config* config, const char* key, int value)
{
  config_add_raw(config, key, g_memdup((gpointer)&value, sizeof(int)));
}

bool config_set_int(Config* config, const char* key, int value)
{
  return config_set_raw(config, key, g_memdup((gpointer)&value, sizeof(int)));
}

bool config_get_bool(Config* config, const char* key)
{
  bool* v = (bool*)config_get_raw(config, key);
  if (v) {
    return *v;
  }
  dd("bool config of '%s' is null. falling back to null", key);
  return false;
}

static void config_add_bool(Config* config, const char* key, bool value)
{
  config_add_raw(config, key, g_memdup((gpointer)&value, sizeof(bool)));
}

bool config_set_bool(Config* config, const char* key, bool value)
{
  return config_set_raw(config, key, g_memdup((gpointer)&value, sizeof(bool)));
}

void config_reset(Config* config)
{
  df();
  g_hash_table_remove_all(config->data);

  GHashTableIter iter;
  char* key = NULL;
  ConfigField* field = NULL;
  g_hash_table_iter_init(&iter, config_fields);
  while (g_hash_table_iter_next(&iter, (void*)&key, (void*)&field)) {
    switch (field->type) {
      case CONFIG_TYPE_STRING:
        config_add_str(config, field->name, field->default_value);
        break;
      case CONFIG_TYPE_INTEGER:
        config_add_int(config, field->name, *(int*)(field->default_value));
        break;
      case CONFIG_TYPE_BOOLEAN:
        config_add_bool(config, field->name, *(bool*)(field->default_value));
        break;
      case CONFIG_TYPE_NONE:
        break;
    }
  }
}

void config_override_by_option(Config* config, Option* option)
{
  if (!option->values) {
    dd("option->values is NULL.");
    return;
  }
  GHashTableIter iter;
  char* key = NULL;
  ConfigField* field = NULL;
  g_hash_table_iter_init(&iter, config_fields);
  while (g_hash_table_iter_next(&iter, (void*)&key, (void*)&field)) {
    char* key = field->name;
    switch (field->type) {
      case CONFIG_TYPE_STRING: {
        char* v = NULL;
        bool has_value = option_get_str_value(option, key, &v);
        if (has_value) {
          config_set_str(config, key, v);
        }
        break;
      }
      case CONFIG_TYPE_INTEGER: {
        int v = 0;
        bool has_value = option_get_int_value(option, key, &v);
        if (has_value) {
          config_set_int(config, key, v);
        }
        break;
      }
      case CONFIG_TYPE_BOOLEAN: {
        bool v = false;
        bool has_value = option_get_bool_value(option, key, &v);
        if (has_value) {
          config_set_bool(config, key, v);
        }
        break;
      }
      case CONFIG_TYPE_NONE:
        break;
    }
  }
}

bool config_acquire_color(Config* config, const char* key, GdkRGBA* color)
{
  if (!config_has_str(config, key)) {
    return false;
  }
  if (config_is_none(config, key)) {
    return false;
  }
  const char* value = config_get_str(config, key);
  bool valid = gdk_rgba_parse(color, value);
  if (!valid) {
    g_message("Invalid color string for '%s': %s", key, value);
  }
  return valid;
}

VteCursorShape config_get_cursor_shape(Config* config)
{
  return match_cursor_shape(config_get_str(config, "cursor_shape"));
}

VteCursorBlinkMode config_get_cursor_blink_mode(Config* config)
{
  return match_cursor_blink_mode(config_get_str(config, "cursor_shape"));
}

unsigned config_get_cjk_width(Config* config)
{
  return match_cjk_width(config_get_str(config, "cjk_width"));
}
