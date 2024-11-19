/**
 * hook.h
 *
 * Copyright (c) 2019 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef HOOK_H
#define HOOK_H

#include "common.h"


typedef struct {
  GHashTable* refs;
} Hook;


Hook* hook_init();
void hook_close(Hook* hook);
bool hook_set_ref(Hook* hook, const char* key, int ref, int* old_ref);
bool hook_perform_title(Hook* hook, lua_State* L, const char* title, bool* result);
bool hook_perform_bell(Hook* hook, lua_State* L, bool* result);
bool hook_perform_clicked(Hook* hook, lua_State* L, int button, const char* uri, bool* result);
bool hook_perform_scroll(Hook* hook, lua_State* L, double delta_x, double delta_y, double x, double y, bool* result);
bool hook_perform_drag(Hook* hook, lua_State* L, char* path, bool* result);
bool hook_perform_activated(Hook* hook, lua_State* L);
bool hook_perform_deactivated(Hook* hook, lua_State* L);
bool hook_perform_selected(Hook* hook, lua_State* L, const char* text);
bool hook_perform_unselected(Hook* hook, lua_State* L);
bool hook_perform_resized(Hook* hook, lua_State* L);
bool hook_perform_signal(Hook* hook, lua_State* L, const char* param);

#endif
