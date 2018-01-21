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
		CloseHandle(_P->m_PortReference);
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
	CLuaSerial::m_PortList[_portname]->m_PortReference = _comHandle;

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
		_hSerial = _P->m_PortReference;
	} 
	else return 0;

	if (_hSerial == INVALID_HANDLE_VALUE) return 0;

	DWORD bytes_written;
	
	int a = WriteFile(_hSerial, &_databyte, 1, &bytes_written, NULL);

	return 0;
}

int CLuaSerial::Lua_Receive(lua_State * State)
{
	std::string _portname = lua_tostring(State, 1);
	
	if (_portname.empty() || !lua_isfunction(State, 2)) return 0;

	if (m_PortList.count(_portname) > 0)
	{
		CLuaSerialPort *_P = m_PortList[_portname];
		
		if (_P->m_LuaReference != 0) // there already is a function hooked to this port
		{
			PRINT_DEBUG("Removed old hook!\n");
			luaL_unref(State, LUA_REGISTRYINDEX, _P->m_LuaReference);
		}

		// function has to be last argument or this will create a wrong reference
		_P->m_LuaReference = luaL_ref(State, LUA_REGISTRYINDEX);

		_P->m_Thread = std::thread(&(CLuaSerial::SerialRecv), _P);
		_P->m_Thread.detach();

		// push and call
		//lua_rawgeti(State, LUA_REGISTRYINDEX, _P->m_LuaReference);
		//lua_pcall(State, 0, 0, 0);

	}
	//else return 0;
	
	return 0;
}

void CLuaSerial::SerialRecv( CLuaSerialPort * Port )
{
	PRINT_DEBUG("Opened Thread for: %s\n", Port->m_Name.c_str());
	for (;;)
	{
		//ReadFile(_hSerial, &_databyte, 1, &bytes_written, NULL);

	}
}

#endif