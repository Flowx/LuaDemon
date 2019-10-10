#include "CLuaUDP.h"
#include "PlatformCompatibility.h"
#include "CLuaEnvironment.h"

#if _WIN32
	#include "CWSAHandler.h"
	#include <Ws2tcpip.h>
	#include <WinSock2.h>
	#include <string>
#else
	#include <arpa/inet.h>
	#include <cstring>

	#define SOCKET_ERROR (-1)
	typedef UINT_PTR SOCKET
#endif

#define CLUAUDP_CLASSNAME "socket_udp"

#pragma region Lua linking

void CLuaUDP::PollFunctions()
{
	char buf[4096]; // 4kB receive buffer

	for (auto _s : CLuaUDP::m_UDPSockets)
	{
		memset(buf, '\0', 4096); // clear the buffer

		if (!_s->m_LuaOnData) continue; // no Lua function available

		struct sockaddr_in client;
		int slen = sizeof(client), recv_len = 0;

		if ((recv_len = recvfrom((SOCKET)_s->m_Socket, buf, 4096, 0, (struct sockaddr *) &client, &slen)) == SOCKET_ERROR)
		{
			int _err = WSAGetLastError();
			if (_err == 10035) continue; // ignore - caused by ioctlsocket setting to prevent blocking; TODO: Optimize!
			PRINT_ERROR("ERROR: recvfrom() failed with WSA error code : %d\n", _err);
		}
		else // we received actual data
		{
			//unsigned char * _IP = (unsigned char *)&client.sin_addr.s_addr;
			//PRINT_DEBUG("Received something from %d.%d.%d.%d\n", _IP[0], _IP[1], _IP[2], _IP[3]);

			lua_rawgeti(CLuaEnvironment::_LuaState, LUA_REGISTRYINDEX, _s->m_LuaOnData); // push the referenced function on the stack and pcall it

			lua_pushlstring(CLuaEnvironment::_LuaState, (const char *)&buf, recv_len);
			//lua_pushinteger(CLuaEnvironment::_LuaState, client.sin_addr.s_addr);

			char str[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(client.sin_addr), str, INET_ADDRSTRLEN);
			lua_pushstring(CLuaEnvironment::_LuaState, str);

			if (lua_pcall(CLuaEnvironment::_LuaState, 2, 0, 0)) // Some error occured
			{
				PRINT_ERROR("CALLBACK ERROR: %s\n", lua_tostring(CLuaEnvironment::_LuaState, -1));
				lua_pop(CLuaEnvironment::_LuaState, 1);
			}
		}
	}
}

void CLuaUDP::PushFunctions()
{
	lua_newtable(CLuaEnvironment::_LuaState);

	lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaUDP::Lua_dump);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "dump");

	lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaUDP::Lua_open);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "open");

	lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaUDP::Lua_close);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "close");

	lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaUDP::Lua_list);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "list");

	lua_setglobal(CLuaEnvironment::_LuaState, "udp");
}

void CLuaUDP::LoadFunctions()
{
}

#pragma endregion

#pragma region Lua Metatable Functions
/*
	used by makeLuaObj
*/
int __tostring(lua_State * State)
{
	lua_getfield(State, -1, "port"); // looks for the table key and pushes the value on the stack

	char _buff[32];
	sprintf(_buff, "%s[%i]", CLUAUDP_CLASSNAME, (int)lua_tointeger(State, -1));

	lua_pushstring(State, _buff);
	return 1;
}

/*
	used by makeLuaObj

	currently broken!
*/
int __eq(lua_State * State)
{
	PRINT_DEBUG("Called");

	//lua_getfield(State, -1, "socket");

	//int a = lua_tointeger(State, -1);

	lua_pushboolean(State, TRUE);
	return 1;
}

/*
	prevents accidental access
*/
int __newindex(lua_State * State)
{
	PRINT_DEBUG("Attempted to write protected table! [%s] \n", CLUAUDP_CLASSNAME);

	return 0;
}

#pragma endregion

