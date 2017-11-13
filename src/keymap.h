/**
 * keymap.h
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef KEYMAP_H
#define KEYMAP_H

#include "common.h"

typedef struct {
  lua_State* lua;
  bool has_custom;
  GSList* custom_key_pairs;
} Keymap;

Keymap* keymap_init(lua_State* l);
void keymap_close(Keymap* keymap);
void keymap_reset(Keymap* keymap);
void keymap_prepare(Keymap* keymap);
void keymap_load(Keymap* keymap, char** error);
bool keymap_perform_custom(Keymap* keymap, unsigned key, GdkModifierType mod);
bool keymap_perform_default(Keymap* keymap, void* context, unsigned key, GdkModifierType mod);

#endif
