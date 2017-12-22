#include <stdio.h>
#include "CLuaEnvironment.h"
#include "PlatformCompatibility.h"
#include "Lua/lua.hpp"

#ifdef _MSC_VER
#pragma warning(disable : 4996) // disables warning about "unsafe" functions
#endif

using namespace std;

bool __nofilespy = false;
int main(int argc, const char ** args)
{

#pragma region Command line argument parser
	for (int i = 0; i < argc; i++)
	{
		string targ = args[i];

		if (targ == "-h" || targ == "--help")
		{
			PRINT_DEBUG("-dir <directory>\t = Sets the root directory for the Lua environment\n\tDirectory must be readable by the process.\n\tDirectory must end with /\n\tDirectory must contain an \'%s\'.\n", LUAENV_INDEXFILE);
			PRINT_DEBUG("-nofilespy\t = Disables automatic reloading on Lua file changes.");
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
			PRINT_SUCCESS("nigger Acquired valid directory \"%s\"!\n", args[i + 1]);
			i += 2;
		}
		else if (targ == "-nofilespy")
			__nofilespy = true;
	};
#pragma endregion

	if (CLuaEnvironment::_Directory.empty()) // Root directory doesnt exist
	{
		PRINT_ERROR("ERROR: No directory! Use \'-dir <directory>\' in the command line.\n");
		return 1;
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

	luaopen_base(CLuaEnvironment::_LuaState);
	luaopen_debug(CLuaEnvironment::_LuaState);

#pragma endregion

	if (!__nofilespy) CLuaEnvironment::LoadFileSpy();

	CLuaEnvironment::Cycle(); // This function blocks forever

	return 0;
}