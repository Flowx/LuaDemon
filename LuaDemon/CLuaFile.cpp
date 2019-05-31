#include "CLuaFile.h"
#include "CLuaEnvironment.h"
#include "PlatformCompatibility.h"

//int CLuaFile::Lua_Dump(lua_State * State)
//{
//	return 0;
//}

/*
	Args: string Filename
	Returns: string Data

	Returns all content of file as string.

	Only use this on smaller files!
*/
int CLuaFile::Lua_readAll(lua_State * State)
{
	const char * filename = lua_tostring(State, 1);

	if (!EXISTS_FILE(filename))
	{
		PRINT_DEBUG("file.readAll: No such file %s\n", filename);
		return 0;
	}

	FILE * _file = fopen(filename, "rb");
	fseek(_file, 0, SEEK_END);
	long _fsize = ftell(_file);
	//PRINT_DEBUG("File size_ %d\n", _fsize);
	fseek(_file, 0, SEEK_SET);

	char * _fileString = (char *)malloc(_fsize + 1);
	fread(_fileString, _fsize, 1, _file);
	fclose(_file);

	lua_pushlstring(State, _fileString, _fsize);

	free(_fileString);

	return 1;
}

/*
	Args: string Filename, string Data, bool NoOverwrite=nil
	Returns: bool Failed

	NoOverwrite = prevent overwriting if file already exists

	Dumps string into file. Works fine with large files.
*/
int CLuaFile::Lua_writeAll(lua_State * State)
{
	const char * filename = lua_tostring(State, 1);

	size_t dataLength;
	const char * data = lua_tolstring(State, 2, &dataLength);


	int nooverride = lua_toboolean(State, 3);

	if (nooverride && EXISTS_FILE(filename))
	{
		PRINT_DEBUG("file.readAll: File %s already exists!\n", filename);

		lua_pushboolean(State, 1);

		return 1;
	}

	FILE * _file = fopen(filename, "w+b");
	


	return 0;
}


void CLuaFile::PushFunctions()
{
	lua_newtable(CLuaEnvironment::_LuaState);

	lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaFile::Lua_readAll);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "readAll");

	lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaFile::Lua_writeAll);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "writeAll");

	//lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaNet::Lua_sendUDP);
	//lua_setfield(CLuaEnvironment::_LuaState, -2, "sendUDP");

	lua_setglobal(CLuaEnvironment::_LuaState, "file");
}

void CLuaFile::PollFunctions()
{
}

void CLuaFile::LoadFunctions()
{
}
