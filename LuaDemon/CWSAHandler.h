#if _WIN32
#pragma once

/*
	Only required on Windows.
	This just makes sure WSA is running and isnt initialized multiple times.
*/

class CWSAHandler
{
public:
	static bool initWSA();
	static void closeWSA();

	static inline bool m_Init;
};

#endif