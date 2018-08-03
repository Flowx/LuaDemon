#include "CLuaNet.h"

// Linker ref
bool CLuaNet::m_Init;
unsigned long CLuaNetSocket::m_Counter;
std::list<CLuaNetSocket *> CLuaNet::m_UDPSockets;
std::list<CLuaNetSocket *> CLuaNet::m_TCPSockets;

CLuaNetConnection::CLuaNetConnection(unsigned int Socket)
{
	m_Socket = Socket;
}



CLuaNetSocket::CLuaNetSocket(unsigned int Socket)
{
	m_Socket = Socket;
	m_ID = getID();
}

unsigned long CLuaNetSocket::getID()
{
	if (m_Counter == 0) m_Counter = 1;
	m_Counter += 1; // overflow should cause wrap around; unlikely with 4 billion sockets
	return m_Counter;
}



void CLuaNet::PushFunctions()
{
	CLuaNet::m_Init = false;

	lua_newtable(CLuaEnvironment::_LuaState);

	lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaNet::Lua_dumpUDP);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "dumpUDP");

	lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaNet::Lua_openUDPSocket);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "openUDP");

	lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaNet::Lua_openTCPSocket);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "openTCP");

	lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaNet::Lua_connect);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "connect");


	lua_setglobal(CLuaEnvironment::_LuaState, "net");
}

void CLuaNet::LoadFunctions()
{
}
