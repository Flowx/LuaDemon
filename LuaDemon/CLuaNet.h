#pragma once
#include "CLuaLib.h"
#include "CLuaEnvironment.h"

class CLuaNet: public CLuaLib
{
	static int Lua_createSocket(lua_State * State);

public:
	static void PushFunctions();

};

