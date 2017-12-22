#include "PlatformCompatibility.h"
#include "CLuaEnvironment.h"

#pragma region Linker Restatements
volatile int CLuaEnvironment::_Error;
volatile bool CLuaEnvironment::_FileChange;
volatile bool CLuaEnvironment::_Initialized;
lua_State * CLuaEnvironment::_LuaState;
std::thread CLuaEnvironment::_FileThread;
#pragma endregion

void CLuaEnvironment::PushEnvTable()
{
	lua_newtable(_LuaState);

	lua_pushstring(_LuaState, _Directory.c_str()); // Root directory of the init.lua file
	lua_setfield(_LuaState, -2, "ENVDIR");

	#ifdef _WIN32
	lua_pushstring(_LuaState, "WINDOWS"); // This just shows what system this was compiled to
	#else
	lua_pushstring(_LuaState, "LINUX"); // This just shows what system this was compiled to
	#endif	
	lua_setfield(_LuaState, -2, "ENVSYS");

	lua_setglobal(_LuaState, "_LUAENV");
}

void CLuaEnvironment::Cycle()
{
	PushEnvTable();

	_FileChange = true; // loading for the first time
	for (;;)
	{
		if (_FileChange) { // this bool is a switch that needs to be reset after use
			_FileChange = false;
			LoadLua();
		}
	}
}

void stackdump_g(lua_State* l)
{
	int i;
	int top = lua_gettop(l);

	printf("total in stack %d\n", top);

	for (i = 1; i <= top; i++)
	{
		int t = lua_type(l, i);
		switch (t) {
		case LUA_TSTRING:  /* strings */
			printf("string: '%s'\n", lua_tostring(l, i));
			break;
		case LUA_TBOOLEAN:  /* booleans */
			printf("boolean %s\n", lua_toboolean(l, i) ? "true" : "false");
			break;
		case LUA_TNUMBER:  /* numbers */
			printf("number: %g\n", lua_tonumber(l, i));
			break;
		default:  /* other values */
			printf("%s\n", lua_typename(l, t));
			break;
		}
		printf("  ");
	}
	printf("\n");
}

void CLuaEnvironment::LoadLua()
{
	_Initialized = false;

	std::this_thread::sleep_for(std::chrono::milliseconds(1)); // reload doesnt work reliably without this for some reason

	std::string buffer = _Directory + LUAENV_INDEXFILE;

	_Error = luaL_loadfile(_LuaState, buffer.c_str()); // Load the entry point file

	if (_Error)
	{
		PRINT_ERROR("ERROR: Lua failed to initialize: %s\n", lua_tostring(_LuaState, -1));
		lua_pop(_LuaState, 1);
	}
	else
	{
		_Error = lua_pcall(_LuaState, 0, 0, 0);
		if (_Error) // Some error occured
		{
			PRINT_ERROR("ERROR: Lua returned with error:\n");

			PRINT_WARNING("ERROR: %s\n", lua_tostring(_LuaState, -1));
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