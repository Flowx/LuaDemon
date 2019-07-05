#include "PlatformCompatibility.h"
#include "CLuaEnvironment.h"
#include "CLuaStdLib.h"

#pragma region Linker Restatements
volatile int CLuaEnvironment::_Error;
volatile bool CLuaEnvironment::_FileChange;
volatile bool CLuaEnvironment::_Initialized;
lua_State * CLuaEnvironment::_LuaState;
std::thread CLuaEnvironment::_FileThread;
#pragma endregion


/*
	The _LUAENV global table contains some useful information about the system architecture, hardware and OS environment.

*/
void CLuaEnvironment::PushEnvTable()
{
	lua_newtable(_LuaState);

	lua_pushstring(_LuaState, _Directory.c_str()); // Root directory of the init.lua file
	lua_setfield(_LuaState, -2, "ENVDIR");

#ifdef _WIN32 // This just shows what system this was compiled to
	lua_pushstring(_LuaState, "WINDOWS"); 
#else
	lua_pushstring(_LuaState, "LINUX");
#endif	
	lua_setfield(_LuaState, -2, "ENVSYS");

	lua_setglobal(_LuaState, "_LUAENV");
}

void CLuaEnvironment::Cycle()
{
	// This runs in the main thread.
	PushEnvTable(); // collection of useful data in a global table
	PushLibraries();

	_FileChange = true; // loading for the first time
	for (;;)
	{
		// TODO: implement configurable poll time
		std::this_thread::sleep_for(std::chrono::microseconds(10)); // 100kHz is a plenty high poll rate
		if (_FileChange) // this bool is a switch that needs to be reset after use
		{
			LoadLibraries();
			_FileChange = false;
			LoadLua();
		}
		
		PollLibraries();
	}
}

/*
	Main function that actually loads the Lua files.
*/
void CLuaEnvironment::LoadLua()
{
	_Initialized = false;

	std::this_thread::sleep_for(std::chrono::milliseconds(10)); // reload doesnt work reliably without this for some reason

	std::string buffer = _Directory + LUAENV_INDEXFILE;

	_Error = luaL_loadfile(_LuaState, buffer.c_str()); // Load the entry point file
	if (_Error)
	{
		PRINT_ERROR("ERROR: Lua failed to initialize: %s \n", lua_tostring(_LuaState, -1));
		lua_pop(_LuaState, 1);
	}
	else
	{
		_Error = lua_pcall(_LuaState, 0, 0, 0);
		if (_Error) // Some error occured
		{
			PRINT_ERROR("ERROR: Lua returned with error:\n");

			PRINT_WARNING("ERROR: %s \n", lua_tostring(_LuaState, -1));
			lua_pop(_LuaState, 1);
		}
		else
		{
			PRINT_SUCCESS("Lua successfully initialized!\n");
			_Initialized = true; // ready to resume operation
		}
	}
}

void CLuaEnvironment::LoadFileSpy()
{
	_FileThread = std::thread(&FileSpy);
	_FileThread.detach();
}