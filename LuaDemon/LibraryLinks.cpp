#include "CLuaEnvironment.h"
#include "CLuaStdLib.h"
#include "CLuaSerial.h"
#include "CLuaFile.h"
#include "CLuaUDP.h"

/*
	Loads the statically written lua libraries

	Not optimal, I know.
*/

void CLuaEnvironment::PollLibraries() // called every cycle/tick
{
	CLuaStdLib::PollFunctions();
	CLuaSerial::PollFunctions();
	CLuaFile::PollFunctions();
	CLuaUDP::PollFunctions();
}

void CLuaEnvironment::PushLibraries() // called on intial startup
{
	CLuaStdLib::PushFunctions();
	CLuaSerial::PushFunctions();
	CLuaFile::PushFunctions();
	CLuaUDP::PushFunctions();
}

void CLuaEnvironment::LoadLibraries() // called when Lua reloads (file change)
{
	CLuaStdLib::LoadFunctions();
	CLuaSerial::LoadFunctions();
	CLuaFile::LoadFunctions();
	CLuaUDP::LoadFunctions();
}