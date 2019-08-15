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
#include "meta.h"


typedef struct {
  Meta* meta;
  GHashTable* data;
} Config;


Config* config_init(Meta* meta);
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
