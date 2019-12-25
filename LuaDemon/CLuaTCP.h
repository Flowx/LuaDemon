#pragma once
#define WIN32_LEAN_AND_MEAN
#include "CLuaLib.h"
#include "CLuaEnvironment.h"

/*
	Basic UDP wrapper for Lua access

*/
class CLuaTCP : CLuaLib
{

	static unsigned int _IDCounter;

	class TCPSocket
	{
	public:
		TCPSocket(unsigned int Socket, unsigned int ID) {
			m_Socket = Socket;
			m_ID = ID;
		};
		std::list<unsigned int> m_Connections; // list of all connected entities

		unsigned int	m_ID;				// ID that gets passed to lua
		unsigned short	m_IPPort = 0;		// Port number
		unsigned int	m_Socket;			// Socket reference; Local/Server Socket!!
				 int	m_LuaOnData;		// Lua function reference
				 int	m_LuaOnConnection;	// decides whether to accept the connection
				 bool	m_reuse;			// reuse socket after reloads
	};

	class TCPConnection
	{
	public:
		TCPConnection(unsigned int Socket) {
			m_Socket = Socket;
		};

		unsigned short	m_LocalPort = 0;	// Port number of host socket
		unsigned short  m_RemotePort = 0;	// port of local socket 
		unsigned int	m_Socket;			// Socket reference; Local/Server Socket!!
		const char *	m_IPString;			// IP
	};


	static int __tostring(lua_State * Stat);
	static int __newindex(lua_State * Stat);
	static int __metatable(lua_State * Stat);

	static void makeLuaObj(TCPSocket * Socket);
	static void closeTCPSocket(short Port);
	static TCPSocket * getSocketFromID(unsigned int SocketID);

	static inline std::list<TCPSocket *> m_TCPSockets;

public:
	static void PushFunctions();
	static void PollFunctions();
	static void LoadFunctions();

	//private:
	//static int Lua_dump(lua_State * State);
	static int Lua_open(lua_State * State);
	static int Lua_close(lua_State * State);
	static int Lua_list(lua_State * State);
	static int Lua_getSocket(lua_State * State);

	// used by metatable on pseudo-objects
	static int Lua_obj_isValid(lua_State * State);
	static int Lua_obj_close(lua_State * State);
	static int Lua_obj_getPort(lua_State * State);
};

