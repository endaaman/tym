/**
 * bindings.h
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
  bool has_custom;
  GSList* custom_key_pairs;
} Keymap;

Keymap* keymap_init();
void keymap_close(Keymap* keymap);
void keymap_reset(Keymap* keymap);

void keymap_prepare_lua(Keymap* keymap, lua_State* l);
void keymap_load_from_lua(Keymap* keymap, lua_State* l);

bool keymap_perform_custom(Keymap* keymap, lua_State* l, unsigned key, GdkModifierType mod);
bool keymap_perform_default(Keymap* keymap, void* context, unsigned key, GdkModifierType mod);

#endif
