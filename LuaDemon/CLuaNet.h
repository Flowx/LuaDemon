#pragma once
#include "CLuaLib.h"
#include "CLuaEnvironment.h"
#include <list>


class CLuaNetSocket
{
public:
	CLuaNetSocket(int Socket);
	
	unsigned short m_IPPort = 0;


	int m_Socket;
};

class CLuaNet
{
	static int Lua_openTCPSocket(lua_State * State);
	static int Lua_openUDPSocket(lua_State * State);
	static int Lua_connect(lua_State * State);
	static int Lua_dumpUDP(lua_State * State); // shortcut to send a udp packet
	
public:
	static bool m_Init;

	static std::list<CLuaNetSocket *> m_UDPSockets;

	static void PushFunctions();
	static void PollFunctions();
	static void LoadFunctions();
};

