/**
 * embedded.h
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef EMBEDDED_H
#define EMBEDDED_H

#include "common.h"


int builtin_get_version(lua_State* l);
int builtin_get_config_file_path(lua_State* l);
int builtin_notify(lua_State* l);
int builtin_put(lua_State* l);
int builtin_reload(lua_State* l);
int builtin_copy_clipboard(lua_State* l);
int builtin_paste_clipboard(lua_State* l);
int builtin_increase_font_scale(lua_State* l);
int builtin_decrease_font_scale(lua_State* l);
int builtin_reset_font_scale(lua_State* l);

#endif
