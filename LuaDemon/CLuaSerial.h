#pragma once
#include "CLuaLib.h"
#include "CLuaEnvironment.h"
#include <map>

#if _WIN32 // ignore this file on linux
#include <Windows.h>
#endif

class CLuaSerialPort
{
public:
	CLuaSerialPort(const char * Name);
	//~CLuaSerialPort();

	std::thread m_Thread;
	std::string m_Name; // This is the port name; Typically COM1 on Windows or ttyS1 on Linux
	
	int m_LuaReference;

#if _WIN32 // a little messy but better than having everything twice
	HANDLE m_PortReference; // Windows Handle
#elif
	int m_PortReference; // Linux File Descriptor
#endif

	//void operator delete(void *) {} // works but good luck with the memory leak
};

class CLuaSerial: public CLuaLib
{
	static int Lua_Discover(lua_State * State); //Args: none
	static int Lua_Open(lua_State * State); //Args: string Port, string Name
	static int Lua_Send(lua_State * State); //Args: string Port, number Data (0-255)
	static int Lua_Receive(lua_State * State); //Args: string Port, function LuaCallback

public:
	static std::map<std::string, CLuaSerialPort *> m_PortList;

	static void PushFunctions();
	static void PollFunctions();
	static void LoadFunctions();
};



