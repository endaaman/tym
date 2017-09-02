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
#include "builtin.h"

typedef void (*TymBuiltinFunc)(Context* context);


typedef struct {
  const char* accel_name;
  TymBuiltinFunc func;
} DefaultKeyPairDefinition;

typedef struct {
  unsigned key;
  GdkModifierType mod;
  TymBuiltinFunc func;
} DefaultKeyPair;

typedef struct {
  unsigned key;
  GdkModifierType mod;
  char* func_key;
} CustomKeyPair;

static const char* KEYMAP_TABLE_NAME = "keymap";
static GSList* default_key_pairs;

static DefaultKeyPairDefinition default_key_pair_definition[] = {
  { "<Ctrl><Shift>r", builtin_reload },
  { "<Ctrl>plus", builtin_increase_font_scale },
  { "<Ctrl>minus", builtin_decrease_font_scale },
  { "<Ctrl>equal", builtin_reset_font_scale, },
  { "<Ctrl><Shift>c", builtin_copy_clipboard },
  { "<Ctrl><Shift>v", builtin_paste_clipboard },
  { NULL, NULL },
};

void custom_key_pair_free(CustomKeyPair* pair)
{
  g_free(pair->func_key);
  g_free(pair);
}


Keymap* keymap_init()
{
  Keymap* keymap = g_malloc0(sizeof(Keymap));
  keymap->custom_key_pairs = NULL;
  keymap->has_custom = false;
  return keymap;
}

void keymap_reset(Keymap* keymap)
{
  dd("keymap reset");
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


void keymap_prepare_lua(Keymap* keymap, lua_State* l)
{
  dd("keymap prepare lua");
  UNUSED(keymap);
  lua_newtable(l);
  lua_setglobal(l, KEYMAP_TABLE_NAME);
}

void keymap_add_custom(Keymap* keymap, CustomKeyPair* pair)
{
  keymap->custom_key_pairs = g_slist_append(keymap->custom_key_pairs, pair);
}

void keymap_load_from_lua(Keymap* keymap, lua_State* l)
{
  dd("keymap load from lua");

  lua_getglobal(l, KEYMAP_TABLE_NAME);

  if (lua_isnil(l, -1)) {
    lua_pop(l, 1);
    return;
  }

  if (!lua_istable(l, -1)) {
    g_print("warning: `%s` is not table.\n", KEYMAP_TABLE_NAME);
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
      dd("loaded: %s", accelator);
    } else {
      g_print("warning: `%s` is invalid accelator.\n", accelator);
    }
    lua_pop(l, 2);
  }
  lua_pop(l, 1);

  keymap->has_custom = true;
}


bool keymap_perform_custom(Keymap* keymap, lua_State* l, unsigned key, GdkModifierType mod)
{
  if (!keymap->has_custom) {
    return false;
  }

  for (GSList* li = keymap->custom_key_pairs; li != NULL; li = li->next) {
    CustomKeyPair* pair = (CustomKeyPair*)li->data;
    if ((key == pair->key) && !(~mod & pair->mod)) {
      lua_getglobal(l, KEYMAP_TABLE_NAME);
      lua_getfield(l, -1, pair->func_key);
      if (0 != lua_pcall(l, 0, 0, 0)) {
        g_print("error: %s\n", lua_tostring(l, -1));
        lua_pop(l, 1);
      }
      lua_pop(l, 1);
      return true;
    }
  }
  return false;
}

bool keymap_perform_default(Keymap* keymap, void* context, unsigned key, GdkModifierType mod)
{
  UNUSED(keymap);
  for (GSList* li = default_key_pairs; li != NULL; li = li->next) {
    DefaultKeyPair* pair = (DefaultKeyPair*)li->data;
    if ((key == pair->key) && !(~mod & pair->mod)) {
      pair->func(context);
      return true;
    }
  }
  return false;
}

__attribute__((constructor))
static void initialization() {
  default_key_pairs = NULL;
  int i = 0;
  while (default_key_pair_definition[i].func) {
    DefaultKeyPair* pair = g_malloc0(sizeof(DefaultKeyPair));
    gtk_accelerator_parse(default_key_pair_definition[i].accel_name, &pair->key, &pair->mod);
    pair->func = default_key_pair_definition[i].func;
    default_key_pairs = g_slist_append(default_key_pairs, pair);
    i = i + 1;
  }
}

__attribute__((destructor))
static void finalization() {
  g_slist_foreach(default_key_pairs, (GFunc)g_free, NULL);
  g_slist_free(default_key_pairs);
}
