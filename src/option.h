/**
 * option.h
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef OPTION_H
#define OPTION_H

#include "common.h"


extern const char* OPTION_CONFIG_PREFIX;

typedef struct {
  GOptionEntry* entries;
  bool version;
  char* config_file_path;
  unsigned idx_config_str;
  char** config_str;
  unsigned idx_config_int;
  int* config_int;
  unsigned idx_config_bool;
  char** config_bool;
} Option;

Option* option_init();
void option_close(Option* option);
bool option_check(Option* option, int* argc, char*** argv, GError** error);

#endif
