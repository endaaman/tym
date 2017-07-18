/**
 * config.h
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <vte/vte.h>

typedef struct {
  char* file_path;
  GHashTable* context;
} Config;

Config* config_init(const char* file_path);
void config_close(Config* config);

char* config_get_str(Config* c, const char* key);

void config_load(Config* c);
void config_apply_all(Config* c, VteTerminal* vte, bool is_startup);

#endif
