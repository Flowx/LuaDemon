#ifdef _WIN32

#include "CLuaEnvironment.h"
#include "PlatformCompatibility.h"
#include <Windows.h>

std::string CLuaEnvironment::_Directory;

// Note: this runs in a seperate thread
// Its purpose is to watch the Lua source code directory
// for file changes to reload the Lua state with the new, changed, code.
// This can be disable in the cmd line using -nofilespy


void CLuaEnvironment::FileSpy()
{
	// NOTE: Sometimes two events are returned for one change. 
	// To prevent reloading twice there is a 100ms cooldown timer
	using namespace std::chrono;
	time_point<system_clock> _lastChange;

	while (true)
	{
		while (!_Initialized & !_Error) std::this_thread::sleep_for(milliseconds(1)); // Block until no errors and current env is ready

		HANDLE _dirHandle = FindFirstChangeNotification(_Directory.c_str(), true, FILE_NOTIFY_CHANGE_LAST_WRITE);

		if (_dirHandle == INVALID_HANDLE_VALUE || _dirHandle == NULL)
		{
			PRINT_ERROR("ERROR: FindFirstChangeNotification() returned invalid or null handle in Lua environment");
		}

		if (WaitForSingleObject(_dirHandle, INFINITE) == WAIT_OBJECT_0)
		{
			if (duration_cast<milliseconds>(system_clock::now() - _lastChange).count() > 100)
			{
				PRINT_DEBUG("File change detected. Reloading.\n");
				_FileChange = true;
				_lastChange = system_clock::now();
			}
		}
		else
		{
			PRINT_WARNING("Something went wrong with the directory handle!\n"); // Do nothing, just ignore it and retry.
		}
	}
}

#endif