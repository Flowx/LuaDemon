#include "CLuaEnvironment.h"
#include "CLuaStdLib.h"
#include "CLuaSerial.h"
#include "CLuaFile.h"
#include "CLuaUDP.h"
#include "CLuaTCP.h"
#include "CLuaThink.h"

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
	CLuaTCP::PollFunctions();
	CLuaThink::PollFunctions();
}

void CLuaEnvironment::PushLibraries() // called on intial startup
{
	CLuaStdLib::PushFunctions();
	CLuaSerial::PushFunctions();
	CLuaFile::PushFunctions();
	CLuaUDP::PushFunctions();
	CLuaTCP::PushFunctions();
	CLuaThink::PushFunctions();
}

void CLuaEnvironment::LoadLibraries() // called when Lua reloads (file change)
{
	CLuaStdLib::LoadFunctions();
	CLuaSerial::LoadFunctions();
	CLuaFile::LoadFunctions();
	CLuaUDP::LoadFunctions();
	CLuaTCP::LoadFunctions();
	CLuaThink::LoadFunctions();
}