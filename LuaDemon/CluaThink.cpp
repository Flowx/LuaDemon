#include "CLuaThink.h"
#include "PlatformCompatibility.h"
#include "CLuaEnvironment.h"



//using namespace std::chrono;

#pragma region Lua linking
void CLuaThink::PollFunctions()
{
	std::chrono::steady_clock::time_point _tp = std::chrono::steady_clock::now();
	for each (CLuaThink::ThinkHook * _h in CLuaThink::m_Hooklist)
	{
		if (std::chrono::duration_cast<std::chrono::milliseconds>(_tp - _h->m_Trigger).count() >= _h->m_Interval)
		{
			_h->m_Trigger = _tp; // reset the timer

			lua_rawgeti(CLuaEnvironment::_LuaState, LUA_REGISTRYINDEX, _h->m_Luafunction); // push the referenced function on the stack and pcall it
			
			if (lua_pcall(CLuaEnvironment::_LuaState, 0, 0, 0)) // Some error occured
			{
				PRINT_ERROR("CALLBACK ERROR: %s\n", lua_tostring(CLuaEnvironment::_LuaState, -1));
				lua_pop(CLuaEnvironment::_LuaState, 1);
			}
		}
	}

	// check all 
}

void CLuaThink::PushFunctions()
{
	lua_newtable(CLuaEnvironment::_LuaState);

	lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaThink::Lua_add);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "add");

	lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaThink::Lua_remove);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "remove");

	lua_pushcfunction(CLuaEnvironment::_LuaState, CLuaThink::Lua_list);
	lua_setfield(CLuaEnvironment::_LuaState, -2, "list");

	lua_setglobal(CLuaEnvironment::_LuaState, "think");
}

void CLuaThink::LoadFunctions()
{
}
#pragma endregion



/*
	Adds updates a hook

	interval is in ms

	think.add(string Identifier, number Interval, function Callback)
*/
int CLuaThink::Lua_add(lua_State* State)
{
	const char* _id = lua_tostring(State, 1);
	int _interval = lua_tointeger(State, 2);

	if (_interval <= 0) {
		PRINT_ERROR("ERROR: think.add received invalid interval %i\n", _interval);
		return 0;
	} else if(_interval < 100) {
		PRINT_DEBUG("CLuaThink: Intervals under 100ms can cause performance issues! (%s)\n", _id);
	}

	if (!lua_isfunction(State, 3)) return 0; // no function provided - you dummy

	for each (CLuaThink::ThinkHook * _hook in CLuaThink::m_Hooklist)
	{
		if (_hook->m_Identifier == _id) // already exists - just update
		{
			PRINT_DEBUG("CLuaThink: Removed old hook!\n");
			_hook->m_Interval = _interval;
			lua_unref(State, _hook->m_Luafunction);
			_hook->m_Luafunction = luaL_ref(State, LUA_REGISTRYINDEX);
			return 0;
		}
	}

	// didnt find anything

	ThinkHook * _newhook = new ThinkHook(_id, _interval, luaL_ref(State, LUA_REGISTRYINDEX));
	m_Hooklist.push_back(_newhook);

	return 0;
}



/*
	Removes a hook

	think.remove(string Identifier)
*/
int CLuaThink::Lua_remove(lua_State* State)
{
	const char * _id = lua_tostring(State, 1);

	ThinkHook * hook = nullptr;

	for each (CLuaThink::ThinkHook * _hook in CLuaThink::m_Hooklist)
	{
		if (_hook->m_Identifier == _id) // gotcha
		{
			hook = _hook;
			break;
		}
	}

	if (hook == nullptr) return 0; // not found

	//lua_unref(State, hook->m_Luafunction);
	
	CLuaThink::m_Hooklist.remove(hook);

	delete hook;

	return 0;
}



/*
	Lists all hooks

	table list = think.list()


*/
int CLuaThink::Lua_list(lua_State* State)
{
	lua_newtable(State); // the 'list' to return

	int c = 0;
	for (auto _h : CLuaThink::m_Hooklist)
	{
		lua_pushnumber(State, ++c);

			lua_newtable(State);

			lua_pushstring(State, _h->m_Identifier);
			lua_setfield(State, -2, "identifier");
	
			lua_pushnumber(State, _h->m_Interval);
			lua_setfield(State, -2, "interval");


		// push this table into the list
		lua_settable(State, -3);
	}

	return 1;
}

