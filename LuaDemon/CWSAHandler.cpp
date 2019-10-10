#if _WIN32

#define _WSA

#include "CWSAHandler.h"
#include "PlatformCompatibility.h"
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

bool CWSAHandler::initWSA()
{
	if (m_Init) return true; // already initialized

	WSADATA wsaData;

	int err = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (err != 0)
	{
		switch (err)
		{
		case WSASYSNOTREADY:
			PRINT_ERROR("ERROR: WSAStartup; WSASYSNOTREADY\n");
			break;
		case WSAVERNOTSUPPORTED:
			PRINT_ERROR("ERROR: WSAStartup; WSAVERNOTSUPPORTED\n");
			break;
		case WSAEINPROGRESS:
			PRINT_ERROR("ERROR: WSAStartup; WSAEINPROGRESS\n");
			break;
		case WSAEPROCLIM:
			PRINT_ERROR("ERROR: WSAStartup; WSAEPROCLIM\n");
			break;
		case WSAEFAULT:
			PRINT_ERROR("ERROR: WSAStartup; WSAEFAULT\n");
			break;
		default:
			PRINT_ERROR("ERROR: WSAStartup; Unknown Error; %i\n", err);
			break;
		}

		return false;
	}
	PRINT_DEBUG("WSAStartup successful! Version: %i.%i\n", (char)wsaData.wVersion, (char)(wsaData.wVersion >> 8)); // is always going to be 2.2 but eh
	m_Init = true;
	return true;
}

void CWSAHandler::closeWSA()
{
	if (!m_Init) return; // not running - nothing to close
	WSACleanup();
}

#endif