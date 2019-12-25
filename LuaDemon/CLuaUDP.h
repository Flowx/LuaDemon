#pragma once
#define WIN32_LEAN_AND_MEAN
#include "CLuaLib.h"
#include "CLuaEnvironment.h"

/*
	Basic UDP wrapper for Lua access

	STATUS: Seems to work fine for now, may need some polish.
*/
class CLuaUDP : CLuaLib
{
	class UDPSocket
	{
	public:
		UDPSocket(unsigned int Socket) {
			m_Socket = Socket;
		};
		unsigned short	m_IPPort = 0;	// Port number
		unsigned int	m_Socket;		// Socket reference; Local/Server Socket!!
				 int	m_LuaOnData;	// Reference to Lua function
				 bool	m_reuse;		// reuse socket after reloads
	};

	static int __tostring(lua_State * Stat);
	static int __newindex(lua_State * Stat);

	static void makeLuaObj(UDPSocket * Socket);

	static inline std::list<UDPSocket *> m_UDPSockets;

public:
	static void PushFunctions();
	static void PollFunctions();
	static void LoadFunctions();

	static int Lua_dump(lua_State * State);
	static int Lua_open(lua_State * State);
	static int Lua_close(lua_State * State);
	static int Lua_list(lua_State * State);
};

