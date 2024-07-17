/**
 * builtin.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "builtin.h"
#include "context.h"
#include "command.h"
#include "app.h"


static int builtin_get(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));

  const char* key = luaL_checkstring(L, 1);
  MetaEntry* e = meta_get_entry(app->meta, key);
  if (!e) {
    luaX_warn(L, "Invalid config key: '%s'", key);
    lua_pushnil(L);
    return 1;
  }

  switch (e->type) {
    case META_ENTRY_TYPE_STRING:
      lua_pushstring(L, context_get_str(context, key));
      break;
    case META_ENTRY_TYPE_INTEGER:
      lua_pushinteger(L, context_get_int(context, key));
      break;
    case META_ENTRY_TYPE_BOOLEAN:
      lua_pushboolean(L, context_get_bool(context, key));
      break;
    default:
      lua_pushnil(L);
      break;
  }
  return 1;
}

static int builtin_quit(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  gtk_window_close(context->layout.window);
  return 0;
}

static int builtin_set(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));

  const char* key = luaL_checkstring(L, 1);

  MetaEntry* e = meta_get_entry(app->meta, key);
  if (!e) {
    luaX_warn(L, "Invalid config key: '%s'", key);
    return 0;
  }

  int type = lua_type(L, 2);
  switch (e->type) {
    case META_ENTRY_TYPE_STRING: {
      const char* value = lua_tostring(L, 2);
      if (!value) {
        luaX_warn(L, "Invalid string config for '%s' (string expected, got %s)", key, lua_typename(L, type));
        break;
      }
      context_set_str(context, key, value);
      break;
    }
    case META_ENTRY_TYPE_INTEGER: {
      if (type != LUA_TNUMBER) {
        luaX_warn(L, "Invalid integer config for '%s': %s (number expected, got %s)", key, lua_tostring(L, 2), lua_typename(L, type));
        break;
      }
      int value = lua_tointeger(L, 2);
      context_set_int(context, key, value);
      break;
    }
    case META_ENTRY_TYPE_BOOLEAN: {
      int value = lua_toboolean(L, 2);
      context_set_bool(context, key, value);
      break;
    }
    default:
      break;
  }
  return 0;
}

static int get_default_value(lua_State* L)
{
  /* Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1)); */

  const char* key = luaL_checkstring(L, 1);
  MetaEntry* e = meta_get_entry(app->meta, key);
  if (!e) {
    luaX_warn(L, "Invalid config key: '%s'", key);
    lua_pushnil(L);
    return 1;
  }

  switch (e->type) {
    case META_ENTRY_TYPE_STRING:
      lua_pushstring(L, (char*)e->default_value);
      break;
    case META_ENTRY_TYPE_INTEGER:
      lua_pushinteger(L, *(int*)e->default_value);
      break;
    case META_ENTRY_TYPE_BOOLEAN:
      lua_pushboolean(L, *(bool*)e->default_value);
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

  for (GList* li = app->meta->list; li != NULL; li = li->next) {
    MetaEntry* e = (MetaEntry*)li->data;
    char* key = e->name;
    lua_pushstring(L, key);
    switch (e->type) {
      case META_ENTRY_TYPE_STRING: {
        const char* value = context_get_str(context, key);
        lua_pushstring(L, value);
        break;
      }
      case META_ENTRY_TYPE_INTEGER:
        lua_pushinteger(L, context_get_int(context, key));
        break;
      case META_ENTRY_TYPE_BOOLEAN:
        lua_pushboolean(L, context_get_bool(context, key));
        break;
      case META_ENTRY_TYPE_NONE:
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
    MetaEntry* e = meta_get_entry(app->meta, key);
    if (e) {
      int type = lua_type(L, -2);
      switch (e->type) {
        case META_ENTRY_TYPE_STRING: {
          const char* value = lua_tostring(L, -2);
          if (!value) {
            luaX_warn(L, "Invalid string config for '%s' (string expected, got %s)", key, lua_typename(L, type));
            break;
          }
          context_set_str(context, key, value);
          break;
        }
        case META_ENTRY_TYPE_INTEGER: {
          if (type != LUA_TNUMBER) {
            luaX_warn(L, "Invalid integer config for '%s': %s (number expected, got %s)", key, lua_tostring(L, -2), lua_typename(L, type));
            break;
          }
          int value = lua_tointeger(L, -2);
          context_set_int(context, key, value);
          break;
        }
        case META_ENTRY_TYPE_BOOLEAN: {
          int value = lua_toboolean(L, -2);
          context_set_bool(context, key, value);
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

  return 0;
}

static int builtin_reset_config(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  context_restore_default(context);
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
    luaX_warn(L, "Invalid accelerator: '%s'", key);
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
      luaX_warn(L, "Invalid value for '%s': function expected, got %s", key, lua_typename(L, lua_type(L, -2)));
    } else {
      lua_pushvalue(L, -2); // push function to stack top
      int ref = luaL_ref(L, LUA_REGISTRYINDEX);
      bool ok = keymap_add_entry(context->keymap, key, ref);
      if (!ok) {
        luaL_unref(L, LUA_REGISTRYINDEX, ref);
        luaX_warn(L, "Invalid accelerator: '%s'", key);
      }
    }
    lua_pop(L, 2);
  }

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
  int ref = luaL_ref(L, LUA_REGISTRYINDEX);
  int old_ref = -1;
  if (hook_set_ref(context->hook, key, ref, &old_ref)) {
    if (old_ref > 0) {
      dd("unref old ref");
      luaL_unref(L, LUA_REGISTRYINDEX, old_ref);
    }
    return 0;
  }
  luaL_unref(L, LUA_REGISTRYINDEX, ref);
  luaX_warn(L, "Invalid hook key: '%s'", key);
  return 0;
}

static int builtin_set_hooks(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  luaL_argcheck(L, lua_istable(L, 1), 1, "table expected");
  lua_pushnil(L);
  while (lua_next(L, -2)) {
    lua_pushvalue(L, -2);
    const char* key = lua_tostring(L, -1);
    if (!lua_isfunction(L, -2)) {
      luaX_warn(L, "Invalid value for '%s': function expected, got %s", key, lua_typename(L, lua_type(L, -2)));
    } else {
      lua_pushvalue(L, -2); // push function to stack top
      int ref = luaL_ref(L, LUA_REGISTRYINDEX);
      int old_ref = -1;
      int ok = hook_set_ref(context->hook, key, ref, &old_ref);
      if (old_ref > 0) {
        dd("unref old ref");
        luaL_unref(L, LUA_REGISTRYINDEX, old_ref);
      }
      if (!ok) {
        luaL_unref(L, LUA_REGISTRYINDEX, ref);
        luaX_warn(L, "Invalid hook key: '%s'", key);
      }
    }
    lua_pop(L, 2);
  }
  return 0;
}

static int builtin_reload(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  context_load_config(context);
  context_load_theme(context);
  return 0;
}

static int builtin_reload_theme(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  context_load_theme(context);
  return 0;
}

static int builtin_send_key(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  const char* accelerator = luaL_checkstring(L, 1);
  unsigned key;
  GdkModifierType mod;
  gtk_accelerator_parse(accelerator, &key, &mod);
  if (0 == key && 0 == mod) {
    luaL_error(L, "Invalid accelerator: '%s'", accelerator);
    return 0;
  }
  GdkEvent* event = gdk_event_new(GDK_KEY_PRESS);
  if (context->device == NULL) {
    g_warning("Could not get input device.");
    return 0;
  }
  gdk_event_set_device(event, context->device);
  event->key.window = g_object_ref(gtk_widget_get_window(GTK_WIDGET(context->layout.window)));
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

typedef struct {
  Context* context;
  int ref;
} TimeoutCallbackNotation;

static int timeout_callback(void* user_data)
{
  TimeoutCallbackNotation* notation = (TimeoutCallbackNotation*)user_data;

  lua_State* L = notation->context->lua;
  lua_rawgeti(L, LUA_REGISTRYINDEX, notation->ref);
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 1); // pop none-function
    dd("tried to call non-function");
    return false;
  }

  if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
    luaX_warn(L, "Error in timeout function: '%s'", lua_tostring(L, -1));
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
  TimeoutCallbackNotation* notation = g_new0(TimeoutCallbackNotation, 1);
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

static int builtin_put(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  const char* text = luaL_checkstring(L, -1);
  vte_terminal_feed_child(context->layout.vte, text, -1);
  return 0;
}

static int builtin_bell(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  gdk_window_beep(context_get_gdk_window(context));
  return 0;
}

static int builtin_open(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  const char* uri = luaL_checkstring(L, -1);
  context_launch_uri(context, uri);
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
  const char* text = luaL_checkstring(L, 1);
  const char* target = lua_tostring(L, 2);
  GdkAtom selection = GDK_SELECTION_CLIPBOARD;
  if (!target || is_equal(target, TYM_CLIPBOARD_CLIPBOARD)) {
  } else if (is_equal(target, TYM_CLIPBOARD_PRIMARY)) {
    selection = GDK_SELECTION_PRIMARY;
  } else if (is_equal(target, TYM_CLIPBOARD_SECONDARY)) {
    selection = GDK_SELECTION_SECONDARY;
  } else {
    luaX_warn(L, "Invalid target(`%s`): 'clipboard', 'primary' or 'secondary' is available.", target);
  }
  GtkClipboard* cb = gtk_clipboard_get(selection);
  gtk_clipboard_set_text(cb, text, -1);
  return 0;
}

static int builtin_copy_selection(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  const char* target = lua_tostring(L, 1);
  if (!target || is_equal(target, TYM_CLIPBOARD_CLIPBOARD)) {
    command_copy_selection(context);
    return 0;
  }
  if (is_equal(target, TYM_CLIPBOARD_PRIMARY)) {
    // nothing to do
    return 0;
  }
  if (is_equal(target, TYM_CLIPBOARD_SECONDARY)) {
    // go down
  } else {
    luaX_warn(L, "Invalid target(`%s`): 'clipboard', 'primary' or 'secondary' is available.", target);
    return 0;
  }

  GtkClipboard* pri = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
  char* text = gtk_clipboard_wait_for_text(pri);
  GtkClipboard* sec = gtk_clipboard_get(GDK_SELECTION_SECONDARY);
  gtk_clipboard_set_text(sec, text, -1);
  g_free(text);
  return 0;
}

static int builtin_paste(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));

  const char* target = lua_tostring(L, 1);
  if (!target || is_equal(target, TYM_CLIPBOARD_CLIPBOARD)) {
    command_paste(context);
    return 0;
  }
  GdkAtom selection = GDK_SELECTION_CLIPBOARD;
  if (is_equal(target, TYM_CLIPBOARD_PRIMARY)) {
    selection = GDK_SELECTION_PRIMARY;
  } else if (is_equal(target, TYM_CLIPBOARD_SECONDARY)) {
    selection = GDK_SELECTION_SECONDARY;
  } else {
    luaX_warn(L, "Invalid target(`%s`): 'clipboard', 'primary' or 'secondary' is available.", target);
    return 0;
  }
  GtkClipboard* cb = gtk_clipboard_get(selection);
  char* text = gtk_clipboard_wait_for_text(cb);
  vte_terminal_feed_child(context->layout.vte, text, -1);
  g_free(text);
  return 0;
}

static int builtin_color_to_rgba(lua_State* L)
{
  GdkRGBA color;
  const char* str = luaL_checkstring(L, -1);
  bool valid = gdk_rgba_parse(&color, str);
  if (!valid) {
    luaX_warn(L, "Invalid color string: '%s'", str);
    return 0;
  }
  lua_pushinteger(L, roundup(color.red * 255));
  lua_pushinteger(L, roundup(color.green * 255));
  lua_pushinteger(L, roundup(color.blue * 255));
  lua_pushnumber(L, color.alpha);
  return 4;
}

static int builtin_rgba_to_color(lua_State* L)
{
  int red = luaL_checknumber(L, 1);
  int green = luaL_checknumber(L, 2);
  int blue = luaL_checknumber(L, 3);
  double alpha = lua_isnone(L, 4) ? 1.0 : lua_tonumber(L, 4);
  char* str = g_strdup_printf("rgba(%d, %d, %d, %f)", red, green, blue, alpha);
  lua_pushstring(L, str);
  g_free(str);
  return 1;
}

static int builtin_rgb_to_hex(lua_State* L)
{
  int red = luaL_checknumber(L, 1);
  int green = luaL_checknumber(L, 2);
  int blue = luaL_checknumber(L, 3);
  char* hex = g_strdup_printf("#%x%x%x", red, green, blue);
  lua_pushstring(L, hex);
  g_free(hex);
  return 1;
}

static int builtin_hex_to_rgb(lua_State* L)
{
  const char* hex = luaL_checkstring(L, 1);

  GdkRGBA color = {};
  bool valid = gdk_rgba_parse(&color, hex);
  if (!valid) {
    luaX_warn(L, "Invalid hex string: '%s'", hex);
    return 0;
  }
  lua_pushinteger(L, roundup(color.red * 255));
  lua_pushinteger(L, roundup(color.green * 255));
  lua_pushinteger(L, roundup(color.blue * 255));
  return 3;
}

static GVariant* table_to_variant(lua_State* L, int table_index)
{
  luaL_argcheck(L, lua_istable(L, table_index), table_index, "table expected");

#if USES_LUAJIT
  size_t num_params = lua_objlen(L, table_index);
#else
  size_t num_params = lua_rawlen(L, table_index);
#endif
  GVariant** vv = g_new0(GVariant*, num_params);
  int i = 0;
  while (i < num_params) {
    lua_rawgeti(L, table_index, i + 1); // args[table_index][i+1]
    vv[i] = g_variant_new_string(lua_tostring(L, -1));
    lua_pop(L, 1);
    i += 1;
  }
  GVariant* p = g_variant_new_tuple(vv, num_params);
  g_free(vv);
  return p;
}

/* usage tym.signal(0, 'hook', {'param'}) */
static int builtin_signal(lua_State* L)
{
  int target_id = luaL_checkinteger(L, 1);
  const char* signal_name = luaL_checkstring(L, 2);
  GVariant* params = lua_gettop(L) >= 3
    ? table_to_variant(L, 3)
    : g_variant_new("()");

  GDBusConnection* conn = g_application_get_dbus_connection(app->gapp);
  GError* error = NULL;
  char* object_path = g_strdup_printf(TYM_OBJECT_PATH_FMT_INT, target_id);
  g_dbus_connection_emit_signal(conn, NULL, object_path, TYM_APP_ID, signal_name, params, &error);
  g_free(object_path);
  if (error) {
    luaX_warn(L, "DBus error: '%s'", error->message);
    g_error_free(error);
    return 0;
  }

  return 0;
}

typedef struct {
  Context* context;
  int ref;
} CallCallbackNotation;

void push_value_by_gvariant(lua_State* L, GVariant* v) {
  if (g_variant_is_of_type(v, G_VARIANT_TYPE_INT32)) {
    lua_pushinteger(L, g_variant_get_int32(v));
  } else if (g_variant_is_of_type(v, G_VARIANT_TYPE_STRING)) {
    char* s = NULL;
    g_variant_get(v, "s", &s);
    lua_pushstring(L, s);
    g_free(s);
  } else if (g_variant_is_of_type(v, G_VARIANT_TYPE_ARRAY) || g_variant_is_of_type(v, G_VARIANT_TYPE_TUPLE)) {
    lua_newtable(L);
    size_t num = g_variant_n_children(v);
    int i = 0;
    while (i < num) {
      GVariant* e = g_variant_get_child_value(v, i);
      push_value_by_gvariant(L, e);
      lua_rawseti(L, -2, i + 1);
      i += 1;
    }
  } else {
    lua_pushstring(L, g_variant_print(v, false));
  }
}

void call_callback(GObject* source_object, GAsyncResult* res, void* user_data)
{
  GError* error = NULL;
  TimeoutCallbackNotation* notation = (TimeoutCallbackNotation*)user_data;
  Context* context = notation->context;
  lua_State* L = context->lua;

  GDBusConnection* conn = g_application_get_dbus_connection(app->gapp);
  GVariant* result = g_dbus_connection_call_finish(conn, res, &error);

  lua_rawgeti(L, LUA_REGISTRYINDEX, notation->ref);
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 1); // pop none-function
    context_log_warn(context, true, "tried to call non-function");
    return;
  }

  int num_args = 0;
  if (error) {
    char* m = g_strdup_printf("DBus error: '%s'", error->message);
    luaX_warn(L, "%s", m);
    lua_pushstring(L, m);
    g_error_free(error);
    g_free(m);
    num_args = 1;
  } else {
    dd("DBus method call result: `%s`", g_variant_print(result, true));
    int num_result = g_variant_n_children(result);
    int i = 0;
    while (i < num_result) {
      GVariant* e = g_variant_get_child_value(result, i);
      push_value_by_gvariant(L, e);
      i += 1;
    }
    num_args = num_result;
  }

  if (lua_pcall(L, num_args, 0, 0) != LUA_OK) {
    luaX_warn(L, "Error in timeout function: '%s'", lua_tostring(L, -1));
    lua_pop(L, 1); // error
  }
}

