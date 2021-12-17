#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <functional>

namespace utils {

	// Timing
	struct Timer {
		Timer() : t(std::chrono::steady_clock::now()) {}
		float ms() {
			auto t2 = std::chrono::steady_clock::now();
			std::chrono::duration<float, std::milli> duration = t2 - t;
			t = std::chrono::steady_clock::now();
			return duration.count();
		}
		std::chrono::steady_clock::time_point t;
		static uint64_t now_ms() { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count(); }
	};

	void sleep(unsigned ms);

	// File I/O
	unsigned timestamp(const std::string& path);
	std::string readFile(const std::string& path, bool binary = false);
	bool writeFile(const std::string& path, const std::string& contents, bool binary = false, bool append = false);
	bool fileExists(const std::string& path);
	bool copyFiles(const std::string& from, const std::string& to);
	bool deleteFile(const std::string& path);
	bool deleteRecursive(const std::string& path);
	bool createDirectories(const std::string& path);
	std::string getTempDir(const std::string& subdir = "", bool clearSubdir = false);
	std::vector<std::string> findFiles(const std::string& path, const std::string& contains = "", const std::vector<std::string>& exclude = {});
	std::string getExeDir();
	std::string removeFilename(const std::string& path);
	std::string removeExtension(const std::string& path);
	std::string extractFilename(const std::string& path);
	std::string joinPaths(const std::string& a, const std::string& b);
	inline std::string joinPaths(const std::string& a, const std::string& b, const std::string& c) { return joinPaths(joinPaths(a, b), c); }

	int execute(const std::string& cmd);
	int openUrl(const std::string& url); // Can be file

	// String manipulation
	std::string nicifyVariableName(const std::string& name);
	std::string variablizeName(const std::string& name, bool capitalizeFirst = false);
	std::string insensitivize(const std::string& name);
	std::string tolower(std::string s);
	std::string toupper(std::string s);
	std::string replace(std::string str, const std::string& search, const std::string& replace);
	std::string trim(std::string s);
	std::string ltrim(std::string s);
	std::string rtrim(std::string s);
	std::string parseEscapes(const std::string& str);
	inline bool contains(const std::string& str, const std::string& what) { return str.find(what) != std::string::npos; }
	inline bool startsWith(const std::string& str, const std::string& starting) { return str.rfind(starting, 0) == 0; }
	bool endsWith(const std::string& str, const std::string& ending);
	std::vector<std::string> split(const std::string& str, const std::string& delim);
	std::string join(const std::vector<std::string>& strings, const std::string& delim = "");

	// Array
	template<typename T>
	constexpr inline int indexOf(const T& needle, const T haystack[], int size) {
		for (int i = 0; i < size; ++i)
			if (haystack[i] == needle)
				return i;
		return -1;
	}
	template<typename T>
	inline int indexOf(const T& needle, const std::vector<T>& haystack) {
		return indexOf(needle, haystack.data(), (int)haystack.size());
	}
	template<typename T>
	inline bool contains(const std::vector<T>& haystack, const T& needle) {
		return indexOf(needle, haystack) >= 0;
	}
	template<typename T>
	constexpr inline T* find(T haystack[], int size, std::function<bool(const T&)> predicate) {
		for (int i = 0; i < size; ++i)
			if (predicate(haystack[i]))
				return &haystack[i];
		return nullptr;
	}
	template<typename T>
	inline T* find(std::vector<T>& haystack, std::function<bool(const T&)> predicate) {
		return find(haystack.data(), (int)haystack.size(), predicate);
	}
	template<typename T>
	constexpr inline const T* find(const T haystack[], int size, std::function<bool(const T&)> predicate) {
		for (int i = 0; i < size; ++i)
			if (predicate(haystack[i]))
				return &haystack[i];
		return nullptr;
	}
	template<typename T>
	inline const T* find(const std::vector<T>& haystack, std::function<bool(const T&)> predicate) {
		return find(haystack.data(), (int)haystack.size(), predicate);
	}
	template<typename T>
	inline bool erase(std::vector<T>& haystack, const T& needle) {
		for (int i = 0; i < (int)haystack.size(); ++i)
			if (haystack[i] == needle) {
				haystack.erase(haystack.begin() + i);
				return true;
			}
		return false;
	}
	template<typename T>
	inline bool erase(std::vector<T>& haystack, std::function<bool(const T&)> predicate) {
		for (int i = 0; i < (int)haystack.size(); ++i)
			if (predicate(haystack[i])) {
				haystack.erase(haystack.begin() + i);
				return true;
			}
		return false;
	}
	template<typename T>
	inline int eraseAll(std::vector<T>& haystack, std::function<bool(const T&)> predicate) {
		int count = 0;
		for (int i = (int)haystack.size() - 1; i >= 0; --i)
			if (predicate(haystack[i])) {
				haystack.erase(haystack.begin() + i);
				count++;
			}
		return count;
	}
	template<typename T>
	inline int eraseAll(std::vector<T>& haystack, const T& needle) {
		int count = 0;
		for (int i = (int)haystack.size() - 1; i >= 0; --i)
			if (haystack[i] == needle) {
				haystack.erase(haystack.begin() + i);
				count++;
			}
		return count;
	}
	template<typename T>
	inline bool pushUnique(std::vector<T>& array, const T& item) {
		if (!contains(array, item)) {
			array.push_back(item);
			return true;
		}
		return false;
	}

} // namespace utils
