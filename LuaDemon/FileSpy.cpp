#include "CLuaEnvironment.h"
#include "PlatformCompatibility.h"

std::string CLuaEnvironment::_Directory;

// Note: this runs in a seperate thread
// Its purpose is to watch the Lua source code directory
// for file changes to reload the Lua state with the new, changed, code.
// This can be disable in the cmd line using -nofilespy

#if _WIN32

#include <Windows.h>

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
			if (duration_cast<milliseconds>(system_clock::now() - _lastChange).count(); > 100)
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
#else // If its not Windows its probably Linux or some Unix derivative

#include <sys/inotify.h>
#include <sys/ioctl.h>

//#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

// TODO: this is half assed and should be cleaned up
void CLuaEnvironment::FileSpy()
{
	using namespace std::chrono;
	time_point<system_clock> _lastChange;
	char buf[BUF_LEN];

	int inotifyFd = inotify_init();
	if (inotifyFd == -1)
	{
		PRINT_ERROR("inotify_init() returned %d\n", inotifyFd);
		return;
	}

	int wd = inotify_add_watch(inotifyFd, _Directory.c_str(), IN_CLOSE_WRITE);

	for (;;) 
	{
		// Block until no errors and current env is ready
		while (!_Initialized & !_Error) std::this_thread::sleep_for(std::chrono::milliseconds(1)); 

		int numRead = read(inotifyFd, buf, BUF_LEN);
		if (numRead == 0 || numRead == -1)
		{
			PRINT_ERROR("read() from inotify fd returned 0 or error!\n");
			continue;
		}

		if (duration_cast<milliseconds>(system_clock::now() - _lastChange).count() > 100)
		{
			PRINT_DEBUG("File change detected. Reloading.\n");
			_FileChange = true;
			_lastChange = system_clock::now();
		}
	}
}
#endif