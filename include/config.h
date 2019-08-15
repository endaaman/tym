/**
 * config.h
 *
 * Copyright (c) 2019 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "common.h"
#include "option.h"


typedef enum {
  CONFIG_TYPE_STRING = 0,
  CONFIG_TYPE_INTEGER = 1,
  CONFIG_TYPE_BOOLEAN = 2,
  CONFIG_TYPE_NONE = 3, // not actual, only shown in help
} ConfigType;

typedef struct {
  char* name;
  char short_name;
  ConfigType type;
  GOptionFlags option_flag;
  void* default_value;
  char* arg_desc;
  char* desc;
  unsigned index;
} ConfigField;

typedef struct {
  GHashTable* data;
} Config;


GHashTable* get_config_fields();
GList* get_config_fields_as_list(bool sorted);
unsigned get_config_fields_count();
ConfigField* get_config_field(const char* key);
Config* config_init();
void config_close(Config* config);
void config_reset(Config* config);
bool config_has_str(Config* config, const char* key);
char* config_get_str(Config* config, const char* key);
bool config_set_str(Config* config, const char* key, const char* value);
int config_get_int(Config* config, const char* key);
bool config_is_none(Config* config, const char* key);
bool config_set_int(Config* config, const char* key, int value);
bool config_get_bool(Config* config, const char* key);
bool config_set_bool(Config* config, const char* key, bool value);
void config_override_by_option(Config* config, Option* option);
bool config_acquire_color(Config* config, const char* key, GdkRGBA* color);
VteCursorShape config_get_cursor_shape(Config* config);
VteCursorBlinkMode config_get_cursor_blink_mode(Config* config);
unsigned config_get_cjk_width(Config* config);

#endif
