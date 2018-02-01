#include "CLuaSerial.h"

#include "PlatformCompatibility.h"

CLuaSerialPort::CLuaSerialPort(const char * Name)
{
	m_Name = Name;
	//CLuaSerial::m_PortList[m_Name] = this;
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

	lua_setglobal(CLuaEnvironment::_LuaState, "serial");
}

void CLuaSerial::LoadFunctions()
{
}
