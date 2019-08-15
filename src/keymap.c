/**
 * keymap.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "keymap.h"


typedef struct {
  unsigned key;
  GdkModifierType mod;
  char* accelerator;
  int ref;
} KeymapEntry;


static void free_keymap_entry(KeymapEntry* e, void* user_data)
{
  // TODO: luaL_unref the ref
  UNUSED(user_data);
  g_free(e->accelerator);
  g_free(e);
}

Keymap* keymap_init()
{
  Keymap* keymap = g_malloc0(sizeof(Keymap));
  keymap->entries = NULL;

  keymap_reset(keymap);
  return keymap;
}

void keymap_reset(Keymap* keymap)
{
  df();
  g_list_foreach(keymap->entries, (GFunc)free_keymap_entry, NULL);
  g_list_free(keymap->entries);
  keymap->entries = NULL;
}

void keymap_close(Keymap* keymap)
{
  keymap_reset(keymap);
  g_free(keymap);
}

bool keymap_add_entry(Keymap* keymap, const char* accelerator, int ref)
{
  unsigned key;
  GdkModifierType mod;
  gtk_accelerator_parse(accelerator, &key, &mod);
  if (0 == key && 0 == mod) {
    return false;
  }
  bool removed = keymap_remove_entry(keymap, accelerator);
  KeymapEntry* e = g_malloc0(sizeof(KeymapEntry));
  e->key = key;
  e->mod = mod;
  e->accelerator = g_strdup(accelerator);
  e->ref = ref;
  keymap->entries = g_list_append(keymap->entries, e);
  if (removed) {
    dd("keymap (%s mod: %x, key: %x) has been overwritten", accelerator, mod, key);
  } else {
    dd("keymap (%s mod: %x, key: %x) has been newly assined", accelerator, mod, key);
  }
  return true;
}

bool keymap_remove_entry(Keymap* keymap, const char* accelerator)
{
  for (GList* li = keymap->entries; li != NULL; li = li->next) {
    KeymapEntry* e = (KeymapEntry*)li->data;
    if (0 == g_strcmp0(e->accelerator, accelerator)) {
      keymap->entries = g_list_remove(keymap->entries, e);
      g_free(e);
      return true;
    }
  }
  return false;
}

bool keymap_perform(Keymap* keymap, lua_State* L, unsigned key, GdkModifierType mod, bool* result, char** error)
{
  assert(result);
  assert(error);
  for (GList* li = keymap->entries; li != NULL; li = li->next) {
    KeymapEntry* e = (KeymapEntry*)li->data;
    if (key == e->key && mod == e->mod) {
      dd("performing keymap: %s (mod: %x, key: %x)", e->accelerator, mod, key);
      lua_rawgeti(L, LUA_REGISTRYINDEX, e->ref);
      if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1); // pop none-function
        dd("tried to call keymap [%s] which is not function.", e->accelerator);
        return false;
      }
      if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
        *error = g_strdup(lua_tostring(L, -1));
        lua_pop(L, 1); // error
        return false;
      }
      *result = lua_toboolean(L, -1);
      lua_pop(L, 1);
      return true;
    }
  }
  return false;
}
