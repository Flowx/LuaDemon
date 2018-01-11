#include "CLuaEnvironment.h"

#include "CLuaNet.h"
#include "CLuaSerial.h"

void CLuaEnvironment::PollLibraries()
{
	CLuaSerial::PollFunctions();
}

void CLuaEnvironment::PushLibraries()
{
	CLuaSerial::PushFunctions();
}

void CLuaEnvironment::LoadLibraries()
{
	CLuaSerial::LoadFunctions();
}