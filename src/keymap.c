/**
 * keymap.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "tym.h"


typedef struct {
  unsigned key;
  GdkModifierType mod;
  char* acceralator;
  int ref;
} KeymapEntry;


static void free_keymap_entry(KeymapEntry* e, void* user_data)
{
  // TODO: luaL_unref the ref
  UNUSED(user_data);
  g_free(e->acceralator);
  g_free(e);
}

Keymap* keymap_init()
{
  dd("init");
  Keymap* keymap = g_malloc0(sizeof(Keymap));
  keymap->entries = NULL;

  keymap_reset(keymap);
  return keymap;
}

void keymap_reset(Keymap* keymap)
{
  dd("reset");
  g_list_foreach(keymap->entries, (GFunc)free_keymap_entry, NULL);
  g_list_free(keymap->entries);
  keymap->entries = NULL;
}

void keymap_close(Keymap* keymap)
{
  keymap_reset(keymap);
  g_free(keymap);
}

bool keymap_add_entry(Keymap* keymap, const char* acceralator, int ref)
{
  unsigned key;
  GdkModifierType mod;
  gtk_accelerator_parse(acceralator, &key, &mod);
  if (0 == key || 0 == mod) {
    return false;
  }
  bool removed = keymap_remove_entry(keymap, acceralator);
  KeymapEntry* e = g_malloc0(sizeof(KeymapEntry));
  e->key = key;
  e->mod = mod;
  e->acceralator = g_strdup(acceralator);
  e->ref = ref;
  keymap->entries = g_list_append(keymap->entries, e);
  if (removed) {
    dd("keymap [%s] has been overwritten", acceralator);
  } else {
    dd("keymap [%s] has been newly assined", acceralator);
  }
  return true;
}

bool keymap_remove_entry(Keymap* keymap, const char* acceralator)
{
  for (GList* li = keymap->entries; li != NULL; li = li->next) {
    KeymapEntry* e = (KeymapEntry*)li->data;
    if (0 == g_strcmp0(e->acceralator, acceralator)) {
      keymap->entries = g_list_remove(keymap->entries, e);
      g_free(e);
      return true;
    }
  }
  return false;
}

bool keymap_perform(Keymap* keymap, lua_State* L, unsigned key, GdkModifierType mod, char** error)
{
  for (GList* li = keymap->entries; li != NULL; li = li->next) {
    KeymapEntry* e = (KeymapEntry*)li->data;
    if ((key == e->key) && !(~mod & e->mod)) {
      dd("perform keymap: %s", e->acceralator);
      lua_rawgeti(L, LUA_REGISTRYINDEX, e->ref);
      if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1); // pop none-function
        g_warning("Tried to call keymap (%s) which is not function.", e->acceralator);
        return true;
      }
      if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
        *error = g_strdup(lua_tostring(L, -1));
        lua_pop(L, 1); // error
        return true;
      }
      bool result = true;
      if (lua_isboolean(L, -1)) {
        result = lua_toboolean(L, -1);
      }
      lua_pop(L, 1); // result
      return result;
    }
  }
  return false;
}