/* usage: tym.call(0, 'eval', {'return 1+2'}, function(...) end) */
static int builtin_call(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  int target_id = luaL_checkinteger(L, 1);
  const char* method_name = luaL_checkstring(L, 2);
  GVariant* params = table_to_variant(L, 3);

  bool has_cb = lua_gettop(L) >= 4;

  CallCallbackNotation* notation = NULL;
  GAsyncReadyCallback cb = NULL;
  if (has_cb) {
    luaL_argcheck(L, lua_isfunction(L, 4), 4, "function expected");
    lua_pushvalue(L, 4);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    notation = g_new0(CallCallbackNotation, 1);
    notation->context = context;
    notation->ref = ref;
    cb = (GAsyncReadyCallback)call_callback;
  }

  GDBusConnection* conn = g_application_get_dbus_connection(app->gapp);
  char* object_path = g_strdup_printf(TYM_OBJECT_PATH_FMT_INT, target_id);

  g_dbus_connection_call(
      conn,        // conn
      TYM_APP_ID,  // bus_name
      object_path, // object_path
      TYM_APP_ID,  // interface_name
      method_name, // method_name
      params,      // parameters
      NULL,        // reply_type
      G_DBUS_CALL_FLAGS_NONE, // flags
      1000,        // timeout
      NULL,        // cancellable
      cb,          // callback
      notation     // user_data
  );
  g_free(object_path);
  return 0;
}

