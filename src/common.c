/**
 * common.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "common.h"

#ifdef DEBUG

void debug_dump_stack(lua_State *l, char* file, unsigned line) {
  g_print("[%s:%u stack dump]\n", file, line);
  int i = lua_gettop(l);
  if ( i > 0 ) {
    while( i ) {
      int t = lua_type(l, i);
      g_print("  ");
      switch (t) {
        case LUA_TSTRING:
          g_print("%d: str: %s\n", i, lua_tostring(l, i));
          break;
        case LUA_TBOOLEAN:
          g_print("%d: bool: %s\n", i, lua_toboolean(l, i) ? "true" : "false");
          break;
        case LUA_TNUMBER:
          g_print("%d: number: %g\n", i, lua_tonumber(l, i));
          break;
        default:
          g_print("%d: %s\n", i, lua_typename(l, t));
          break;
      }
      i--;
    }
  } else {
    g_print("  stack is empty\n");
  }
  g_print("[stack dump end]\n");
}

#endif

