#if _WIN32 // ignore this file on linux
#include "CLuaNet.h"
#include "PlatformCompatibility.h"
#include <Ws2tcpip.h>
#include <WinSock2.h>

// Windows specific stuff
#pragma comment(lib, "ws2_32.lib")

// checks if WSA is initialized; Initializes it if required
bool initWSA()
{
	if (CLuaNet::m_Init) return true; // already initialized

	WSADATA wsaData;

	int err = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (err != 0)
	{
		switch (err)
		{
		case WSASYSNOTREADY:
			PRINT_ERROR("ERROR: WSAStartup; WSASYSNOTREADY\n");
			break;
		case WSAVERNOTSUPPORTED:
			PRINT_ERROR("ERROR: WSAStartup; WSAVERNOTSUPPORTED\n");
			break;
		case WSAEINPROGRESS:
			PRINT_ERROR("ERROR: WSAStartup; WSAEINPROGRESS\n");
			break;
		case WSAEPROCLIM:
			PRINT_ERROR("ERROR: WSAStartup; WSAEPROCLIM\n");
			break;
		case WSAEFAULT:
			PRINT_ERROR("ERROR: WSAStartup; WSAEFAULT\n");
			break;
		default:
			PRINT_ERROR("ERROR: WSAStartup; Unknown Error; %i\n", err);
			break;
		}
		
		return false;
	}
	PRINT_DEBUG("WSAStartup successful! Version: %i.%i\n", (char)wsaData.wVersion, (char)(wsaData.wVersion >> 8)); // is always going to be 2.2 but eh
	CLuaNet::m_Init = true;
	return true;
}



// Lua param:
// string Addr, short Port, string Data
// Opens a UDP socket, sends a packet and closes the port again
// should only used for small occasional data packets
int CLuaNet::Lua_dumpUDP(lua_State * State)
{
	if (!initWSA()) return 0;

	const char * IP_Addr = lua_tostring(State, 1);
	unsigned short IP_Port = (unsigned short)lua_tointeger(State, 2);
	const char * data = lua_tostring(State, 3); // debugger doesnt see it but it gets the whole string including Nulls

	if (data == NULL || IP_Addr == NULL) return 0;

	size_t datalen = lua_strlen(State, 3);
	// max UDP size in IPv4 is 65507
	if (datalen > 65507)
	{
		PRINT_ERROR("ERROR: dumpUDP() data string is longer than 65507\n");
		return 0;
	}

	SOCKET s;

	struct sockaddr_in si_other;
	memset((char *)&si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(IP_Port);

	// indicates that something most likely went wrong while parsing; though 204... is technically a valid IP
	inet_pton(AF_INET, IP_Addr, &si_other.sin_addr.s_addr);
	if (si_other.sin_addr.S_un.S_addr == 3435973836) PRINT_WARNING("WARNING: IP Parsed as 204.204.204.204 may be invalid!\n");

	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		PRINT_ERROR("ERROR: socket() failed with error code : %d\n", WSAGetLastError());
		return 0;
	}

	int res = sendto(s, data, datalen, 0, (struct sockaddr *) &si_other, sizeof(si_other));

	closesocket(s);

	if (res != SOCKET_ERROR)
	{
		lua_pushboolean(State, 1);
		return 1;
	}
	else PRINT_ERROR("ERROR: sendto() failed with error code : %d\n", WSAGetLastError());

	return 0;
}

