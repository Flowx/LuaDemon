#include "CLuaStdLib.h"
#include "PlatformCompatibility.h"
#include "CLuaEnvironment.h"
#include <string>

#if _WIN32
	#include <Windows.h>
	#include <conio.h>
#endif


using namespace std;

void CLuaStdLib::PushFunctions()
{
	// NOTE: May be dropped! Read header.
	// includes another Lua file and runs it.
	lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaStdLib::Lua_include);
	lua_setglobal(CLuaEnvironment::_LuaState, "include");

	// forces the Lua environment to reload.
	lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaStdLib::Lua_forceReload);
	lua_setglobal(CLuaEnvironment::_LuaState, "forceReload");

	lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaStdLib::Lua_setConsoleTitle);
	lua_setglobal(CLuaEnvironment::_LuaState, "setConsoleTitle");
}

void CLuaStdLib::PollFunctions()
{
}

void CLuaStdLib::LoadFunctions()
{
}


/*
	_G.include(string Filename)

	Includes a Lua file and loads its.
*/
int CLuaStdLib::Lua_include(lua_State *LState) 
{
	string _file = lua_tostring(LState, 1);

	lua_Debug myDebug;
	lua_getstack(LState, 1, &myDebug);
	lua_getinfo(LState, "S", &myDebug);

	string src = myDebug.source + 1; // +1 to skip the '@'
	src = src.substr(0, src.find_last_of('/') + 1) + _file;

	if (EXISTS_FILE(src.c_str()))
	{
		if (luaL_loadfile(LState, src.c_str()))
		{
			PRINT_ERROR("ERROR: Lua failed to initialize: %s \n", lua_tostring(LState, -1));
			lua_pop(LState, 1);

			return 0;
		}
		else
		{
			lua_call(LState, 0, 0); // dont pcall here, as it would hide any errors that may occur in an included file
			// Any error thrown will be caught by the main pcall in CLuaEnvironment.
			/*
				if (!lua_pcall(LState, 0, 0, 0))
				{
					PRINT_DEBUG("Sucess pcall\n");
				}
				else
				{
					PRINT_ERROR("ERROR: Lua failed to pcall: %s \n", lua_tostring(LState, -1));
					lua_pop(LState, 1);
					return 0;
				};
			*/
		}
	}
	else PRINT_WARNING("Include \"%s\" was not found\n", _file.c_str());

	return 0;
}

/*
	_G.forceReload(nil)

	forces the lua environment to reload
*/
int CLuaStdLib::Lua_forceReload(lua_State *LState)
{
	CLuaEnvironment::_FileChange = true;
	return 0;
}


/*
	Sets the console/window title (in desktop GUI)

	May not work on all systems.

	_G.setConsoleTitle(string Title)
*/
int CLuaStdLib::Lua_setConsoleTitle(lua_State* LState)
{
#if _WIN32 // windows.h should be included already

	const char* str = lua_tostring(LState, 1);

	if(str) SetConsoleTitle(str);

#else
	//TODO: implement this on linux
	//prob never gonna do this as I use linux GUIs

#endif

	return 0;
}



//void CLuaStdLib::stackdump_g(lua_State* l)
//{
//	int i;
//	int top = lua_gettop(l);
//
//	printf("total in stack %d\n", top);
//
//	for (i = 1; i <= top; i++)
//	{
//		int t = lua_type(l, i);
//		switch (t) {
//		case LUA_TSTRING:  /* strings */
//			printf("string: '%s'\n", lua_tostring(l, i));
//			break;
//		case LUA_TBOOLEAN:  /* booleans */
//			printf("boolean %s\n", lua_toboolean(l, i) ? "true" : "false");
//			break;
//		case LUA_TNUMBER:  /* numbers */
//			printf("number: %g\n", lua_tonumber(l, i));
//			break;
//		default:  /* other values */
//			printf("%s\n", lua_typename(l, t));
//			break;
//		}
//		printf("  ");
//	}
//	printf("\n");
//}