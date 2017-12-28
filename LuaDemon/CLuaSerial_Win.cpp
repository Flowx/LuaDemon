#if _WIN32 // ignore this file on linux

#include "CLuaSerial.h"
#include "PlatformCompatibility.h"
#include <windows.h>

#ifdef _MSC_VER
#pragma warning(disable : 4996) // disables warning about "unsafe" functions
#endif

std::map<std::string, HANDLE> CLuaSerial::_portList;

void CLuaSerial::PushFunctions()
{
	lua_newtable(CLuaEnvironment::_LuaState);

	lua_pushcfunction(CLuaEnvironment::_LuaState, Lua_Open);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "Open");

	lua_pushcfunction(CLuaEnvironment::_LuaState, Lua_Discover);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "Discover");

	lua_pushcfunction(CLuaEnvironment::_LuaState, Lua_Send);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "Send");

	lua_pushcfunction(CLuaEnvironment::_LuaState, Lua_Receive);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "Receive");

	lua_setglobal(CLuaEnvironment::_LuaState, "serial");
}

void CLuaSerial::PollFunctions()
{

}

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

	std::string buff = ("\\\\.\\" + _portname);
	HANDLE _comHandle = CreateFile(buff.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);

	if (_comHandle == INVALID_HANDLE_VALUE)
	{
		PRINT_DEBUG("Failed to open port %s\n", _portname.c_str());
		return 0;
	}

	_portList[_portname] = _comHandle;

	//DCB _parameters = { 0 }; // Initializing DCB structure
	//_parameters.DCBlength = sizeof(_parameters);
	//_parameters.BaudRate = _portspeed;
	//_parameters.StopBits = _stopbits;

	return 0;
}

// Lua param:
// string Portname, number data (0-255)
// TODO: FINISH IT!!!!
int CLuaSerial::Lua_Send(lua_State * State)
{
	std::string _portname = lua_tostring(State, 1);
	char _databyte = lua_tointeger(State, 2);

	if (_portname.empty()) return 0;

	HANDLE _hSerial = _portList[_portname];

	if (_hSerial == 0 || _hSerial == INVALID_HANDLE_VALUE) return 0;

	DWORD bytes_written;
	int a = WriteFile(_hSerial, &_databyte, 1, &bytes_written, NULL);
	if (a == 0)
	{
		_portList.erase(_portname);
		return 0;
	}

	return 0;
}

int CLuaSerial::Lua_Receive(lua_State * State)
{
	std::string _portname = lua_tostring(State, 1);
	char _databyte = lua_tointeger(State, 2);

	if (_portname.empty()) return 0;

	HANDLE _hSerial = _portList[_portname];

	if (_hSerial == 0 || _hSerial == INVALID_HANDLE_VALUE) return 0;

	DWORD bytes_written;
	int a = WriteFile(_hSerial, &_databyte, 1, &bytes_written, NULL);
	if (a == 0)
	{
		_portList.erase(_portname);
		return 0;
	}

	return 0;
}




#endif