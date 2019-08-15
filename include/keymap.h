/**
 * keymap.h
 *
 * Copyright (c) 2019 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef KEYMAP_H
#define KEYMAP_H

#include "common.h"


typedef struct {
  GList* entries;
} Keymap;


Keymap* keymap_init();
void keymap_close(Keymap* keymap);
void keymap_reset(Keymap* keymap);
bool keymap_add_entry(Keymap* keymap, const char* accelerator, int ref);
bool keymap_remove_entry(Keymap* keymap, const char* accelerator);
bool keymap_perform(Keymap* keymap, lua_State* L, unsigned key, GdkModifierType mod, bool* result, char** error);

#endif
