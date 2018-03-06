#if !_WIN32 // ignore this file on linux

#include "CLuaSerial.h"
#include "PlatformCompatibility.h"
//#include <stdio.h>
//#include <termios.h>
#include <fcntl.h>

std::map<std::string, CLuaSerialPort *> CLuaSerial::m_PortList;

// Lua Exposed Functions

// TODO: Figure this out on Linux
// Lua param:
// none
// Returns all available COM Ports
int CLuaSerial::Lua_Discover(lua_State * State)
{
	//char _paths[0xFF], _namebuf[7];

	//size_t _k = 0;

	lua_newtable(State);

	//for (int i = 0; i <= 0xFF; i++)
	//{
	//	sprintf(_namebuf, "COM%i", i);
	//	if (QueryDosDevice(_namebuf, _paths, sizeof(_paths)))
	//	{
	//		_k++;
	//		lua_pushinteger(State, _k);
	//		lua_pushstring(State, _namebuf);
	//		lua_settable(State, -3);
	//	}
	//}

	return 1;
}

// Lua param:
// string Portname, number Portspeed = 9600, number Bytesize = 8, number Stopbits = 1
// Opens a new COM Port
int CLuaSerial::Lua_Open(lua_State * State)
{
	std::string _portname = lua_tostring(State, 1);

	if (_portname.empty()) return 0;

	size_t _portspeed = lua_tointeger(State, 2);
	char _bytesize = lua_tointeger(State, 3);
	char _stopbits = lua_tointeger(State, 4);

	if (!_portspeed) _portspeed = 9600;
	if (!_bytesize) _bytesize = 8;
	if (!_stopbits) _stopbits = 1;

	if (m_PortList.count(_portname) > 0) // there is already a Lua serial port on this port
	{
		CLuaSerialPort *_P = m_PortList[_portname];
		//CloseHandle(_P->m_PortReference);
		delete _P;
		m_PortList.erase(_portname);
	}

	int fd; // file description for the serial port

	std::string buff = ("/dev/" + _portname); // NOTE: This provides access to everything in dev/ --- Is this a security issue?

	fd = open(buff.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1)
	{
		PRINT_DEBUG("Failed to open port %s (Missing permissions?)\n", _portname.c_str());
		lua_pushboolean(State, 0);
		return 1;
	}
	
	fcntl(fd, F_SETFL, 0);

	struct termios tty;

	if (tcgetattr(fd, &tty) < 0) {
		//printf("Error from tcgetattr: %s\n", strerror(errno));
		PRINT_ERROR("ERROR: tcgetattr");
		lua_pushboolean(State, 0);
		return 1;
	}

	cfsetospeed(&tty, (speed_t)_portspeed);
	cfsetispeed(&tty, (speed_t)_portspeed);

	tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= CS8;         /* 8-bit characters */
	tty.c_cflag &= ~PARENB;     /* no parity bit */
	tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
	tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

	tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	tty.c_oflag &= ~OPOST;

	/* fetch bytes as they become available */
	// NOTE: THIS IS IMPORTANT!!!!
	tty.c_cc[VMIN] = 1;
	tty.c_cc[VTIME] = 1;

	if (tcsetattr(fd, TCSANOW, &tty) != 0) {
		//printf("Error from tcsetattr: %s\n", strerror(errno));
		PRINT_ERROR("ERROR: tcgetattr");
		lua_pushboolean(State, 0);
		return 1;
	}




	lua_pushboolean(State, 1);
	return 1;
}

// Lua param:
// string Portname, var Data
// Sends Data
int CLuaSerial::Lua_Send(lua_State * State)
{
	return 0;
}

// Lua param:
// string Portname
// Returns available data (in bytes) ready to be read
int CLuaSerial::Lua_Available(lua_State * State)
{
	return 0;
}

// Lua param:
// string Portname
// Read all currently available data; Returns string
int CLuaSerial::Lua_ReadAll(lua_State * State)
{
	return 0;
}


// Lua param:
// string Portname, function Callback
// Receives Data
int CLuaSerial::Lua_Receive(lua_State * State)
{
	return 0;
}


void CLuaSerial::PollFunctions()
{
}

#endif