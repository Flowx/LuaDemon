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
#else // If its not Windows its probably Linux or some Unix derivative

#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <vector>
#include <string>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

std::vector<std::string> _DirectoryList = std::vector<std::string>();

void ReadDirectory(std::string dir)
{
	DIR *dp;
	if ((dp = opendir(dir.c_str())) == NULL) {
		PRINT_ERROR("Directory lookup failed. Dir: %s\n", dir.c_str());
		return;
	}

	struct dirent *_entry;
	while ((_entry = readdir(dp)) != NULL) {
		std::string _tmp = _entry->d_name;
		if (_tmp == "." || _tmp == ".." || _entry->d_type != DT_DIR) continue;
		ReadDirectory((dir + _tmp + '/'));
		_DirectoryList.push_back(std::string(dir + _tmp + '/'));
	}
	closedir(dp);
}

// Now supports recursive directory lookup
// cant do anything about the spastic reloads caused by shitty Linux editors though
void CLuaEnvironment::FileSpy()
{
	using namespace std::chrono;
	time_point<system_clock> _lastChange;
	char buf[BUF_LEN];

	ReadDirectory(_Directory.c_str());

	int inotifyFd = inotify_init();
	if (inotifyFd == -1)
	{
		PRINT_ERROR("inotify_init() returned %d\n", inotifyFd);
		return;
	}

	_DirectoryList.push_back(_Directory);

	for (unsigned int i = 0; i < _DirectoryList.size(); i++) inotify_add_watch(inotifyFd, _DirectoryList[i].c_str(), IN_CLOSE_WRITE);

	for (;;) 
	{
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