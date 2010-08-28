/*
** Library initialization.
** Major parts taken verbatim from the Lua interpreter.
** Copyright (C) 1994-2008 Lua.org, PUC-Rio. See Copyright Notice in lua.h
*/

extern "C" {

#define lib_init_c
#define LUA_LIB

#ifdef APL
	/* Prevent conflicts with standard Lua and enforce luajit on OS X */
	#include "luajit-2.0/lua.h"
	#include "luajit-2.0/lauxlib.h"
	#include "luajit-2.0/lualib.h"
#else
	#include "lua.h"
	#include "lauxlib.h"
	#include "lualib.h"
#endif

static const luaL_Reg lualibs[] = {
  { "",			luaopen_base },
  { LUA_LOADLIBNAME,	luaopen_package },
  { LUA_TABLIBNAME,	luaopen_table },
  { LUA_IOLIBNAME,	luaopen_io },
  { LUA_OSLIBNAME,	luaopen_os },
  { LUA_STRLIBNAME,	luaopen_string },
  { LUA_MATHLIBNAME,	luaopen_math },
  { LUA_DBLIBNAME,	luaopen_debug },
  { LUA_BITLIBNAME,	luaopen_bit },
  { LUA_JITLIBNAME,	luaopen_jit },
  { NULL,		NULL }
};

LUALIB_API void luaL_openlibs(lua_State *L)
{
  const luaL_Reg *lib = lualibs;
  for (; lib->func; lib++) {
    lua_pushcfunction(L, lib->func);
    lua_pushstring(L, lib->name);
    lua_call(L, 1, 0);
  }
}

}

