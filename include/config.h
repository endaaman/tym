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
  GHashTable* data;
  bool restoring;
} Config;


Config* config_init();
void config_close(Config* config);
void config_restore_default(Config* config, Meta* meta);
const char* config_get_str(Config* config, const char* key);
void config_set_str(Config* config, const char* key, const char* value);
int config_get_int(Config* config, const char* key);
void config_set_int(Config* config, const char* key, int value);
bool config_get_bool(Config* config, const char* key);
void config_set_bool(Config* config, const char* key, bool value);
VteCursorShape config_get_cursor_shape(Config* config);
VteCursorBlinkMode config_get_cursor_blink_mode(Config* config);
unsigned config_get_cjk_width(Config* config);

#endif
