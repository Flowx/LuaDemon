#pragma once
#include "Lua/lua.hpp"
class CLuaStdLib
{
public:
	static int Lua_include(lua_State * S);
	static int Lua_forceReload(lua_State * S);

	
	
	static void stackdump_g(lua_State * S);
};

