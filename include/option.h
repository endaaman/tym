/**
 * option.h
 *
 * Copyright (c) 2019 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef OPTION_H
#define OPTION_H

#include "common.h"
#include "meta.h"

typedef struct {
  GOptionContext* option_context;
  GOptionEntry* entries;
  bool version;
  bool nolua;
  char* config_path;
  char* theme_path;
  char* signal;
  GVariantDict* values;
} Option;


void* option_get(Option* option, const char* key);

Option* option_init(Meta* meta);
void option_close(Option* option);
bool option_parse(Option* option, int* argc, char*** argv);
void option_load_from_cli(Option* option, GApplicationCommandLine* cli);
bool option_get_str_value(Option* option, const char* key, const char** value);
bool option_get_int_value(Option* option, const char* key, int* value);
bool option_get_bool_value(Option* option, const char* key, bool* value);
bool option_get_version(Option* option);
char* option_get_config_path(Option* option);
char* option_get_theme_path(Option* option);
char* option_get_signal(Option* option);
bool option_get_nolua(Option* option);

#endif
