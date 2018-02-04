/**
 * builtin.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "builtin.h"
#include "command.h"


int builtin_get_version(lua_State* l)
{
  lua_pushstring(l, PACKAGE_VERSION);
  return 1;
}

int builtin_get_config_file_path(lua_State* l)
{
  Context* context = (Context*)lua_touserdata(l, lua_upvalueindex(1));

  lua_pushstring(l, context->config_file_path);
  return 1;
}

int builtin_notify(lua_State* l)
{
  Context* context = (Context*)lua_touserdata(l, lua_upvalueindex(1));

  const char* body = luaL_checkstring(l, 1);
  const char* title = lua_tostring(l, 2);
  command_notify(context, body, title);
  return 0;
}

int builtin_put(lua_State* l)
{
  Context* context = (Context*)lua_touserdata(l, lua_upvalueindex(1));
  VteTerminal* vte = context->vte;

  const char* text = luaL_checkstring(l, -1);
  vte_terminal_feed_child(vte, text, -1);
  return 1;
}

int builtin_reload(lua_State* l)
{
  Context* context = (Context*)lua_touserdata(l, lua_upvalueindex(1));
  if (!context->config_file_path) {
    return 0;
  }
  command_reload(context);
  return 0;
}

int builtin_copy_clipboard(lua_State* l)
{
  Context* context = (Context*)lua_touserdata(l, lua_upvalueindex(1));
  command_copy_clipboard(context);
  return 0;
}

int builtin_paste_clipboard(lua_State* l)
{
  Context* context = (Context*)lua_touserdata(l, lua_upvalueindex(1));
  command_paste_clipboard(context);
  return 0;
}

int builtin_increase_font_scale(lua_State* l)
{
  Context* context = (Context*)lua_touserdata(l, lua_upvalueindex(1));
  command_increase_font_scale(context);
  return 0;
}

int builtin_decrease_font_scale(lua_State* l)
{
  Context* context = (Context*)lua_touserdata(l, lua_upvalueindex(1));
  command_decrease_font_scale(context);
  return 0;
}

int builtin_reset_font_scale(lua_State* l)
{
  Context* context = (Context*)lua_touserdata(l, lua_upvalueindex(1));
  command_reset_font_scale(context);
  return 0;
}
