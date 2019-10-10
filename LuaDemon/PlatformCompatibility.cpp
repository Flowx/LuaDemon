#pragma once
#include "PlatformCompatibility.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

/*
	PlatformCompatability.c

	Provides functions to unfiy UNIX and Windows IO
*/

bool __nodebug = false; // True = supress debug messages to be printed

#ifdef _WIN32 // Win32 console
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

HANDLE ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

void PRINT(const char * Message, ...)
{
	va_list args;
	va_start(args, Message);

	SetConsoleTextAttribute(ConsoleHandle, PRINTCOLOR::DEFAULT);
	vprintf(Message, args);

	va_end(args);
}

void PRINT_SUCCESS(const char * Message, ...)
{
	va_list args;
	va_start(args, Message);

	SetConsoleTextAttribute(ConsoleHandle, PRINTCOLOR::SUCCESS);
	vprintf(Message, args);
	SetConsoleTextAttribute(ConsoleHandle, PRINTCOLOR::DEFAULT);

	va_end(args);
};

void PRINT_WARNING(const char * Message, ...)
{
	va_list args;
	va_start(args, Message);

	SetConsoleTextAttribute(ConsoleHandle, PRINTCOLOR::WARNING);
	vprintf(Message, args);
	SetConsoleTextAttribute(ConsoleHandle, PRINTCOLOR::DEFAULT);

	va_end(args);
};

void PRINT_ERROR(const char * Message, ...)
{
	va_list args;
	va_start(args, Message);

	SetConsoleTextAttribute(ConsoleHandle, PRINTCOLOR::FAILURE);
	vprintf(Message, args);
	SetConsoleTextAttribute(ConsoleHandle, PRINTCOLOR::DEFAULT);

	va_end(args);
};

void PRINT_DEBUG(const char * Message, ...)
{
	if (__nodebug) return;

	va_list args;
	va_start(args, Message);

	SetConsoleTextAttribute(ConsoleHandle, PRINTCOLOR::DEBUG);
	vprintf(Message, args);
	SetConsoleTextAttribute(ConsoleHandle, PRINTCOLOR::DEFAULT);

	va_end(args);
};

#else // UNIX systems

#include <cstdarg>

#define LUASANDBOX_DEFAULTCOLOR "\x1B[0m"
void PRINT(const char * Message, ...)
{
	va_list args;
	va_start(args, Message);

	vprintf(Message, args);

	va_end(args);
};

void PRINT_SUCCESS(const char * Message, ...)
{
	va_list args;
	va_start(args, Message);

	printf("\x1B[%dm", PRINTCOLOR::SUCCESS);
	vprintf(Message, args);
	printf(LUASANDBOX_DEFAULTCOLOR);

	va_end(args);
};

void PRINT_WARNING(const char * Message, ...)
{
	va_list args;
	va_start(args, Message);

	printf("\x1B[%dm", PRINTCOLOR::WARNING);
	vprintf(Message, args);
	printf(LUASANDBOX_DEFAULTCOLOR);

	va_end(args);
};

void PRINT_ERROR(const char * Message, ...)
{
	va_list args;
	va_start(args, Message);

	printf("\x1B[%dm", PRINTCOLOR::FAILURE);
	vprintf(Message, args);
	printf(LUASANDBOX_DEFAULTCOLOR);

	va_end(args);
};

void PRINT_DEBUG(const char * Message, ...)
{
	if (__nodebug) return;

	va_list args;
	va_start(args, Message);

	printf("\x1B[%dm", PRINTCOLOR::DEBUG);
	vprintf(Message, args);
	printf(LUASANDBOX_DEFAULTCOLOR);

	va_end(args);
};

#endif

bool EXISTS_DIRECTORY(const char *path)
{

	int a = strlen(path);
	char * buffer = (char *)malloc(strlen(path) + 1);

	int i = 0;
	while (path[i] != 0 && i < 0xFFFF)
	{
		if (path[i] == '\\')
			buffer[i] = '/';
		else
			buffer[i] = path[i];

		i++;
	}

	buffer[i] = 0; // null terminate it
	
	struct stat info;

	if (stat(buffer, &info) != 0)
	{
		free(buffer);
		return false;
	}
	else if (info.st_mode & S_IFDIR)
	{
		free(buffer);
		return true;
	}

	free(buffer);
	return false;
}

bool EXISTS_FILE(const char * Path)
{
	struct stat _buffer;
	return (stat(Path, &_buffer) == 0);
}

//void PATH_NORMALIZE(std::string Path)
//{
//	for (size_t i = 0; i < CLuaEnvironment::_Directory.length(); i++)
//		if (CLuaEnvironment::_Directory[i] == '\\') CLuaEnvironment::_Directory[i] = '/';
//}