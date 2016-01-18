#include "common.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <SDL.h>
#include <sys/stat.h>

#if defined(_WIN32) || defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h> // For Sleep
#endif

string vlformat(const char* format, va_list vl)
{
	char buffer[65536];
	if (vsnprintf(buffer, sizeof(buffer), format, vl) < 0)
		buffer[sizeof(buffer) - 1] = '\0';
	return string(buffer);
}

string format(const char* format, ...)
{
	va_list vl;
	va_start(vl, format);
	string result = vlformat(format, vl);
	va_end(vl);
	return result;
}

void logDebug(const char* format, ...)
{
	va_list vl;
	va_start(vl, format);
	string message = "Debug: " + vlformat(format, vl);
	va_end(vl);
	std::cout << message << std::endl;
	PROFILER_LOG(message.c_str());
}

void logInfo(const char* format, ...)
{
	va_list vl;
	va_start(vl, format);
	string message = vlformat(format, vl);
	va_end(vl);
	std::cout << message << std::endl;
	PROFILER_LOG(message.c_str());
}

void logWarning(const char* format, ...)
{
	va_list vl;
	va_start(vl, format);
	string message = "Warning: " + vlformat(format, vl);
	va_end(vl);
	std::cerr << message << std::endl;
	PROFILER_LOG(message.c_str());
}

void logError(const char* format, ...)
{
	va_list vl;
	va_start(vl, format);
	string message = "ERROR: " + vlformat(format, vl);
	va_end(vl);
	std::cerr << message << std::endl;
	PROFILER_LOG(message.c_str());
}

void panic(const char* format, ...)
{
	va_list vl;
	va_start(vl, format);
	string message = "PANIC: " + vlformat(format, vl);
	va_end(vl);
	std::cerr << message << std::endl;
	PROFILER_LOG(message.c_str());

	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
		"Fatal Error", message.c_str(), NULL);

	exit(EXIT_FAILURE);
}

string readFile(const string& path)
{
	std::ifstream f(path);
	std::stringstream buffer;
	buffer << f.rdbuf();
	return buffer.str();
}

string replace(string str, const string& search, const string& replace)
{
	for (size_t pos = 0, len = replace.length(); ; pos += len) {
		pos = str.find(search, pos);
		if (pos == string::npos) break;
		str.replace(pos, search.length(), replace);
	}
	return str;
}

bool endsWith(const string& str, const string& ending)
{
	const int strLen = str.length(), endingLen = ending.length();
	return strLen >= endingLen &&
		str.compare(strLen - endingLen, endingLen, ending) == 0;
}

uint timestamp(const string& path)
{
#if defined(_MSC_VER) || defined(__MINGW32__)
	struct _stat buf;
	// TODO: Unicode
	if (_stat(path.c_str(), &buf) == 0)
		return buf.st_mtime;
#else
	struct stat buf;
	if (stat(path.c_str(), &buf) == 0)
		return buf.st_mtime;
#endif
	return 0;
}

void sleep(uint ms)
{
#if defined(_WIN32) || defined(WIN32)
	Sleep(ms);
#else
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
#endif
}
