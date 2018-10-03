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
    luaL_warn(L, "Invalid config key: `%s`", key);
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

static int builtin_set(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));

  const char* key = luaL_checkstring(L, 1);

  ConfigField* field = get_config_field(key);
  if (!field) {
    luaL_warn(L, "Invalid config key: `%s`", key);
    return 0;
  }

  int type = lua_type(L, 2);
  switch (field->type) {
    case CONFIG_TYPE_STRING: {
      const char* value = lua_tostring(L, 2);
      if (!value) {
        luaL_warn(L, "Invalid string config for `%s` (string expected, got %s)", key, lua_typename(L, type));
        break;
      }
      config_set_str(context->config, key, value);
      break;
    }
    case CONFIG_TYPE_INTEGER: {
      if (type != LUA_TNUMBER) {
        luaL_warn(L, "Invalid integer config for `%s`: `%s` (number expected, got %s)", key, lua_tostring(L, 2), lua_typename(L, type));
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
            luaL_warn(L, "Invalid string config for `%s` (string expected, got %s)", key, lua_typename(L, type));
            break;
          }
          config_set_str(context->config, key, value);
          break;
        }
        case CONFIG_TYPE_INTEGER: {
          if (type != LUA_TNUMBER) {
            luaL_warn(L, "Invalid integer config for `%s`: `%s` (number expected, got %s)", key, lua_tostring(L, -2), lua_typename(L, type));
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
      luaL_warn(L, "Invalid config key: `%s`", key);
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
  context_apply_config(context);
  context_apply_theme(context);
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
    luaL_error(L, "Invalid acceralator: `%s`", key);
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
    luaL_warn(L, "Tried to remove en empty keymap (`%s`) which is not assigned function to", key);
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
      luaL_warn(L, "Invalid keymap value for `%s`: function expected, got %s", key, lua_typename(L, lua_type(L, -2)));
    } else {
      lua_pushvalue(L, -2); // push function to stack top
      int ref = luaL_ref(L, LUA_REGISTRYINDEX);
      bool ok = keymap_add_entry(context->keymap, key, ref);
      if (!ok) {
        luaL_unref(L, LUA_REGISTRYINDEX, ref);
        luaL_warn(L, "Invalid acceralator: `%s`", key);
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

static int builtin_set_hook(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));

  const char* key = luaL_checkstring(L, 1);
  luaL_argcheck(L, lua_isfunction(L, 2), 2, "function expected");

  // register hook
  UNUSED(key);
  UNUSED(context);
  dd("UNDER CONSTRUCTION");

  return 0;
}

static int builtin_set_hooks(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));

  UNUSED(context);
  dd("UNDER CONSTRUCTION");

  return 0;
}

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

static int builtin_get_config_path(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  lua_pushstring(L, context->config_path);
  return 1;
}

static int builtin_get_theme_path(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  lua_pushstring(L, context->theme_path);
  return 1;
}

static int builtin_get_version(lua_State* L)
{
  lua_pushstring(L, PACKAGE_VERSION);
  return 1;
}

static int builtin_notify(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));

  const char* body = luaL_checkstring(L, 1);
  const char* title = lua_tostring(L, 2);
  context_notify(context, body, title);
  return 0;
}

static int builtin_feed(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  VteTerminal* vte = context_get_vte(context);

  const char* text = luaL_checkstring(L, -1);
  vte_terminal_feed_child(vte, text, -1);
  return 1;
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
    { "get_config"          , builtin_get_config           },
    { "set"                 , builtin_set                  },
    { "set_config"          , builtin_set_config           },
    { "reset_config"        , builtin_reset_config         },
    { "set_keymap"          , builtin_set_keymap           },
    { "unset_keymap"        , builtin_unset_keymap         },
    { "set_keymaps"         , builtin_set_keymaps          },
    { "reset_keymaps"       , builtin_reset_keymaps        },
    { "set_hook"            , builtin_set_hook             },
    { "set_hooks"           , builtin_set_hooks            },
    { "reload"              , builtin_reload               },
    { "reload_theme"        , builtin_reload_theme         },
    { "apply"               , builtin_apply                },
    { "reset_keymaps"       , builtin_reset_keymaps        },
    { "get_config_path"     , builtin_get_config_path      },
    { "get_theme_path"      , builtin_get_theme_path       },
    { "get_version"         , builtin_get_version          },
    { "notify"              , builtin_notify               },
    { "feed"                , builtin_feed                 },
    { "increase_font_scale" , builtin_increase_font_scale  },
    { "decrease_font_scale" , builtin_decrease_font_scale  },
    { "reset_font_scale"    , builtin_reset_font_scale     },
    { "copy"                , builtin_copy                 },
    { "paste"               , builtin_paste                },
    { NULL, NULL },
  };
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));

  luaL_newlibtable(L, table);
  lua_pushlightuserdata(L, context);
  luaL_setfuncs(L, table, 1);
  return 1;
}
