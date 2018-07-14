#pragma once
#include "CLuaEnvironment.h"

//Lua has a native File IO
//This is used for shortcuts and extras

class CLuaFile
{
	static int Lua_Dump(lua_State * State);		//Args: string Filename, string Data
	
public:
	static void PushFunctions();
	static void PollFunctions();
	static void LoadFunctions();
};


