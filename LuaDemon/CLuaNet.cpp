#include "CLuaNet.h"

// Linker ref
bool CLuaNet::m_Init;
unsigned long CLuaNet::CLuaNetSocket::m_Counter;
std::list<CLuaNet::CLuaNetSocket *> CLuaNet::m_UDPSockets;
std::list<CLuaNet::CLuaNetSocket *> CLuaNet::m_TCPSockets;
//std::list<CLuaNet::CLuaNetClient *> CLuaNet::CLuaNetSocket::m_Clients;


CLuaNet::CLuaNetClient::CLuaNetClient(unsigned int Socket)
{
	m_Socket = Socket;
}

CLuaNet::CLuaNetSocket::CLuaNetSocket(unsigned int Socket)
{
	m_Socket = Socket;
	m_ID = getID();
}

unsigned long CLuaNet::CLuaNetSocket::getID()
{
	if (m_Counter == 0) m_Counter = 1;
	m_Counter += 1; // overflow should cause wrap around; unlikely with 4 billion sockets
	if (m_Counter == 0xFFFF) m_Counter = 1;
	return m_Counter;
}

/*
	Lua param:
	int SocketID

	Closes an UDP socket
*/
int CLuaNet::Lua_closeUDPSocket(lua_State * State)
{
	int _id = lua_tointeger(State, 1);
	if (_id == 0) return 0;


	for (auto i = m_UDPSockets.begin(); i != m_UDPSockets.end(); i++) {
		CLuaNetSocket* _c = *i;

		if (_c->m_ID != _id) continue;

		
		
	}

	return 0;
}

/*
	Lua param:
	int SocketID

	Closes a TCP socket
*/
int CLuaNet::Lua_closeTCPSocket(lua_State * State)
{
	int _id = lua_tointeger(State, 1);
	if (_id == 0) return 0;



	return 0;
}

/*
	Takes an int32 and returns an IP as string



*/
int CLuaNet::Lua_parseIP(lua_State * State)
{
	int input = lua_tointeger(State, 1);
	char * buff;
	inet_ntop(AF_INET, 0, input);



	return 0;
}

void CLuaNet::PushFunctions()
{
	CLuaNet::m_Init = false;

	lua_newtable(CLuaEnvironment::_LuaState);

	lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaNet::Lua_dumpUDP);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "dumpUDP");

	lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaNet::Lua_openUDPSocket);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "openUDP");

	lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaNet::Lua_closeUDPSocket);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "closeUDP");

	lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaNet::Lua_openTCPSocket);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "openTCP");

	lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaNet::Lua_closeUDPSocket);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "closeUDP");

	lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaNet::Lua_connect);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "connect");

	lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaNet::Lua_parseIP);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "parseIP");

	lua_setglobal(CLuaEnvironment::_LuaState, "net");
}

void CLuaNet::LoadFunctions()
{
}
