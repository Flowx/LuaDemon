#pragma once
#include "CLuaLib.h"
#include "CLuaEnvironment.h"
#include <map>


#include <Windows.h>

class CLuaSerial: public CLuaLib
{
	static std::map<std::string, HANDLE> _portList;

public:
	static void PushFunctions();
	static void PollFunctions();

	static int Lua_Discover(lua_State * State); //Args: none
	static int Lua_Open(lua_State * State); //Args: string Port, string Name
	static int Lua_Send(lua_State * State); //Args: string Port, number Data (0-255)
	static int Lua_Receive(lua_State * State); //Args: string Port, function LuaCallback
};

