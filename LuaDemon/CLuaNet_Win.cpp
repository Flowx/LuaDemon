#if _WIN32 // ignore this file on linux
#include "CLuaNet.h"
#include "PlatformCompatibility.h"
#include <Ws2tcpip.h>
#include <WinSock2.h>


// Linker stuff...
#pragma comment(lib, "ws2_32.lib")

// checks if WSA is initialized; Initializes it if required
// Windows specific stuff
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
	if (!initWSA()) return 0;

	unsigned short IP_Port = (unsigned short)lua_tointeger(State, 1);
	
	if (!lua_isfunction(State, 2)) return 0;

//	int luaRef = lua_tocfunction(State, 2);

	bool noReuse = (bool)lua_toboolean(State, 3);


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
		PRINT_ERROR("ERROR: bind() failed with error code : %d\n", WSAGetLastError());
		if (closesocket(s) == SOCKET_ERROR) PRINT_ERROR("ERROR: closesocket() failed with error code : %d\n\tTerminating Socket...\n", WSAGetLastError());
		return 0;
	}

	unsigned long NonBlock = 1; // used in polling mode; allows using only one thread
	if (ioctlsocket(s, FIONBIO, &NonBlock) == SOCKET_ERROR)
	{
		PRINT_ERROR("ERROR: ioctlsocket() failed with error code : %d\n\tTerminating Socket...\n", WSAGetLastError());
		if (closesocket(s) == SOCKET_ERROR) PRINT_ERROR("ERROR: closesocket() failed with error code : %d\n", WSAGetLastError());
		return 0;
	}


	//most of the input and error handling is already done by WSA
	
	CLuaNetSocket _luasock(s);
	_luasock.m_IPPort = IP_Port;

	CLuaNet::m_UDPSockets.push_front(&_luasock); // add it to the list



	//char buf[4096];
	//memset(buf, '\0', 4096);

	//struct sockaddr_in si_other;
	//int slen = sizeof(si_other), recv_len = 0;

	//while(true) {
	//	if ((recv_len = recvfrom(s, buf, 4096, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR)
	//	{
	//		int _err = WSAGetLastError();
	//		if (_err == 10035) continue; // ignore, caused by ioctlsocket setting to prevent blocking; TODO: Optimize!
	//		PRINT_ERROR("ERROR: recvfrom() failed with error code : %d\n", _err);
	//		return 0;
	//	}
	//	else // we received actual data
	//	{
	//		PRINT_DEBUG("Received something!\n", WSAGetLastError());
	//		//break;
	//	}

	//	
	//}

	return 0;
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

	if(res != SOCKET_ERROR)
	{
		lua_pushboolean(State, TRUE);
		return 1;
	} 
	else PRINT_ERROR("ERROR: sendto() failed with error code : %d\n", WSAGetLastError());

	return 0;
}


#endif