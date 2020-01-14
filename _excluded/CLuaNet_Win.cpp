#if _WIN32 // ignore this file on linux
#include "CLuaNet.h"
#include "PlatformCompatibility.h"

// Windows specific stuff
#pragma comment(lib, "ws2_32.lib")

std::list<CLuaNet::CLuaNetSocket *> CLuaNet::CLuaNetSocket::_Connections;

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
	PRINT_DEBUG("WSAStartup successful! Version: %i.%i\n", (char)wsaData.wVersion, (char)(wsaData.wVersion >> 8)); // its always going to be 2.2 but eh
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

	int res = sendto(s, data, (int)datalen, 0, (struct sockaddr *) &si_other, sizeof(si_other));

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
	if (IP_Port == 0) {
		PRINT_DEBUG("openUDPSocket received 0 as port number!\n");
		return 0;
	}

	bool noReuse = (bool)lua_toboolean(State, 2);
	if (!noReuse)
	{
		for (auto _s : CLuaNet::m_UDPSockets)
		{
			if (_s->m_IPPort == IP_Port)
			{
				PRINT_DEBUG("CLuaNet: Removed old hook!\n");
				luaL_unref(State, LUA_REGISTRYINDEX, _s->m_LuaOnData);

				// reuse the existing socket and only attach the new function
				// function has to be last argument or this will create a wrong reference
				_s->m_LuaOnData = luaL_ref(State, LUA_REGISTRYINDEX);

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

	if (_luasock->m_LuaOnData != 0) // there already is a function hooked to this port
	{
		PRINT_DEBUG("CLuaSerial: Removed old hook!\n");
		luaL_unref(State, LUA_REGISTRYINDEX, _luasock->m_LuaOnData);
	}
	_luasock->m_LuaOnData = luaL_ref(State, LUA_REGISTRYINDEX);

	CLuaNet::m_UDPSockets.push_front(_luasock); // add it to the list

	lua_pushinteger(State, _luasock->m_ID);
	return 1;
}

// Lua param:
// short Port, *bool Reuse,* func DataCallback, func AcceptCallback
// returns: socket ID
// Opens a receiving socket
// Callback: Called when a new connection is initiated
int CLuaNet::Lua_openTCPSocket(lua_State * State)
{
	if (!initWSA()) return 0;

	if (!lua_isfunction(State, 2)) return 0;

	unsigned short IP_Port = (unsigned short)lua_tointeger(State, 1);
	if (IP_Port == 0) {
		PRINT_DEBUG("openTCPSocket received 0 as port number!\n");
		return 0;
	}

	//int SocketBacklog = lua_tointeger(State, 2);
	//if (SocketBacklog < 0) SocketBacklog = 1;

	for (auto i = CLuaNet::m_TCPSockets.begin(); i != CLuaNet::m_TCPSockets.end(); i++) {
		auto _s = *i;
	//for (auto _s : CLuaNet::m_TCPSockets)
	//{
		if (_s->m_IPPort == IP_Port)
		{
			PRINT_DEBUG("CLuaNet: Removed old TCP socket! (%d)\n", _s->m_IPPort);
			if (closesocket(_s->m_Socket) == SOCKET_ERROR) PRINT_ERROR("ERROR: closesocket() failed with error code : %d\n", WSAGetLastError());
			delete _s;

			i = CLuaNet::m_TCPSockets.erase(i);
			if (i !=  CLuaNet::m_TCPSockets.end()) i++; else break;
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

	//if (listen(s, SocketBacklog) == SOCKET_ERROR) // open the socket for connections
	if (listen(s, 0) == SOCKET_ERROR) // open the socket for connections
	{
		PRINT_ERROR("ERROR: listen() with error code : %d\n", WSAGetLastError());
		if (closesocket(s) == SOCKET_ERROR) PRINT_ERROR("ERROR: closesocket() failed with error code : %d\n", WSAGetLastError());
		return 0;
	}

	CLuaNetSocket * _luasock = new CLuaNetSocket(s);
	_luasock->m_IPPort = IP_Port;
	_luasock->m_LuaOnData = luaL_ref(State, LUA_REGISTRYINDEX);
	CLuaNet::m_TCPSockets.push_front(_luasock); // add it to the list

	lua_pushinteger(State, _luasock->m_ID);
	return 1;
}

// Lua param:
// string Addr, short Port, function Callback
// Connects to a TCP server and returns the socket if successful
int CLuaNet::Lua_connect(lua_State * State)
{
	if (!initWSA()) return 0;

	if (!lua_isfunction(State, 3)) return 0;

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

	// simply use the TCP Server sockets for this

	CLuaNetSocket * _clientsock = new CLuaNetSocket(s);
	_clientsock->m_IPPort = IP_Port;
	_clientsock->m_LuaOnData = luaL_ref(State, LUA_REGISTRYINDEX);
	CLuaNetSocket::_Connections.push_front(_clientsock);

	lua_pushinteger(State, _clientsock->m_ID);
	return 1;
}



void CLuaNet::PollFunctions()
{
	char buf[4096]; // 4kB receive buffer

	for (auto _s : CLuaNet::m_UDPSockets)
	{
		memset(buf, '\0', 4096); // clear the buffer

		if (!_s->m_LuaOnData) continue; // no Lua function available

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
			//unsigned char * _IP = (unsigned char *)&client.sin_addr.s_addr;
			//PRINT_DEBUG("Received something from %d.%d.%d.%d\n", _IP[0], _IP[1], _IP[2], _IP[3]);
			
			lua_rawgeti(CLuaEnvironment::_LuaState, LUA_REGISTRYINDEX, _s->m_LuaOnData); // push the referenced function on the stack and pcall it

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

		// new connection incoming
		SOCKET connector = accept((SOCKET)_s->m_Socket, (struct sockaddr *) &client, &slen);
		if (connector != INVALID_SOCKET)
		{
			unsigned char * _IP = (unsigned char *)&client.sin_addr.s_addr;

			unsigned long NonBlock = 1; // used in polling mode; allows using only one thread for all ports
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
			CLuaNetClient * _clientsock = new CLuaNetClient(connector);
			_clientsock->m_RemoteIP = client.sin_addr.s_addr;
			_s->m_Clients.push_front(_clientsock); // add it to the list
			PRINT_DEBUG("Active connections on Port %i:   %i#\n", _s->m_IPPort, _s->m_Clients.size());
		}

		// check for new data on this ports' clients
		u_long _avail = 0;
		for (auto _c : _s->m_Clients)
		{
			SOCKET s = _c->m_Socket;
			int iResult = ioctlsocket(s, FIONREAD, &_avail);
			if (_avail)
			{
				PRINT_DEBUG("Received from %d Buffer: %d\n", _c->m_RemoteIP, _avail);
				char __buff[0xFF];
				memset(__buff, 0, 0xFF);
				recvfrom(s, __buff, _avail, 0, 0, 0);

				//sockaddr_in SenderAddr;
				//int SenderAddrSize = sizeof(SenderAddr);
				//recvfrom(s, __buff, 0xFF, 0, (SOCKADDR *)& SenderAddr, &SenderAddrSize); // this will ALWAYS return -1 because of WSA_WOULDBLOCK
				//if (WSAGetLastError() != WSAEWOULDBLOCK) PRINT_ERROR("ERROR: recvfrom failed with error: #%d\n", WSAGetLastError());

				continue;
			}
		}

		// check existing connections for new data
		for (auto i = _s->m_Clients.begin(); i != _s->m_Clients.end(); i++) {
			auto _c = *i;

			struct sockaddr_in client;
			int slen = sizeof(client), recv_len = 0;
			
			if ((recv_len = recvfrom((SOCKET)_c->m_Socket, buf, 4096, 0, (struct sockaddr *) &client, &slen)) == SOCKET_ERROR)
			{
				int _err = WSAGetLastError();
				if (_err == 10035) continue; // ignore - caused by ioctlsocket setting to prevent blocking; TODO: Optimize!
				PRINT_ERROR("ERROR: recvfrom() failed with error code : %d\n", _err);
			}
			else // we received actual data
			{
				if (recv_len == 0) // connection closed by remote host
				{
					// TODO: VERFIY AND DEBUG THIS SCOPE
					PRINT_DEBUG("Removed closed socket on %i\n", _s->m_IPPort);
					closesocket(_c->m_Socket);

					delete _c;
					i = _s->m_Clients.erase(i); // remove the socket from the list
					if (i != _s->m_Clients.end()) i++; else break;

					continue;
				}
				
				if (!_s->m_LuaOnData) continue; // no function to call

				// call lua function to accept the data
				lua_rawgeti(CLuaEnvironment::_LuaState, LUA_REGISTRYINDEX, _s->m_LuaOnData); // push the referenced function on the stack and pcall it

				lua_pushlstring(CLuaEnvironment::_LuaState, (const char *)&buf, recv_len);
				lua_pushinteger(CLuaEnvironment::_LuaState, client.sin_addr.s_addr);

				if (lua_pcall(CLuaEnvironment::_LuaState, 2, 0, 0)) // Some error occured
				{
					PRINT_ERROR("CALLBACK ERROR: %s\n", lua_tostring(CLuaEnvironment::_LuaState, -1));
					lua_pop(CLuaEnvironment::_LuaState, 1);
				}
			}
		}
	}
	
	//TCP connections to remote servers
	for (auto i = CLuaNetSocket::_Connections.begin(); i != CLuaNetSocket::_Connections.end();) {
		CLuaNetSocket* _s = *i;

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
			if (recv_len == 0) // connection closed by remote host
			{
				// TODO: VERFIY AND DEBUG THIS SCOPE
				PRINT_DEBUG("Removed closed socket on %i\n", _s->m_IPPort);
				closesocket(_s->m_Socket);

				delete _s;
				i = CLuaNetSocket::_Connections.erase(i); // remove the socket from the list
				if (i != CLuaNetSocket::_Connections.end()) i++; else break;

				continue;
			}

			if (!_s->m_LuaOnData) continue; // no function to call

			// call lua function to accept the data
			lua_rawgeti(CLuaEnvironment::_LuaState, LUA_REGISTRYINDEX, _s->m_LuaOnData); // push the referenced function on the stack and pcall it

			lua_pushlstring(CLuaEnvironment::_LuaState, (const char *)&buf, recv_len);
			lua_pushinteger(CLuaEnvironment::_LuaState, client.sin_addr.s_addr);

			if (lua_pcall(CLuaEnvironment::_LuaState, 2, 0, 0)) // Some error occured
			{
				PRINT_ERROR("CALLBACK ERROR: %s\n", lua_tostring(CLuaEnvironment::_LuaState, -1));
				lua_pop(CLuaEnvironment::_LuaState, 1);
			}
		}
	}
}


#endif