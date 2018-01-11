#pragma once
#include <list>


class CLuaLib
{
	static std::list<CLuaLib> m_Libraries;

	virtual void PushFunctions() = 0; // Called once at startup to expose functions to Lua
	virtual void PollFunctions() = 0; // Called every cycle to update Lua
	virtual void LoadFunctions() = 0; // Called before Lua is being reloaded (by a file change)

public:
	static void Initialize();
};

