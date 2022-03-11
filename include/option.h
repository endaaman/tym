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
  GHashTable* entries_as_table;
} Option;

void* option_get(Option* option, const char* key);

Option* option_init(GOptionEntry* entries);
void option_close(Option* option);
bool option_parse(Option* option, int argc, char** argv);
char* option_get_str(Option* option, const char* key);
int option_get_int(Option* option, const char* key);
bool option_get_bool(Option* option, const char* key);

#endif
