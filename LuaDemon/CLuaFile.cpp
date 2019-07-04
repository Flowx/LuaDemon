#include "CLuaFile.h"
#include "CLuaEnvironment.h"
#include "PlatformCompatibility.h"


/*
	Args: string Filename
	Returns: string Data

	Returns all content of file as string.

	Only use this on smaller files!


	TODO: ERROR CHECK THIS!!!!
*/
int CLuaFile::Lua_readAll(lua_State * State)
{
	const char * filename = lua_tostring(State, 1);

	if (!EXISTS_FILE(filename))
	{
		PRINT_DEBUG("file.readAll: No such file %s\n", filename); // debug or warning?
		return 0;
	}

	FILE * _file = fopen(filename, "rb");

	if (_file == NULL) // we got no pointer
	{
		PRINT_WARNING("file.readAll: fopen returned null for file %s\n", filename);
		lua_pushboolean(State, 1);
		return 1;
	}

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
	Returns: bool Failed, nil if successfull

	NoOverwrite = prevent overwriting if file already exists

	Dumps string into file. Works fine with large files.



	TODO: ERROR CHECK THIS!!!!
*/
int CLuaFile::Lua_writeAll(lua_State * State)
{
	const char * filename = lua_tostring(State, 1);

	size_t dataLength;
	const char * data = lua_tolstring(State, 2, &dataLength);

	int nooverride = lua_toboolean(State, 3);

	if (nooverride && EXISTS_FILE(filename))
	{
		PRINT_DEBUG("file.writeAll: File %s already exists!\n", filename);

		lua_pushboolean(State, 1);

		return 1;
	}

	FILE * _file = fopen(filename, "w+b");

	if (_file == NULL) // we got no pointer
	{
		PRINT_WARNING("file.writeAll: fopen returned null for file %s\n", filename);
		lua_pushboolean(State, 1);
		return 1;
	}

	if (fwrite(data, 1, dataLength, _file) != dataLength) // something didnt work out here
	{
		PRINT_WARNING("file.writeAll: Failed to write all data to %s !\n", filename);
		fclose(_file);
		lua_pushboolean(State, 1);
		return 1;
	}

	fclose(_file);
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
