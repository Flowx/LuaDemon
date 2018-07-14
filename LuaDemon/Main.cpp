#include "CLuaEnvironment.h"
#include "PlatformCompatibility.h"
#include "Lua/lua.hpp"
//#include <string>
#include <stdio.h>

using namespace std;

int CLuaEnvironment::_FileSpyInterval; //c++ linkers are horrible

static bool __nofilespy = false;
int main(int argc, const char ** args)
{
	CLuaEnvironment::_FileSpyInterval = 0;

#pragma region Command line argument parser
	for (int i = 0; i < argc; i++)
	{
		string targ = args[i];

		if (targ == "-h" || targ == "--help")
		{
			PRINT_DEBUG("-dir <directory>\t = Sets the root directory for the Lua environment.\n\tDirectory must be readable by the process.\n\tDirectory must end with /\n\tDirectory must contain an \'%s\'.\n\n", LUAENV_INDEXFILE);
			PRINT_DEBUG("-filespyinterval <interval>\t = Linux only. Interval is in milliseconds.\n\tAllows periodic checking of files by comparing last change.\n\tResource intensive - Use for debug only!\n\n");
			PRINT_DEBUG("-nofilespy\t = Disables automatic reloading on Lua file changes.\n\n");
			PRINT_DEBUG("-nodebug\t = Hides debug (cyan-colored) messages.");
			return 0;
		}

		if (targ == "-dir")
		{
			if (argc < i + 2) // Missing argmuments
			{
				PRINT_ERROR("ERROR: Missing arguments for -dir\n");
				return 1;
			}

			if (!EXISTS_DIRECTORY(args[i + 1])) // Root directory doesnt exist
			{
				PRINT_ERROR("ERROR: Directory \"%s\" does not exist or is not accessible.\n", args[i + 1]);
				return 1;
			}
			else
			{
				std::string _A = args[i + 1];
				_A += LUAENV_INDEXFILE;
				if (!EXISTS_FILE(_A.c_str()))
				{
					PRINT_ERROR("ERROR: Directory is missing entry point file (%s). Does the directory path end with / ?\n", LUAENV_INDEXFILE);
					return 1;
				}
			}

			CLuaEnvironment::_Directory = args[i + 1];
			PRINT_SUCCESS("Acquired valid directory \"%s\"!\n", args[i + 1]);
			i++; // skip the argument for -dir
		}
		else if (targ == "-filespyinterval") // Allows periodical checking of files on Linux
		{
			if (argc < i + 2) // checks if an interval is specified, defaults to 500ms
				CLuaEnvironment::_FileSpyInterval = 500;
			else 
				CLuaEnvironment::_FileSpyInterval = strtol(args[i + 1], nullptr, 0);
		}
		else if (targ == "-nofilespy")
			__nofilespy = true;
		else if (targ == "-nodebug")
			__nodebug = true;
	};
#pragma endregion

	if (CLuaEnvironment::_Directory.empty()) // Root directory doesnt exist
	{
		PRINT_ERROR("ERROR: No directory! Use \'-dir <directory>\' in the command line.\n");
		return 1;
	}
	else 
	{
		// only use forward slashes in directories
		for (size_t i = 0; i < CLuaEnvironment::_Directory.length(); i++) 
			if (CLuaEnvironment::_Directory[i] == '\\') CLuaEnvironment::_Directory[i] = '/';
	}

#pragma region Spawn Lua State

	CLuaEnvironment::_Error = 0;
	CLuaEnvironment::_Initialized = false;
	CLuaEnvironment::_LuaState = luaL_newstate();

	std::string _dir = *args;

	if (CLuaEnvironment::_LuaState == 0) // TODO: Output error message
	{
		PRINT_ERROR("Failed to create Lua state!");
		return 1;
	}

	luaL_openlibs(CLuaEnvironment::_LuaState); // opens all libraries

#pragma endregion

	if (!__nofilespy) CLuaEnvironment::LoadFileSpy();

	CLuaEnvironment::Cycle(); // This function blocks forever

	return 0;
}