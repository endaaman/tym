/**
 * builtin.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "tym.h"


static int builtin_get(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));

  const char* key = luaL_checkstring(L, 1);
  ConfigField* field = get_config_field(key);
  if (!field) {
    luaX_warn(L, "Invalid config key: '%s'", key);
    lua_pushnil(L);
    return 1;
  }

  switch (field->type) {
    case CONFIG_TYPE_STRING:
      lua_pushstring(L, config_get_str(context->config, key));
      break;
    case CONFIG_TYPE_INTEGER:
      lua_pushinteger(L, config_get_int(context->config, key));
      break;
    case CONFIG_TYPE_BOOLEAN:
      lua_pushboolean(L, config_get_bool(context->config, key));
      break;
    default:
      lua_pushnil(L);
      break;
  }
  return 1;
}

static int builtin_set(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));

  const char* key = luaL_checkstring(L, 1);

  ConfigField* field = get_config_field(key);
  if (!field) {
    luaX_warn(L, "Invalid config key: '%s'", key);
    return 0;
  }

  int type = lua_type(L, 2);
  switch (field->type) {
    case CONFIG_TYPE_STRING: {
      const char* value = lua_tostring(L, 2);
      if (!value) {
        luaX_warn(L, "Invalid string config for '%s' (string expected, got %s)", key, lua_typename(L, type));
        break;
      }
      config_set_str(context->config, key, value);
      break;
    }
    case CONFIG_TYPE_INTEGER: {
      if (type != LUA_TNUMBER) {
        luaX_warn(L, "Invalid integer config for '%s': %s (number expected, got %s)", key, lua_tostring(L, 2), lua_typename(L, type));
        break;
      }
      int value = lua_tointeger(L, 2);
      config_set_int(context->config, key, value);
      break;
    }
    case CONFIG_TYPE_BOOLEAN: {
      int value = lua_toboolean(L, 2);
      config_set_bool(context->config, key, value);
      break;
    }
    default:
      break;
  }
  return 0;
}

static int builtin_get_config(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));

  lua_newtable(L);

  for (GList* li = config_fields; li != NULL; li = li->next) {
    ConfigField* field = (ConfigField*)li->data;
    char* key = field->name;
    lua_pushstring(L, key);
    switch (field->type) {
      case CONFIG_TYPE_STRING: {
        const char* value = config_get_str(context->config, key);
        lua_pushstring(L, value);
        break;
      }
      case CONFIG_TYPE_INTEGER:
        lua_pushinteger(L, config_get_int(context->config, key));
        break;
      case CONFIG_TYPE_BOOLEAN:
        lua_pushboolean(L, config_get_bool(context->config, key));
        break;
      case CONFIG_TYPE_NONE:
        lua_pop(L, 1);
        continue;
    }
    lua_settable(L, -3);
  }
  return 1;
}

static int builtin_set_config(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));

  luaL_argcheck(L, lua_istable(L, 1), 1, "table expected");

  lua_pushnil(L);
  while (lua_next(L, -2)) {
    lua_pushvalue(L, -2);
    const char* key = lua_tostring(L, -1);
    ConfigField* field = get_config_field(key);
    if (field) {
      int type = lua_type(L, -2);
      switch (field->type) {
        case CONFIG_TYPE_STRING: {
          const char* value = lua_tostring(L, -2);
          if (!value) {
            luaX_warn(L, "Invalid string config for '%s' (string expected, got %s)", key, lua_typename(L, type));
            break;
          }
          config_set_str(context->config, key, value);
          break;
        }
        case CONFIG_TYPE_INTEGER: {
          if (type != LUA_TNUMBER) {
            luaX_warn(L, "Invalid integer config for '%s': %s (number expected, got %s)", key, lua_tostring(L, -2), lua_typename(L, type));
            break;
          }
          int value = lua_tointeger(L, -2);
          config_set_int(context->config, key, value);
          break;
        }
        case CONFIG_TYPE_BOOLEAN: {
          int value = lua_toboolean(L, -2);
          config_set_bool(context->config, key, value);
          break;
        }
        default:
          break;
      }
    } else {
      luaX_warn(L, "Invalid config key: '%s'", key);
    }
    lua_pop(L, 2);
  }
  lua_pop(L, 1);

  return 0;
}

static int builtin_reset_config(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  config_reset(context->config);
  return 0;
}

static int builtin_set_keymap(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));

  const char* key = luaL_checkstring(L, 1);
  luaL_argcheck(L, lua_isfunction(L, 2), 2, "function expected");

  int ref = luaL_ref(L, LUA_REGISTRYINDEX);
  bool ok = keymap_add_entry(context->keymap, key, ref);
  if (!ok) {
    luaL_unref(L, LUA_REGISTRYINDEX, ref);
    luaL_error(L, "Invalid acceralator: '%s'", key);
    return 0;
  }
  return 0;
}

static int builtin_unset_keymap(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  const char* key = luaL_checkstring(L, 1);
  bool removed = keymap_remove_entry(context->keymap, key);
  if (!removed) {
    luaX_warn(L, "Tried to remove en empty keymap '(%s') which is not assigned function to", key);
  }
  return 0;
}

static int builtin_set_keymaps(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));

  luaL_argcheck(L, lua_istable(L, 1), 1, "table expected");

  lua_pushnil(L);
  while (lua_next(L, -2)) {
    lua_pushvalue(L, -2);
    const char* key = lua_tostring(L, -1);
    if (!lua_isfunction(L, -2)) {
      luaX_warn(L, "Invalid keymap value for '%s': function expected, got %s", key, lua_typename(L, lua_type(L, -2)));
    } else {
      lua_pushvalue(L, -2); // push function to stack top
      int ref = luaL_ref(L, LUA_REGISTRYINDEX);
      bool ok = keymap_add_entry(context->keymap, key, ref);
      if (!ok) {
        luaL_unref(L, LUA_REGISTRYINDEX, ref);
        luaX_warn(L, "Invalid acceralator: '%s'", key);
      }
    }
    lua_pop(L, 2);
  }
  lua_pop(L, 1);

  return 0;
}

static int builtin_reset_keymaps(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  keymap_reset(context->keymap);
  return 0;
}

static int builtin_send_key(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  UNUSED(context);
  const char* acceralator = luaL_checkstring(L, 1);
  unsigned key;
  GdkModifierType mod;
  gtk_accelerator_parse(acceralator, &key, &mod);
  if (0 == key && 0 == mod) {
    luaL_error(L, "Invalid acceralator: '%s'", acceralator);
    return 0;
  }
  GdkEvent* event = gdk_event_new(GDK_KEY_PRESS);
  GdkDisplay* display = gdk_display_get_default();
  GdkSeat* seat = gdk_display_get_default_seat(display);
  GdkDevice* device = gdk_seat_get_keyboard(seat);
  gdk_event_set_device(event, device);
  event->key.window = g_object_ref(gtk_widget_get_window(GTK_WIDGET(context_get_window(context))));
  event->key.send_event = false;
  event->key.time = GDK_CURRENT_TIME;
  event->key.state = mod;
  event->key.keyval = key;
  gtk_main_do_event((GdkEvent*)event);
  event->type = GDK_KEY_RELEASE;
  gtk_main_do_event((GdkEvent*)event);
  gdk_event_free((GdkEvent*)event);
  return 0;
}

/* static int builtin_set_hook(lua_State* L) */
/* { */
/*   return 0 */
/* } */

