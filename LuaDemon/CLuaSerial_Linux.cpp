#include "CLuaSerial.h"
#include "PlatformCompatibility.h"
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>

std::map<std::string, CLuaSerial::CLuaSerialPort *> CLuaSerial::m_PortList;

// Lua Exposed Functions

// TODO: Figure this out on Linux
// Lua param:
// none
// Returns all available COM Ports
int CLuaSerial::Lua_Discover(lua_State * State)
{
	//char _paths[0xFF], _namebuf[7];

	//size_t _k = 0;

	//lua_newtable(State);

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

	return 0;
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
		CLuaSerial::CLuaSerialPort *_P = m_PortList[_portname];
		close(_P->m_PortReference);
		delete _P;
		m_PortList.erase(_portname);
		PRINT_DEBUG("Closed existing port %s\n", _portname.c_str());
	}

	std::string buff = ("/dev/" + _portname); // NOTE: This provides access to everything in dev/ --- Is this a security issue?

	int fd = open(buff.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1)
	{
		PRINT_WARNING("Failed to open port %s (Missing permissions?)\n", _portname.c_str());
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

	CLuaSerial::m_PortList[_portname] = new CLuaSerial::CLuaSerialPort(_portname.c_str());
	CLuaSerial::m_PortList[_portname]->m_PortReference = fd;

	lua_pushboolean(State, 1);
	return 1;
}

// Lua param:
// string Portname, var Data
// Sends Data
// NOTE: Crude and unsafe but works for now
int CLuaSerial::Lua_Send(lua_State * State)
{
	std::string _portname = lua_tostring(State, 1);

	if (_portname.empty()) return 0;

	int _file;
	if (m_PortList.count(_portname) > 0)
	{
		CLuaSerial::CLuaSerialPort *_P = m_PortList[_portname];
		_file = _P->m_PortReference;
	}
	else return 0;

	char _databyte = (char)lua_tointeger(State, 2);

	if (lua_isstring(State, 2))
	{
		size_t _l = lua_strlen(State, 2);
		if (_l == 0) return 0;

		const char * _data = lua_tostring(State, 2);

		write(_file, _data, _l);
	}

	return 0;
}

// Lua param:
// string Portname
// Returns available data (in bytes) ready to be read
int CLuaSerial::Lua_Available(lua_State * State)
{
	std::string _portname = lua_tostring(State, 1);

	if (_portname.empty()) return 0;

	if (m_PortList.count(_portname) > 0)
	{
		CLuaSerial::CLuaSerialPort *_P = m_PortList[_portname];
		
		int _available = 0;

		ioctl(_P->m_PortReference, FIONREAD, &_available);

		lua_pushinteger(State, _available);

		return 1;
	}

	return 0;
}

// Lua param:
// string Portname, number count = 1
// Reads a certain amount from buffer
int CLuaSerial::Lua_Read(lua_State * State)
{
	std::string _portname = lua_tostring(State, 1);

	if (!_portname.empty() && (m_PortList.count(_portname) > 0))
	{
		CLuaSerial::CLuaSerialPort *_P = m_PortList[_portname];

		size_t _Length = 0;

		ioctl(_P->m_PortReference, FIONREAD, &_Length);

		_P->m_FreeBuffer = new (std::nothrow) char[_Length];
		_P->m_IsFreed = false;

		if (read(_P->m_PortReference, _P->m_FreeBuffer, _Length) != _Length)
		{
			PRINT_WARNING("Read on %s failed!\n", _portname);
			return 0;
		}

		_P->m_LastAvailable = 0;

		lua_pushlstring(State, _P->m_FreeBuffer, _Length);

		return 1;
	}
	return 0;
}

// Lua param:
// string Portname
// Read all currently available data; Returns string
int CLuaSerial::Lua_ReadAll(lua_State * State)
{
	std::string _portname = lua_tostring(State, 1);

	if (!_portname.empty() && (m_PortList.count(_portname) > 0))
	{
		CLuaSerial::CLuaSerialPort *_P = m_PortList[_portname];

		size_t _Length = 0;

		ioctl(_P->m_PortReference, FIONREAD, &_Length);

		_P->m_FreeBuffer = new (std::nothrow) char[_Length];
		_P->m_IsFreed = false;

		if(read(_P->m_PortReference, _P->m_FreeBuffer, _Length) != _Length )
		{
			PRINT_WARNING("Read on %s failed!\n", _portname.c_str());
			return 0;
		}

		_P->m_LastAvailable = 0;

		lua_pushlstring(State, _P->m_FreeBuffer, _Length);

		return 1;
	}
	return 0;
}

void CLuaSerial::PollFunctions()
{
	for (std::pair<std::string, CLuaSerial::CLuaSerialPort*> _v : m_PortList)
	{
		CLuaSerial::CLuaSerialPort * Port = _v.second;
		
		// TODO: Add ioctl to check if file descriptor is valid

		if (!Port->m_IsFreed && Port->m_FreeBuffer != 0) // This deletes the buffer created by ReadAll()
		{
			delete[] Port->m_FreeBuffer;
			Port->m_IsFreed = true;
			Port->m_FreeBuffer = NULL;
		}

		// Calls the lua callback whenever a new byte arrives
		size_t _Available = 0;
		ioctl(Port->m_PortReference, FIONREAD, &_Available);

		if (_Available > Port->m_LastAvailable)
		{
			Port->m_LastAvailable = _Available;

			if (!Port->m_LuaReference) continue; // no Lua function available

			lua_rawgeti(CLuaEnvironment::_LuaState, LUA_REGISTRYINDEX, Port->m_LuaReference); // push the referenced function on the stack and pcall it

			if (lua_pcall(CLuaEnvironment::_LuaState, 0, 0, 0)) // Some error occured
			{
				PRINT_WARNING("CALLBACK ERROR: %s\n", lua_tostring(CLuaEnvironment::_LuaState, -1));
				lua_pop(CLuaEnvironment::_LuaState, 1);
			}
		}
	}
}