static int builtin_check_mod_state(lua_State* L)
{
  const char* accelerator = luaL_checkstring(L, 1);
  unsigned key;
  GdkModifierType mod;
  gtk_accelerator_parse(accelerator, &key, &mod);
  GdkDisplay* display = gdk_display_get_default();
  GdkKeymap* kmap = gdk_keymap_get_for_display(display);
  unsigned current_mod = gdk_keymap_get_modifier_state(kmap);
  lua_pushboolean(L, mod & current_mod);
  return 1;
}

static int builtin_get_cursor_position(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  long col = 0;
  long row = 0;
  vte_terminal_get_cursor_position(context->layout.vte, &col, &row);
  lua_pushinteger(L, col);
  lua_pushinteger(L, row);
  return 2;
}

static int builtin_get_clipboard(lua_State* L)
{
  const char* target = lua_tostring(L, 1);
  GdkAtom selection = GDK_SELECTION_CLIPBOARD;
  if (!target || is_equal(target, TYM_CLIPBOARD_CLIPBOARD)) {
  } else if (is_equal(target, TYM_CLIPBOARD_PRIMARY)) {
    selection = GDK_SELECTION_PRIMARY;
  } else if (is_equal(target, TYM_CLIPBOARD_SECONDARY)) {
    selection = GDK_SELECTION_SECONDARY;
  } else {
    luaX_warn(L, "Invalid target(`%s`): 'clipboard', 'primary' or 'secondary' is available.", target);
    return 0;
  }
  GtkClipboard* cb = gtk_clipboard_get(selection);
  char* text = gtk_clipboard_wait_for_text(cb);
  lua_pushstring(L, text);
  g_free(text);
  return 1;
}

