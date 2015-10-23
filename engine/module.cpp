#include "module.hpp"
#include <chrono>
#include <thread>
#include <SDL2/SDL_loadso.h>

static inline string getPath(const string& moduleName)
{
#if defined(WIN32) || defined(_WIN32)
	return moduleName + ".dll";
#else
	return "./lib" + moduleName + ".so";
#endif
}

Module::Module(const string& moduleName)
{
	name = moduleName;
	string path = getPath(moduleName);
	handle = SDL_LoadObject(path.c_str());
	if (!handle) {
		logError("%s", SDL_GetError()); // SDL already produces a descriptive error
		ASSERT(!"Module load failed");
		return;
	}
	func = (ModuleFunc)SDL_LoadFunction(handle, "ModuleFunc");
	if (!func) {
		logError("%s (%s)", SDL_GetError(), path.c_str());
		ASSERT(!"ModuleFunc load failed");
		return;
	}
	mtime = timestamp(path);
	logDebug("Loaded module %s", path.c_str());
}

Module::~Module()
{
	if (handle) {
		SDL_UnloadObject(handle);
		handle = 0;
		logDebug("Unloaded module %s", name.c_str());
	}
}

void Modules::load(const Json& modules)
{
	clear();
	if (modules.is_array()) {
		for (auto& it : modules.array_items())
			emplace(it.string_value(), it.string_value());
	}
}

void Modules::reload(const string& name)
{
	erase(name);
	emplace(name, name);
}

bool Modules::autoReload()
{
	for (auto& it : *this) {
		string name = it.second.name;
		string path = getPath(name);
		if (timestamp(path) > it.second.mtime) {
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
			reload(name);
			return true; // Iterators are now invalid
		}
	}
	return false;
}

void Modules::call(uint msg, void* param)
{
	for (auto& it : *this)
		if (it.second.func && it.second.enabled)
			it.second.func(msg, param);
}

void Modules::call(const string& module, uint msg, void* param)
{
	const auto it = find(module);
	if (it != end() && it->second.func && it->second.enabled)
		it->second.func(msg, param);
}
