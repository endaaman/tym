/**
 * config.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "config.h"


typedef enum {
  VTE_CJK_WIDTH_NARROW = 1,
  VTE_CJK_WIDTH_WIDE = 2
} VteCjkWidth;


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

Config* config_init(Meta* meta)
{
  Config* config = g_malloc0(sizeof(Config));
  config->meta = meta;
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

  MetaIter iter = {};
  char* key = NULL;
  MetaEntry* e = NULL;
  meta_iter_init(&iter, config->meta);
  while (meta_iter_next(&iter, &key, &e)) {
    switch (e->type) {
      case META_ENTRY_TYPE_STRING:
        config_add_str(config, e->name, e->default_value);
        break;
      case META_ENTRY_TYPE_INTEGER:
        config_add_int(config, e->name, *(int*)(e->default_value));
        break;
      case META_ENTRY_TYPE_BOOLEAN:
        config_add_bool(config, e->name, *(bool*)(e->default_value));
        break;
      case META_ENTRY_TYPE_NONE:
        break;
    }
  }
}

void config_override_by_option(Config* config, Option* option)
{
  df();
  if (!option->values) {
    dd("option->values is NULL.");
    return;
  }
  MetaIter iter = {};
  char* key = NULL;
  MetaEntry* e = NULL;
  meta_iter_init(&iter, config->meta);
  while (meta_iter_next(&iter, &key, &e)) {
    char* key = e->name;
    switch (e->type) {
      case META_ENTRY_TYPE_STRING: {
        char* v = NULL;
        bool has_value = option_get_str_value(option, key, &v);
        if (has_value) {
          config_set_str(config, key, v);
        }
        break;
      }
      case META_ENTRY_TYPE_INTEGER: {
        int v = 0;
        bool has_value = option_get_int_value(option, key, &v);
        if (has_value) {
          config_set_int(config, key, v);
        }
        break;
      }
      case META_ENTRY_TYPE_BOOLEAN: {
        bool v = false;
        bool has_value = option_get_bool_value(option, key, &v);
        if (has_value) {
          config_set_bool(config, key, v);
        }
        break;
      }
      case META_ENTRY_TYPE_NONE:
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