static int builtin_get_selection(lua_State* L)
{
  GtkClipboard* cb = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
  char* text = gtk_clipboard_wait_for_text(cb);
  lua_pushstring(L, text);
  g_free(text);
  return 1;
}

static int builtin_unselect_all(lua_State* L)
{
	Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
	vte_terminal_unselect_all(context->layout.vte);
	return 0;
}

static int builtin_select_all(lua_State *L) {
	Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
	vte_terminal_select_all(context->layout.vte);
	return 0;
}

static int builtin_has_selection(lua_State *L) {
	Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
	lua_pushboolean(L, vte_terminal_get_has_selection(context->layout.vte));
	return 1;
}


static int builtin_get_text(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  int start_row = luaL_checkinteger(L, 1);
  int start_col = luaL_checkinteger(L, 2);
  int end_row = luaL_checkinteger(L, 3);
  int end_col = luaL_checkinteger(L, 4);
  if (end_row < 0) {
    end_row = vte_terminal_get_row_count(context->layout.vte);
  }
  if (end_col < 0) {
    end_col = vte_terminal_get_column_count(context->layout.vte);
  }

#ifdef TYM_USE_VTE_GET_TEXT_RANGE_FORMAT
  char* selection = vte_terminal_get_text_range_format(context->layout.vte, VTE_FORMAT_TEXT, start_row, start_col, end_row, end_col, NULL);
#else
  char* selection = vte_terminal_get_text_range(context->layout.vte, start_row, start_col, end_row, end_col, NULL, NULL, NULL);
#endif
  lua_pushstring(L, selection);
  g_free(selection);
  return 1;
}

