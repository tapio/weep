#include "common.hpp"
#include "utils.hpp"
#include <iostream>
#include <unordered_map>
#include <SDL.h>

#if !defined(_WIN32) && !defined(WIN32)
namespace unistd {
#include <unistd.h> // isatty()
}
#endif

#define ANSI_RESET                "\033[0m"
#define ANSI_BLACK                "\033[22;30m"
#define ANSI_RED                  "\033[22;31m"
#define ANSI_GREEN                "\033[22;32m"
#define ANSI_BROWN                "\033[22;33m"
#define ANSI_BLUE                 "\033[22;34m"
#define ANSI_MAGENTA              "\033[22;35m"
#define ANSI_CYAN                 "\033[22;36m"
#define ANSI_GREY                 "\033[22;37m"
#define ANSI_DARKGREY             "\033[01;30m"
#define ANSI_LIGHTRED             "\033[01;31m"
#define ANSI_LIGHTGREEN           "\033[01;32m"
#define ANSI_YELLOW               "\033[01;33m"
#define ANSI_LIGHTBLUE            "\033[01;34m"
#define ANSI_LIGHTMAGENTA         "\033[01;35m"
#define ANSI_LIGHTCYAN            "\033[01;36m"
#define ANSI_WHITE                "\033[01;37m"
#define ANSI_BACKGROUND_BLACK     "\033[40m"
#define ANSI_BACKGROUND_RED       "\033[41m"
#define ANSI_BACKGROUND_GREEN     "\033[42m"
#define ANSI_BACKGROUND_YELLOW    "\033[43m"
#define ANSI_BACKGROUND_BLUE      "\033[44m"
#define ANSI_BACKGROUND_MAGENTA   "\033[45m"
#define ANSI_BACKGROUND_CYAN      "\033[46m"
#define ANSI_BACKGROUND_WHITE     "\033[47m"

static void printColorized(std::ostream* out, const std::string& msg, const char* color) {
#if defined(_WIN32) || defined(WIN32)
	*out << msg << std::endl; // Usually no ANSI escape support on Windows
#else
	if (unistd::isatty(out == &std::cerr ? 2 : 1))
		*out << color << msg << ANSI_RESET << std::endl;
	else *out << msg << std::endl;
#endif
}

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
	printColorized(&std::cout, message, ANSI_CYAN);
	PROFILER_LOG(message.c_str());
}

void logInfo(const char* format, ...)
{
	va_list vl;
	va_start(vl, format);
	string message = vlformat(format, vl);
	va_end(vl);
	printColorized(&std::cout, message, ANSI_GREEN);
	PROFILER_LOG(message.c_str());
}

void logWarning(const char* format, ...)
{
	va_list vl;
	va_start(vl, format);
	string message = "Warning: " + vlformat(format, vl);
	va_end(vl);
	printColorized(&std::cerr, message, ANSI_YELLOW);
	PROFILER_LOG(message.c_str());
}

void logError(const char* format, ...)
{
	va_list vl;
	va_start(vl, format);
	string message = "ERROR: " + vlformat(format, vl);
	va_end(vl);
	printColorized(&std::cerr, message, ANSI_RED);
	PROFILER_LOG(message.c_str());
}

void panic(const char* format, ...)
{
	va_list vl;
	va_start(vl, format);
	string message = "PANIC: " + vlformat(format, vl);
	va_end(vl);
	printColorized(&std::cerr, message, ANSI_RED);
	PROFILER_LOG(message.c_str());

	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
		"Fatal Error", message.c_str(), NULL);

	exit(EXIT_FAILURE);
}



static std::unordered_map<string, CVarBase*> s_cvars;

void CVarBase::registerCVar(const string& name, CVarBase* cvar)
{
	if (name.empty()) { logError("Registering empty cvar!"); return; }
	if (!cvar) { logError("Registering nullptr cvar!"); return; }
	string nameLower = utils::tolower(name);
	if (s_cvars.count(nameLower) > 0) { logWarning("Re-registering cvar %s", name.c_str()); }
	s_cvars[nameLower] = cvar;
}

CVarBase* CVarBase::getCVar(const string& name)
{
	auto it = s_cvars.find(utils::tolower(name));
	if (it != s_cvars.end())
		return it->second;
	return nullptr;
}

std::vector<string> CVarBase::getMatchingCVarNames(const std::string& prefix)
{
	string prefixLower = utils::tolower(prefix);
	std::vector<string> res;
	for (auto& it : s_cvars) {
		if (utils::startsWith(it.first, prefixLower))
			res.push_back(it.second->name);
	}
	return res;
}

bool CVarBase::tryParseFrom(const std::string& newValue)
{
	if (newValue.empty()) return false;
	if (newValue == "true") { value = 1; return true; }
	if (newValue == "false") { value = 0; return true; }
	// GCC lacks float std::from_chars even though it's C++17...
	try {
		ValueType parsed = std::stof(newValue);
		value = parsed;
		return true;
	} catch (...) { }
	return false;
}
