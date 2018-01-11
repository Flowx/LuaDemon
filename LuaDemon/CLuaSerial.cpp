#include "CLuaSerial.h"



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

	lua_setglobal(CLuaEnvironment::_LuaState, "serial");
}

void CLuaSerial::PollFunctions()
{
}

void CLuaSerial::LoadFunctions()
{
}
