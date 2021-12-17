#include "utils.hpp"

#include <algorithm>
#include <iostream>
#include <cctype>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <sys/stat.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#elif __APPLE__
#include <mach-o/dyld.h>
#else
#include <unistd.h>
#endif

namespace fs = std::filesystem;

namespace utils {

void sleep(unsigned ms)
{
#if defined(_WIN32) || defined(WIN32)
	Sleep(ms);
#else
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
#endif
}

unsigned timestamp(const std::string& path)
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

std::string readFile(const std::string& path, bool binary)
{
	std::ifstream f(path, std::ios_base::in | (binary ? std::ios_base::binary : std::ios_base::in /*dummy*/));
	std::stringstream buffer;
	buffer << f.rdbuf();
	return buffer.str();
}

bool writeFile(const std::string& path, const std::string& contents, bool binary, bool append)
{
	std::ofstream f(path, std::ios_base::out | (binary ? std::ios_base::binary : std::ios_base::out /*dummy*/) | (append ? std::ios_base::app : std::ios_base::trunc));
	f << contents;
	return f.good();
}

bool fileExists(const std::string& path)
{
	std::error_code ec;
	return fs::exists(path, ec);
}

bool copyFiles(const std::string& from, const std::string& to)
{
	std::error_code ec;
	fs::copy(from, to, fs::copy_options::overwrite_existing | fs::copy_options::recursive, ec);
	return !ec;
}

bool deleteFile(const std::string& path)
{
	std::error_code ec;
	return fs::remove(path, ec);
}

bool deleteRecursive(const std::string& path)
{
	std::error_code ec;
	return fs::remove_all(path, ec) > 0;
}

bool createDirectories(const std::string& path)
{
	std::error_code ec;
	return fs::create_directories(path, ec);
}

std::string getTempDir(const std::string& subdir, bool clearSubdir)
{
	auto tempDir = fs::temp_directory_path().string(); // No ec, let's throw on error
	if (!tempDir.empty() && !subdir.empty()) {
		tempDir = joinPaths(tempDir, subdir);
		if (clearSubdir && fileExists(tempDir)) {
			deleteRecursive(tempDir);
		}
		createDirectories(tempDir);
	}
	return tempDir;
}

std::vector<std::string> findFiles(const std::string& path, const std::string& contains, const std::vector<std::string>& exclude)
{
	std::vector<std::string> paths;
	for (const auto& entry : fs::directory_iterator(path)) {
		if (entry.is_directory())
			continue;
		std::string p = entry.path().string();
		if (!contains.empty() && p.find(contains) == std::string::npos)
			continue;
		if (!exclude.empty()) {
			bool excluded = false;
			for (const auto& excludeCandidate : exclude) {
				if (p.find(excludeCandidate) != std::string::npos) {
					excluded = true;
					break;
				}
			}
			if (excluded)
				continue;
		}
		paths.push_back(std::move(p));
	}
	return paths;
}

std::string getExeDir()
{
#if defined(_WIN32)
	char buf[1024];
	DWORD ret = GetModuleFileName(NULL, buf, sizeof(buf));
	if (ret == 0 || ret == sizeof(buf)) return "";
#elif __APPLE__
	char buf[1024];
	uint32_t size = sizeof(buf);
	int ret = _NSGetExecutablePath(buf, &size);
	if (ret != 0) return "";
#else
	char buf[1024];
	ssize_t size = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
	if (size < 0) return "";
	buf[size] = '\0';
#endif
	return removeFilename(buf);
}

static bool hasTrailingSlash(const std::string& path)
{
	if (path.empty())
		return false;
	const char c = path[path.size() - 1];
	return c == '/' || c == '\\';
}

static char guessPathSeparator(const std::string& path)
{
	auto index = path.find_first_of("/\\");
	return index != std::string::npos ? path[index] : '/';
}

std::string removeFilename(const std::string& path)
{
	if (hasTrailingSlash(path))
		return path;
	return path.substr(0, path.find_last_of("/\\") + 1);
}

std::string removeExtension(const std::string& path)
{
	if (hasTrailingSlash(path))
		return path;
	size_t lastDot = path.find_last_of(".");
	if (lastDot == std::string::npos)
		return path;
	size_t lastSlash = path.find_last_of("/\\");
	if (lastSlash != std::string::npos && lastSlash > lastDot)
		return path;
	return path.substr(0, lastDot);
}

std::string extractFilename(const std::string& path)
{
	if (hasTrailingSlash(path))
		return "";
	auto index = path.find_last_of("/\\");
	return index != std::string::npos ? path.substr(index + 1) : path;
}

std::string joinPaths(const std::string& a, const std::string& b)
{
	if (a.empty()) return b;
	if (b.empty()) return a;
	if (hasTrailingSlash(a))
		return a + b;
	return a + guessPathSeparator(a) + b;
}


int execute(const std::string& cmd)
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	std::string exec = "start /b \"" + cmd + "\""; // + " & pause";
#else
	std::string exec = "xterm -e '" + cmd + "'";
#endif
	return system(cmd.c_str());
}

