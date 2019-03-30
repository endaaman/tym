/**
 * hook.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "tym.h"


Hook* hook_init()
{
  Hook* hook = g_malloc0(sizeof(Hook));
  return hook;
}

void hook_close(Hook* hook)
{
  g_free(hook);
}

bool hook_set_ref(Hook* hook, const char* name, int ref)
{
  if (0 == g_strcmp0(name, "title")) {
    hook->title_ref = ref;
  } else if (0 == g_strcmp0(name, "bell")) {
    hook->bell_ref = ref;
  /* } else if (0 == g_strcmp0(name, "uri")) { */
  /*   hook->uri_ref = ref; */
  } else if (0 == g_strcmp0(name, "activated")) {
    hook->activated_ref = ref;
  } else if (0 == g_strcmp0(name, "deactivated")) {
    hook->deactivated_ref = ref;
  } else {
    return false;
  }
  return true;
}

static bool hook_perform(lua_State* L, int ref, int narg, int nresult)
{
  lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 1); // pop none-function
    dd("tried to call hook which is not function.");
    return false;
  }
  lua_insert(L, - narg - 1);
  if (lua_pcall(L, narg, nresult, 0) != LUA_OK) {
    luaX_warn(L, "Error in hook function: '%s'", lua_tostring(L, -1));
    lua_pop(L, 1); // error
    return false;
  }
  return true;
}

bool hook_perform_title(Hook* hook, lua_State* L, const char* title, char** next_title)
{
  if (hook->title_ref == 0) {
    return NULL;
  }
  dd("hook perform: title");
  lua_pushstring(L, title);
  bool result = hook_perform(L, hook->title_ref, 1, 1);
  if (!result) {
    return NULL;
  }
  const char* t = lua_tostring(L, -1);
  bool r = lua_toboolean(L, -1);
  lua_pop(L, 1);
  if (next_title) {
    *next_title = t ? g_strdup(t) : NULL;
  }
  return r;
}

bool hook_perform_bell(Hook* hook, lua_State* L)
{
  if (hook->bell_ref == 0) {
    return false;
  }
  dd("hook perform: bell");
  bool result = hook_perform(L, hook->bell_ref, 0, 1);
  if (!result) {
    return false;
  }
  return lua_toboolean(L, -1);
}

bool hook_perform_activated(Hook* hook, lua_State* L)
{
  if (hook->activated_ref == 0) {
    return false;
  }
  dd("hook perform: activated");
  return hook_perform(L, hook->activated_ref, 0, 0);
}

bool hook_perform_deactivated(Hook* hook, lua_State* L)
{
  if (hook->deactivated_ref == 0) {
    return false;
  }
  dd("hook perform: deactivated");
  return hook_perform(L, hook->deactivated_ref, 0, 0);
}
