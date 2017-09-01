/**
 * embeded.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "embedded.h"
#include "builtin.h"


static const char* LIB_NAME = "tym";
static const char* DEFAULT_NOTIFICATION_TITLE = "tym";

static int get_version(lua_State* l)
{
  lua_pushstring(l, PACKAGE_VERSION);
  return 1;
}

static int get_config_file_path(lua_State* l)
{
  Context* context = (Context*)lua_touserdata(l, lua_upvalueindex(1));

  lua_pushstring(l, context->config_file_path);
  return 1;
}

static int notify(lua_State* l)
{
  Context* context = (Context*)lua_touserdata(l, lua_upvalueindex(1));
  GtkApplication* app = context->app;

  const char* body = luaL_checkstring(l, 1);
  const char* title = lua_tostring(l, 2);

  GNotification* notification = g_notification_new(title ? title : DEFAULT_NOTIFICATION_TITLE);
  GIcon* icon = g_themed_icon_new("terminal");

  g_notification_set_icon (notification, G_ICON (icon));
  g_notification_set_body(notification, body);
  g_application_send_notification(G_APPLICATION(app), "lunch-is-ready", notification);

  g_object_unref(notification);
  g_object_unref(icon);

  return 0;
}

static int put(lua_State* l)
{
  Context* context = (Context*)lua_touserdata(l, lua_upvalueindex(1));
  VteTerminal* vte = context->vte;

  const char* text = luaL_checkstring(l, -1);
  vte_terminal_feed_child(vte, text, -1);
  return 1;
}

static int reload(lua_State* l)
{
  Context* context = (Context*)lua_touserdata(l, lua_upvalueindex(1));
  if (!context->config_file_path) {
    return 0;
  }
  if (context->running_in_keybinding) {
    g_print("warning: this function must be called by shortcut key event.");
    return 0;
  }
  builtin_reload(context);
  return 0;
}

static int copy_clipboard(lua_State* l)
{
  Context* context = (Context*)lua_touserdata(l, lua_upvalueindex(1));
  builtin_reload(context);
  return 0;
}

static int paste_clipboard(lua_State* l)
{
  Context* context = (Context*)lua_touserdata(l, lua_upvalueindex(1));
  builtin_paste_clipboard(context);
  return 0;
}

static int increase_font_scale(lua_State* l)
{
  Context* context = (Context*)lua_touserdata(l, lua_upvalueindex(1));
  builtin_increase_font_scale(context);
  return 0;
}

static int decrease_font_scale(lua_State* l)
{
  Context* context = (Context*)lua_touserdata(l, lua_upvalueindex(1));
  builtin_decrease_font_scale(context);
  return 0;
}

static int reset_font_scale(lua_State* l)
{
  Context* context = (Context*)lua_touserdata(l, lua_upvalueindex(1));
  builtin_reset_font_scale(context);
  return 0;
}

void context_embed_functions(Context* context)
{
  const luaL_Reg BUILTIN_FUNCTIONS[] = {
    { "get_version", get_version },
    { "get_config_file_path", get_config_file_path },
    { "notify", notify },
    { "put", put },
    { "reload", reload },
    { "increase_font_scale", increase_font_scale },
    { "decrease_font_scale", decrease_font_scale },
    { "reset_font_scale", reset_font_scale },
    { "copy_clipboard", copy_clipboard },
    { "paste_clipboard", paste_clipboard },
    { NULL, NULL },
  };

  lua_State* l = context->lua;
  luaL_newlibtable(l, BUILTIN_FUNCTIONS);
  lua_pushlightuserdata(l, context);
  luaL_setfuncs(l, BUILTIN_FUNCTIONS, 1);
  lua_setglobal(l, LIB_NAME);
}
