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
  GVariantDict* values;
} Option;


typedef struct{
  bool version;
  char* signal;
} LocalOption;


void* option_get(Option* option, const char* key);

Option* option_init();
void option_close(Option* option);
void option_set_values(Option* option, GVariantDict* values);
bool option_get_str_value(Option* option, const char* key, char** value);
bool option_get_int_value(Option* option, const char* key, int* value);
bool option_get_bool_value(Option* option, const char* key, bool* value);
char* option_get_str(Option* option, const char* key);
int option_get_int(Option* option, const char* key);
bool option_get_bool(Option* option, const char* key);

#endif