/*
	Turns an UDPSocket into a Lua "object"

	TODO: Currently every object gets its own metatable!
		  This means functions like __eq wont work!
*/
void CLuaUDP::makeLuaObj(lua_State * State, UDPSocket * Socket)
{
	lua_newtable(State); // every list entry is actually a table

	// Properties/members
	lua_pushnumber(State, Socket->m_IPPort);
	lua_setfield(State, -2, "port");

	//lua_pushnumber(State, Socket->m_LuaOnData); // kind of useless in lua..
	//lua_setfield(State, -2, "luaref");

	lua_pushnumber(State, Socket->m_Socket);
	lua_setfield(State, -2, "socket");

	// Metatable
	lua_newtable(State);
	
	//lua_pushcfunction(State, __eq);
	//lua_setfield(State, -2, "__eq");

	lua_pushcfunction(State, __tostring);
	lua_setfield(State, -2, "__tostring");

	lua_pushcfunction(State, __newindex);
	lua_setfield(State, -2, "__newindex");

	lua_pushstring(State, CLUAUDP_CLASSNAME);
	lua_setfield(State, -2, "__type");

	lua_setmetatable(State, -2);
}

// Lua functions

/*
Lua param:
	string addr, short port, string data

	Standalone function. Sends a string using UDP.
	Opens its own socket and closes it afterwards.

	Note: Usually very reliable unless you feed it stupid IP Addresses or Ports

	nil = udp.dump(address, port, data)
*/
int CLuaUDP::Lua_dump(lua_State * State)
{
#if _WIN32
	if (!CWSAHandler::initWSA()) return 0;
#endif

	const char * IP_Addr = lua_tostring(State, 1);
	unsigned short IP_Port = (unsigned short)lua_tointeger(State, 2);
	const char * data = lua_tostring(State, 3); // debugger doesnt see it but it gets the whole string including Nulls

	if (data == NULL || IP_Addr == NULL) return 0;

	size_t datalen = lua_strlen(State, 3);
	// max UDP size in IPv4 is 65507
	if (datalen > 65507)
	{
		PRINT_ERROR("ERROR: udp.dump() data string is longer than 65507\n");
		return 0;
	}

	SOCKET s;

	struct sockaddr_in si_other;
	memset((char *)&si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(IP_Port);

	if (strlen(IP_Addr) == 9 && tolower(IP_Addr[0]) == 'l' && tolower(IP_Addr[8]) == 't') // checks if ip is "localhost"
		si_other.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	else
		inet_pton(AF_INET, IP_Addr, &si_other.sin_addr.s_addr);

	// indicates that something most likely went wrong while parsing; though 204... is technically a valid IP
	if (si_other.sin_addr.S_un.S_addr == 3435973836) PRINT_WARNING("WARNING: IP Parsed as 204.204.204.204 may be invalid!\n");
	if (si_other.sin_addr.S_un.S_addr == 0)
	{
		PRINT_ERROR("ERROR: udp.dump failed to parse IP.\n");
		return 0;
	}

	// create the socket and send the data

	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
#if _WIN32
		PRINT_ERROR("ERROR: socket() failed with error code : %d\n", WSAGetLastError());
#else
		PRINT_ERROR("ERROR: socket() failed");
#endif
		return 0;
	}

	int res = sendto(s, data, (int)datalen, 0, (struct sockaddr *) &si_other, sizeof(si_other));

	closesocket(s);

	if (res != SOCKET_ERROR)
	{
		lua_pushboolean(State, 1);
		return 1;
	}
#if _WIN32
	else PRINT_ERROR("ERROR: sendto() failed with error code : %d\n", WSAGetLastError());
#else
	else PRINT_ERROR("ERROR: sendto() failed with error code : %d\n", res);
#endif
	return 0;
}

