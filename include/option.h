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
  GOptionEntry* entries;
  bool version;
  char* config_path;
  char* signal;
  GVariantDict* values;
} Option;


Option* option_init(Meta* meta);
void option_close(Option* option);
void option_set_values(Option* option, GVariantDict* values);
int option_process(Option* option);
bool option_get_str_value(Option* option, const char* key, char** value);
bool option_get_int_value(Option* option, const char* key, int* value);
bool option_get_bool_value(Option* option, const char* key, bool* value);
bool option_get_version(Option* option);
char* option_get_config_path(Option* option);
char* option_get_signal(Option* option);

#endif