static int builtin_get_monitor_model(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  GdkDisplay* display = gdk_display_get_default();
  GdkMonitor* monitor = gdk_display_get_monitor_at_window(display, context_get_gdk_window(context));
  lua_pushstring(L, gdk_monitor_get_model(monitor));
  return 1;
}

static int builtin_get_config_path(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  char* path = context_acquire_config_path(context);
  lua_pushstring(L, path);
  if (path) {
    g_free(path);
  }
  return 1;
}

static int builtin_get_theme_path(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  char* path = context_acquire_theme_path(context);
  lua_pushstring(L, path);
  if (path) {
    g_free(path);
  }
  return 1;
}

static int builtin_get_id(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  lua_pushinteger(L, context->id);
  return 1;
}

static int builtin_get_ids(lua_State* L)
{
  lua_newtable(L);
  int i = 0;
  for (GList* li = app->contexts; li != NULL; li = li->next) {
    Context* c = (Context*)li->data;
    lua_pushinteger(L, c->id);
    lua_rawseti(L, -2, i + 1);
    i += 1;
  }
  return 1;
}

static int builtin_get_object_path(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  lua_pushstring(L, context->object_path);
  return 1;
}

static int builtin_get_pid(lua_State* L)
{
  lua_pushinteger(L, getpid());
  return 1;
}

