#include "CLuaTCP.h"
#include "PlatformCompatibility.h"
#include "CLuaEnvironment.h"

#if _WIN32
	#include "CWSAHandler.h"
	#include <Ws2tcpip.h>
	#include <WinSock2.h>
	#include <string>
#else
	#include <arpa/inet.h>
	#include <sys/socket.h>
	#include <fcntl.h>
	#include <cstring>

	#define TRUE 1
	#define FALSE 0

	#define SOCKET_ERROR (-1)

	typedef int SOCKET;
#endif

#define CLUATCP_CLASSNAME "socket_tcp"

unsigned int CLuaTCP::_IDCounter = 0;

#pragma region Lua linking
	void CLuaTCP::PollFunctions()
	{
		char buf[4096]; // 4kB receive buffer

		for (auto _s : CLuaTCP::m_TCPSockets)
		{
			memset(buf, '\0', 4096); // clear the buffer

			if (!_s->m_LuaOnData) continue; // no Lua function available

			struct sockaddr_in client;
			int slen = sizeof(client);

			// new connection incoming
			SOCKET connector = accept((SOCKET)_s->m_Socket, (struct sockaddr *) &client, &slen);
			if (connector != INVALID_SOCKET)
			{
				unsigned char * _IP = (unsigned char *)&client.sin_addr.s_addr;

				unsigned long NonBlock = 1; // prevents blocking
				if (ioctlsocket(connector, FIONBIO, &NonBlock) == SOCKET_ERROR)
				{
					PRINT_ERROR("ERROR: ioctlsocket() failed with error code : %d\n\tTerminating Socket...\n", WSAGetLastError());
					if (closesocket(connector) == SOCKET_ERROR) PRINT_ERROR("ERROR: closesocket() failed with error code : %d\n", WSAGetLastError());
					continue;
				}

				PRINT_DEBUG("Connection incoming from %d.%d.%d.%d\n", _IP[0], _IP[1], _IP[2], _IP[3]); // TODO: Do this properly; Endianess!!!
				/*
	#pragma region Call Lua function
				if (_s->m_LuaReference != NULL)
				{
					lua_rawgeti(CLuaEnvironment::_LuaState, LUA_REGISTRYINDEX, _s->m_LuaReference); // push the referenced function on the stack and pcall it

					lua_pushinteger(CLuaEnvironment::_LuaState, client.sin_addr.s_addr);

					if (lua_pcall(CLuaEnvironment::_LuaState, 1, 1, 0)) // Some error occured
					{
						PRINT_ERROR("CALLBACK ERROR: %s\n", lua_tostring(CLuaEnvironment::_LuaState, -1));
						lua_pop(CLuaEnvironment::_LuaState, 1);
					}

					bool a = (bool)lua_toboolean(CLuaEnvironment::_LuaState, 1);
				}
				//closesocket(connector);
	#pragma endregion
				*/

				// simply accept all connections right now
				//CLuaNetClient * _clientsock = new CLuaNetClient(connector);
				//_clientsock->m_RemoteIP = client.sin_addr.s_addr;
				//_s->m_Clients.push_front(_clientsock); // add it to the list
				//PRINT_DEBUG("Active connections on Port %i:   %i#\n", _s->m_IPPort, _s->m_Clients.size());
				
				_s->m_Connections.push_front(connector);
			}

		}
	}
	
	void CLuaTCP::PushFunctions()
	{
		lua_newtable(CLuaEnvironment::_LuaState);

		lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaTCP::Lua_list);
		lua_setfield(CLuaEnvironment::_LuaState, -2, "list");

		lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaTCP::Lua_open);
		lua_setfield(CLuaEnvironment::_LuaState, -2, "open");

		lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaTCP::Lua_close);
		lua_setfield(CLuaEnvironment::_LuaState, -2, "close");

		lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaTCP::Lua_getSocket);
		lua_setfield(CLuaEnvironment::_LuaState, -2, "getSocket");

		lua_setglobal(CLuaEnvironment::_LuaState, "tcp");
	}

	void CLuaTCP::LoadFunctions()
	{
	}
