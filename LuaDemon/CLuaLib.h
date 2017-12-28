#pragma once
#include <list>


class CLuaLib
{
	static std::list<CLuaLib> m_Libraries;

public:
	static void Initialize();
};

