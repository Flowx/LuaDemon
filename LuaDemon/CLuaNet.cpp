#include "CLuaNet.h"

// Linker ref
bool CLuaNet::m_Init;
std::list<CLuaNetSocket *> CLuaNet::m_UDPSockets;


CLuaNetSocket::CLuaNetSocket(int Socket)
{
	m_Socket = Socket;
}


void CLuaNet::PushFunctions()
{
	CLuaNet::m_Init = false;

	lua_newtable(CLuaEnvironment::_LuaState);

	lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaNet::Lua_openTCPSocket);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "openTCP");

	lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaNet::Lua_openUDPSocket);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "openUDP");

	lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaNet::Lua_connect);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "connect");

	lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaNet::Lua_dumpUDP);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "dumpUDP");

	lua_setglobal(CLuaEnvironment::_LuaState, "net");
}

void CLuaNet::PollFunctions()
{

	for (size_t i = 0; i < CLuaNet::; i++)
	{

	}
}

void CLuaNet::LoadFunctions()
{
}