#pragma endregion

#pragma region Lua Metatable Functions
/*
	used by makeLuaObj
*/
int CLuaTCP::__tostring(lua_State * State)
{
	lua_getfield(State, -1, "socketid"); // looks for the table key and pushes the value on the stack
	
	unsigned int _id = (int)lua_tointeger(State, -1);

	TCPSocket * _sock = getSocketFromID(_id);
	
	char _buff[32];
	if (_sock == nullptr) {
		sprintf(_buff, "%s[%i][INV]", CLUATCP_CLASSNAME, _id);
	} else {
		sprintf(_buff, "%s[%i]", CLUATCP_CLASSNAME, _id);
	}

	lua_pushstring(State, _buff);
	return 1;
}

/*
	prevents accidental access
*/
int CLuaTCP::__newindex(lua_State * State)
{
	PRINT_DEBUG("Attempted to write protected table! [%s] \n", CLUATCP_CLASSNAME);
	return 0;
}

/*
	prevents accidental fuckery with the metatable
*/
int CLuaTCP::__metatable(lua_State * State)
{
	PRINT_DEBUG("Attempted to accesss metatable! [%s] \n", CLUATCP_CLASSNAME);
	return 0;
}

#pragma endregion

/*
	Turns an TCPSocket into a Lua "object"

	TODO: Currently every object gets its own metatable!
			This means functions like __eq wont work!
*/
void CLuaTCP::makeLuaObj(TCPSocket * Socket)
{
	lua_State * State = CLuaEnvironment::_LuaState;

	lua_newtable(State);

	lua_pushnumber(State, Socket->m_ID);
	lua_setfield(State, -2, "socketid");

	// object functions
	lua_pushcfunction(State, Lua_obj_close);
	lua_setfield(State, -2, "close");

	lua_pushcfunction(State, Lua_obj_getPort);
	lua_setfield(State, -2, "getPort");

	lua_pushcfunction(State, Lua_obj_isValid);
	lua_setfield(State, -2, "isValid");

	// Metatable
	lua_newtable(State);

		lua_pushcfunction(State, __tostring);
		lua_setfield(State, -2, "__tostring");

		lua_pushcfunction(State, __newindex);
		lua_setfield(State, -2, "__newindex");
		
		lua_pushcfunction(State, __metatable);
		lua_setfield(State, -2, "__metatable");

		lua_pushstring(State, CLUATCP_CLASSNAME);
		lua_setfield(State, -2, "__type");

	lua_setmetatable(State, -2);
}

/*
	shortcut function

	closes a TCP socket and all connections to it,
	removes it from the list and then deletes it
*/
void CLuaTCP::closeTCPSocket(short _port)
{
	// for loop because we need to change the length of the list while iterating
	for (auto i = CLuaTCP::m_TCPSockets.begin(); i != CLuaTCP::m_TCPSockets.end(); i++) {
		TCPSocket * _s = *i;

		if (_port == _s->m_IPPort) // gotcha
		{
			luaL_unref(CLuaEnvironment::_LuaState, LUA_REGISTRYINDEX, _s->m_LuaOnData); // remove the lua hook

			int count = 0;
#if _WIN32
			closesocket(_s->m_Socket);
			for each (SOCKET s in _s->m_Connections)
			{
				closesocket(s);
				count++;
			}
#else
			close(_s->m_Socket);
			for each (SOCKET s in _s->m_Connections)
			{
				close(s);
				count++;
			}
#endif
			if (count > 0) PRINT_DEBUG("CLuaTCP: Closed %i connections on port %i\n", count, _s->m_IPPort);

			// TODO: delete the lua table

			delete _s;
			i = CLuaTCP::m_TCPSockets.erase(i); // remove the socket from the list
			break;
		}
	}
}

CLuaTCP::TCPSocket * CLuaTCP::getSocketFromID(unsigned int SocketID)
{
	for each (TCPSocket * _s in CLuaTCP::m_TCPSockets)
	{
		if (_s->m_ID == SocketID) return _s;
	}

	return nullptr;
}

