//
//	Makeshift way to launch the libraries in a static way
//

#include "CLuaEnvironment.h"
#include "CLuaStdLib.h"
#include "CLuaSerial.h"
#include "CLuaNet.h"
#include "CLuaFile.h"

void CLuaEnvironment::PollLibraries() // called every cycle
{
	CLuaStdLib::PollFunctions();
	CLuaSerial::PollFunctions();
	CLuaNet::PollFunctions();
	CLuaFile::PollFunctions();
}

void CLuaEnvironment::PushLibraries() // called on intial startup
{
	CLuaStdLib::PushFunctions();
	CLuaSerial::PushFunctions();
	CLuaNet::PushFunctions();
	CLuaFile::PushFunctions();
}

void CLuaEnvironment::LoadLibraries() // called when Lua reloads (file change)
{
	CLuaStdLib::LoadFunctions();
	CLuaSerial::LoadFunctions();
	CLuaNet::LoadFunctions();
	CLuaFile::LoadFunctions();
}