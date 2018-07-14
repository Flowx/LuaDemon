#pragma once
#include "Lua/lua.hpp"

class CLuaStdLib
{
	static int Lua_include(lua_State * S);
	static int Lua_forceReload(lua_State * S);

public:
	static void PushFunctions();
	static void PollFunctions();
	static void LoadFunctions();
};

