#include "CLuaStdLib.h"
#include "PlatformCompatibility.h"
#include "CLuaEnvironment.h"
#include <string>

using namespace std;

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
			PRINT_WARNING("Something went wrong loading include file!\n");
			return 0;
		}
		else lua_pcall(LState, 0, 0, 0);
	}
	else PRINT_WARNING("Include \"%s\" was not found\n", _file.c_str());

	return 0;
}

int CLuaStdLib::Lua_forceReload(lua_State *LState)
{
	CLuaEnvironment::_FileChange = true;

	return 0;
}



void CLuaStdLib::stackdump_g(lua_State* l)
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