static int builtin_get_version(lua_State* L)
{
  lua_pushstring(L, PACKAGE_VERSION);
  return 1;
}

static int builtin_apply(lua_State* L)
{
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));
  const char* message = "DEPRECATED: `tym.apply()` is never needed. You can `tym.set()` and the value is applied right away to the app.";
  context_notify(context, message, NULL);
  luaX_warn(L, "%s", message);
  return 0;
}

int builtin_register_module(lua_State* L)
{
  const luaL_Reg table[] = {
    { "quit"                , builtin_quit                 },
    { "get"                 , builtin_get                  },
    { "set"                 , builtin_set                  },
    { "get_default_value"   , get_default_value            },
    { "get_config"          , builtin_get_config           },
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
    { "send_key"            , builtin_send_key             },
    { "set_timeout"         , builtin_set_timeout          },
    { "clear_timeout"       , builtin_clear_timeout        },
    { "put"                 , builtin_put                  },
    { "bell"                , builtin_bell                 },
    { "open"                , builtin_open                 },
    { "notify"              , builtin_notify               },
    { "copy"                , builtin_copy                 },
    { "copy_selection"      , builtin_copy_selection       },
    { "paste"               , builtin_paste                },
    { "check_mod_state"     , builtin_check_mod_state      },
    { "color_to_rgba"       , builtin_color_to_rgba        },
    { "rgba_to_color"       , builtin_rgba_to_color        },
    { "rgb_to_hex"          , builtin_rgb_to_hex           },
    { "hex_to_rgb"          , builtin_hex_to_rgb           },
    { "signal"              , builtin_signal               },
    { "call"                , builtin_call                 },
    { "get_monitor_model"   , builtin_get_monitor_model    },
    { "get_cursor_position" , builtin_get_cursor_position  },
    { "get_clipboard"       , builtin_get_clipboard        },
    { "get_selection"       , builtin_get_selection        },
    { "unselect_all"        , builtin_unselect_all         },
    { "select_all"          , builtin_select_all           },
    { "has_selection"       , builtin_has_selection        },
    { "get_text"            , builtin_get_text             },
    { "get_config_path"     , builtin_get_config_path      },
    { "get_theme_path"      , builtin_get_theme_path       },
    { "get_id"              , builtin_get_id               },
    { "get_ids"             , builtin_get_ids              },
    { "get_object_path"     , builtin_get_object_path      },
    { "get_pid"             , builtin_get_pid              },
    { "get_version"         , builtin_get_version          },
    // DEPRECATED
    { "apply"               , builtin_apply                },
    { NULL, NULL },
  };
  Context* context = (Context*)lua_touserdata(L, lua_upvalueindex(1));

  luaL_newlibtable(L, table);
  lua_pushlightuserdata(L, context);
  luaL_setfuncs(L, table, 1);
  return 1;
}
