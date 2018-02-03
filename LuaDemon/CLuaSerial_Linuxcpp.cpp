#if !_WIN32 // ignore this file on linux

#include "CLuaSerial.h"
#include "PlatformCompatibility.h"

std::map<std::string, CLuaSerialPort *> CLuaSerial::m_PortList;

// Lua Exposed Functions

// Lua param:
// none
// Returns all available COM Ports
int CLuaSerial::Lua_Discover(lua_State * State)
{
	//char _paths[0xFF], _namebuf[7];

	//size_t _k = 0;

	//lua_newtable(State);

	//for (int i = 0; i <= 0xFF; i++)
	//{
	//	sprintf(_namebuf, "COM%i", i);
	//	if (QueryDosDevice(_namebuf, _paths, sizeof(_paths)))
	//	{
	//		_k++;
	//		lua_pushinteger(State, _k);
	//		lua_pushstring(State, _namebuf);
	//		lua_settable(State, -3);
	//	}
	//}

	return 0;
}

// Lua param:
// string Portname, number Portspeed = 9600, number Bytesize = 8, number Stopbits = 1
// Opens a new COM Port
int CLuaSerial::Lua_Open(lua_State * State)
{
	return 0;
}

// Lua param:
// string Portname, var Data
// Sends Data
int CLuaSerial::Lua_Send(lua_State * State)
{
	return 0;
}

// Lua param:
// string Portname
// Returns available data (in bytes) ready to be read
int CLuaSerial::Lua_Available(lua_State * State)
{
	return 0;
}

// Lua param:
// string Portname
// Read all currently available data; Returns string
int CLuaSerial::Lua_ReadAll(lua_State * State)
{
	return 0;
}


// Lua param:
// string Portname, function Callback
// Receives Data
int CLuaSerial::Lua_Receive(lua_State * State)
{
	return 0;
}


void CLuaSerial::PollFunctions()
{
}

#endif