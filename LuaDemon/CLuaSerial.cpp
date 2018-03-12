#include "CLuaSerial.h"

#include "PlatformCompatibility.h"

CLuaSerialPort::CLuaSerialPort(const char * Name)
{
	m_Name = Name;
}

// Lists all open or active serial ports
// NOTE: Is manually setting a number index required?
int CLuaSerial::Lua_ListOpen(lua_State * State)
{
	lua_newtable(State);

	int count = 0;
	for (std::pair<std::string, CLuaSerialPort*> v : m_PortList)
	{
		count++;
		lua_pushinteger(State, count);
		lua_pushstring(State, v.first.c_str());
		lua_settable(State, -3);
	}

	return 1;
}

void CLuaSerial::PushFunctions()
{
	lua_newtable(CLuaEnvironment::_LuaState);

	lua_pushcfunction(CLuaEnvironment::_LuaState, Lua_Discover);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "Discover");

	lua_pushcfunction(CLuaEnvironment::_LuaState, Lua_Open);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "Open");

	lua_pushcfunction(CLuaEnvironment::_LuaState, Lua_Send);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "Send");

	lua_pushcfunction(CLuaEnvironment::_LuaState, Lua_Receive);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "Receive");

	lua_pushcfunction(CLuaEnvironment::_LuaState, Lua_Available);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "Available");

	lua_pushcfunction(CLuaEnvironment::_LuaState, Lua_ReadAll);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "ReadAll");

	lua_pushcfunction(CLuaEnvironment::_LuaState, Lua_ListOpen);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "ListOpen");

	lua_setglobal(CLuaEnvironment::_LuaState, "serial");
}

void CLuaSerial::LoadFunctions()
{
}