/* 
Lua param:
	short Port, bool Reuse, func Callback

	Opens a receiving socket
	Callback: Called when a packet is received

	nil = udp.open(port, reuse_on_reload, callback(data, ip))
*/
int CLuaUDP::Lua_open(lua_State * State)
{
#if _WIN32
	if (!CWSAHandler::initWSA()) return 0;
#endif

	if (!lua_isfunction(State, 3)) return 0;

	unsigned short IP_Port = (unsigned short)lua_tointeger(State, 1);
	if (IP_Port == 0) {
		PRINT_ERROR("ERROR: udp.open received 0 as port number!\n");
		return 0;
	}

	if (!(bool)lua_toboolean(State, 2)) // this just removes the old callback and attaches the new one
	{
		for (auto _s : CLuaUDP::m_UDPSockets)
		{
			if (_s->m_IPPort == IP_Port)
			{
				PRINT_DEBUG("CLuaUDP: Removed old hook!\n");
				luaL_unref(State, LUA_REGISTRYINDEX, _s->m_LuaOnData);

				// reuse the existing socket and only attach the new function
				// function has to be last argument or this will create a wrong reference
				_s->m_LuaOnData = luaL_ref(State, LUA_REGISTRYINDEX);

				makeLuaObj(State, _s);
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
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == SOCKET_ERROR)
	{
		PRINT_ERROR("ERROR: socket() failed with error code : %d\n", WSAGetLastError());
		return 0;
	}

	if (bind(s, (sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		PRINT_ERROR("ERROR: bind() failed with error code : %d\n\tTerminating Socket...\n", WSAGetLastError());
		if (closesocket(s) == SOCKET_ERROR) PRINT_ERROR("ERROR: closesocket() failed with error code : %d\n", WSAGetLastError());
		return 0;
	}

	unsigned long NonBlock = 1; // used in polling mode; allows using only one thread for all ports
	if (ioctlsocket(s, FIONBIO, &NonBlock) == SOCKET_ERROR)
	{
		PRINT_ERROR("ERROR: ioctlsocket() failed with error code : %d\n\tTerminating Socket...\n", WSAGetLastError());
		if (closesocket(s) == SOCKET_ERROR) PRINT_ERROR("ERROR: closesocket() failed with error code : %d\n", WSAGetLastError());
		return 0;
	}

	//most of the input and error handling is already done by WSA

	UDPSocket * _luasock = new UDPSocket(s);
	_luasock->m_IPPort = IP_Port;
	_luasock->m_Socket = (unsigned int)s; // keep the socket refernce

	if (_luasock->m_LuaOnData != 0) // there already is a function hooked to this port
	{
		PRINT_DEBUG("CLuaUDP: Removed old hook!\n");
		luaL_unref(State, LUA_REGISTRYINDEX, _luasock->m_LuaOnData);
	}
	_luasock->m_LuaOnData = luaL_ref(State, LUA_REGISTRYINDEX); // attach the callback
	
	CLuaUDP::m_UDPSockets.push_front(_luasock); // add the socket to the list

	makeLuaObj(State, _luasock);
	return 1;
}

/*
Lua param:
	table UDPSocket
		OR
	short Port

	Closes a socket
	
	nil = udp.close(socket)
	nil = udp.close(port)
*/
int CLuaUDP::Lua_close(lua_State * State)
{
	if (CLuaUDP::m_UDPSockets.size() == 0) return 0; // there is no active sockets to close...

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

	// for loop because we need to change the length of the list while iterating
	for (auto i = CLuaUDP::m_UDPSockets.begin(); i != CLuaUDP::m_UDPSockets.end(); i++) {
		UDPSocket * _s = *i;

		if (_port == _s->m_IPPort) //gotcha
		{
			luaL_unref(State, LUA_REGISTRYINDEX, _s->m_LuaOnData); // remove the lua hook
			closesocket(_s->m_Socket); // close the socket
			delete _s;
			i = CLuaUDP::m_UDPSockets.erase(i); // remove the socket from the list
			//if (i != CLuaUDP::m_UDPSockets.end()) i++; else break; // we dont need to contiue after this
			break;
		}
	}

	return 0;
}

/*
Lua param:
	nil

	Returns a table with all active Lua UDP Sockets

	table = udp.list()
*/
int CLuaUDP::Lua_list(lua_State * State)
{
	lua_newtable(State); // the 'list' to return

	int c = 0;
	for (auto _s : CLuaUDP::m_UDPSockets)
	{
		lua_pushnumber(State, ++c);
		
		makeLuaObj(State, _s); // this pushes a table on the stack

		// push this table into the list
		lua_settable(State, -3);
	}

	return 1;
}

