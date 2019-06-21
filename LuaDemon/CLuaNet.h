#pragma once
#include "CLuaLib.h"
#include "CLuaEnvironment.h"
#include <list>

#if _WIN32
	#include <WinSock2.h>
	#include <Ws2tcpip.h>
#else
	#include<arpa/inet.h>
#endif

class CLuaNet : CLuaLib
{
public:

	class CLuaNetClient // TCP only
	{
	public:
		CLuaNetClient(unsigned int Socket);
		unsigned int	m_Socket; // Connection Socket reference
		unsigned int	m_RemoteIP; // the clients IP address
	};




	class CLuaNetSocket // may be UDP or TCP; These are Server sockets!
	{
	public:
		CLuaNetSocket(unsigned int Socket);
		unsigned long	m_ID; // reference ID for internal handling
		unsigned short	m_IPPort = 0; // Port number
		unsigned int	m_Socket; // Socket reference; Server Socket!!
				 int	m_LuaOnData; // Reference to Lua function

		std::list<CLuaNetClient *> m_Clients; // connected clients on that local *server* socket
		static std::list<CLuaNetSocket *> _Connections; // connections to remote servers

		static unsigned long m_Counter; // incremental counter, prevents accidental re-referencing; This is also the identifier passed to Lua
		static unsigned long getID();
	};




private:
	static int Lua_openUDPSocket(lua_State * State);
	static int Lua_closeUDPSocket(lua_State * State);
	static int Lua_dumpUDP(lua_State * State); // shortcut to send a udp packet

	static int Lua_openTCPSocket(lua_State * State);
	static int Lua_closeTCPSocket(lua_State * State);
	static int Lua_connect(lua_State * State);

	static int Lua_parseIP(lua_State * State);

public:
	static bool m_Init;

	static std::list<CLuaNetSocket *> m_UDPSockets;
	static std::list<CLuaNetSocket *> m_TCPSockets; // listening sockets; NOT connections!

	static void PushFunctions();
	static void PollFunctions();
	static void LoadFunctions();
};
