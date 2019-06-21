#include "CLuaNet.h"
#include "PlatformCompatibility.h"
#include<string.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<fcntl.h>



// Lua param:
// string Addr, short Port, string Data
// Opens a UDP socket, sends a packet and closes the port again
// should only used for small occasional data packets
int CLuaNet::Lua_dumpUDP(lua_State * State)
{
	const char * IP_Addr = lua_tostring(State, 1);
	unsigned short IP_Port = (unsigned short)lua_tointeger(State, 2);
	const char * data = lua_tostring(State, 3); // debugger doesnt see it but it gets the whole string including Nulls

	if (data == NULL || IP_Addr == NULL) return 0;

	size_t datalen = lua_strlen(State, 3);
	// max UDP size in IPv4 is 65507
	if (datalen > 65507)
	{
		PRINT_ERROR("ERROR: dumpUDP() data string is longer than 65507");
		return 0;
	}

	struct sockaddr_in si_other;
	int s;

	memset((char *)&si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(IP_Port);

	if (inet_aton(IP_Addr, &si_other.sin_addr) == 0)
	{
		PRINT_ERROR("ERROR: Failed to parse IP! \"%s\"\n", IP_Addr);
		return 0;
	}

	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		PRINT_ERROR("ERROR: Failed to open UDP socket!\n");
		return 0;
	}

	if (sendto(s, data, datalen, 0, (struct sockaddr *) &si_other, sizeof(si_other)) == -1)
	{
		PRINT_ERROR("ERROR: sendto() in UDP failed!\n");
		return 0;
	}

	close(s);

	return 0;
}

// Lua param:
// short Port, func Callback
// Opens a receiving socket
// Callback: Called when a packet is received
int CLuaNet::Lua_openUDPSocket(lua_State * State)
{
	unsigned short IP_Port = (unsigned short)lua_tointeger(State, 1);

	if (!lua_isfunction(State, 2)) return 0;

	bool noReuse = (bool)lua_toboolean(State, 3);

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


	struct sockaddr_in server;
	memset((char *)&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(IP_Port);


	unsigned int s;
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		PRINT_ERROR("ERROR: socket() failed: %s", strerror(errno));
		return 0;
	}

	//bind socket to port
	if (bind(s, (struct sockaddr*)&server, sizeof(server)) == -1)
	{
		PRINT_ERROR("ERROR: bind() failed: %s\n\tTerminating Socket...\n", strerror(errno));
		if (close(s) == -1) PRINT_ERROR("ERROR: closesocket() failed.\n");
		return 0;
	}

	// prevent blocking
	if (fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK) == -1)
	{
		PRINT_ERROR("ERROR: fcntl() O_NONBLOCK failed: %s\n\tTerminating Socket...\n", strerror(errno));
		if (close(s) == -1) PRINT_ERROR("ERROR: closesocket() failed.\n");
		return 0;
	}

	// NOTE: Some error handling may be required on Linux

	CLuaNetSocket * _luasock = new CLuaNetSocket(s);
	_luasock->m_IPPort = IP_Port;
	//_luasock.m_SockAddr = server;

	CLuaNet::m_UDPSockets.push_front(_luasock); // add it to the list

	lua_pushboolean(State, 1);
	return 1;
}



// Lua param:
// short Port, func Callback
// Opens a receiving socket
// Callback: Called when a new connection is initiated
int CLuaNet::Lua_openTCPSocket(lua_State * State)
{
	return 0;
}

// Lua param:
// string Addr, short Port
// Connects to a TCP server and returns the socket if successful
int CLuaNet::Lua_connect(lua_State * State)
{
	return 0;
}



void CLuaNet::PollFunctions()
{
	char buf[4096]; // 4kB receive buffer

	for (auto _s : CLuaNet::m_UDPSockets)
	{
		memset(buf, '\0', 4096); // clear the buffer

		struct sockaddr_in si_other;
		int recv_len = 0;
		unsigned int slen = sizeof(si_other);

		if ((recv_len = recvfrom(_s->m_Socket, buf, 4096, 0, (struct sockaddr *) &si_other, &slen)) == -1)
		{
			if (errno == EWOULDBLOCK) continue; // ignore - caused by ioctlsocket setting to prevent blocking; TODO: Optimize!
			PRINT_ERROR("ERROR: recvfrom() failed: %s\n", strerror(errno));
		}
		else // we received actual data
		{
			PRINT_DEBUG("Received something!\n");
		}
	}
}
