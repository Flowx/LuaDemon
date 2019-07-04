#pragma once
#include "Lua/lua.hpp"
#include "CLuaLib.h"

class CLuaStdLib : CLuaLib
{
	static int Lua_include(lua_State * S); // NOTE: Is this useful? require and dofile exists too ...
	static int Lua_forceReload(lua_State * S);

public:
	static void PushFunctions();
	static void PollFunctions();
	static void LoadFunctions();
};
