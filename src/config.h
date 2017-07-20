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

typedef struct {
  GHashTable* data;
} Config;

Config* config_init();
void config_close(Config* c);
void config_reset(Config* c);
void config_prepare_lua(Config* c, lua_State* l);
void config_load_from_lua(Config* c, lua_State* l);
void config_apply_all(Config* c, VteTerminal* vte, bool is_startup);
char* config_get_shell(Config* c);
bool config_get_use_default_keymap(Config* c);


#endif
