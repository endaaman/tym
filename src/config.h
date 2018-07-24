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

#include "common.h"
#include "option.h"

typedef struct {
  lua_State* lua;
  GHashTable* data;
} Config;


extern const char* fields_str[29];
extern const char* fields_int[2];
extern const char* fields_bool[2];

Config* config_init(lua_State* lua);
void config_close(Config* c);
void config_reset(Config* c);
void config_prepare(Config* c);
void config_load(Config* c, char** error);
void config_load_option(Config* c, Option* option);
void config_apply(Config* c, VteTerminal* vte);
char* config_get_shell(Config* c);
char* config_get_title(Config* c);
char* config_get_icon(Config* c);
bool config_get_use_default_keymap(Config* c);


#endif
