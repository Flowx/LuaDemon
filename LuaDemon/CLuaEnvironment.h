#pragma once
#include "Lua/lua.hpp"
#include <string>
#include <thread>

#define LUAENV_INDEXFILE "init.lua"

// Main class to contain the Lua State and functions to control it
class CLuaEnvironment
{
public:
	static lua_State * _LuaState; 
	
	static std::string _Directory;

	static std::thread _FileThread;
	static void FileSpy(); // directory watchdog - refreshes Lua state on file change

	static void Cycle(); // poll function called every 10us; Paused during initialization
	static void LoadLua();
	static void LoadFileSpy();

	static void PollLibraries();
	static void PushLibraries(); 
	static void LoadLibraries();

	static void PushEnvTable();
	static void PushEnvFuncs();

	// Flag states
	static volatile int  _Error; // If not 0 an error occured during initialization; Contains Lua error codes!
	static volatile bool _Initialized; // Set when the Lua State has finished its initialization run (also set when a reload is completed)
	static volatile bool _FileChange; // Set to true if you want to reload the Lua State (FileSpy uses this)
};


