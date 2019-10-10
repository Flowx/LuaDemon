#pragma once
#define WIN32_LEAN_AND_MEAN
#include "CLuaLib.h"
#include "CLuaEnvironment.h"

/*
	Basic UDP wrapper for Lua access

*/
class CLuaUDP : CLuaLib
{
	class UDPSocket
	{
	public:
		UDPSocket(unsigned int Socket) {
			m_Socket = Socket;
			//m_ID = 0; // getID();
		};
		//unsigned long	m_ID; // reference ID for internal handling
		unsigned short	m_IPPort = 0; // Port number
		unsigned int	m_Socket; // Socket reference; Local/Server Socket!!
		int	m_LuaOnData; // Reference to Lua function
	};

	static void makeLuaObj(lua_State * State, UDPSocket * Socket);

	static inline std::list<UDPSocket *> m_UDPSockets;

public:
	static void PushFunctions();
	static void PollFunctions();
	static void LoadFunctions();

//private:
	static int Lua_dump(lua_State * State);
	static int Lua_open(lua_State * State);
	static int Lua_close(lua_State * State);
	static int Lua_list(lua_State * State);
};

