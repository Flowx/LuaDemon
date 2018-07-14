#pragma once
#include "CLuaLib.h"
#include "CLuaEnvironment.h"
#include <map>
#include <vector>

#if _WIN32 // ignore this file on linux
#include <Windows.h>
#endif

class CLuaSerialPort
{
public:
	CLuaSerialPort(const char * Name);
	//~CLuaSerialPort();

	std::string m_Name; // This is the port name; Typically COM1 on Windows or ttyS1 on Linux
	
	unsigned long m_LastAvailable = 0;

	char * m_FreeBuffer; // this buffer has to be freed after Read or ReadAll returns!
	bool m_IsFreed;

	int m_LuaReference;

#if _WIN32 // a little messy but better than having everything twice
	HANDLE m_PortReference; // Windows Handle
#else
	int m_PortReference; // Linux File Descriptor
#endif

	//void operator delete(void *) {} // works but good luck with the memory leak
};

class CLuaSerial //: public CLuaLib
{
	static int Lua_Discover(lua_State * State);		//Args: none; Only works on Windows right now!
	static int Lua_Open(lua_State * State);			//Args: string Port, string Name
	static int Lua_Send(lua_State * State);			//Args: string Port, number Data (0-255)
	static int Lua_Receive(lua_State * State);		//Args: string Port, function LuaCallback
	static int Lua_Available(lua_State * State);	//Args: string Port
	static int Lua_Read(lua_State * State);			//Args: string Port, number amount
	static int Lua_ReadAll(lua_State * State);		//Args: string Port
	static int Lua_ListOpen(lua_State * State);		//Args: string Port

public:
	static std::map<std::string, CLuaSerialPort *> m_PortList;

	static void PushFunctions();
	static void PollFunctions();
	static void LoadFunctions();
};