int openUrl(const std::string& url)
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
	return system(("start " + url).c_str());
#else
	return system(("open " + url).c_str());
#endif
}


std::string tolower(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(),
		[](unsigned char c) { return std::tolower(c); }
	);
	return s;
}

std::string toupper(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(),
		[](unsigned char c) { return std::toupper(c); }
	);
	return s;
}

std::string nicifyVariableName(const std::string& name)
{
	std::string ret = "";
	ret.reserve(name.length() + 2);
	char prev = 0;
	for (unsigned i = 0; i < name.size(); i++) {
		const char c = name[i];
		if (i == 0 || prev == ' ' || prev == '_' || prev == '(') { // First letter or after space is uppercase
			ret += std::toupper(c);
		} else if (c == '_') { // Convert _ to space
			ret += ' ';
		} else if (std::isupper(c) && !std::isupper(prev) && std::isalnum(prev)) { // Prepend space to uppercase letter in the middle of word
			ret += ' ';
			ret += c;
		} else if (std::isdigit(prev) && std::isalpha(c)) { // Prepend space and uppercase after digit
			ret += ' ';
			ret += std::toupper(c);
		} else ret += c;
		prev = c;
	}
	return ret;
}

std::string variablizeName(const std::string& name, bool capitalizeFirst)
{
	std::string ret = "";
	ret.reserve(name.length());
	char prev = 0;
	for (unsigned i = 0; i < name.size(); i++) {
		const char c = name[i];
		if (i == 0) {
			ret += capitalizeFirst ? std::toupper(c) : std::tolower(c);
		}
		else if (c == ' ') {
			//ret += '_';
		}
		else if (prev == ' ') {
			ret += std::toupper(c);
		}
		else ret += c;
		prev = c;
	}
	return ret;
}

std::string insensitivize(const std::string& name)
{
	std::string ret = "";
	ret.reserve(name.length());
	for (unsigned i = 0; i < name.size(); i++) {
		const char c = name[i];
		if (c == ' ' || c == '\t')
			continue;
		ret += std::tolower(c);
	}
	return ret;
}

std::string replace(std::string str, const std::string& search, const std::string& replace)
{
	for (size_t pos = 0, len = replace.length(); ; pos += len) {
		pos = str.find(search, pos);
		if (pos == std::string::npos) break;
		str.replace(pos, search.length(), replace);
	}
	return str;
}

std::string trim(std::string s)
{
	return rtrim(ltrim(s));
}

std::string ltrim(std::string s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
	}));
	return s;
}

std::string rtrim(std::string s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
	}).base(), s.end());
	return s;
}

std::string parseEscapes(const std::string& str)
{
	return replace(replace(replace(replace(replace(replace(str, "\\f", "\f"), "\\n", "\n"), "\\r", "\r"), "\\t", "\t"), "\\v", "\v"), "\\\\", "\\");
}

bool endsWith(const std::string& str, const std::string& ending)
{
	const int strLen = (int)str.length(), endingLen = (int)ending.length();
	return strLen >= endingLen &&
		str.compare(strLen - endingLen, endingLen, ending) == 0;
}

std::vector<std::string> split(const std::string& str, const std::string& delim)
{
	std::vector<std::string> out;
	std::string::size_type beg = 0;
	for (std::string::size_type end = 0; (end = str.find(delim, end)) != std::string::npos; ++end) {
		out.push_back(str.substr(beg, end - beg));
		beg = end + 1;
	}
	out.push_back(str.substr(beg));
	return out;
}

std::string join(const std::vector<std::string>& strings, const std::string& delim)
{
	std::string ret;
	if (delim.empty()) {
		for (const std::string& s : strings)
			ret += s;
	} else {
		for (const std::string& s : strings) {
			ret += s;
			if (&s != &strings.back())
				ret += delim;
		}
	}
	return ret;
}

} // namespace utils
