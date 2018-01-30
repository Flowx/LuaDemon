#if _WIN32 // ignore this file on linux

#include "CLuaSerial.h"
#include "PlatformCompatibility.h"
#include <windows.h>

#ifdef _MSC_VER
#pragma warning(disable : 4996) // disables warning about "unsafe" functions
#endif

std::map<std::string, CLuaSerialPort *> CLuaSerial::m_PortList;

// Lua Exposed Functions

// Lua param:
// none
// Returns all available COM Ports
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
// Opens a new COM Port
int CLuaSerial::Lua_Open(lua_State * State) 
{
	std::string _portname = lua_tostring(State, 1);

	if (_portname.empty()) return 0;

	size_t _portspeed = lua_tointeger(State, 2);
	char _bytesize = lua_tointeger(State, 3);
	char _stopbits = lua_tointeger(State, 4);

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
	HANDLE _comHandle = CreateFile(buff.c_str(), (GENERIC_READ | GENERIC_WRITE), (FILE_SHARE_READ | FILE_SHARE_WRITE), 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (_comHandle == INVALID_HANDLE_VALUE)
	{
		PRINT_DEBUG("Failed to open port %s\n", _portname.c_str());
		lua_pushboolean(State, FALSE);
		return 1;
	}

#pragma region Settings

	COMMTIMEOUTS _Timeouts;

	_Timeouts.ReadIntervalTimeout = 1;
	_Timeouts.ReadTotalTimeoutMultiplier = 1;
	_Timeouts.ReadTotalTimeoutConstant = 1;
	_Timeouts.WriteTotalTimeoutMultiplier = 1;
	_Timeouts.WriteTotalTimeoutConstant = 1;
	if (!SetCommTimeouts(_comHandle, &_Timeouts)) PRINT_WARNING("Failed to set Timeouts\n");

	DCB _serialParams = { 0 };
	_serialParams.DCBlength = sizeof(_serialParams);

	GetCommState(_comHandle, &_serialParams);
	_serialParams.BaudRate = _portspeed;
	_serialParams.ByteSize = _bytesize;
	_serialParams.StopBits = _stopbits;
	_serialParams.Parity = 0;
	_serialParams.fDtrControl = 0;
	_serialParams.fRtsControl = 0;
	SetCommState(_comHandle, &_serialParams);

#pragma endregion

	CLuaSerial::m_PortList[_portname] = new CLuaSerialPort(_portname.c_str());
	CLuaSerial::m_PortList[_portname]->m_PortReference = _comHandle;

	lua_pushboolean(State, TRUE);
	return 1;
}

// Lua param:
// string Portname, var Data
// Sends Data
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

// Lua param:
// string Portname
// Returns available data (in bytes) ready to be read
int CLuaSerial::Lua_Available(lua_State * State)
{
	std::string _portname = lua_tostring(State, 1);

	if (_portname.empty()) return 0;

	if (m_PortList.count(_portname) > 0)
	{
		CLuaSerialPort *_P = m_PortList[_portname];

		COMSTAT _s;
		ClearCommError(_P->m_PortReference, 0, &_s);

		_P->m_Available = _s.cbInQue;

		lua_pushinteger(State, _P->m_Available);

		return 1;
	}

	return 0;
}

// Lua param:
// string Portname, function Callback
// Receives Data
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

		std::thread _T(&(CLuaSerial::SerialRecv), _P);
		_T.detach();

		//_P->m_Thread = a;

		// push and call
		//lua_rawgeti(State, LUA_REGISTRYINDEX, _P->m_LuaReference);
		//lua_pcall(State, 0, 0, 0);

	}
	//else return 0;
	
	return 0;
}


// TODO: Move this to the "Cycle" function.
void CLuaSerial::SerialRecv( CLuaSerialPort * Port )
{
	PRINT_DEBUG("Opened Thread for: %s\n", Port->m_Name.c_str());

	//LPDWORD _bytes = 0;
	HANDLE _Port = Port->m_PortReference;
	
	char _lBuff[16];
	memset(_lBuff, 0, sizeof(_lBuff));

	for (;;)
	{
		COMSTAT _Stat;
		ClearCommError(_Port, 0, &_Stat);

		if (_Stat.cbInQue > Port->m_LastAvailable)
		{
			Port->m_Available = _Stat.cbInQue;
			int a = ReadFile(_Port, _lBuff, 2, 0, 0);
			PRINT_DEBUG("RECEIVED: %s\n", _lBuff);
		}


		std::this_thread::sleep_for(std::chrono::microseconds(10)); // 100kHz is a plenty high poll rate
		//std::this_thread::sleep_for(std::chrono::milliseconds(1)); // reload doesnt work reliably without this for some reason
	}
}

#endif