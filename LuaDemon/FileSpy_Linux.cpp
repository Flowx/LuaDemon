#include "CLuaEnvironment.h"
#include "PlatformCompatibility.h"
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <vector>
#include <string>

#define EVENT_SIZE  (sizeof (struct inotify_event))
#define BUF_LEN     (1024 * (EVENT_SIZE + 16))

//int CLuaEnvironment::_FileSpyInterval;
std::string CLuaEnvironment::_Directory;
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

long GetLastModifiedTimestamp(std::string dir)
{
	long _last = 0;

	DIR *dp;
	if ((dp = opendir(dir.c_str())) == NULL) {
		PRINT_ERROR("Directory lookup failed. Dir: %s\n", dir.c_str());
		return 0;
	}

	struct dirent *_entry;
	while ((_entry = readdir(dp)) != NULL) {
		std::string _tmp = _entry->d_name;
		if (_tmp == "." || _tmp == "..") continue;

		if (_entry->d_type == DT_DIR)
		{
			long _t = GetLastModifiedTimestamp((dir + _tmp + '/'));
			if (_t > _last) _last = _t;
		}
		else if(_entry->d_type == DT_REG)
		{
			std::string _filePath(dir + _tmp);

			struct stat attrib;
			stat(_filePath.c_str(), &attrib);

			long _t = attrib.st_ctime;
			if (_t > _last) _last = _t;
		}

		//_DirectoryList.push_back(std::string(dir + _tmp + '/'));
	}
	closedir(dp);

	return _last;
}

// Now supports recursive directory lookup
// cant do anything about the spastic reloads caused by shitty Linux text editors though

long _lastTime = 0;
void CLuaEnvironment::FileSpy()
{
	using namespace std::chrono;
	time_point<system_clock> _lastChange;

	_lastTime = GetLastModifiedTimestamp(_Directory.c_str());

	if (_FileSpyInterval != 0)
	{
		for (;;)
		{
			long time = GetLastModifiedTimestamp(_Directory.c_str());
			
			if (_lastTime < time)
			{
				_lastTime = time;
				_FileChange = true;
				_lastChange = system_clock::now();
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(_FileSpyInterval));
		}
	}
	else 
	{
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
}