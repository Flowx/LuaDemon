#pragma once
// The following functions allow you to print color messages on any system, Windows and Linux

#ifdef _WIN32 // Win32 console

#define SLEEP(Time) Sleep(Time)

enum PRINTCOLOR
{
	DEFAULT = 0x7, // Default console color, usually grey
	SUCCESS = 0xA, // Green
	WARNING = 0xE, // Orange or yellow
	FAILURE = 0xC, // Red; Note: ERROR keyword is taken by WinAPI
	DEBUG	= 0xB  // Cyan or blue 
};

#else
#include <unistd.h>

//#define SLEEP(Time) usleep(Time * 1000);

// Linux colors
enum PRINTCOLOR
{
	DEFAULT = 0,  // Default console color, usually grey
	SUCCESS = 32, // Green
	WARNING = 33, // Orange or yellow
	FAILURE = 31, // Red, Note: ERROR keyword is taken by winAPI
	DEBUG = 36  // Cyan or blue 
};

#endif

void PRINT(const char *, ...);
void PRINT_SUCCESS(const char *, ...);
void PRINT_WARNING(const char *, ...);
void PRINT_ERROR(const char *, ...);
void PRINT_DEBUG(const char *, ...);

bool EXISTS_DIRECTORY(const char *);
bool EXISTS_FILE(const char *);
