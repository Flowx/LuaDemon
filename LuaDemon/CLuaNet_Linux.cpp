#include "CLuaNet.h"
#include "PlatformCompatibility.h"
#include<string.h>
#include<arpa/inet.h>
#include<sys/socket.h>


// Lua param:
// short Port, func Callback
// Opens a receiving socket
// Callback: Called when a new connection is initiated
int CLuaNet::Lua_openTCPSocket(lua_State * State)
{
	return 0;
}

// Lua param:
// short Port, func Callback
// Opens a receiving socket
// Callback: Called when a packet is received
int CLuaNet::Lua_openUDPSocket(lua_State * State)
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


#ifdef Opens UDP Socket on Linux
int CLuaNet::Lua_OpenUDP(lua_State * State)
{
	struct sockaddr_in _addr;
	short _port = (short)lua_tonumber(State, 1);

	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(_port);
	_addr.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0

	SOCKET _socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (_socket == 0 || _socket == -1)
	{
		PRINT_ERROR("Failed to create socket! %d\n", _socket);
		return 0;
	}

	if (bind(_socket, (struct sockaddr *) &_addr, sizeof(_addr)) < 0)
	{
		PRINT_ERROR("Failed to bind socket to port %d\n\tPort may be in use already. Do you have permissions?\n", _port);
		return 0;
	}

	PRINT_DEBUG("Successfully spawned UDP socket on port %d\n", _port);

	lua_pushnumber(State, (int)_socket); // just give it the socket number

	CLuaSocket _luasocket = CLuaSocket();
	_luasocket.m_Port = _port;
	_luasocket.m_Socket = _socket;

	//m_Sockets.push_back(_luasocket);

	return 1;
}
#endif