// Lua param:
// short Port, bool Reuse, func Callback
// Opens a receiving socket
// Callback: Called when a packet is received
int CLuaNet::Lua_openUDPSocket(lua_State * State)
{
	if (!initWSA()) return 0;

	if (!lua_isfunction(State, 3)) return 0;
	
	unsigned short IP_Port = (unsigned short)lua_tointeger(State, 1);

	bool noReuse = (bool)lua_toboolean(State, 2);
	if (!noReuse)
	{
		for (auto _s : CLuaNet::m_UDPSockets)
		{
			if (_s->m_IPPort == IP_Port)
			{
				PRINT_DEBUG("CLuaNet: Removed old hook!\n");
				luaL_unref(State, LUA_REGISTRYINDEX, _s->m_LuaReference);

				// reuse the existing socket and only attach the new function
				// function has to be last argument or this will create a wrong reference
				_s->m_LuaReference = luaL_ref(State, LUA_REGISTRYINDEX);

				lua_pushboolean(State, 1);
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

	CLuaNetSocket * _luasock = new CLuaNetSocket(s);
	_luasock->m_IPPort = IP_Port;

	if (_luasock->m_LuaReference != 0) // there already is a function hooked to this port
	{
		PRINT_DEBUG("CLuaSerial: Removed old hook!\n");
		luaL_unref(State, LUA_REGISTRYINDEX, _luasock->m_LuaReference);
	}
	_luasock->m_LuaReference = luaL_ref(State, LUA_REGISTRYINDEX);

	CLuaNet::m_UDPSockets.push_front(_luasock); // add it to the list

	lua_pushinteger(State, _luasock->m_ID);
	return 1;
}



// Lua param:
// short Port, int Backlog, func Callback, bool Reuse
// Opens a receiving socket
// Callback: Called when a new connection is initiated
int CLuaNet::Lua_openTCPSocket(lua_State * State)
{
	if (!initWSA()) return 0;

	unsigned short IP_Port = (unsigned short)lua_tointeger(State, 1);
	int SocketBacklog = lua_tointeger(State, 2);
	if (SocketBacklog < 0) SocketBacklog = 1;

	for (auto _s : CLuaNet::m_TCPSockets)
	{
		if (_s->m_IPPort == IP_Port)
		{
			PRINT_DEBUG("CLuaNet: Removed old TCP socket! (%d)\n", _s->m_IPPort);
			if (closesocket(_s->m_Socket) == SOCKET_ERROR) PRINT_ERROR("ERROR: closesocket() failed with error code : %d\n", WSAGetLastError());
			delete _s;
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
		PRINT_ERROR("ERROR: socket() failed with error code : %d\n", WSAGetLastError());
		return 0;
	}

	if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		PRINT_ERROR("ERROR: bind() failed with error code : %d\n", WSAGetLastError());
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

	if (listen(s, SocketBacklog) == SOCKET_ERROR) // open the socket for connections
	{
		PRINT_ERROR("ERROR: listen() with error code : %d\n", WSAGetLastError());
		if (closesocket(s) == SOCKET_ERROR) PRINT_ERROR("ERROR: closesocket() failed with error code : %d\n", WSAGetLastError());
		return 0;
	}

	CLuaNetSocket * _luasock = new CLuaNetSocket(s);
	_luasock->m_IPPort = IP_Port;
	CLuaNet::m_TCPSockets.push_front(_luasock); // add it to the list

	lua_pushinteger(State, _luasock->m_ID);
	return 1;
}

// Lua param:
// string Addr, short Port
// Connects to a TCP server and returns the socket if successful
int CLuaNet::Lua_connect(lua_State * State)
{
	if (!initWSA()) return 0;

	const char * IP_Addr = lua_tostring(State, 1);
	unsigned short IP_Port = (unsigned short)lua_tointeger(State, 2);

	SOCKET s;
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == SOCKET_ERROR)
	{
		PRINT_ERROR("ERROR: socket() failed with error code : %d\n", WSAGetLastError());
		return 0;
	}

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(IP_Port);
	// indicates that something most likely went wrong while parsing; though 204... is technically a valid IP
	inet_pton(AF_INET, IP_Addr, &server.sin_addr.s_addr);
	if (server.sin_addr.S_un.S_addr == 3435973836) PRINT_WARNING("WARNING: IP Parsed as 204.204.204.204 may be invalid!\n");

	if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		PRINT_ERROR("ERROR: connect() failed!\n");
		return 0;
	}

	return 0;
}



void CLuaNet::PollFunctions()
{
	char buf[4096]; // 4kB receive buffer
	for (auto  _s : CLuaNet::m_UDPSockets)
	{
		memset(buf, '\0', 4096); // clear the buffer

		if (!_s->m_LuaReference) continue; // no Lua function available

		struct sockaddr_in client;
		int slen = sizeof(client), recv_len = 0;

		if ((recv_len = recvfrom((SOCKET)_s->m_Socket, buf, 4096, 0, (struct sockaddr *) &client, &slen)) == SOCKET_ERROR)
		{
			int _err = WSAGetLastError();
			if (_err == 10035) continue; // ignore - caused by ioctlsocket setting to prevent blocking; TODO: Optimize!
			PRINT_ERROR("ERROR: recvfrom() failed with error code : %d\n", _err);
		}
		else // we received actual data
		{
			unsigned char * _IP = (unsigned char *)&client.sin_addr.s_addr;
			PRINT_DEBUG("Received something from %d.%d.%d.%d\n", _IP[0], _IP[1], _IP[2], _IP[3]);
			
			lua_rawgeti(CLuaEnvironment::_LuaState, LUA_REGISTRYINDEX, _s->m_LuaReference); // push the referenced function on the stack and pcall it

			lua_pushlstring(CLuaEnvironment::_LuaState, (const char *)&buf, recv_len);
			lua_pushinteger(CLuaEnvironment::_LuaState, client.sin_addr.s_addr);

			if (lua_pcall(CLuaEnvironment::_LuaState, 2, 0, 0)) // Some error occured
			{
				PRINT_ERROR("CALLBACK ERROR: %s\n", lua_tostring(CLuaEnvironment::_LuaState, -1));
				lua_pop(CLuaEnvironment::_LuaState, 1);
			}
		}
	}



	for (auto _s : CLuaNet::m_TCPSockets) // this only accepts connections and returns new sockets
	{
		struct sockaddr_in client;
		int slen = sizeof(client);

		SOCKET connector = accept((SOCKET)_s->m_Socket, (struct sockaddr *) &client, &slen);
		if (connector != INVALID_SOCKET)
		{
			unsigned char * _IP = (unsigned char *)&client.sin_addr.s_addr;
			PRINT_DEBUG("Connection incoming from %d.%d.%d.%d\n", _IP[0], _IP[1], _IP[2], _IP[3]); // TODO: Do this properly; Endianess!!!
			//closesocket(connector);
		}
	}
}


#endif