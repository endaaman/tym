/**
 * hook.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "tym.h"


#define HOOK_KEY_TITLE "title"
#define HOOK_KEY_BELL "bell"
#define HOOK_KEY_CLICKED "clicked"
#define HOOK_KEY_URI_CLICKED "uri_clicked"
#define HOOK_KEY_ACTIVATED "activated"
#define HOOK_KEY_DEACTIVATED "deactivated"

const char* HOOK_KEYS[] = {
  HOOK_KEY_TITLE,
  HOOK_KEY_BELL,
  HOOK_KEY_CLICKED,
  HOOK_KEY_URI_CLICKED,
  HOOK_KEY_ACTIVATED,
  HOOK_KEY_DEACTIVATED,
  NULL
};

Hook* hook_init()
{
  Hook* hook = g_malloc0(sizeof(Hook));
  hook->refs = g_hash_table_new_full(
    g_str_hash,
    g_str_equal,
    (GDestroyNotify)g_free,
    (GDestroyNotify)g_free
  );

  int i = 0;
  while (HOOK_KEYS[i]) {
    int* p = g_malloc0(sizeof(int));
    *p = -1;
    g_hash_table_insert(hook->refs, g_strdup(HOOK_KEYS[i]), p);
    i += 1;
  }
  return hook;
}

void hook_close(Hook* hook)
{
  g_hash_table_destroy(hook->refs);
  g_free(hook);
}

static int hook_get_ref(Hook* hook, const char* key)
{
  int* ptr = g_hash_table_lookup(hook->refs, key);
  if (!ptr) {
    dd("invalid hook key: '%s'", key);
    return -1;
  }
  return *ptr;
}

bool hook_set_ref(Hook* hook, const char* key, int ref, int* old_ref)
{
  assert(old_ref);
  void* old_key = NULL;
  void* old_value = NULL;
  bool has_value = g_hash_table_lookup_extended(hook->refs, key, &old_key, &old_value);
  if (old_value) {
    *old_ref = *(int*)old_value;
  }
  if (!has_value) {
    return false;
  }
  g_hash_table_remove(hook->refs, old_key);
  g_hash_table_insert(hook->refs, g_strdup(key), g_memdup(&ref, sizeof(int)));
  dd("hook (%s) is registered. ref: %d", key, ref);
  return true;
}

static bool hook_perform(Hook* hook, lua_State* L, const char* key, int narg, int nresult)
{
  int ref = hook_get_ref(hook, key);
  if (ref < 0) {
    lua_pop(L, narg);
    return false;
  }
  lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 1); // pop none-function
    dd("tried to call hook which is not function.");
    return false;
  }
  lua_insert(L, - narg - 1);
  dd("perform custom hook: %s", key);
  if (lua_pcall(L, narg, nresult, 0) != LUA_OK) {
    luaX_warn(L, "Error in hook function: '%s'", lua_tostring(L, -1));
    lua_pop(L, 1); // error
    return false;
  }
  return true;
}

bool hook_perform_title(Hook* hook, lua_State* L, const char* title, char** next_title)
{
  lua_pushstring(L, title);
  bool result = hook_perform(hook, L, HOOK_KEY_TITLE, 1, 1);
  if (!result) {
    return false;
  }
  const char* t = lua_tostring(L, -1);
  lua_pop(L, 1);
  if (next_title) {
    *next_title = t ? g_strdup(t) : NULL;
  }
  return true;
}

bool hook_perform_bell(Hook* hook, lua_State* L, bool* result)
{
  assert(result);
  bool succeeded = hook_perform(hook, L, HOOK_KEY_BELL, 0, 1);
  if (!succeeded) {
    return false;
  }
  *result = lua_toboolean(L, -1);
  lua_pop(L, 1);
  return succeeded;
}

bool hook_perform_activated(Hook* hook, lua_State* L)
{
  return hook_perform(hook, L, HOOK_KEY_ACTIVATED, 0, 0);
}

bool hook_perform_deactivated(Hook* hook, lua_State* L)
{
  return hook_perform(hook, L, HOOK_KEY_DEACTIVATED, 0, 0);
}

bool hook_perform_clicked(Hook* hook, lua_State* L, int button, bool* result)
{
  assert(result);
  lua_pushinteger(L, button);
  bool succeeded = hook_perform(hook, L, HOOK_KEY_CLICKED, 1, 0);
  if (!succeeded) {
    return false;
  }
  *result = lua_toboolean(L, -1);
  lua_pop(L, 1);
  return true;
}

bool hook_perform_uri_clicked(Hook* hook, lua_State* L, const char* uri, int button, bool* result)
{
  assert(result);
  lua_pushstring(L, uri);
  lua_pushinteger(L, button);
  bool succeeded = hook_perform(hook, L, HOOK_KEY_URI_CLICKED, 2, 1);
  if (!succeeded) {
    return false;
  }
  *result = lua_toboolean(L, -1);
  lua_pop(L, 1);
  return succeeded;
}