/* static int builtin_set_hooks(lua_State* L) */
/* { */
/*   return 0 */
/* } */

static int builtin_reload(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  context_load_config(context);
  context_load_theme(context);
  context_apply_config(context);
  context_apply_theme(context);
  return 0;
}

static int builtin_reload_theme(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  context_load_theme(context);
  context_apply_theme(context);
  return 0;
}

static int builtin_apply(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  context_apply_config(context);
  context_apply_theme(context);
  return 0;
}

typedef struct {
  Context* context;
  int ref;
} TimeoutNotation;

static int timeout_callback(void* user_data)
{
  TimeoutNotation* notation = (TimeoutNotation*) user_data;

  lua_State* L = notation->context->lua;
  lua_rawgeti(L, LUA_REGISTRYINDEX, notation->ref);
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 1); // pop none-function
    dd("tried to call non-function");
    return false;
  }

  if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
    luaL_error(L, "Error in timeout function: '%s'", lua_tostring(L, -1));
    lua_pop(L, 1); // error
    return false;
  }
  bool result = lua_toboolean(L, -1);
  lua_pop(L, 1);
  return result;
}

static int builtin_set_timeout(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));

  luaL_argcheck(L, lua_isfunction(L, 1), 1, "function expected");
  int interval = lua_tointeger(L, 2); // if non-number, falling back to 0

  lua_pushvalue(L, 1);
  int ref = luaL_ref(L, LUA_REGISTRYINDEX);
  TimeoutNotation* notation = g_malloc0(sizeof(TimeoutNotation));
  notation->context = context;
  notation->ref = ref;
  int tag = g_timeout_add_full(G_PRIORITY_DEFAULT, interval, (GSourceFunc)timeout_callback, notation, g_free);
  lua_pushinteger(L, tag);
  return 1;
}

