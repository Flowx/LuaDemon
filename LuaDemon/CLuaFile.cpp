#include "CLuaFile.h"

int CLuaFile::Lua_Dump(lua_State * State)
{
	return 0;
}




void CLuaFile::PushFunctions()
{
	lua_newtable(CLuaEnvironment::_LuaState);

	//lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaNet::Lua_createSocket);
	//lua_setfield(CLuaEnvironment::_LuaState, -2, "create");

	//lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaNet::Lua_sendUDP);
	//lua_setfield(CLuaEnvironment::_LuaState, -2, "sendUDP");

	lua_setglobal(CLuaEnvironment::_LuaState, "file");
}

void CLuaFile::PollFunctions()
{
}

void CLuaFile::LoadFunctions()
{
}
