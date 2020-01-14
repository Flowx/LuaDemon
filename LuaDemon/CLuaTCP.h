#pragma once
#define WIN32_LEAN_AND_MEAN
#include "CLuaLib.h"
#include "CLuaEnvironment.h"

/*
	Basic UDP wrapper for Lua TCP access

	TODO: Add "connect"
*/
class CLuaTCP : CLuaLib
{
	static unsigned int _IDCounter;
	
public: 
	class TCPConnection // NOTE: Expose this to lua?
	{
	public:
		TCPConnection(unsigned int Socket, unsigned int ID) {
			m_Socket = Socket;
			m_ID = ID;
		};

		unsigned short	m_LocalPort = 0;	// Port number of host socket
		unsigned short  m_RemotePort = 0;	// port of local socket 
		unsigned int	m_Socket;			// Socket reference; Local/Server Socket!!
		unsigned int	m_ID;				// duh
				 char	m_IPString[16];		// IP as string
				 char	m_Identifier[64];	// 64 char string, can be used by lua to uniquely identify this. Persistent over reloads
				 int	m_LuaOnData;
		
		static void makeLuaObj(TCPConnection * Socket);
	};

	class TCPSocket
	{
	public:
		TCPSocket(unsigned int Socket, unsigned int ID) {
			m_Socket = Socket;
			m_ID = ID;
		};
		std::list<TCPConnection *> m_Connections; // list of all connected entities

		TCPConnection * getConnectionFromID(unsigned int ConnectionID);

		unsigned int	m_ID;				// ID that gets passed to lua
		unsigned short	m_IPPort = 0;		// Port number
		unsigned int	m_Socket;			// Socket reference; Local/Server Socket!!
				 int	m_LuaOnData;		// Lua function reference; not referenced here - see tcpconnection
				 //int	m_LuaOnConnection;	// decides whether to accept the connection
				 bool	m_reuse;			// reuse socket after reloads

		static void makeLuaObj(TCPSocket * Socket);
	};

private:
	static int __tostring_socket(lua_State * Stat);
	static int __tostring_connection(lua_State * Stat);
	static int __newindex(lua_State * Stat);
	static int __metatable(lua_State * Stat);

	static void closeTCPSocket(short Port);
	static TCPSocket * getSocketFromID(unsigned int SocketID);
	static TCPConnection * getConnectionFromID(unsigned int ConnectionID); // expensive/slow function!

	static inline std::list<TCPSocket *> m_TCPSockets;
	//static inline std::list<TCPConnection *> m_TCPConnections;

public:
	static void PushFunctions();
	static void PollFunctions();
	static void LoadFunctions();

	// lua functions on the tcp. global table
	static int Lua_open(lua_State * State);
	static int Lua_close(lua_State * State);
	static int Lua_list(lua_State * State);
	static int Lua_getSocket(lua_State * State);
	
	// used by the pseudo-object
	static int Lua_socket_isValid(lua_State * State);
	static int Lua_socket_close(lua_State * State);
	static int Lua_socket_getPort(lua_State * State);
	static int Lua_socket_list(lua_State * State);
};

