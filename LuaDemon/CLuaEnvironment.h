#pragma once
#include "Lua/lua.hpp"
#include <string>
#include <thread>

#define LUAENV_INDEXFILE "init.lua"

class CLuaEnvironment
{
public:
	static lua_State * _LuaState; 
	static std::thread _FileThread;

	static void FileSpy(); // directory watchdog - refreshes Lua state on file change

	static std::string _Directory;

	static void Cycle();
	static void LoadLua();
	static void LoadFileSpy();

	static void PollLibraries();
	static void PushLibraries(); 

	static void PushEnvTable();
	static void PushEnvFuncs();

	// Flag states
	static volatile int  _Error;
	static volatile bool _Initialized;
	static volatile bool _FileChange;
};


