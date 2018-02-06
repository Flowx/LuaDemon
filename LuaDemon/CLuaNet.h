#pragma once
#include "CLuaLib.h"
#include "CLuaEnvironment.h"

class CLuaNet
{
	static int Lua_createSocket(lua_State * State);

public:
	static void PushFunctions();

};

