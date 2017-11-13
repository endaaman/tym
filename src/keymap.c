/**
 * keymap.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "keymap.h"
#include "context.h"
#include "command.h"


typedef struct {
  unsigned key;
  GdkModifierType mod;
  char* func_key;
} CustomKeyPair;


static const char* KEYMAP_TABLE_NAME = "keymap";

void custom_key_pair_free(CustomKeyPair* pair)
{
  g_free(pair->func_key);
  g_free(pair);
}


Keymap* keymap_init(lua_State* lua)
{
  Keymap* keymap = g_malloc0(sizeof(Keymap));
  keymap->lua = lua;
  keymap->custom_key_pairs = NULL;
  keymap->has_custom = false;

  keymap_reset(keymap);
  return keymap;
}

void keymap_reset(Keymap* keymap)
{
  g_slist_foreach(keymap->custom_key_pairs, (GFunc)custom_key_pair_free, NULL);
  g_slist_free(keymap->custom_key_pairs);
  keymap->custom_key_pairs = NULL;
  keymap->has_custom = false;
}

void keymap_close(Keymap* keymap)
{
  keymap_reset(keymap);
  g_free(keymap);
}


void keymap_prepare(Keymap* keymap)
{
  lua_State* l = keymap->lua;
  lua_newtable(l);
  lua_setglobal(l, KEYMAP_TABLE_NAME);
}

void keymap_add_custom(Keymap* keymap, CustomKeyPair* pair)
{
  keymap->custom_key_pairs = g_slist_append(keymap->custom_key_pairs, pair);
}

void keymap_load(Keymap* keymap, char** error)
{
  lua_State* l = keymap->lua;
  lua_getglobal(l, KEYMAP_TABLE_NAME);

  if (lua_isnil(l, -1)) {
    // no error for nil
    lua_pop(l, 1);
    return;
  }

  if (!lua_istable(l, -1)) {
    *error = g_strdup_printf("`%s` is not table", KEYMAP_TABLE_NAME);
    lua_pop(l, 1);
    return;
  }

  lua_pushnil(l);
  while (lua_next(l, -2)) {
    lua_pushvalue(l, -2);
    const char* accelator = lua_tostring(l, -1);

    unsigned key;
    GdkModifierType mod;
    gtk_accelerator_parse(accelator, &key, &mod);
    if (0 != key && 0 != mod) {
      CustomKeyPair* pair = g_malloc0(sizeof(CustomKeyPair));
      pair->key = key;
      pair->mod = mod;
      pair->func_key = g_strdup(lua_tostring(l, -1));
      keymap_add_custom(keymap, pair);
      dd("loaded: %s as key: 0x%x mod: 0x%x", accelator, mod, key);
    } else {
      g_warning("`%s` is invalid accelator", accelator);
    }
    lua_pop(l, 2);
  }
  lua_pop(l, 1);

  keymap->has_custom = true;
}

bool keymap_perform_custom(Keymap* keymap, unsigned key, GdkModifierType mod, char** error)
{
  if (!keymap->has_custom) {
    return false;
  }

  lua_State* l = keymap->lua;
  for (GSList* li = keymap->custom_key_pairs; li != NULL; li = li->next) {
    CustomKeyPair* pair = (CustomKeyPair*)li->data;
    if ((key == pair->key) && !(~mod & pair->mod)) {
      lua_getglobal(l, KEYMAP_TABLE_NAME);
      lua_getfield(l, -1, pair->func_key);
      if (0 != lua_pcall(l, 0, 0, 0)) {
        *error = g_strdup(lua_tostring(l, -1));
      }
      lua_pop(l, 1);
      return true;
    }
  }
  return false;
}
