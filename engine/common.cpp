#include "common.hpp"
#include <iostream>
#include <fstream>
#include <SDL2/SDL.h>

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
	string message = vlformat(format, vl);
	va_end(vl);
	std::cout << "Debug: " << message << std::endl;
}

void logInfo(const char* format, ...)
{
	va_list vl;
	va_start(vl, format);
	string message = vlformat(format, vl);
	va_end(vl);
	std::cout << message << std::endl;
}

void logWarning(const char* format, ...)
{
	va_list vl;
	va_start(vl, format);
	string message = vlformat(format, vl);
	va_end(vl);
	std::cerr << "Warning: " << message << std::endl;
}

void logError(const char* format, ...)
{
	va_list vl;
	va_start(vl, format);
	string message = vlformat(format, vl);
	va_end(vl);
	std::cerr << "ERROR: " << message << std::endl;
}

void panic(const char* format, ...)
{
	va_list vl;
	va_start(vl, format);
	string message = vlformat(format, vl);
	va_end(vl);
	std::cerr << "PANIC: " << message << std::endl;

	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
		"Fatal Error", message.c_str(), NULL);

	exit(EXIT_FAILURE);
}

string readFile(const string& path)
{
	std::ifstream f(path);
	return std::string(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
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
