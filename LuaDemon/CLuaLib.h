#pragma once
#include <list>

/*
	Used as base class for all libraries. Mainly to be able to iterate trough them.

	The class is not pure-virtual because instancing is required.

	NOTE: Im an idiot, discard this. The libaries are static classes which OBVIOUSLY cannot be instantiated.
*/

class CLuaLib
{
public:
	static void PushFunctions() {}; // Called once at startup to expose functions to Lua
	static void PollFunctions() {}; // Called every cycle to update Lua
	static void LoadFunctions() {}; // Called before Lua is being reloaded (by a file change)

	//static std::list<CLuaLib> m_Libraries;
};