static int builtin_clear_timeout(lua_State* L)
{
  int tag = luaL_checkinteger(L, 1);
  g_source_remove(tag);
  return 0;
}

static int builtin_get_config_path(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  lua_pushstring(L, context->config_path);
  return 1;
}

static int builtin_get_theme_path(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  char* path = config_acquire_theme_path(context->config);
  lua_pushstring(L, path);
  g_free(path);
  return 1;
}

static int builtin_get_version(lua_State* L)
{
  lua_pushstring(L, PACKAGE_VERSION);
  return 1;
}

static int builtin_put(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));

  const char* text = luaL_checkstring(L, -1);
  vte_terminal_feed_child(context->vte, text, -1);
  return 0;
}

static int builtin_beep(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  gdk_window_beep(gtk_widget_get_window(GTK_WIDGET(context_get_window(context))));
  return 0;
}

static int builtin_notify(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));

  const char* body = luaL_checkstring(L, 1);
  const char* title = lua_tostring(L, 2);
  context_notify(context, body, title);
  return 0;
}

static int builtin_copy(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  command_copy_clipboard(context);
  return 0;
}

static int builtin_paste(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  command_paste_clipboard(context);
  return 0;
}

static int builtin_increase_font_scale(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  command_increase_font_scale(context);
  return 0;
}

static int builtin_decrease_font_scale(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  command_decrease_font_scale(context);
  return 0;
}

static int builtin_reset_font_scale(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  command_reset_font_scale(context);
  return 0;
}

int builtin_register_module(lua_State* L)
{
  const luaL_Reg table[] = {
    { "get"                 , builtin_get                  },
    { "set"                 , builtin_set                  },
    { "get_config"          , builtin_get_config           },
    { "set_config"          , builtin_set_config           },
    { "reset_config"        , builtin_reset_config         },
    { "set_keymap"          , builtin_set_keymap           },
    { "set_keymaps"         , builtin_set_keymaps          },
    { "unset_keymap"        , builtin_unset_keymap         },
    { "reset_keymaps"       , builtin_reset_keymaps        },
    { "send_key"            , builtin_send_key             },
    /* { "set_hook"            , builtin_set_hook             }, */
    /* { "set_hooks"           , builtin_set_hooks            }, */
    { "reload"              , builtin_reload               },
    { "reload_theme"        , builtin_reload_theme         },
    { "apply"               , builtin_apply                },
    { "set_timeout"         , builtin_set_timeout          },
    { "clear_timeout"       , builtin_clear_timeout        },
    { "put"                 , builtin_put                  },
    { "beep"                , builtin_beep                 },
    { "notify"              , builtin_notify               },
    { "increase_font_scale" , builtin_increase_font_scale  },
    { "decrease_font_scale" , builtin_decrease_font_scale  },
    { "reset_font_scale"    , builtin_reset_font_scale     },
    { "copy"                , builtin_copy                 },
    { "paste"               , builtin_paste                },
    { "get_version"         , builtin_get_version          },
    { "get_config_path"     , builtin_get_config_path      },
    { "get_theme_path"      , builtin_get_theme_path       },
    { NULL, NULL },
  };
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));

  luaL_newlibtable(L, table);
  lua_pushlightuserdata(L, context);
  luaL_setfuncs(L, table, 1);
  return 1;
}
