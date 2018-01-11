#include "CLuaNet.h"


void CLuaNet::PushFunctions()
{
	lua_newtable(CLuaEnvironment::_LuaState);

	lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaNet::Lua_createSocket);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "create");

	lua_setglobal(CLuaEnvironment::_LuaState, "net");

}

int CLuaNet::Lua_createSocket(lua_State * State)
{
//	unsigned short _port = (short)lua_tointeger(State, 1);


	return 0;
}


#ifdef nigger
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