#pragma once
#include "CLuaLib.h"
#include "CLuaEnvironment.h"
#include <list>

#if _WIN32
	//#include <WinSock2.h>
#else
	#include<arpa/inet.h>
#endif

class CLuaNetConnection // TCP only
{
public:
	CLuaNetConnection(unsigned int Socket);
	unsigned int	m_Socket; // Connection Socket reference
	unsigned int	m_RemoteIP; // the clients IP address
};



class CLuaNetSocket // may be UDP or TCP
{
public:
	CLuaNetSocket(unsigned int Socket);
	unsigned long	m_ID; // reference ID for internal handling
	unsigned short	m_IPPort = 0; // Port number
	unsigned int	m_Socket; // Socket reference
	int				m_LuaReference; // Reference to Lua function

	static unsigned long m_Counter; // incremental counter, prevents accidental re-referencing; This is also the identifier passed to Lua
	static unsigned long getID();
};



class CLuaNet
{
	static int Lua_dumpUDP(lua_State * State); // shortcut to send a udp packet
	static int Lua_openUDPSocket(lua_State * State);
	static int Lua_openTCPSocket(lua_State * State);
	static int Lua_connect(lua_State * State);
	
public:
	static bool m_Init;

	static std::list<CLuaNetSocket *> m_UDPSockets;
	static std::list<CLuaNetSocket *> m_TCPSockets; // listening sockets; NOT connections!

	static void PushFunctions();
	static void PollFunctions();
	static void LoadFunctions();
};

