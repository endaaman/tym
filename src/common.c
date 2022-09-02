/**
 * common.c
 *
 * Copyright (c) 2017 endaaman
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "common.h"

const int TYM_DEFAULT_WIDTH = 80;
const int TYM_DEFAULT_HEIGHT = 22;
const int TYM_DEFAULT_SCALE = 100;
const int TYM_DEFAULT_CELL_SIZE = 100;
const int TYM_DEFAULT_SCROLLBACK = 512;


#ifdef DEBUG
void debug_dump_stack(lua_State* L, char* file, unsigned line)
{
  g_print("[%-10s:%3u] (stack dump)\n", file, line);
  int len = lua_gettop(L);
  int i = lua_gettop(L);
  if ( i > 0 ) {
    while( i ) {
      int t = lua_type(L, i);
      g_print("  [%d:%d] ", i, i - len - 1);
      switch (t) {
        case LUA_TSTRING:
          g_print("str: %s ", lua_tostring(L, i));
          break;
        case LUA_TBOOLEAN:
          g_print("bool: %s ", lua_toboolean(L, i) ? "true" : "false");
          break;
        case LUA_TNUMBER:
          g_print("number: %g ", lua_tonumber(L, i));
          break;
        case LUA_TTABLE: {
          g_print("*%s ", lua_typename(L, t));
          /* g_print("table: "); */
          /* lua_pushnil(L); */
          /* while (lua_next(L, -2)) { */
          /*   lua_pushvalue(L, -2); */
          /*   g_print("[%s] ", lua_tostring(L, -1)); */
          /*   lua_pop(L, 2); */
          /* } */
          /* lua_pop(L, 1); */
          break;
        }
        default:
          g_print("*%s ", lua_typename(L, t));
          break;
      }
      if (len == i) {
        g_print("(top)");
      }
      g_print("\n");
      i--;
    }
  } else {
    g_print("  stack is empty\n");
  }
}
#endif

int roundup(double x)
{
  return (int)(x + 0.5);
}

bool is_equal(const char* a, const char* b)
{
  return g_strcmp0(a, b) == 0;
}

bool is_none(const char* s)
{
  return g_strcmp0(s, TYM_SYMBOL_NONE) == 0;
}

bool is_empty(const char* s)
{
  return g_strcmp0(s, "") == 0;
}

void luaX_requirec(lua_State* L, const char* modname, lua_CFunction openf, int glb, void* userdata)
{
#if USES_LUAJIT
  luaL_getmetatable(L, LUA_LOADED_TABLE);
#else
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE);
#endif
  lua_getfield(L, -1, modname);  /* LOADED[modname] */
  if (!lua_toboolean(L, -1)) {  /* package not already loaded? */
    lua_pop(L, 1);  /* remove field */
    lua_pushlightuserdata(L, userdata);
    lua_pushcclosure(L, openf, 1);
    lua_pushstring(L, modname);  /* argument to open function */
    lua_call(L, 1, 1);  /* call 'openf' to open module */
    lua_pushvalue(L, -1);  /* make copy of module (call result) */
    lua_setfield(L, -3, modname);  /* LOADED[modname] = module */
  }
  lua_remove(L, -2);  /* remove LOADED table */
  if (glb) {
    lua_pushvalue(L, -1);  /* copy of module */
    lua_setglobal(L, modname);  /* _G[modname] = module */
  }
}

int luaX_warn(lua_State* L, const char* fmt, ...)
{
  va_list argp;
  va_start(argp, fmt);
  luaL_where(L, 1);
  lua_pushvfstring(L, fmt, argp);
  va_end(argp);
  lua_concat(L, 2);
  g_message("%s", lua_tostring(L,-1));
  lua_pop(L, 1);
  return 0;
}