/*
Lua param:
	nil

	Returns a table with all open TCP Sockets

	table = tcp.list()
*/
int CLuaTCP::Lua_list(lua_State * State)
{
	lua_newtable(State); // the 'list' to return

	int c = 0;
	for (auto _s : CLuaTCP::m_TCPSockets)
	{
		lua_pushnumber(State, ++c);

		makeLuaObj(_s); // this pushes a lua table on the stack

		// push this table into the list
		lua_settable(State, -3);
	}

	return 1;
}

/*
Lua param:
	int SocketID

	Returns a socket object from ID (as integer)

	Useful if you lost your object, idiot.

	socket_tcp[] = tcp.getSocket(id)
*/
int CLuaTCP::Lua_getSocket(lua_State * State)
{
	unsigned int _id = lua_tointeger(State, 1);

	TCPSocket * _s = getSocketFromID(_id);

	if (_s == nullptr)
	{
		PRINT_DEBUG("CLuaTCP: No socket found for %i\n", _id);
		return 0;
	}

	makeLuaObj(_s);

	return 1;
}

/*
Lua param:
	short Port, bool Reuse, func DataCallback

	Warning: Setting reuse=false will cause all connections to close when reloading!

	Note: Accept connection callback has to be added later as only one function can be passed at once

	Opens a receiving socket
	Callback: Called when a packet is received

	nil = tcp.open(port, reuse_on_reload, callback(data, ip))
*/
int CLuaTCP::Lua_open(lua_State * State)
{

#if _WIN32
	if (!CWSAHandler::initWSA()) return 0;
#endif

	if (!lua_isfunction(State, 3)) return 0;

	unsigned short IP_Port = (unsigned short)lua_tointeger(State, 1);
	if (IP_Port == 0) {
		PRINT_ERROR("ERROR: tcp.open received 0 as port number!\n");
		return 0;
	}

	bool _reuse = (bool)lua_toboolean(State, 2);
	if (!_reuse) // this just removes the old callback and attaches the new one
	{
		for (auto _s : CLuaTCP::m_TCPSockets)
		{
			if (_s->m_IPPort == IP_Port)
			{
				PRINT_DEBUG("CLuaTCP: Removed old hooks!\n");
				luaL_unref(State, LUA_REGISTRYINDEX, _s->m_LuaOnData);

				// reuse the existing socket and only attach the new function
				// function has to be last argument or this will create a wrong reference
				_s->m_LuaOnData = luaL_ref(State, LUA_REGISTRYINDEX);

				makeLuaObj(_s);
				return 1;
			}
		}
	}

	sockaddr_in server;
	memset((char *)&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(IP_Port);

	SOCKET s;
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == SOCKET_ERROR)
	{
#if _WIN32
		PRINT_ERROR("ERROR: socket() failed with WSA error code : %d\n", WSAGetLastError());
#else
		PRINT_ERROR("ERROR: socket() failed with error code : %d\n", errno);
#endif
		return 0;
	}

	if (bind(s, (sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
#if _WIN32
		PRINT_ERROR("ERROR: bind() failed with error code : %d\n\tTerminating Socket...\n", WSAGetLastError());
		if (closesocket(s) == SOCKET_ERROR) PRINT_ERROR("ERROR: closesocket() failed with WSA error code : %d\n", WSAGetLastError());
#else
		PRINT_ERROR("ERROR: bind() failed with error code : %d\n\tTerminating Socket...\n", errno);
		if (close(s) == SOCKET_ERROR) PRINT_ERROR("ERROR: close() failed with error code : %d\n", errno);
#endif
		return 0;
	}

#if	_WIN32
	unsigned long NonBlock = 1; // used in polling mode; allows using only one thread for all ports
	if (ioctlsocket(s, FIONBIO, &NonBlock) == SOCKET_ERROR)
	{
		PRINT_ERROR("ERROR: ioctlsocket() failed with error code : %d\n\tTerminating Socket...\n", WSAGetLastError());
		if (closesocket(s) == SOCKET_ERROR) PRINT_ERROR("ERROR: closesocket() failed with WSA error code : %d\n", WSAGetLastError());
		return 0;
	}
#else

	// prevent blocking
	if (fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK) == -1)
	{
		PRINT_ERROR("ERROR: fcntl() O_NONBLOCK failed: %s\n\tTerminating Socket...\n", strerror(errno));
		if (close(s) == -1) PRINT_ERROR("ERROR: close() failed.\n");
		return 0;
	}
#endif

	// start listening on it
	if (listen(s, 0) == SOCKET_ERROR) // open the socket for connections
	{
#if	_WIN32
		PRINT_ERROR("ERROR: listen() with error code : %d\n", WSAGetLastError());
		if (closesocket(s) == SOCKET_ERROR) PRINT_ERROR("ERROR: closesocket() failed with error code : %d\n", WSAGetLastError());
#else
		PRINT_ERROR("ERROR: fcntl() O_NONBLOCK failed: %s\n\tTerminating Socket...\n", strerror(errno));
		if (close(s) == -1) PRINT_ERROR("ERROR: close() failed.\n");
#endif
		return 0;
	}

	//most of the input and error handling is already done by WSA


	// generates a ongoing id for reference in lua
	if (_IDCounter == 0)
		_IDCounter = (unsigned int)((rand() % 3000) + 3000); // dont start off at 0
	else
		_IDCounter++;

	TCPSocket * _luasock = new TCPSocket(s, _IDCounter);
	_luasock->m_IPPort = IP_Port;
	_luasock->m_Socket = (unsigned int)s; // keep the socket refernce
	_luasock->m_reuse  = _reuse;

	if (_luasock->m_LuaOnData != 0) // there already is a function hooked to this port
	{
		PRINT_DEBUG("CLuaTCP: Removed old hook!\n");
		luaL_unref(State, LUA_REGISTRYINDEX, _luasock->m_LuaOnData);
	}
	_luasock->m_LuaOnData = luaL_ref(State, LUA_REGISTRYINDEX); // attach the callback

	CLuaTCP::m_TCPSockets.push_front(_luasock); // add the socket to the list

	makeLuaObj(_luasock);
	return 1;
}

/*
Lua param:
	short Port

	Closes a TCP socket

	nil = tcp.close(port)
*/
int CLuaTCP::Lua_close(lua_State * State)
{
	if (CLuaTCP::m_TCPSockets.size() == 0) return 0; // there is no active sockets to close...

#if _WIN32
	if (!CWSAHandler::initWSA()) return 0;
#endif

	short _port = -1;

	if (lua_isnumber(State, 1))
	{
		_port = lua_tonumber(State, 1);
	}
	else if (lua_istable(State, 1))
	{
		lua_getfield(State, -1, "port"); // looks for the table key and pushes the value on the stack
		_port = lua_tonumber(State, -1);
	}
	else return 0; // invalid argument

	closeTCPSocket(_port);

	return 0;
}


// lua object functions
/*
	Checks if the socket is still valid

	socket:isValid()
*/
int CLuaTCP::Lua_obj_isValid(lua_State * State)
{
	lua_getfield(State, -1, "socketid");

	unsigned int _id = (int)lua_tointeger(State, -1);

	TCPSocket * _sock = getSocketFromID(_id);
	
	lua_pushboolean(State, (_sock != nullptr));

	return 1;
}

/*
	Closes a TCP socket

	socket:close()
*/
int CLuaTCP::Lua_obj_close(lua_State * State)
{
	lua_getfield(State, -1, "socketid");

	unsigned int _id = (int)lua_tointeger(State, -1);

	TCPSocket * _sock = getSocketFromID(_id);
	if (_sock == nullptr) return 0;

	closeTCPSocket(_sock->m_IPPort);

	return 0;
}

/*
	Returns the TCP port number, eg. 443

	socket:getPort()
*/
int CLuaTCP::Lua_obj_getPort(lua_State * State)
{
	lua_getfield(State, -1, "socketid");

	unsigned int _id = (int)lua_tointeger(State, -1);

	TCPSocket * _sock = getSocketFromID(_id);
	if (_sock == nullptr) return 0;

	lua_pushinteger(State, _sock->m_IPPort);

	return 1;
}