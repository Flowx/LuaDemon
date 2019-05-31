#pragma once
#include "CLuaLib.h"
#include "CLuaEnvironment.h"

//Lua has a native File IO
//This is used for shortcuts and extras

class CLuaFile : CLuaLib 
{
	//static int Lua_Dump(lua_State * State);		//Args: string Filename, string Data
	static int Lua_readAll(lua_State * State);		//Args: string Filename ; Returns: string Data
	static int Lua_writeAll(lua_State * State);		//Args: string Filename, string Data, bool NoOverwrite=nil; Returns bool Success

public:
	static void PushFunctions();
	static void PollFunctions();
	static void LoadFunctions();
};