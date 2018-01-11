#if _WIN32 // ignore this file on linux

#include "CLuaSerial.h"
#include "PlatformCompatibility.h"
#include <windows.h>

#ifdef _MSC_VER
#pragma warning(disable : 4996) // disables warning about "unsafe" functions
#endif

std::map<std::string, CLuaSerialPort *> CLuaSerial::m_PortList;

// Lua Exposed Functions

int CLuaSerial::Lua_Discover(lua_State * State)
{
	char _paths[0xFF], _namebuf[7];

	size_t _k = 0;

	lua_newtable(State);

	for (int i = 0; i <= 0xFF; i++)
	{
		sprintf(_namebuf, "COM%i", i);
		if (QueryDosDevice(_namebuf, _paths, sizeof(_paths)))
		{
			_k++;
			lua_pushinteger(State, _k);
			lua_pushstring(State, _namebuf);
			lua_settable(State, -3);
		}
	}

	return 1;
}

// Lua param:
// string Portname, number Portspeed = 9600, number Bytesize = 8, number Stopbits = 1
int CLuaSerial::Lua_Open(lua_State * State) 
{
	std::string _portname = lua_tostring(State, 1);

	if (_portname.empty()) return 0;

	size_t _portspeed = lua_tointeger(State, 2);
	size_t _bytesize = lua_tointeger(State, 3);
	size_t _stopbits = lua_tointeger(State, 4);

	if (!_portspeed) _portspeed = 9600;
	if (!_bytesize) _bytesize = 8;
	if (!_stopbits) _stopbits = 1;

	if (m_PortList.count(_portname) > 0) // there is already a Lua serial port on this port
	{
		CLuaSerialPort *_P = m_PortList[_portname];
		CloseHandle(_P->m_Reference);
		delete _P;
		m_PortList.erase(_portname);
	}

	std::string buff = ("\\\\.\\" + _portname);
	HANDLE _comHandle = CreateFile(buff.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
	if (_comHandle == INVALID_HANDLE_VALUE)
	{
		PRINT_DEBUG("Failed to open port %s\n", _portname.c_str());
		lua_pushboolean(State, FALSE);
		return 1;
	}
/*
	CLuaSerialPort myPort = CLuaSerialPort(_portname.c_str());
	myPort.m_Reference = _comHandle;
	CLuaSerial::m_PortList[_portname] = &myPort;*/
	//CLuaSerial::m_PortList[_portname] = myPort;

	CLuaSerial::m_PortList[_portname] = new CLuaSerialPort(_portname.c_str());
	CLuaSerial::m_PortList[_portname]->m_Reference = _comHandle;

	lua_pushboolean(State, TRUE);
	return 1;
}

// Lua param:
// string Portname, number data (0-255)
// TODO: FINISH IT!!!!
int CLuaSerial::Lua_Send(lua_State * State)
{
	std::string _portname = lua_tostring(State, 1);
	char _databyte = (char)lua_tointeger(State, 2);

	if (_portname.empty()) return 0;

	HANDLE _hSerial;
	if (m_PortList.count(_portname) > 0)
	{
		CLuaSerialPort *_P = m_PortList[_portname];
		_hSerial = _P->m_Reference;
	} 
	else return 0;

	if (_hSerial == INVALID_HANDLE_VALUE) return 0;

	DWORD bytes_written;
	int a = WriteFile(_hSerial, &_databyte, 1, &bytes_written, NULL);
	//if (a == 0)
	//{
	//	m_PortList.erase(_portname);
	//	return 0;
	//}

	return 0;
}

int CLuaSerial::Lua_Receive(lua_State * State)
{
	std::string _portname = lua_tostring(State, 1);
	
	int r = luaL_ref(State, LUA_REGISTRYINDEX);
	
	lua_pcall(State, 1, 0, 0);

	return 0;
}

//int CLuaLibrary::Lua_hook_Add(lua_State *LState) {
//	//string f = lua_getfiel
//	std::string _name = lua_tostring(LState, 1);
//	std::string _id = lua_tostring(LState, 2);
//	lua_CFunction _func = lua_tocfunction(LState, 3);
//
//	return 0;
//}


#endif