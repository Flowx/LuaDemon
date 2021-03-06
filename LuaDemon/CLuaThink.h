#pragma once
#include "CLuaLib.h"
#include "CLuaEnvironment.h"
#include <chrono>


/*

	OnTick routines

*/
class CLuaThink : CLuaLib
{
	class ThinkHook
	{
	public: 
		const char* m_Identifier;
		int m_Interval;
		int m_Luafunction;
		std::chrono::steady_clock::time_point m_Trigger;

		ThinkHook(const char * Identifier, int Interval, int LuaCallback)
		{
			m_Identifier = Identifier;
			m_Interval = Interval;
			m_Luafunction = LuaCallback;
			m_Trigger = std::chrono::steady_clock::now();
		}
	};

public:
	static void PushFunctions();
	static void PollFunctions();
	static void LoadFunctions();

	static inline std::list<ThinkHook *> m_Hooklist;

	static int Lua_add(lua_State* State); // creates a new hook
	static int Lua_remove(lua_State* State); // removes a hook
	static int Lua_list(lua_State* State); // lists all hooks
};